#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include <rasgo/performance/PerformanceSessionController.hpp>
#include <rasgo/performance/PerformanceTerritoryFactory.hpp>

namespace {

using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

// Since 2026-10-28 the seed chooses stars, not cohorts: find the first seed
// whose asterism lights `moduleId` on body 4 instead of fixing a seed number
// from the former seed % 5 rule.
std::uint32_t seedWithFourth(std::string_view moduleId) {
    for (std::uint32_t seed = 181U; seed < 181U + 100000U; ++seed)
        if (performanceStarLibraryId(performanceInitialAsterism(seed)[3]) == moduleId) return seed;
    std::cerr << "FAIL: no seed lights " << moduleId << " on body 4\n";
    std::exit(1);
}

bool fourthIs(std::uint32_t seed, std::string_view moduleId) {
    return performanceStarLibraryId(performanceInitialAsterism(seed)[3]) == moduleId;
}

PerformanceSessionDocument makeDocument() {
    PerformanceSessionDocument document;
    document.territory.seed = 181U;
    expect(document.territory.addActor({1U, PerformanceFamily::Tone, 1U,
                                        roleBit(PerformanceRole::Chao) | roleBit(PerformanceRole::Corpo),
                                        0.6, 0.0, 1.0}), "actor enters territory");
    PerformanceScene opening;
    opening.id = 10U;
    opening.session.territorySeed = document.territory.seed;
    opening.session.currentSceneId = opening.id;
    opening.session.targetSceneId = opening.id;
    opening.session.playing = true;
    expect(opening.constellation.addActor({1U, PerformanceRole::Chao, ActorLifeState::Active,
                                           0.65, 0.60, 0.5, 0.8}), "actor enters opening");
    expect(opening.constellation.addActor({2U, PerformanceRole::Corpo, ActorLifeState::Emerging,
                                           0.42, 0.55, 0.4, 0.7}), "second actor enters opening");
    expect(document.territory.addActor({2U, PerformanceFamily::Texture, 1U,
                                        roleBit(PerformanceRole::Corpo), 0.5, 0.0, 1.0}),
           "second actor enters territory");
    expect(opening.constellation.addRelation({1U, 2U, RelationKind::Support, RelationDomain::Matter,
                                              0.50, 10.0, 1.0, 0.8}), "relation enters opening");
    expect(document.addScene(opening), "opening enters document");
    auto escalation = opening;
    escalation.id = 20U;
    escalation.session.currentSceneId = escalation.id;
    escalation.session.targetSceneId = escalation.id;
    escalation.session.risk = 0.82;
    escalation.constellation.actors[0].role = PerformanceRole::Corpo;
    expect(document.addScene(escalation), "escalation enters document");
    return document;
}

void testControlFlow() {
    PerformanceSessionController controller;
    const auto document = makeDocument();
    expect(!controller.loadDocument(document), "document cannot load before audio preparation");
    expect(controller.prepare(48000.0, 256U, 2U), "controller prepares its own engine");
    expect(controller.loadDocument(document), "controller loads initial scene from document");
    expect(controller.document() != nullptr && controller.currentSceneId() == 10U,
           "first scene becomes current by default");
    expect(!controller.recallScene(999U, 1.0), "unknown scene is rejected");
    expect(!controller.recallScene(20U, 0.01), "unsafe morph duration is rejected");
    expect(controller.recallScene(20U, 0.25) && controller.currentSceneId() == 10U,
           "controller queues recall without mutating audio scene on control thread");
    expect(controller.setMacro(PerformanceMacro::Density, 0.82)
           && !controller.setMacro(PerformanceMacro::Density, 1.1),
           "controller queues only bounded macro variation");
    expect(controller.setTempoBpm(148.0) && !controller.setTempoBpm(19.0)
           && !controller.setTempoBpm(301.0),
           "controller queues only safe explicit tempo changes");
    expect(controller.setQuantizedGestures(true),
           "controller queues quantized gesture mode without touching audio state");
    expect(controller.setGestureGrid(true, GestureGridDivision::Half)
           && !controller.setGestureGrid(true, static_cast<GestureGridDivision>(3U)),
           "controller accepts only declared gesture-grid divisions");
    expect(controller.setActorPresence(1U, 0.24)
           && !controller.setActorPresence(kInvalidActorId, 0.4),
           "controller queues only bounded actor-presence commands");
    expect(controller.setModuleControl(1U, PerformanceModuleControl::Primary, 0.82)
           && controller.setModuleControl(5U, PerformanceModuleControl::Primary, 0.82)
           && controller.setModuleControl(8U, PerformanceModuleControl::Motion, 0.36)
           && !controller.setModuleControl(99U, PerformanceModuleControl::Primary, 0.5)
           && !controller.setModuleControl(1U, PerformanceModuleControl::Count, 0.5)
           && !controller.setModuleControl(1U, PerformanceModuleControl::Motion, 1.1),
           "controller queues bounded controls for production stems and internal module lanes");
    expect(controller.setRelationIntensity(1U, 2U, 0.66),
           "controller queues relation controls only for declared relations");
    expect(controller.setRelationPhenomenon(
               1U, 2U, PerformanceRelationalPhenomenon::Merge, 1.5)
               && !controller.setRelationPhenomenon(
                   1U, 99U, PerformanceRelationalPhenomenon::Merge)
               && !controller.setRelationPhenomenon(
                   1U, 2U, PerformanceRelationalPhenomenon::Count),
           "controller queues only declared musical phenomena on an existing pair");
    expect(controller.setMotionField(PerformanceMotionField::Pulse)
           && controller.setMotionField(PerformanceMotionField::FigureEight)
           && !controller.setMotionField(static_cast<PerformanceMotionField>(3U)),
           "controller queues only declared motion-field choices");
    expect(controller.setPopulationPhenomenon(PerformancePopulationPhenomenon::Cluster, 0.74)
           && !controller.setPopulationPhenomenon(
               static_cast<PerformancePopulationPhenomenon>(10U), 0.74)
           && !controller.setPopulationPhenomenon(PerformancePopulationPhenomenon::Cluster, 1.1),
           "controller queues only bounded collective phenomena");
    expect(controller.setActorMotionField(1U, PerformanceMotionField::Pulse)
           && !controller.setActorMotionField(99U, PerformanceMotionField::Pulse)
           && !controller.setActorMotionField(1U, static_cast<PerformanceMotionField>(3U)),
           "controller queues individual dance only for a declared body");
    expect(controller.setActorMotionDirection(1U, false)
           && !controller.setActorMotionDirection(99U, false),
           "controller queues individual dance direction only for a declared body");
    expect(controller.setSeedCollage(182U, 0.58) && !controller.setSeedCollage(182U, 1.1),
           "controller queues only bounded structural seed collage choices");
    expect(controller.setActorSeedVariation(1U, 0.72)
           && controller.setActorSeedVariation(5U, 0.72)
           && controller.setActorSeedVariation(8U, 0.28)
           && !controller.setActorSeedVariation(99U, 0.72)
           && !controller.setActorSeedVariation(1U, 1.1),
           "controller queues bounded variation for declared bodies and folded internal modules");
    PerformanceGestureEvent stutter;
    stutter.type = GestureType::Stutter;
    stutter.targetActorId = 1U;
    stutter.intensity = 0.82;
    stutter.duration = 0.55;
    PerformanceGestureEvent tempoShift = stutter;
    tempoShift.type = GestureType::TempoShift;
    PerformanceGestureEvent gridCue = stutter;
    gridCue.type = GestureType::GridCue;
    expect(controller.pushGesture(stutter) && controller.pushGesture(tempoShift)
               && controller.pushGesture(gridCue),
           "controller forwards bounded recut gestures through the existing realtime event queue");
    expect(controller.setPlaying(false), "controller queues transport without touching the audio thread");
    expect(controller.setOutputTrimDb(-8.0) && !controller.setOutputTrimDb(-24.1),
           "controller queues only bounded master trim commands");
    expect(controller.setStemGain(1U, 0.45) && !controller.setStemGain(99U, 0.45)
           && !controller.setStemGain(1U, 1.1) && controller.setStemMuted(2U, true)
           && controller.setStemSolo(1U, true),
           "controller queues bounded stem gain, mute and solo commands");

    PerformanceGestureEvent gesture;
    gesture.type = GestureType::Mutate;
    gesture.intensity = 0.7;
    expect(controller.pushGesture(gesture), "loaded controller sends gesture to engine");
    float left[256]{};
    float right[256]{};
    controller.processBlock(left, right, 256U);
    expect(controller.readTelemetry().quantizedGestures
           && controller.readTelemetry().gestureGridDivision == static_cast<std::uint8_t>(GestureGridDivision::Half),
           "quantize mode and selected division cross the bounded controller queue into telemetry");
    expect(!controller.setRelationIntensity(1U, 99U, 0.5),
           "controller accepts relations only after they exist in the loaded constellation");
    expect(controller.currentSceneId() == 20U && controller.readTelemetry().sceneId == 10U,
           "audio thread begins scene morph and telemetry retains its source until progression");
    for (int block = 0; block < 48; ++block) controller.processBlock(left, right, 256U);
    expect(controller.readTelemetry().activeActorCount == 2U
           && controller.readTelemetry().sceneId == 20U,
           "controller preserves ecology and reaches recalled scene through audio processing");

    expect(controller.setPlaying(true), "PLAY queues before recall-continuity regression");
    controller.processBlock(left, right, 256U);
    expect(controller.readTelemetry().playing,
           "explicit PLAY is active before recalling a scene that stored STOP");
    auto stoppedScene = document.scenes[0];
    stoppedScene.session.playing = false;
    expect(controller.captureScene(stoppedScene),
           "controller accepts a stopped scene document without changing live transport");
    controller.processBlock(left, right, 256U);
    expect(controller.recallScene(stoppedScene.id, 0.25),
           "controller queues a captured stopped scene while performance is playing");
    for (int block = 0; block < 48; ++block) controller.processBlock(left, right, 256U);
    expect(controller.currentSceneId() == stoppedScene.id && controller.readTelemetry().playing,
           "scene recall preserves explicit PLAY even when captured scene stored STOP");

    expect(controller.setPanicSafe(true), "controller queues PanicSafe latch without direct engine mutation");
    controller.processBlock(left, right, 256U);
    expect(controller.readTelemetry().panicSafe,
           "PanicSafe latch crosses the bounded controller queue into telemetry");
    expect(controller.setPanicSafe(false), "controller queues explicit PanicSafe release");
    controller.processBlock(left, right, 256U);
    expect(!controller.readTelemetry().panicSafe,
           "PanicSafe release crosses the bounded controller queue into telemetry");

    auto captured = document.scenes[1];
    captured.session.risk = 0.23;
    captured.constellation.actors[0].presence = 0.31;
    expect(controller.captureScene(captured),
           "controller queues a valid captured scene without touching audio state directly");
    controller.processBlock(left, right, 256U);
    expect(controller.document() != nullptr && controller.document()->scenes[1].session.risk == 0.23
           && controller.document()->scenes[1].constellation.actors[0].presence == 0.31,
           "audio side installs capture only as a future-recall document update");

    expect(controller.captureScene(document.scenes[1]),
           "controller accepts a previous scene as an explicit capture restore");
    controller.processBlock(left, right, 256U);
    expect(controller.document()->scenes[1].session.risk
               == std::min(document.scenes[1].session.risk, document.territory.identity.riskLimit)
           && controller.document()->scenes[1].constellation.actors[0].presence
               == document.scenes[1].constellation.actors[0].presence,
           "later capture restores the prior scene without crossing its territory risk budget");

    // A territory change is a bounded live transition and may be queued while
    // the current session is playing.
    const auto newTerritory = PerformanceTerritoryFactory::make(31337U);
    expect(controller.queueTerritory(newTerritory),
           "playing session accepts a bounded prepared territory command");
    expect(controller.document() != nullptr
           && controller.document()->territory.seed == 31337U,
           "control side publishes only the accepted prepared territory copy");
    controller.processBlock(left, right, 256U);
    expect(controller.document() != nullptr && controller.document()->territory.seed == 31337U
           && controller.currentSceneId() == 1U && controller.readTelemetry().playing,
           "audio installs the new territory before immediate PLAY instead of discarding it");
}

void testRejectedCommandsDoNotPublishControlState() {
    PerformanceSessionController controller;
    const auto startup = PerformanceTerritoryFactory::make(181U);
    expect(controller.prepare(48000.0, 64U, 2U) && controller.loadDocument(startup),
           "queue-publication fixture starts stopped");

    const auto firstTerritory = PerformanceTerritoryFactory::make(182U);
    const auto rejectedTerritory = PerformanceTerritoryFactory::make(183U);
    expect(controller.queueTerritory(firstTerritory)
           && !controller.queueTerritory(rejectedTerritory),
           "bounded territory queue rejects replacement instead of overwriting");
    expect(controller.document() != nullptr
           && controller.document()->territory.seed == 182U,
           "rejected territory never replaces the accepted control document");

    PerformanceSessionController captures;
    expect(captures.prepare(48000.0, 64U, 2U) && captures.loadDocument(startup),
           "capture-publication fixture loads a private control document");
    auto scene = startup.scenes[0];
    double lastAcceptedDensity = scene.session.density;
    for (std::size_t index = 0U;
         index + 1U < PerformanceSessionController::kSceneCaptureQueueCapacity; ++index) {
        scene.session.density = 0.10 + static_cast<double>(index) * 0.07;
        expect(captures.captureScene(scene), "capture enters available bounded slot");
        lastAcceptedDensity = scene.session.density;
    }
    scene.session.density = 0.97;
    expect(!captures.captureScene(scene), "full capture queue reports rejection");
    expect(captures.document()->scenes[0].session.density == lastAcceptedDensity,
           "rejected capture leaves the last accepted control snapshot visible");
}

void testAccumulatedWalshControlPath() {
    PerformanceSessionController controller;
    const auto document = PerformanceTerritoryFactory::makeWithWalsh(seedWithFourth("walsh"), true);
    expect(controller.prepare(48000.0, 64U, 2U) && controller.loadDocument(document),
           "accumulated Walsh controller loads the four-body document");
    expect(controller.setPlaying(true)
               && controller.addLibraryModule(4U, "walsh", 0.20, 0.46, 0.78)
               && controller.setRelationIntensity(1U, 4U, 0.73),
           "Walsh desk admission, position and orbit cross the controller queues");
    expect(controller.cumulativeDesk().occupiedCount() == 1U
               && controller.cumulativeDesk().slot(0U)->libraryEntry == 8U,
           "controller exposes the explicit library provenance of the inserted module");
    float left[64]{};
    float right[64]{};
    std::uint32_t observedFourthPeak = 0U;
    for (int block = 0; block < 240; ++block) {
        controller.processBlock(left, right, 64U);
        observedFourthPeak = std::max(observedFourthPeak,
            controller.readTelemetry().stemPeakMicros[3]);
    }
    const auto telemetry = controller.readTelemetry();
    if (!(telemetry.playing && telemetry.stemCount == 4U
          && telemetry.stemActorIds[3] == 4U && observedFourthPeak > 100U)) {
        std::cerr << "Walsh telemetry: playing=" << telemetry.playing
                  << " stems=" << telemetry.stemCount
                  << " fourth-id=" << telemetry.stemActorIds[3]
                  << " fourth-peak=" << telemetry.stemPeakMicros[3]
                  << " observed-fourth-peak=" << observedFourthPeak
                  << " fourth-life=" << static_cast<int>(telemetry.stemLifeStates[3])
                  << " fourth-ready=" << static_cast<int>(telemetry.stemReadiness[3]) << '\n';
    }
    expect(telemetry.playing && telemetry.stemCount == 4U
               && telemetry.stemActorIds[3] == 4U && observedFourthPeak > 100U,
           "queued Walsh controls produce a stable fourth stem during PLAY");
    expect(controller.removeDeskModule(4U), "desk recollects Walsh without clearing the session");
    for (int block = 0; block < 16; ++block) controller.processBlock(left, right, 64U);
    expect(controller.cumulativeDesk().occupiedCount() == 0U
               && controller.readTelemetry().stemCount == 4U
               && controller.readTelemetry().stemActorIds[3] == 4U,
           "recollection removes the desk entry while retaining the fixed audio slot");
    expect(controller.addLibraryModule(4U, "walsh", 0.74, 0.62, 0.66),
           "the same optional module can be inserted again after recollection");
}

void testSafeLimitRejectsUnrepresentedActorCommands() {
#if RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT < 5
    PerformanceSessionController controller;
    auto document = PerformanceTerritoryFactory::makeWithWalsh(seedWithFourth("walsh"), true);
    expect(document.territory.addActor({5U, PerformanceFamily::Rupture, 0x44U,
                                        roleBit(PerformanceRole::Rasgo), 0.5, 0.0, 1.0}),
           "fifth declared actor enters the control document");
    expect(controller.prepare(48000.0, 64U, 2U) && controller.loadDocument(document),
           "controller loads a document larger than the realtime-safe realization");
    expect(!controller.setActorPresence(5U, 0.8)
               && !controller.setActorPosition(5U, 0.5, 0.5)
               && !controller.setModuleControl(5U, PerformanceModuleControl::Primary, 0.5)
               && !controller.addLibraryModule(5U, "walsh", 0.5, 0.5, 0.5)
               && controller.cumulativeDesk().occupiedCount() == 0U,
           "future fifth-stem commands are rejected without changing the desk");
#endif
}

void testModalResonatorIsTheFifthRealBody() {
#if RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT >= 5
    PerformanceSessionController controller;
    const auto document = PerformanceTerritoryFactory::makeWithModalReserve(184U, true);
    expect(controller.prepare(48000.0, 64U, 2U) && controller.loadDocument(document),
           "controller accepts the five-body ModalResonator territory");
    expect(controller.setActorPresence(5U, 0.60)
               && controller.setActorPosition(5U, 0.50, 0.50)
               && controller.setModuleControl(5U, PerformanceModuleControl::Primary, 0.66),
           "fifth ModalResonator accepts real body controls");
    float left[64]{};
    float right[64]{};
    for (int block = 0; block < 128; ++block) controller.processBlock(left, right, 64U);
    const auto telemetry = controller.readTelemetry();
    expect(telemetry.stemCount == 5U && telemetry.stemActorIds[4] == 5U
               && telemetry.stemReadiness[4]
                   != static_cast<std::uint8_t>(PerformanceStemReadiness::Prepared),
           "telemetry names the fifth ModalResonator as a realized stem, never a false reserve");
#endif
}

[[maybe_unused]] void testAccumulatedHardSyncControlPath() {
    PerformanceSessionController controller;
    const auto document = PerformanceTerritoryFactory::makeWithWalsh(seedWithFourth("hard-sync"), true);
    expect(controller.prepare(48000.0, 64U, 2U) && controller.loadDocument(document),
           "accumulated Hard Sync controller loads the four-body document");
#if !defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V3)
    expect(!controller.addLibraryModule(4U, "hard-sync", 0.26, 0.44, 0.74),
           "contracts-only build does not claim the unavailable Hard Sync adapter");
    return;
#endif
    expect(controller.setPlaying(true)
               && controller.addLibraryModule(4U, "hard-sync", 0.26, 0.44, 0.74)
               && controller.setRelationIntensity(1U, 4U, 0.69),
           "Hard Sync desk admission, position and orbit cross the controller queues");
    expect(controller.cumulativeDesk().slot(0U)->libraryEntry == 9U,
           "Hard Sync keeps its distinct library provenance");
    expect(!controller.addLibraryModule(4U, "walsh", 0.5, 0.5, 0.5)
               && !controller.addLibraryModule(4U, "deja-vu", 0.5, 0.5, 0.5),
           "controller rejects library bodies not prepared by the current seed");
    float left[64]{};
    float right[64]{};
    std::uint32_t observedFourthPeak = 0U;
    for (int block = 0; block < 240; ++block) {
        controller.processBlock(left, right, 64U);
        observedFourthPeak = std::max(observedFourthPeak,
            controller.readTelemetry().stemPeakMicros[3]);
    }
    const auto telemetry = controller.readTelemetry();
    expect(telemetry.playing && telemetry.stemCount == 4U
               && telemetry.stemActorIds[3] == 4U && observedFourthPeak > 100U,
           "queued Hard Sync controls produce a stable fourth stem during PLAY");
}

[[maybe_unused]] void testAccumulatedDigitalControlPath() {
    PerformanceSessionController controller;
    const auto document = PerformanceTerritoryFactory::makeWithOptionalModule(seedWithFourth("vosim"), true);
    expect(controller.prepare(48000.0, 64U, 2U) && controller.loadDocument(document),
           "accumulated Digital controller loads the four-body document");
    expect(controller.setPlaying(true)
               && controller.addLibraryModule(4U, "vosim", 0.74, 0.58, 0.72)
               && controller.setRelationIntensity(1U, 4U, 0.66),
           "Digital VOSIM desk admission, position and relation cross the queues");
    float left[64]{};
    float right[64]{};
    std::uint32_t observedFourthPeak = 0U;
    for (int block = 0; block < 240; ++block) {
        controller.processBlock(left, right, 64U);
        observedFourthPeak = std::max(observedFourthPeak,
            controller.readTelemetry().stemPeakMicros[3]);
    }
    const auto telemetry = controller.readTelemetry();
    expect(telemetry.playing && telemetry.stemCount == 4U
               && telemetry.stemActorIds[3] == 4U && observedFourthPeak > 100U,
           "queued Digital VOSIM controls produce a stable fourth stem during PLAY");
}

[[maybe_unused]] void testAccumulatedBreathAtariPunkControlPath() {
    PerformanceSessionController controller;
    const auto document = PerformanceTerritoryFactory::makeWithOptionalModule(seedWithFourth("atari-punk"), true);
    expect(controller.prepare(48000.0, 64U, 2U) && controller.loadDocument(document),
           "Breath controller loads a fourth Atari Punk reserve");
    expect(controller.setPlaying(true)
               && controller.addLibraryModule(4U, "atari-punk", 0.30, 0.68, 0.72)
               && controller.setModuleControl(4U, PerformanceModuleControl::Primary, 0.88)
               && controller.setRelationIntensity(2U, 4U, 0.61),
           "Atari Punk admission, local control and live relation cross the controller queues");
    float left[64]{};
    float right[64]{};
    std::uint32_t observedFourthPeak = 0U;
    for (int block = 0; block < 240; ++block) {
        controller.processBlock(left, right, 64U);
        observedFourthPeak = std::max(observedFourthPeak,
            controller.readTelemetry().stemPeakMicros[3]);
    }
    const auto telemetry = controller.readTelemetry();
    expect(telemetry.playing && telemetry.stemCount == 4U
               && telemetry.stemActorIds[3] == 4U && observedFourthPeak > 100U
               && controller.cumulativeDesk().occupiedCount() == 1U,
           "Atari Punk becomes a measured fourth stem without replacing the Breath opening");
}

[[maybe_unused]] void testAllPreparedEcologiesKeepTheLiveDrawingDuringSeedDialogue() {
    struct Fixture {
        const char* module;
        std::uint16_t relationSource;
    };
    // One fixture per star that the seed may light on body 4. The seed
    // chooses the star; the performer still chooses its position, local
    // control and relation.
    constexpr Fixture fixtures[] = {
        {"walsh", 1U},
        {"atari-punk", 2U},
        {"vosim", 1U},
        {"hard-sync", 1U},
        {"deja-vu", 1U},
        {"atropelo", 1U},
        {"galo", 1U},
    };
    for (const auto& fixture : fixtures) {
        PerformanceSessionController controller;
        const auto fixtureSeed = seedWithFourth(fixture.module);
        const auto document = PerformanceTerritoryFactory::makeWithOptionalModule(fixtureSeed, true);
        expect(fourthIs(fixtureSeed, fixture.module),
               "fixture's seed/module reservation is coherent before controller load");
        expect(controller.prepare(48000.0, 64U, 2U) && controller.loadDocument(document),
               "prepared ecology loads through the controller");
        expect(controller.document()->territory.seed == fixtureSeed
                   && fourthIs(controller.document()->territory.seed, fixture.module),
               "controller retains the seed/module reservation after load");
        const auto* libraryEntry = findPerformanceModule(fixture.module);
        expect(libraryEntry != nullptr
                   && libraryEntry->state == PerformanceLibraryState::ActiveCohort,
               "fixture module remains an active library entry");
        const auto drawingIndex = (fixtureSeed - 181U) % 5U;
        const double x = 0.17 + static_cast<double>(drawingIndex) * 0.16;
        const double y = 0.79 - static_cast<double>(drawingIndex) * 0.11;
        expect(controller.setPlaying(true), "prepared ecology queues PLAY");
        expect(controller.addLibraryModule(4U, fixture.module, x, y, 0.72),
               "seed-prepared fourth body is admitted by the library");
        expect(controller.setModuleControl(4U, PerformanceModuleControl::Primary, 0.84),
               "fourth body accepts its local control");
        expect(controller.setRelationIntensity(fixture.relationSource, 4U, 0.67),
               "fourth body accepts its live relation");
        expect(controller.setMacro(PerformanceMacro::Autonomy, 1.0)
                   && controller.setMacro(PerformanceMacro::Intervention, 0.0),
               "seed autonomy is released for the integration interval");
        float left[64]{};
        float right[64]{};
        std::uint32_t observedFourthPeak = 0U;
        for (int block = 0; block < 2400 && !controller.lastSpatialGameEvent().valid(); ++block) {
            controller.processBlock(left, right, 64U);
            observedFourthPeak = std::max(observedFourthPeak,
                controller.readTelemetry().stemPeakMicros[3]);
        }
        const auto telemetry = controller.readTelemetry();
        const auto position = controller.actorPosition(4U);
        expect(telemetry.playing && telemetry.stemCount == 4U
                   && telemetry.stemActorIds[3] == 4U
                   && observedFourthPeak > 100U,
               "every prepared ecology keeps an audible fourth stem through PLAY");
        expect(std::fabs(position.x - x) < 1.0e-12
                   && std::fabs(position.y - y) < 1.0e-12,
               "seed dialogue never redraws the performer's fourth-module position");
        expect(controller.lastSpatialGameEvent().valid(),
               "inactivity lets the seed answer the live field with a situated event");
    }
}

[[maybe_unused]] void testFourthBodyOffersPerformerChosenTopology() {
    PerformanceSessionController controller;
    const auto document = PerformanceTerritoryFactory::makeWithOptionalModule(seedWithFourth("vosim"), true);
    expect(document.scenes[0].constellation.relationCount == 6U,
           "fourth body declares every pair without forcing their activation");
    expect(document.scenes[0].constellation.relations[4].intensity == 0.0
               && document.scenes[0].constellation.relations[5].intensity == 0.0,
           "new fourth-body cables begin closed by performer choice");
    expect(controller.prepare(48000.0, 64U, 2U) && controller.loadDocument(document)
               && controller.addLibraryModule(4U, "vosim", 0.72, 0.64, 0.70)
               && controller.setRelationIntensity(2U, 4U, 0.71)
               && controller.setRelationIntensity(3U, 4U, 0.53),
           "performer can open both additional fourth-body connections live");
}

} // namespace

