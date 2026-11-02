#pragma once

// MASTERIZAR O ÁLBUM (2/11/2026): igualar o nível entre as faixas, offline,
// só com ganho — sem limitador nem compressão, para a dinâmica de cada
// performance ficar inteira (a arquitetura comum de saída RASGO: a loudness
// informa a decisão; normalizar para álbum é um passo separado, na entrega).
//
// Conceito do Modo Álbum do Rasgo Synth Studio (AlbumRenderer.hpp, commit
// cfe1698, só lido): cada faixa vai em direção à mediana do álbum, com o
// ajuste limitado a ×0,7…×1,4 (≈ −3,1…+2,9 dB). Aqui, depois disso, o álbum
// inteiro sobe ou desce junto até a mediana chegar a −16 LUFS, sem nenhum
// true peak passar de −1 dBTP. A medição é a do próprio Asterismus
// (PerformanceLoudnessMeter: BS.1770-4 com portas, true peak 4×).
//
// Leitura e escrita em pedaços (memória fixa, qualquer duração). As tomadas
// originais nunca são alteradas: as cópias masterizadas são arquivos novos.
// Só fora do callback de áudio (numa thread de trabalho ou ferramenta).

#include "PerformanceLoudnessMeter.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace rasgo::performance {

struct PerformanceWavInfo {
    std::uint32_t sampleRate = 0U;
    std::uint16_t channels = 0U, bits = 0U, format = 0U;  // 1 = PCM, 3 = float
    std::uint64_t dataOffset = 0U, frames = 0U;
};

namespace album_master_detail {
inline std::uint32_t le32(const unsigned char* b) noexcept {
    return static_cast<std::uint32_t>(b[0]) | (static_cast<std::uint32_t>(b[1]) << 8U)
        | (static_cast<std::uint32_t>(b[2]) << 16U) | (static_cast<std::uint32_t>(b[3]) << 24U);
}
inline std::uint16_t le16(const unsigned char* b) noexcept {
    return static_cast<std::uint16_t>(b[0] | (b[1] << 8U));
}
inline void put32(std::ofstream& out, std::uint32_t v) {
    const unsigned char b[4] = {static_cast<unsigned char>(v), static_cast<unsigned char>(v >> 8U),
                                static_cast<unsigned char>(v >> 16U), static_cast<unsigned char>(v >> 24U)};
    out.write(reinterpret_cast<const char*>(b), 4);
}
inline void put16(std::ofstream& out, std::uint16_t v) {
    const unsigned char b[2] = {static_cast<unsigned char>(v), static_cast<unsigned char>(v >> 8U)};
    out.write(reinterpret_cast<const char*>(b), 2);
}
} // namespace album_master_detail

// Lê o cabeçalho RIFF/WAVE (PCM 16/24/32 bits ou float 32, mono ou estéreo).
inline bool readWavInfo(std::ifstream& in, PerformanceWavInfo& info) {
    using namespace album_master_detail;
    unsigned char header[12];
    if (!in.read(reinterpret_cast<char*>(header), 12) || std::memcmp(header, "RIFF", 4) != 0
        || std::memcmp(header + 8, "WAVE", 4) != 0) return false;
    bool haveFormat = false;
    for (int chunks = 0; chunks < 64; ++chunks) {
        unsigned char chunk[8];
        if (!in.read(reinterpret_cast<char*>(chunk), 8)) return false;
        const auto size = le32(chunk + 4);
        if (std::memcmp(chunk, "fmt ", 4) == 0) {
            if (size < 16U || size > 64U) return false;
            unsigned char fmt[64];
            if (!in.read(reinterpret_cast<char*>(fmt), size)) return false;
            info.format = le16(fmt);
            if (info.format == 0xFFFEU && size >= 26U) info.format = le16(fmt + 24);  // WAVE_FORMAT_EXTENSIBLE
            info.channels = le16(fmt + 2);
            info.sampleRate = le32(fmt + 4);
            info.bits = le16(fmt + 14);
            haveFormat = true;
        } else if (std::memcmp(chunk, "data", 4) == 0) {
            if (!haveFormat || info.channels < 1U || info.channels > 2U || info.sampleRate < 8000U
                || !((info.format == 1U && (info.bits == 16U || info.bits == 24U || info.bits == 32U))
                     || (info.format == 3U && info.bits == 32U))) return false;
            info.dataOffset = static_cast<std::uint64_t>(in.tellg());
            info.frames = size / (static_cast<std::uint64_t>(info.bits / 8U) * info.channels);
            return true;
        } else {
            in.seekg(size + (size & 1U), std::ios::cur);
        }
    }
    return false;
}

