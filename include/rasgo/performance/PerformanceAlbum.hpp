#pragma once

// O ÁLBUM do Asterismus (2/11/2026, pedido do autor: "veja no studio como
// isso funciona — a criação de faixas, registros também de faixas"; "pode
// fazer, nessa ordem, seguindo a forma do céu").
//
// Conceito do Modo Álbum do Rasgo Synth Studio (só lido; nada copiado):
// rasgo-synth-core/src/engine/AlbumPlan.hpp e AlbumRenderer.hpp, commit
// cfe1698 — uma seed de álbum deriva as seeds das faixas (peças
// relacionadas, cada uma com a sua identidade; não uma peça longa cortada); o
// andamento das faixas segue um contorno (equilibrado, contrastante,
// ascendente, arco) com uma abertura em %; um registro (album_plan.json) diz,
// faixa a faixa, seed, BPM, duração e arquivo. Lá o álbum é renderizado
// offline; aqui ele é tocado: o performer avança faixa a faixa e cada tomada
// gravada entra no registro.
//
// Só do lado de controle (interface); nunca no callback de áudio.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace rasgo::performance {

inline constexpr std::size_t kPerformanceAlbumMaxTracks = 40U;  // o limite do Studio

enum class PerformanceAlbumContour : std::uint8_t { Balanced, Contrasting, Rising, Arch, Count };

struct PerformanceAlbumTake {
    std::size_t track = 0U;  // 1..N
    std::uint32_t seed = 0U;
    double bpm = 0.0;
    std::string file;
    std::string memory;  // a memória lembrada antes da tomada, se houver
    std::string started;
    double seconds = 0.0;
    double integratedLufs = -70.0;
    double maxTruePeakDbtp = -70.0;
    bool percurso = false;
};

class PerformanceAlbum {
public:
    // Um álbum novo: nome, seed do álbum, quantas faixas (1–40), andamento de
    // base, contorno e abertura (0–35 %, como no Studio).
    bool begin(const std::string& name, std::uint32_t albumSeed, std::size_t tracks, double baseBpm,
               PerformanceAlbumContour contour, double spreadPercent, const std::string& created) {
        if (name.empty() || tracks == 0U || tracks > kPerformanceAlbumMaxTracks || !std::isfinite(baseBpm)
            || !std::isfinite(spreadPercent) || static_cast<std::uint8_t>(contour) >= static_cast<std::uint8_t>(PerformanceAlbumContour::Count))
            return false;
        name_ = name;
        seed_ = albumSeed;
        tracks_ = tracks;
        baseBpm_ = std::clamp(baseBpm, 20.0, 300.0);
        contour_ = contour;
        spread_ = std::clamp(spreadPercent, 0.0, 35.0) / 100.0;
        created_ = created;
        current_ = 1U;
        takes_.clear();
        active_ = true;
        return true;
    }
    void end() noexcept { active_ = false; }
    bool active() const noexcept { return active_; }

    // A seed de uma faixa (1..N): a do álbum misturada com o número da faixa
    // (splitmix32); reprodutível, e faixas vizinhas não se parecem por acaso.
    std::uint32_t trackSeed(std::size_t track) const noexcept {
        std::uint32_t value = seed_ ^ (static_cast<std::uint32_t>(track) * 0x9E3779B9U);
        value += 0x9E3779B9U;
        value = (value ^ (value >> 16U)) * 0x85EBCA6BU;
        value = (value ^ (value >> 13U)) * 0xC2B2AE35U;
        return value ^ (value >> 16U);
    }
    // O andamento de uma faixa pelo contorno: −1..+1 da abertura em torno da
    // base. Equilibrado: um sorteio por faixa; contrastante: alterna lados;
    // ascendente: do mais lento ao mais rápido; arco: sobe e desce.
    double trackBpm(std::size_t track) const noexcept {
        if (tracks_ <= 1U || spread_ <= 0.0) return baseBpm_;
        const double at = static_cast<double>(track - 1U) / static_cast<double>(tracks_ - 1U);
        const double draw = static_cast<double>(trackSeed(track) >> 8U) / 16777216.0;
        double offset = 0.0;
        switch (contour_) {
        case PerformanceAlbumContour::Rising: offset = 2.0 * at - 1.0; break;
        case PerformanceAlbumContour::Arch: offset = std::sin(at * 3.14159265358979323846) * 2.0 - 1.0; break;
        case PerformanceAlbumContour::Contrasting: offset = (track % 2U == 1U ? -1.0 : 1.0) * (0.65 + draw * 0.35); break;
        case PerformanceAlbumContour::Balanced:
        case PerformanceAlbumContour::Count: offset = 2.0 * draw - 1.0; break;
        }
        return std::clamp(std::round(baseBpm_ * (1.0 + offset * spread_) * 10.0) / 10.0, 20.0, 300.0);
    }

