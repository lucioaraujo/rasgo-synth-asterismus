#pragma once

#include "PerformanceContracts.hpp"

#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>

namespace rasgo::performance {

// Persistence belongs to the control/I/O side. It is deliberately absent from
// the audio callback and serializes no Studio state or implementation detail.
constexpr std::size_t kSessionDocumentSceneCapacity = 8;
constexpr std::size_t kSessionDocumentMaximumBytes = 64U * 1024U;

enum class SessionDocumentError : std::uint8_t {
    None, TooLarge, Header, Version, Syntax, Range, Territory, Scene, TrailingData
};

struct PerformanceSessionDocument {
    PerformanceTerritory territory{};
    std::array<PerformanceScene, kSessionDocumentSceneCapacity> scenes{};
    std::size_t sceneCount = 0;

    bool addScene(const PerformanceScene& scene) noexcept {
        if (sceneCount >= scenes.size() || !scene.valid(territory)) return false;
        for (std::size_t index = 0; index < sceneCount; ++index)
            if (scenes[index].id == scene.id) return false;
        scenes[sceneCount++] = scene;
        return true;
    }

    bool valid() const noexcept {
        if (!territory.valid() || sceneCount > scenes.size()) return false;
        for (std::size_t index = 0; index < sceneCount; ++index) {
            if (!scenes[index].valid(territory)) return false;
            for (std::size_t other = index + 1; other < sceneCount; ++other)
                if (scenes[index].id == scenes[other].id) return false;
        }
        return true;
    }
};

struct SessionDocumentParseResult {
    SessionDocumentError error = SessionDocumentError::None;
    std::size_t line = 0;

