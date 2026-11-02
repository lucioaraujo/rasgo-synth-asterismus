#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "PerformanceContracts.hpp"

namespace rasgo::performance {

// The musical lifetime of one recorded realisation.  This type deliberately
// owns no audio buffers, filesystem handles or clocks: it is safe to advance
// from the audio callback and gives the host unambiguous points to open and
// close a recording outside that callback.
enum class PerformanceMorceauState : std::uint8_t {
    Prepared,
    Opening,
    Developing,
    Coda,
    Tail,
    Finalized,
    Interrupted
};

struct PerformanceMorceauPlan {
    double openingSeconds = 0.24;
    double codaSeconds = 4.0;
    double tailSeconds = 1.2;

    bool valid() const noexcept {
        return finite(openingSeconds) && finite(codaSeconds) && finite(tailSeconds)
            && openingSeconds >= 0.02 && openingSeconds <= 4.0
            && codaSeconds >= 0.25 && codaSeconds <= 30.0
            && tailSeconds >= 0.0 && tailSeconds <= 12.0;
    }
};

class PerformanceMorceauLifecycle {
public:
    bool prepare(double sampleRate, std::uint32_t seed,
                 PerformanceMorceauPlan requested = {}) noexcept {
        if (!finite(sampleRate) || sampleRate < 8000.0 || sampleRate > 384000.0
            || !requested.valid())
            return false;

        sampleRate_ = sampleRate;
        seed_ = seed;
        plan_ = requested;
        openingSamples_ = samplesFor(plan_.openingSeconds);
        codaSamples_ = samplesFor(plan_.codaSeconds);
        tailSamples_ = samplesFor(plan_.tailSeconds);
        state_ = PerformanceMorceauState::Prepared;
        phaseSamples_ = 0U;
        return true;
    }

    bool begin() noexcept {
        if (state_ != PerformanceMorceauState::Prepared
            && state_ != PerformanceMorceauState::Finalized
            && state_ != PerformanceMorceauState::Interrupted)
            return false;
        state_ = PerformanceMorceauState::Opening;
        phaseSamples_ = 0U;
        return true;
    }

    // A coda is intentional and reversible only before it is requested.  The
    // host should first tell the ecology to compose its closing gestures, then
    // call this at the selected musical boundary.
    bool beginCoda() noexcept {
        if (state_ != PerformanceMorceauState::Opening
            && state_ != PerformanceMorceauState::Developing)
            return false;
        state_ = PerformanceMorceauState::Coda;
        phaseSamples_ = 0U;
        return true;
    }

    void interrupt() noexcept {
        state_ = PerformanceMorceauState::Interrupted;
        phaseSamples_ = 0U;
    }

    // Returns the master musical envelope for this sample and advances one
    // sample.  The tail is deliberately silent but remains timed so a recorder
    // can retain decay/reverb already emitted by the DSP without being cut at
    // the coda boundary.
    double nextGain() noexcept {
        switch (state_) {
        case PerformanceMorceauState::Opening:
            return advanceOpening();
        case PerformanceMorceauState::Developing:
            return 1.0;
        case PerformanceMorceauState::Coda:
            return advanceCoda();
        case PerformanceMorceauState::Tail:
            advanceTail();
            return 0.0;
        case PerformanceMorceauState::Prepared:
        case PerformanceMorceauState::Finalized:
        case PerformanceMorceauState::Interrupted:
            return 0.0;
        }
        return 0.0;
    }

    PerformanceMorceauState state() const noexcept { return state_; }
    const PerformanceMorceauPlan& plan() const noexcept { return plan_; }
    std::uint32_t seed() const noexcept { return seed_; }
    bool recordingShouldRemainOpen() const noexcept {
        return state_ == PerformanceMorceauState::Opening
            || state_ == PerformanceMorceauState::Developing
            || state_ == PerformanceMorceauState::Coda
            || state_ == PerformanceMorceauState::Tail;
    }
    bool finished() const noexcept {
        return state_ == PerformanceMorceauState::Finalized
            || state_ == PerformanceMorceauState::Interrupted;
    }

private:
    std::uint64_t samplesFor(double seconds) const noexcept {
        return std::max<std::uint64_t>(1U, static_cast<std::uint64_t>(std::llround(seconds * sampleRate_)));
    }

    double advanceOpening() noexcept {
        const auto denominator = std::max<std::uint64_t>(1U, openingSamples_ - 1U);
        const auto position = std::min(phaseSamples_, denominator);
        const double amount = static_cast<double>(position) / static_cast<double>(denominator);
        const double gain = 0.5 - 0.5 * std::cos(amount * 3.14159265358979323846);
        ++phaseSamples_;
        if (phaseSamples_ >= openingSamples_) {
            state_ = PerformanceMorceauState::Developing;
            phaseSamples_ = 0U;
        }
        return gain;
    }

    double advanceCoda() noexcept {
        const auto denominator = std::max<std::uint64_t>(1U, codaSamples_ - 1U);
        const auto position = std::min(phaseSamples_, denominator);
        const double amount = static_cast<double>(position) / static_cast<double>(denominator);
        const double gain = 0.5 + 0.5 * std::cos(amount * 3.14159265358979323846);
        ++phaseSamples_;
        if (phaseSamples_ >= codaSamples_) {
            state_ = PerformanceMorceauState::Tail;
            phaseSamples_ = 0U;
        }
        return gain;
    }

    void advanceTail() noexcept {
        ++phaseSamples_;
        if (phaseSamples_ >= tailSamples_) {
            state_ = PerformanceMorceauState::Finalized;
            phaseSamples_ = 0U;
        }
    }

    double sampleRate_ = 48000.0;
    std::uint32_t seed_ = 0U;
    PerformanceMorceauPlan plan_{};
    PerformanceMorceauState state_ = PerformanceMorceauState::Prepared;
    std::uint64_t openingSamples_ = 1U;
    std::uint64_t codaSamples_ = 1U;
    std::uint64_t tailSamples_ = 1U;
    std::uint64_t phaseSamples_ = 0U;
};

} // namespace rasgo::performance
