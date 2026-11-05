#pragma once

// GERAR O ÁLBUM (5/11/2026, auditoria da seed, item H). Pergunta do autor:
// "as faixas dos álbum precisam ser escolhidas manualmente ou é possível gerar
// o álbum inteiro automaticamente como o studio?"; depois, "avance em todos
// os itens".
//
// Uma faixa renderizada fora do tempo real, com o mesmo caminho de uma faixa
// gravada ao vivo sem tocar em nada: o território da seed com o andamento da
// faixa em todas as cenas, VARIAÇÃO e trama ligadas, MASTER do performer, a
// forma encaixada na duração (o mesmo plano do 1/2/3/5 MIN), o PERCURSO
// trocando as cenas (a mesma ordem da seed que o app usa), a coda na
// proporção da faixa e a cauda. Saem a mistura (WAV estéreo de 24 bits com
// dither TPDF), um WAV mono de 24 bits por corpo e o manifesto JSON.
//
// Conceito do Modo Álbum do Rasgo Synth Studio (AlbumRenderer: o álbum
// renderizado inteiro, faixa a faixa), sem código copiado. Só fora do
// callback de áudio: numa thread de trabalho, memória fixa (blocos de 512),
// cancelável entre blocos.

#include "PerformanceLoudnessMeter.hpp"
#include "PerformanceModuleTypes.hpp"
#include "PerformanceSessionEngine.hpp"
#include "PerformanceSky.hpp"
#include "PerformanceSkyDirector.hpp"
#include "PerformanceTerritoryFactory.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace rasgo::performance {

// A cena de cada parte no PERCURSO (a mesma regra do app): a abertura no
// NASCENTE (cena 1); desenvolvimento, contraste e retorno tiram as cenas 2, 3
// e 4 numa das 6 ordens, pela seed.
inline std::uint16_t performancePercursoScene(std::uint32_t seed, std::uint8_t section) noexcept {
    if (section == 0U) return 1U;
    static constexpr std::array<std::array<std::uint16_t, 3>, 6> kOrders{{
        {{2U, 3U, 4U}}, {{2U, 4U, 3U}}, {{3U, 2U, 4U}}, {{3U, 4U, 2U}}, {{4U, 2U, 3U}}, {{4U, 3U, 2U}}}};
    const auto order = performanceSeedStream(seed, 0x5CE0U) % kOrders.size();
    return kOrders[order][std::min<std::size_t>(section, 3U) - 1U];
}

// O plano da forma de uma faixa com duração (o mesmo do app): os compassos
// até a coda, 15 % de abertura, 35 % de desenvolvimento, 25 % de contraste e o
// resto de retorno; a gramática da seed reparte o que vem depois da abertura.
inline std::array<std::uint32_t, 4> performanceTrackFormPlan(double seconds, double bpm, double codaLeadSeconds) noexcept {
    const double bars = std::max(4.0, (seconds - codaLeadSeconds) * std::max(20.0, bpm) / 240.0);
    std::array<std::uint32_t, 4> plan{{
        static_cast<std::uint32_t>(std::max(1.0, std::round(bars * 0.15))),
        static_cast<std::uint32_t>(std::max(1.0, std::round(bars * 0.35))),
        static_cast<std::uint32_t>(std::max(1.0, std::round(bars * 0.25))), 0U}};
    plan[3] = static_cast<std::uint32_t>(std::max(1.0, bars - plan[0] - plan[1] - plan[2]));
    return plan;
}

inline double performanceTrackCodaSeconds(double seconds) noexcept { return std::clamp(seconds * 0.05, 4.0, 12.0); }

