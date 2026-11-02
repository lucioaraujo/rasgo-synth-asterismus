#pragma once

// A MEMÓRIA do Asterismus (2/11/2026): o músico guarda a sessão que quiser,
// com nome, e a reabre quando quiser; o app sempre abre algo novo (decisão do
// autor: "o app abre algo novo a cada nova sessão; talvez criar um dispositivo
// ou memória para salvar a sessão que o usuário desejar").
//
// Conceito tomado da família RASGO, sem copiar código: o banco de cenas do
// Aquorbium (lucioaraujo/aquorbium, commit 19140e2, src/app/
// PerformancePresetBank.h e PerformanceSceneFile.h, AGPL-3.0-or-later, mesma
// autoria) — lugares com nome validado, guardar/lembrar/desfazer, arquivo
// exportável que guarda parâmetros e nunca áudio, notas ou memória de DSP,
// leitura limitada em tamanho. Aqui, além da cena (o documento de sessão já
// existente: BPM, macros, presenças, relações), a memória guarda o que o
// performer mexe ao vivo: as estrelas trocadas e fixadas, o mix, mute/solo,
// os controles locais, a variação da seed, MASTER, TRAMA, VARIAÇÃO, DANÇA e
// CÉU.
//
// Só do lado de controle (thread da interface); nunca no callback de áudio.

#include "PerformanceContracts.hpp"
#include "PerformanceModuleTypes.hpp"
#include "PerformanceSessionDocument.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>

