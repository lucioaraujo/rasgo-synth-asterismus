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
// LIMITADOR (5/11/2026). Pedido do autor: "se precisar desenvolva o
// limitador". Só com ganho, o álbum de 5/11 ficou em −21 LUFS (mediana), não
// em −16: o ganho comum é barrado pela faixa de pico mais alto. Agora, com
// `maxLimiterDb` > 0, o ganho comum pode passar do teto até essa redução de
// pico prevista (6 dB no app), e a cópia passa por um limitador de true peak
// com antecipação (PerformanceTruePeakLimiter, abaixo): os picos ficam no
// teto, o resto da dinâmica não muda. maxLimiterDb = 0 é o comportamento
// antigo (só ganho).
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

// Limitador de true peak, offline, com antecipação. Desenho clássico de
// "mínimo e média": o ganho necessário em cada amostra (teto ÷ pico 4×) passa
// por um mínimo deslizante de L+1 amostras e por uma média de L amostras; o
// áudio sai atrasado L (+ a latência do detector), então a rampa de ataque
// termina exatamente no pico, sem clique. A liberação é um polo de 150 ms.
// Memória fixa (anéis de L amostras), sem alocação no laço.
class PerformanceTruePeakLimiter {
public:
    void prepare(double sampleRate, double ceilingDbtp, double lookaheadMs = 3.0, double releaseMs = 150.0) {
        ceiling_ = std::pow(10.0, ceilingDbtp / 20.0);
        lookahead_ = std::max<std::size_t>(8U, static_cast<std::size_t>(std::lround(sampleRate * lookaheadMs / 1000.0)));
        delay_ = lookahead_ + kDetectorLatency;
        release_ = 1.0 - std::exp(-1.0 / std::max(1.0, sampleRate * releaseMs / 1000.0));
        minIndex_.assign(lookahead_ + 2U, 0U);
        minValue_.assign(lookahead_ + 2U, 1.0);
        head_ = tail_ = 0U;
        average_.assign(lookahead_, 1.0);
        averageSum_ = static_cast<double>(lookahead_);
        averagePos_ = 0U;
        delayLeft_.assign(delay_ + 1U, 0.0);
        delayRight_.assign(delay_ + 1U, 0.0);
        delayPos_ = 0U;
        envelope_ = 1.0;
        counter_ = 0U;
        maxReduction_ = 1.0;
        detectors_[0].reset();
        detectors_[1].reset();
    }

    // Uma amostra entra, a amostra de `latency()` atrás sai, limitada.
    void process(double left, double right, double& outLeft, double& outRight) noexcept {
        const double peak = std::max({detectors_[0].process(left), detectors_[1].process(right),
                                      std::fabs(left), std::fabs(right)});
        const double target = peak > ceiling_ ? ceiling_ / peak : 1.0;
        // mínimo deslizante (fila monotônica) das últimas L+1 metas
        const auto capacity = minIndex_.size();
        while (head_ != tail_) {
            const auto last = (tail_ + capacity - 1U) % capacity;
            if (minValue_[last] < target) break;
            tail_ = last;
        }
        minIndex_[tail_] = counter_;
        minValue_[tail_] = target;
        tail_ = (tail_ + 1U) % capacity;
        while (counter_ - minIndex_[head_] > lookahead_) head_ = (head_ + 1U) % capacity;
        const double windowMin = minValue_[head_];
        // média de L mínimos
        averageSum_ += windowMin - average_[averagePos_];
        average_[averagePos_] = windowMin;
        averagePos_ = (averagePos_ + 1U) % lookahead_;
        const double smooth = std::min(1.0, averageSum_ / static_cast<double>(lookahead_));
        envelope_ = smooth < envelope_ ? smooth : envelope_ + (smooth - envelope_) * release_;
        envelope_ = std::min(envelope_, smooth);
        maxReduction_ = std::min(maxReduction_, envelope_);
        ++counter_;
        // atraso do áudio
        const double delayedLeft = delayLeft_[delayPos_];
        const double delayedRight = delayRight_[delayPos_];
        delayLeft_[delayPos_] = left;
        delayRight_[delayPos_] = right;
        delayPos_ = (delayPos_ + 1U) % delayLeft_.size();
        // a última proteção: nenhuma amostra passa do teto
        outLeft = std::clamp(delayedLeft * envelope_, -ceiling_, ceiling_);
        outRight = std::clamp(delayedRight * envelope_, -ceiling_, ceiling_);
    }

