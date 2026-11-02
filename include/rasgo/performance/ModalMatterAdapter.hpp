#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

#include "PerformanceContracts.hpp"

#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
#include <dsp/ModalResonator.hpp>
#endif

namespace rasgo::performance {

// This boundary deliberately knows the audited Studio source only through
// provenance strings. It must compile without its headers until a fixed package
// and compatibility tests are approved.
struct ModalMatterDescriptor {
    static constexpr std::uint32_t kContractVersion = 1U;
    const char* adapterId = "rasgo.performance.modal-matter";
    const char* implementationId = "native-contract-double";
    const char* auditedCoreRevision = "ae33cba5a1104bccf9427b9d7cf229250cc4dcd5";
    const char* auditedHeaderSha256 = "5e87d761f3f9b07ee337234b4e402fb6571e2dba44ce09df4f1593649628b996";
    std::uint32_t latencySamples = 0U;
};

struct ModalMatterParameters {
    double fundamentalHz = 110.0;
    double brightness = 0.55;
    double excitation = 0.0;
    double relationPressure = 0.0;
    double memory = 0.5;
    PerformanceRole role = PerformanceRole::Corpo;

    bool valid() const noexcept {
        return finite(fundamentalHz) && fundamentalHz >= 20.0 && fundamentalHz <= 8000.0
            && unit(brightness) && unit(excitation) && unit(relationPressure) && unit(memory)
            && static_cast<std::uint8_t>(role) < static_cast<std::uint8_t>(PerformanceRole::Count);
    }
};

class ModalMatterAdapter {
public:
    virtual ~ModalMatterAdapter() = default;
    virtual bool prepare(double sampleRate, std::uint32_t maximumBlockSize,
                         std::uint32_t channels) noexcept = 0;
    virtual void reset() noexcept = 0;
    virtual bool setParameters(const ModalMatterParameters& parameters) noexcept = 0;
    virtual void excite(double amount) noexcept = 0;
    virtual float process() noexcept = 0;
    virtual ModalMatterDescriptor descriptor() const noexcept = 0;
};

// Independent temporary realization of the contract. It is intentionally not
// an implementation of ModalResonator and must never be represented as one.
class NativeModalMatterDouble final : public ModalMatterAdapter {
public:
    bool prepare(double sampleRate, std::uint32_t maximumBlockSize,
                 std::uint32_t channels) noexcept override {
        if (!finite(sampleRate) || sampleRate < 8000.0 || sampleRate > 384000.0
            || maximumBlockSize == 0U || channels == 0U || channels > 2U)
            return false;
        sampleRate_ = sampleRate;
        prepared_ = true;
        reset();
        return true;
    }

    void reset() noexcept override {
        phases_ = {};
        energies_ = {};
        parameters_ = {};
    }

    bool setParameters(const ModalMatterParameters& parameters) noexcept override {
        if (!parameters.valid() || !prepared_) return false;
        parameters_ = parameters;
        return true;
    }

    void excite(double amount) noexcept override {
        const double safe = clampUnit(amount);
        for (std::size_t index = 0; index < energies_.size(); ++index)
            energies_[index] = std::min(1.0, energies_[index] + safe * (1.0 - index * 0.12));
    }

    float process() noexcept override {
        if (!prepared_) return 0.0F;
        if (parameters_.excitation > 0.0) excite(parameters_.excitation * 0.015);
        static constexpr std::array<double, 4> ratios{{1.0, 2.07, 3.19, 4.42}};
        double out = 0.0;
        for (std::size_t index = 0; index < ratios.size(); ++index) {
            const double frequency = std::min(parameters_.fundamentalHz * ratios[index], sampleRate_ * 0.45);
            phases_[index] += frequency / sampleRate_;
            if (phases_[index] >= 1.0) phases_[index] -= std::floor(phases_[index]);
            const double colour = index == 0U ? 1.0 : parameters_.brightness;
            out += std::sin(phases_[index] * kTwoPi) * energies_[index] * colour;
            const double decay = 0.99982 + parameters_.memory * 0.00010
                - index * 0.00002 - parameters_.relationPressure * 0.00003;
            energies_[index] *= std::clamp(decay, 0.9996, 0.9999);
        }
        return static_cast<float>(std::clamp(finite(out) ? out * roleGain(parameters_.role) * 0.20 : 0.0,
                                              -0.90, 0.90));
    }

    ModalMatterDescriptor descriptor() const noexcept override { return {}; }

private:
    static constexpr double kTwoPi = 6.28318530717958647692;
    bool prepared_ = false;
    double sampleRate_ = 48000.0;
    ModalMatterParameters parameters_{};
    std::array<double, 4> phases_{};
    std::array<double, 4> energies_{};