    constexpr bool ok() const noexcept { return error == SessionDocumentError::None; }
};

namespace detail {

inline bool readUnsigned(std::istringstream& input, std::uint64_t& value) {
    return static_cast<bool>(input >> value);
}

template <typename Enum>
inline bool enumValue(std::uint64_t value, std::uint64_t count, Enum& destination) {
    if (value >= count) return false;
    destination = static_cast<Enum>(value);
    return true;
}

inline bool readActor(std::istringstream& input, PerformanceActorPotential& actor) {
    std::uint64_t id = 0, family = 0, capabilities = 0, roles = 0;
    if (!readUnsigned(input, id) || !readUnsigned(input, family)
        || !readUnsigned(input, capabilities) || !readUnsigned(input, roles)
        || !(input >> actor.salience >> actor.minimumPresence >> actor.maximumPresence)
        || id >= kInvalidActorId || capabilities > std::numeric_limits<std::uint32_t>::max()
        || roles > std::numeric_limits<PerformanceRoleMask>::max()
        || !enumValue(family, static_cast<std::uint64_t>(PerformanceFamily::Count), actor.family))
        return false;
    actor.id = static_cast<std::uint16_t>(id);
    actor.capabilities = static_cast<std::uint32_t>(capabilities);
    actor.allowedRoles = static_cast<PerformanceRoleMask>(roles);
    return actor.valid();
}

inline bool readState(std::istringstream& input, PerformanceSessionState& state,
                      bool hasPopulationPhenomenon = false) {
    std::uint64_t seed = 0, clock = 0, playing = 0, current = 0, target = 0, panic = 0;
    std::uint64_t populationPhenomenon = 0, populationEnabled = 0;
    if (!readUnsigned(input, seed) || !(input >> state.tempoBpm) || !readUnsigned(input, clock)
        || !readUnsigned(input, playing) || !(input >> state.autonomy >> state.intervention
        >> state.density >> state.tension >> state.memory >> state.risk >> state.space)
        || !readUnsigned(input, current) || !readUnsigned(input, target)
        || !(input >> state.morphSeconds >> state.outputTrimDb) || !readUnsigned(input, panic)
        || (hasPopulationPhenomenon && (!readUnsigned(input, populationPhenomenon)
            || !(input >> state.populationPhenomenonIntensity)
            || !readUnsigned(input, populationEnabled)))
        || seed > std::numeric_limits<std::uint32_t>::max()
        || current > std::numeric_limits<std::uint16_t>::max()
        || target > std::numeric_limits<std::uint16_t>::max() || playing > 1 || panic > 1
        || populationEnabled > 1
        || !enumValue(clock, static_cast<std::uint64_t>(ClockSource::Count), state.clock)
        || (hasPopulationPhenomenon && !enumValue(populationPhenomenon,
            static_cast<std::uint64_t>(PerformancePopulationPhenomenon::Count),
            state.populationPhenomenon)))
        return false;
    state.territorySeed = static_cast<std::uint32_t>(seed);
    state.playing = playing != 0;
    state.currentSceneId = static_cast<std::uint16_t>(current);
    state.targetSceneId = static_cast<std::uint16_t>(target);
    state.panicSafe = panic != 0;
    state.populationPhenomenonEnabled = hasPopulationPhenomenon && populationEnabled != 0U;
    return state.valid();
}

inline bool readActorState(std::istringstream& input, PerformanceActorState& actor) {
    std::uint64_t id = 0, role = 0, life = 0;
    if (!readUnsigned(input, id) || !readUnsigned(input, role) || !readUnsigned(input, life)
        || !(input >> actor.presence >> actor.nourishment >> actor.observedAudibility >> actor.gestureBudget)
        || id >= kInvalidActorId
        || !enumValue(role, static_cast<std::uint64_t>(PerformanceRole::Count), actor.role)
        || !enumValue(life, static_cast<std::uint64_t>(ActorLifeState::Count), actor.life))
        return false;
    actor.actorId = static_cast<std::uint16_t>(id);
    return actor.valid();
}

inline bool readRelation(std::istringstream& input, PerformanceRelation& relation,
                         bool hasPhenomenon = false) {
    std::uint64_t source = 0, destination = 0, kind = 0, domain = 0, phenomenon = 0;
    std::uint64_t phenomenonEnabled = 0;
    if (!readUnsigned(input, source) || !readUnsigned(input, destination)
        || !readUnsigned(input, kind) || !readUnsigned(input, domain)
        || !(input >> relation.intensity >> relation.latencyMilliseconds >> relation.durationSeconds >> relation.limit)
        || (hasPhenomenon && (!readUnsigned(input, phenomenon)
            || !(input >> relation.resonanceRatio)
            || !readUnsigned(input, phenomenonEnabled)))
        || source >= kInvalidActorId || destination >= kInvalidActorId
        || !enumValue(kind, static_cast<std::uint64_t>(RelationKind::Count), relation.kind)
        || !enumValue(domain, static_cast<std::uint64_t>(RelationDomain::Count), relation.domain)
        || (hasPhenomenon && (!enumValue(phenomenon,
                static_cast<std::uint64_t>(PerformanceRelationalPhenomenon::Count), relation.phenomenon)
            || phenomenonEnabled > 1U)))
        return false;
    relation.phenomenonEnabled = hasPhenomenon && phenomenonEnabled != 0U;
    relation.sourceActorId = static_cast<std::uint16_t>(source);
    relation.destinationActorId = static_cast<std::uint16_t>(destination);
    return relation.valid();
}

inline bool readGamePlan(std::istringstream& input, PerformanceGamePlan& plan) {
    std::uint64_t strategy = 0U, rule = 0U;
    if (!readUnsigned(input, strategy) || !readUnsigned(input, rule)
        || !(input >> plan.energy >> plan.memory >> plan.risk >> plan.freedom
        >> plan.hiddenInformation)
        || !enumValue(strategy, static_cast<std::uint64_t>(PerformanceGameStrategy::Count), plan.strategy)
        || !enumValue(rule, static_cast<std::uint64_t>(PerformanceGameRule::Count), plan.rule))
        return false;
    return plan.valid();
}

inline void writeGamePlan(std::ostringstream& output, const PerformanceGamePlan& plan) {
    output << static_cast<unsigned>(plan.strategy) << ' ' << static_cast<unsigned>(plan.rule) << ' '
           << plan.energy << ' ' << plan.memory << ' ' << plan.risk << ' '
           << plan.freedom << ' ' << plan.hiddenInformation;
}

inline void writeState(std::ostringstream& output, const PerformanceSessionState& state) {
    output << state.territorySeed << ' ' << state.tempoBpm << ' '
           << static_cast<unsigned>(state.clock) << ' ' << state.playing << ' '
           << state.autonomy << ' ' << state.intervention << ' ' << state.density << ' '
           << state.tension << ' ' << state.memory << ' ' << state.risk << ' ' << state.space << ' '
           << state.currentSceneId << ' ' << state.targetSceneId << ' ' << state.morphSeconds << ' '
           << state.outputTrimDb << ' ' << state.panicSafe << ' '
           << static_cast<unsigned>(state.populationPhenomenon) << ' '
           << state.populationPhenomenonIntensity << ' ' << state.populationPhenomenonEnabled;
}

} // namespace detail

inline std::string serializeSessionDocument(const PerformanceSessionDocument& document) {
    if (!document.valid()) return {};
    std::ostringstream output;
    output << std::setprecision(17) << "RSPERF 4\nterritory " << document.territory.seed << ' '
           << document.territory.identity.tonalVocabulary << ' '
           << document.territory.identity.temporalDna << ' ' << document.territory.identity.matterBias
           << ' ' << document.territory.identity.relationBias << ' '
           << document.territory.identity.memoryLimit << ' ' << document.territory.identity.riskLimit << '\n'
           << "game ";
    detail::writeGamePlan(output, document.territory.gamePlan);
    output << "\nactors " << document.territory.actorCount << '\n';
    for (std::size_t index = 0; index < document.territory.actorCount; ++index) {
        const auto& actor = document.territory.actors[index];
        output << "actor " << actor.id << ' ' << static_cast<unsigned>(actor.family) << ' '
               << actor.capabilities << ' ' << actor.allowedRoles << ' ' << actor.salience << ' '
               << actor.minimumPresence << ' ' << actor.maximumPresence << '\n';
    }
    output << "scenes " << document.sceneCount << '\n';
    for (std::size_t index = 0; index < document.sceneCount; ++index) {
        const auto& scene = document.scenes[index];
        output << "scene " << scene.id << ' ';
        detail::writeState(output, scene.session);
        output << "\nsceneactors " << scene.constellation.actorCount << '\n';
        for (std::size_t actorIndex = 0; actorIndex < scene.constellation.actorCount; ++actorIndex) {
            const auto& actor = scene.constellation.actors[actorIndex];
            output << "sceneactor " << actor.actorId << ' ' << static_cast<unsigned>(actor.role) << ' '
                   << static_cast<unsigned>(actor.life) << ' ' << actor.presence << ' ' << actor.nourishment
                   << ' ' << actor.observedAudibility << ' ' << actor.gestureBudget << '\n';
        }
        output << "scenerelations " << scene.constellation.relationCount << '\n';
        for (std::size_t relationIndex = 0; relationIndex < scene.constellation.relationCount; ++relationIndex) {
            const auto& relation = scene.constellation.relations[relationIndex];
            output << "scenerelation " << relation.sourceActorId << ' ' << relation.destinationActorId << ' '
                   << static_cast<unsigned>(relation.kind) << ' ' << static_cast<unsigned>(relation.domain) << ' '
                   << relation.intensity << ' ' << relation.latencyMilliseconds << ' '
                   << relation.durationSeconds << ' ' << relation.limit << ' '
                   << static_cast<unsigned>(relation.phenomenon) << ' '
                   << relation.resonanceRatio << ' ' << relation.phenomenonEnabled << '\n';
        }
        output << "endscene\n";
    }
    output << "end\n";
    return output.str();
}

inline SessionDocumentParseResult parseSessionDocument(
    std::string_view text, PerformanceSessionDocument& destination) {
    if (text.size() > kSessionDocumentMaximumBytes) return {SessionDocumentError::TooLarge, 0};
    std::istringstream input{std::string(text)};
    std::string word;
    std::size_t line = 1;
    auto expect = [&](const char* expected) { return static_cast<bool>(input >> word) && word == expected; };
    std::uint64_t version = 0;
    if (!expect("RSPERF") || !detail::readUnsigned(input, version)) return {SessionDocumentError::Header, line};
    if (version != 1 && version != 2 && version != 3 && version != 4)
        return {SessionDocumentError::Version, line};
    PerformanceSessionDocument parsed{};
    std::uint64_t seed = 0;
    if (!expect("territory") || !detail::readUnsigned(input, seed)
        || !(input >> parsed.territory.identity.tonalVocabulary >> parsed.territory.identity.temporalDna
        >> parsed.territory.identity.matterBias >> parsed.territory.identity.relationBias
        >> parsed.territory.identity.memoryLimit >> parsed.territory.identity.riskLimit)
        || seed > std::numeric_limits<std::uint32_t>::max()
        || !parsed.territory.identity.valid()) return {SessionDocumentError::Territory, line};
    parsed.territory.seed = static_cast<std::uint32_t>(seed);
    ++line;
    if (version >= 2U) {
        if (!expect("game") || !detail::readGamePlan(input, parsed.territory.gamePlan))
            return {SessionDocumentError::Territory, line};
        ++line;
    }
    std::uint64_t actorCount = 0;
    if (!expect("actors") || !detail::readUnsigned(input, actorCount)
        || actorCount > kTerritoryActorCapacity) return {SessionDocumentError::Range, ++line};
    ++line;
    for (std::uint64_t index = 0; index < actorCount; ++index, ++line) {
        PerformanceActorPotential actor{};
        if (!expect("actor") || !detail::readActor(input, actor) || !parsed.territory.addActor(actor))
            return {SessionDocumentError::Territory, line};
    }
    std::uint64_t sceneCount = 0;
    if (!expect("scenes") || !detail::readUnsigned(input, sceneCount)
        || sceneCount > kSessionDocumentSceneCapacity) return {SessionDocumentError::Range, line};
    ++line;
    for (std::uint64_t sceneIndex = 0; sceneIndex < sceneCount; ++sceneIndex) {
        PerformanceScene scene{};
        std::uint64_t id = 0;
        if (!expect("scene") || !detail::readUnsigned(input, id) || id > std::numeric_limits<std::uint16_t>::max()
            || !detail::readState(input, scene.session, version >= 4U))
            return {SessionDocumentError::Scene, line};
        scene.id = static_cast<std::uint16_t>(id);
        ++line;
        std::uint64_t constellationActors = 0;
        if (!expect("sceneactors") || !detail::readUnsigned(input, constellationActors)
            || constellationActors > kConstellationActorCapacity) return {SessionDocumentError::Range, line};
        ++line;
        for (std::uint64_t index = 0; index < constellationActors; ++index, ++line) {
            PerformanceActorState actor{};
            if (!expect("sceneactor") || !detail::readActorState(input, actor)
                || !scene.constellation.addActor(actor)) return {SessionDocumentError::Scene, line};
        }
        std::uint64_t relationCount = 0;
        if (!expect("scenerelations") || !detail::readUnsigned(input, relationCount)
            || relationCount > kConstellationRelationCapacity) return {SessionDocumentError::Range, line};
        ++line;
        for (std::uint64_t index = 0; index < relationCount; ++index, ++line) {
            PerformanceRelation relation{};
            if (!expect("scenerelation") || !detail::readRelation(input, relation, version >= 3U)
                || !scene.constellation.addRelation(relation)) return {SessionDocumentError::Scene, line};
        }
        if (!expect("endscene")) return {SessionDocumentError::Syntax, line};
        ++line;
        if (!parsed.addScene(scene)) return {SessionDocumentError::Scene, line};
    }
    if (!expect("end")) return {SessionDocumentError::Syntax, line};
    if (input >> word) return {SessionDocumentError::TrailingData, line};
    if (!parsed.valid()) return {SessionDocumentError::Scene, line};
    destination = parsed;
    return {};
}

} // namespace rasgo::performance
