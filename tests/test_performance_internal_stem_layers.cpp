#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include <rasgo/performance/PerformanceActorEcology.hpp>
#include <rasgo/performance/PerformanceSessionEngine.hpp>
#include <rasgo/performance/PerformanceTerritoryFactory.hpp>

namespace {
using namespace rasgo::performance;

// internalLayerMix reports the largest participating layer, not just the
// native-to-native invitation (0.34). The 8->4 fold and owned Modal return
// have distinct bounded coefficients. Keep their actual budgets explicit.
constexpr double foldedLayerCeiling = (0.12 + 0.12 + 0.12 + 0.10)
    * (0.86 + 0.18 + 0.10);
constexpr double modalLayerCeiling = 0.18 + 0.28 + 0.10;
constexpr double reportedLayerCeiling = modalLayerCeiling > foldedLayerCeiling
    ? modalLayerCeiling : foldedLayerCeiling;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

PerformanceScene scene() {
    PerformanceScene out;
    out.id = 1U;
    out.session.territorySeed = 181U;
    out.session.playing = true;
    out.session.currentSceneId = 1U;
    out.session.targetSceneId = 1U;
    out.session.density = 0.70;
    out.session.memory = 0.72;
    out.session.tension = 0.58;
    expect(out.constellation.addActor({1U, PerformanceRole::Chao, ActorLifeState::Active, 0.78, 0.72, 0.56, 0.80}),
           "layer fixture adds first actor");
    expect(out.constellation.addActor({2U, PerformanceRole::Corpo, ActorLifeState::Active, 0.70, 0.66, 0.50, 0.76}),
           "layer fixture adds second actor");
    expect(out.constellation.addActor({3U, PerformanceRole::Atrito, ActorLifeState::Active, 0.62, 0.60, 0.46, 0.72}),
           "layer fixture adds third actor");
    expect(out.constellation.addRelation({1U, 2U, RelationKind::Contagion, RelationDomain::Matter,
                                          0.90, 12.0, 1.0, 0.85}),
           "layer fixture adds first relation");
    expect(out.constellation.addRelation({2U, 3U, RelationKind::Friction, RelationDomain::Time,
                                          0.88, 10.0, 1.0, 0.82}),
           "layer fixture adds second relation");
    return out;
}

void testInternalLayerMixPreservesFieldGeometry() {
    const auto sourceScene = scene();
    const auto morph = PerformanceSceneMorpher::morph(sourceScene, sourceScene, 1.0);
    PerformanceActorEcology ecology;
    expect(ecology.prepare(48000.0, 64U, 2U), "layer ecology prepares");
    ecology.setTerritoryIdentity(TerritoryIdentity{}, 181U);
    ecology.setMorph(morph);
    ecology.setMacroState(sourceScene.session);
    const auto firstAnchor = ecology.actorPosition(1U);
    const auto secondAnchor = ecology.actorPosition(2U);
    double greatestLayer = 0.0;
    double renderedEnergy = 0.0;
    for (int sample = 0; sample < 24000; ++sample) {
        const auto frame = ecology.renderStems(48000.0, 0.0, 0.0);
        expect(frame.valid(), "layer rendering remains finite and bounded");
        expect(frame.actorCount <= kPerformanceSafeBodyLimit,
               "internal layers never add a production output stem");
        for (std::size_t stem = 0U; stem < frame.actorCount; ++stem)
            renderedEnergy += frame.actors[stem].mono * frame.actors[stem].mono;
        greatestLayer = std::max({greatestLayer, ecology.internalLayerMix(1U),
                                  ecology.internalLayerMix(2U), ecology.internalLayerMix(3U)});
    }
    std::cout << "greatest internal layer: " << greatestLayer << '\n';
    // Since 2026-10-28 one module is one stem: no module (Modal included) is
    // folded as a layer inside another body's stem.
    (void) reportedLayerCeiling;
    expect(greatestLayer == 0.0, "no module is folded into another body's stem");
    expect(renderedEnergy > 1.0e-12, "owned output stems contain nonzero audio");
    expect(ecology.actorPosition(1U).x == firstAnchor.x && ecology.actorPosition(1U).y == firstAnchor.y
               && ecology.actorPosition(2U).x == secondAnchor.x && ecology.actorPosition(2U).y == secondAnchor.y,
           "internal mixing never rewrites the performer-authored field positions");
}

void testComplementaryFourthMatterCanEnterTheInternalMix() {
    const auto document = PerformanceTerritoryFactory::makeWithOptionalModule(184U, true);
    expect(document.valid() && document.territory.actorCount == 4U,
           "digital seed prepares its complementary fourth matter");
    const auto morph = PerformanceSceneMorpher::morph(document.scenes[0], document.scenes[0], 1.0);
    PerformanceActorEcology ecology;
    expect(ecology.prepare(48000.0, 64U, 2U), "fourth-matter ecology prepares");
    ecology.setTerritoryIdentity(document.territory.identity, document.territory.seed,
                                 document.territory.gamePlan);
    ecology.setMorph(morph);
    ecology.setMacroState(document.scenes[0].session);
    const auto fourthAnchor = ecology.actorPosition(4U);
    double fourthLayer = 0.0;
    double fourthEnergy = 0.0;
    for (int sample = 0; sample < 48000; ++sample) {
        const auto frame = ecology.renderStems(48000.0, 0.0, 0.0);
        expect(frame.valid() && frame.findActor(4U) != nullptr,
               "fourth matter remains a valid owned stem");
        fourthLayer = std::max(fourthLayer, ecology.internalLayerMix(4U));
        fourthEnergy += frame.findActor(4U)->mono * frame.findActor(4U)->mono;
    }
    // Since 2026-10-28: the fourth star sounds on its own stem and nothing is
    // folded into it.
    expect(fourthLayer == 0.0 && fourthEnergy > 1.0e-9,
           "the seed-selected fourth star sounds on its own stem, with no folded layer");
    expect(ecology.actorPosition(4U).x == fourthAnchor.x
               && ecology.actorPosition(4U).y == fourthAnchor.y,
           "layering the fourth matter does not rewrite its field position");

    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 64U, 2U)
               && engine.setScene(document.scenes[0], document.territory),
           "layer telemetry engine prepares the same four-matter territory");
    std::array<float, 64> left{}, right{};
    bool telemetryReportedLayer = false;
    for (int block = 0; block < 800; ++block) {
        engine.processBlock(left.data(), right.data(), 64U);
        const auto telemetry = engine.readTelemetry();
        for (std::size_t stem = 0U; stem < telemetry.stemCount; ++stem)
            telemetryReportedLayer = telemetryReportedLayer
                || telemetry.stemLayerMixMicros[stem] > 0U;
    }
    expect(!telemetryReportedLayer,
           "telemetry reports no folded layer, since none exists");
}

