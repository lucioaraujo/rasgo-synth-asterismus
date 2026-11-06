#pragma once

// Os tipos comuns aos módulos do Asterismus: a identidade de cada corpo
// sonoro, os seus três controles e o perfil de composição que a seed gera.
//
// Vieram de PerformanceModuleCohortAdapter.hpp (commit 4f518e2), retirado na
// auditoria de 28/10: desde que cada módulo virou uma estrela do céu
// (PerformanceSky.hpp), o adaptador de coortes não tocava mais nada.

#include <cstdint>

namespace rasgo::performance {

enum class PerformanceModuleBody : std::uint8_t {
    Acid = 0,
    Gendyn,
    Percussion, FmFlute, Theremin, Scat, Choir, Vosim, Walsh, HardSync, AtariPunk,
    DejaVu, Atropelo, Galo, ModalResonator, Tuba,
    // Estrelas da biblioteca acrescentadas depois (30/10), sempre no fim:
    Footnote, Bluff, Spectral,
    // Estrelas de controle (2/11): não soam, mudam como as outras tocam.
    Sieve, Polymeter, Hocket, Rupture,
    // Reescritas a partir do conceito (2/11, grupo B).
    Harmonaig, Bloom, ThermalStatic, Core, Texture, BassFallback,
    // Vozes do Studio trazidas em 6/11 (auditoria dos módulos): na
    // biblioteca e entre as convidadas, fora do sorteio das 8 da seed.
    AyChip, Morph, Rossler, EnvelopePulse,
    Count
};

enum class PerformanceModuleControl : std::uint8_t {
    Primary = 0,
    Secondary,
    Motion,
    Count
};

// The seed describes a compositional world shared by all bodies. It is not a
// preset bank: the values are stable coordinates from which each star builds
// its own rhythm, melody, timbre, collective behaviour and event grammar.
enum class PerformanceCompositionStyle : std::uint8_t {
    Pulse = 0,
    Derive,
    Collage,
    Cortege,
    Orbit
};

struct PerformanceCompositionProfile {
    PerformanceCompositionStyle style = PerformanceCompositionStyle::Pulse;
    std::uint8_t harmonicDialect = 0U;
    double melodicRange = 0.5;
    double rhythmicDrive = 0.5;
    double timbralBrightness = 0.5;
    double eventAffinity = 0.5;
    double spatialAffinity = 0.5;
    double collectiveAffinity = 0.5;
};

// Um fluxo da seed por domínio (5/11, auditoria da seed): a mesma mistura
// splitmix64 da fábrica de territórios, com o domínio no lugar do sal. Antes
// o perfil lia fatias de bits do próprio número — toda seed abaixo de 256
// ficava com extensão melódica, impulso, brilho, eventos e coletividade no
// mínimo, e as fatias se sobrepunham (genes correlacionados). Agora cada gene
// tem o seu fluxo, independente, para qualquer seed.
inline constexpr std::uint32_t performanceSeedStream(std::uint32_t seed, std::uint32_t domain) noexcept {
    std::uint64_t value = ((static_cast<std::uint64_t>(seed) << 32U) | domain) + 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
    return static_cast<std::uint32_t>((value ^ (value >> 31U)) >> 16U);
}

inline constexpr PerformanceCompositionProfile compositionProfileForSeed(
    std::uint32_t seed) noexcept {
    const auto gene = [seed](std::uint32_t domain) constexpr {
        return 0.12 + static_cast<double>(performanceSeedStream(seed, domain) & 0xffffU) / 65535.0 * 0.76;
    };
    return {static_cast<PerformanceCompositionStyle>(performanceSeedStream(seed, 0x51A1U) % 5U),
            static_cast<std::uint8_t>(performanceSeedStream(seed, 0x51A2U) % 6U),
            gene(0x51A3U), gene(0x51A4U), gene(0x51A5U),
            gene(0x51A6U), gene(0x51A7U), gene(0x51A8U)};
}

} // namespace rasgo::performance