namespace rasgo::performance {

inline constexpr std::size_t kPerformanceMemorySlots = 8U;
inline constexpr std::size_t kPerformanceMemoryMaximumBytes = 96U * 1024U;
inline constexpr std::size_t kPerformanceMemoryNameMaximum = 40U;

struct PerformanceMemory {
    std::string name;
    std::string created;
    std::string document;  // o documento de sessão serializado (RSPERF …)
    std::uint16_t sceneId = 1U;  // a cena em que a memória foi guardada
    std::array<double, 8> stemGain{{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
    std::array<bool, 8> stemMuted{};
    int solo = -1;  // −1: nenhum
    std::array<std::array<double, 3>, 8> moduleControls{};
    std::array<double, 8> seedVariation{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
    double masterDb = -6.0;
    bool skyWeb = false;
    bool mixerVariation = true;
    std::uint8_t collectiveFamily = static_cast<std::uint8_t>(PerformanceMotionFamily::Count);
    std::array<std::uint8_t, 8> bodyFamily{{8, 8, 8, 8, 8, 8, 8, 8}};
    std::uint8_t constellationMotion = 0U;
    // A estrela trocada de cada corpo; Count = a da seed.
    std::array<std::uint8_t, 8> starSwap{};
    std::array<bool, 8> pinned{};

    PerformanceMemory() noexcept {
        starSwap.fill(static_cast<std::uint8_t>(PerformanceModuleBody::Count));
        bodyFamily.fill(static_cast<std::uint8_t>(PerformanceMotionFamily::Count));
        for (auto& controls : moduleControls) controls = {{0.5, 0.5, 0.5}};
    }
};

// Nome de uma memória: 1 a 40 caracteres, numa linha, sem controles.
inline bool performanceMemoryNameValid(std::string_view name) noexcept {
    if (name.empty() || name.size() > kPerformanceMemoryNameMaximum * 4U) return false;
    std::size_t characters = 0U;
    for (const unsigned char c : name) {
        if (c < 0x20U || c == 0x7FU) return false;
        if ((c & 0xC0U) != 0x80U) ++characters;  // conta pontos de código UTF-8
    }
    return characters <= kPerformanceMemoryNameMaximum;
}

inline std::string serializePerformanceMemory(const PerformanceMemory& memory) {
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::setprecision(6);
    out << "RASGO_ASTERISMUS_MEMORIA 1\n";
    out << "nome " << memory.name << '\n';
    out << "criada " << memory.created << '\n';
    out << "cena " << memory.sceneId << '\n';
    out << "mix";
    for (const auto gain : memory.stemGain) out << ' ' << gain;
    out << "\nmudo";
    for (const auto muted : memory.stemMuted) out << ' ' << (muted ? 1 : 0);
    out << "\nsolo " << memory.solo << "\ncontroles";
    for (const auto& controls : memory.moduleControls)
        for (const auto value : controls) out << ' ' << value;
    out << "\nvariacao_da_seed";
    for (const auto value : memory.seedVariation) out << ' ' << value;
    out << "\nmaster " << memory.masterDb << "\ntrama " << (memory.skyWeb ? 1 : 0) << "\nvariacao "
        << (memory.mixerVariation ? 1 : 0) << "\ndanca " << static_cast<int>(memory.collectiveFamily);
    for (const auto family : memory.bodyFamily) out << ' ' << static_cast<int>(family);
    out << "\nceu " << static_cast<int>(memory.constellationMotion) << "\nestrelas";
    for (const auto star : memory.starSwap) out << ' ' << static_cast<int>(star);
    out << "\nfixadas";
    for (const auto pinned : memory.pinned) out << ' ' << (pinned ? 1 : 0);
    out << "\ndocumento " << memory.document.size() << '\n' << memory.document;
    return out.str();
}

enum class PerformanceMemoryError : std::uint8_t { None, TooLarge, Header, Field, Range, Document };

inline PerformanceMemoryError parsePerformanceMemory(std::string_view text, PerformanceMemory& destination) {
    if (text.size() > kPerformanceMemoryMaximumBytes) return PerformanceMemoryError::TooLarge;
    std::istringstream in{std::string(text)};
    in.imbue(std::locale::classic());
    std::string line;
    if (!std::getline(in, line) || line != "RASGO_ASTERISMUS_MEMORIA 1") return PerformanceMemoryError::Header;
    PerformanceMemory parsed;
    auto field = [&](const char* key, std::string& rest) {
        if (!std::getline(in, line)) return false;
        const std::string prefix = std::string(key) + ' ';
        if (line.rfind(prefix, 0U) != 0U) return false;
        rest = line.substr(prefix.size());
        return true;
    };
    auto unit = [](double value) { return std::isfinite(value) && value >= 0.0 && value <= 1.0; };
    std::string rest;
    if (!field("nome", rest) || !performanceMemoryNameValid(rest)) return PerformanceMemoryError::Field;
    parsed.name = rest;
    if (!field("criada", rest) || rest.size() > 64U) return PerformanceMemoryError::Field;
    parsed.created = rest;
    {
        std::string sceneText;
        if (!field("cena", sceneText)) return PerformanceMemoryError::Field;
        std::istringstream v{sceneText};
        unsigned scene = 0U;
        if (!(v >> scene) || scene == 0U || scene > 0xFFFEU) return PerformanceMemoryError::Range;
        parsed.sceneId = static_cast<std::uint16_t>(scene);
    }
    auto numbers = [&](const char* key) -> std::istringstream {
        std::istringstream values;
        values.imbue(std::locale::classic());
        if (field(key, rest)) values.str(rest);
        else values.setstate(std::ios::failbit);
        return values;
    };
    {
        auto v = numbers("mix");
        for (auto& gain : parsed.stemGain) if (!(v >> gain) || !unit(gain)) return PerformanceMemoryError::Range;
    }
    {
        auto v = numbers("mudo");
        for (auto&& muted : parsed.stemMuted) {
            int flag = 0;
            if (!(v >> flag) || (flag != 0 && flag != 1)) return PerformanceMemoryError::Range;
            muted = flag == 1;
        }
    }
    {
        auto v = numbers("solo");
        if (!(v >> parsed.solo) || parsed.solo < -1 || parsed.solo > 7) return PerformanceMemoryError::Range;
    }
    {
        auto v = numbers("controles");
        for (auto& controls : parsed.moduleControls)
            for (auto& value : controls) if (!(v >> value) || !unit(value)) return PerformanceMemoryError::Range;
    }
    {
        auto v = numbers("variacao_da_seed");
        for (auto& value : parsed.seedVariation) if (!(v >> value) || !unit(value)) return PerformanceMemoryError::Range;
    }
    {
        auto v = numbers("master");
        if (!(v >> parsed.masterDb) || !std::isfinite(parsed.masterDb) || parsed.masterDb < -24.0
            || parsed.masterDb > 0.0) return PerformanceMemoryError::Range;
    }
    auto flag = [&](const char* key, bool& destinationFlag) {
        auto v = numbers(key);
        int value = 0;
        if (!(v >> value) || (value != 0 && value != 1)) return false;
        destinationFlag = value == 1;
        return true;
    };
    if (!flag("trama", parsed.skyWeb) || !flag("variacao", parsed.mixerVariation)) return PerformanceMemoryError::Range;
    {
        auto v = numbers("danca");
        const int families = static_cast<int>(PerformanceMotionFamily::Count);
        int family = 0;
        if (!(v >> family) || family < 0 || family > families) return PerformanceMemoryError::Range;
        parsed.collectiveFamily = static_cast<std::uint8_t>(family);
        for (auto& body : parsed.bodyFamily) {
            if (!(v >> family) || family < 0 || family > families) return PerformanceMemoryError::Range;
            body = static_cast<std::uint8_t>(family);
        }
    }
    {
        auto v = numbers("ceu");
        int motion = 0;
        if (!(v >> motion) || motion < 0 || motion >= static_cast<int>(PerformanceConstellationMotion::Count))
            return PerformanceMemoryError::Range;
        parsed.constellationMotion = static_cast<std::uint8_t>(motion);
    }
    {
        auto v = numbers("estrelas");
        for (auto& star : parsed.starSwap) {
            int body = 0;
            if (!(v >> body) || body < 0 || body > static_cast<int>(PerformanceModuleBody::Count))
                return PerformanceMemoryError::Range;
            star = static_cast<std::uint8_t>(body);
        }
    }
    {
        auto v = numbers("fixadas");
        for (auto&& pinned : parsed.pinned) {
            int value = 0;
            if (!(v >> value) || (value != 0 && value != 1)) return PerformanceMemoryError::Range;
            pinned = value == 1;
        }
    }
    std::size_t bytes = 0U;
    {
        auto v = numbers("documento");
        if (!(v >> bytes) || bytes == 0U || bytes > kSessionDocumentMaximumBytes) return PerformanceMemoryError::Document;
    }
    const auto offset = static_cast<std::size_t>(in.tellg());
    if (offset == static_cast<std::size_t>(-1) || offset + bytes != text.size()) return PerformanceMemoryError::Document;
    parsed.document = std::string(text.substr(offset, bytes));
    PerformanceSessionDocument check{};
    if (!parseSessionDocument(parsed.document, check).ok()) return PerformanceMemoryError::Document;
    bool sceneFound = false;
    for (std::size_t index = 0U; index < check.sceneCount; ++index) sceneFound = sceneFound || check.scenes[index].id == parsed.sceneId;
    if (!sceneFound) return PerformanceMemoryError::Document;
    destination = parsed;
    return PerformanceMemoryError::None;
}

} // namespace rasgo::performance