// Lê até `frames` quadros estéreo (mono vira os dois canais) em [-1, 1].
inline std::size_t readWavFrames(std::ifstream& in, const PerformanceWavInfo& info, std::size_t frames,
                                 std::vector<unsigned char>& scratch, double* left, double* right) {
    const std::size_t bytesPerSample = info.bits / 8U;
    const std::size_t frameBytes = bytesPerSample * info.channels;
    scratch.resize(frames * frameBytes);
    in.read(reinterpret_cast<char*>(scratch.data()), static_cast<std::streamsize>(scratch.size()));
    const auto got = static_cast<std::size_t>(in.gcount()) / frameBytes;
    for (std::size_t frame = 0U; frame < got; ++frame) {
        double values[2] = {0.0, 0.0};
        for (std::size_t channel = 0U; channel < info.channels; ++channel) {
            const unsigned char* b = scratch.data() + frame * frameBytes + channel * bytesPerSample;
            double v = 0.0;
            if (info.format == 3U) {
                float f = 0.0F;
                std::memcpy(&f, b, 4);
                v = f;
            } else if (info.bits == 16U) {
                v = static_cast<std::int16_t>(album_master_detail::le16(b)) / 32768.0;
            } else if (info.bits == 24U) {
                std::int32_t s = static_cast<std::int32_t>(b[0] | (b[1] << 8U) | (b[2] << 16U));
                if ((s & 0x800000) != 0) s -= 0x1000000;
                v = s / 8388608.0;
            } else {
                v = static_cast<std::int32_t>(album_master_detail::le32(b)) / 2147483648.0;
            }
            values[channel] = std::isfinite(v) ? v : 0.0;
        }
        left[frame] = values[0];
        right[frame] = info.channels == 2U ? values[1] : values[0];
    }
    return got;
}

struct PerformanceTrackLevel {
    double integratedLufs = -70.0;
    double maxTruePeakDbtp = -70.0;
    double seconds = 0.0;
};

inline std::optional<PerformanceTrackLevel> measureWavLevel(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    PerformanceWavInfo info;
    if (!in || !readWavInfo(in, info)) return std::nullopt;
    PerformanceLoudnessMeter meter;
    meter.prepare(info.sampleRate);
    std::vector<unsigned char> scratch;
    std::array<double, 4096> left{}, right{};
    std::uint64_t total = 0U;
    for (;;) {
        const auto got = readWavFrames(in, info, left.size(), scratch, left.data(), right.data());
        for (std::size_t i = 0U; i < got; ++i) meter.process(left[i], right[i]);
        total += got;
        if (got < left.size()) break;
    }
    PerformanceTrackLevel level;
    level.integratedLufs = meter.integrated();
    level.maxTruePeakDbtp = 20.0 * std::log10(std::max(1.0e-9, meter.maxTruePeak()));
    level.seconds = static_cast<double>(total) / info.sampleRate;
    return level;
}

struct PerformanceAlbumMasterPlan {
    std::vector<double> gainDb;   // por faixa
    double medianLufs = -70.0;    // antes
    double albumGainDb = 0.0;     // comum a todas
    double targetLufs = -16.0;
    double ceilingDbtp = -1.0;
};