void testRaisingPresenceLightsBodiesSixToEight() {
#if !defined(RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT) || RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT >= 8
    // The author's report (2026-10-28): raising Galo, Atropelo and Deja Vu
    // did nothing. Each of bodies 6..8, unlit at the opening, must sound on
    // its own stem once its presence fader is raised (heard in SOLO, as the
    // performer checks a body).
    for (std::uint16_t actorId = 6U; actorId <= 8U; ++actorId) {
        PerformanceSessionController controller;
        const auto document = PerformanceTerritoryFactory::makeForReleasedStems(4065351394U, 8U, true);
        expect(controller.prepare(48000.0, 64U, 2U) && controller.loadDocument(document)
                   && controller.setPlaying(true),
               "the eight-body opening loads");
        float left[64]{};
        float right[64]{};
        for (int block = 0; block < 32; ++block) controller.processBlock(left, right, 64U);
        expect(controller.setActorPresence(actorId, 0.85) && controller.setStemSolo(actorId, true),
               "an unlit body accepts its presence fader");
        std::uint32_t peak = 0U;
        for (int block = 0; block < 7500; ++block) {
            controller.processBlock(left, right, 64U);
            peak = std::max(peak, controller.readTelemetry().stemPeakMicros[actorId - 1U]);
        }
        if (peak <= 1000U) std::cerr << "body " << actorId << " peak " << peak << '\n';
        expect(peak > 1000U, "raising presence lights the body's own stem");
    }
#endif
}

int main() {
    testRaisingPresenceLightsBodiesSixToEight();
    testControlFlow();
    testRejectedCommandsDoNotPublishControlState();
    testAccumulatedWalshControlPath();
    testSafeLimitRejectsUnrepresentedActorCommands();
    testModalResonatorIsTheFifthRealBody();
#if defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V3)
    testAccumulatedHardSyncControlPath();
    testAccumulatedDigitalControlPath();
#endif
#if defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V2)
    testAccumulatedBreathAtariPunkControlPath();
#endif
#if defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V2) \
    && defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V3) \
    && defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V4)
    testAllPreparedEcologiesKeepTheLiveDrawingDuringSeedDialogue();
    testFourthBodyOffersPerformerChosenTopology();
#endif
    std::cout << "rasgo_synth_performance_session_controller_tests: OK\n";
    return 0;
}
