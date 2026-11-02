// A MEMÓRIA do Asterismus (PerformanceMemory.hpp): ida e volta sem perda;
// arquivos estragados ou fora dos limites são recusados.
#include <rasgo/performance/PerformanceMemory.hpp>
#include <rasgo/performance/PerformanceTerritoryFactory.hpp>

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
    PerformanceMemory memory;
    memory.name = "Constelação de terça · seed 181";
    memory.created = "2026-11-02 21:40";
    memory.sceneId = 2U;
    memory.document = serializeSessionDocument(PerformanceTerritoryFactory::makeForReleasedStems(181U, 8U, true));
    memory.stemGain[2] = 0.37;
    memory.stemMuted[5] = true;
    memory.solo = 3;
    memory.moduleControls[1] = {{0.1, 0.9, 0.25}};
    memory.seedVariation[7] = 0.83;
    memory.masterDb = -9.5;
    memory.skyWeb = true;
    memory.mixerVariation = false;
    memory.collectiveFamily = static_cast<std::uint8_t>(PerformanceMotionFamily::Pendulum);
    memory.bodyFamily[4] = static_cast<std::uint8_t>(PerformanceMotionFamily::Foam);
    memory.constellationMotion = static_cast<std::uint8_t>(PerformanceConstellationMotion::RotateTravel);
    memory.starSwap[0] = static_cast<std::uint8_t>(PerformanceModuleBody::Harmonaig);
    memory.starSwap[4] = static_cast<std::uint8_t>(PerformanceModuleBody::Tuba);
    memory.pinned[0] = true;

    const auto text = serializePerformanceMemory(memory);
    PerformanceMemory back;
    ok = expect(parsePerformanceMemory(text, back) == PerformanceMemoryError::None, "a memory reads back") && ok;
    ok = expect(back.name == memory.name && back.created == memory.created && back.document == memory.document
                    && back.sceneId == 2U
                    && back.stemGain == memory.stemGain && back.stemMuted == memory.stemMuted && back.solo == 3
                    && back.moduleControls == memory.moduleControls && back.seedVariation == memory.seedVariation
                    && back.masterDb == -9.5 && back.skyWeb && !back.mixerVariation
                    && back.collectiveFamily == memory.collectiveFamily && back.bodyFamily == memory.bodyFamily
                    && back.constellationMotion == memory.constellationMotion && back.starSwap == memory.starSwap
                    && back.pinned == memory.pinned,
                "nothing is lost on the way") && ok;
    ok = expect(serializePerformanceMemory(back) == text, "writing it again gives the same text") && ok;

    auto replaced = [&](const std::string& from, const std::string& to) {
        auto changed = text;
        const auto at = changed.find(from);
        if (at != std::string::npos) changed.replace(at, from.size(), to);
        PerformanceMemory sink;
        return parsePerformanceMemory(changed, sink);
    };
    ok = expect(replaced("RASGO_ASTERISMUS_MEMORIA 1", "RASGO_ASTERISMUS_MEMORIA 9") == PerformanceMemoryError::Header,
                "an unknown version is refused") && ok;
    ok = expect(replaced("master -9.5", "master 12") == PerformanceMemoryError::Range, "MASTER above 0 dB is refused")
        && ok;
    ok = expect(replaced("solo 3", "solo 9") == PerformanceMemoryError::Range, "a solo outside the 8 bodies is refused")
        && ok;
    ok = expect(replaced("cena 2", "cena 77") == PerformanceMemoryError::Document,
                "a memory pointing to a scene the document lacks is refused") && ok;
    ok = expect(replaced("RSPERF", "XSPERF") == PerformanceMemoryError::Document, "a broken scene is refused") && ok;
    PerformanceMemory sink;
    ok = expect(parsePerformanceMemory(text.substr(0U, text.size() - 3U), sink) == PerformanceMemoryError::Document,
                "a cut file is refused") && ok;
    ok = expect(parsePerformanceMemory(std::string(kPerformanceMemoryMaximumBytes + 1U, 'x'), sink)
                    == PerformanceMemoryError::TooLarge, "an oversized file is refused") && ok;
    ok = expect(!performanceMemoryNameValid("") && !performanceMemoryNameValid("linha\nquebrada")
                    && !performanceMemoryNameValid(std::string(41U, 'a')) && performanceMemoryNameValid("Ação · 7"),
                "names: one line, 1 to 40 characters") && ok;

    if (!ok) return EXIT_FAILURE;
    std::cout << "PerformanceMemory tests passed (" << text.size() << " bytes)\n";
    return EXIT_SUCCESS;
}
