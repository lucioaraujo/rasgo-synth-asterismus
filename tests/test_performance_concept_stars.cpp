// Thermal Static, Core, Texture e Bass Fallback reescritos
// (PerformanceStudioConceptStars.hpp).
#include <rasgo/performance/PerformanceStudioConceptStars.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace rasgo::performance;

namespace {
bool expect(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}
struct Level { double peak = 0.0, energy = 0.0; bool finite = true; };
template <typename Tick> Level measure(int samples, Tick tick) {
    Level level;
    for (int sample = 0; sample < samples; ++sample) {
        const double value = tick(sample);
        level.finite = level.finite && std::isfinite(value);
        level.peak = std::max(level.peak, std::fabs(value));
        level.energy += value * value;
    }
    return level;
}
} // namespace

int main() {
    bool ok = true;
    constexpr double rate = 48000.0;

    // Thermal: quente ao máximo, vive ~6 s, morre, esfria e volta; com MORTE
    // em 1 toda morte é abrupta; com 0, nenhuma.
    PerformanceThermalStatic hot;
    hot.prepare(rate, 181U);
    const auto thermal = measure(static_cast<int>(rate * 40.0),
                                 [&](int) { return hot.tick(110.0, 1.0, 1.0, 0.8); });
    ok = expect(thermal.finite && thermal.peak < 1.0 && thermal.energy > 10.0, "Thermal Static sounds, bounded") && ok;
    ok = expect(hot.deaths() >= 3U && hot.abruptDeaths() == hot.deaths(), "it lives, dies abruptly and comes back")
        && ok;
    PerformanceThermalStatic slow;
    slow.prepare(rate, 181U);
    measure(static_cast<int>(rate * 40.0), [&](int) { return slow.tick(110.0, 1.0, 0.0, 0.2); });
    ok = expect(slow.deaths() >= 2U && slow.abruptDeaths() == 0U, "a slow death loses harmonics, no cut") && ok;

    PerformanceCoreVoice core;
    core.prepare(rate);
    const auto coreLevel = measure(static_cast<int>(rate * 4.0), [&](int sample) {
        if (sample % 24000 == 0) core.trigger(220.0 + sample / 24000 * 20.0);
        return core.tick(1.0, 0.6, 0.4);
    });
    ok = expect(coreLevel.finite && coreLevel.peak <= 0.5 + 1e-9 && coreLevel.energy > 10.0, "Core sounds, bounded")
        && ok;

    PerformanceTextureBed texture;
    texture.prepare(rate, 7U);
    for (const double colour : {0.0, 0.5, 1.0}) {
        const auto level = measure(static_cast<int>(rate * 3.0),
                                   [&](int) { return texture.tick(800.0, colour, 0.9, 1.0, 1.0); });
        ok = expect(level.finite && level.peak < 1.5 && level.energy > 1.0, "Texture sounds in every colour") && ok;
    }

    PerformanceBassFallback bass;
    bass.prepare(rate);
    const auto bassLevel = measure(static_cast<int>(rate * 4.0), [&](int sample) {
        if (sample % 24000 == 0) bass.trigger(220.0, 0.0, 0.3);
        return bass.tick(0.2);
    });
    ok = expect(bassLevel.finite && bassLevel.peak < 0.7 && bassLevel.energy > 10.0, "Bass Fallback sounds, bounded")
        && ok;

    if (!ok) return EXIT_FAILURE;
    std::cout << "PerformanceStudioConceptStars tests passed (thermal deaths " << hot.deaths() << ", texture peak ok)\n";
    return EXIT_SUCCESS;
}
