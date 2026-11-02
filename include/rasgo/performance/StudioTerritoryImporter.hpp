#pragma once

#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>

#include "PerformanceSessionDocument.hpp"

namespace rasgo::performance {

// Control-side reader for RSTM 1/2, emitted by the Studio's
// RasgoTerritoryManifest. The parser intentionally knows no Studio headers:
// the versioned text contract is the only bridge between applications.
enum class StudioTerritoryImportError : std::uint8_t {
    None, Header, Version, Territory, GamePlan, Actors, Actor, Capacity
};

struct StudioTerritoryImportResult {
    StudioTerritoryImportError error = StudioTerritoryImportError::None;
    std::size_t actorCount = 0U;
    std::uint8_t version = 0U;
    bool hasGamePlan = false;
    bool valid() const noexcept { return error == StudioTerritoryImportError::None; }
};

class StudioTerritoryImporter {
public:
    static StudioTerritoryImportResult import(std::string_view text,
                                               PerformanceSessionDocument& destination) {
        std::istringstream input{std::string(text)};
        std::string word;
        std::uint64_t version = 0U;
        if (!(input >> word >> version) || word != "RSTM") return {StudioTerritoryImportError::Header, 0U};
        if (version != 1U && version != 2U) return {StudioTerritoryImportError::Version, 0U};
        std::uint64_t seed = 0U;
        double tempo = 0.0;
        if (!(input >> word >> seed >> tempo) || word != "territory"
            || seed > std::numeric_limits<std::uint32_t>::max() || !finite(tempo)
            || tempo < 20.0 || tempo > 300.0) return {StudioTerritoryImportError::Territory, 0U};
        PerformanceSessionDocument document;
        document.territory.seed = static_cast<std::uint32_t>(seed);
        document.territory.identity = identityFor(document.territory.seed);
        if (version == 2U) {
            std::uint64_t strategy = 0U, rule = 0U;
            auto& plan = document.territory.gamePlan;
            if (!(input >> word >> strategy >> rule >> plan.energy >> plan.memory >> plan.risk
                  >> plan.freedom >> plan.hiddenInformation)
                || word != "game"
                || strategy >= static_cast<std::uint64_t>(PerformanceGameStrategy::Count)
                || rule >= static_cast<std::uint64_t>(PerformanceGameRule::Count))
                return {StudioTerritoryImportError::GamePlan, 0U};
            plan.strategy = static_cast<PerformanceGameStrategy>(strategy);
            plan.rule = static_cast<PerformanceGameRule>(rule);
            if (!plan.valid()) return {StudioTerritoryImportError::GamePlan, 0U};
        }
        std::uint64_t count = 0U;
        if (!(input >> word >> count) || word != "actors") return {StudioTerritoryImportError::Actors, 0U};
        if (count == 0U || count > kTerritoryActorCapacity || count > kConstellationActorCapacity)
            return {StudioTerritoryImportError::Capacity, 0U};
        PerformanceScene opening;
        opening.id = 1U;
        opening.session.territorySeed = document.territory.seed;
        opening.session.tempoBpm = tempo;
        opening.session.currentSceneId = opening.id;
        opening.session.targetSceneId = opening.id;
        for (std::uint64_t index = 0U; index < count; ++index) {
            std::uint64_t id = 0U, family = 0U, capabilities = 0U;
            double salience = 0.0, audibility = 0.0, budget = 0.0;
            if (!(input >> word >> id >> family >> capabilities >> salience >> audibility >> budget)
                || word != "actor" || id >= kInvalidActorId
                || capabilities > std::numeric_limits<std::uint32_t>::max()
                || family >= static_cast<std::uint64_t>(PerformanceFamily::Count)
                || !unit(salience) || !unit(audibility) || !unit(budget))
                return {StudioTerritoryImportError::Actor, static_cast<std::size_t>(index)};
            const auto actorFamily = static_cast<PerformanceFamily>(family);
            if (!document.territory.addActor({static_cast<std::uint16_t>(id), actorFamily,
                                              static_cast<std::uint32_t>(capabilities),
                                              rolesFor(actorFamily), salience, 0.0, 1.0}))
                return {StudioTerritoryImportError::Actor, static_cast<std::size_t>(index)};
            const auto role = initialRoleFor(actorFamily);
            if (!opening.constellation.addActor({static_cast<std::uint16_t>(id), role,
                                                 ActorLifeState::Active,
                                                 presenceFor(audibility), 0.50,
                                                 audibility, budget}))
                return {StudioTerritoryImportError::Actor, static_cast<std::size_t>(index)};
            if (index > 0U) {
                const auto previousId = opening.constellation.actors[index - 1U].actorId;
                if (!opening.constellation.addRelation({previousId, static_cast<std::uint16_t>(id),
                                                        RelationKind::Support, RelationDomain::Time,
                                                        0.35 + salience * 0.45, 12.0, 1.5, 0.90}))
                    return {StudioTerritoryImportError::Capacity, static_cast<std::size_t>(index)};
            }
        }
        if (!document.addScene(opening) || !document.valid())
            return {StudioTerritoryImportError::Capacity, static_cast<std::size_t>(count)};
        destination = document;
        return {StudioTerritoryImportError::None, static_cast<std::size_t>(count),
                static_cast<std::uint8_t>(version), version == 2U};
    }

private:
    static TerritoryIdentity identityFor(std::uint32_t seed) noexcept {
        const auto part = [seed](std::uint32_t salt) {
            std::uint64_t value = (static_cast<std::uint64_t>(seed) << 32U) | salt;
            value += 0x9e3779b97f4a7c15ULL;
            value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
            value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
            value ^= value >> 31U;
            return static_cast<double>(value >> 11U) * (1.0 / 9007199254740991.0);
        };
        return {part(1U), part(2U), part(3U), part(4U), 0.70, 0.60};
    }

