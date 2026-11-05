#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <rasgo/performance/PerformanceSessionEngine.hpp>
#include <rasgo/performance/PerformanceTerritoryFactory.hpp>

namespace {
using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

PerformanceGestureEvent gesture(GestureType type, std::uint64_t sample, double intensity = 0.6) {
    PerformanceGestureEvent event;
    event.type = type;
    event.sampleTimestamp = sample;
    event.blockOffset = 0U;
    event.intensity = intensity;
    return event;
}

PerformanceTerritory ecologyTerritory() {
    PerformanceTerritory territory;
    territory.seed = 181U;
    for (std::uint16_t id = 11U; id <= 15U; ++id) {
        PerformanceActorPotential actor;
        actor.id = id;
        actor.family = id == 11U ? PerformanceFamily::Tone
            : id == 12U ? PerformanceFamily::Texture
            : id == 13U ? PerformanceFamily::Rupture
            : id == 14U ? PerformanceFamily::Relation : PerformanceFamily::Pattern;
        actor.capabilities = 0x01U;
        expect(territory.addActor(actor), "ecology actor enters territory");
    }
    return territory;
}

PerformanceScene ecologyScene(std::uint16_t id, double density) {
    PerformanceScene scene;
    scene.id = id;
    scene.session.territorySeed = 181U;
    scene.session.playing = true;
    scene.session.currentSceneId = id;
    scene.session.targetSceneId = id;
    scene.session.density = density;
    scene.session.tension = density * 0.8;
    expect(scene.constellation.addActor({11U, PerformanceRole::Chao, ActorLifeState::Active,
                                         0.75, 0.70, 0.55, 0.80}), "Chao enters ecology");
    expect(scene.constellation.addActor({12U, PerformanceRole::Corpo, ActorLifeState::Active,
                                         0.62, 0.58, 0.52, 0.74}), "Corpo enters ecology");
    expect(scene.constellation.addActor({13U, PerformanceRole::Atrito, ActorLifeState::Emerging,
                                         0.35, 0.42, 0.25, 0.68}), "Atrito enters ecology");
    expect(scene.constellation.addRelation({11U, 12U, RelationKind::Support, RelationDomain::Time,
                                            0.45, 12.0, 1.0, 0.7}), "time relation enters ecology");
    expect(scene.constellation.addRelation({12U, 13U, RelationKind::Friction, RelationDomain::Matter,
                                            0.50, 8.0, 0.8, 0.7}), "matter relation enters ecology");
    return scene;
}

void testLiveStemReconstruction() {
    const auto scene = ecologyScene(19U, 0.58);
    const auto morph = PerformanceSceneMorpher::morph(scene, scene, 1.0);
    PerformanceActorEcology summed;
    PerformanceActorEcology split;
    expect(summed.prepare(48000.0, 64U, 2U) && split.prepare(48000.0, 64U, 2U),
           "stem comparison ecologies prepare");
    summed.reset();
    split.reset();
    summed.setMorph(morph);
    split.setMorph(morph);
    summed.setMacroState(scene.session);
    split.setMacroState(scene.session);

    std::array<double, kPerformanceStemActorCapacity> energy{};
    double maximumError = 0.0;
    for (int sample = 0; sample < 4096; ++sample) {
        const double mixed = summed.render(48000.0, 0.23, 0.17);
        const auto stems = split.renderStems(48000.0, 0.23, 0.17);
        expect(stems.valid() && stems.actorCount == 3U,
               "live stem frame preserves the bounded three-actor ecology");
        expect(stems.findActor(11U) != nullptr && stems.findActor(12U) != nullptr
               && stems.findActor(13U) != nullptr,
               "actor identity owns each stem independently of its role");
        maximumError = std::max(maximumError, std::fabs(mixed - stems.mixedMono()));
        for (std::size_t index = 0U; index < stems.actorCount; ++index)
            energy[index] += stems.actors[index].mono * stems.actors[index].mono;
    }
    expect(maximumError < 1.0e-12,
           "actor stems plus modal return reconstruct the previous ecology output exactly");
    expect(std::all_of(energy.begin(), energy.begin() + 3,
                       [](double value) { return value > 1.0e-8; }),
           "each declared actor stem carries real signal energy");
}

void testLiveStemMixer() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(18U, 0.62);
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 64U, 2U) && engine.setScene(scene, territory),
           "stem mixer engine prepares with a declared ecology");
    std::array<float, 64> left{}, right{};
    for (int block = 0; block < 48; ++block) engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().stemPeakMicros[0] > 1000U
           && engine.readTelemetry().stemPeakMicros[1] > 1000U,
           "declared stems are audible before mixer intervention");
    expect(engine.readTelemetry().stemReadiness[0]
               == static_cast<std::uint8_t>(PerformanceStemReadiness::Audible),
           "telemetry identifies an active stem as audible");

    expect(engine.setStemGain(11U, 0.0) && !engine.setStemGain(99U, 0.5)
           && !engine.setStemGain(12U, 1.1),
           "stem gain accepts only bounded active actors");
    for (int block = 0; block < 128; ++block) engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().stemPeakMicros[0] < 100U,
           "zero stem gain fades one actor from the live mix");
    expect(engine.readTelemetry().stemReadiness[0]
               == static_cast<std::uint8_t>(PerformanceStemReadiness::Silent),
           "telemetry identifies a muted stem as silent");

    expect(engine.setStemMuted(12U, true) && !engine.setStemMuted(99U, true),
           "stem mute accepts only active actors");
    for (int block = 0; block < 128; ++block) engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().stemPeakMicros[1] < 100U,
           "stem mute fades the selected actor without stopping the engine");

    expect(engine.setStemSolo(13U, true) && engine.setOutputTrimDb(-9.0),
           "territory-boundary fixture prepares a live mixer and master override");
    expect(engine.setTerritoryScene(scene, territory)
           && engine.stemGain(11U) == 1.0 && !engine.stemMuted(12U)
           && engine.soloActorId() == kInvalidActorId && engine.outputTrimDb() == -9.0,
           "new territory resets the performance desk while preserving master trim");

    PerformanceSessionEngine solo;
    expect(solo.prepare(48000.0, 64U, 2U) && solo.setScene(scene, territory)
           && solo.setStemSolo(13U, true) && !solo.setStemSolo(99U, true),
           "stem solo accepts exactly one active actor");
    // A live constellation enters through event envelopes. Let that declared
    // body become present before verifying that SOLO isolates it; an empty
    // constellation is intentionally silent (there is no safety carrier).
    // Desde 3/11 a seed escolhe entre todas as estrelas: o corpo 3 pode ser
    // uma estrela de ataques raros (o Bluff, na seed 181). Por isso o solo é
    // conferido ao longo de até 8 s: o solado soa em algum momento, os
    // outros nunca, depois que o solo assentou. (Desde 4/11 a atividade da
    // parte da forma pode deixar passar só 35 % dos ataques.)
    for (int block = 0; block < 176; ++block) solo.processBlock(left.data(), right.data(), 64U);
    std::uint32_t othersPeak = 0U;
    std::uint32_t soloPeak = 0U;
    for (int block = 0; block < 6000 && soloPeak <= 1000U; ++block) {
        solo.processBlock(left.data(), right.data(), 64U);
        const auto live = solo.readTelemetry();
        othersPeak = std::max({othersPeak, live.stemPeakMicros[0], live.stemPeakMicros[1]});
        soloPeak = std::max(soloPeak, live.stemPeakMicros[2]);
    }
    std::cout << "solo: soloed stem peak " << soloPeak << " micros, others " << othersPeak << " micros\n";
    expect(othersPeak < 100U && soloPeak > 1000U && solo.soloActorId() == 13U,
           "solo isolates one stem through the same realtime mixer path");
    expect(solo.setStemSolo(13U, false) && solo.soloActorId() == kInvalidActorId,
           "solo release restores the full live stem set without a session reload");
}

void testEngineLifecycleAndGestures() {
    PerformanceSessionEngine engine;
    expect(!engine.prepare(48000.0, 0U, 2U), "zero block preparation is rejected");
    expect(engine.prepare(48000.0, 64U, 2U), "engine prepares native stereo voice");

    std::vector<float> left(64U), right(64U);
    engine.processBlock(left.data(), right.data(), 64U);
    expect(std::all_of(left.begin(), left.end(), [](float value) { return std::fabs(value) < 1.0e-7F; })
           && std::all_of(right.begin(), right.end(), [](float value) { return std::fabs(value) < 1.0e-7F; }),
           "an unpopulated engine is silent rather than producing a safety tone");

    auto firstLiveScene = ecologyScene(1U, 0.30);
    firstLiveScene.session.tension = 0.20;
    const auto territory = ecologyTerritory();
    expect(engine.setScene(firstLiveScene, territory), "first live scene enters engine");
    PerformanceSessionState sceneOne = firstLiveScene.session;

    for (int pass = 0; pass < 20; ++pass) engine.processBlock(left.data(), right.data(), 64U);
    const auto audible = std::any_of(left.begin(), left.end(), [](float value) { return std::fabs(value) > 1.0e-5F; });
    expect(audible, "prepared live scene produces ecology audio");
    expect(engine.readTelemetry().outputPeakMicros > 0U,
           "output telemetry reports a non-zero stereo peak for audible audio");
    const auto deadlineMisses = engine.readTelemetry().xrunCount;
    engine.reportDeadlineMiss();
    engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().xrunCount == deadlineMisses + 1U,
           "host-reported deadline miss reaches telemetry without pretending to be driver state");
    expect(engine.setPlaying(false) && !engine.playing(), "engine accepts a bounded stop transition");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(std::any_of(left.begin(), left.end(), [](float value) { return std::fabs(value) > 1.0e-7F; }),
           "STOP begins with a short de-click boundary rather than cutting a live wave");
    for (int pass = 0; pass < 64; ++pass) engine.processBlock(left.data(), right.data(), 64U);
    expect(std::all_of(left.begin(), left.end(), [](float value) { return std::fabs(value) < 1.0e-7F; })
           && std::all_of(right.begin(), right.end(), [](float value) { return std::fabs(value) < 1.0e-7F; }),
           "STOP reaches a hard silent output boundary without an audible tail");
    expect(engine.setPlaying(true) && engine.playing(), "engine restores transport without rebuilding its session");
    bool resumedAudible = false;
    for (int pass = 0; pass < 96; ++pass) {
        engine.processBlock(left.data(), right.data(), 64U);
        resumedAudible = resumedAudible || std::any_of(left.begin(), left.end(),
            [](float value) { return std::fabs(value) > 1.0e-5F; });
    }
    expect(resumedAudible, "PLAY fades back to an audible signal after a hard STOP");
    expect(engine.setOutputTrimDb(-9.0) && engine.outputTrimDb() == -9.0,
           "engine accepts bounded master trim in decibels");
    expect(!engine.setOutputTrimDb(-24.1) && !engine.setOutputTrimDb(12.1),
           "engine rejects master trim outside its safe range");
    expect(engine.setTempoBpm(148.0) && !engine.setTempoBpm(19.0) && !engine.setTempoBpm(301.0),
           "engine accepts bounded live tempo without rebuilding its session");
    expect(engine.pushGesture(gesture(GestureType::Hold, 0U)), "Hold queues for pulse suspension");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().holding,
           "Hold becomes observable after the audio side freezes the pulse clock");
    expect(engine.pushGesture(gesture(GestureType::Release, 0U)), "Release queues pulse continuation");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(!engine.readTelemetry().holding,
           "Release resumes the pulse clock through the same event path");

    auto wildMutate = gesture(GestureType::Mutate, 0U, 0.82);
    wildMutate.variationToken = 0x3d9a51c7U;
    expect(engine.pushGesture(wildMutate), "Mutate queues one bounded wild variation token");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().wildMicros > 1000U,
           "Mutate makes its temporary wild opening observable in telemetry");
    expect(engine.pushGesture(gesture(GestureType::Recall, 0U)), "Recall queues a return from wild variation");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().wildMicros < 200000U,
           "Recall actively recollects the wild opening instead of waiting for decay");

    expect(engine.pushGesture(gesture(GestureType::Hold, 1280U)), "Hold queues");
    expect(engine.pushGesture(gesture(GestureType::Release, 1280U)), "Release queues");
    expect(engine.pushGesture(gesture(GestureType::Mutate, 1280U, 0.8)), "Mutate queues");
    expect(engine.pushGesture(gesture(GestureType::Rasgar, 1280U, 0.7)), "Rasgar queues");
    expect(engine.pushGesture(gesture(GestureType::Interrupt, 1280U, 0.4)), "Interrupt queues");
    expect(engine.pushGesture(gesture(GestureType::Recall, 1280U)), "Recall queues");
    for (int pass = 0; pass < 8; ++pass) engine.processBlock(left.data(), right.data(), 64U);
    expect(std::all_of(left.begin(), left.end(), [](float value) { return std::isfinite(value); }),
           "five gestures keep output finite");

    auto sceneTwo = sceneOne;
    sceneTwo.currentSceneId = 2U;
    sceneTwo.density = 0.70;
    sceneTwo.tension = 0.65;
    expect(engine.setSessionSnapshot(sceneTwo), "second scene enters engine");
    engine.processBlock(left.data(), right.data(), 64U);
    auto sceneThree = sceneTwo;
    sceneThree.currentSceneId = 3U;
    sceneThree.risk = 0.85;
    sceneThree.space = 0.80;
    expect(engine.setSessionSnapshot(sceneThree), "third scene enters engine");
    engine.processBlock(left.data(), right.data(), 64U);

    expect(engine.pushGesture(gesture(GestureType::PanicSafe, 1920U, 1.0)), "PanicSafe queues");
    engine.processBlock(left.data(), right.data(), 64U);
    const auto telemetry = engine.readTelemetry();
    expect(telemetry.valid() && telemetry.sceneId == 3U && telemetry.panicSafe,
           "telemetry reports third scene and PanicSafe");
    expect(engine.pushGesture(gesture(GestureType::Release, 1984U)),
           "Release remains accepted after PanicSafe clears Hold");
    expect(engine.setPanicSafe(false), "PanicSafe can be released through an explicit control command");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(!engine.readTelemetry().panicSafe,
           "released PanicSafe is reflected by telemetry without rebuilding the session");

    float maximumJump = 0.0F;
    for (std::size_t index = 1; index < left.size(); ++index)
        maximumJump = std::max(maximumJump, std::fabs(left[index] - left[index - 1U]));
    expect(maximumJump < 0.25F, "smoothing prevents discontinuous output during gestures");

    engine.reset();
    expect(!engine.readTelemetry().panicSafe, "reset clears safety state for a new session");
}

