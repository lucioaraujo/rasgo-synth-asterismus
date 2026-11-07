// A sombra binaural do céu (7/11): cada canal leva o seu seno e a diferença
// de frequência entre eles é de alguns Hz; calada, não soma nada; o diretor a
// põe em parte das partes da forma, com a portadora na faixa do batimento.
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include <rasgo/performance/PerformanceSkyBinaural.hpp>
#include <rasgo/performance/PerformanceSkyDirector.hpp>

namespace {
using namespace rasgo::performance;
bool ok = true;
void expect(bool condition, std::string_view message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ok = false;
    }
}
} // namespace

int main() {
    constexpr double rate = 48000.0;
    {
        PerformanceSkyBinaural binaural;
        binaural.prepare(rate);
        binaural.seed(181U);
        int crossLeft = 0, crossRight = 0;
        double lastLeft = 0.0, lastRight = 0.0, peak = 0.0;
        bool finite = true;
        const auto samples = static_cast<int>(rate * 20.0);
        for (int i = 0; i < samples; ++i) {
            double left = 0.0, right = 0.0;
            binaural.process(220.0, 6.0, 0.5, left, right);
            finite = finite && std::isfinite(left) && std::isfinite(right);
            peak = std::max(peak, std::max(std::fabs(left), std::fabs(right)));
            if (i > 0 && lastLeft < 0.0 && left >= 0.0) ++crossLeft;
            if (i > 0 && lastRight < 0.0 && right >= 0.0) ++crossRight;
            lastLeft = left;
            lastRight = right;
        }
        const double leftHz = crossLeft / 20.0, beat = (crossRight - crossLeft) / 20.0;
        std::cout << "binaural: left " << leftHz << " Hz, right − left " << beat << " Hz, peak " << peak << '\n';
        expect(finite && peak <= 0.5 + 1e-9, "the binaural shadow is finite and bounded by its gain");
        expect(std::fabs(leftHz - 220.0) < 1.0, "the left channel carries the carrier");
        expect(beat >= 2.5 && beat <= 9.5, "the right channel is a few Hz above: the binaural beat (6 Hz ± its slow drift)");
        double left = 0.0, right = 0.0;
        binaural.process(220.0, 6.0, 0.0, left, right);
        expect(left == 0.0 && right == 0.0, "silenced, it adds nothing");
    }
    {
        int parts = 0, chosen = 0;
        bool carriers = true;
        for (std::uint32_t seed = 1U; seed <= 300U; ++seed) {
            PerformanceSkyDirector director;
            director.setSeed(seed);
            for (std::size_t part = 0U; part < 4U; ++part) {
                ++parts;
                const double amount = director.binauralForPart(part);
                if (amount > 0.0) ++chosen;
                carriers = carriers && (amount == 0.0 || (amount >= 0.5 && amount <= 1.0));
            }
            const double hz = director.binauralCarrierHz();
            carriers = carriers && hz >= 160.0 && hz < 640.0;
            carriers = carriers && director.binauralBeatHz() >= 3.0 && director.binauralBeatHz() <= 10.0;
        }
        const double fraction = static_cast<double>(chosen) / parts;
        std::cout << "binaural in " << fraction * 100.0 << " % of the parts\n";
        expect(fraction > 0.2 && fraction < 0.4, "the seed puts the binaural shadow in about 30 % of the parts");
        expect(carriers, "carrier 160–640 Hz, beat 3–10 Hz, intensity 0.5–1");
    }
    if (ok) std::cout << "Performance sky binaural tests passed\n";
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
