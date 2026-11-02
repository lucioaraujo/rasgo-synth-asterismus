#include <cstdlib>
#include <cmath>
#include <iostream>

#include <rasgo/performance/PerformanceSeedTrajectory.hpp>

namespace {
using rasgo::performance::PerformanceSeedTrajectory;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

int mainImpl() {
    PerformanceSeedTrajectory first;
    PerformanceSeedTrajectory second;
    PerformanceSeedTrajectory other;
    first.configure(181U);
    second.configure(181U);
    other.configure(182U);
    bool different = false;
    for (std::uint32_t index = 0U; index < PerformanceSeedTrajectory::kEventCapacity; ++index) {
        const auto& a = first.event(index);
        const auto& b = second.event(index);
        expect(a.ordinal == index && a.variation < 8U
                   && (a.pauseBeats == 4U || a.pauseBeats == 6U || a.pauseBeats == 8U),
               "event is bounded, ordered and has a performable pause");
        expect(a.densityBias == b.densityBias && a.tensionBias == b.tensionBias,
               "same seed reproduces the same history");
        different = different || a.variation != other.event(index).variation;
    }
    expect(different, "different seeds produce different histories");
    expect(first.event(32U).ordinal == first.event(0U).ordinal,
           "trajectory wraps on a fixed bounded cycle");
    expect(first.event(0U).pauseBeats == 4U,
           "the opening narrative answer remains prompt at four beats");
    const auto primary = first.event(7U);
    const auto secondary = other.event(7U);
    const auto none = PerformanceSeedTrajectory::collage(primary, secondary, 0.0);
    const auto full = PerformanceSeedTrajectory::collage(primary, secondary, 1.0);
    const auto half = PerformanceSeedTrajectory::collage(primary, secondary, 0.5);
    expect(none.variation == primary.variation && none.pauseBeats == primary.pauseBeats
               && std::fabs(none.densityBias - primary.densityBias) < 1.0e-12
               && std::fabs(none.memoryBias - primary.memoryBias) < 1.0e-12,
           "zero collage preserves the primary seed exactly");
    expect(full.variation == secondary.variation && full.pauseBeats == secondary.pauseBeats
               && std::fabs(full.tensionBias - secondary.tensionBias) < 1.0e-12
               && std::fabs(full.matterBias - secondary.matterBias) < 1.0e-12,
           "full collage reproduces the secondary seed traits");
    expect(half.pauseBeats == secondary.pauseBeats
               && std::fabs(half.densityBias
                    - (primary.densityBias + secondary.densityBias) * 0.5) < 1.0e-12,
           "middle collage has bounded secondary discrete traits and interpolated pressures");
    return 0;
}
}

int main() { return mainImpl(); }
