// O pan pela física (PerformanceSkyPan.hpp, 3/11/2026).
#include <rasgo/performance/PerformanceSkyPan.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace rasgo::performance;

namespace {
bool expect(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}

constexpr double kPi = 3.14159265358979323846;
constexpr double kRate = 48000.0;

// RMS de cada canal para um seno de `hz`, com o stem parado em `x`.
void measure(double hz, double x, double width, double& leftRms, double& rightRms) {
    PerformanceSkyPan pan;
    pan.prepare(kRate);
    double l2 = 0.0, r2 = 0.0;
    const int total = 48000, skip = 9600;
    for (int i = 0; i < total; ++i) {
        double left = 0.0, right = 0.0;
        pan.add(0U, std::sin(2.0 * kPi * hz * i / kRate), PerformanceSkyPan::targetPan(x, width), left, right);
        if (i >= skip) {
            l2 += left * left;
            r2 += right * right;
        }
    }
    leftRms = std::sqrt(l2 / (total - skip));
    rightRms = std::sqrt(r2 / (total - skip));
}
} // namespace

int main() {
    bool ok = true;

    // O lugar: centro em 0, nunca além de 70 %, ESPAÇO abre o campo.
    ok = expect(PerformanceSkyPan::targetPan(0.5, 1.0) == 0.0, "centre is centre") && ok;
    ok = expect(std::fabs(PerformanceSkyPan::targetPan(1.0, 1.0) - 0.70) < 1e-12
                    && std::fabs(PerformanceSkyPan::targetPan(-3.0, 1.0) + 0.70) < 1e-12,
                "pan never goes beyond 70 %") && ok;
    ok = expect(PerformanceSkyPan::targetPan(1.0, 0.0) == 0.0
                    && std::fabs(PerformanceSkyPan::targetPan(1.0, 0.5) - 0.35) < 1e-12,
                "ESPACO 0 is mono, 0.5 opens half") && ok;
    // Mono exato no centro: grave + agudo somam o sinal nos dois canais.
    {
        PerformanceSkyPan centre;
        centre.prepare(kRate);
        bool same = true;
        for (int i = 0; i < 4800; ++i) {
            double left = 0.0, right = 0.0;
            centre.add(0U, std::sin(i * 0.01) * 0.7, 0.0, left, right);
            same = same && left == right;
        }
        ok = expect(same, "centre is exact mono") && ok;
    }
    ok = expect(PerformanceSkyPan::targetPan(std::nan(""), 1.0) == 0.0, "non-finite place is centre") && ok;

    // No centro, cada canal tem o ganho da mistura de antes (0,94).
    double l = 0.0, r = 0.0;
    measure(1000.0, 0.5, 1.0, l, r);
    const double sineRms = std::sqrt(0.5);
    ok = expect(std::fabs(l / sineRms - 0.94) < 0.01 && std::fabs(r / sineRms - 0.94) < 0.01,
                "centre gain matches the old mix") && ok;

    // À direita, o agudo vai para a direita; a potência total se mantém.
    double lr = 0.0, rr = 0.0;
    measure(1000.0, 1.0, 1.0, lr, rr);
    ok = expect(rr > lr * 3.0, "a star on the right sounds on the right") && ok;
    ok = expect(std::fabs((lr * lr + rr * rr) / (l * l + r * r) - 1.0) < 0.05, "equal power") && ok;

    // O grave fica no centro, mesmo com a estrela num canto.
    double lb = 0.0, rb = 0.0;
    measure(40.0, 1.0, 1.0, lb, rb);
    ok = expect(lb > rb * 0.75, "bass stays near the centre") && ok;

    // Sem degraus: um salto de lugar desliza (~30 ms).
    PerformanceSkyPan glide;
    glide.prepare(kRate);
    double a = 0.0, b = 0.0;
    glide.add(0U, 0.0, -0.7, a, b);
    glide.add(0U, 0.0, 0.7, a, b);
    ok = expect(glide.pan(0U) < -0.69, "a jump in place glides") && ok;
    for (int i = 0; i < 9600; ++i) glide.add(0U, 0.0, 0.7, a, b);
    ok = expect(glide.pan(0U) > 0.69, "the glide arrives") && ok;
    // Um stem novo no mesmo lugar começa onde está, sem deslizar.
    glide.release(0U);
    glide.add(0U, 0.0, -0.5, a, b);
    ok = expect(std::fabs(glide.pan(0U) + 0.5) < 1e-12, "a new stem starts at its place") && ok;

    if (!ok) return EXIT_FAILURE;
    std::cout << "sky pan tests passed\n";
    return EXIT_SUCCESS;
}
