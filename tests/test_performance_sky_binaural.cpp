// A sombra binaural do céu (7/11): cada canal leva o seu seno e a diferença
// de frequência entre eles é de alguns Hz; calada, não soma nada; o diretor a
// põe em parte das partes da forma, com a portadora na faixa do batimento.
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include <rasgo/performance/PerformanceSkyBinaural.hpp>
#include <rasgo/performance/PerformanceSkyDirector.hpp>
#include <rasgo/performance/PerformanceSessionEngine.hpp>
#include <rasgo/performance/PerformanceTerritoryFactory.hpp>
#include <vector>

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
    {
        // O slider BINAURAL no motor (7/11): −1 cala, +1 = 6 dB acima do nível
        // da seed — a mesma seed com o binaural na abertura, ESPAÇO aberto.
        std::uint32_t seed = 0U;
        for (std::uint32_t s = 9100U; s < 9400U && seed == 0U; ++s) {
            PerformanceSkyDirector director;
            director.setSeed(s);
            if (director.binauralForPart(0U) > 0.8) seed = s;
        }
        const auto render = [seed](double bias) {
            auto doc = PerformanceTerritoryFactory::makeForReleasedStems(seed, kPerformanceSafeBodyLimit, true);
            PerformanceSessionEngine engine;
            engine.prepare(48000.0, 512U, 2U);
            engine.setScene(doc.scenes[0], doc.territory);
            engine.setSkyInfluence(1.0);
            engine.setMacro(PerformanceMacro::Space, 1.0);
            engine.setBinauralBias(bias);
            std::vector<float> out, left(512), right(512);
            for (int block = 0; block < 12 * 48000 / 512; ++block) {
                engine.processBlock(left.data(), right.data(), 512);
                for (int i = 0; i < 512; ++i) { out.push_back(left[i]); out.push_back(right[i]); }
            }
            return out;
        };
        const auto off = render(-1.0), atSeed = render(0.0), up = render(1.0);
        const auto difference = [](const std::vector<float>& a, const std::vector<float>& b) {
            double energy = 0.0;
            for (std::size_t i = 0U; i < a.size(); ++i) energy += (a[i] - b[i]) * (a[i] - b[i]);
            return 10.0 * std::log10(energy / static_cast<double>(a.size()) + 1e-30);
        };
        const double seedDb = difference(atSeed, off), upDb = difference(up, off);
        std::cout << "slider (seed " << seed << "): seed − off " << seedDb << " dB, +1 − off " << upDb << " dB\n";
        expect(seed != 0U && seedDb > -70.0, "at the seed's level the binaural shadow is in the output");
        expect(std::fabs((upDb - seedDb) - 6.0) < 0.5, "the BINAURAL slider at +1 is 6 dB above the seed's level");
    }
    if (ok) std::cout << "Performance sky binaural tests passed\n";
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