// Os ganhos: cada faixa em direção à mediana (−3,1…+2,9 dB, o limite do
// Studio), depois um ganho comum até a mediana chegar ao alvo, sem passar o
// teto de true peak em nenhuma faixa. Faixas silenciosas (≤ −69 LUFS) ficam
// com ganho 0.
inline PerformanceAlbumMasterPlan planAlbumMaster(const std::vector<PerformanceTrackLevel>& levels,
                                                  double targetLufs = -16.0, double ceilingDbtp = -1.0) {
    PerformanceAlbumMasterPlan plan;
    plan.targetLufs = targetLufs;
    plan.ceilingDbtp = ceilingDbtp;
    plan.gainDb.assign(levels.size(), 0.0);
    std::vector<double> loud;
    for (const auto& level : levels) if (level.integratedLufs > -69.0) loud.push_back(level.integratedLufs);
    if (loud.empty()) return plan;
    std::sort(loud.begin(), loud.end());
    plan.medianLufs = loud.size() % 2U == 1U ? loud[loud.size() / 2U]
                                             : 0.5 * (loud[loud.size() / 2U - 1U] + loud[loud.size() / 2U]);
    std::vector<double> relative(levels.size(), 0.0);
    double album = targetLufs - plan.medianLufs;
    for (std::size_t index = 0U; index < levels.size(); ++index) {
        if (levels[index].integratedLufs <= -69.0) continue;
        relative[index] = std::clamp(plan.medianLufs - levels[index].integratedLufs, -3.1, 2.9);
        album = std::min(album, ceilingDbtp - levels[index].maxTruePeakDbtp - relative[index]);
    }
    plan.albumGainDb = album;
    for (std::size_t index = 0U; index < levels.size(); ++index)
        if (levels[index].integratedLufs > -69.0) plan.gainDb[index] = relative[index] + album;
    return plan;
}

// Uma cópia com ganho, em WAV estéreo de 24 bits com dither TPDF de ±1 LSB.
inline bool writeMasteredWav(const std::string& source, const std::string& destination, double gainDb,
                             std::uint32_t ditherSeed = 0x5EEDu) {
    using namespace album_master_detail;
    std::ifstream in(source, std::ios::binary);
    PerformanceWavInfo info;
    if (!in || !readWavInfo(in, info)) return false;
    std::ofstream out(destination, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    const std::uint64_t dataBytes = info.frames * 2U * 3U;
    if (dataBytes > 0xFFFFFFFFULL - 36U) return false;
    out.write("RIFF", 4);
    put32(out, static_cast<std::uint32_t>(36U + dataBytes));
    out.write("WAVEfmt ", 8);
    put32(out, 16U);
    put16(out, 1U);
    put16(out, 2U);
    put32(out, info.sampleRate);
    put32(out, info.sampleRate * 6U);
    put16(out, 6U);
    put16(out, 24U);
    out.write("data", 4);
    put32(out, static_cast<std::uint32_t>(dataBytes));
    const double gain = std::pow(10.0, gainDb / 20.0);
    std::uint32_t state = ditherSeed == 0U ? 0x5EEDu : ditherSeed;
    auto uniform = [&state]() {
        state ^= state << 13U;
        state ^= state >> 17U;
        state ^= state << 5U;
        return static_cast<double>(state >> 8U) / 16777216.0;
    };
    std::vector<unsigned char> scratch, bytes;
    std::array<double, 4096> left{}, right{};
    std::uint64_t written = 0U;
    for (;;) {
        const auto got = readWavFrames(in, info, left.size(), scratch, left.data(), right.data());
        bytes.resize(got * 6U);
        for (std::size_t frame = 0U; frame < got; ++frame) {
            const double values[2] = {left[frame], right[frame]};
            for (std::size_t channel = 0U; channel < 2U; ++channel) {
                const double dither = uniform() - uniform();  // TPDF, ±1 LSB
                const double scaled = std::clamp(values[channel] * gain * 8388607.0 + dither, -8388608.0, 8388607.0);
                const auto sample = static_cast<std::int32_t>(std::lround(scaled));
                unsigned char* b = bytes.data() + frame * 6U + channel * 3U;
                b[0] = static_cast<unsigned char>(sample);
                b[1] = static_cast<unsigned char>(sample >> 8U);
                b[2] = static_cast<unsigned char>(sample >> 16U);
            }
        }
        out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        written += got;
        if (got < left.size()) break;
    }
    return static_cast<bool>(out) && written == info.frames;
}

} // namespace rasgo::performance
