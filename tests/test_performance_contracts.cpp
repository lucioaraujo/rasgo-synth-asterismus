#include <cmath>
#include <cstdlib>
#include <iostream>

#include <rasgo/performance/PerformanceContracts.hpp>
#include <rasgo/performance/PerformanceConstellationGeometry.hpp>
#include <rasgo/performance/PerformanceMotionGeometry.hpp>
#include <rasgo/performance/PerformanceRelationalPhenomena.hpp>

namespace {

using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

bool close(double left, double right) {
    return std::fabs(left - right) < 1.0e-9;
}

PerformanceTerritory makeTerritory() {
    PerformanceTerritory territory;
    territory.seed = 181U;
    territory.identity.riskLimit = 0.68;

    PerformanceActorPotential ground;
    ground.id = 1U;
    ground.family = PerformanceFamily::Tone;
    ground.capabilities = 0x03U;
    ground.allowedRoles = roleBit(PerformanceRole::Chao) | roleBit(PerformanceRole::Corpo)
        | roleBit(PerformanceRole::Vestigio);
    ground.minimumPresence = 0.05;
    expect(territory.addActor(ground), "ground actor enters territory");

    PerformanceActorPotential friction;
    friction.id = 2U;
    friction.family = PerformanceFamily::Texture;
    friction.capabilities = 0x14U;
    friction.allowedRoles = roleBit(PerformanceRole::Atrito)
        | roleBit(PerformanceRole::Contagio) | roleBit(PerformanceRole::Rasgo);
    expect(territory.addActor(friction), "friction actor enters territory");
    return territory;
}

PerformanceConstellation makeConstellation() {
    PerformanceConstellation constellation;
    expect(constellation.addActor({1U, PerformanceRole::Chao, ActorLifeState::Active,
                                   0.70, 0.60, 0.55, 0.80}), "ground actor enters constellation");
    expect(constellation.addActor({2U, PerformanceRole::Atrito, ActorLifeState::Emerging,
                                   0.35, 0.45, 0.30, 0.70}), "friction actor enters constellation");
    expect(constellation.addRelation({1U, 2U, RelationKind::Friction, RelationDomain::Matter,
                                      0.40, 18.0, 2.0, 0.75}), "relation enters constellation");
    return constellation;
}

void testTerritoryAndConstellation() {
    const auto territory = makeTerritory();
    expect(territory.valid(), "territory is valid");

    auto duplicate = territory;
    expect(!duplicate.addActor(duplicate.actors[0]), "duplicate actor is rejected");

    const auto constellation = makeConstellation();
    expect(constellation.valid(territory), "constellation fits territory");

    auto wrongRole = constellation;
    wrongRole.actors[0].role = PerformanceRole::Rasgo;
    expect(!wrongRole.valid(territory), "role not allowed by potential is rejected");

    auto brokenRelation = constellation;
    brokenRelation.relations[0].destinationActorId = 77U;
    expect(!brokenRelation.valid(territory), "relation to absent actor is rejected");
}

void testSessionGesturesQueueAndTelemetry() {
    PerformanceSessionState session;
    session.territorySeed = 181U;
    expect(session.valid(), "default session is bounded");
    session.tempoBpm = 301.0;
    expect(!session.valid(), "tempo upper bound is enforced");

    PerformanceGestureEvent gesture;
    gesture.type = GestureType::Rasgar;
    gesture.sampleTimestamp = 48000U;
    gesture.blockOffset = 63U;
    gesture.targetActorId = 2U;
    gesture.intensity = 0.80;
    gesture.duration = 0.35;
    expect(gesture.valid(64U), "gesture accepts a bounded sample offset");
    gesture.blockOffset = 64U;
    expect(!gesture.valid(64U), "gesture rejects offset outside block");

    FixedEventQueue<PerformanceGestureEvent, 4> queue;
    gesture.blockOffset = 0U;
    expect(queue.push(gesture), "queue accepts first event");
    gesture.sampleTimestamp = 48001U;
    expect(queue.push(gesture), "queue accepts second event");
    gesture.sampleTimestamp = 48002U;
    expect(queue.push(gesture), "queue accepts third event");
    expect(!queue.push(gesture), "queue preserves spare slot instead of overwriting");
    PerformanceGestureEvent popped;
    expect(queue.pop(popped) && popped.sampleTimestamp == 48000U, "queue preserves FIFO order");
    expect(queue.size() == 2U, "queue exposes bounded depth");
    queue.reset();
    expect(queue.size() == 0U && !queue.pop(popped),
           "queue reset clears pending events while the queue is quiescent");

    PerformanceTelemetryMailbox mailbox;
    PerformanceTelemetrySnapshot published;
    published.cpuMilliPercent = 1200U;
    published.outputPeakMicros = 250000U;
    published.truePeakMicros = 300000U;
    published.xrunCount = 2U;
    published.activeActorCount = 2U;
    published.sceneId = 4U;
    published.eventQueueDepth = 2U;
    published.playing = false;
    published.panicSafe = true;
    published.narrativeEventValid = true;
    published.narrativeTurn = 17U;
    published.narrativeMove = 5U;
    published.narrativeStrategy = 3U;
    published.narrativeRule = 5U;
    published.motionPhaseMicros = 731000U;
    published.motionField = static_cast<std::uint8_t>(PerformanceMotionField::FigureEight);
    published.stemCount = 2U;
    published.stemActorIds[0] = 11U;
    published.stemActorIds[1] = 12U;
    published.stemPeakMicros[0] = 180000U;
    published.stemPeakMicros[1] = 90000U;
    published.stemLifeStates[0] = static_cast<std::uint8_t>(ActorLifeState::Active);
    published.stemLifeStates[1] = static_cast<std::uint8_t>(ActorLifeState::Emerging);
    published.stemReadiness[0] = static_cast<std::uint8_t>(PerformanceStemReadiness::Audible);
    published.stemReadiness[1] = static_cast<std::uint8_t>(PerformanceStemReadiness::Silent);
    published.modalReturnPeakMicros = 45000U;
    published.scarMicros = 310000U;
    published.repairMicros = 620000U;
    published.wildMicros = 410000U;
    mailbox.publish(published);
    const auto telemetry = mailbox.snapshot();
    expect(telemetry.valid() && !telemetry.playing && telemetry.panicSafe && telemetry.sceneId == 4U
           && telemetry.stemCount == 2U && telemetry.stemActorIds[1] == 12U
            && telemetry.stemPeakMicros[0] == 180000U && telemetry.modalReturnPeakMicros == 45000U
           && telemetry.stemLifeStates[0] == static_cast<std::uint8_t>(ActorLifeState::Active)
           && telemetry.stemLifeStates[1] == static_cast<std::uint8_t>(ActorLifeState::Emerging)
           && telemetry.stemReadiness[0] == static_cast<std::uint8_t>(PerformanceStemReadiness::Audible)
           && telemetry.stemReadiness[1] == static_cast<std::uint8_t>(PerformanceStemReadiness::Silent)
           && telemetry.scarMicros == 310000U && telemetry.repairMicros == 620000U
           && telemetry.wildMicros == 410000U && telemetry.narrativeEventValid
           && telemetry.narrativeTurn == 17U && telemetry.narrativeMove == 5U
           && telemetry.narrativeStrategy == 3U && telemetry.narrativeRule == 5U
           && telemetry.motionPhaseMicros == 731000U
           && telemetry.motionField == static_cast<std::uint8_t>(PerformanceMotionField::FigureEight),
           "telemetry snapshot preserves safety, trajectory, shared motion and bounded live-stem life states");
    published.playing = true;
    published.panicSafe = false;
    mailbox.publish(published);
    expect(mailbox.snapshot().playing && !mailbox.snapshot().panicSafe,
           "telemetry snapshot preserves transport state");
}

void testSceneMorphAndSharedTerritory() {
    const auto territory = makeTerritory();
    PerformanceScene source;
    source.id = 10U;
    source.session.territorySeed = territory.seed;
    source.session.currentSceneId = source.id;
    source.session.autonomy = 0.80;
    source.session.risk = 0.20;
    source.constellation = makeConstellation();
    expect(source.valid(territory), "source scene is valid");

    auto target = source;
    target.id = 11U;
    target.session.currentSceneId = target.id;
    target.session.targetSceneId = target.id;
    target.session.autonomy = 0.30;
    target.session.risk = 0.90;
    target.constellation.actors[0].role = PerformanceRole::Corpo;
    target.constellation.actors[0].presence = 0.45;
    target.constellation.actors[1].presence = 0.75;
    target.constellation.relations[0].intensity = 0.90;
    expect(target.valid(territory), "target scene shares territory with a new trajectory");

    const auto first = PerformanceSceneMorpher::morph(source, target, 0.5);
    const auto second = PerformanceSceneMorpher::morph(source, target, 0.5);
    expect(first.actorCount == 2U && first.relationCount == 1U, "morph keeps the active constellation legible");
    expect(close(first.session.autonomy, 0.55) && close(first.session.risk, 0.55),
           "macro nutrition fields interpolate deterministically");
    expect(close(first.actors[0].presence, 0.575) && close(first.relations[0].relation.intensity, 0.65),
           "actor presence and relation intensity interpolate");
    expect(first.actors[0].sourceRole == PerformanceRole::Chao
           && first.actors[0].targetRole == PerformanceRole::Corpo
           && close(first.actors[0].roleMorph, 0.5), "role change remains explicit during morph");
    expect(close(first.session.autonomy, second.session.autonomy)
           && close(first.actors[0].presence, second.actors[0].presence),
           "same territory and scenes produce deterministic morph");

    auto anotherNutrition = target;
    anotherNutrition.session.risk = 0.10;
    anotherNutrition.constellation.actors[1].nourishment = 0.10;
    const auto alternate = PerformanceSceneMorpher::morph(source, anotherNutrition, 0.5);
    expect(!close(first.session.risk, alternate.session.risk)
           && !close(first.actors[1].nourishment, alternate.actors[1].nourishment),
           "same seed can sustain a distinct nourished trajectory");

    auto foreignTerritory = target;
    foreignTerritory.session.territorySeed = 999U;
    const auto rejected = PerformanceSceneMorpher::morph(source, foreignTerritory, 0.5);
    expect(!rejected.compatibleTerritory && close(rejected.amount, 0.0)
           && rejected.session.territorySeed == territory.seed,
           "scene morph never crosses into a different territory");
}

void testConstellationRelationProjection() {
    const std::array<double, 3> relations{{0.52, 0.46, 0.36}};
    const auto first = projectConstellationRelations(relations);
    const auto second = projectConstellationRelations(relations);
    for (std::size_t index = 0U; index < first.size(); ++index) {
        expect(close(first[index].x, second[index].x) && close(first[index].y, second[index].y),
               "relation geometry projection is deterministic");
        expect(first[index].x >= 0.04 && first[index].x <= 0.96
               && first[index].y >= 0.08 && first[index].y <= 0.92,
               "projected relation geometry stays inside the interaction field");
    }
    const auto distance = [](const PerformanceConstellationPoint& left,
                             const PerformanceConstellationPoint& right) {
        return std::hypot(left.x - right.x, left.y - right.y);
    };
    expect(close(constellationRelationFromDistance(distance(first[0], first[1])), relations[0])
           && close(constellationRelationFromDistance(distance(first[1], first[2])), relations[1])
           && close(constellationRelationFromDistance(distance(first[0], first[2])), relations[2]),
           "a valid relation triple round-trips through node geometry");

    const auto impossible = projectConstellationRelations({{1.0, 0.0, 1.0}});
    for (const auto& point : impossible)
        expect(std::isfinite(point.x) && std::isfinite(point.y),
               "an impossible distance triple still produces finite best-fit geometry");
}

void testRigidCollectiveOrbitPreservesConstellationDistances() {
    const auto firstAnchor = PerformanceMotionPoint{0.20, 0.32, 0.5};
    const auto secondAnchor = PerformanceMotionPoint{0.76, 0.61, 0.5};
    const auto original = performanceMotionDistance(firstAnchor, secondAnchor);
    for (const auto phase : {0.0, 0.71, 2.36, 4.91}) {
        const auto first = performanceCollectiveMotionPoint(
            firstAnchor.x, firstAnchor.y, 0.50, 0.50, PerformanceMotionField::Orbit, phase);
        const auto second = performanceCollectiveMotionPoint(
            secondAnchor.x, secondAnchor.y, 0.50, 0.50, PerformanceMotionField::Orbit, phase);
        expect(close(performanceMotionDistance(first, second), original),
               "collective orbit is a rigid rotation: constellation distances remain invariant");
    }
}

void testRelationalPhenomenaAreMusicalAndBounded() {
    const PerformanceMotionPoint previousSource{0.20, 0.50, 0.50};
    const PerformanceMotionPoint previousDestination{0.80, 0.50, 0.50};
    const PerformanceMotionPoint source{0.24, 0.48, 0.50};
    const PerformanceMotionPoint destination{0.70, 0.56, 0.50};
    const auto observation = observePerformanceRelation(
        previousSource, previousDestination, source, destination, 0.25);
    expect(observation.valid() && observation.distance < 1.0
               && observation.radialVelocity < 0.0,
           "relation observation exposes bounded distance, angle and relative velocity");

    for (std::size_t index = 0U;
         index < static_cast<std::size_t>(PerformanceRelationalPhenomenon::Count); ++index) {
        const auto phenomenon = static_cast<PerformanceRelationalPhenomenon>(index);
        const auto forces = composeRelationalPhenomenon(
            phenomenon, observation, 1.17, 0.84, 1.5);
        expect(forces.valid(), "every relational phenomenon produces bounded musical forces");
        const auto movement = relationalDisplacement(
            phenomenon, source, destination, 1.17, 1.0);
        expect(std::fabs(movement.sourceX) <= 0.08
                   && std::fabs(movement.sourceY) <= 0.08
                   && std::fabs(movement.destinationX) <= 0.08
                   && std::fabs(movement.destinationY) <= 0.08,
               "relation movement is bounded and cannot replace the authored drawing");
    }
    const auto attractForces = composeRelationalPhenomenon(
        PerformanceRelationalPhenomenon::Attract, observation, 1.17, 0.84);
    const auto repelForces = composeRelationalPhenomenon(
        PerformanceRelationalPhenomenon::Repel, observation, 1.17, 0.84);
    const auto orbitForces = composeRelationalPhenomenon(
        PerformanceRelationalPhenomenon::Orbit, observation, 1.17, 0.84);
    const auto mergeForces = composeRelationalPhenomenon(
        PerformanceRelationalPhenomenon::Merge, observation, 1.17, 0.84);
    const auto syncForces = composeRelationalPhenomenon(
        PerformanceRelationalPhenomenon::Synchronize, observation, 1.17, 0.84);
    const auto releaseForces = composeRelationalPhenomenon(
        PerformanceRelationalPhenomenon::Release, observation, 1.17, 0.84);
    expect(repelForces.dynamicTension > attractForces.dynamicTension
               && orbitForces.spatialCirculation > attractForces.spatialCirculation
               && mergeForces.matterFusion > orbitForces.matterFusion
               && syncForces.pulseCoupling > repelForces.pulseCoupling
               && releaseForces.memoryTrace > attractForces.memoryTrace,
           "core phenomena retain distinct compositional dimensions");

    const auto attraction = relationalDisplacement(
        PerformanceRelationalPhenomenon::Attract, source, destination, 0.0, 1.0);
    const auto repulsion = relationalDisplacement(
        PerformanceRelationalPhenomenon::Repel, source, destination, 0.0, 1.0);
    expect(attraction.sourceX * repulsion.sourceX <= 0.0
               && attraction.destinationX * repulsion.destinationX <= 0.0,
           "attraction and repulsion move the pair in opposite directions");
}

void testPopulationPhenomenaPreserveAuthorshipAndProduceMusicalForces() {
    const PerformanceMotionPoint authored{0.72, 0.44, 0.50};
    const PerformanceMotionPoint centre{0.50, 0.50, 0.50};
    for (std::size_t index = 0U;
         index < static_cast<std::size_t>(PerformancePopulationPhenomenon::Count); ++index) {
        const auto phenomenon = static_cast<PerformancePopulationPhenomenon>(index);
        const auto forces = composePopulationPhenomenon(phenomenon, 1.24, 0.82, 0.66);
        const auto live = populationMotionPoint(
            phenomenon, authored, centre, 1.24, 0.82, 2U, 6U);
        expect(forces.valid(), "every population phenomenon produces bounded musical forces");
        expect(unit(live.x) && unit(live.y) && unit(live.z),
               "every population phenomenon remains inside the live field");
    }
    const auto restored = populationMotionPoint(
        PerformancePopulationPhenomenon::Fragment, authored, centre, 1.24, 0.0, 2U, 6U);
    expect(close(restored.x, authored.x) && close(restored.y, authored.y),
           "zero population intensity restores the musician's authored point");

    const PerformanceMotionPoint first{0.28, 0.40, 0.50};
    const PerformanceMotionPoint second{0.74, 0.63, 0.50};
    const auto originalDistance = performanceMotionDistance(first, second);
    const auto movedFirst = populationMotionPoint(
        PerformancePopulationPhenomenon::CollectiveRotate, first, centre, 2.1, 0.74, 0U, 2U);
    const auto movedSecond = populationMotionPoint(
        PerformancePopulationPhenomenon::CollectiveRotate, second, centre, 2.1, 0.74, 1U, 2U);
    expect(close(performanceMotionDistance(movedFirst, movedSecond), originalDistance),
           "collective population rotation preserves the constellation geometry");
}

void testEightBodyContractCapacity() {
    expect(kPerformanceStemActorCapacity == 8U && kPerformanceSafeBodyLimit == 8U,
           "eight structural slots are eight audible bodies");
    expect(capacityProfileForCpus(2U) == PerformanceCapacityProfile::Compatibility
               && safeBodyLimitForProfile(PerformanceCapacityProfile::Compatibility) == 3U
               && capacityProfileForCpus(4U) == PerformanceCapacityProfile::Performance
               && safeBodyLimitForProfile(PerformanceCapacityProfile::Performance) == 4U,
           "CPU profile selects only a conservative three/four-body floor");
    PerformanceTerritory territory;
    territory.seed = 184U;
    for (std::uint16_t id = 1U; id <= 8U; ++id) {
        PerformanceActorPotential actor;
        actor.id = id;
        actor.family = static_cast<PerformanceFamily>(id % static_cast<std::uint16_t>(PerformanceFamily::Count));
        actor.capabilities = 0x03U + id;
        actor.allowedRoles = allRoles();
        expect(territory.addActor(actor), "eight-body territory accepts each bounded potential");
    }
    expect(territory.valid() && territory.actorCount == 8U,
           "eight-body territory remains valid within the expanded contract capacity");

    PerformanceConstellation constellation;
    for (std::uint16_t id = 1U; id <= 8U; ++id)
        expect(constellation.addActor({id, PerformanceRole::Corpo, ActorLifeState::Active,
                                       0.5, 0.5, 0.5, 0.5}),
               "eight-body constellation accepts each active body");
    expect(constellation.addRelation({1U, 2U, RelationKind::Support, RelationDomain::Matter,
                                      0.4, 0.0, 1.0, 1.0}), "first four-body relation is valid");
    expect(constellation.addRelation({2U, 3U, RelationKind::Friction, RelationDomain::Space,
                                      0.4, 0.0, 1.0, 1.0}), "second four-body relation is valid");
    expect(constellation.addRelation({3U, 4U, RelationKind::Orbit, RelationDomain::Dynamics,
                                      0.4, 0.0, 1.0, 1.0}), "third four-body relation is valid");
    expect(constellation.valid(territory) && constellation.actorCount == 8U,
           "eight-body constellation validates against the territory");
    expect(constellation.removeActor(2U) && constellation.actorCount == 7U
           && constellation.relationCount == 1U && constellation.findActor(2U) == nullptr,
           "recollecting a body removes its relations without invalidating the table");
    expect(constellation.valid(territory), "table remains valid after body recollection");
}

} // namespace

int main() {
    testTerritoryAndConstellation();
    testSessionGesturesQueueAndTelemetry();
    testSceneMorphAndSharedTerritory();
    testConstellationRelationProjection();
    testRigidCollectiveOrbitPreservesConstellationDistances();
    testRelationalPhenomenaAreMusicalAndBounded();
    testPopulationPhenomenaPreserveAuthorshipAndProduceMusicalForces();
    testEightBodyContractCapacity();
    std::cout << "rasgo_synth_performance_contract_tests: OK\n";
    return 0;
}
