#include <array>
#include <cstdlib>
#include <iostream>

#include <rasgo/performance/PerformanceTerritoryFactory.hpp>

namespace {
using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testDeterministicTerritories() {
    const auto first = PerformanceTerritoryFactory::make(181U);
    const auto repeat = PerformanceTerritoryFactory::make(181U);
    const auto different = PerformanceTerritoryFactory::make(182U);
    expect(first.valid() && repeat.valid() && different.valid(),
           "factory emits complete valid session documents");
    expect(first.sceneCount == 4U && first.territory.actorCount == 3U,
           "each territory has a bounded four-scene three-actor starting ecology");
    expect(first.scenes[0].constellation.relationCount == 3U
           && first.scenes[0].constellation.relations[2].kind == RelationKind::Orbit
           && first.scenes[0].constellation.relations[2].domain == RelationDomain::Space,
           "each territory declares a third audible spatial orbit relation");
    const auto& oddOpening = first.scenes[0].constellation;
    const auto& evenOpening = different.scenes[0].constellation;
    expect(oddOpening.actors[0].presence > 0.0 && oddOpening.actors[1].presence > 0.0
           && oddOpening.actors[2].presence > 0.0
           && evenOpening.actors[0].presence > 0.0 && evenOpening.actors[1].presence > 0.0
           && evenOpening.actors[2].presence > 0.0,
           "a seed opens its complete three-body coorte as audible material, not a silent reserve");
    expect(first.territory.gamePlan.valid() && different.territory.gamePlan.valid()
           && (first.territory.gamePlan.strategy != different.territory.gamePlan.strategy
               || first.territory.gamePlan.rule != different.territory.gamePlan.rule
               || first.territory.gamePlan.energy != different.territory.gamePlan.energy),
           "each direct Performance seed supplies a deterministic initial game plan");
    expect(serializeSessionDocument(first) == serializeSessionDocument(repeat),
           "the same seed recreates the same territory exactly");
    expect(serializeSessionDocument(first) != serializeSessionDocument(different),
           "a different seed changes the playable territory");
    for (std::size_t index = 0U; index < first.sceneCount; ++index) {
        expect(first.scenes[index].session.territorySeed == 181U
               && !first.scenes[index].session.playing,
               "new territories are prepared stopped for a safe explicit entry");
        expect(first.scenes[index].session.memory <= first.territory.identity.memoryLimit
               && first.scenes[index].session.risk <= first.territory.identity.riskLimit,
               "seed identity constrains each scene memory and risk budget");
    }
}

void testPlayingIntent() {
    const auto document = PerformanceTerritoryFactory::make(31337U, true);
    expect(document.valid() && document.scenes[0].session.playing,
           "caller can explicitly prepare a territory to play at audio start");
}

void testZeroSeedIsARealTerritory() {
    const auto document = PerformanceTerritoryFactory::make(0U);
    expect(document.valid() && document.territory.seed == 0U,
           "zero is a valid reproducible territory seed, not an alias for one");
    for (std::size_t index = 0U; index < document.sceneCount; ++index)
        expect(document.scenes[index].session.territorySeed == 0U,
               "zero remains canonical in every generated scene");
}

void testAccumulatedWalshOpening() {
    const auto document = PerformanceTerritoryFactory::makeWithWalsh(181U);
    expect(document.valid() && document.territory.actorCount == 4U,
           "Walsh accumulation adds a bounded fourth territory potential");
    for (std::size_t index = 0U; index < document.sceneCount; ++index) {
        const auto& constellation = document.scenes[index].constellation;
        const auto* walsh = constellation.findActor(4U);
        expect(constellation.valid(document.territory) && walsh != nullptr
                   && walsh->presence > 0.0
                   && walsh->life == ActorLifeState::Emerging,
               "the fourth matter enters each scene as audible seed material");
    }
}

void testAccumulatedHardSyncOpening() {
    const auto document = PerformanceTerritoryFactory::makeWithWalsh(186U);
    expect(document.valid() && document.territory.actorCount == 4U,
           "alternating Circuit seed prepares the bounded fourth potential");
    for (std::size_t scene = 0U; scene < document.sceneCount; ++scene)
        expect(document.scenes[scene].constellation.findActor(4U) != nullptr,
               "Hard Sync variant keeps a recollectable fourth actor in every scene");
}

void testAccumulatedDigitalOpening() {
    const auto document = PerformanceTerritoryFactory::makeWithOptionalModule(184U);
    expect(document.valid() && document.territory.actorCount == 4U,
           "Digital seed prepares a bounded fourth potential");
    for (std::size_t scene = 0U; scene < document.sceneCount; ++scene) {
        const auto* optional = document.scenes[scene].constellation.findActor(4U);
        expect(optional != nullptr && optional->presence > 0.0
                   && optional->life == ActorLifeState::Emerging,
               "Digital VOSIM enters each scene as audible seed material");
    }
}

void testEveryAdaptedEcologyPreparesOneComplementaryFourthMatter() {
    constexpr std::array<std::uint32_t, 3> seeds{{182U, 183U, 185U}};
    for (const auto seed : seeds) {
        const auto document = PerformanceTerritoryFactory::makeWithOptionalModule(seed);
        expect(document.valid() && document.territory.actorCount == 4U,
               "each adapted ecology declares exactly one bounded fourth potential");
        for (std::size_t scene = 0U; scene < document.sceneCount; ++scene) {
            const auto* optional = document.scenes[scene].constellation.findActor(4U);
            expect(optional != nullptr && optional->presence > 0.0
                       && optional->life == ActorLifeState::Emerging,
                   "complementary matter begins audible without replacing the seed opening");
        }
    }
}

void testModalFifthStemReserve() {
    const auto document = PerformanceTerritoryFactory::makeWithModalReserve(184U);
    expect(document.valid() && document.territory.actorCount == 5U,
           "ModalResonator reserve adds a valid fifth structural actor");
    for (std::size_t scene = 0U; scene < document.sceneCount; ++scene) {
        const auto& constellation = document.scenes[scene].constellation;
        const auto* modal = constellation.findActor(5U);
        expect(modal != nullptr && modal->presence > 0.0
                   && modal->life == ActorLifeState::Emerging
                   && constellation.findRelation({1U, 5U, RelationKind::Shadow,
                                                   RelationDomain::Memory, 0.0, 0.0, 0.0, 1.0}) != nullptr,
               "ModalResonator enters as an emerging body with a declared memory relation");
    }
}

} // namespace

