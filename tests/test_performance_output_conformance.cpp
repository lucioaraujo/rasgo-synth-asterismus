// Conformidade da saída técnica do Asterismus (16/11/2026, nivelamento pela
// saida-ref 1.0, RASGO/REFERENCIAS/SAIDA_TECNICA; auditoria de excelência
// sonora, itens 2 e 3). Sinais da EBU Tech 3341 V4 (2023), tabela 1,
// sintetizados a partir da descrição (nenhum arquivo da EBU), como nos testes
// da referência. Prova: medidor LUFS (casos 1-5, +-0,1 LU), detector de true
// peak (15-23, +0,2/-0,4 dBTP), o master em sinais quentes (true peak no teto,
// nível intacto abaixo do joelho) e o dither de 24 bits até o arquivo do JUCE.
#include <rasgo/performance/PerformanceLoudnessMeter.hpp>
#include <rasgo/performance/PerformanceMasterStage.hpp>
#include <rasgo/performance/family/saida/Dither.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kFs = 48000.0;
int failures = 0;

void check(const std::string& name, bool ok) {
    std::printf("%s %s\n", ok ? "ok  " : "FAIL", name.c_str());
    if (!ok) ++failures;
}

struct Reading { double m, s, i; };
Reading playTone(const std::vector<std::pair<double, double>>& segments) {  // segundos, dBFS (<= -200: silêncio)
    rasgo::performance::PerformanceLoudnessMeter meter;
    meter.prepare(kFs);
    std::size_t n = 0;
    for (const auto& [seconds, db] : segments) {
        const double a = db <= -200.0 ? 0.0 : std::pow(10.0, db / 20.0);
        const auto count = static_cast<std::size_t>(std::llround(seconds * kFs));
        for (std::size_t k = 0; k < count; ++k, ++n) {
            const double x = a * std::sin(2.0 * kPi * 1000.0 * static_cast<double>(n) / kFs);
            meter.process(x, x);
        }
    }
    return { meter.momentary(), meter.shortTerm(), meter.integrated() };
}

double detectorDb(const std::vector<double>& x) {
    rasgo::performance::PerformanceTruePeakDetector d;
    d.prepare();
    double peak = 0.0;
    for (const double v : x) peak = std::max(peak, d.process(v));
    return 20.0 * std::log10(std::max(peak, 1.0e-12));
}

std::vector<double> tone(double f, double a, double degrees) {
    const std::size_t n = 48000, fade = 480;
    std::vector<double> x(n);
    for (std::size_t i = 0; i < n; ++i)
        x[i] = std::min({1.0, double(i) / fade, double(n - 1 - i) / fade})
            * a * std::sin(2.0 * kPi * f * double(i) / kFs + degrees * kPi / 180.0);
    return x;
}

std::vector<double> singlePeriod(std::size_t offset) {
    const double high = 4.0 * kFs;
    const auto n = static_cast<std::size_t>(high * 0.5);
    std::vector<double> x(n);
    const double slow = high / (kFs / 6.0), fast = high / (kFs / 4.0);
    const auto start = static_cast<std::size_t>(slow * 2000.0);
    const auto fade = static_cast<std::size_t>(high * 0.005);
    for (std::size_t i = 0; i < n; ++i) {
        double v;
        if (i < start) v = 0.5 * std::sin(2.0 * kPi * double(i) / slow);
        else if (double(i - start) < fast) v = std::sin(2.0 * kPi * double(i - start) / fast);
        else v = 0.5 * std::sin(2.0 * kPi * (double(i - start) - fast) / slow);
        x[i] = std::min({1.0, double(i) / fade, double(n - 1 - i) / fade}) * v;
    }
    constexpr int taps = 255;
    const double cutoff = 0.45 * kFs / high;
    std::array<double, taps> h{};
    double sum = 0.0;
    for (int k = 0; k < taps; ++k) {
        const int m = k - (taps - 1) / 2;
        const double sinc = m == 0 ? 2.0 * cutoff : std::sin(2.0 * kPi * cutoff * m) / (kPi * m);
        h[std::size_t(k)] = sinc * (0.42 - 0.5 * std::cos(2.0 * kPi * k / (taps - 1)) + 0.08 * std::cos(4.0 * kPi * k / (taps - 1)));
        sum += h[std::size_t(k)];
    }
    std::vector<double> out;
    for (std::size_t i = offset + taps; i + taps < n; i += 4) {
        double y = 0.0;
        for (int k = 0; k < taps; ++k) y += h[std::size_t(k)] / sum * x[i - std::size_t(k)];
        out.push_back(y);
    }
    return out;
}
}  // namespace

