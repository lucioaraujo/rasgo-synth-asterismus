#include <cmath>
#include <cstdlib>
#include <iostream>

#include <rasgo/performance/PerformanceActorEcology.hpp>
#include <rasgo/performance/PerformanceTerritoryFactory.hpp>

namespace {
using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testOfflineModalFifthStem() {
    const auto document = PerformanceTerritoryFactory::makeWithModalReserve(184U, true);
    expect(document.valid() && document.territory.actorCount == 5U,
           "offline fixture prepares the fifth ModalResonator actor");

    PerformanceActorEcology ecology;
    expect(ecology.prepare(48000.0, 64U, 2U), "fifth-stem ecology prepares");
    ecology.setTerritoryIdentity(document.territory.identity, document.territory.seed);
    ecology.setMorph(PerformanceSceneMorpher::morph(document.scenes[0], document.scenes[0], 1.0));
    ecology.setMacroState(document.scenes[0].session);
    expect(ecology.voiceCount() == 5U && ecology.setPresence(5U, 0.72)
               && ecology.setModuleControl(5U, PerformanceModuleControl::Primary, 0.84)
               && ecology.setModuleControl(5U, PerformanceModuleControl::Secondary, 0.66)
               && ecology.setRelationIntensity(1U, 5U, 0.78),
           "offline fifth actor accepts presence, modal controls and memory relation");

    double energy = 0.0;
    for (int sample = 0; sample < 8192; ++sample) {
        const auto frame = ecology.renderStems(48000.0, 0.18, 0.48);
        const auto* modal = frame.findActor(5U);
        expect(frame.valid() && modal != nullptr && frame.modalReturn == 0.0,
               "ModalResonator claims its own fifth stem instead of a hidden return");
        energy += modal->mono * modal->mono;
    }
    expect(energy > 1.0e-7, "fifth ModalResonator stem carries bounded audible energy");

    expect(ecology.setStemMuted(5U, true), "fifth stem enters normal mixer mute path");
    double mutedEnergy = 0.0;
    for (int sample = 0; sample < 4096; ++sample) {
        const auto frame = ecology.renderStems(48000.0, 0.0, 0.0);
        const auto* modal = frame.findActor(5U);
        expect(modal != nullptr, "fifth actor remains addressable while muted");
        mutedEnergy += modal->mono * modal->mono;
    }
    expect(mutedEnergy < energy * 0.02,
           "fifth ModalResonator stem mute removes it without disturbing its contract");
}
} // namespace

int main() {
    testOfflineModalFifthStem();
    std::cout << "rasgo_synth_performance_modal_fifth_stem_tests: OK\n";
    return 0;
}