void testMorceauTransportKeepsCodaAndStopDistinct() {
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 64U, 2U), "morceau transport engine prepares");
    PerformanceSessionState session;
    session.territorySeed = 181U;
    session.playing = true;
    expect(engine.setSessionSnapshot(session)
               && engine.morceauState() == PerformanceMorceauState::Opening,
           "PLAY enters the musical opening lifecycle");
    expect(engine.beginMorceauCoda()
               && engine.morceauState() == PerformanceMorceauState::Coda,
           "intentional coda is distinct from stopping transport");
    std::array<float, 64> left{}, right{};
    engine.processBlock(left.data(), right.data(), 64U);
    expect(std::all_of(left.begin(), left.end(), [](float sample) { return std::isfinite(sample); }),
           "coda envelope keeps the final stereo boundary finite");
    expect(engine.readTelemetry().codaGesture < 4U,
           "the bounded coda verb reaches telemetry without sharing live ecology state");
    expect(engine.setPlaying(false)
               && engine.morceauState() == PerformanceMorceauState::Interrupted,
           "STOP remains an explicit emergency interruption");
}

// 5/11, pedido do autor: quando a tomada acaba, só a gravação termina — o som
// continua; uma nova peça abre sem STOP.
void testMorceauContinuesAfterCodaWithoutStop() {
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 512U, 2U), "continuation engine prepares");
    PerformanceSessionState session;
    session.territorySeed = 181U;
    session.playing = true;
    expect(engine.setSessionSnapshot(session), "continuation session plays");
    expect(!engine.continueAfterMorceau(), "a piece still playing is not restarted");
    expect(engine.beginMorceauCoda(), "the take asks for its coda");
    std::array<float, 512> left{}, right{};
    for (int block = 0; block < 2000 && engine.morceauState() != PerformanceMorceauState::Finalized; ++block)
        engine.processBlock(left.data(), right.data(), 512U);
    expect(engine.morceauState() == PerformanceMorceauState::Finalized, "the coda and tail reach the end of the piece");
    expect(engine.continueAfterMorceau()
               && engine.morceauState() == PerformanceMorceauState::Opening,
           "after the coda a new piece opens without STOP");
    for (int block = 0; block < 400; ++block) engine.processBlock(left.data(), right.data(), 512U);
    expect(engine.morceauState() == PerformanceMorceauState::Developing,
           "the new piece leaves its opening and keeps playing");
}

void testSeedNarrativeMakesCodaEligibleAfterPerformerPause() {
    const auto territory = ecologyTerritory();
    auto scene = ecologyScene(41U, 0.62);
    scene.session.tempoBpm = 300.0;
    scene.session.memory = 0.78;
    scene.session.risk = 0.72;
    PerformanceSessionEngine engine;
    expect(engine.prepare(8000.0, 256U, 2U) && engine.setScene(scene, territory),
           "seed coda fixture prepares a live ecology");
    std::array<float, 256> left{}, right{};
    for (int block = 0; block < 1400 && engine.readTelemetry().codaReadinessMicros < 1000000U; ++block)
        engine.processBlock(left.data(), right.data(), 256U);
    expect(engine.readTelemetry().codaReadinessMicros == 1000000U,
           "seed, relations, narrative turns and sustained pause make an epilogue eligible");
    expect(engine.setActorPosition(11U, 0.23, 0.71),
           "a fresh performer drawing is accepted before the automatic coda");
    engine.processBlock(left.data(), right.data(), 256U);
    expect(engine.readTelemetry().codaReadinessMicros < 1000000U,
           "fresh performer activity cancels automatic coda eligibility without redrawing the field");
}

void testEcologyAndSceneMorph() {
    const auto territory = ecologyTerritory();
    auto source = ecologyScene(21U, 0.35);
    auto target = ecologyScene(22U, 0.72);
    target.constellation.actors[1].role = PerformanceRole::Contagio;
    target.constellation.actors[2].role = PerformanceRole::Rasgo;
    target.constellation.actors[2].presence = 0.68;
    expect(source.valid(territory) && target.valid(territory), "three-actor scenes fit the territory");

    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 64U, 2U), "ecology engine prepares");
    expect(engine.setScene(source, territory), "source constellation enters engine");
    expect(engine.activeActorCount() == 3U, "engine exposes three active native actors");
    std::vector<float> left(64U), right(64U);
    for (int pass = 0; pass < 24; ++pass) engine.processBlock(left.data(), right.data(), 64U);
    expect(std::any_of(left.begin(), left.end(), [](float sample) { return std::fabs(sample) > 1.0e-5F; }),
           "three-actor ecology is audible");
    // Desde 3/11 a seed escolhe entre todas as estrelas: uma delas pode ser de
    // ataques raros (o Bluff no corpo 3 da seed 181). O pico de cada stem é
    // conferido ao longo de até 8 s, não num instante só (desde 4/11 a
    // atividade da parte pode deixar passar só 35 % dos ataques).
    auto stemTelemetry = engine.readTelemetry();
    std::array<std::uint32_t, 3> stemPeaks{};
    for (int pass = 0; pass < 6000; ++pass) {
        for (std::size_t stem = 0U; stem < stemPeaks.size(); ++stem)
            stemPeaks[stem] = std::max(stemPeaks[stem], stemTelemetry.stemPeakMicros[stem]);
        if (stemPeaks[0] > 0U && stemPeaks[1] > 0U && stemPeaks[2] > 0U) break;
        engine.processBlock(left.data(), right.data(), 64U);
        stemTelemetry = engine.readTelemetry();
    }
    expect(stemTelemetry.stemCount == 3U && stemTelemetry.stemActorIds[0] == 11U
           && stemTelemetry.stemActorIds[1] == 12U && stemTelemetry.stemActorIds[2] == 13U
           && stemPeaks[0] > 0U && stemPeaks[1] > 0U && stemPeaks[2] > 0U,
           "engine publishes realtime-safe peak telemetry for each actor stem");

    expect(engine.beginSceneMorph(source, target, territory, 0.04), "scene morph starts inside one territory");
    expect(engine.setOutputTrimDb(-11.0), "master trim can be set during a scene morph");
    for (int pass = 0; pass < 36; ++pass) engine.processBlock(left.data(), right.data(), 64U);
    const auto telemetry = engine.readTelemetry();
    expect(telemetry.sceneId == target.id && telemetry.activeActorCount == 3U,
           "scene morph reaches target constellation without losing actors");
    expect(engine.outputTrimDb() == -11.0,
           "master trim remains global instead of returning to a scene value after morph");
    expect(std::all_of(left.begin(), left.end(), [](float sample) { return std::isfinite(sample); }),
           "ecology morph remains finite");

    auto foreign = target;
    foreign.session.territorySeed = 999U;
    expect(!engine.beginSceneMorph(source, foreign, territory, 0.04),
           "engine refuses to morph across territories");

    auto fiveActors = target;
    expect(fiveActors.constellation.addActor({14U, PerformanceRole::Contagio, ActorLifeState::Active,
                                              0.50, 0.56, 0.42, 0.70}), "Contagio enters five-actor ecology");
    expect(fiveActors.constellation.addActor({15U, PerformanceRole::Vestigio, ActorLifeState::Yielding,
                                              0.38, 0.48, 0.30, 0.64}), "Vestigio enters five-actor ecology");
    expect(fiveActors.constellation.addRelation({14U, 15U, RelationKind::Shadow, RelationDomain::Memory,
                                                 0.35, 28.0, 1.5, 0.60}), "memory relation enters five-actor ecology");
    expect(fiveActors.valid(territory), "five-actor constellation fits territory");
    expect(engine.setScene(fiveActors, territory), "five-actor constellation enters engine");
    engine.processBlock(left.data(), right.data(), 64U);
    const auto realizedCount = std::min<std::size_t>(fiveActors.constellation.actorCount,
                                                     kPerformanceSafeBodyLimit);
    expect(engine.activeActorCount() == realizedCount
               && engine.readTelemetry().activeActorCount == realizedCount,
           "engine realizes every declared body up to the current safety limit");
    expect(engine.setActorPresence(11U, 0.18) && engine.actorPresence(11U) == 0.18,
           "actor presence remains a bounded realtime ecology control");
    expect(!engine.setActorPresence(99U, 0.5) && !engine.setActorPresence(11U, 1.1),
           "unknown or unbounded actor presence is rejected");
    expect(engine.setRelationIntensity(14U, 15U, 0.72)
           && engine.relationIntensity(14U, 15U) == 0.72,
           "relation intensity remains a bounded realtime ecology control");
    expect(!engine.setRelationIntensity(14U, 99U, 0.5)
           && !engine.setRelationIntensity(14U, 15U, -0.1),
           "unknown or unbounded relation control is rejected");
    expect(engine.recallScene(fiveActors, territory), "scene recall keeps the performance ecology live");
    expect(engine.actorPresence(11U) == 0.18
           && engine.relationIntensity(14U, 15U) == 0.72,
           "scene recall preserves live presence and relation choices until the performer changes them");
}

double macroDifference(PerformanceMacro macro) {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(31U, 0.52);
    PerformanceSessionEngine low;
    PerformanceSessionEngine high;
    expect(low.prepare(48000.0, 64U, 2U) && high.prepare(48000.0, 64U, 2U),
           "macro comparison engines prepare");
    expect(low.setScene(scene, territory) && high.setScene(scene, territory),
           "macro comparison scenes load");
    expect(low.setMacro(macro, 0.10) && high.setMacro(macro, 0.90),
           "macro comparison values enter both engines");
    std::array<float, 64> lowLeft{}, lowRight{}, highLeft{}, highRight{};
    double difference = 0.0;
    // 1 s (era 0,13 s): com a seed escolhendo entre todas as estrelas (3/11),
    // a AUTONOMIA da seed 181 só se ouve depois do primeiro décimo de segundo.
    for (int block = 0; block < 750; ++block) {
        low.processBlock(lowLeft.data(), lowRight.data(), 64U);
        high.processBlock(highLeft.data(), highRight.data(), 64U);
        for (std::size_t sample = 0; sample < lowLeft.size(); ++sample) {
            const auto leftDelta = static_cast<double>(lowLeft[sample] - highLeft[sample]);
            const auto rightDelta = static_cast<double>(lowRight[sample] - highRight[sample]);
            difference += leftDelta * leftDelta + rightDelta * rightDelta;
        }
    }
    return difference;
}

void testEveryMacroHasAudibleCausality() {
    for (std::uint8_t value = 0U; value < static_cast<std::uint8_t>(PerformanceMacro::Count); ++value) {
        const auto macro = static_cast<PerformanceMacro>(value);
        const auto difference = macroDifference(macro);
        expect(difference > 1.0e-4,
               "every visible macro creates a non-trivial rendered stereo difference");
    }
}

double gestureDifference(GestureType type) {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(39U, 0.66);
    PerformanceSessionEngine dry;
    PerformanceSessionEngine acted;
    expect(dry.prepare(48000.0, 64U, 2U) && acted.prepare(48000.0, 64U, 2U)
               && dry.setScene(scene, territory) && acted.setScene(scene, territory),
           "gesture comparison engines prepare");
    std::array<float, 64> dryLeft{}, dryRight{}, actedLeft{}, actedRight{};
    for (int block = 0; block < 48; ++block) {
        dry.processBlock(dryLeft.data(), dryRight.data(), 64U);
        acted.processBlock(actedLeft.data(), actedRight.data(), 64U);
    }
    auto event = gesture(type, 0U, 0.86);
    event.targetActorId = 12U;
    event.variationToken = 0x91e10da5U;
    expect(acted.pushGesture(event), "audible gesture enters the realtime queue");
    double difference = 0.0;
    for (int block = 0; block < 96; ++block) {
        dry.processBlock(dryLeft.data(), dryRight.data(), 64U);
        acted.processBlock(actedLeft.data(), actedRight.data(), 64U);
        for (std::size_t sample = 0; sample < dryLeft.size(); ++sample) {
            const auto left = static_cast<double>(dryLeft[sample] - actedLeft[sample]);
            const auto right = static_cast<double>(dryRight[sample] - actedRight[sample]);
            difference += left * left + right * right;
        }
    }
    expect(acted.readTelemetry().playing,
           "a creative gesture never stops the transport or becomes an accidental silence");
    return difference;
}

void testCreativeGesturesHaveAudibleCausalityAndKeepPlaying() {
    for (const auto type : {GestureType::Rasgar, GestureType::Recall,
                            GestureType::Stutter, GestureType::TempoShift}) {
        expect(gestureDifference(type) > 1.0e-4,
               "every visible creative gesture changes the rendered audio");
    }
}

void testNamedScenesProduceDistinctMaterial() {
    const auto document = PerformanceTerritoryFactory::makeExpandedEnsemble(184U, true);
    expect(document.sceneCount >= 4U, "seed territory includes the four named scenes");
    const auto& spiral = document.scenes[1U];
    const auto& rupture = document.scenes[2U];
    const auto& balance = document.scenes[3U];
    expect(spiral.session.populationPhenomenon == PerformancePopulationPhenomenon::CollectiveRotate
               && rupture.session.populationPhenomenon == PerformancePopulationPhenomenon::Fragment
               && balance.session.populationPhenomenon == PerformancePopulationPhenomenon::Align,
           "Espiral, Rasgo and Balanço each install a distinct constellation phenomenon");
    PerformanceSessionEngine spiralEngine;
    PerformanceSessionEngine ruptureEngine;
    expect(spiralEngine.prepare(48000.0, 64U, 2U) && ruptureEngine.prepare(48000.0, 64U, 2U)
               && spiralEngine.setScene(spiral, document.territory)
               && ruptureEngine.setScene(rupture, document.territory),
           "named scene comparison engines prepare");
    std::array<float, 64> spiralLeft{}, spiralRight{}, ruptureLeft{}, ruptureRight{};
    double difference = 0.0;
    for (int block = 0; block < 128; ++block) {
        spiralEngine.processBlock(spiralLeft.data(), spiralRight.data(), 64U);
        ruptureEngine.processBlock(ruptureLeft.data(), ruptureRight.data(), 64U);
        for (std::size_t sample = 0; sample < spiralLeft.size(); ++sample) {
            const auto left = static_cast<double>(spiralLeft[sample] - ruptureLeft[sample]);
            const auto right = static_cast<double>(spiralRight[sample] - ruptureRight[sample]);
            difference += left * left + right * right;
        }
    }
    expect(difference > 1.0e-4,
           "named scenes generate distinct rendered material, not only different labels");
}