int main() {
    // Loudness, EBU 3341 casos 1-5 (+-0,1 LU)
    for (const auto& [test, level] : {std::pair{1, -23.0}, std::pair{2, -33.0}}) {
        const auto r = playTone({{20.0, level}});
        std::printf("      #%d: M %.2f S %.2f I %.2f (esperado %.0f)\n", test, r.m, r.s, r.i, level);
        check("EBU 3341 #" + std::to_string(test) + " M, S e I",
              std::abs(r.m - level) <= 0.1 && std::abs(r.s - level) <= 0.1 && std::abs(r.i - level) <= 0.1);
    }
    const std::vector<std::pair<int, std::vector<std::pair<double, double>>>> gated{
        {3, {{10.0, -36.0}, {60.0, -23.0}, {10.0, -36.0}}},
        {4, {{10.0, -72.0}, {10.0, -36.0}, {60.0, -23.0}, {10.0, -36.0}, {10.0, -72.0}}},
        {5, {{20.0, -26.0}, {20.1, -20.0}, {20.0, -26.0}}}};
    for (const auto& [test, segments] : gated) {
        const double i = playTone(segments).i;
        std::printf("      #%d: I %.2f (esperado -23)\n", test, i);
        check("EBU 3341 #" + std::to_string(test) + " I (portas absoluta e relativa)", std::abs(i + 23.0) <= 0.1);
    }

    // True peak, EBU 3341 sinais 15-23 (+0,2/-0,4 dBTP)
    const struct { const char* name; double f, a, deg, expected; } tones[] = {
        {"#15", 12000.0, 0.5, 0.0, -6.0}, {"#16", 12000.0, 0.5, 45.0, -6.0}, {"#17", 8000.0, 0.5, 60.0, -6.0},
        {"#18", 6000.0, 0.5, 67.5, -6.0}, {"#19", 12000.0, 1.41, 45.0, 3.0}};
    for (const auto& t : tones) {
        const double db = detectorDb(tone(t.f, t.a, t.deg));
        check(std::string("EBU 3341 ") + t.name + " " + std::to_string(db) + " dBTP",
              db <= t.expected + 0.2 && db >= t.expected - 0.4);
    }
    for (std::size_t offset = 0; offset < 4; ++offset) {
        const double db = detectorDb(singlePeriod(offset));
        check("EBU 3341 #" + std::to_string(20 + offset) + " " + std::to_string(db) + " dBTP", db <= 0.2 && db >= -0.4);
    }

    // O master: true peak no teto em sinais quentes; abaixo do joelho, nada muda
    double worst = 0.0;
    const std::vector<double (*)(int)> hot{
        [](int n) { return 2.0 * std::sin(kPi / 2.0 * n + kPi / 4.0); },
        [](int n) { return ((n / 4800) % 2 ? 2.0 : 0.05) * std::sin(kPi / 2.0 * n + kPi / 4.0); },
        [](int n) { return 3.0 * std::sin(2.0 * kPi * 11025.0 * n / kFs + 0.3); },
        [](int n) { return (n % 4800) == 0 ? 3.0 : 0.0; }};
    for (const auto f : hot) {
        rasgo::performance::PerformanceMasterStage stage;
        stage.prepare(kFs); stage.setMakeupDb(0.0); stage.setMasterGain(1.0);
        rasgo::performance::PerformanceTruePeakDetector meter;
        meter.prepare();
        for (int n = 0; n < 96000; ++n) {
            double l = f(n), r = l;
            stage.process(l, r);
            const double tp = meter.process(l);
            if (n > 24000) worst = std::max(worst, tp);
        }
    }
    std::printf("      master: pior true peak %.2f dBTP (teto %.2f)\n", 20.0 * std::log10(worst),
                20.0 * std::log10(rasgo::performance::PerformanceMasterStage::kCeiling));
    check("master: true peak no teto (0,85 + 1%)", worst <= rasgo::performance::PerformanceMasterStage::kCeiling * 1.01);
    {
        rasgo::performance::PerformanceMasterStage stage;
        stage.prepare(kFs); stage.setMakeupDb(0.0); stage.setMasterGain(1.0);
        double in = 0.0, out = 0.0;
        for (int n = 0; n < 48000; ++n) {
            const double x = 0.5 * std::sin(2.0 * kPi * 440.0 * n / kFs);
            double l = x, r = x;
            stage.process(l, r);
            if (n > 4800) { in += x * x; out += l * l; }
        }
        const double db = 10.0 * std::log10(out / in);
        std::printf("      master abaixo do joelho: %.4f dB\n", db);
        check("master: abaixo do joelho o nível não muda (< 0,01 dB)", std::abs(db) < 0.01);
    }

    // Dither: todo inteiro de 24 bits passa intacto pela conversão do JUCE
    // 9.0.3 (round(x * (2^31 - 1)) >> 8); erro TPDF dentro de 1,5 LSB, sem média
    bool exact = true;
    for (std::int32_t k = -8388608; k <= 8388607 && exact; ++k) {
        const double x = rasgo::performance::floatForJuceInt24(k);
        const int i32 = x <= -1.0 ? std::numeric_limits<int>::min()
                      : x >= 1.0 ? std::numeric_limits<int>::max()
                      : static_cast<int>(std::lround(std::numeric_limits<int>::max() * x));
        exact = (i32 >> 8) == k;
    }
    check("dither: os inteiros de 24 bits chegam intactos ao arquivo", exact);
    rasgo::performance::TpdfQuantiser q(24);
    double sum = 0.0, worstError = 0.0;
    for (int n = 0; n < 200000; ++n) {
        const auto x = static_cast<float>(0.7 * std::sin(2.0 * kPi * 997.0 * n / kFs));
        const double e = static_cast<double>(q.quantise(x)) - static_cast<double>(x) * 8388607.0;
        sum += e; worstError = std::max(worstError, std::abs(e));
    }
    std::printf("      dither: erro médio %.5f LSB, pior %.3f LSB\n", sum / 200000.0, worstError);
    check("dither: erro dentro de 1,5 LSB, sem média", worstError <= 1.5 + 1e-6 && std::abs(sum / 200000.0) < 0.01);

    std::printf("%s\n", failures == 0 ? "conformidade da saída: OK" : "conformidade da saída: FALHOU");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