void testCuesFallOnTheSeedScale() {
    // The sky web's answers are snapped to the seed's scale (root, mode and
    // tonal transposition used to tune the bodies): idempotent, never more
    // than a whole tone away, always finite.
    PerformanceActorEcology ecology;
    expect(ecology.prepare(48000.0, 64U, 2U), "scale ecology prepares");
    for (const auto seed : {181U, 772637541U, 3570930745U}) {
        ecology.setTerritoryIdentity(TerritoryIdentity{}, seed);
        for (double hz = 40.0; hz < 2000.0; hz *= 1.0137) {
            const double once = ecology.quantizeToSeedScale(hz);
            const double twice = ecology.quantizeToSeedScale(once);
            expect(std::isfinite(once) && once > 0.0, "the snapped pitch is finite");
            expect(std::fabs(twice - once) < 1.0e-9 * once, "snapping is idempotent");
            expect(std::fabs(std::log2(once / hz)) <= 2.0 / 12.0 + 1.0e-9,
                   "snapping never moves a pitch by more than a whole tone");
        }
    }
}

} // namespace

int main() {
    testInternalLayerMixPreservesFieldGeometry();
    testComplementaryFourthMatterCanEnterTheInternalMix();
    testCuesFallOnTheSeedScale();
    std::cout << "Performance internal stem layer tests passed\n";
}
