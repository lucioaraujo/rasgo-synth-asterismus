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

inline constexpr PerformanceCompositionProfile compositionProfileForSeed(
    std::uint32_t seed) noexcept {
    const auto fold = seed ^ (seed >> 9U) ^ (seed >> 17U);
    const auto unitFrom = [](std::uint32_t value) constexpr {
        return 0.12 + static_cast<double>(value & 0xffU) / 255.0 * 0.76;
    };
    return {static_cast<PerformanceCompositionStyle>(fold % 5U),
            static_cast<std::uint8_t>((fold >> 5U) % 6U),
            unitFrom(fold >> 8U), unitFrom(fold >> 12U), unitFrom(fold >> 16U),
            unitFrom(fold >> 20U), unitFrom(fold >> 4U), unitFrom(fold >> 24U)};
}

} // namespace rasgo::performance
