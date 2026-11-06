// O LEARN (7/11): toda estrela que o céu toca, e o MODAL, tem a sua ficha,
// com cada campo preenchido nos quatro idiomas; as estrelas de controle
// dizem que não soam.
#include <cstdlib>
#include <iostream>
#include <string_view>

#include <rasgo/performance/PerformanceSky.hpp>
#include <rasgo/performance/PerformanceStarLearn.hpp>

namespace {
using namespace rasgo::performance;

bool ok = true;
void expect(bool condition, std::string_view message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ok = false;
    }
}

std::size_t fiches(PerformanceModuleBody body) {
    std::size_t count = 0U;
    for (const auto& entry : kPerformanceStarLearn) count += entry.body == body;
    return count;
}

bool complete(const PerformanceLearnText& text) {
    return !text.pt.empty() && !text.en.empty() && !text.fr.empty() && !text.es.empty();
}
} // namespace

int main() {
    expect(kPerformanceStarLearn.size() == PerformanceSky::kStarCount + 1U,
           "one fiche per sky star, plus the MODAL");
    for (const auto star : kPerformanceAllStars)
        expect(fiches(star) == 1U, "every sky star has one LEARN fiche");
    expect(fiches(PerformanceModuleBody::ModalResonator) == 1U, "the MODAL has one LEARN fiche");
    for (const auto& entry : kPerformanceStarLearn) {
        bool filled = complete(entry.quick) && complete(entry.how) && complete(entry.listen) && complete(entry.origin);
        for (const auto& control : entry.controls) filled = filled && complete(control);
        expect(filled, "every field of every fiche exists in PT, EN, FR and ES");
        if (performanceStarFunction(entry.body) == PerformanceStarFunction::Control)
            expect(entry.how.pt.find("Não soa") != std::string_view::npos
                       && entry.how.en.find("no sound") != std::string_view::npos,
                   "control-star fiches say that the star makes no sound");
    }
    if (ok) std::cout << "Performance star LEARN tests passed (" << kPerformanceStarLearn.size() << " fiches)\n";
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
