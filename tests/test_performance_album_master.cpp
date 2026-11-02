// MASTERIZAR O ÁLBUM (PerformanceAlbumMaster.hpp): três faixas em níveis
// diferentes saem igualadas, só com ganho, sem passar o teto de true peak.
#include <rasgo/performance/PerformanceAlbumMaster.hpp>

#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace rasgo::performance;

namespace {
bool expect(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}
void put32(std::ofstream& out, std::uint32_t v) { out.write(reinterpret_cast<const char*>(&v), 4); }
void put16(std::ofstream& out, std::uint16_t v) { out.write(reinterpret_cast<const char*>(&v), 2); }
// Uma faixa de teste: 1 kHz em rajadas (som e silêncio, como uma peça).
void writeFloatWav(const std::string& path, double amplitude, double seconds) {
    const std::uint32_t rate = 48000U;
    const auto frames = static_cast<std::uint32_t>(rate * seconds);
    std::ofstream out(path, std::ios::binary);
    out.write("RIFF", 4); put32(out, 36U + frames * 8U); out.write("WAVEfmt ", 8); put32(out, 16U);
    put16(out, 3U); put16(out, 2U); put32(out, rate); put32(out, rate * 8U); put16(out, 8U); put16(out, 32U);
    out.write("data", 4); put32(out, frames * 8U);
    for (std::uint32_t i = 0U; i < frames; ++i) {
        const bool on = (i / (rate / 2U)) % 4U != 3U;
        const float v = on ? static_cast<float>(amplitude * std::sin(6.283185307179586 * 1000.0 * i / rate)) : 0.0F;
        out.write(reinterpret_cast<const char*>(&v), 4);
        out.write(reinterpret_cast<const char*>(&v), 4);
    }
}
} // namespace

int main() {
    bool ok = true;
    const std::vector<double> amplitudes{0.05, 0.2, 0.5};  // ~−29, −17, −9 LUFS
    std::vector<std::string> sources, masters;
    std::vector<PerformanceTrackLevel> before;
    for (std::size_t index = 0U; index < amplitudes.size(); ++index) {
        sources.push_back("album_master_test_" + std::to_string(index) + ".wav");
        masters.push_back("album_master_test_" + std::to_string(index) + "_master.wav");
        writeFloatWav(sources.back(), amplitudes[index], 6.0);
        const auto level = measureWavLevel(sources.back());
        ok = expect(level.has_value(), "the test track is read") && ok;
        if (level) before.push_back(*level);
    }
    if (before.size() != amplitudes.size()) return EXIT_FAILURE;
    const auto plan = planAlbumMaster(before);
    std::vector<PerformanceTrackLevel> after;
    for (std::size_t index = 0U; index < sources.size(); ++index) {
        ok = expect(writeMasteredWav(sources[index], masters[index], plan.gainDb[index]), "the master is written") && ok;
        const auto level = measureWavLevel(masters[index]);
        if (level) after.push_back(*level);
        std::cout << "faixa " << index + 1U << ": " << before[index].integratedLufs << " LUFS, TP "
                  << before[index].maxTruePeakDbtp << " → ganho " << plan.gainDb[index] << " dB → "
                  << (level ? level->integratedLufs : -99.0) << " LUFS, TP " << (level ? level->maxTruePeakDbtp : -99.0)
                  << '\n';
    }
    if (after.size() != sources.size()) return EXIT_FAILURE;
    double peak = -99.0;
    for (const auto& level : after) peak = std::max(peak, level.maxTruePeakDbtp);
    ok = expect(peak <= -0.95, "no master passes −1 dBTP") && ok;
    // A do meio fica na mediana; as das pontas chegam o mais perto que o
    // limite de ajuste do Studio (≈ ±3 dB) permite.
    ok = expect(std::fabs(after[1].integratedLufs - (before[1].integratedLufs + plan.gainDb[1])) < 0.2,
                "the measured gain is the planned gain") && ok;
    ok = expect(after[2].integratedLufs - after[0].integratedLufs
                    < before[2].integratedLufs - before[0].integratedLufs - 5.0,
                "the album spread shrinks by the allowed amount") && ok;
    ok = expect(std::fabs(plan.medianLufs - before[1].integratedLufs) < 1e-9, "the median is the middle track") && ok;
    PerformanceTrackLevel silent;
    const auto silentPlan = planAlbumMaster({silent, before[1]});
    ok = expect(silentPlan.gainDb[0] == 0.0, "a silent take keeps its level") && ok;
    for (const auto& file : sources) std::remove(file.c_str());
    for (const auto& file : masters) std::remove(file.c_str());
    if (!ok) return EXIT_FAILURE;
    std::cout << "PerformanceAlbumMaster tests passed (ganho do álbum " << plan.albumGainDb << " dB)\n";
    return EXIT_SUCCESS;
}