void testNamedScenesNeverInterruptAnActiveMorceau() {
    // Territory changes can legitimately start stopped, while the performer
    // may already be playing when choosing ESPIRAL/RASGO/BALANÇO.  This used
    // to close the morceau envelope and leave a visually live but silent app.
    const auto document = PerformanceTerritoryFactory::makeExpandedEnsemble(185U, false);
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 64U, 2U)
               && engine.setTerritoryScene(document.scenes[0], document.territory)
               && engine.setPlaying(true),
           "an active morceau prepares before recalling stopped stored scenes");
    std::array<float, 64> left{}, right{};
    for (int block = 0; block < 48; ++block) engine.processBlock(left.data(), right.data(), 64U);
    for (std::size_t index = 1U; index < 4U; ++index) {
        expect(engine.beginSceneMorph(document.scenes[index - 1U], document.scenes[index],
                                      document.territory, 0.10),
               "every named scene begins a compositional morph during PLAY");
        bool audible = false;
        for (int block = 0; block < 120; ++block) {
            engine.processBlock(left.data(), right.data(), 64U);
            audible = audible || std::any_of(left.begin(), left.end(), [](float sample) {
                return std::fabs(sample) > 1.0e-6F;
            });
        }
        expect(engine.playing() && engine.morceauState() != PerformanceMorceauState::Interrupted
                   && engine.morceauState() != PerformanceMorceauState::Finalized && audible,
               "Espiral, Rasgo and Balanco preserve a sounding active morceau");
    }
}

#if RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT >= 8
double expandedModuleControlDifference(std::uint16_t actorId,
                                       PerformanceModuleControl control) {
    const auto document = PerformanceTerritoryFactory::makeExpandedEnsemble(196U, true);
    PerformanceSessionEngine low;
    PerformanceSessionEngine high;
    expect(low.prepare(48000.0, 64U, 2U) && high.prepare(48000.0, 64U, 2U)
               && low.setScene(document.scenes[0], document.territory)
               && high.setScene(document.scenes[0], document.territory),
           "expanded-module comparison engines prepare");
    expect(low.setModuleControl(actorId, control, 0.05)
               && high.setModuleControl(actorId, control, 0.95),
           "every expanded module accepts its declared local control");
    // Since 2026-10-28 bodies 5..8 play their real event-based stars on their
    // own stems; a performer tunes such a control with the body in SOLO.
    expect(low.setStemSolo(actorId, true) && high.setStemSolo(actorId, true),
           "an expanded module can be heard alone");
    std::array<float, 64> lowLeft{}, lowRight{}, highLeft{}, highRight{};
    double difference = 0.0;
    // 1,5 s (era 0,19 s): desde 4/11 a atividade da parte pode filtrar os
    // primeiros ataques de uma estrela.
    for (int block = 0; block < 1125; ++block) {
        low.processBlock(lowLeft.data(), lowRight.data(), 64U);
        high.processBlock(highLeft.data(), highRight.data(), 64U);
        for (std::size_t sample = 0; sample < lowLeft.size(); ++sample) {
            const auto left = static_cast<double>(lowLeft[sample] - highLeft[sample]);
            const auto right = static_cast<double>(lowRight[sample] - highRight[sample]);
            difference += left * left + right * right;
        }
    }
    return difference;
}

void testExpandedModulesHaveWorkingLocalControls() {
    for (const auto actorId : {5U, 6U, 7U, 8U}) {
        for (const auto control : {PerformanceModuleControl::Primary,
                                   PerformanceModuleControl::Secondary,
                                   PerformanceModuleControl::Motion}) {
            const auto difference = expandedModuleControlDifference(actorId, control);
            if (difference <= 1.0e-4)
                std::cerr << "expanded control weak: actor=" << actorId
                          << " control=" << static_cast<int>(control)
                          << " difference=" << difference << '\n';
            expect(difference > 1.0e-4,
                   "every visible local control of every expanded module changes audio");
        }
    }
}
#endif

void testTerritoryIdentityHasAudibleCausality() {
    auto lowIdentity = PerformanceTerritoryFactory::make(181U, true);
    auto highIdentity = lowIdentity;
    lowIdentity.territory.identity = {0.10, 0.10, 0.10, 0.10, 0.90, 0.90};
    highIdentity.territory.identity = {0.90, 0.90, 0.90, 0.90, 0.90, 0.90};
    PerformanceSessionEngine low;
    PerformanceSessionEngine high;
    expect(low.prepare(48000.0, 64U, 2U) && high.prepare(48000.0, 64U, 2U)
           && low.setScene(lowIdentity.scenes[0], lowIdentity.territory)
           && high.setScene(highIdentity.scenes[0], highIdentity.territory),
           "contrasting valid territory identities prepare the same constellation");
    std::array<float, 64> lowLeft{}, lowRight{}, highLeft{}, highRight{};
    double difference = 0.0;
    for (int block = 0; block < 128; ++block) {
        low.processBlock(lowLeft.data(), lowRight.data(), 64U);
        high.processBlock(highLeft.data(), highRight.data(), 64U);
        for (std::size_t sample = 0; sample < lowLeft.size(); ++sample) {
            const auto leftDelta = static_cast<double>(lowLeft[sample] - highLeft[sample]);
            const auto rightDelta = static_cast<double>(lowRight[sample] - highRight[sample]);
            difference += leftDelta * leftDelta + rightDelta * rightDelta;
        }
    }
    expect(difference > 1.0e-4,
           "territory tonal, temporal, matter and relation DNA change the rendered audio");
}

void testRasgarLeavesRecoverableScar() {
    PerformanceSessionEngine engine;
    PerformanceSessionState session;
    session.territorySeed = 181U;
    session.playing = true;
    session.currentSceneId = 9U;
    expect(engine.prepare(48000.0, 64U, 2U) && engine.setSessionSnapshot(session),
           "scar test engine prepares a playing session");
    std::array<float, 64> left{}, right{};
    expect(engine.pushGesture(gesture(GestureType::Rasgar, 0U, 0.90)),
           "Rasgar queues a scar event");
    engine.processBlock(left.data(), right.data(), 64U);
    const auto firstScar = engine.readTelemetry().scarMicros;
    expect(firstScar > 500000U, "Rasgar leaves a substantial observable scar after its transient");
    for (int block = 0; block < 128; ++block) engine.processBlock(left.data(), right.data(), 64U);
    const auto lingeringScar = engine.readTelemetry().scarMicros;
    expect(lingeringScar > 250000U && lingeringScar < firstScar,
           "scar decays slowly enough to remain a musical consequence");
    expect(engine.pushGesture(gesture(GestureType::Recall, 0U)), "Recall queues recovery from scar");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().scarMicros < lingeringScar / 2U,
           "Recall actively reduces the lingering scar rather than merely changing a label");
    expect(engine.readTelemetry().repairMicros > 750000U,
           "Recall publishes the repair trajectory that is already acting on audio");
    expect(engine.pushGesture(gesture(GestureType::PanicSafe, 0U, 1.0)), "PanicSafe queues scar reset");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().scarMicros == 0U,
           "PanicSafe clears scar as part of its safe recovery boundary");
    expect(engine.readTelemetry().repairMicros == 0U,
           "PanicSafe clears repair as part of its safe recovery boundary");
}

void testRecallHasAudibleRepairWhenClean() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(37U, 0.64);
    PerformanceSessionEngine dry;
    PerformanceSessionEngine recalled;
    expect(dry.prepare(48000.0, 64U, 2U) && recalled.prepare(48000.0, 64U, 2U)
           && dry.setScene(scene, territory) && recalled.setScene(scene, territory),
           "clean Recall comparison engines prepare");
    std::array<float, 64> dryLeft{}, dryRight{}, recalledLeft{}, recalledRight{};
    // Let the declared rhythmic bodies enter before comparing a clean repair.
    // The assertion below then measures RECALL, rather than the initial
    // silence between two still-unborn event cycles.
    for (int block = 0; block < 48; ++block) {
        dry.processBlock(dryLeft.data(), dryRight.data(), 64U);
        recalled.processBlock(recalledLeft.data(), recalledRight.data(), 64U);
    }
    expect(recalled.pushGesture(gesture(GestureType::Recall, 0U)),
           "Recall queues even when no scar is present");
#if !defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V1)
    // This contracts-only binary deliberately has no native bodies. It can
    // prove that a clean Recall enters the realtime repair trajectory, but it
    // cannot honestly claim a stem timbre difference that belongs to the
    // optional cohort package.
    recalled.processBlock(recalledLeft.data(), recalledRight.data(), 64U);
    expect(recalled.readTelemetry().repairMicros > 750000U,
           "clean Recall enters the audible-cohort repair trajectory");
    return;
#endif
    double difference = 0.0;
    for (int block = 0; block < 96; ++block) {
        dry.processBlock(dryLeft.data(), dryRight.data(), 64U);
        recalled.processBlock(recalledLeft.data(), recalledRight.data(), 64U);
        for (std::size_t sample = 0; sample < dryLeft.size(); ++sample) {
            const auto left = static_cast<double>(dryLeft[sample] - recalledLeft[sample]);
            const auto right = static_cast<double>(dryRight[sample] - recalledRight[sample]);
            difference += left * left + right * right;
        }
    }
    expect(difference > 1.0e-3,
           "Recall creates an audible repair contour even without a prior scar");
}

void testHoldHasAudibleConcentrationAndRelease() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(38U, 0.64);
    PerformanceSessionEngine free;
    PerformanceSessionEngine held;
    expect(free.prepare(48000.0, 64U, 2U) && held.prepare(48000.0, 64U, 2U)
           && free.setScene(scene, territory) && held.setScene(scene, territory),
           "Hold comparison engines prepare");
    std::array<float, 64> freeLeft{}, freeRight{}, heldLeft{}, heldRight{};
    for (int block = 0; block < 24; ++block) {
        free.processBlock(freeLeft.data(), freeRight.data(), 64U);
        held.processBlock(heldLeft.data(), heldRight.data(), 64U);
    }
    expect(held.pushGesture(gesture(GestureType::Hold, 0U)), "Hold queues its concentration state");
    double heldDifference = 0.0;
    for (int block = 0; block < 96; ++block) {
        free.processBlock(freeLeft.data(), freeRight.data(), 64U);
        held.processBlock(heldLeft.data(), heldRight.data(), 64U);
        for (std::size_t sample = 0; sample < freeLeft.size(); ++sample) {
            const auto left = static_cast<double>(freeLeft[sample] - heldLeft[sample]);
            const auto right = static_cast<double>(freeRight[sample] - heldRight[sample]);
            heldDifference += left * left + right * right;
        }
    }
    expect(held.readTelemetry().holding && heldDifference > 1.0e-3,
           "Hold freezes pulse and creates a non-trivial concentrated sound");
    expect(held.pushGesture(gesture(GestureType::Release, 0U)), "Release queues after Hold");
    held.processBlock(heldLeft.data(), heldRight.data(), 64U);
    expect(!held.readTelemetry().holding, "Release returns the engine to its free pulse state");
}

void testQuantizedGesturesRespectSafetyBoundary() {
    PerformanceSessionEngine engine;
    PerformanceSessionState session;
    session.territorySeed = 181U;
    session.playing = true;
    session.tempoBpm = 120.0;
    expect(engine.prepare(48000.0, 64U, 2U) && engine.setSessionSnapshot(session)
           && engine.setQuantizedGestures(true),
           "quantize test engine prepares with explicit grid mode");
    std::array<float, 64> left{}, right{};
    expect(engine.pushGesture(gesture(GestureType::Rasgar, 0U, 0.8)),
           "Rasgar queues onto the next beat");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().quantizedGestures && engine.readTelemetry().eventQueueDepth > 0U
           && engine.readTelemetry().scarMicros == 0U,
           "quantized Rasgar waits instead of changing sound mid-beat");
    for (int block = 0; block < 390; ++block) engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().scarMicros > 100000U,
           "quantized Rasgar enters at the next beat and leaves its consequence");
    expect(engine.pushGesture(gesture(GestureType::PanicSafe, 0U, 1.0)),
           "PanicSafe queues while grid mode is enabled");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().panicSafe && engine.readTelemetry().scarMicros == 0U,
           "PanicSafe remains immediate and never waits for the musical grid");
}

void testGestureGridDivisions() {
    PerformanceSessionEngine engine;
    PerformanceSessionState session;
    session.territorySeed = 182U;
    session.playing = true;
    session.tempoBpm = 120.0;
    expect(engine.prepare(48000.0, 64U, 2U) && engine.setSessionSnapshot(session)
           && engine.setGestureGrid(true, GestureGridDivision::Quarter),
           "quarter-grid mode accepts a valid bounded division");
    std::array<float, 64> left{}, right{};
    expect(engine.pushGesture(gesture(GestureType::Rasgar, 0U, 0.8)),
           "quarter-grid Rasgar queues");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().gestureGridDivision == static_cast<std::uint8_t>(GestureGridDivision::Quarter)
           && engine.readTelemetry().scarMicros == 0U,
           "telemetry exposes the selected division while the event awaits its grid point");
    for (int block = 0; block < 92; ++block) engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().scarMicros == 0U,
           "quarter-grid gesture remains pending before the 1/4-beat boundary");
    engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.readTelemetry().scarMicros > 100000U,
           "quarter-grid Rasgar enters on its earlier musical boundary");
    expect(!engine.setGestureGrid(true, static_cast<GestureGridDivision>(3U)),
           "invalid grid divisions are rejected by the engine");
}

