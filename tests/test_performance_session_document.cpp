#include <cstdlib>
#include <iostream>
#include <string>

#include <rasgo/performance/PerformanceSessionDocument.hpp>

namespace {

using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

PerformanceSessionDocument makeDocument() {
    PerformanceSessionDocument document;
    document.territory.seed = 181U;
    document.territory.identity = {0.61, 0.44, 0.72, 0.53, 0.84, 0.67};
    document.territory.gamePlan = {PerformanceGameStrategy::Counter,
                                   PerformanceGameRule::Oppose,
                                   0.71, 0.62, 0.48, 0.57, 0.33};
    expect(document.territory.addActor({1U, PerformanceFamily::Tone, 0x03U,
                                        roleBit(PerformanceRole::Chao) | roleBit(PerformanceRole::Corpo),
                                        0.65, 0.05, 0.90}), "ground potential enters territory");
    expect(document.territory.addActor({2U, PerformanceFamily::Texture, 0x14U,
                                        roleBit(PerformanceRole::Atrito) | roleBit(PerformanceRole::Vestigio),
                                        0.50, 0.0, 1.0}), "texture potential enters territory");

    PerformanceScene opening;
    opening.id = 7U;
    opening.session.territorySeed = document.territory.seed;
    opening.session.currentSceneId = opening.id;
    opening.session.targetSceneId = opening.id;
    opening.session.tempoBpm = 126.25;
    opening.session.autonomy = 0.72;
    opening.session.outputTrimDb = -2.5;
    opening.session.populationPhenomenon = PerformancePopulationPhenomenon::Fragment;
    opening.session.populationPhenomenonIntensity = 0.68;
    opening.session.populationPhenomenonEnabled = true;
    expect(opening.constellation.addActor({1U, PerformanceRole::Chao, ActorLifeState::Active,
                                           0.65, 0.51, 0.48, 0.80}), "ground actor enters opening");
    expect(opening.constellation.addActor({2U, PerformanceRole::Atrito, ActorLifeState::Emerging,
                                           0.32, 0.62, 0.30, 0.73}), "texture actor enters opening");
    PerformanceRelation relation{1U, 2U, RelationKind::Friction, RelationDomain::Matter,
                                 0.57, 18.0, 2.5, 0.77};
    relation.phenomenon = PerformanceRelationalPhenomenon::SpiralIn;
    relation.resonanceRatio = 1.5;
    relation.phenomenonEnabled = true;
    expect(opening.constellation.addRelation(relation), "relation enters opening");
    expect(document.addScene(opening), "opening enters document");

    auto rupture = opening;
    rupture.id = 12U;
    rupture.session.currentSceneId = rupture.id;
    rupture.session.targetSceneId = rupture.id;
    rupture.session.tempoBpm = 154.0;
    rupture.session.risk = 0.88;
    rupture.session.panicSafe = true;
    rupture.constellation.actors[1].role = PerformanceRole::Vestigio;
    rupture.constellation.actors[1].presence = 0.78;
    expect(document.addScene(rupture), "rupture enters document");
    expect(document.valid(), "fixture document is valid");
    return document;
}

void testRoundTrip() {
    const auto source = makeDocument();
    const auto encoded = serializeSessionDocument(source);
    expect(!encoded.empty() && encoded.rfind("RSPERF 4\n", 0U) == 0U,
           "document has canonical versioned header");
    PerformanceSessionDocument decoded;
    const auto result = parseSessionDocument(encoded, decoded);
    expect(result.ok() && decoded.valid(), "canonical document parses to a valid document");
    expect(decoded.territory.seed == 181U && decoded.territory.actorCount == 2U,
           "territory remains intact through round trip");
    expect(decoded.territory.gamePlan.strategy == PerformanceGameStrategy::Counter
           && decoded.territory.gamePlan.rule == PerformanceGameRule::Oppose
           && decoded.territory.gamePlan.energy == 0.71,
           "game plan remains intact through round trip");
    expect(decoded.sceneCount == 2U && decoded.scenes[1].id == 12U,
           "ordered scenes remain intact through round trip");
    expect(decoded.scenes[0].constellation.relationCount == 1U
           && decoded.scenes[1].session.panicSafe
           && decoded.scenes[1].constellation.actors[1].role == PerformanceRole::Vestigio
           && decoded.scenes[0].constellation.relations[0].phenomenon
                == PerformanceRelationalPhenomenon::SpiralIn
           && decoded.scenes[0].constellation.relations[0].resonanceRatio == 1.5
           && decoded.scenes[0].constellation.relations[0].phenomenonEnabled
           && decoded.scenes[0].session.populationPhenomenon
                == PerformancePopulationPhenomenon::Fragment
           && decoded.scenes[0].session.populationPhenomenonIntensity == 0.68
           && decoded.scenes[0].session.populationPhenomenonEnabled,
           "scene state and relations remain intact through round trip");
}

void testRejectionAndBoundaries() {
    PerformanceSessionDocument destination = makeDocument();
    const auto originalSeed = destination.territory.seed;
    const auto originalSceneCount = destination.sceneCount;
    const auto wrongVersion = parseSessionDocument("RSPERF 5\n", destination);
    expect(wrongVersion.error == SessionDocumentError::Version, "unknown version is rejected");
    expect(destination.territory.seed == originalSeed && destination.sceneCount == originalSceneCount,
           "failed parse never replaces an existing session");

    const auto tooManyActors = parseSessionDocument(
        "RSPERF 1\nterritory 1 0.5 0.5 0.5 0.5 0.5 0.5\nactors 97\n", destination);
    expect(tooManyActors.error == SessionDocumentError::Range, "actor count is bounded before allocation");

    const auto foreignScene = parseSessionDocument(
        "RSPERF 1\nterritory 181 0.5 0.5 0.5 0.5 0.5 0.5\nactors 0\nscenes 1\n"
        "scene 1 999 120 0 0 0.65 0.35 0.5 0.35 0.5 0.35 0.5 1 1 1 0 0\n"
        "sceneactors 0\nscenerelations 0\nendscene\nend\n", destination);
    expect(foreignScene.error == SessionDocumentError::Scene, "scene cannot cross territory seed");

    std::string oversized(kSessionDocumentMaximumBytes + 1U, 'x');
    const auto tooLarge = parseSessionDocument(oversized, destination);
    expect(tooLarge.error == SessionDocumentError::TooLarge, "input byte size is bounded");

    const auto canonical = serializeSessionDocument(makeDocument());
    for (std::size_t length = 0; length + 1U < canonical.size(); ++length) {
        const auto truncated = parseSessionDocument(std::string_view(canonical).substr(0U, length), destination);
        expect(!truncated.ok(), "every truncated canonical prefix is rejected");
        expect(destination.territory.seed == originalSeed && destination.sceneCount == originalSceneCount,
               "truncated input preserves previously loaded session");
    }
    const auto trailing = parseSessionDocument(canonical + "unexpected\n", destination);
    expect(trailing.error == SessionDocumentError::TrailingData, "trailing data is rejected");
}

} // namespace

int main() {
    testRoundTrip();
    testRejectionAndBoundaries();
    std::cout << "rasgo_synth_performance_session_document_tests: OK\n";
    return 0;
}
