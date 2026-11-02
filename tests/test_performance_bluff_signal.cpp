// O jogo de blefe e confiança da estrela Bluff (PerformanceBluffSignal.hpp).
#include <rasgo/performance/PerformanceBluffSignal.hpp>

#include <cstdlib>
#include <iostream>

using rasgo::performance::PerformanceBluffSignal;

namespace {
bool expect(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}
} // namespace

int main() {
    bool ok = true;

    // Sempre verdadeiro: cada sinal tem a sua confirmação, a confiança sobe,
    // nunca há traição.
    PerformanceBluffSignal honest;
    honest.configure(1.0, 181U);
    int signals = 0, confirmations = 0;
    for (int i = 0; i < 400; ++i) {
        const auto beat = honest.beat(0.5, 2U);
        signals += beat.signal ? 1 : 0;
        confirmations += beat.confirmation ? 1 : 0;
        ok = expect(!(beat.signal && beat.confirmation), "signal and confirmation never share a beat") && ok;
    }
    ok = expect(signals > 40 && confirmations >= signals - 1, "every genuine signal is confirmed") && ok;
    ok = expect(honest.trust() > 0.95 && !honest.betrayed(), "honesty builds trust") && ok;

    // Confiança construída e depois só blefes: a traição chega e a resposta
    // se cala; oito sinais depois, a reorganização devolve a confiança neutra.
    PerformanceBluffSignal liar;
    liar.configure(1.0, 7U);
    for (int i = 0; i < 200; ++i) liar.beat(0.6, 1U);
    liar.setTruthfulness(0.0);
    bool sawBetrayal = false, silentAfter = true, reorganized = false;
    for (int i = 0; i < 400; ++i) {
        const auto beat = liar.beat(0.6, 1U);
        if (liar.betrayed()) {
            sawBetrayal = true;
            silentAfter = silentAfter && liar.responseGain() == 0.0;
        } else if (sawBetrayal) {
            reorganized = true;
        }
        ok = expect(!beat.confirmation, "a bluff is followed by nothing") && ok;
    }
    ok = expect(sawBetrayal && silentAfter, "betrayal needs earned trust and silences the answer") && ok;
    ok = expect(reorganized, "the field reorganizes after a betrayal") && ok;

    // Sem confiança prévia não há traição (o blefe desde o início não trai).
    PerformanceBluffSignal bluffer;
    bluffer.configure(0.0, 99U);
    for (int i = 0; i < 400; ++i) bluffer.beat(0.6, 1U);
    ok = expect(!bluffer.betrayed(), "no betrayal without earned trust") && ok;

    if (!ok) return EXIT_FAILURE;
    std::cout << "PerformanceBluffSignal tests passed\n";
    return EXIT_SUCCESS;
}