    std::size_t latency() const noexcept { return delayLeft_.size(); }
    double maxReductionDb() const noexcept { return 20.0 * std::log10(std::max(1.0e-9, maxReduction_)); }

private:
    // O detector 4× responde ~8 amostras depois (16 taps por fase / 2).
    static constexpr std::size_t kDetectorLatency = PerformanceTruePeakDetector::kTapsPerPhase / 2U;
    std::array<PerformanceTruePeakDetector, 2> detectors_{};
    double ceiling_ = 1.0, release_ = 0.001, envelope_ = 1.0, averageSum_ = 0.0, maxReduction_ = 1.0;
    std::size_t lookahead_ = 8U, delay_ = 16U, head_ = 0U, tail_ = 0U, averagePos_ = 0U, delayPos_ = 0U;
    std::uint64_t counter_ = 0U;
    std::vector<std::uint64_t> minIndex_;
    std::vector<double> minValue_, average_, delayLeft_, delayRight_;
};

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
                                                  double targetLufs = -16.0, double ceilingDbtp = -1.0,
                                                  double maxLimiterDb = 0.0) {
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
        // Sem limitador, nenhum pico passa do teto; com ele, até maxLimiterDb
        // de redução prevista no pico mais alto da faixa.
        album = std::min(album, ceilingDbtp + std::max(0.0, maxLimiterDb)
                                    - levels[index].maxTruePeakDbtp - relative[index]);
    }
    plan.albumGainDb = album;
    for (std::size_t index = 0U; index < levels.size(); ++index)
        if (levels[index].integratedLufs > -69.0) plan.gainDb[index] = relative[index] + album;
    return plan;
}

// Uma cópia com ganho, em WAV estéreo de 24 bits com dither TPDF de ±1 LSB.
// Com `limit`, a cópia passa pelo PerformanceTruePeakLimiter no teto
// `ceilingDbtp` (com 0,3 dB de margem para o dither e a reconstrução).
inline bool writeMasteredWav(const std::string& source, const std::string& destination, double gainDb,
                             std::uint32_t ditherSeed = 0x5EEDu, bool limit = false, double ceilingDbtp = -1.0,
                             double* limiterReductionDb = nullptr) {
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
    PerformanceTruePeakLimiter limiter;
    if (limit) limiter.prepare(info.sampleRate, ceilingDbtp - 0.3);
    std::uint64_t pending = limit ? limiter.latency() : 0U;   // amostras de atraso a descartar
    std::uint64_t consumed = 0U;
    bool sourceDone = false;
    for (;;) {
        std::size_t got = 0U;
        if (!sourceDone) {
            got = readWavFrames(in, info, left.size(), scratch, left.data(), right.data());
            consumed += got;
            if (got < left.size()) sourceDone = true;
        }
        // Depois do fim da fonte, o atraso do limitador é esvaziado com zeros.
        if (sourceDone && limit && written + got < info.frames) {
            const auto missing = std::min<std::uint64_t>(left.size() - got, info.frames + limiter.latency() - consumed);
            for (std::size_t i = 0U; i < missing; ++i) { left[got + i] = 0.0; right[got + i] = 0.0; }
            got += static_cast<std::size_t>(missing);
            consumed += missing;
        }
        if (limit) {
            std::size_t kept = 0U;
            for (std::size_t frame = 0U; frame < got; ++frame) {
                double outLeft = 0.0, outRight = 0.0;
                limiter.process(left[frame] * gain, right[frame] * gain, outLeft, outRight);
                if (pending > 0U) { --pending; continue; }
                left[kept] = outLeft;
                right[kept] = outRight;
                ++kept;
            }
            got = std::min<std::uint64_t>(kept, info.frames - written);
        }
        const double frameGain = limit ? 1.0 : gain;
        bytes.resize(got * 6U);
        for (std::size_t frame = 0U; frame < got; ++frame) {
            const double values[2] = {left[frame] * frameGain, right[frame] * frameGain};
            for (std::size_t channel = 0U; channel < 2U; ++channel) {
                const double dither = uniform() - uniform();  // TPDF, ±1 LSB
                const double scaled = std::clamp(values[channel] * 8388607.0 + dither, -8388608.0, 8388607.0);
                const auto sample = static_cast<std::int32_t>(std::lround(scaled));
                unsigned char* b = bytes.data() + frame * 6U + channel * 3U;
                b[0] = static_cast<unsigned char>(sample);
                b[1] = static_cast<unsigned char>(sample >> 8U);
                b[2] = static_cast<unsigned char>(sample >> 16U);
            }
        }
        out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        written += got;
        if (written >= info.frames || (sourceDone && (!limit || consumed >= info.frames + limiter.latency()))) break;
    }
    if (limiterReductionDb != nullptr) *limiterReductionDb = limit ? limiter.maxReductionDb() : 0.0;
    return static_cast<bool>(out) && written == info.frames;
}

} // namespace rasgo::performance