    std::size_t current() const noexcept { return current_; }
    std::size_t tracks() const noexcept { return tracks_; }
    bool goTo(std::size_t track) noexcept {
        if (!active_ || track == 0U || track > tracks_) return false;
        current_ = track;
        return true;
    }
    void addTake(const PerformanceAlbumTake& take) { if (takes_.size() < 4096U) takes_.push_back(take); }
    const std::vector<PerformanceAlbumTake>& takes() const noexcept { return takes_; }
    std::size_t takesOf(std::size_t track) const noexcept {
        return static_cast<std::size_t>(std::count_if(takes_.begin(), takes_.end(),
            [track](const PerformanceAlbumTake& take) { return take.track == track; }));
    }
    const std::string& name() const noexcept { return name_; }
    std::uint32_t seed() const noexcept { return seed_; }
    PerformanceAlbumContour contour() const noexcept { return contour_; }

    // O registro (album.json): o plano inteiro (todas as faixas, gravadas ou
    // não) e cada tomada, na ordem em que foi gravada.
    std::string registerJson() const {
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out << std::setprecision(10);
        static constexpr std::array<const char*, 4> contours{{"equilibrado", "contrastante", "ascendente", "arco"}};
        out << "{\n  \"schema\": \"rasgo-asterismus-album\",\n  \"schemaVersion\": 1,\n  \"name\": " << json(name_)
            << ",\n  \"created\": " << json(created_) << ",\n  \"albumSeed\": " << seed_
            << ",\n  \"baseBpm\": " << baseBpm_ << ",\n  \"bpmSpreadPercent\": " << spread_ * 100.0
            << ",\n  \"tempoContour\": \"" << contours[std::min<std::size_t>(static_cast<std::size_t>(contour_), 3U)]
            << "\",\n  \"concept\": \"Modo Álbum do Rasgo Synth Studio (seeds derivadas, contorno de andamento, registro por faixa)\""
            << ",\n  \"tracks\": [";
        for (std::size_t track = 1U; track <= tracks_; ++track) {
            out << (track > 1U ? "," : "") << "\n    {\"track\": " << track << ", \"seed\": " << trackSeed(track)
                << ", \"bpm\": " << trackBpm(track) << ", \"takes\": " << takesOf(track) << "}";
        }
        out << "\n  ],\n  \"takes\": [";
        for (std::size_t index = 0U; index < takes_.size(); ++index) {
            const auto& take = takes_[index];
            out << (index > 0U ? "," : "") << "\n    {\"track\": " << take.track << ", \"seed\": " << take.seed
                << ", \"bpm\": " << take.bpm << ", \"file\": " << json(take.file) << ", \"memory\": "
                << (take.memory.empty() ? std::string("null") : json(take.memory)) << ", \"started\": "
                << json(take.started) << ", \"seconds\": " << take.seconds << ", \"integratedLufs\": "
                << take.integratedLufs << ", \"maxTruePeakDbtp\": " << take.maxTruePeakDbtp
                << ", \"percurso\": " << (take.percurso ? "true" : "false") << "}";
        }
        out << "\n  ]\n}\n";
        return out.str();
    }

private:
    static std::string json(const std::string& text) {
        std::string out = "\"";
        for (const unsigned char c : text) {
            if (c == '"' || c == '\\') { out += '\\'; out += static_cast<char>(c); }
            else if (c < 0x20U) {
                static constexpr char hex[] = "0123456789abcdef";
                out += "\\u00";
                out += hex[c >> 4U];
                out += hex[c & 15U];
            } else out += static_cast<char>(c);
        }
        return out + "\"";
    }

    std::string name_, created_;
    std::uint32_t seed_ = 0U;
    std::size_t tracks_ = 0U, current_ = 1U;
    double baseBpm_ = 100.0, spread_ = 0.15;
    PerformanceAlbumContour contour_ = PerformanceAlbumContour::Balanced;
    std::vector<PerformanceAlbumTake> takes_;
    bool active_ = false;
};

} // namespace rasgo::performance
