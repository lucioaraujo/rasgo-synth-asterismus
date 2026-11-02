#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "PerformanceSpatialGame.hpp"

namespace rasgo::performance {

// Structural capacity for the accumulating desk. This is intentionally larger
// than the current measured realtime limit: a module may be admitted to the
// desk only when the host capacity policy also permits its stem to sound.
constexpr std::size_t kCumulativeDeskSlotCapacity = kConstellationActorCapacity;

struct PerformanceDeskSlot {
    std::uint16_t actorId = kInvalidActorId;
    std::uint16_t libraryEntry = std::numeric_limits<std::uint16_t>::max();
    PerformanceSpatialPoint position{};
    double presence = 0.0;
    bool occupied = false;
    bool audible = false;

    bool valid() const noexcept {
        return occupied && actorId != kInvalidActorId
            && libraryEntry != std::numeric_limits<std::uint16_t>::max()
            && position.valid() && unit(presence);
    }
};

class PerformanceCumulativeDesk {
public:
    static constexpr std::size_t kCapacity = kCumulativeDeskSlotCapacity;

    void clear() noexcept { slots_ = {}; }

    bool add(std::uint16_t actorId, std::uint16_t libraryEntry,
             PerformanceSpatialPoint position, double presence,
             bool audible) noexcept {
        if (actorId == kInvalidActorId || libraryEntry == std::numeric_limits<std::uint16_t>::max()
            || !position.valid() || !unit(presence) || contains(actorId)) return false;
        for (auto& slot : slots_) {
            if (slot.occupied) continue;
            slot = {actorId, libraryEntry, position, presence, true, audible};
            return true;
        }
        return false;
    }

    bool setPresence(std::uint16_t actorId, double presence, bool audible) noexcept {
        if (!unit(presence)) return false;
        auto* slot = find(actorId);
        if (slot == nullptr) return false;
        slot->presence = presence;
        slot->audible = audible;
        return true;
    }

    bool setPosition(std::uint16_t actorId, PerformanceSpatialPoint position) noexcept {
        if (!position.valid()) return false;
        auto* slot = find(actorId);
        if (slot == nullptr) return false;
        slot->position = position;
        return true;
    }

    bool remove(std::uint16_t actorId) noexcept {
        auto* slot = find(actorId);
        if (slot == nullptr) return false;
        *slot = {};
        return true;
    }

    bool contains(std::uint16_t actorId) const noexcept { return find(actorId) != nullptr; }
    std::size_t occupiedCount() const noexcept {
        std::size_t count = 0U;
        for (const auto& slot : slots_) count += slot.occupied ? 1U : 0U;
        return count;
    }
    std::size_t audibleCount() const noexcept {
        std::size_t count = 0U;
        for (const auto& slot : slots_) count += slot.occupied && slot.audible ? 1U : 0U;
        return count;
    }
    const PerformanceDeskSlot* slot(std::size_t index) const noexcept {
        return index < slots_.size() && slots_[index].occupied ? &slots_[index] : nullptr;
    }

private:
    PerformanceDeskSlot* find(std::uint16_t actorId) noexcept {
        for (auto& slot : slots_)
            if (slot.occupied && slot.actorId == actorId) return &slot;
        return nullptr;
    }
    const PerformanceDeskSlot* find(std::uint16_t actorId) const noexcept {
        for (const auto& slot : slots_)
            if (slot.occupied && slot.actorId == actorId) return &slot;
        return nullptr;
    }

    std::array<PerformanceDeskSlot, kCapacity> slots_{};
};

} // namespace rasgo::performance