void testEightReleasedStemsPutTheWholeAsterismInTheSky() {
    // The author raised the presence of Galo, Atropelo and Deja Vu and heard
    // nothing: with five bodies they were reserves outside the sound. With
    // eight released stems every body is in the sky from the opening.
    for (const auto seed : {181U, 196U, 4065351394U}) {
        const auto document = PerformanceTerritoryFactory::makeForReleasedStems(seed, 8U, true);
        expect(document.territory.actorCount == 8U, "eight released stems open eight bodies");
        const auto& constellation = document.scenes[0].constellation;
        expect(constellation.actorCount == 8U, "the opening scene holds the eight bodies");
        // Bodies 6..8 begin with the seed's presence, like the others: the
        // mixer variation, not a fixed zero, decides who opens lit.
        for (std::size_t index = 5U; index < 8U; ++index)
            expect(constellation.actors[index].presence > 0.0,
                   "bodies 6..8 begin with a seed presence");
    }
}

int main() {
    testEightReleasedStemsPutTheWholeAsterismInTheSky();
    testDeterministicTerritories();
    testPlayingIntent();
    testZeroSeedIsARealTerritory();
    testAccumulatedWalshOpening();
    testAccumulatedHardSyncOpening();
    testAccumulatedDigitalOpening();
    testEveryAdaptedEcologyPreparesOneComplementaryFourthMatter();
    testModalFifthStemReserve();
    std::cout << "Performance territory factory tests passed\n";
}
