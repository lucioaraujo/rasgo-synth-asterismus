#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace rasgo::performance {

// The Performance does not expose the whole RASGO catalogue as a grid of
// inert widgets. This registry is the control-side bridge between the broad
// catalogue and small, audible seed-selected cohorts.
enum class PerformanceLibraryState : std::uint8_t {
    ActiveCohort,
    RealtimeCandidate,
    NeedsAudit
};

enum class PerformanceModuleFamily : std::uint8_t {
    Tone,
    Texture,
    Voice,
    Pulse,
    Resource
};

struct PerformanceLibraryEntry {
    std::string_view id{};
    std::string_view displayName{};
    PerformanceModuleFamily family = PerformanceModuleFamily::Tone;
    PerformanceLibraryState state = PerformanceLibraryState::NeedsAudit;
    // Bit 0: Circuit, bit 1: Breath, bit 2: Voice, bit 3: Digital, bit 4:
    // Memory. A body may belong to more than one seed ecology without being
    // instantiated twice.
    std::uint8_t cohortMask = 0U;
    // Provenance remains explicit even while an entry has no Performance DSP
    // adapter. This catalogue is a local, read-only bridge from Studio, never
    // a runtime dependency on Studio/Core.
    std::string_view source{};
};

inline constexpr std::array<PerformanceLibraryEntry, 29> kPerformanceModuleLibrary{{
    {"acid", "Acid", PerformanceModuleFamily::Tone, PerformanceLibraryState::ActiveCohort, 1U},
    {"gendyn", "Gendyn", PerformanceModuleFamily::Texture, PerformanceLibraryState::ActiveCohort, 1U},
    {"drums", "Drums", PerformanceModuleFamily::Pulse, PerformanceLibraryState::ActiveCohort, 1U},
    {"fm-flute", "FM Flute", PerformanceModuleFamily::Tone, PerformanceLibraryState::ActiveCohort, 2U},
    {"theremin", "Theremin", PerformanceModuleFamily::Tone, PerformanceLibraryState::ActiveCohort, 2U},
    {"scat", "Scat", PerformanceModuleFamily::Voice, PerformanceLibraryState::ActiveCohort, 4U},
    {"choir", "Choir", PerformanceModuleFamily::Voice, PerformanceLibraryState::ActiveCohort, 4U},
    {"vosim", "VOSIM", PerformanceModuleFamily::Tone, PerformanceLibraryState::ActiveCohort, 14U},
    {"walsh", "Walsh", PerformanceModuleFamily::Tone, PerformanceLibraryState::ActiveCohort, 5U},
    {"hard-sync", "Hard Sync", PerformanceModuleFamily::Tone, PerformanceLibraryState::ActiveCohort, 17U},
    {"atari-punk", "Atari Punk", PerformanceModuleFamily::Pulse, PerformanceLibraryState::ActiveCohort, 10U},
    {"deja-vu", "Deja Vu", PerformanceModuleFamily::Texture, PerformanceLibraryState::ActiveCohort, 16U},
    {"atropelo", "Atropelo", PerformanceModuleFamily::Voice, PerformanceLibraryState::ActiveCohort, 16U},
    {"galo", "Galo", PerformanceModuleFamily::Tone, PerformanceLibraryState::ActiveCohort, 16U},
    {"modal-resonator", "Modal Resonator", PerformanceModuleFamily::Tone,
     PerformanceLibraryState::RealtimeCandidate, 31U},
    // Canonical Studio organism stems not yet adapted to the realtime
    // Performance engine. They are catalogued here so the Field/library can
    // name them honestly and future adapter work has stable provenance.
    {"core", "Core", PerformanceModuleFamily::Tone, PerformanceLibraryState::NeedsAudit,
     0U, "Studio OrganismStem::Core"},
    {"texture", "Texture", PerformanceModuleFamily::Texture, PerformanceLibraryState::NeedsAudit,
     0U, "Studio OrganismStem::Texture"},
    // Tuba: estrela da biblioteca desde 30/10 (o performer a põe num corpo;
    // a seed não a escolhe). Voz local TubaBreathVoice.hpp.
    {"tuba", "Tuba / Breath", PerformanceModuleFamily::Tone, PerformanceLibraryState::ActiveCohort,
     0U, "Studio OrganismStem::Tuba"},
    {"bass-fallback", "Bass Fallback", PerformanceModuleFamily::Tone, PerformanceLibraryState::NeedsAudit,
     0U, "Studio OrganismStem::BassFallback"},
    // Harmonaig e Bloom: reescritas a partir do conceito desde 2/11
    // (PerformanceHarmonyStars.hpp).
    {"harmonaig", "Harmonaig", PerformanceModuleFamily::Voice, PerformanceLibraryState::ActiveCohort,
     0U, "Studio OrganismStem::Harmonaig"},
    // Sieve, Polymeter, Hocket, Rupture: estrelas de controle desde 2/11 (não
    // soam; filtram os ataques das outras — PerformanceSky::controlGate).
    {"sieve", "Sieve", PerformanceModuleFamily::Resource, PerformanceLibraryState::ActiveCohort,
     0U, "Studio OrganismStem::Sieve"},
    {"bloom", "Bloom", PerformanceModuleFamily::Tone, PerformanceLibraryState::ActiveCohort,
     0U, "Studio OrganismStem::Bloom"},
    {"hocket", "Hocket", PerformanceModuleFamily::Pulse, PerformanceLibraryState::ActiveCohort,
     0U, "Studio OrganismStem::Hocket"},
    // Spectral Object: estrela da biblioteca desde 30/10 (PerformanceSpectralObject.hpp).
    {"spectral", "Spectral Object", PerformanceModuleFamily::Texture, PerformanceLibraryState::ActiveCohort,
     0U, "Studio OrganismStem::Spectral"},
    {"polymeter", "Polymeter", PerformanceModuleFamily::Pulse, PerformanceLibraryState::ActiveCohort,
     0U, "Studio OrganismStem::Polymeter"},
    // Bluff Signal: estrela da biblioteca desde 30/10 (PerformanceBluffSignal.hpp).
    {"bluff", "Bluff Signal", PerformanceModuleFamily::Voice, PerformanceLibraryState::ActiveCohort,
     0U, "Studio OrganismStem::Bluff"},
    // Footnote: estrela da biblioteca desde 30/10 (FootnotePluck vendorado).
    {"footnote", "Footnote", PerformanceModuleFamily::Voice, PerformanceLibraryState::ActiveCohort,
     0U, "Studio OrganismStem::Footnote"},
    {"rupture", "Rupture", PerformanceModuleFamily::Resource, PerformanceLibraryState::ActiveCohort,
     0U, "Studio OrganismStem::Rupture"},
    {"thermal-static", "Thermal Static", PerformanceModuleFamily::Texture, PerformanceLibraryState::NeedsAudit,
     0U, "Studio OrganismStem::ThermalStatic"},
}};

inline constexpr const PerformanceLibraryEntry* findPerformanceModule(
    std::string_view id) noexcept {
    for (const auto& entry : kPerformanceModuleLibrary)
        if (entry.id == id) return &entry;
    return nullptr;
}

inline constexpr std::size_t performanceModuleLibraryIndex(
    std::string_view id) noexcept {
    for (std::size_t index = 0U; index < kPerformanceModuleLibrary.size(); ++index)
        if (kPerformanceModuleLibrary[index].id == id) return index;
    return kPerformanceModuleLibrary.size();
}

} // namespace rasgo::performance
