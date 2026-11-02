#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "PerformanceContracts.hpp"

#if defined(RASGO_PERFORMANCE_WITH_RASGO_FRACTURE)
#include <dsp/RasgoFracture.hpp>
#endif

namespace rasgo::performance {

struct RasgoFractureDescriptor {
    static constexpr std::uint32_t kContractVersion = 5U;
    const char* adapterId = "rasgo.performance.rasgo-fracture";
    const char* implementationId = "rasgo-core-rasgo-fracture-v5";
    const char* auditedCoreRevision = "local-core-2026-08-30";
    const char* auditedHeaderSha256 =
        "83c92aefe7cccdb302ea498a3db26a6bd63768ab286970b2f0c739b0c92a0a0c";
    std::uint32_t latencySamples = 0U;
};

struct RasgoFractureParameters {
    std::uint32_t seed = 1U;
    double stress = 0.0;
    double pressure = 0.0;
    double repair = 0.0;

    bool valid() const noexcept {
        return unit(stress) && unit(pressure) && unit(repair);
    }
};

class RasgoFractureAdapter {
public:
    bool prepare(double sampleRate, std::uint32_t maximumBlockSize,
                 std::uint32_t channels) noexcept {
        if (!finite(sampleRate) || sampleRate < 8000.0 || sampleRate > 384000.0
            || maximumBlockSize == 0U || channels == 0U || channels > 2U)
            return false;
        sampleRate_ = sampleRate;
        prepared_ = true;
        reset();
        return true;
    }

    void reset() noexcept {
        scar_ = 0.0;
        repair_ = 0.0;
        ruptureImpulse_ = 0.0;
        parameters_ = {};
#if defined(RASGO_PERFORMANCE_WITH_RASGO_FRACTURE)
        fracture_.configure(parameters_.seed);
#endif
    }

    bool setParameters(const RasgoFractureParameters& parameters) noexcept {
        if (!prepared_ || !parameters.valid()) return false;
        parameters_ = parameters;
#if defined(RASGO_PERFORMANCE_WITH_RASGO_FRACTURE)
        if (parameters_.seed != configuredSeed_) {
            fracture_.configure(parameters_.seed);
            configuredSeed_ = parameters_.seed;
            scar_ = 0.0;
            repair_ = 0.0;
            ruptureImpulse_ = 0.0;
        }
#endif
        return true;
    }

    // Called at a bounded musical/control boundary, never once per sample.
    // The Core's broken fibres remain broken until reset; repair only reduces
    // the presented scar and therefore cannot rewrite provenance.
    void applyStress(double amount) noexcept {
        if (!prepared_) return;
        const double safe = std::clamp(finite(amount) ? amount : 0.0, 0.0, 1.0);
#if defined(RASGO_PERFORMANCE_WITH_RASGO_FRACTURE)
        const int newBreaks = fracture_.applyAmbientStress(
            safe * (0.22 + parameters_.pressure * 0.78));
        if (newBreaks > 0) {
            ruptureImpulse_ = std::min(1.0, ruptureImpulse_ + 0.22 * newBreaks);
            scar_ = std::max(scar_, static_cast<double>(fracture_.totalBreaks()) / 5.0);
        }
#else
        ruptureImpulse_ = std::min(1.0, ruptureImpulse_ + safe * 0.22);
        scar_ = std::max(scar_, safe * (0.35 + parameters_.pressure * 0.65));
#endif
    }

    float process() noexcept {
        if (!prepared_) return 0.0F;
        repair_ += (parameters_.repair - repair_) * 0.0025;
        ruptureImpulse_ *= 0.9992;
        scar_ = std::max(0.0, scar_ * 0.999985 - repair_ * 0.00045);
        const double output = std::clamp(
            scar_ * (0.24 + parameters_.pressure * 0.28)
                + ruptureImpulse_ * 0.32,
            0.0, 1.0);
        return static_cast<float>(finite(output) ? output : 0.0);
    }

    void setRepair(double amount) noexcept { parameters_.repair = clampUnit(amount); }
    double scar() const noexcept { return scar_; }
    int totalBreaks() const noexcept {
#if defined(RASGO_PERFORMANCE_WITH_RASGO_FRACTURE)
        return fracture_.totalBreaks();
#else
        return static_cast<int>(std::round(scar_ * 5.0));
#endif
    }
    bool prepared() const noexcept { return prepared_; }
    RasgoFractureDescriptor descriptor() const noexcept { return {}; }

private:
    bool prepared_ = false;
    double sampleRate_ = 48000.0;
    RasgoFractureParameters parameters_{};
    double scar_ = 0.0;
    double repair_ = 0.0;
    double ruptureImpulse_ = 0.0;
    std::uint32_t configuredSeed_ = 1U;
#if defined(RASGO_PERFORMANCE_WITH_RASGO_FRACTURE)
    rasgo::RasgoFracture fracture_{};
#endif
};

} // namespace rasgo::performance