// Um WAV PCM de 24 bits (mono ou estéreo) escrito em pedaços, com dither TPDF
// de ±1 LSB; o cabeçalho é corrigido no fechamento.
class PerformanceWav24Writer {
public:
    bool open(const std::string& path, std::uint32_t sampleRate, std::uint16_t channels, std::uint32_t ditherSeed) {
        out_.open(path, std::ios::binary | std::ios::trunc);
        if (!out_) return false;
        channels_ = channels;
        sampleRate_ = sampleRate;
        dither_ = ditherSeed == 0U ? 0x5EEDu : ditherSeed;
        frames_ = 0U;
        writeHeader(0U);
        return static_cast<bool>(out_);
    }
    void write(const float* left, const float* right, std::size_t frames) {
        bytes_.resize(frames * channels_ * 3U);
        for (std::size_t frame = 0U; frame < frames; ++frame) {
            for (std::size_t channel = 0U; channel < channels_; ++channel) {
                const float value = channel == 0U ? left[frame] : (right != nullptr ? right[frame] : left[frame]);
                const double dither = uniform() - uniform();
                const double scaled = std::clamp(static_cast<double>(std::isfinite(value) ? value : 0.0F) * 8388607.0 + dither,
                                                 -8388608.0, 8388607.0);
                const auto sample = static_cast<std::int32_t>(std::lround(scaled));
                unsigned char* b = bytes_.data() + (frame * channels_ + channel) * 3U;
                b[0] = static_cast<unsigned char>(sample);
                b[1] = static_cast<unsigned char>(sample >> 8U);
                b[2] = static_cast<unsigned char>(sample >> 16U);
            }
        }
        out_.write(reinterpret_cast<const char*>(bytes_.data()), static_cast<std::streamsize>(bytes_.size()));
        frames_ += frames;
    }
    bool close() {
        if (!out_.is_open()) return false;
        out_.seekp(0);
        writeHeader(frames_);
        const bool ok = static_cast<bool>(out_);
        out_.close();
        return ok;
    }

private:
    void put32(std::uint32_t v) {
        const unsigned char b[4] = {static_cast<unsigned char>(v), static_cast<unsigned char>(v >> 8U),
                                    static_cast<unsigned char>(v >> 16U), static_cast<unsigned char>(v >> 24U)};
        out_.write(reinterpret_cast<const char*>(b), 4);
    }
    void put16(std::uint16_t v) {
        const unsigned char b[2] = {static_cast<unsigned char>(v), static_cast<unsigned char>(v >> 8U)};
        out_.write(reinterpret_cast<const char*>(b), 2);
    }
    void writeHeader(std::uint64_t frames) {
        const auto dataBytes = static_cast<std::uint32_t>(std::min<std::uint64_t>(frames * channels_ * 3U, 0xFFFFFFFFULL - 36U));
        out_.write("RIFF", 4);
        put32(36U + dataBytes);
        out_.write("WAVEfmt ", 8);
        put32(16U);
        put16(1U);
        put16(channels_);
        put32(sampleRate_);
        put32(sampleRate_ * channels_ * 3U);
        put16(static_cast<std::uint16_t>(channels_ * 3U));
        put16(24U);
        out_.write("data", 4);
        put32(dataBytes);
    }
    double uniform() noexcept {
        dither_ ^= dither_ << 13U;
        dither_ ^= dither_ >> 17U;
        dither_ ^= dither_ << 5U;
        return static_cast<double>(dither_ >> 8U) / 16777216.0;
    }
    std::ofstream out_;
    std::uint16_t channels_ = 2U;
    std::uint32_t sampleRate_ = 48000U;
    std::uint32_t dither_ = 0x5EEDu;
    std::uint64_t frames_ = 0U;
    std::vector<unsigned char> bytes_;
};

struct PerformanceOfflineTrack {
    std::uint32_t seed = 0U;
    double bpm = 120.0;
    double seconds = 120.0;
    double masterDb = -6.0;
    double polyphonyBias = 0.0;               // o POLIFONIA do músico (−1 a +1)
    std::string mixPath;                      // a mistura; os stems ao lado, como numa tomada ao vivo
    std::string manifestPath;
    std::string startedAt;                    // carimbo ISO, do chamador
};

struct PerformanceOfflineResult {
    bool ok = false;
    bool cancelled = false;
    double seconds = 0.0;
    double integratedLufs = -70.0;
    double maxTruePeakDbtp = -70.0;
    std::array<std::string, kPerformanceSafeBodyLimit> stemFiles{};
    std::vector<std::string> trace;
};