    static PerformanceRoleMask rolesFor(PerformanceFamily family) noexcept {
        switch (family) {
            case PerformanceFamily::Tone: return roleBit(PerformanceRole::Chao) | roleBit(PerformanceRole::Corpo) | roleBit(PerformanceRole::Vestigio);
            case PerformanceFamily::Hit: return roleBit(PerformanceRole::Chao) | roleBit(PerformanceRole::Corpo) | roleBit(PerformanceRole::Rasgo);
            case PerformanceFamily::Texture: return roleBit(PerformanceRole::Atrito) | roleBit(PerformanceRole::Contagio) | roleBit(PerformanceRole::Vestigio);
            case PerformanceFamily::Pattern: return roleBit(PerformanceRole::Chao) | roleBit(PerformanceRole::Contagio) | roleBit(PerformanceRole::Rasgo);
            case PerformanceFamily::Rupture: return roleBit(PerformanceRole::Atrito) | roleBit(PerformanceRole::Rasgo) | roleBit(PerformanceRole::Vestigio);
            case PerformanceFamily::Relation: return roleBit(PerformanceRole::Corpo) | roleBit(PerformanceRole::Contagio) | roleBit(PerformanceRole::Vestigio);
            case PerformanceFamily::Count: break;
        }
        return 0U;
    }

    static PerformanceRole initialRoleFor(PerformanceFamily family) noexcept {
        switch (family) {
            case PerformanceFamily::Tone: case PerformanceFamily::Hit: case PerformanceFamily::Pattern: return PerformanceRole::Chao;
            case PerformanceFamily::Texture: case PerformanceFamily::Rupture: return PerformanceRole::Atrito;
            case PerformanceFamily::Relation: return PerformanceRole::Contagio;
            case PerformanceFamily::Count: break;
        }
        return PerformanceRole::Chao;
    }

    static double presenceFor(double audibility) noexcept { return 0.22 + audibility * 0.68; }
};

} // namespace rasgo::performance
