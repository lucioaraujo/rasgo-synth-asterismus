#pragma once

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace rasgo::performance {

// Performance-owned adaptation of the Studio Tuba path: a low, near-harmonic
// modal bank driven by a bounded breath-like excitation. Provenance is kept in
// PROVENIENCIA_TUBA_BREATH.md; no Studio header is included at runtime.
class TubaBreathVoice {
public:
    void prepare(double sampleRate, std::uint32_t seed) noexcept {
        sampleRate_ = std::max(8000.0, sampleRate); state_ = {}; noise_ = seed ? seed : 1U;
        for (std::size_t i = 0; i < ratios_.size(); ++i) {
            ratios_[i] = static_cast<double>(i + 1U) + (unit() - .5) * .03;
            gains_[i] = 1.0 / static_cast<double>((i + 1U) * (i + 1U));
        }
        setFrequency(55.0);
    }
    void setFrequency(double hz) noexcept {
        for (std::size_t i = 0; i < ratios_.size(); ++i) {
            const double f = std::clamp(hz * ratios_[i], 20.0, sampleRate_ * .45);
            const double r = std::exp(-1.0 / (sampleRate_ * (.06 / (i + 1.0) + .01)));
            a1_[i] = 2.0 * r * std::cos(6.283185307179586 * f / sampleRate_); a2_[i] = r * r;
        }
    }
    double process(double breath, double brightness) noexcept {
        breathEnv_ += (std::clamp(breath, 0.0, 1.0) - breathEnv_) * .004;
        const double excitation = (unit() * 2.0 - 1.0) * breathEnv_ * .18;
        double out = 0.0;
        for (std::size_t i = 0; i < state_.size(); ++i) {
            const double y = a1_[i] * state_[i][0] - a2_[i] * state_[i][1] + excitation * gains_[i];
            state_[i][1] = state_[i][0]; state_[i][0] = std::isfinite(y) && std::fabs(y) < 50.0 ? y : 0.0;
            out += state_[i][0] * (i == 0U ? 1.0 : std::clamp(brightness, 0.0, 1.0));
        }
        return std::tanh(out * .22);
    }
private:
    double unit() noexcept { noise_ = noise_ * 1664525U + 1013904223U; return double(noise_ >> 8U) / 16777215.0; }
    double sampleRate_ = 48000.0, breathEnv_ = 0.0; std::uint32_t noise_ = 1U;
    std::array<double, 6> ratios_{}, gains_{}, a1_{}, a2_{};
    std::array<std::array<double, 2>, 6> state_{};
};

} // namespace rasgo::performance