namespace offline_detail {
inline std::string json(const std::string& text) {
    std::string out = "\"";
    for (const char c : text) {
        if (c == '"' || c == '\\') out += '\\';
        if (c == '\n') { out += "\\n"; continue; }
        out += c;
    }
    return out + "\"";
}
inline std::string stemLabel(PerformanceModuleBody star) {
    std::string label{performanceStarLibraryId(star)};
    for (auto& c : label) c = static_cast<char>(c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c);
    return label.empty() ? std::string("CORPO") : label;
}
inline std::string siblingStem(const std::string& mixPath, std::size_t body, PerformanceModuleBody star) {
    const auto dot = mixPath.rfind(".wav");
    const auto base = dot == std::string::npos ? mixPath : mixPath.substr(0U, dot);
    return base + "_stem" + std::to_string(body + 1U) + "_" + stemLabel(star) + ".wav";
}
inline std::string fileName(const std::string& path) {
    const auto slash = path.find_last_of('/');
    return slash == std::string::npos ? path : path.substr(slash + 1U);
}
} // namespace offline_detail

// Renderiza uma faixa. `cancel` (opcional) é lido entre blocos.
inline PerformanceOfflineResult renderOfflineTrack(const PerformanceOfflineTrack& track,
                                                   const std::atomic<bool>* cancel = nullptr) {
    using namespace offline_detail;
    constexpr double kRate = 48000.0;
    constexpr std::uint32_t kBlock = 512U;
    PerformanceOfflineResult result;
    auto document = PerformanceTerritoryFactory::makeForReleasedStems(track.seed, kPerformanceSafeBodyLimit, true);
    for (std::size_t index = 0U; index < document.sceneCount; ++index) document.scenes[index].session.tempoBpm = track.bpm;
    auto engine = std::make_unique<PerformanceSessionEngine>();
    if (!engine->prepare(kRate, kBlock, 2U) || !engine->setScene(document.scenes[0], document.territory)) return result;
    std::vector<std::vector<float>> stems(kPerformanceSafeBodyLimit, std::vector<float>(kBlock, 0.0F));
    std::array<float*, kPerformanceSafeBodyLimit> stemPointers{};
    for (std::size_t index = 0U; index < stems.size(); ++index) stemPointers[index] = stems[index].data();
    engine->setStemTap(stemPointers.data(), stemPointers.size(), kBlock);
    engine->setMixerVariation(true);
    engine->setSkyInfluence(1.0);
    engine->setOutputTrimDb(track.masterDb);
    engine->setPolyphonyBias(track.polyphonyBias);
    const double coda = performanceTrackCodaSeconds(track.seconds);
    engine->setCodaSeconds(coda);
    const double codaLead = coda + 1.2;
    engine->setFormPlan(performanceTrackFormPlan(track.seconds, track.bpm, codaLead));

    const auto stars = performanceInitialAsterism(track.seed);
    PerformanceWav24Writer mix;
    std::array<PerformanceWav24Writer, kPerformanceSafeBodyLimit> stemWriters;
    if (!mix.open(track.mixPath, static_cast<std::uint32_t>(kRate), 2U, track.seed ^ 0x51A7U)) return result;
    for (std::size_t body = 0U; body < stemWriters.size(); ++body) {
        const auto path = siblingStem(track.mixPath, body, stars[body]);
        result.stemFiles[body] = fileName(path);
        if (!stemWriters[body].open(path, static_cast<std::uint32_t>(kRate), 1U, track.seed ^ static_cast<std::uint32_t>(body * 0x9E37U)))
            return result;
    }
    PerformanceLoudnessMeter meter;
    meter.prepare(kRate);

    result.trace.push_back("start:seed=" + std::to_string(track.seed));
    result.trace.push_back("render:offline");
    std::uint8_t section = 255U, pass = 0U;
    std::uint32_t guests = 0U;
    bool codaAsked = false, grammarTraced = false;
    std::array<float, kBlock> left{}, right{};
    const auto maxBlocks = static_cast<std::uint64_t>((track.seconds + 40.0) * kRate / kBlock);
    std::uint64_t frames = 0U;
    for (std::uint64_t block = 0U; block < maxBlocks; ++block) {
        if (cancel != nullptr && cancel->load(std::memory_order_relaxed)) { result.cancelled = true; break; }
        engine->processBlock(left.data(), right.data(), kBlock);
        mix.write(left.data(), right.data(), kBlock);
        for (std::size_t body = 0U; body < stemWriters.size(); ++body) stemWriters[body].write(stems[body].data(), nullptr, kBlock);
        for (std::uint32_t i = 0U; i < kBlock; ++i) meter.process(left[i], right[i]);
        frames += kBlock;
        const double elapsed = static_cast<double>(frames) / kRate;
        const auto telemetry = engine->readTelemetry();
        if (!grammarTraced) {
            result.trace.push_back(std::string("forma:") + PerformanceSkyDirector::grammarName(telemetry.skyGrammar));
            grammarTraced = true;
            pass = telemetry.skyFormPass;
        }
        // O PERCURSO, como no app: a cena da parte; com a duração, a forma
        // que completou a primeira volta fica onde está até a coda.
        const auto now = static_cast<std::uint8_t>(std::min<unsigned>(telemetry.skySection, 3U));
        const bool passEnded = telemetry.skyFormPass != pass;
        pass = telemetry.skyFormPass;
        if (now != section && !passEnded) {
            section = now;
            const auto scene = performancePercursoScene(track.seed, section);
            if (scene >= 1U && scene <= document.sceneCount)
                engine->recallScene(document.scenes[scene - 1U], document.territory);
            result.trace.push_back("percurso:" + std::to_string(static_cast<int>(section)));
        }
        if (telemetry.skyGuests != guests) {
            if (telemetry.skyGuests == 0U) result.trace.push_back("convidadas:fim");
            for (int k = 0; k < 2; ++k) {
                const auto entry = (telemetry.skyGuests >> (k * 16)) & 0xffffU;
                if (entry != 0U)
                    result.trace.push_back("convidada:corpo" + std::to_string(entry & 0xffU) + ":"
                        + std::string(performanceStarLibraryId(static_cast<PerformanceModuleBody>((entry >> 8U) & 0xffU))));
            }
            guests = telemetry.skyGuests;
        }
        if (!codaAsked && elapsed >= track.seconds - codaLead) {
            engine->beginMorceauCoda();
            codaAsked = true;
            result.trace.push_back("coda:duracao");
        }
        if (codaAsked && engine->morceauState() == PerformanceMorceauState::Finalized) break;
    }
    bool ok = mix.close();
    for (auto& writer : stemWriters) ok = writer.close() && ok;
    result.seconds = static_cast<double>(frames) / kRate;
    result.integratedLufs = meter.integrated();
    result.maxTruePeakDbtp = 20.0 * std::log10(std::max(1.0e-9, meter.maxTruePeak()));

    // O manifesto, no formato das tomadas ao vivo (RASGO_PERFORMANCE_TAKE),
    // com "render": "offline".
    std::ofstream manifest(track.manifestPath, std::ios::trunc);
    manifest << "{\n  \"format\": \"RASGO_PERFORMANCE_TAKE\",\n  \"version\": 1,\n  \"render\": \"offline\",\n"
             << "  \"startedAt\": " << json(track.startedAt) << ",\n  \"wav\": " << json(fileName(track.mixPath)) << ",\n"
             << "  \"seed\": " << track.seed << ",\n  \"bpm\": " << track.bpm << ",\n  \"sampleRate\": 48000.0,\n"
             << "  \"channels\": 2,\n  \"acceptedSamples\": " << frames << ",\n  \"durationSeconds\": " << result.seconds
             << ",\n  \"targetSeconds\": " << track.seconds << ",\n  \"masterDb\": " << track.masterDb
             << ",\n  \"mixerVariation\": true,\n  \"loudness\": {\"integratedLufs\": " << result.integratedLufs
             << ", \"maxTruePeakDbtp\": " << result.maxTruePeakDbtp << "},\n  \"asterism\": [";
    for (std::size_t body = 0U; body < stars.size(); ++body)
        manifest << (body > 0U ? ", " : "") << "{\"body\": " << body + 1U << ", \"star\": "
                 << json(std::string(performanceStarLibraryId(stars[body]))) << "}";
    manifest << "],\n  \"stems\": [";
    for (std::size_t body = 0U; body < result.stemFiles.size(); ++body)
        manifest << (body > 0U ? ", " : "") << "{\"body\": " << body + 1U << ", \"wav\": " << json(result.stemFiles[body]) << "}";
    manifest << "],\n  \"state\": " << json(result.cancelled ? "interrompido" : "finalizado") << ",\n  \"trace\": [";
    for (std::size_t index = 0U; index < result.trace.size(); ++index)
        manifest << (index > 0U ? ", " : "") << json(result.trace[index]);
    manifest << "]\n}\n";
    ok = ok && static_cast<bool>(manifest);
    result.ok = ok && !result.cancelled;
    return result;
}

} // namespace rasgo::performance
