#pragma once

#include "PerformanceContracts.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace rasgo::performance {

// One sample of each body's own stem (one module, one stem, since
// 2026-10-28). Actor identity owns the stem; role is scene metadata and may
// morph without changing routing.
struct PerformanceActorStemSample {
    std::uint16_t actorId = kInvalidActorId;
    PerformanceRole role = PerformanceRole::Corpo;
    ActorLifeState life = ActorLifeState::Dormant;
    double mono = 0.0;

    bool valid() const noexcept {
        return actorId != kInvalidActorId
            && static_cast<std::uint8_t>(role) < static_cast<std::uint8_t>(PerformanceRole::Count)
            && static_cast<std::uint8_t>(life) < static_cast<std::uint8_t>(ActorLifeState::Count)
            && finite(mono);
    }
};

struct PerformanceStemFrame {
    std::array<PerformanceActorStemSample, kPerformanceStemActorCapacity> actors{};
    std::size_t actorCount = 0U;
    double modalReturn = 0.0;

    const PerformanceActorStemSample* findActor(std::uint16_t actorId) const noexcept {
        for (std::size_t index = 0U; index < actorCount; ++index)
            if (actors[index].actorId == actorId) return &actors[index];
        return nullptr;
    }

    double mixedMono() const noexcept {
        double sum = 0.0;
        for (std::size_t index = 0U; index < actorCount; ++index) sum += actors[index].mono;
        sum += modalReturn;
        return std::clamp(finite(sum) ? sum : 0.0, -0.90, 0.90);
    }

    bool valid() const noexcept {
        if (actorCount > actors.size() || !finite(modalReturn)) return false;
        for (std::size_t index = 0U; index < actorCount; ++index) {
            if (!actors[index].valid()) return false;
            for (std::size_t other = index + 1U; other < actorCount; ++other)
                if (actors[index].actorId == actors[other].actorId) return false;
        }
        return true;
    }
};

} // namespace rasgo::performance
