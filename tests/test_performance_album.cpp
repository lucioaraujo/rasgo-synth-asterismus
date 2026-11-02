// O ÁLBUM do Asterismus (PerformanceAlbum.hpp): seeds derivadas, contornos de
// andamento e o registro das faixas.
#include <rasgo/performance/PerformanceAlbum.hpp>

#include <cstdlib>
#include <iostream>
#include <set>

using namespace rasgo::performance;

namespace {
bool expect(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}
} // namespace

int main() {
    bool ok = true;
    PerformanceAlbum album;
    ok = expect(!album.begin("", 1U, 8U, 100.0, PerformanceAlbumContour::Rising, 15.0, "x"), "an album needs a name") && ok;
    ok = expect(!album.begin("x", 1U, 41U, 100.0, PerformanceAlbumContour::Rising, 15.0, "x"), "at most 40 tracks") && ok;
    ok = expect(album.begin("Céu de \"novembro\"", 2026U, 8U, 100.0, PerformanceAlbumContour::Rising, 20.0,
                            "2026-11-02 22:00"), "an album begins") && ok;

    std::set<std::uint32_t> seeds;
    for (std::size_t track = 1U; track <= 8U; ++track) seeds.insert(album.trackSeed(track));
    ok = expect(seeds.size() == 8U, "each track has its own seed") && ok;
    PerformanceAlbum again;
    again.begin("y", 2026U, 8U, 100.0, PerformanceAlbumContour::Rising, 20.0, "z");
    ok = expect(again.trackSeed(5U) == album.trackSeed(5U), "the same album seed gives the same tracks") && ok;

    // Ascendente: do mais lento (−20 %) ao mais rápido (+20 %), sempre subindo.
    bool rising = true;
    for (std::size_t track = 2U; track <= 8U; ++track) rising = rising && album.trackBpm(track) > album.trackBpm(track - 1U);
    ok = expect(rising && album.trackBpm(1U) == 80.0 && album.trackBpm(8U) == 120.0, "the rising contour climbs 80 → 120")
        && ok;
    PerformanceAlbum arch;
    arch.begin("arco", 7U, 9U, 100.0, PerformanceAlbumContour::Arch, 20.0, "z");
    ok = expect(arch.trackBpm(5U) > arch.trackBpm(1U) && arch.trackBpm(5U) > arch.trackBpm(9U), "the arch peaks in the middle")
        && ok;
    PerformanceAlbum contrast;
    contrast.begin("c", 7U, 6U, 100.0, PerformanceAlbumContour::Contrasting, 20.0, "z");
    bool alternates = true;
    for (std::size_t track = 1U; track <= 6U; ++track)
        alternates = alternates && ((track % 2U == 1U) ? contrast.trackBpm(track) < 100.0 : contrast.trackBpm(track) > 100.0);
    ok = expect(alternates, "the contrasting contour alternates slower and faster") && ok;

    ok = expect(album.goTo(3U) && album.current() == 3U && !album.goTo(9U), "moving between tracks stays inside the album")
        && ok;
    album.addTake({3U, album.trackSeed(3U), album.trackBpm(3U), "faixa-03.wav", "memória 1", "22:10", 182.5, -17.2, -2.1, true});
    album.addTake({3U, album.trackSeed(3U), album.trackBpm(3U), "faixa-03b.wav", "", "22:15", 95.0, -18.0, -3.0, false});
    const auto json = album.registerJson();
    ok = expect(album.takesOf(3U) == 2U && album.takesOf(1U) == 0U, "takes are counted per track") && ok;
    ok = expect(json.find("\"name\": \"Céu de \\\"novembro\\\"\"") != std::string::npos
                    && json.find("\"tempoContour\": \"ascendente\"") != std::string::npos
                    && json.find("\"file\": \"faixa-03.wav\"") != std::string::npos
                    && json.find("\"memory\": null") != std::string::npos
                    && json.find("\"percurso\": true") != std::string::npos,
                "the register lists the plan and every take") && ok;

    if (!ok) return EXIT_FAILURE;
    std::cout << "PerformanceAlbum tests passed\n" << json;
    return EXIT_SUCCESS;
}