void testSpaceWidthAndSilenceGesture() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(31U, 0.52);
    PerformanceSessionEngine narrow;
    PerformanceSessionEngine wide;
    expect(narrow.prepare(48000.0, 64U, 2U) && wide.prepare(48000.0, 64U, 2U),
           "space comparison engines prepare");
    expect(narrow.setScene(scene, territory) && wide.setScene(scene, territory),
           "space comparison scenes load");
    expect(narrow.setMacro(PerformanceMacro::Space, 0.0)
           && wide.setMacro(PerformanceMacro::Space, 1.0),
           "space comparison values enter both engines");
    std::array<float, 64> narrowLeft{}, narrowRight{}, wideLeft{}, wideRight{};
    double narrowWidth = 0.0;
    double wideWidth = 0.0;
    for (int block = 0; block < 96; ++block) {
        narrow.processBlock(narrowLeft.data(), narrowRight.data(), 64U);
        wide.processBlock(wideLeft.data(), wideRight.data(), 64U);
        for (std::size_t sample = 0; sample < narrowLeft.size(); ++sample) {
            narrowWidth += std::fabs(static_cast<double>(narrowLeft[sample] - narrowRight[sample]));
            wideWidth += std::fabs(static_cast<double>(wideLeft[sample] - wideRight[sample]));
        }
    }
    expect(narrowWidth < 1.0e-6 && wideWidth > narrowWidth + 1.0e-3,
           "space widens stereo from a centered baseline");

    auto orbitScene = scene;
    expect(orbitScene.constellation.addRelation({11U, 13U, RelationKind::Orbit, RelationDomain::Space,
                                                 0.90, 14.0, 1.6, 0.9}),
           "space orbit relation enters the declared ecology");
    PerformanceSessionEngine orbit;
    expect(orbit.prepare(48000.0, 64U, 2U) && orbit.setScene(orbitScene, territory)
           && orbit.setMacro(PerformanceMacro::Space, 0.0),
           "orbit relation prepares at a centered macro baseline");
    std::array<float, 64> orbitLeft{}, orbitRight{};
    double orbitWidth = 0.0;
    for (int block = 0; block < 96; ++block) {
        orbit.processBlock(orbitLeft.data(), orbitRight.data(), 64U);
        for (std::size_t sample = 0; sample < orbitLeft.size(); ++sample)
            orbitWidth += std::fabs(static_cast<double>(orbitLeft[sample] - orbitRight[sample]));
    }
    expect(orbitWidth > narrowWidth + 1.0e-3,
           "spatial orbit relation widens stereo even when the macro begins centered");

    PerformanceSessionEngine silent;
    expect(silent.prepare(48000.0, 64U, 2U) && silent.setScene(scene, territory),
           "silence gesture engine prepares");
    std::array<float, 64> left{}, right{};
    for (int block = 0; block < 24; ++block) silent.processBlock(left.data(), right.data(), 64U);
    expect(silent.pushGesture(gesture(GestureType::Silence, 0U, 1.0)),
           "silence gesture queues");
    for (int block = 0; block < 96; ++block) silent.processBlock(left.data(), right.data(), 64U);
    expect(silent.readTelemetry().outputPeakMicros < 1000U,
           "silence gesture reaches a clearly silent output");
}

double relationDifference(std::uint16_t source, std::uint16_t destination) {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(41U, 0.62);
    PerformanceSessionEngine low;
    PerformanceSessionEngine high;
    expect(low.prepare(48000.0, 64U, 2U) && high.prepare(48000.0, 64U, 2U)
           && low.setScene(scene, territory) && high.setScene(scene, territory)
           && low.setRelationIntensity(source, destination, 0.0)
           && high.setRelationIntensity(source, destination, 1.0),
           "declared relation accepts contrasting live intensities");
    std::array<float, 64> lowLeft{}, lowRight{}, highLeft{}, highRight{};
    double difference = 0.0;
    for (int block = 0; block < 160; ++block) {
        low.processBlock(lowLeft.data(), lowRight.data(), 64U);
        high.processBlock(highLeft.data(), highRight.data(), 64U);
        for (std::size_t sample = 0; sample < lowLeft.size(); ++sample) {
            const auto left = static_cast<double>(lowLeft[sample] - highLeft[sample]);
            const auto right = static_cast<double>(lowRight[sample] - highRight[sample]);
            difference += left * left + right * right;
        }
    }
    return difference;
}

void testEveryDeclaredRelationHasAudibleCausality() {
    expect(relationDifference(11U, 12U) > 1.0e-3,
           "matter/friction relation creates a non-trivial timbral difference");
    expect(relationDifference(12U, 13U) > 1.0e-3,
           "time/contagion relation creates a non-trivial temporal difference");
}

void testRelationalPhenomenaComposeDifferentAudioWithoutRedrawingAnchors() {
    const auto scene = ecologyScene(47U, 0.64);
    const auto morph = PerformanceSceneMorpher::morph(scene, scene, 1.0);
    PerformanceActorEcology attraction;
    PerformanceActorEcology repulsion;
    expect(attraction.prepare(48000.0, 64U, 2U)
               && repulsion.prepare(48000.0, 64U, 2U),
           "relational phenomenon comparison ecologies prepare");
    attraction.setMorph(morph);
    repulsion.setMorph(morph);
    attraction.setMacroState(scene.session);
    repulsion.setMacroState(scene.session);
    expect(attraction.setRelationPhenomenon(
               11U, 12U, PerformanceRelationalPhenomenon::Attract)
               && repulsion.setRelationPhenomenon(
                   11U, 12U, PerformanceRelationalPhenomenon::Repel),
           "performer selects distinct phenomena for the same declared cable");
    const auto attractionAnchor = attraction.actorPosition(11U);
    const auto repulsionAnchor = repulsion.actorPosition(11U);
    double difference = 0.0;
    for (int sample = 0; sample < 8192; ++sample) {
        const auto a = attraction.renderStems(48000.0, 0.08, 0.12).mixedMono();
        const auto b = repulsion.renderStems(48000.0, 0.08, 0.12).mixedMono();
        difference += std::fabs(a - b);
    }
    expect(difference > 1.0e-3,
           "attraction and repulsion produce different compositional audio");
    expect(attraction.actorPosition(11U).x == attractionAnchor.x
               && attraction.actorPosition(11U).y == attractionAnchor.y
               && repulsion.actorPosition(11U).x == repulsionAnchor.x
               && repulsion.actorPosition(11U).y == repulsionAnchor.y,
           "relational phenomena act on live state without overwriting authored anchors");
}

void testMemoryRelationColoursModalMatter() {
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
    auto lowScene = ecologyScene(46U, 0.66);
    auto highScene = lowScene;
    expect(lowScene.constellation.addRelation({11U, 13U, RelationKind::Shadow, RelationDomain::Memory,
                                               0.0, 18.0, 2.4, 0.72})
               && highScene.constellation.addRelation({11U, 13U, RelationKind::Shadow, RelationDomain::Memory,
                                                       1.0, 18.0, 2.4, 0.72}),
           "memory relation enters modal comparison scenes");
    lowScene.session.memory = highScene.session.memory = 0.82;
    PerformanceActorEcology low;
    PerformanceActorEcology high;
    expect(low.prepare(48000.0, 64U, 2U) && high.prepare(48000.0, 64U, 2U),
           "modal memory comparison ecologies prepare");
    low.setMorph(PerformanceSceneMorpher::morph(lowScene, lowScene, 1.0));
    high.setMorph(PerformanceSceneMorpher::morph(highScene, highScene, 1.0));
    low.setMacroState(lowScene.session);
    high.setMacroState(highScene.session);
    double difference = 0.0;
    for (int sample = 0; sample < 8192; ++sample) {
        const auto a = low.renderStems(48000.0, 0.12, 0.42);
        const auto b = high.renderStems(48000.0, 0.12, 0.42);
        expect(a.modalReturn == 0.0 && b.modalReturn == 0.0,
               "four-stem ModalResonator never escapes through a hidden global return");
        const auto* aOwner = a.findActor(13U);
        const auto* bOwner = b.findActor(13U);
        expect(aOwner != nullptr && bOwner != nullptr,
               "memory destination remains an addressable owned Modal layer");
        difference += std::fabs(aOwner->mono - bOwner->mono);
    }
    expect(difference > 1.0e-3,
           "memory cable has a bounded audible consequence in ModalResonator matter");
#else
    // Modal matter is an optional Performance-owned package. A contracts-only
    // build has no ModalResonator return to compare, so it must not pretend to
    // prove an audible consequence that is not part of this binary.
    expect(true, "contracts-only build correctly skips absent modal matter");
#endif
}

void testLifeStateShapesStemAudibility() {
    auto activeScene = ecologyScene(71U, 0.58);
    auto dormantScene = activeScene;
    dormantScene.constellation.actors[0].life = ActorLifeState::Dormant;
    PerformanceActorEcology active;
    PerformanceActorEcology dormant;
    expect(active.prepare(48000.0, 64U, 2U) && dormant.prepare(48000.0, 64U, 2U),
           "life-state comparison ecologies prepare");
    active.setMorph(PerformanceSceneMorpher::morph(activeScene, activeScene, 1.0));
    dormant.setMorph(PerformanceSceneMorpher::morph(dormantScene, dormantScene, 1.0));
    active.setMacroState(activeScene.session);
    dormant.setMacroState(dormantScene.session);
    double activeEnergy = 0.0;
    double dormantEnergy = 0.0;
    for (int sample = 0; sample < 8192; ++sample) {
        const auto activeStem = active.renderStems(48000.0, 0.0, 0.0).findActor(11U)->mono;
        const auto dormantStem = dormant.renderStems(48000.0, 0.0, 0.0).findActor(11U)->mono;
        activeEnergy += activeStem * activeStem;
        dormantEnergy += dormantStem * dormantStem;
    }
    expect(activeEnergy > dormantEnergy * 100.0,
           "active life state makes a stem substantially more audible than dormant life");
    expect(dormant.setPresence(11U, 0.75), "dragging presence can wake a dormant stem");
    double awakenedEnergy = 0.0;
    for (int sample = 0; sample < 8192; ++sample) {
        const auto stem = dormant.renderStems(48000.0, 0.0, 0.0).findActor(11U)->mono;
        awakenedEnergy += stem * stem;
    }
    expect(awakenedEnergy > dormantEnergy * 100.0,
           "waking a stem by presence restores an audible body without constructing DSP");
}

void testWildTokensChangeTheSeedTrajectory() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(72U, 0.61);
    PerformanceSessionEngine first;
    PerformanceSessionEngine second;
    expect(first.prepare(48000.0, 64U, 2U) && second.prepare(48000.0, 64U, 2U)
           && first.setScene(scene, territory) && second.setScene(scene, territory),
           "matching engines prepare one seed for wild-token comparison");
    // Estes dois tokens diferem só no "grão" (bits 16–23). Sem a trama, ele
    // acelera a fase estéreo da mistura antiga; com a trama, empurra a
    // estrela (R7, 3/11) e soa pelo pan pela física — os dois caminhos são
    // conferidos (o com trama logo abaixo).
    first.setSkyInfluence(0.0);
    second.setSkyInfluence(0.0);

    auto firstGesture = gesture(GestureType::Mutate, 0U, 0.86);
    auto secondGesture = firstGesture;
    firstGesture.variationToken = 0x00100001U;
    secondGesture.variationToken = 0x00e000f1U;
    expect(first.pushGesture(firstGesture) && second.pushGesture(secondGesture),
           "each bounded Mutate gesture accepts its control-side variation token");

    std::array<float, 64> firstLeft{}, firstRight{}, secondLeft{}, secondRight{};
    double difference = 0.0;
    for (int block = 0; block < 128; ++block) {
        first.processBlock(firstLeft.data(), firstRight.data(), 64U);
        second.processBlock(secondLeft.data(), secondRight.data(), 64U);
        for (std::size_t sample = 0U; sample < firstLeft.size(); ++sample) {
            const auto leftDelta = static_cast<double>(firstLeft[sample] - secondLeft[sample]);
            const auto rightDelta = static_cast<double>(firstRight[sample] - secondRight[sample]);
            difference += leftDelta * leftDelta + rightDelta * rightDelta;
        }
    }
    expect(difference > 1.0e-4,
           "different wild tokens audibly subvert the same seed along different trajectories");

    // Com a trama: o grão empurra as estrelas em direções diferentes e a
    // diferença chega ao estéreo pelo pan pela física.
    PerformanceSessionEngine woven;
    PerformanceSessionEngine otherWoven;
    expect(woven.prepare(48000.0, 64U, 2U) && otherWoven.prepare(48000.0, 64U, 2U)
           && woven.setScene(scene, territory) && otherWoven.setScene(scene, territory),
           "woven engines prepare one seed for the grain push");
    std::array<float, 64> wovenLeft{}, wovenRight{}, otherLeft{}, otherRight{};
    for (int block = 0; block < 400; ++block) {
        woven.processBlock(wovenLeft.data(), wovenRight.data(), 64U);
        otherWoven.processBlock(otherLeft.data(), otherRight.data(), 64U);
    }
    expect(woven.pushGesture(firstGesture) && otherWoven.pushGesture(secondGesture),
           "woven engines accept the two grains");
    double wovenDifference = 0.0;
    for (int block = 0; block < 1200; ++block) {
        woven.processBlock(wovenLeft.data(), wovenRight.data(), 64U);
        otherWoven.processBlock(otherLeft.data(), otherRight.data(), 64U);
        for (std::size_t sample = 0U; sample < wovenLeft.size(); ++sample) {
            const auto leftDelta = static_cast<double>(wovenLeft[sample] - otherLeft[sample]);
            const auto rightDelta = static_cast<double>(wovenRight[sample] - otherRight[sample]);
            wovenDifference += leftDelta * leftDelta + rightDelta * rightDelta;
        }
    }
    expect(wovenDifference > 1.0e-4,
           "with the sky web, the grain pushes the star and is heard through the physical pan");
}

