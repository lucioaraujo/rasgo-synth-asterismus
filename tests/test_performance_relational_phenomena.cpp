#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include <rasgo/performance/PerformanceActorEcology.hpp>
#include <rasgo/performance/PerformanceContracts.hpp>
#include <rasgo/performance/PerformanceRelationalPhenomena.hpp>

namespace {

using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

PerformanceScene relationalScene() {
    PerformanceScene scene;
    scene.id = 1U;
    scene.session.territorySeed = 181U;
    scene.session.currentSceneId = 1U;
    scene.session.targetSceneId = 1U;
    scene.session.playing = true;
    scene.session.density = 0.62;
    scene.session.tension = 0.56;
    expect(scene.constellation.addActor({11U, PerformanceRole::Chao,
                                         ActorLifeState::Active,
                                         0.78, 0.68, 0.52, 0.80}),
           "source actor enters relational scene");
    expect(scene.constellation.addActor({12U, PerformanceRole::Atrito,
                                         ActorLifeState::Active,
                                         0.70, 0.62, 0.48, 0.76}),
           "destination actor enters relational scene");
    expect(scene.constellation.addRelation({11U, 12U, RelationKind::Friction,
                                             RelationDomain::Matter,
                                             0.86, 12.0, 2.0, 0.82}),
           "musical cable enters relational scene");
    return scene;
}

double differenceBetween(PerformanceRelationalPhenomenon first,
                         PerformanceRelationalPhenomenon second) {
    const auto scene = relationalScene();
    const auto morph = PerformanceSceneMorpher::morph(scene, scene, 1.0);
    PerformanceActorEcology left;
    PerformanceActorEcology right;
    expect(left.prepare(48000.0, 64U, 2U) && right.prepare(48000.0, 64U, 2U),
           "comparison ecologies prepare");
    left.setMorph(morph);
    right.setMorph(morph);
    left.setMacroState(scene.session);
    right.setMacroState(scene.session);
    expect(left.setRelationPhenomenon(11U, 12U, first)
               && right.setRelationPhenomenon(11U, 12U, second),
           "comparison phenomena enter the same cable");
    const auto leftAnchor = left.actorPosition(11U);
    const auto rightAnchor = right.actorPosition(11U);
    double difference = 0.0;
    for (int sample = 0; sample < 8192; ++sample) {
        const auto a = left.renderStems(48000.0, 0.10, 0.14).mixedMono();
        const auto b = right.renderStems(48000.0, 0.10, 0.14).mixedMono();
        difference += std::fabs(a - b);
    }
    expect(left.actorPosition(11U).x == leftAnchor.x
               && left.actorPosition(11U).y == leftAnchor.y
               && right.actorPosition(11U).x == rightAnchor.x
               && right.actorPosition(11U).y == rightAnchor.y,
           "phenomena preserve the authored anchors");
    return difference;
}

void testAudibleCompositionalContrasts() {
    expect(differenceBetween(PerformanceRelationalPhenomenon::Attract,
                             PerformanceRelationalPhenomenon::Repel) > 1.0e-3,
           "attraction and repulsion are audibly different");
    expect(differenceBetween(PerformanceRelationalPhenomenon::Orbit,
                             PerformanceRelationalPhenomenon::Merge) > 1.0e-3,
           "orbit and fusion are audibly different");
    expect(differenceBetween(PerformanceRelationalPhenomenon::Synchronize,
                             PerformanceRelationalPhenomenon::Scatter) > 1.0e-3,
           "synchronization and scattering are audibly different");
    expect(differenceBetween(PerformanceRelationalPhenomenon::EllipticalOrbit,
                             PerformanceRelationalPhenomenon::BarycentricDance) > 1.0e-3,
           "elliptical orbit and barycentric dance are audibly different");
}

void testEveryPairOperatorHasAnAudibleSignature() {
    constexpr std::array operators{
        PerformanceRelationalPhenomenon::Repel,
        PerformanceRelationalPhenomenon::Orbit,
        PerformanceRelationalPhenomenon::EllipticalOrbit,
        PerformanceRelationalPhenomenon::BarycentricDance,
        PerformanceRelationalPhenomenon::Encounter,
        PerformanceRelationalPhenomenon::Merge,
        PerformanceRelationalPhenomenon::Flyby,
        PerformanceRelationalPhenomenon::Scatter,
        PerformanceRelationalPhenomenon::Chase,
        PerformanceRelationalPhenomenon::Escape,
        PerformanceRelationalPhenomenon::Parallel,
        PerformanceRelationalPhenomenon::Converge,
        PerformanceRelationalPhenomenon::Diverge,
        PerformanceRelationalPhenomenon::Oscillate,
        PerformanceRelationalPhenomenon::SpiralIn,
        PerformanceRelationalPhenomenon::SpiralOut,
        PerformanceRelationalPhenomenon::Equilibrium,
        PerformanceRelationalPhenomenon::Lock,
        PerformanceRelationalPhenomenon::Synchronize,
        PerformanceRelationalPhenomenon::Resonate,
        PerformanceRelationalPhenomenon::Swap,
        PerformanceRelationalPhenomenon::Release};
    for (const auto phenomenon : operators)
        expect(differenceBetween(PerformanceRelationalPhenomenon::Attract, phenomenon) > 1.0e-4,
               "every relational operator produces a non-zero audible signature");
}

void testPopulationPhenomenaComposeAudioWithoutRewritingTheField() {
    const auto scene = relationalScene();
    const auto morph = PerformanceSceneMorpher::morph(scene, scene, 1.0);
    PerformanceActorEcology cluster;
    PerformanceActorEcology scatter;
    expect(cluster.prepare(48000.0, 64U, 2U) && scatter.prepare(48000.0, 64U, 2U),
           "population comparison ecologies prepare");
    cluster.setMorph(morph);
    scatter.setMorph(morph);
    cluster.setMacroState(scene.session);
    scatter.setMacroState(scene.session);
    expect(cluster.setPopulationPhenomenon(PerformancePopulationPhenomenon::Cluster, 0.78)
               && scatter.setPopulationPhenomenon(PerformancePopulationPhenomenon::Scatter, 0.78),
           "population phenomena enter the collective field");
    const auto clusterAnchor = cluster.actorPosition(11U);
    const auto scatterAnchor = scatter.actorPosition(11U);
    double difference = 0.0;
    for (int sample = 0; sample < 8192; ++sample) {
        const auto a = cluster.renderStems(48000.0, 0.10, 0.14).mixedMono();
        const auto b = scatter.renderStems(48000.0, 0.10, 0.14).mixedMono();
        difference += std::fabs(a - b);
    }
    expect(difference > 5.0,
           "cluster and scatter generate a clearly separated collective audio trajectory");
    expect(cluster.actorPosition(11U).x == clusterAnchor.x
               && cluster.actorPosition(11U).y == clusterAnchor.y
               && scatter.actorPosition(11U).x == scatterAnchor.x
               && scatter.actorPosition(11U).y == scatterAnchor.y,
           "population phenomena preserve the musician's anchors");
}

void testEveryPopulationPhenomenonHasPerformativeContrast() {
    const auto scene = relationalScene();
    const auto morph = PerformanceSceneMorpher::morph(scene, scene, 1.0);
    for (std::size_t value = 0U;
         value < static_cast<std::size_t>(PerformancePopulationPhenomenon::Count);
         ++value) {
        PerformanceActorEcology dry;
        PerformanceActorEcology shaped;
        expect(dry.prepare(48000.0, 64U, 2U) && shaped.prepare(48000.0, 64U, 2U),
               "population contrast ecologies prepare");
        dry.setMorph(morph);
        shaped.setMorph(morph);
        dry.setMacroState(scene.session);
        shaped.setMacroState(scene.session);
        const auto phenomenon = static_cast<PerformancePopulationPhenomenon>(value);
        expect(shaped.setPopulationPhenomenon(phenomenon, 0.82),
               "population phenomenon enters the audible field");
        double dryMagnitude = 0.0;
        double difference = 0.0;
        for (int sample = 0; sample < 16384; ++sample) {
            const auto plain = dry.renderStems(48000.0, 0.10, 0.14).mixedMono();
            const auto transformed = shaped.renderStems(48000.0, 0.10, 0.14).mixedMono();
            dryMagnitude += std::fabs(plain);
            difference += std::fabs(plain - transformed);
        }
        expect(difference > std::max(0.50, dryMagnitude * 0.08),
               "every EIXO phenomenon exceeds a microscopic numerical difference");
    }
}

} // namespace

int main() {
    testAudibleCompositionalContrasts();
    testEveryPairOperatorHasAnAudibleSignature();
    testPopulationPhenomenaComposeAudioWithoutRewritingTheField();
    testEveryPopulationPhenomenonHasPerformativeContrast();
    std::cout << "rasgo_synth_performance_relational_phenomena_tests: OK\n";
    return 0;
}
