// GERAR O ÁLBUM (PerformanceOfflineRender.hpp, 5/11/2026): uma faixa curta
// renderizada fora do tempo real sai com a mistura, os 8 stems e o manifesto,
// na duração pedida, com a forma, o PERCURSO e a coda registrados.
#include <rasgo/performance/PerformanceAlbumMaster.hpp>
#include <rasgo/performance/PerformanceOfflineRender.hpp>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace rasgo::performance;

namespace {
bool expect(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}
bool contains(const std::vector<std::string>& trace, const std::string& prefix) {
    for (const auto& entry : trace) if (entry.rfind(prefix, 0U) == 0U) return true;
    return false;
}
} // namespace

int main() {
    bool ok = true;
    PerformanceOfflineTrack track;
    track.seed = 2026U;
    track.bpm = 128.0;
    track.seconds = 24.0;
    track.mixPath = "offline_render_test.wav";
    track.manifestPath = "offline_render_test.json";
    track.startedAt = "2026-11-05T00:00:00";
    const auto start = std::chrono::steady_clock::now();
    const auto result = renderOfflineTrack(track);
    const double wall = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::cout << "offline: " << result.seconds << " s rendered in " << wall << " s ("
              << (wall > 0.0 ? result.seconds / wall : 0.0) << "× real time), " << result.integratedLufs << " LUFS, TP "
              << result.maxTruePeakDbtp << " dBTP, " << result.trace.size() << " trace entries\n";
    ok = expect(result.ok, "the offline track renders") && ok;
    ok = expect(std::fabs(result.seconds - track.seconds) < 1.0, "the track lasts the asked duration") && ok;
    ok = expect(result.integratedLufs > -40.0 && result.maxTruePeakDbtp <= -1.0, "the track sounds, under the ceiling") && ok;
    ok = expect(contains(result.trace, "forma:") && contains(result.trace, "percurso:") && contains(result.trace, "coda:duracao"),
                "the trace records the grammar, the path and the coda") && ok;
    const auto level = measureWavLevel(track.mixPath);
    ok = expect(level.has_value() && std::fabs(level->seconds - result.seconds) < 0.01, "the mix file is a valid WAV") && ok;
    for (const auto& stem : result.stemFiles) {
        const auto stemLevel = measureWavLevel(stem);
        ok = expect(stemLevel.has_value() && std::fabs(stemLevel->seconds - result.seconds) < 0.01, "each stem is a valid WAV") && ok;
        std::remove(stem.c_str());
    }
    std::ifstream manifest(track.manifestPath);
    const std::string text((std::istreambuf_iterator<char>(manifest)), std::istreambuf_iterator<char>());
    ok = expect(text.find("\"render\": \"offline\"") != std::string::npos && text.find("\"state\": \"finalizado\"") != std::string::npos,
                "the manifest marks the offline render") && ok;
    std::remove(track.mixPath.c_str());
    std::remove(track.manifestPath.c_str());
    if (!ok) return EXIT_FAILURE;
    std::cout << "PerformanceOfflineRender tests passed\n";
    return EXIT_SUCCESS;
}