void testFocusedWildGesturePrioritizesTheSelectedStem() {
    const auto scene = ecologyScene(73U, 0.60);
    const auto morph = PerformanceSceneMorpher::morph(scene, scene, 1.0);
    PerformanceActorEcology baseline;
    PerformanceActorEcology focused;
    expect(baseline.prepare(48000.0, 64U, 2U) && focused.prepare(48000.0, 64U, 2U),
           "focused wild comparison ecologies prepare");
    baseline.setMorph(morph);
    focused.setMorph(morph);
    baseline.setMacroState(scene.session);
    focused.setMacroState(scene.session);
    std::array<double, 3> deltas{};
    for (int sample = 0; sample < 4096; ++sample) {
        const auto plain = baseline.renderStems(48000.0, 0.0, 0.0);
        const auto selected = focused.renderStems(48000.0, 0.86, 0.0, 0.0, 0.0, 11U);
        for (std::size_t stem = 0U; stem < deltas.size(); ++stem)
            deltas[stem] += std::fabs(selected.actors[stem].mono - plain.actors[stem].mono);
    }
    expect(deltas[0] > deltas[1] * 1.35 && deltas[0] > deltas[2] * 1.35,
           "a focused wild gesture prioritizes its selected stem over the surrounding ecology");

    PerformanceActorEcology tearBaseline;
    PerformanceActorEcology focusedTear;
    expect(tearBaseline.prepare(48000.0, 64U, 2U) && focusedTear.prepare(48000.0, 64U, 2U),
           "focused tear comparison ecologies prepare");
    tearBaseline.setMorph(morph);
    focusedTear.setMorph(morph);
    tearBaseline.setMacroState(scene.session);
    focusedTear.setMacroState(scene.session);
    deltas.fill(0.0);
    for (int sample = 0; sample < 4096; ++sample) {
        const auto plain = tearBaseline.renderStems(48000.0, 0.0, 0.0);
        const auto selected = focusedTear.renderStems(48000.0, 0.0, 0.86, 0.0, 0.0,
                                                      kInvalidActorId, 11U);
        for (std::size_t stem = 0U; stem < deltas.size(); ++stem)
            deltas[stem] += std::fabs(selected.actors[stem].mono - plain.actors[stem].mono);
    }
    expect(deltas[0] > deltas[1] * 1.25 && deltas[0] > deltas[2] * 1.25,
           "a focused Rasgar gesture prioritizes its selected stem over the surrounding ecology");

    PerformanceActorEcology repairBaseline;
    PerformanceActorEcology focusedRepair;
    expect(repairBaseline.prepare(48000.0, 64U, 2U) && focusedRepair.prepare(48000.0, 64U, 2U),
           "focused repair comparison ecologies prepare");
    repairBaseline.setMorph(morph);
    focusedRepair.setMorph(morph);
    repairBaseline.setMacroState(scene.session);
    focusedRepair.setMacroState(scene.session);
    deltas.fill(0.0);
    for (int sample = 0; sample < 4096; ++sample) {
        const auto plain = repairBaseline.renderStems(48000.0, 0.0, 0.0);
        const auto selected = focusedRepair.renderStems(48000.0, 0.0, 0.0, 0.86, 0.0,
                                                        kInvalidActorId, kInvalidActorId, 11U);
        for (std::size_t stem = 0U; stem < deltas.size(); ++stem)
            deltas[stem] += std::fabs(selected.actors[stem].mono - plain.actors[stem].mono);
    }
    expect(deltas[0] > deltas[1] * 1.20 && deltas[0] > deltas[2] * 1.20,
           "a focused Recall gesture prioritizes its selected stem over the surrounding ecology");
}

void testSeedEcologiesReachTheCompleteEngine() {
#if defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V2)
    std::array<double, 3> signatures{};
    for (std::size_t territoryIndex = 0U; territoryIndex < signatures.size(); ++territoryIndex) {
        const auto seed = static_cast<std::uint32_t>(181U + territoryIndex);
        const auto document = PerformanceTerritoryFactory::make(seed, true);
        PerformanceSessionEngine engine;
        expect(engine.prepare(48000.0, 64U, 2U)
               && engine.setScene(document.scenes[1U], document.territory),
               "seed ecology reaches the complete session engine");
        std::array<float, 64> left{}, right{};
        std::array<std::uint32_t, 3> maximumStemPeak{};
        double energy = 0.0;
        // Os 3 primeiros segundos (era 1 s): desde 4/11 a abertura pode ser
        // rarefeita (o caráter da parte), e a segunda estrela entra depois.
        for (int block = 0; block < 2250; ++block) {
            engine.processBlock(left.data(), right.data(), 64U);
            const auto telemetry = engine.readTelemetry();
            expect(telemetry.stemCount == 3U,
                   "seed ecology preserves three independently mixed stems");
            for (std::size_t stem = 0U; stem < maximumStemPeak.size(); ++stem)
                maximumStemPeak[stem] = std::max(maximumStemPeak[stem],
                                                 telemetry.stemPeakMicros[stem]);
            for (std::size_t sample = 0U; sample < left.size(); ++sample) {
                expect(std::isfinite(left[sample]) && std::isfinite(right[sample]),
                       "seed ecology remains finite through master and stereo stages");
                energy += static_cast<double>(left[sample] * left[sample]
                                               + right[sample] * right[sample]);
                signatures[territoryIndex] += static_cast<double>(left[sample])
                    * std::sin(static_cast<double>(block * 64 + static_cast<int>(sample) + 1) * 0.0017);
            }
        }
        expect(energy > 1.0e-4,
               "seed ecology produces music through the complete engine");
        const auto audibleStemCount = static_cast<std::size_t>(std::count_if(
            maximumStemPeak.begin(), maximumStemPeak.end(),
            [](std::uint32_t peak) { return peak > 100U; }));
        expect(audibleStemCount >= 2U,
               "each seed begins with at least two audible stems while one body may remain in reserve");
    }
    expect(std::fabs(signatures[0] - signatures[1]) > 1.0e-4
           && std::fabs(signatures[1] - signatures[2]) > 1.0e-4,
           "circuit, breath and voice seeds remain musically distinct after mixing");
#endif
}

void testWalshCanBeAwakenedWithoutTransportReset() {
#if defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V3)
    const auto document = PerformanceTerritoryFactory::makeWithWalsh(181U, true);
    expect(document.scenes[0].constellation.findActor(4U) != nullptr
               && document.scenes[0].constellation.findActor(4U)->presence > 0.0
               && document.scenes[0].constellation.findActor(4U)->life == ActorLifeState::Emerging,
           "the seed opens the fourth matter with audible non-zero presence");
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 64U, 2U)
               && engine.setScene(document.scenes[0], document.territory),
           "accumulated Walsh territory prepares in PLAY");
    std::array<float, 64> left{}, right{};
    for (int block = 0; block < 32; ++block) engine.processBlock(left.data(), right.data(), 64U);
    const auto before = engine.readTelemetry();
    expect(before.playing && before.stemCount == 4U,
           "seed-opened fourth body remains in the bounded stem table while transport runs");
    expect(engine.setActorPresence(4U, 0.78), "Walsh presence enters through the existing realtime path");
    for (int block = 0; block < 240; ++block) engine.processBlock(left.data(), right.data(), 64U);
    const auto after = engine.readTelemetry();
    expect(after.playing && after.stemActorIds[3] == 4U && after.stemPeakMicros[3] > 100U,
           "Walsh becomes an audible fourth stem without stopping transport");
#endif
}

void testWalshSpatialCouplingChangesTrajectory() {
#if defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V3)
    const auto document = PerformanceTerritoryFactory::makeWithWalsh(181U, true);
    PerformanceSessionEngine nearEngine;
    PerformanceSessionEngine farEngine;
    expect(nearEngine.prepare(48000.0, 64U, 2U) && farEngine.prepare(48000.0, 64U, 2U)
               && nearEngine.setScene(document.scenes[0], document.territory)
               && farEngine.setScene(document.scenes[0], document.territory),
           "spatial Walsh sessions prepare");
    expect(nearEngine.setActorPresence(4U, 0.82) && farEngine.setActorPresence(4U, 0.82),
           "spatial Walsh sessions activate the fourth body");
    expect(nearEngine.setActorPosition(4U, 0.18, 0.48)
               && farEngine.setActorPosition(4U, 0.96, 0.92),
           "Walsh position crosses the bounded realtime path");
    std::array<float, 64> nearLeft{}, nearRight{}, farLeft{}, farRight{};
    double difference = 0.0;
    for (int block = 0; block < 160; ++block) {
        nearEngine.processBlock(nearLeft.data(), nearRight.data(), 64U);
        farEngine.processBlock(farLeft.data(), farRight.data(), 64U);
        for (std::size_t sample = 0U; sample < nearLeft.size(); ++sample)
            difference += std::fabs(static_cast<double>(nearLeft[sample] - farLeft[sample]));
    }
    expect(difference > 1.0e-3, "Walsh distance changes the spatially coupled trajectory");
#endif
}

void testAutonomousGameWaitsForPauseAndPreservesPosition() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(27U, 0.58);
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 64U, 2U) && engine.setScene(scene, territory),
           "autonomous game fixture prepares");
    expect(engine.setActorPosition(11U, 0.21, 0.73),
           "autonomous game fixture accepts the musician's field");
    std::array<float, 64> left{}, right{};
    for (int block = 0; block < 1499; ++block)
        engine.processBlock(left.data(), right.data(), 64U);
    expect(!engine.lastSpatialGameEvent().valid(),
           "autonomous game waits for the complete inactivity interval");
    expect(engine.setActorPosition(11U, 0.22, 0.74),
           "a new spatial gesture is accepted before autonomous play");
    for (int block = 0; block < 1499; ++block)
        engine.processBlock(left.data(), right.data(), 64U);
    expect(!engine.lastSpatialGameEvent().valid(),
           "spatial activity restarts the inactivity interval");
    engine.processBlock(left.data(), right.data(), 64U);
    const auto event = engine.lastSpatialGameEvent();
    const auto position = engine.actorPosition(11U);
    expect(event.valid(), "autonomous game produces a situated event after the pause");
    expect(std::fabs(position.x - 0.22) < 1.0e-12 && std::fabs(position.y - 0.74) < 1.0e-12,
           "autonomous game does not redraw the musician's spatial field");
}

void testAutonomousGameHonoursImportedTerritoryPlan() {
    auto territory = ecologyTerritory();
    territory.gamePlan = {PerformanceGameStrategy::Interrupt, PerformanceGameRule::Break,
                          0.72, 0.31, 0.87, 0.66, 0.42};
    const auto scene = ecologyScene(31U, 0.58);
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 64U, 2U) && engine.setScene(scene, territory),
           "imported game-plan fixture prepares");
    expect(engine.gamePlan().strategy == PerformanceGameStrategy::Interrupt
               && engine.gamePlan().rule == PerformanceGameRule::Break,
           "engine retains the territory game plan at installation");
    std::array<float, 64> left{}, right{};
    for (int block = 0; block < 1500; ++block)
        engine.processBlock(left.data(), right.data(), 64U);
    const auto event = engine.lastSpatialGameEvent();
    expect(event.valid() && event.strategy == PerformanceGameStrategy::Interrupt
               && event.rule == PerformanceGameRule::Break,
           "the unattended field acts through the imported Studio policy");
}

void testAutonomousStoryDoesNotRequireAutonomyDominance() {
    const auto territory = ecologyTerritory();
    auto scene = ecologyScene(29U, 0.58);
    scene.session.autonomy = 0.24;
    scene.session.intervention = 0.86;
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 64U, 2U) && engine.setScene(scene, territory),
           "low-autonomy narrative fixture prepares");
    expect(engine.setActorPosition(11U, 0.27, 0.71),
           "low-autonomy narrative fixture accepts the musician field");
    std::array<float, 64> left{}, right{};
    for (int block = 0; block < 1500; ++block)
        engine.processBlock(left.data(), right.data(), 64U);
    const auto position = engine.actorPosition(11U);
    const auto telemetry = engine.readTelemetry();
    expect(engine.lastSpatialGameEvent().valid(),
           "the seed continues its story after a pause even when intervention is higher");
    expect(telemetry.narrativeEventValid && telemetry.narrativeTurn == 0U,
           "the first autonomous seed turn reaches readonly telemetry");
    expect(std::fabs(position.x - 0.27) < 1.0e-12 && std::fabs(position.y - 0.71) < 1.0e-12,
           "low-autonomy narrative still preserves the performer drawing");
    for (int block = 0; block < 4500 && engine.readTelemetry().narrativeTurn < 1U; ++block)
        engine.processBlock(left.data(), right.data(), 64U);
    const auto secondTelemetry = engine.readTelemetry();
    const auto secondPosition = engine.actorPosition(11U);
    expect(secondTelemetry.narrativeEventValid && secondTelemetry.narrativeTurn == 1U,
           "an unattended performance advances to the next seeded turn instead of stopping at one event");
    expect(std::fabs(secondPosition.x - 0.27) < 1.0e-12
               && std::fabs(secondPosition.y - 0.71) < 1.0e-12,
           "the continuing seed sequence keeps the performer drawing intact");
}

void testAutonomousGameReachesAudioDifferentlyFromTwoFields() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(28U, 0.58);
    PerformanceSessionEngine nearEngine;
    PerformanceSessionEngine farEngine;
    expect(nearEngine.prepare(48000.0, 64U, 2U) && farEngine.prepare(48000.0, 64U, 2U)
               && nearEngine.setScene(scene, territory) && farEngine.setScene(scene, territory),
           "A/B autonomous game engines prepare");
    expect(nearEngine.setActorPosition(11U, 0.20, 0.30)
               && nearEngine.setActorPosition(12U, 0.25, 0.34)
               && nearEngine.setActorPosition(13U, 0.80, 0.80)
               && farEngine.setActorPosition(11U, 0.20, 0.30)
               && farEngine.setActorPosition(12U, 0.90, 0.90)
               && farEngine.setActorPosition(13U, 0.80, 0.80),
           "A/B autonomous game fields are accepted");
    std::array<float, 64> nearLeft{}, nearRight{}, farLeft{}, farRight{};
    double difference = 0.0;
    // A janela fica logo depois da primeira pausa da narrativa (o som volta
    // por volta do bloco 1100). Até 30/10 ela ficava nos blocos 1500–1510,
    // que já são o começo da pausa seguinte: quase silêncio nos dois campos,
    // e o teste passava por um resíduo de 3,6e-5 contra o limiar de 1e-5.
    // Com o movimento físico do céu esse resíduo encolheu, mas os dois
    // campos continuam bem diferentes ao longo da peça (diferença total
    // ~656 contra ~674 antes) — a janela agora mede onde há som.
    for (int block = 0; block < 1510; ++block) {
        nearEngine.processBlock(nearLeft.data(), nearRight.data(), 64U);
        farEngine.processBlock(farLeft.data(), farRight.data(), 64U);
        if (block < 1100 || block >= 1110) continue;
        for (std::size_t sample = 0U; sample < nearLeft.size(); ++sample)
            difference += std::fabs(static_cast<double>(nearLeft[sample] - farLeft[sample]));
    }
    expect(nearEngine.lastSpatialGameEvent().valid() && farEngine.lastSpatialGameEvent().valid(),
           "both fields produce autonomous situated events");
    expect(nearEngine.lastSpatialGameEvent().spatialInfluence
               != farEngine.lastSpatialGameEvent().spatialInfluence,
           "the two fields produce different spatial influences");
    expect(difference > 1.0e-5,
           "different fields produce an audible post-pause A/B trajectory");
}

