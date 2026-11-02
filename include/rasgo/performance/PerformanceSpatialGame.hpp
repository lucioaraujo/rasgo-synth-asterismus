#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "PerformanceContracts.hpp"

namespace rasgo::performance {

struct PerformanceSpatialPoint {
    double x = 0.5;
    double y = 0.5;

    bool valid() const noexcept { return unit(x) && unit(y); }
};

struct PerformanceSpatialField {
    std::array<std::uint16_t, kConstellationActorCapacity> actorIds{};
    std::array<PerformanceSpatialPoint, kConstellationActorCapacity> points{};
    std::array<double, kConstellationRelationCapacity> relationIntensities{};
    std::size_t actorCount = 0U;
    std::size_t relationCount = 0U;
    std::uint16_t focusActorId = kInvalidActorId;

    bool valid() const noexcept {
        if (actorCount > points.size() || relationCount > relationIntensities.size()) return false;
        for (std::size_t i = 0U; i < actorCount; ++i) {
            if (actorIds[i] == kInvalidActorId || !points[i].valid()) return false;
            for (std::size_t j = 0U; j < i; ++j)
                if (actorIds[i] == actorIds[j]) return false;
        }
        for (std::size_t i = 0U; i < relationCount; ++i)
            if (!unit(relationIntensities[i])) return false;
        return focusActorId == kInvalidActorId || actorCount > 0U;
    }
};

struct PerformanceSpatialGameState {
    std::uint32_t seed = 0U;
    std::uint32_t turn = 0U;
    double autonomy = 0.65;
    double density = 0.50;
    double tension = 0.35;
    double memory = 0.50;
    double risk = 0.35;
    PerformanceSpatialField field{};

    bool valid() const noexcept {
        return unit(autonomy) && unit(density) && unit(tension)
            && unit(memory) && unit(risk) && field.valid();
    }
};

enum class PerformanceSpatialMove : std::uint8_t {
    Sustain,
    Approach,
    Answer,
    Contest,
    Withhold,
    Break,
    Exchange,
    Count
};

struct PerformanceSpatialGameEvent {
    PerformanceSpatialMove move = PerformanceSpatialMove::Sustain;
    PerformanceGameStrategy strategy = PerformanceGameStrategy::Follow;
    PerformanceGameRule rule = PerformanceGameRule::Preserve;
    std::uint16_t sourceActorId = kInvalidActorId;
    std::uint16_t targetActorId = kInvalidActorId;
    double densityDelta = 0.0;
    double tensionDelta = 0.0;
    double memoryDelta = 0.0;
    double relationDelta = 0.0;
    double spatialInfluence = 0.0;
    std::uint32_t turn = 0U;

    bool valid() const noexcept {
        return static_cast<std::uint8_t>(move) < static_cast<std::uint8_t>(PerformanceSpatialMove::Count)
            && static_cast<std::uint8_t>(strategy) < static_cast<std::uint8_t>(PerformanceGameStrategy::Count)
            && static_cast<std::uint8_t>(rule) < static_cast<std::uint8_t>(PerformanceGameRule::Count)
            && sourceActorId != kInvalidActorId && targetActorId != kInvalidActorId
            && sourceActorId != targetActorId && finite(densityDelta)
            && finite(tensionDelta) && finite(memoryDelta) && finite(relationDelta)
            && unit(spatialInfluence);
    }
};

// Native, allocation-free translation of the Studio's game idea. It produces
// an event for modules to interpret; it never changes positions, relations or
// actor presence. The same seed/turn/field always yields the same move.
class PerformanceSpatialGame {
public:
    PerformanceSpatialGame() = default;
    explicit PerformanceSpatialGame(std::uint32_t seed) noexcept : seed_(seed) {}

    // A seed-only configuration begins a native, self-contained field.  It
    // must not accidentally retain the imported policy of a previous field.
    void configure(std::uint32_t seed) noexcept {
        seed_ = seed;
        gamePlan_ = {};
    }
    void configure(std::uint32_t seed, const PerformanceGamePlan& gamePlan) noexcept {
        if (!gamePlan.valid()) return;
        seed_ = seed;
        gamePlan_ = gamePlan;
    }
    std::uint32_t seed() const noexcept { return seed_; }
    const PerformanceGamePlan& gamePlan() const noexcept { return gamePlan_; }

