#include <cstdlib>
#include <iostream>

#include <rasgo/performance/PerformanceSpatialGame.hpp>

namespace {
using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}

PerformanceSpatialGameState state(bool near) {
    PerformanceSpatialGameState result;
    result.seed = 181U;
    result.field.actorCount = 3U;
    result.field.actorIds[0] = 11U;
    result.field.actorIds[1] = 22U;
    result.field.actorIds[2] = 33U;
    result.field.relationCount = 2U;
    result.field.relationIntensities[0] = 0.65;
    result.field.relationIntensities[1] = 0.35;
    result.field.points[0] = {0.20, 0.30};
    result.field.points[1] = near ? PerformanceSpatialPoint{0.24, 0.34}
                                 : PerformanceSpatialPoint{0.90, 0.85};
    result.field.points[2] = {0.70, 0.60};
    result.field.focusActorId = 11U;
    return result;
}

int mainImpl() {
    PerformanceSpatialGame game(181U);
    const auto first = game.propose(state(true));
    const auto second = game.propose(state(true));
    const auto far = game.propose(state(false));
    expect(first.valid(), "situated event is valid");
    expect(first.sourceActorId == second.sourceActorId && first.targetActorId == second.targetActorId
        && first.spatialInfluence == second.spatialInfluence, "same seed and field are deterministic");
    expect(first.spatialInfluence != far.spatialInfluence, "different fields influence the move");
    expect(first.sourceActorId == 11U, "focus remains the source");
    expect(first.strategy != PerformanceGameStrategy::Count
        && first.rule != PerformanceGameRule::Count,
        "seed policy is exposed as a bounded strategy and rule");
    expect(first.strategy == second.strategy && first.rule == second.rule,
        "same seed and field reproduce the same game policy");
    expect(first.densityDelta <= 0.10 && first.densityDelta >= -0.10, "density consequence is bounded");
    PerformanceGamePlan imported;
    imported.strategy = PerformanceGameStrategy::Interrupt;
    imported.rule = PerformanceGameRule::Break;
    imported.energy = 0.72;
    imported.memory = 0.31;
    imported.risk = 0.87;
    imported.freedom = 0.66;
    imported.hiddenInformation = 0.42;
    game.configure(181U, imported);
    const auto studioPolicy = game.propose(state(true));
    expect(studioPolicy.valid()
           && studioPolicy.strategy == PerformanceGameStrategy::Interrupt
           && studioPolicy.rule == PerformanceGameRule::Break,
           "an imported Studio game plan remains the policy of the live field");
    return 0;
}
}

int main() { return mainImpl(); }