void testMotionFieldsReachAudioWithoutMovingTheDesk() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(30U, 0.62);
    PerformanceSessionEngine orbit;
    PerformanceSessionEngine pulse;
    PerformanceSessionEngine figureEight;
    expect(orbit.prepare(48000.0, 64U, 2U) && pulse.prepare(48000.0, 64U, 2U)
               && figureEight.prepare(48000.0, 64U, 2U)
               && orbit.setScene(scene, territory) && pulse.setScene(scene, territory)
               && figureEight.setScene(scene, territory),
           "motion-field A/B engines prepare");
    expect(orbit.setMotionField(PerformanceMotionField::Orbit)
               && pulse.setMotionField(PerformanceMotionField::Pulse)
               && figureEight.setMotionField(PerformanceMotionField::FigureEight),
           "motion-field A/B engines accept every declared regime");
    const auto drawing = orbit.actorPosition(11U);
    const auto initialLive = orbit.actorLivePosition(11U);
    std::array<float, 64> orbitLeft{}, orbitRight{}, pulseLeft{}, pulseRight{}, figureLeft{}, figureRight{};
    double orbitPulseDifference = 0.0;
    double pulseFigureDifference = 0.0;
    for (int block = 0; block < 360; ++block) {
        orbit.processBlock(orbitLeft.data(), orbitRight.data(), 64U);
        pulse.processBlock(pulseLeft.data(), pulseRight.data(), 64U);
        figureEight.processBlock(figureLeft.data(), figureRight.data(), 64U);
        for (std::size_t sample = 0U; sample < orbitLeft.size(); ++sample) {
            orbitPulseDifference += std::fabs(static_cast<double>(orbitLeft[sample] - pulseLeft[sample]));
            pulseFigureDifference += std::fabs(static_cast<double>(pulseLeft[sample] - figureLeft[sample]));
        }
    }
    const auto preserved = orbit.actorPosition(11U);
    const auto finalLive = orbit.actorLivePosition(11U);
    const auto motionTelemetry = orbit.readTelemetry();
    expect(orbitPulseDifference > 1.0e-4 && pulseFigureDifference > 1.0e-4,
           "orbit, pulse and figure-eight create distinct audible motion fields");
    expect(std::fabs(drawing.x - preserved.x) < 1.0e-12 && std::fabs(drawing.y - preserved.y) < 1.0e-12,
           "motion fields never rewrite the performer drawing");
    expect(std::fabs(initialLive.x - finalLive.x) > 1.0e-4
               || std::fabs(initialLive.y - finalLive.y) > 1.0e-4,
           "the audible body moves independently around the preserved drawing anchor");
    expect(motionTelemetry.motionPhaseMicros > 0U
               && motionTelemetry.motionField == static_cast<std::uint8_t>(PerformanceMotionField::Orbit),
           "the UI can read the exact audio-motion phase and field from telemetry");
    expect(motionTelemetry.motionPhaseMicros < 200000U,
           "the default orbit remains slow enough to preserve the authored drawing after one second");
}

void testMotionFieldChangeIsImmediateAndPreservesTheDesk() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(30U, 0.62);
    PerformanceSessionEngine orbit;
    PerformanceSessionEngine pulse;
    expect(orbit.prepare(48000.0, 64U, 2U) && pulse.prepare(48000.0, 64U, 2U)
               && orbit.setScene(scene, territory) && pulse.setScene(scene, territory),
           "motion-field immediate A/B engines prepare");
    expect(orbit.setMotionField(PerformanceMotionField::Orbit)
               && pulse.setMotionField(PerformanceMotionField::Pulse),
           "motion field changes enter both engines");
    const auto drawing = pulse.actorPosition(11U);
    std::array<float, 64> orbitLeft{}, orbitRight{}, pulseLeft{}, pulseRight{};
    double difference = 0.0;
    for (int block = 0; block < 8; ++block) {
        orbit.processBlock(orbitLeft.data(), orbitRight.data(), 64U);
        pulse.processBlock(pulseLeft.data(), pulseRight.data(), 64U);
        for (std::size_t sample = 0U; sample < orbitLeft.size(); ++sample)
            difference += std::fabs(static_cast<double>(orbitLeft[sample] - pulseLeft[sample]));
    }
    const auto preserved = pulse.actorPosition(11U);
    expect(difference > 1.0e-4,
           "motion selection has an audible consequence within eight audio blocks");
    expect(std::fabs(drawing.x - preserved.x) < 1.0e-12 && std::fabs(drawing.y - preserved.y) < 1.0e-12,
           "immediate motion consequence never moves the performer drawing");
}

void testSpaceMacroControlsBoundedMotionRate() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(30U, 0.62);
    PerformanceSessionEngine slow;
    PerformanceSessionEngine fast;
    expect(slow.prepare(48000.0, 64U, 2U) && fast.prepare(48000.0, 64U, 2U)
               && slow.setScene(scene, territory) && fast.setScene(scene, territory)
               && slow.setMacro(PerformanceMacro::Space, 0.0)
               && fast.setMacro(PerformanceMacro::Space, 1.0),
           "space-speed A/B engines prepare and accept bounded endpoints");
    const auto drawing = slow.actorPosition(11U);
    std::array<float, 64> slowLeft{}, slowRight{}, fastLeft{}, fastRight{};
    for (int block = 0; block < 480; ++block) {
        slow.processBlock(slowLeft.data(), slowRight.data(), 64U);
        fast.processBlock(fastLeft.data(), fastRight.data(), 64U);
    }
    const auto slowTelemetry = slow.readTelemetry();
    const auto fastTelemetry = fast.readTelemetry();
    expect(fastTelemetry.motionPhaseMicros > slowTelemetry.motionPhaseMicros,
           "ESPAÇO changes the shared movement rate audibly through the audio clock");
    expect(slow.actorPosition(11U).x == drawing.x && slow.actorPosition(11U).y == drawing.y,
           "ESPAÇO speed does not rewrite the performer's authored drawing");
}

void testBodyMotionFieldsCanDivergeWithoutChangingAnchors() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(30U, 0.62);
    PerformanceSessionEngine reference;
    PerformanceSessionEngine varied;
    expect(reference.prepare(48000.0, 64U, 2U) && varied.prepare(48000.0, 64U, 2U)
               && reference.setScene(scene, territory) && varied.setScene(scene, territory),
           "per-body movement fixtures prepare");
    expect(varied.setActorMotionField(11U, PerformanceMotionField::Pulse)
               && varied.setActorMotionField(12U, PerformanceMotionField::FigureEight)
               && varied.setActorMotionDirection(11U, false)
               && !varied.setActorMotionDirection(99U, false)
               && !varied.setActorMotionField(99U, PerformanceMotionField::Pulse),
           "only real bodies receive bounded individual dances and directions");
    const auto firstAnchor = varied.actorPosition(11U);
    const auto secondAnchor = varied.actorPosition(12U);
    std::array<float, 64> referenceLeft{}, referenceRight{}, variedLeft{}, variedRight{};
    double difference = 0.0;
    for (int block = 0; block < 24; ++block) {
        reference.processBlock(referenceLeft.data(), referenceRight.data(), 64U);
        varied.processBlock(variedLeft.data(), variedRight.data(), 64U);
        for (std::size_t sample = 0U; sample < variedLeft.size(); ++sample)
            difference += std::fabs(static_cast<double>(referenceLeft[sample] - variedLeft[sample]));
    }
    const auto telemetry = varied.readTelemetry();
    bool firstReported = false;
    bool secondReported = false;
    bool firstReverse = false;
    for (std::size_t stem = 0U; stem < telemetry.stemCount; ++stem) {
        if (telemetry.stemActorIds[stem] == 11U)
            firstReported = telemetry.stemMotionFields[stem]
                == static_cast<std::uint8_t>(PerformanceMotionField::Pulse);
        if (telemetry.stemActorIds[stem] == 11U)
            firstReverse = telemetry.stemMotionDirections[stem] == 0U;
        if (telemetry.stemActorIds[stem] == 12U)
            secondReported = telemetry.stemMotionFields[stem]
                == static_cast<std::uint8_t>(PerformanceMotionField::FigureEight);
    }
    const auto firstPreserved = varied.actorPosition(11U);
    const auto secondPreserved = varied.actorPosition(12U);
    expect(difference > 1.0e-4, "different body dances cause an immediate audible change");
    expect(firstReported && secondReported && firstReverse,
           "telemetry identifies each body with the movement geometry and direction it actually hears");
    expect(std::fabs(firstAnchor.x - firstPreserved.x) < 1.0e-12
               && std::fabs(firstAnchor.y - firstPreserved.y) < 1.0e-12
               && std::fabs(secondAnchor.x - secondPreserved.x) < 1.0e-12
               && std::fabs(secondAnchor.y - secondPreserved.y) < 1.0e-12,
           "independent dances never rewrite either authored anchor");
}

void testCapturedPopulationPhenomenonReturnsToAudioWithoutMovingTheDesk() {
    const auto territory = ecologyTerritory();
    auto scene = ecologyScene(30U, 0.62);
    scene.session.populationPhenomenon = PerformancePopulationPhenomenon::Cluster;
    scene.session.populationPhenomenonIntensity = 0.78;
    scene.session.populationPhenomenonEnabled = true;
    PerformanceSessionEngine baseline;
    PerformanceSessionEngine recalled;
    expect(baseline.prepare(48000.0, 64U, 2U) && recalled.prepare(48000.0, 64U, 2U)
               && baseline.setScene(ecologyScene(30U, 0.62), territory)
               && recalled.setScene(scene, territory),
           "captured population fixture prepares both engines");
    const auto firstAnchor = recalled.actorPosition(11U);
    const auto secondAnchor = recalled.actorPosition(12U);
    std::array<float, 64> baselineLeft{}, baselineRight{}, recalledLeft{}, recalledRight{};
    double difference = 0.0;
    for (int block = 0; block < 360; ++block) {
        baseline.processBlock(baselineLeft.data(), baselineRight.data(), 64U);
        recalled.processBlock(recalledLeft.data(), recalledRight.data(), 64U);
        for (std::size_t sample = 0U; sample < baselineLeft.size(); ++sample)
            difference += std::fabs(static_cast<double>(baselineLeft[sample] - recalledLeft[sample]));
    }
    expect(difference > 1.0e-4, "captured population phenomenon changes the audio trajectory");
    const auto finalFirstAnchor = recalled.actorPosition(11U);
    const auto finalSecondAnchor = recalled.actorPosition(12U);
    expect(std::fabs(firstAnchor.x - finalFirstAnchor.x) < 1.0e-12
               && std::fabs(firstAnchor.y - finalFirstAnchor.y) < 1.0e-12
               && std::fabs(secondAnchor.x - finalSecondAnchor.x) < 1.0e-12
               && std::fabs(secondAnchor.y - finalSecondAnchor.y) < 1.0e-12,
           "captured population phenomenon preserves both authored anchors");
}

void testSeedCollageReachesAudioWithoutChangingThePrimaryDesk() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(32U, 0.61);
    PerformanceSessionEngine dry;
    PerformanceSessionEngine collage;
    expect(dry.prepare(48000.0, 64U, 2U) && collage.prepare(48000.0, 64U, 2U)
               && dry.setScene(scene, territory) && collage.setScene(scene, territory)
               && dry.setSeedCollage(182U, 0.0) && collage.setSeedCollage(182U, 1.0),
           "seed-collage A/B engines prepare with bounded extremes");
    const auto drawing = collage.actorPosition(11U);
    std::array<float, 64> dryLeft{}, dryRight{}, collageLeft{}, collageRight{};
    double difference = 0.0;
    for (int block = 0; block < 420; ++block) {
        dry.processBlock(dryLeft.data(), dryRight.data(), 64U);
        collage.processBlock(collageLeft.data(), collageRight.data(), 64U);
        for (std::size_t sample = 0U; sample < dryLeft.size(); ++sample)
            difference += std::fabs(static_cast<double>(dryLeft[sample] - collageLeft[sample]));
    }
    const auto preserved = collage.actorPosition(11U);
#if defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V1)
    expect(difference > 1.0e-4, "a secondary seed collage changes the audible stem history");
#else
    expect(collage.activeActorCount() == 3U,
           "contracts-only seed collage preserves the declared bounded ensemble");
#endif
    expect(std::fabs(drawing.x - preserved.x) < 1.0e-12 && std::fabs(drawing.y - preserved.y) < 1.0e-12,
           "seed collage preserves the primary performer drawing");
}

void testActorSeedVariationReachesOnlyItsStemWithoutChangingTheDesk() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(32U, 0.61);
    PerformanceSessionEngine origin;
    PerformanceSessionEngine varied;
    expect(origin.prepare(48000.0, 64U, 2U) && varied.prepare(48000.0, 64U, 2U)
               && origin.setScene(scene, territory) && varied.setScene(scene, territory)
               && origin.setSeedCollage(182U, 0.0) && varied.setSeedCollage(182U, 0.0)
               && varied.setActorSeedVariation(11U, 1.0),
           "local seed variation prepares on one existing body without replacing the collective origin");
    const auto firstAnchor = varied.actorPosition(11U);
    const auto secondAnchor = varied.actorPosition(12U);
    std::array<float, 64> originLeft{}, originRight{}, variedLeft{}, variedRight{};
    double difference = 0.0;
    for (int block = 0; block < 420; ++block) {
        origin.processBlock(originLeft.data(), originRight.data(), 64U);
        varied.processBlock(variedLeft.data(), variedRight.data(), 64U);
        for (std::size_t sample = 0U; sample < originLeft.size(); ++sample)
            difference += std::fabs(static_cast<double>(originLeft[sample] - variedLeft[sample]));
    }
    const auto firstPreserved = varied.actorPosition(11U);
    const auto secondPreserved = varied.actorPosition(12U);
    expect(difference > 1.0e-4,
           "a local seed variation changes audible history without waiting for a new territory");
    expect(std::fabs(firstAnchor.x - firstPreserved.x) < 1.0e-12
               && std::fabs(firstAnchor.y - firstPreserved.y) < 1.0e-12
               && std::fabs(secondAnchor.x - secondPreserved.x) < 1.0e-12
               && std::fabs(secondAnchor.y - secondPreserved.y) < 1.0e-12,
           "local seed variation preserves the complete performer drawing");
}