    PerformanceSpatialGameEvent propose(const PerformanceSpatialGameState& state) const noexcept {
        PerformanceSpatialGameEvent event;
        event.turn = state.turn;
        if (state.seed != seed_ || !state.valid() || state.field.actorCount < 2U) return event;

        std::size_t source = 0U;
        std::size_t target = 1U;
        if (state.field.focusActorId != kInvalidActorId) {
            for (std::size_t i = 0U; i < state.field.actorCount; ++i) {
                if (state.field.points[i].valid() && state.field.focusActorId == state.field.actorIds[i]) {
                    source = i;
                    break;
                }
            }
        }
        target = nearestOther(state.field, source);
        if (target == source) return event;

        const auto& a = state.field.points[source];
        const auto& b = state.field.points[target];
        const double dx = a.x - b.x;
        const double dy = a.y - b.y;
        const double distance = std::clamp(std::sqrt(dx * dx + dy * dy) / 1.41421356237, 0.0, 1.0);
        const double relation = state.field.relationCount == 0U
            ? 0.0 : state.field.relationIntensities[(state.turn + source + target)
                                                     % state.field.relationCount];
        const double spatial = std::clamp(0.58 * (1.0 - distance) + 0.42 * relation, 0.0, 1.0);
        event.strategy = gamePlan_.strategy;
        event.rule = gamePlan_.rule;
        const auto choice = static_cast<std::uint8_t>(
            (mix(state.seed, state.turn) + static_cast<std::uint8_t>(event.strategy) * 2U
             + static_cast<std::uint8_t>(event.rule)) % 7U);
        event.move = static_cast<PerformanceSpatialMove>(choice);
        event.sourceActorId = state.field.actorIds[source];
        event.targetActorId = state.field.actorIds[target];
        event.spatialInfluence = spatial;
        const double force = 0.02 + 0.08 * (0.25 * state.autonomy
                                           + 0.20 * state.risk
                                           + 0.20 * spatial
                                           + 0.20 * gamePlan_.energy
                                           + 0.15 * gamePlan_.freedom);
        switch (event.move) {
            case PerformanceSpatialMove::Sustain: event.memoryDelta = force * state.memory; break;
            case PerformanceSpatialMove::Approach: event.densityDelta = force; event.relationDelta = force * spatial; break;
            case PerformanceSpatialMove::Answer: event.tensionDelta = force * (0.4 + spatial); event.densityDelta = force * 0.5; break;
            case PerformanceSpatialMove::Contest: event.tensionDelta = force * (0.7 + state.risk); break;
            case PerformanceSpatialMove::Withhold: event.densityDelta = -force * (0.6 + state.memory); event.memoryDelta = force * 0.4; break;
            case PerformanceSpatialMove::Break: event.tensionDelta = force * (0.8 + state.risk); event.memoryDelta = force * 0.6; break;
            case PerformanceSpatialMove::Exchange: event.relationDelta = force * (0.5 + spatial); event.memoryDelta = force * 0.35; break;
            case PerformanceSpatialMove::Count: break;
        }
        return event;
    }

private:
    static std::size_t nearestOther(const PerformanceSpatialField& field, std::size_t source) noexcept {
        std::size_t result = source;
        double best = 2.0;
        for (std::size_t i = 0U; i < field.actorCount; ++i) {
            if (i == source) continue;
            const double dx = field.points[source].x - field.points[i].x;
            const double dy = field.points[source].y - field.points[i].y;
            const double distance = dx * dx + dy * dy;
            if (distance < best) { best = distance; result = i; }
        }
        return result;
    }

    static std::uint32_t mix(std::uint32_t seed, std::uint32_t turn) noexcept {
        std::uint32_t value = seed ^ (turn * 0x9e3779b9U) ^ 0x85ebca6bU;
        value ^= value >> 16U; value *= 0x7feb352dU;
        value ^= value >> 15U; value *= 0x846ca68bU; value ^= value >> 16U;
        return value;
    }

    std::uint32_t seed_ = 0U;
    PerformanceGamePlan gamePlan_{};
};

} // namespace rasgo::performance
