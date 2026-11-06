#include <cstdlib>
#include <iostream>

#include <rasgo/performance/PerformanceModuleLibrary.hpp>

namespace {
using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}

void testLibrarySeparatesAudibleCohortsFromPotential() {
    std::size_t active = 0U;
    std::size_t candidates = 0U;
    std::size_t studioOnly = 0U;
    for (const auto& entry : kPerformanceModuleLibrary) {
        active += entry.state == PerformanceLibraryState::ActiveCohort;
        candidates += entry.state == PerformanceLibraryState::RealtimeCandidate;
        studioOnly += entry.state == PerformanceLibraryState::NeedsAudit
            && entry.source.find("Studio OrganismStem::") == 0U;
    }
    // 30/10: the Tuba became a library star (sounding, placed by the
    // performer), leaving thirteen sourced Studio organisms to adapt.
    // 6/11: + AY-8910, MORPH, RÖSSLER, ENVELOPE PULSE (vozes do Studio).
    expect(kPerformanceModuleLibrary.size() == 35U && active == 34U && candidates == 1U
               && studioOnly == 0U,
           "library: thirty-four playable stars, one candidate, every Studio organism adapted or rewritten");
    const auto* flute = findPerformanceModule("fm-flute");
    const auto* walsh = findPerformanceModule("walsh");
    const auto* vosim = findPerformanceModule("vosim");
    const auto* hardSync = findPerformanceModule("hard-sync");
    const auto* atariPunk = findPerformanceModule("atari-punk");
    const auto* dejaVu = findPerformanceModule("deja-vu");
    const auto* atropelo = findPerformanceModule("atropelo");
    const auto* galo = findPerformanceModule("galo");
    const auto* modal = findPerformanceModule("modal-resonator");
    const auto* harmonaig = findPerformanceModule("harmonaig");
    expect(flute != nullptr && flute->state == PerformanceLibraryState::ActiveCohort
           && flute->cohortMask == 2U && vosim != nullptr && vosim->cohortMask == 14U
           && walsh != nullptr && walsh->state == PerformanceLibraryState::ActiveCohort
           && walsh->cohortMask == 5U && hardSync != nullptr && hardSync->cohortMask == 17U
           && atariPunk != nullptr && atariPunk->cohortMask == 10U
           && dejaVu != nullptr && dejaVu->cohortMask == 16U
           && atropelo != nullptr && atropelo->cohortMask == 16U
           && galo != nullptr && galo->cohortMask == 16U
           && modal != nullptr && modal->state == PerformanceLibraryState::RealtimeCandidate
           && modal->cohortMask == 31U,
           "library exposes sounding cohorts and the explicitly gated ModalResonator candidate");
    // Harmonaig e Thermal Static tocam desde 2/11 (reescritos); a origem no
    // Studio segue explícita.
    const auto* thermal = findPerformanceModule("thermal-static");
    expect(harmonaig != nullptr && harmonaig->state == PerformanceLibraryState::ActiveCohort
               && thermal != nullptr && thermal->state == PerformanceLibraryState::ActiveCohort
               && thermal->source == "Studio OrganismStem::ThermalStatic",
           "Studio-only organisms retain explicit provenance instead of appearing as false audio modules");
    expect(performanceModuleLibraryIndex("walsh") == 8U
               && performanceModuleLibraryIndex("hard-sync") == 9U
               && performanceModuleLibraryIndex("not-a-module") == kPerformanceModuleLibrary.size(),
           "library IDs resolve to stable bounded provenance indices");
}
}

int main() {
    testLibrarySeparatesAudibleCohortsFromPotential();
    std::cout << "Performance module library tests passed\n";
    return 0;
}