    static double roleGain(PerformanceRole role) noexcept {
        switch (role) {
        case PerformanceRole::Chao: return 0.85;
        case PerformanceRole::Corpo: return 1.0;
        case PerformanceRole::Atrito: return 0.72;
        case PerformanceRole::Contagio: return 0.78;
        case PerformanceRole::Rasgo: return 0.65;
        case PerformanceRole::Vestigio: return 0.45;
        case PerformanceRole::Count: break;
        }
        return 1.0;
    }
};

#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
// Realisation backed by the hash-pinned rasgo::ModalResonator package. The
// adapter owns the body directly: no heap, lookup or cross-process access is
// required during audio processing.
class CoreModalMatterAdapter final : public ModalMatterAdapter {
public:
    bool prepare(double sampleRate, std::uint32_t maximumBlockSize,
                 std::uint32_t channels) noexcept override {
        if (!finite(sampleRate) || sampleRate < 8000.0 || sampleRate > 384000.0
            || maximumBlockSize == 0U || channels == 0U || channels > 2U)
            return false;
        sampleRate_ = sampleRate;
        prepared_ = true;
        reset();
        return true;
    }

    void reset() noexcept override {
        resonator_ = rasgo::ModalResonator{};
        resonator_.setSampleRate(sampleRate_);
        parameters_ = {};
        configureBody();
    }

    bool setParameters(const ModalMatterParameters& parameters) noexcept override {
        if (!prepared_ || !parameters.valid()) return false;
        parameters_ = parameters;
        configureBody();
        return true;
    }

    void excite(double amount) noexcept override {
        if (!prepared_) return;
        const auto pressureGain = 0.55 + parameters_.relationPressure * 0.35;
        resonator_.excite(static_cast<float>(clampUnit(amount) * pressureGain));
    }

    float process() noexcept override {
        if (!prepared_) return 0.0F;
        if (parameters_.excitation > 0.0)
            excite(parameters_.excitation * 0.01);
        const auto output = static_cast<double>(resonator_.process()) * roleGain(parameters_.role);
        return static_cast<float>(std::clamp(finite(output) ? output : 0.0, -0.90, 0.90));
    }

    ModalMatterDescriptor descriptor() const noexcept override {
        auto out = ModalMatterDescriptor{};
        out.implementationId = "rasgo-core-modal-resonator-v1";
        return out;
    }

private:
    bool prepared_ = false;
    double sampleRate_ = 48000.0;
    ModalMatterParameters parameters_{};
    rasgo::ModalResonator resonator_{};

    void configureBody() noexcept {
        const auto pressure = parameters_.relationPressure;
        const std::array<double, rasgo::ModalResonator::kNumModes> ratios{
            1.0, 2.01 + pressure * 0.03, 3.06 + pressure * 0.07,
            4.15 + pressure * 0.11, 5.35 + pressure * 0.16, 6.70 + pressure * 0.22};
        const auto bodyDecay = roleDecay(parameters_.role) * (0.55 + parameters_.memory * 1.10);
        const std::array<double, rasgo::ModalResonator::kNumModes> decays{
            bodyDecay, bodyDecay * 0.78, bodyDecay * 0.59,
            bodyDecay * 0.43, bodyDecay * 0.31, bodyDecay * 0.22};
        const std::array<double, rasgo::ModalResonator::kNumModes> gains{
            1.0, 0.62, 0.41, 0.28, 0.18, 0.11};
        resonator_.setFundamental(parameters_.fundamentalHz);
        resonator_.setModes(ratios, decays, gains);
        resonator_.setBrightness(static_cast<float>(parameters_.brightness));
    }

    static double roleDecay(PerformanceRole role) noexcept {
        switch (role) {
        case PerformanceRole::Chao: return 2.4;
        case PerformanceRole::Corpo: return 1.6;
        case PerformanceRole::Atrito: return 0.82;
        case PerformanceRole::Contagio: return 1.15;
        case PerformanceRole::Rasgo: return 0.46;
        case PerformanceRole::Vestigio: return 3.2;
        case PerformanceRole::Count: break;
        }
        return 1.2;
    }

    static double roleGain(PerformanceRole role) noexcept {
        switch (role) {
        case PerformanceRole::Chao: return 0.70;
        case PerformanceRole::Corpo: return 0.82;
        case PerformanceRole::Atrito: return 0.58;
        case PerformanceRole::Contagio: return 0.64;
        case PerformanceRole::Rasgo: return 0.54;
        case PerformanceRole::Vestigio: return 0.46;
        case PerformanceRole::Count: break;
        }
        return 0.65;
    }
};
#endif

} // namespace rasgo::performance