void testInternalModuleSeedVariationReachesFoldedStemWithoutAddingBodies() {
    const auto document = PerformanceTerritoryFactory::makeForReleasedStems(185U, 4U, true);
    for (std::uint16_t actorId = 5U; actorId <= 8U; ++actorId) {
        PerformanceSessionEngine origin;
        PerformanceSessionEngine varied;
        expect(origin.prepare(48000.0, 64U, 2U) && varied.prepare(48000.0, 64U, 2U)
                   && origin.setScene(document.scenes[0], document.territory)
                   && varied.setScene(document.scenes[0], document.territory)
                   && varied.setActorSeedVariation(actorId, 1.0)
                   && varied.setModuleControl(actorId, PerformanceModuleControl::Primary, 1.0),
               "each internal lane accepts variation and local control without releasing a fifth stem");
        expect(origin.activeActorCount() == 4U && varied.activeActorCount() == 4U,
               "each internal lane preserves the four-stem safety limit");
        std::array<float, 64> originLeft{}, originRight{}, variedLeft{}, variedRight{};
        double difference = 0.0;
        for (int block = 0; block < 420; ++block) {
            origin.processBlock(originLeft.data(), originRight.data(), 64U);
            varied.processBlock(variedLeft.data(), variedRight.data(), 64U);
            for (std::size_t sample = 0U; sample < originLeft.size(); ++sample)
                difference += std::fabs(static_cast<double>(originLeft[sample] - variedLeft[sample]));
        }
        // Since 2026-10-28 a body outside the released asterism keeps its
        // stored variation and controls but is never folded, unheard and
        // unmixable, into another body's stem.
        expect(difference == 0.0,
               "a body outside the asterism never leaks into another stem");
    }
}

void testRecutsChangeAudioWithoutChangingTheDeskOrStoredTempo() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(33U, 0.64);
    PerformanceSessionEngine dry;
    PerformanceSessionEngine recut;
    expect(dry.prepare(48000.0, 64U, 2U) && recut.prepare(48000.0, 64U, 2U)
               && dry.setScene(scene, territory) && recut.setScene(scene, territory),
           "recut A/B engines prepare");
    const auto originalA = recut.actorPosition(11U);
    const auto originalB = recut.actorPosition(12U);
    PerformanceGestureEvent stutter;
    stutter.type = GestureType::Stutter;
    stutter.targetActorId = 11U;
    stutter.intensity = 0.92;
    stutter.duration = 0.74;
    stutter.variationToken = 0x51a7U;
    PerformanceGestureEvent tempoShift;
    tempoShift.type = GestureType::TempoShift;
    tempoShift.targetActorId = 12U;
    tempoShift.intensity = 0.78;
    tempoShift.duration = 0.70;
    tempoShift.variationToken = 0x51a8U;
    expect(recut.pushGesture(stutter) && recut.pushGesture(tempoShift),
           "bounded stutter and tempo-shift events enter the realtime queue");
    std::array<float, 64> dryLeft{}, dryRight{}, recutLeft{}, recutRight{};
    double difference = 0.0;
    for (int block = 0; block < 480; ++block) {
        dry.processBlock(dryLeft.data(), dryRight.data(), 64U);
        recut.processBlock(recutLeft.data(), recutRight.data(), 64U);
        for (std::size_t sample = 0U; sample < dryLeft.size(); ++sample)
            difference += std::fabs(static_cast<double>(dryLeft[sample] - recutLeft[sample]));
    }
    const auto preservedA = recut.actorPosition(11U);
    const auto preservedB = recut.actorPosition(12U);
    expect(difference > 1.0e-4,
           "stutter and tempo shift create an audible A/B difference over existing stems");
    expect(std::fabs(originalA.x - preservedA.x) < 1.0e-12
               && std::fabs(originalA.y - preservedA.y) < 1.0e-12
               && std::fabs(originalB.x - preservedB.x) < 1.0e-12
               && std::fabs(originalB.y - preservedB.y) < 1.0e-12,
           "recuts never rewrite the performer drawing");
    expect(std::fabs(recut.tempoBpm() - scene.session.tempoBpm) < 1.0e-12,
           "tempo shift never overwrites the stored session BPM");
}

void testGridCueIsImmediateWithoutMovingTheDesk() {
    const auto territory = ecologyTerritory();
    const auto scene = ecologyScene(34U, 0.64);
    PerformanceSessionEngine dry;
    PerformanceSessionEngine cued;
    expect(dry.prepare(48000.0, 64U, 2U) && cued.prepare(48000.0, 64U, 2U)
               && dry.setScene(scene, territory) && cued.setScene(scene, territory)
               && cued.setGestureGrid(true, GestureGridDivision::Quarter),
           "grid cue fixture prepares with quantization enabled");
    const auto drawing = cued.actorPosition(11U);
    PerformanceGestureEvent cue;
    cue.type = GestureType::GridCue;
    cue.targetActorId = 11U;
    cue.intensity = 0.78;
    cue.duration = 0.16;
    cue.variationToken = 0x6a1dU;
    expect(cued.pushGesture(cue), "grid cue enters the realtime event queue");
    std::array<float, 64> dryLeft{}, dryRight{}, cueLeft{}, cueRight{};
    double difference = 0.0;
    for (int block = 0; block < 8; ++block) {
        dry.processBlock(dryLeft.data(), dryRight.data(), 64U);
        cued.processBlock(cueLeft.data(), cueRight.data(), 64U);
        for (std::size_t sample = 0U; sample < dryLeft.size(); ++sample)
            difference += std::fabs(static_cast<double>(dryLeft[sample] - cueLeft[sample]));
    }
    const auto preserved = cued.actorPosition(11U);
    expect(difference > 1.0e-5,
           "grid cue is audible immediately instead of waiting for the selected grid boundary");
    expect(std::fabs(drawing.x - preserved.x) < 1.0e-12 && std::fabs(drawing.y - preserved.y) < 1.0e-12,
           "grid cue never rewrites the performer drawing");

    PerformanceSessionEngine beat;
    PerformanceSessionEngine quarter;
    expect(beat.prepare(48000.0, 64U, 2U) && quarter.prepare(48000.0, 64U, 2U)
               && beat.setScene(scene, territory) && quarter.setScene(scene, territory)
               && beat.setGestureGrid(true, GestureGridDivision::Beat)
               && quarter.setGestureGrid(true, GestureGridDivision::Quarter),
           "grid-rhythm A/B engines prepare with different divisions");
    double gridDifference = 0.0;
    for (int block = 0; block < 480; ++block) {
        beat.processBlock(dryLeft.data(), dryRight.data(), 64U);
        quarter.processBlock(cueLeft.data(), cueRight.data(), 64U);
        for (std::size_t sample = 0U; sample < dryLeft.size(); ++sample)
            gridDifference += std::fabs(static_cast<double>(dryLeft[sample] - cueLeft[sample]));
    }
    expect(gridDifference > 1.0e-3,
           "grid division changes the ongoing rhythmic field, not only event scheduling");
}

void testSceneRecallPreservesLiveModuleDrawing() {
    const auto territory = ecologyTerritory();
    const auto source = ecologyScene(29U, 0.42);
    auto target = source;
    target.id = 30U;
    target.session.currentSceneId = 30U;
    target.session.targetSceneId = 30U;
    target.session.density = 0.84;
    target.session.space = 0.82;
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 64U, 2U)
               && engine.setScene(source, territory),
           "scene recall preservation fixture prepares");
    expect(engine.setActorPosition(11U, 0.19, 0.77)
               && engine.setActorPosition(12U, 0.73, 0.23)
               && engine.setActorPosition(13U, 0.54, 0.66),
           "performer drawing enters the live ecology");
    const auto first = engine.actorPosition(11U);
    const auto second = engine.actorPosition(12U);
    const auto third = engine.actorPosition(13U);
    expect(engine.beginSceneMorph(source, target, territory, 0.05),
           "scene morph queues without replacing the live drawing");
    std::array<float, 64> left{}, right{};
    for (int block = 0; block < 96; ++block)
        engine.processBlock(left.data(), right.data(), 64U);
    expect(engine.actorPosition(11U).x == first.x && engine.actorPosition(11U).y == first.y
               && engine.actorPosition(12U).x == second.x && engine.actorPosition(12U).y == second.y
               && engine.actorPosition(13U).x == third.x && engine.actorPosition(13U).y == third.y,
           "scene recall preserves every live module position");
}

void testSkyWebReachesTelemetry() {
    // The Constellation draws the sky web from telemetry: while playing, the
    // focused relations (caller, responder, act) and each star's brightness
    // must arrive decodable and in range.
    const auto document = PerformanceTerritoryFactory::makeForReleasedStems(3570930745U, 8U, true);
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 512U, 2U) && engine.setScene(document.scenes[0], document.territory),
           "sky telemetry engine prepares");
    std::array<float, 512> left{}, right{};
    bool sawFocus = false;
    bool sawGrouping = false;
    for (int block = 0; block < 94 * 10; ++block) {
        engine.processBlock(left.data(), right.data(), 512U);
        const auto telemetry = engine.readTelemetry();
        const auto count = telemetry.skyFocusPacked & 3U;
        for (std::uint32_t index = 0U; index < count; ++index) {
            const auto entry = (telemetry.skyFocusPacked >> (2U + index * 10U)) & 0x3ffU;
            expect((entry & 7U) != ((entry >> 3U) & 7U), "a focused relation joins two different stars");
            expect(((entry >> 6U) & 15U) < static_cast<std::uint32_t>(RelationKind::Count),
                   "the focused act is a known relation kind");
            sawFocus = true;
        }
        for (const auto brightness : telemetry.starBrightnessMicros)
            expect(brightness <= 1000000U, "star brightness stays in range");
        if (telemetry.skyGroupingMask != 0U) {
            expect((telemetry.skyGroupingMask & 1U) != 0U, "the bar grouping starts on the downbeat");
            sawGrouping = true;
        }
        expect(telemetry.skyDriftBody == 255U
                   || telemetry.skyDriftBody < static_cast<std::uint8_t>(PerformanceModuleBody::Count),
               "the drifting body is a known module or none");
        expect(telemetry.skyEclipseBody == 255U
                   || telemetry.skyEclipseBody < static_cast<std::uint8_t>(PerformanceModuleBody::Count),
               "the eclipsed body is a known module or none");
    }
    expect(sawFocus, "the sky web reaches telemetry while playing");
    expect(sawGrouping, "the bar grouping reaches telemetry while counting");
}

void testFractureTearsTheStruckStem() {
    // Since 2026-10-29 the fracture level (scar + rupture) tears the stem of
    // the star struck by RASGAR, with the sky web on. Counting holes in the
    // audio proved a lottery for a sparse star (it may simply not play in the
    // window), so the test counts the cuts the tear itself opens: the struck
    // twin gets cuts while the fracture is high; the untouched twin, none.
    const auto document = PerformanceTerritoryFactory::makeForReleasedStems(3570930745U, 8U, true);
    const auto run = [&](bool tear) {
        PerformanceSessionEngine engine;
        expect(engine.prepare(48000.0, 512U, 2U) && engine.setScene(document.scenes[0], document.territory),
               "fracture engine prepares");
        engine.setSkyInfluence(1.0);
        std::array<float, 512> left{}, right{};
        for (int block = 0; block < 94 * 2; ++block) engine.processBlock(left.data(), right.data(), 512U);
        if (tear) {
            auto event = gesture(GestureType::Rasgar, 0U, 0.95);
            event.targetActorId = engine.readTelemetry().stemActorIds[0];
            expect(engine.pushGesture(event), "targeted Rasgar queues");
        }
        for (int block = 0; block < 94 * 2; ++block) engine.processBlock(left.data(), right.data(), 512U);
        return engine.tearCuts();
    };
    const auto untouched = run(false);
    const auto torn = run(true);
    std::cout << "fracture: " << torn << " cuts opened in the torn stem, " << untouched << " untouched\n";
    expect(untouched == 0U, "without a tear there are no cuts");
    expect(torn >= 3U, "the fracture tears the struck stem");
}

void testMasterStageReachesAlbumLevelSafely() {
    // The master (30/10, SAIDA_AUDIO_COMUM.md): fixed gain staging (+35 dB
    // since 5/11, no automatic normalisation of a live performance) and a ceiling that a
    // hot signal never passes.
    PerformanceMasterStage master;
    master.prepare(48000.0);
    master.setMasterGain(1.0);
    double quietPeak = 0.0;
    for (int sample = 0; sample < 48000 * 2; ++sample) {
        double left = 0.005 * std::sin(sample * 0.0573), right = left;
        master.process(left, right);
        if (sample > 48000) quietPeak = std::max(quietPeak, std::fabs(left));
    }
    std::cout << "master: a -46 dBFS sine leaves at peak " << quietPeak << " (fixed +35 dB)\n";
    expect(std::fabs(quietPeak - 0.005 * master.makeupGain()) < 0.002, "the master applies its fixed gain, no more");
    double hottest = 0.0;
    for (int sample = 0; sample < 48000 * 5; ++sample) {
        double left = 3.0 * std::sin(sample * 0.0573) + ((sample % 4801) == 0 ? 6.0 : 0.0), right = -left;
        master.process(left, right);
        hottest = std::max({hottest, std::fabs(left), std::fabs(right)});
    }
    expect(hottest <= PerformanceMasterStage::kCeiling + 1e-9, "the master never passes the ceiling");
    // Intersample peaks: a sine at fs/4 with a 45-degree phase has samples
    // at 0.707 of its true peak. Driven far above the ceiling, the master's
    // true-peak limiter keeps even the peaks between samples under it.
    PerformanceMasterStage hot;
    hot.prepare(48000.0);
    PerformanceTruePeakDetector check;
    double worstTrue = 0.0;
    for (int sample = 0; sample < 48000 * 3; ++sample) {
        double left = 2.0 * std::sin(3.141592653589793 * 0.5 * sample + 0.785398), right = left;
        hot.process(left, right);
        // The check detector runs from the start (a detector started mid-
        // signal rings on the jump from silence and reads a false peak).
        const double truePeak = check.process(left);
        if (sample > 48000) worstTrue = std::max(worstTrue, truePeak);
    }
    std::cout << "master: intersample sine true peak " << 20.0 * std::log10(worstTrue) << " dBTP\n";
    expect(worstTrue <= PerformanceMasterStage::kCeiling * 1.02, "true peaks stay under the ceiling");
}

void testLoudnessMeterMatchesTheStandard() {
    // The meter (30/10) against EBU Tech 3341, case 1: a 1 kHz sine at
    // -23 dBFS on both channels measures -23.0 LUFS; the true peak of a sine
    // is its own peak.
    PerformanceLoudnessMeter meter;
    meter.prepare(48000.0);
    const double amplitude = std::pow(10.0, -23.0 / 20.0);
    for (int sample = 0; sample < 48000 * 5; ++sample) {
        const double value = amplitude * std::sin(2.0 * 3.141592653589793 * 1000.0 * sample / 48000.0);
        meter.process(value, value);
    }
    std::cout << "meter: 1 kHz at -23 dBFS reads " << meter.shortTerm() << " LUFS, true peak "
              << 20.0 * std::log10(meter.truePeakLinear()) << " dBTP\n";
    expect(std::fabs(meter.shortTerm() + 23.0) < 0.2, "the meter reads -23 LUFS for the EBU 3341 tone");
    expect(std::fabs(20.0 * std::log10(meter.truePeakLinear()) + 23.0) < 0.3, "true peak of a sine is its peak");
    // Integrated, gated: the same tone over 5 s integrates to -23 LUFS; a long
    // silence after it does not pull the integrated value down (absolute gate).
    std::cout << "meter: integrated " << meter.integrated() << " LUFS\n";
    expect(std::fabs(meter.integrated() + 23.0) < 0.2, "the integrated loudness of the tone is -23 LUFS");
    for (int sample = 0; sample < 48000 * 10; ++sample) meter.process(0.0, 0.0);
    expect(std::fabs(meter.integrated() + 23.0) < 0.2, "silence is gated out of the integrated loudness");
}

