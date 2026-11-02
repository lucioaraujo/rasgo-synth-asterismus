#pragma once

#include "PerformanceContracts.hpp"
#include "PerformanceModuleLibrary.hpp"
#include "PerformanceSky.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace rasgo::performance {

// A structural slot is not automatically an audible stem.  This plan is made
// on the control side before a territory enters the audio queue: it binds a
// seed's cohort to the stems that the host has actually released, and keeps
// future matters visible only as audited reserves.
enum class PerformanceStemDisposition : std::uint8_t {
    Audible,
    Reserved
};

struct PerformanceStemAssignment {
    static constexpr std::size_t kModuleCapacity = 8U;

    std::uint16_t actorId = kInvalidActorId;
    // The first identity is the opening voice of this stem. The rest are
    // seed-compatible matters available for successive events, scene changes
    // and recollection. They are alternatives, never simultaneous voices.
    std::array<std::string_view, kModuleCapacity> moduleIds{};
    std::size_t moduleCount = 0U;
    PerformanceStemDisposition disposition = PerformanceStemDisposition::Reserved;

    constexpr std::string_view primaryModuleId() const noexcept {
        return moduleCount > 0U ? moduleIds[0] : std::string_view{};
    }

    constexpr bool addModule(std::string_view moduleId) noexcept {
        if (moduleId.empty() || moduleCount >= moduleIds.size()) return false;
        for (std::size_t index = 0U; index < moduleCount; ++index)
            if (moduleIds[index] == moduleId) return false;
        moduleIds[moduleCount++] = moduleId;
        return true;
    }
};

struct PerformanceStemDistribution {
    static constexpr std::size_t kCapacity = kPerformanceStemActorCapacity;

    std::array<PerformanceStemAssignment, kCapacity> assignments{};
    std::size_t requestedStemCount = 0U;
    std::size_t audibleStemCount = 0U;

    constexpr const PerformanceStemAssignment* assignmentFor(
        std::uint16_t actorId) const noexcept {
        for (const auto& assignment : assignments)
            if (assignment.actorId == actorId) return &assignment;
        return nullptr;
    }
};

class PerformanceStemDistributor {
public:
    // Um corpo, uma estrela, um stem (2026-10-28). Cada stem recebe a estrela
    // que o primeiro asterismo da seed acende ali; não há mais "alternativas"
    // empilhadas num stem nem o teto auditado de cinco.
    static constexpr PerformanceStemDistribution make(
        std::uint32_t seed, std::size_t releasedStemCount) noexcept {
        PerformanceStemDistribution result;
        result.requestedStemCount = std::min(releasedStemCount, result.kCapacity);
        const auto admitted = std::min(result.requestedStemCount, kPerformanceSafeBodyLimit);
        result.audibleStemCount = admitted;
        const auto asterism = performanceInitialAsterism(seed);
        for (std::size_t index = 0U; index < result.assignments.size(); ++index) {
            auto& assignment = result.assignments[index];
            assignment.actorId = static_cast<std::uint16_t>(index + 1U);
            assignment.disposition = index < admitted ? PerformanceStemDisposition::Audible
                                                       : PerformanceStemDisposition::Reserved;
            assignment.addModule(performanceStarLibraryId(asterism[index]));
        }
        return result;
    }

};

} // namespace rasgo::performance
