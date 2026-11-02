#pragma once

#include <array>
#include <algorithm>
#include <cstdint>

namespace rasgo::performance {

struct PerformanceSeedEvent {
    std::uint32_t ordinal = 0U;
    std::uint8_t variation = 0U;
    // Duration before this event may answer an unattended field. The opening
    // is intentionally prompt; following turns form a seed-specific rhythm.
    std::uint8_t pauseBeats = 4U;
    double densityBias = 0.0;
    double tensionBias = 0.0;
    double matterBias = 0.0;
    double memoryBias = 0.0;
};

// A fixed, deterministic history for a territory. It is intentionally not a
// random generator in the audio callback: all 32 phrase events are prepared
// from the seed and then read by ordinal. The musician can perturb the current
// event, but when the surface is left alone the seeded history continues.
class PerformanceSeedTrajectory {
public:
    static constexpr std::size_t kEventCapacity = 32U;

    void configure(std::uint32_t seed) noexcept {
        seed_ = seed;
        for (std::size_t index = 0U; index < events_.size(); ++index) {
            const auto a = unit(seed_, static_cast<std::uint32_t>(index * 5U + 1U));
            const auto b = unit(seed_, static_cast<std::uint32_t>(index * 5U + 2U));
            const auto c = unit(seed_, static_cast<std::uint32_t>(index * 5U + 3U));
            const auto d = unit(seed_, static_cast<std::uint32_t>(index * 5U + 4U));
            const std::uint8_t pauseBeats = index == 0U ? static_cast<std::uint8_t>(4U)
                : static_cast<std::uint8_t>(4U + 2U
                    * (static_cast<std::uint8_t>(unit(seed_, static_cast<std::uint32_t>(index * 5U + 6U))
                                                   * 3.0) % 3U));
            events_[index] = {
                static_cast<std::uint32_t>(index),
                static_cast<std::uint8_t>(unit(seed_, static_cast<std::uint32_t>(index * 5U + 5U)) * 7.0),
                pauseBeats,
                (a - 0.5) * 0.28,
                (b - 0.5) * 0.32,
                (c - 0.5) * 0.26,
                (d - 0.5) * 0.24};
        }
    }

    std::uint32_t seed() const noexcept { return seed_; }

    const PerformanceSeedEvent& event(std::uint32_t ordinal) const noexcept {
        return events_[ordinal % events_.size()];
    }

    // Structural collage: trajectories are combined by their prepared traits,
    // never by arithmetically mixing seed integers. At the extremes this is
    // exactly either source; between them, biases interpolate while a bounded
    // variation/pause is chosen from the nearer structural source.
    static PerformanceSeedEvent collage(const PerformanceSeedEvent& primary,
                                        const PerformanceSeedEvent& secondary,
                                        double amount) noexcept {
        amount = std::clamp(amount, 0.0, 1.0);
        const bool secondaryOwnsDiscreteTraits = amount >= 0.5;
        const auto lerp = [amount](double first, double second) noexcept {
            return first + (second - first) * amount;
        };
        return {primary.ordinal,
                secondaryOwnsDiscreteTraits ? secondary.variation : primary.variation,
                secondaryOwnsDiscreteTraits ? secondary.pauseBeats : primary.pauseBeats,
                lerp(primary.densityBias, secondary.densityBias),
                lerp(primary.tensionBias, secondary.tensionBias),
                lerp(primary.matterBias, secondary.matterBias),
                lerp(primary.memoryBias, secondary.memoryBias)};
    }

private:
    static double unit(std::uint32_t seed, std::uint32_t index) noexcept {
        std::uint32_t value = seed ^ (index * 0x9e3779b9U) ^ 0x85ebca6bU;
        value ^= value >> 16U;
        value *= 0x7feb352dU;
        value ^= value >> 15U;
        value *= 0x846ca68bU;
        value ^= value >> 16U;
        return static_cast<double>(value & 0xffffU) / 65535.0;
    }

    std::uint32_t seed_ = 0U;
    std::array<PerformanceSeedEvent, kEventCapacity> events_{};
};

} // namespace rasgo::performance