void testPerformerSwapsAStar() {
    // Module management (30/10): the performer puts a library star (the Tuba)
    // in a body; the body plays it and its stem sounds; back to the seed's
    // star restores the original; a body can't take a star the sky can't play.
    const auto document = PerformanceTerritoryFactory::makeForReleasedStems(181U, 8U, true);
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 512U, 2U) && engine.setScene(document.scenes[0], document.territory),
           "swap engine prepares");
    std::array<float, 512> left{}, right{};
    engine.processBlock(left.data(), right.data(), 512U);
    const auto actor = engine.readTelemetry().stemActorIds[0];
    const auto seedStar = engine.actorStar(actor);
    expect(engine.setActorStar(actor, PerformanceModuleBody::Tuba), "a body takes the Tuba");
    expect(engine.actorStar(actor) == PerformanceModuleBody::Tuba, "the body now plays the Tuba");
    std::array<float*, kPerformanceStemActorCapacity> taps{};
    std::array<std::array<float, 512>, kPerformanceStemActorCapacity> buffers{};
    for (std::size_t index = 0U; index < taps.size(); ++index) taps[index] = buffers[index].data();
    engine.setStemTap(taps.data(), taps.size(), 512U);
    double energy = 0.0;
    for (int block = 0; block < 94 * 3; ++block) {
        engine.processBlock(left.data(), right.data(), 512U);
        for (const auto sample : buffers[0]) energy += static_cast<double>(sample) * sample;
    }
    expect(energy > 1.0e-6, "the swapped-in Tuba sounds in its body's stem");
    expect(!engine.setActorStar(actor, static_cast<PerformanceModuleBody>(250)),
           "a body can't take a star the sky can't play");
    expect(engine.setActorStar(actor, PerformanceModuleBody::Count) && engine.actorStar(actor) == seedStar,
           "back to the seed's star restores the original");

    // Body 5 (30/10): the Modal used to overwrite actor 5's stem whatever its
    // star, so a star swapped into body 5 was silent. Now the Modal follows
    // the body whose star it is: body 5 plays the Tuba, and the Modal sounds
    // from body 1 when the performer moves it there.
    const auto telemetry = engine.readTelemetry();
    std::size_t fifth = kPerformanceStemActorCapacity;
    for (std::size_t index = 0U; index < kPerformanceStemActorCapacity; ++index)
        if (telemetry.stemActorIds[index] == 5U) fifth = index;
    expect(fifth < kPerformanceStemActorCapacity && engine.actorStar(5U) == PerformanceModuleBody::ModalResonator,
           "body 5 starts as the Modal");
    expect(engine.setActorStar(5U, PerformanceModuleBody::Tuba)
               && engine.setActorStar(actor, PerformanceModuleBody::ModalResonator),
           "the Tuba goes to body 5 and the Modal to body 1");
    double fifthTuba = 0.0, firstModal = 0.0;
    for (int block = 0; block < 94 * 4; ++block) {
        engine.processBlock(left.data(), right.data(), 512U);
        for (const auto sample : buffers[fifth]) fifthTuba += static_cast<double>(sample) * sample;
        for (const auto sample : buffers[0]) firstModal += static_cast<double>(sample) * sample;
    }
    expect(fifthTuba > 1.0e-6, "the Tuba swapped into body 5 sounds");
    expect(firstModal > 1.0e-8, "the Modal moved to body 1 sounds there");
    expect(engine.setActorStar(5U, PerformanceModuleBody::Count)
               && engine.setActorStar(actor, PerformanceModuleBody::Count)
               && engine.actorStar(5U) == PerformanceModuleBody::ModalResonator,
           "both bodies go back to the seed's stars");
}

void testSceneRecallKeepsTheSkyForm() {
    // 2/11: recalling a scene re-applied the territory and reseeded the sky
    // director, sending the form back to OPENING — the PERCURSO got stuck
    // between opening and development, and a manual scene click did the
    // same. With the same seed, a recall must leave the form where it is; a
    // new territory (even with the same seed) still starts over.
    const auto document = PerformanceTerritoryFactory::makeForReleasedStems(181U, 8U, true);
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 512U, 2U) && engine.setScene(document.scenes[0], document.territory),
           "form engine prepares");
    engine.setSkyInfluence(1.0);
    expect(engine.setFormPlan({{2U, 40U, 40U, 40U}}), "a short opening is planned");
    std::array<float, 512> left{}, right{};
    auto section = [&] { return engine.readTelemetry().skySection; };
    int blocks = 0;
    while (section() != 1U && blocks < 94 * 30) { engine.processBlock(left.data(), right.data(), 512U); ++blocks; }
    expect(section() == 1U, "the form reaches DEVELOPMENT");
    expect(engine.recallScene(document.scenes[1], document.territory), "another scene is recalled");
    for (int block = 0; block < 94; ++block) engine.processBlock(left.data(), right.data(), 512U);
    expect(section() == 1U, "a scene recall keeps the sky's form (no jump back to OPENING)");
    expect(engine.setTerritoryScene(document.scenes[0], document.territory), "the same territory is installed again");
    for (int block = 0; block < 8; ++block) engine.processBlock(left.data(), right.data(), 512U);
    expect(section() == 0U, "a new territory starts the piece over");
}

void testSkyAnchorsSeparateTheStars() {
    // The engine places actors 4-8 all at (0.50, 0.78) until dragged; the
    // screen draws them apart. Sky anchors give the physics the drawn places
    // without moving the ecology: the anchored bodies settle near their own
    // places, no longer stacked.
    const auto document = PerformanceTerritoryFactory::makeForReleasedStems(2876238397U, 8U, true);
    PerformanceSessionEngine engine;
    expect(engine.prepare(48000.0, 512U, 2U) && engine.setScene(document.scenes[0], document.territory),
           "anchor engine prepares");
    engine.setSkyInfluence(1.0);
    const std::array<std::array<double, 2>, 8> drawn{{{0.16, 0.48}, {0.50, 0.28}, {0.84, 0.58}, {0.50, 0.78},
                                                      {0.66, 0.66}, {0.33, 0.69}, {0.73, 0.38}, {0.42, 0.54}}};
    std::array<float, 512> left{}, right{};
    engine.processBlock(left.data(), right.data(), 512U);
    const auto ids = engine.readTelemetry().stemActorIds;
    for (std::size_t slot = 0U; slot < drawn.size(); ++slot)
        expect(engine.setSkyAnchor(ids[slot], drawn[slot][0], drawn[slot][1]), "a sky anchor is accepted");
    for (int block = 0; block < 94 * 3; ++block) engine.processBlock(left.data(), right.data(), 512U);
    const auto& director = engine.skyDirector();
    double closest = 1.0;
    for (std::size_t a = 3U; a < 8U; ++a)
        for (std::size_t b = a + 1U; b < 8U; ++b)
            closest = std::min(closest, std::hypot(director.positionX(a) - director.positionX(b),
                                                   director.positionY(a) - director.positionY(b)));
    std::cout << "anchored stars 4-8: closest pair " << closest << '\n';
    expect(closest > 0.06, "anchored stars are no longer stacked");
    expect(std::hypot(director.positionX(4U) - 0.66, director.positionY(4U) - 0.66) < 0.3,
           "an anchored star dances around its drawn place");
}

void testSeedTrajectoryReachesModuleAudio() {
    // Since 2026-10-28 the modules are the sky's stars (PerformanceSky).
    PerformanceSky first;
    PerformanceSky second;
    PerformanceSeedTrajectory firstTrajectory;
    PerformanceSeedTrajectory secondTrajectory;
    expect(first.prepare(48000.0) && second.prepare(48000.0),
           "skies prepare for seed/audio dialogue");
    firstTrajectory.configure(181U);
    secondTrajectory.configure(182U);
    first.setSeed(181U);
    second.setSeed(182U);
    first.setSeedEvent(firstTrajectory.event(0U));
    second.setSeedEvent(secondTrajectory.event(0U));
    PerformanceSkyContext context;
    context.density = 0.58;
    context.tension = 0.46;
    double difference = 0.0;
    for (int sample = 0; sample < 4096; ++sample) {
        first.advance(context);
        second.advance(context);
        for (const auto star : {PerformanceModuleBody::Acid, PerformanceModuleBody::Gendyn,
                                PerformanceModuleBody::Percussion})
            difference += std::fabs(first.renderStar(star, 110.0) - second.renderStar(star, 110.0));
    }
    expect(!PerformanceSky::available() || difference > 1.0e-3,
           "different seeds reach the audible module bodies");
}

} // namespace

// Auditoria da seed, item B (5/11): a seed liga e escolhe os fenômenos dos
// cabos e o fenômeno de população das cenas 2–4 — não mais todo cabo ATRAIR e
// cada cena com o mesmo fenômeno.
void testSeedChoosesPhenomena() {
    std::array<bool, static_cast<std::size_t>(PerformanceRelationalPhenomenon::Count)> seenRelational{};
    std::array<bool, static_cast<std::size_t>(PerformancePopulationPhenomenon::Count)> seenPopulation{};
    std::size_t enabled = 0U, total = 0U;
    for (std::uint32_t seed = 1U; seed <= 24U; ++seed) {
        const auto document = PerformanceTerritoryFactory::makeForReleasedStems(seed, kPerformanceSafeBodyLimit, true);
        expect(document.valid(), "a seeded document with phenomena stays valid");
        for (std::size_t scene = 0U; scene < document.sceneCount; ++scene) {
            const auto& constellation = document.scenes[scene].constellation;
            for (std::size_t index = 0U; index < constellation.relationCount; ++index) {
                ++total;
                if (!constellation.relations[index].phenomenonEnabled) continue;
                ++enabled;
                seenRelational[static_cast<std::size_t>(constellation.relations[index].phenomenon)] = true;
            }
            if (scene > 0U && document.scenes[scene].session.populationPhenomenonEnabled)
                seenPopulation[static_cast<std::size_t>(document.scenes[scene].session.populationPhenomenon)] = true;
        }
    }
    const auto count = [](const auto& seen) { return static_cast<std::size_t>(std::count(seen.begin(), seen.end(), true)); };
    std::cout << "phenomena: " << enabled << "/" << total << " cables enabled, " << count(seenRelational)
              << " relational and " << count(seenPopulation) << " population phenomena across 24 seeds\n";
    expect(enabled > total / 4U && enabled < total, "the seed enables a part of the cables, not all nor none");
    expect(count(seenRelational) >= 15U, "the seed reaches most of the 23 relational phenomena");
    expect(count(seenPopulation) >= 7U, "the seed reaches most of the 10 population phenomena");
}

int main() {
    testSeedChoosesPhenomena();
    testLiveStemReconstruction();
    testLiveStemMixer();
    testEngineLifecycleAndGestures();
    testMorceauTransportKeepsCodaAndStopDistinct();
    testMorceauContinuesAfterCodaWithoutStop();
    testSeedNarrativeMakesCodaEligibleAfterPerformerPause();
    testEcologyAndSceneMorph();
    testEveryMacroHasAudibleCausality();
    testCreativeGesturesHaveAudibleCausalityAndKeepPlaying();
    testNamedScenesProduceDistinctMaterial();
    testNamedScenesNeverInterruptAnActiveMorceau();
#if RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT >= 8
    // The eight-body path is a calibration build, never the production host.
    testExpandedModulesHaveWorkingLocalControls();
#endif
    testTerritoryIdentityHasAudibleCausality();
    testRasgarLeavesRecoverableScar();
    testRecallHasAudibleRepairWhenClean();
    testHoldHasAudibleConcentrationAndRelease();
    testQuantizedGesturesRespectSafetyBoundary();
    testGestureGridDivisions();
    testSpaceWidthAndSilenceGesture();
    testEveryDeclaredRelationHasAudibleCausality();
    testRelationalPhenomenaComposeDifferentAudioWithoutRedrawingAnchors();
    testMemoryRelationColoursModalMatter();
    testLifeStateShapesStemAudibility();
    testWildTokensChangeTheSeedTrajectory();
    testFocusedWildGesturePrioritizesTheSelectedStem();
    testSeedEcologiesReachTheCompleteEngine();
    testWalshCanBeAwakenedWithoutTransportReset();
    testWalshSpatialCouplingChangesTrajectory();
    testAutonomousGameWaitsForPauseAndPreservesPosition();
    testAutonomousGameHonoursImportedTerritoryPlan();
    testAutonomousStoryDoesNotRequireAutonomyDominance();
    testAutonomousGameReachesAudioDifferentlyFromTwoFields();
    testMotionFieldsReachAudioWithoutMovingTheDesk();
    testMotionFieldChangeIsImmediateAndPreservesTheDesk();
    testSpaceMacroControlsBoundedMotionRate();
    testBodyMotionFieldsCanDivergeWithoutChangingAnchors();
    testCapturedPopulationPhenomenonReturnsToAudioWithoutMovingTheDesk();
    testSeedCollageReachesAudioWithoutChangingThePrimaryDesk();
    testActorSeedVariationReachesOnlyItsStemWithoutChangingTheDesk();
    testInternalModuleSeedVariationReachesFoldedStemWithoutAddingBodies();
    testRecutsChangeAudioWithoutChangingTheDeskOrStoredTempo();
    testGridCueIsImmediateWithoutMovingTheDesk();
    testSceneRecallPreservesLiveModuleDrawing();
    testSeedTrajectoryReachesModuleAudio();
    testSkyWebReachesTelemetry();
    testFractureTearsTheStruckStem();
    testSkyAnchorsSeparateTheStars();
    testPerformerSwapsAStar();
    testSceneRecallKeepsTheSkyForm();
    testMasterStageReachesAlbumLevelSafely();
    testLoudnessMeterMatchesTheStandard();
    std::cout << "rasgo_synth_performance_engine_tests: OK\n";
    return 0;
}
