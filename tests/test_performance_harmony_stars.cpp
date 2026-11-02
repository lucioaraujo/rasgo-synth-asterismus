// Harmonaig e Bloom reescritos (PerformanceHarmonyStars.hpp).
#include <rasgo/performance/PerformanceHarmonyStars.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace rasgo::performance;

namespace {
bool expect(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}
} // namespace

int main() {
    bool ok = true;

    // O modo do céu, contínuo entre oitavas.
    ok = expect(performanceModeSemitones(0) == 0.0 && performanceModeSemitones(7) == 12.0
                    && performanceModeSemitones(-1) == -2.0 && performanceModeSemitones(2) == 3.0,
                "mode degrees wrap across octaves") && ok;

    // Bloom: o galho é o tronco aplicado a si mesmo; com CAMINHO 1 troca de
    // nível a cada volta, com CAMINHO 0 nunca.
    PerformanceBloomMelody bloom;
    bloom.prepare(48000.0, 181U);
    bool selfSimilar = true;
    for (std::size_t i = 0U; i < 5U; ++i)
        for (std::size_t j = 0U; j < 5U; ++j)
            selfSimilar = selfSimilar && bloom.branch()[i * 5U + j] == bloom.trunk()[i] + bloom.trunk()[j];
    ok = expect(selfSimilar, "the branch is the trunk applied to itself") && ok;
    for (int note = 0; note < 120; ++note) bloom.step(220.0, 1.0, 0.0);
    ok = expect(bloom.levelSwitches() >= 4U, "a full path walks between trunk and branch") && ok;
    PerformanceBloomMelody still;
    still.prepare(48000.0, 181U);
    for (int note = 0; note < 120; ++note) still.step(220.0, 0.0, 0.0);
    ok = expect(still.levelSwitches() == 0U, "an empty path stays on one level") && ok;
    double energy = 0.0;
    bool finite = true;
    bloom.setGateSamples(4800U);
    for (int sample = 0; sample < 48000; ++sample) {
        if (sample % 9600 == 0) bloom.step(220.0, 0.5, 0.0);
        const double value = bloom.tick();
        finite = finite && std::isfinite(value);
        energy += value * value;
    }
    ok = expect(finite && energy > 1.0, "Bloom sounds") && ok;

    // Harmonaig: quatro vozes que soam, finitas, limitadas; muda de acorde.
    PerformanceHarmonaigPad pad;
    pad.prepare(48000.0);
    double peak = 0.0;
    energy = 0.0;
    int root = 0;
    for (int sample = 0; sample < 48000 * 6; ++sample) {
        if (sample % 48000 == 0) {
            pad.voice(110.0, root, (sample / 48000) % 4, (sample / 48000) % 2 == 0 ? 0.0 : 1.0);
            root = (root + 3) % 7;
        }
        const double value = pad.tick();
        finite = finite && std::isfinite(value);
        peak = std::max(peak, std::fabs(value));
        energy += value * value;
    }
    ok = expect(finite && energy > 10.0 && peak < 1.0, "Harmonaig sounds, bounded") && ok;
    ok = expect(pad.chords() == 6U, "one chord per call") && ok;

    if (!ok) return EXIT_FAILURE;
    std::cout << "PerformanceHarmonyStars tests passed (Harmonaig peak " << peak << ")\n";
    return EXIT_SUCCESS;
}
