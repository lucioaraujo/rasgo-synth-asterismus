#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace rasgo::performance {

constexpr std::uint16_t kInvalidActorId = std::numeric_limits<std::uint16_t>::max();
constexpr std::size_t kTerritoryActorCapacity = 96;
constexpr std::size_t kConstellationActorCapacity = 8;
constexpr std::size_t kConstellationRelationCapacity = 16;
constexpr std::size_t kMorphActorCapacity = kConstellationActorCapacity * 2;
constexpr std::size_t kMorphRelationCapacity = kConstellationRelationCapacity * 2;
constexpr std::size_t kPerformanceStemActorCapacity = 8;
// Eight bounded bodies are the structural constellation capacity, and since
// 2026-10-28 also the audible limit: each body plays its own star on its own
// stem. The former limit of four came from 1010 deadline misses on 05/09,
// whose cause was the audio thread running without realtime priority (see
// docs/DIAGNOSTICO_2026-10-28.md); the fourteen stars of the sky together
// cost 0.65 ms of a 10.7 ms block in tests/test_performance_sky.cpp.
#if defined(RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT)
constexpr std::size_t kPerformanceSafeBodyLimit = RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT;
#else
constexpr std::size_t kPerformanceSafeBodyLimit = 8;
#endif
static_assert(kPerformanceSafeBodyLimit <= kPerformanceStemActorCapacity,
              "The safe body limit cannot exceed allocated stem capacity");

enum class PerformanceCapacityProfile : std::uint8_t {
    Compatibility = 0,
    Performance
};

constexpr PerformanceCapacityProfile capacityProfileForCpus(std::size_t logicalCpus) noexcept {
    return logicalCpus >= 4U ? PerformanceCapacityProfile::Performance
                             : PerformanceCapacityProfile::Compatibility;
}

constexpr std::size_t safeBodyLimitForProfile(PerformanceCapacityProfile profile) noexcept {
    // CPU count is only a conservative floor selector. It is not a calibration
    // and must never unlock five to eight stems by itself.
    return profile == PerformanceCapacityProfile::Performance ? 4U : 3U;
}

inline bool finite(double value) noexcept { return std::isfinite(value); }
inline bool unit(double value) noexcept { return finite(value) && value >= 0.0 && value <= 1.0; }
inline double clampUnit(double value) noexcept {
    return std::clamp(finite(value) ? value : 0.0, 0.0, 1.0);
}
inline double lerp(double from, double to, double amount) noexcept {
    const double t = clampUnit(amount);
    return from + (to - from) * t;
}

enum class PerformanceFamily : std::uint8_t {
    Tone, Hit, Texture, Pattern, Rupture, Relation, Count
};

// A family says how an actor can act; a role says what it does in this scene.
enum class PerformanceRole : std::uint8_t {
    Chao, Corpo, Atrito, Contagio, Rasgo, Vestigio, Count
};

using PerformanceRoleMask = std::uint32_t;
constexpr PerformanceRoleMask roleBit(PerformanceRole role) noexcept {
    const auto value = static_cast<std::uint8_t>(role);
    return value < static_cast<std::uint8_t>(PerformanceRole::Count)
        ? (PerformanceRoleMask{1} << value) : 0U;
}
constexpr PerformanceRoleMask allRoles() noexcept {
    return (PerformanceRoleMask{1} << static_cast<std::uint8_t>(PerformanceRole::Count)) - 1U;
}
constexpr bool roleAllowed(PerformanceRoleMask mask, PerformanceRole role) noexcept {
    return (mask & roleBit(role)) != 0U;
}

enum class ActorLifeState : std::uint8_t {
    Dormant, Emerging, Active, Yielding, Ruptured, Recovering, Count
};

enum class PerformanceStemReadiness : std::uint8_t {
    Unreserved, Prepared, Silent, Audible, Rejected, Count
};

enum class RelationKind : std::uint8_t {
    Support, CallResponse, Hocket, Shadow, Contagion, Friction, Infiltration,
    Repair, Orbit, Count
};

enum class RelationDomain : std::uint8_t {
    Pitch, Time, Matter, Space, Dynamics, Memory, Count
};

// Persisted compositional intent for a relation. The audio/geometry behaviour
// lives in PerformanceRelationalPhenomena.hpp; keeping the identifier here
// lets scenes retain that intent without coupling the document contract to a
// renderer or DSP implementation.
enum class PerformanceRelationalPhenomenon : std::uint8_t {
    Attract, Repel, Orbit, EllipticalOrbit, BarycentricDance, Encounter, Merge,
    Flyby, Scatter, Chase, Escape, Parallel, Converge, Diverge, Oscillate,
    SpiralIn, SpiralOut, Equilibrium, Lock, Synchronize, Resonate, Swap,
    Release, Count
};

constexpr bool validRelationalPhenomenon(PerformanceRelationalPhenomenon value) noexcept {
    return static_cast<std::uint8_t>(value)
        < static_cast<std::uint8_t>(PerformanceRelationalPhenomenon::Count);
}

enum class PerformancePopulationPhenomenon : std::uint8_t {
    Align, Cluster, Scatter, CollectiveRotate, Expand, Contract, Pulse,
    Fragment, MergeGroups, Drift, Count
};

constexpr bool validPopulationPhenomenon(PerformancePopulationPhenomenon value) noexcept {
    return static_cast<std::uint8_t>(value)
        < static_cast<std::uint8_t>(PerformancePopulationPhenomenon::Count);
}

// Versioned translation of the Studio GamePlan. These values deliberately
// preserve the core ordering so an RSTM hand-off can cross the application
// boundary without importing the Studio renderer or its mutable DSP state.
enum class PerformanceGameStrategy : std::uint8_t {
    Follow, Transform, Counter, Interrupt, Conceal, Negotiate, Count
};

enum class PerformanceGameRule : std::uint8_t {
    Preserve, Mutate, Oppose, Withhold, Exchange, Break, Count
};

struct PerformanceGamePlan {
    PerformanceGameStrategy strategy = PerformanceGameStrategy::Follow;
    PerformanceGameRule rule = PerformanceGameRule::Preserve;
    double energy = 0.5;
    double memory = 0.5;
    double risk = 0.5;
    double freedom = 0.5;
    double hiddenInformation = 0.5;

    bool valid() const noexcept {
        return static_cast<std::uint8_t>(strategy)
                   < static_cast<std::uint8_t>(PerformanceGameStrategy::Count)
            && static_cast<std::uint8_t>(rule)
                   < static_cast<std::uint8_t>(PerformanceGameRule::Count)
            && unit(energy) && unit(memory) && unit(risk) && unit(freedom)
            && unit(hiddenInformation);
    }
};

enum class ClockSource : std::uint8_t { Internal, Midi, Link, Osc, Count };
enum class PerformanceMacro : std::uint8_t {
    Autonomy, Intervention, Density, Tension, Memory, Risk, Space, Depth, Count
};

// A performer's choice of how the graphic bodies move. It is a bounded audio
// field as well as a visual regime, but never carries actor coordinates.
enum class PerformanceMotionField : std::uint8_t { Orbit, Pulse, FigureEight, Count };
// Famílias de movimento (30/10, decisão do autor: "a dança vira família de
// movimento"). Com a trama, o botão DANÇA escolhe como a física move uma
// estrela (ou o campo inteiro); sem a trama, DANÇA continua sendo ÓRBITA,
// PULSO e FIGURA-8.
enum class PerformanceMotionFamily : std::uint8_t { Free, Billiard, Foam, Pendulum, Magnetic, Gravity, Count };
// O movimento da constelação inteira (30/10, pedido do autor: "giro de
// rotação da constelação, translação"): os lugares de repouso giram em torno
// do centro da constelação e/ou viajam pelo campo; a física os segue.
enum class PerformanceConstellationMotion : std::uint8_t { Still, Rotate, Travel, RotateTravel, Count };
constexpr bool validPerformanceMotionField(PerformanceMotionField field) noexcept {
    return static_cast<std::uint8_t>(field)
        < static_cast<std::uint8_t>(PerformanceMotionField::Count);
}
enum class GestureType : std::uint8_t {
    Hold, Release, Mutate, Rasgar, Interrupt, GridCue, Stutter, TempoShift, Recall, Silence, PanicSafe, Count
};

// Musical grid resolution. The numerical value is the number of entrances per
// beat, keeping scheduling bounded to the existing sample-clock contract.
enum class GestureGridDivision : std::uint8_t { Beat = 1U, Half = 2U, Quarter = 4U };
constexpr bool validGestureGridDivision(GestureGridDivision division) noexcept {
    return division == GestureGridDivision::Beat || division == GestureGridDivision::Half
        || division == GestureGridDivision::Quarter;
}

struct TerritoryIdentity {
    double tonalVocabulary = 0.50;
    double temporalDna = 0.50;
    double matterBias = 0.50;
    double relationBias = 0.50;
    double memoryLimit = 0.70;
    double riskLimit = 0.55;

    bool valid() const noexcept {
        return unit(tonalVocabulary) && unit(temporalDna) && unit(matterBias)
            && unit(relationBias) && unit(memoryLimit) && unit(riskLimit);
    }
};

struct PerformanceActorPotential {
    std::uint16_t id = kInvalidActorId;
    PerformanceFamily family = PerformanceFamily::Texture;
    std::uint32_t capabilities = 0U;
    PerformanceRoleMask allowedRoles = allRoles();
    double salience = 0.50;
    double minimumPresence = 0.0;
    double maximumPresence = 1.0;

    bool valid() const noexcept {
        return id != kInvalidActorId
            && static_cast<std::uint8_t>(family) < static_cast<std::uint8_t>(PerformanceFamily::Count)
            && capabilities != 0U && allowedRoles != 0U && unit(salience)
            && unit(minimumPresence) && unit(maximumPresence)
            && minimumPresence <= maximumPresence;
    }
};

struct PerformanceTerritory {
    std::uint32_t seed = 0U;
    TerritoryIdentity identity{};
    PerformanceGamePlan gamePlan{};
    std::array<PerformanceActorPotential, kTerritoryActorCapacity> actors{};
    std::size_t actorCount = 0;

    bool addActor(const PerformanceActorPotential& actor) noexcept {
        if (!actor.valid() || actorCount >= actors.size() || find(actor.id) != nullptr)
            return false;
        actors[actorCount++] = actor;
        return true;
    }

    const PerformanceActorPotential* find(std::uint16_t id) const noexcept {
        for (std::size_t index = 0; index < actorCount; ++index)
            if (actors[index].id == id) return &actors[index];
        return nullptr;
    }

    bool valid() const noexcept {
        if (!identity.valid() || !gamePlan.valid() || actorCount > actors.size()) return false;
        for (std::size_t left = 0; left < actorCount; ++left) {
            if (!actors[left].valid()) return false;
            for (std::size_t right = left + 1; right < actorCount; ++right)
                if (actors[left].id == actors[right].id) return false;
        }
        return true;
    }
};

struct PerformanceActorState {
    std::uint16_t actorId = kInvalidActorId;
    PerformanceRole role = PerformanceRole::Corpo;
    ActorLifeState life = ActorLifeState::Dormant;
    double presence = 0.0;
    double nourishment = 0.0;
    double observedAudibility = 0.0;
    double gestureBudget = 0.50;

    bool valid() const noexcept {
        return actorId != kInvalidActorId
            && static_cast<std::uint8_t>(role) < static_cast<std::uint8_t>(PerformanceRole::Count)
            && static_cast<std::uint8_t>(life) < static_cast<std::uint8_t>(ActorLifeState::Count)
            && unit(presence) && unit(nourishment) && unit(observedAudibility)
            && unit(gestureBudget);
    }
};

struct PerformanceRelation {
    std::uint16_t sourceActorId = kInvalidActorId;
    std::uint16_t destinationActorId = kInvalidActorId;
    RelationKind kind = RelationKind::Support;
    RelationDomain domain = RelationDomain::Matter;
    double intensity = 0.0;
    double latencyMilliseconds = 0.0;
    double durationSeconds = 0.0;
    double limit = 1.0;
    PerformanceRelationalPhenomenon phenomenon = PerformanceRelationalPhenomenon::Attract;
    double resonanceRatio = 1.0;
    bool phenomenonEnabled = false;

    bool valid() const noexcept {
        return sourceActorId != kInvalidActorId && destinationActorId != kInvalidActorId
            && sourceActorId != destinationActorId
            && static_cast<std::uint8_t>(kind) < static_cast<std::uint8_t>(RelationKind::Count)
            && static_cast<std::uint8_t>(domain) < static_cast<std::uint8_t>(RelationDomain::Count)
            && unit(intensity) && finite(latencyMilliseconds) && latencyMilliseconds >= 0.0
            && latencyMilliseconds <= 2000.0 && finite(durationSeconds)
            && durationSeconds >= 0.0 && durationSeconds <= 120.0 && unit(limit)
            && validRelationalPhenomenon(phenomenon) && finite(resonanceRatio)
            && resonanceRatio >= 0.25 && resonanceRatio <= 4.0;
    }

    bool sameIdentity(const PerformanceRelation& other) const noexcept {
        return sourceActorId == other.sourceActorId
            && destinationActorId == other.destinationActorId && kind == other.kind
            && domain == other.domain;
    }
};

struct PerformanceConstellation {
    std::array<PerformanceActorState, kConstellationActorCapacity> actors{};
    std::size_t actorCount = 0;
    std::array<PerformanceRelation, kConstellationRelationCapacity> relations{};
    std::size_t relationCount = 0;

    bool addActor(const PerformanceActorState& actor) noexcept {
        if (!actor.valid() || actorCount >= actors.size() || findActor(actor.actorId) != nullptr)
            return false;
        actors[actorCount++] = actor;
        return true;
    }

    bool addRelation(const PerformanceRelation& relation) noexcept {
        if (!relation.valid() || relationCount >= relations.size()
            || findRelation(relation) != nullptr)
            return false;
        relations[relationCount++] = relation;
        return true;
    }

    const PerformanceActorState* findActor(std::uint16_t id) const noexcept {
        for (std::size_t index = 0; index < actorCount; ++index)
            if (actors[index].actorId == id) return &actors[index];
        return nullptr;
    }

    bool removeActor(std::uint16_t id) noexcept {
        std::size_t actorIndex = actorCount;
        for (std::size_t index = 0U; index < actorCount; ++index) {
            if (actors[index].actorId == id) { actorIndex = index; break; }
        }
        if (actorIndex == actorCount) return false;
        for (std::size_t index = actorIndex + 1U; index < actorCount; ++index)
            actors[index - 1U] = actors[index];
        --actorCount;
        std::size_t write = 0U;
        for (std::size_t index = 0U; index < relationCount; ++index) {
            const auto& relation = relations[index];
            if (relation.sourceActorId == id || relation.destinationActorId == id) continue;
            relations[write++] = relation;
        }
        relationCount = write;
        return true;
    }

    const PerformanceRelation* findRelation(const PerformanceRelation& relation) const noexcept {
        for (std::size_t index = 0; index < relationCount; ++index)
            if (relations[index].sameIdentity(relation)) return &relations[index];
        return nullptr;
    }

    bool valid(const PerformanceTerritory& territory) const noexcept {
        if (!territory.valid() || actorCount > actors.size() || relationCount > relations.size())
            return false;
        for (std::size_t index = 0; index < actorCount; ++index) {
            const auto& state = actors[index];
            const auto* potential = territory.find(state.actorId);
            if (!state.valid() || potential == nullptr || !roleAllowed(potential->allowedRoles, state.role)
                || state.presence < potential->minimumPresence
                || state.presence > potential->maximumPresence)
                return false;
        }
        for (std::size_t index = 0; index < relationCount; ++index) {
            const auto& relation = relations[index];
            if (!relation.valid() || findActor(relation.sourceActorId) == nullptr
                || findActor(relation.destinationActorId) == nullptr)
                return false;
        }
        return true;
    }
};

struct PerformanceSessionState {
    std::uint32_t territorySeed = 0U;
    double tempoBpm = 120.0;
    ClockSource clock = ClockSource::Internal;
    bool playing = false;
    double autonomy = 0.65;
    double intervention = 0.35;
    double density = 0.50;
    double tension = 0.35;
    double memory = 0.50;
    double risk = 0.35;
    double space = 0.50;
    double depth = 0.50;
    std::uint16_t currentSceneId = 0U;
    std::uint16_t targetSceneId = 0U;
    double morphSeconds = 1.0;
    double outputTrimDb = 0.0;
    bool panicSafe = false;
    PerformancePopulationPhenomenon populationPhenomenon
        = PerformancePopulationPhenomenon::CollectiveRotate;
    double populationPhenomenonIntensity = 0.0;
    bool populationPhenomenonEnabled = false;

    bool valid() const noexcept {
        return finite(tempoBpm) && tempoBpm >= 20.0 && tempoBpm <= 300.0
            && static_cast<std::uint8_t>(clock) < static_cast<std::uint8_t>(ClockSource::Count)
            && unit(autonomy) && unit(intervention) && unit(density) && unit(tension)
            && unit(memory) && unit(risk) && unit(space) && finite(morphSeconds)
            && morphSeconds >= 0.0 && morphSeconds <= 30.0 && finite(outputTrimDb)
            && outputTrimDb >= -24.0 && outputTrimDb <= 12.0
            && validPopulationPhenomenon(populationPhenomenon)
            && unit(populationPhenomenonIntensity);
    }
};

struct PerformanceGestureEvent {
    GestureType type = GestureType::Hold;
    std::uint64_t sampleTimestamp = 0U;
    std::uint32_t blockOffset = 0U;
    std::uint16_t targetActorId = kInvalidActorId; // invalid id means global gesture
    double intensity = 0.50;
    double duration = 0.0;
    double condition = 1.0;
    // A control-side variation token. It travels through the existing bounded
    // gesture queue and is only interpreted by MUTATE; audio never gathers
    // entropy, allocates or performs I/O to create a wild variation.
    std::uint32_t variationToken = 0U;

    bool valid(std::uint32_t maximumBlockSize) const noexcept {
        return maximumBlockSize > 0U && blockOffset < maximumBlockSize
            && static_cast<std::uint8_t>(type) < static_cast<std::uint8_t>(GestureType::Count)
            && unit(intensity) && unit(duration) && unit(condition);
    }
};

template <typename Event, std::size_t Capacity>
class FixedEventQueue {
    static_assert(Capacity >= 2, "A fixed event queue needs one spare slot");

public:
    bool push(const Event& event) noexcept {
        const auto write = writeIndex_.load(std::memory_order_relaxed);
        const auto next = increment(write);
        if (next == readIndex_.load(std::memory_order_acquire)) return false;
        events_[write] = event;
        writeIndex_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(Event& event) noexcept {
        const auto read = readIndex_.load(std::memory_order_relaxed);
        if (read == writeIndex_.load(std::memory_order_acquire)) return false;
        event = events_[read];
        readIndex_.store(increment(read), std::memory_order_release);
        return true;
    }

    std::size_t size() const noexcept {
        const auto read = readIndex_.load(std::memory_order_acquire);
        const auto write = writeIndex_.load(std::memory_order_acquire);
        return write >= read ? write - read : Capacity - read + write;
    }

    // Only call while producer and consumer are quiescent, such as before a
    // device starts or after it stops. Never reset a live audio queue.
    void reset() noexcept {
        readIndex_.store(0U, std::memory_order_relaxed);
        writeIndex_.store(0U, std::memory_order_relaxed);
    }

private:
    static constexpr std::size_t increment(std::size_t index) noexcept {
        return (index + 1U) % Capacity;
    }

    std::array<Event, Capacity> events_{};
    std::atomic<std::size_t> readIndex_{0U};
    std::atomic<std::size_t> writeIndex_{0U};
};

struct PerformanceTelemetrySnapshot {
    std::uint32_t cpuMilliPercent = 0U;
    std::uint32_t outputPeakMicros = 0U;
    std::uint32_t truePeakMicros = 0U;
    // Loudness da saída (30/10, BS.1770), em centésimos de LUFS (−7000 = sem
    // sinal): momentânea (400 ms) e de curto prazo (3 s).
    std::int16_t momentaryCentiLufs = -7000;
    std::int16_t shortTermCentiLufs = -7000;
    // Desde o PLAY: loudness integrada (portas BS.1770), maior true peak e a
    // maior redução de ganho do limitador (centésimos de dB; 0 = não agiu).
    std::int16_t integratedCentiLufs = -7000;
    std::uint32_t maxTruePeakMicros = 0U;
    std::uint16_t limiterMaxReductionCentiDb = 0U;
    std::uint32_t xrunCount = 0U;
    std::uint16_t activeActorCount = 0U;
    std::uint16_t sceneId = 0U;
    std::uint16_t eventQueueDepth = 0U;
    bool playing = false;
    bool holding = false;
    bool quantizedGestures = false;
    std::uint8_t gestureGridDivision = static_cast<std::uint8_t>(GestureGridDivision::Beat);
    bool panicSafe = false;
    // The closing verb is published as bounded scalar telemetry so archival
    // work never reads mutable ecology state from the control thread.
    std::uint8_t codaGesture = 0U;
    std::uint32_t codaReadinessMicros = 0U;
    // Last autonomous seed turn, carried as bounded scalar data so telemetry
    // remains independent from the spatial-game header.
    bool narrativeEventValid = false;
    std::uint32_t narrativeTurn = 0U;
    std::uint8_t narrativeMove = 0U;
    std::uint8_t narrativeStrategy = 0U;
    std::uint8_t narrativeRule = 0U;
    // The visual body reads this same bounded phase and field used by the
    // audio ecology; it must never animate on an independent GUI clock.
    std::uint32_t motionPhaseMicros = 0U;
    std::uint8_t motionField = static_cast<std::uint8_t>(PerformanceMotionField::Orbit);
    // A trama do céu, para o desenho da Constelação: até três relações em
    // foco empacotadas (bits 0–1: quantidade; por relação, 10 bits: quem
    // chama 3, quem responde 3, tipo 4), a tensão harmônica e o brilho de
    // cada estrela (variação do mixer).
    std::uint32_t skyFocusPacked = 0U;
    std::uint32_t skyTensionMicros = 0U;
    // Seção da forma (0 abertura, 1 desenvolvimento, 2 contraste, 3 retorno).
    std::uint8_t skySection = 0U;
    // A gramática da forma (5/11) e as voltas que ela já completou.
    std::uint8_t skyGrammar = 0U;
    std::uint8_t skyFormPass = 0U;
    // As estrelas convidadas do contraste (5/11): por convidada, 8 bits do
    // corpo + 1 e 8 bits da estrela; 0 = nenhuma.
    std::uint32_t skyGuests = 0U;
    // O último corte do céu (6/11): 4 bits de contador e 4 bits do tipo.
    std::uint8_t skyCut = 0U;
    // Tempo maleável: fator de andamento (em milésimos) e swing (% da
    // primeira colcheia, 50 = reto).
    std::uint16_t skyTempoMillis = 1000U;
    std::uint8_t skySwingPercent = 50U;
    // Agrupamento do compasso (bit = início de grupo; 0 sem contagens) e o
    // corpo que deriva no desenvolvimento (255: nenhum).
    std::uint16_t skyGroupingMask = 0U;
    std::uint8_t skyDriftBody = 255U;
    // O corpo encoberto por um eclipse agora (255: nenhum).
    std::uint8_t skyEclipseBody = 255U;
    std::array<std::uint32_t, kPerformanceStemActorCapacity> starBrightnessMicros{};
    // Para a Constelação: a fase de cada corpo no pulso (0–255 = uma volta;
    // o coro de fases) e os corpos que derivam e são encobertos (por
    // posição; 255: nenhum).
    std::array<std::uint8_t, kPerformanceStemActorCapacity> skyChorusPhase{};
    std::uint8_t skyDriftSlot = 255U;
    std::uint8_t skyEclipseSlot = 255U;
    // O movimento físico (29/10): o lugar de cada corpo no Campo (0–65535),
    // válido só com a trama ligada.
    bool skyMotionValid = false;
    std::array<std::uint16_t, kPerformanceStemActorCapacity> skyPositionX{};
    std::array<std::uint16_t, kPerformanceStemActorCapacity> skyPositionY{};
    std::uint16_t stemCount = 0U;
    std::array<std::uint16_t, kPerformanceStemActorCapacity> stemActorIds{
        kInvalidActorId, kInvalidActorId, kInvalidActorId, kInvalidActorId, kInvalidActorId};
    std::array<std::uint8_t, kPerformanceStemActorCapacity> stemMotionFields{
        static_cast<std::uint8_t>(PerformanceMotionField::Orbit),
        static_cast<std::uint8_t>(PerformanceMotionField::Orbit),
        static_cast<std::uint8_t>(PerformanceMotionField::Orbit),
        static_cast<std::uint8_t>(PerformanceMotionField::Orbit),
        static_cast<std::uint8_t>(PerformanceMotionField::Orbit)};
    std::array<std::uint8_t, kPerformanceStemActorCapacity> stemMotionDirections{1U, 1U, 1U, 1U, 1U};
    std::array<std::uint32_t, kPerformanceStemActorCapacity> stemPeakMicros{};
    // Amount of an internal module layer currently entering this stem.
    // It is a bounded gain coefficient, not a count of extra stems.
    std::array<std::uint32_t, kPerformanceStemActorCapacity> stemLayerMixMicros{};
    std::array<std::uint8_t, kPerformanceStemActorCapacity> stemLifeStates{};
    std::array<std::uint8_t, kPerformanceStemActorCapacity> stemReadiness{
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved)};
    std::uint32_t modalReturnPeakMicros = 0U;
    std::uint32_t scarMicros = 0U;
    std::uint32_t repairMicros = 0U;
    std::uint32_t wildMicros = 0U;

    bool valid() const noexcept {
        return cpuMilliPercent <= 100000U && outputPeakMicros <= 1000000U
            && truePeakMicros <= 1000000U && stemCount <= stemPeakMicros.size()
            && (gestureGridDivision == static_cast<std::uint8_t>(GestureGridDivision::Beat)
                || gestureGridDivision == static_cast<std::uint8_t>(GestureGridDivision::Half)
                || gestureGridDivision == static_cast<std::uint8_t>(GestureGridDivision::Quarter))
            && codaGesture < 4U
            && codaReadinessMicros <= 1000000U
            && (!narrativeEventValid || (narrativeMove < 7U && narrativeStrategy < 6U
                                         && narrativeRule < 6U))
            && motionPhaseMicros <= 1000000U
            && motionField < static_cast<std::uint8_t>(PerformanceMotionField::Count)
            && modalReturnPeakMicros <= 1000000U
            && scarMicros <= 1000000U
            && repairMicros <= 1000000U
            && wildMicros <= 1000000U
            && std::all_of(stemPeakMicros.begin(), stemPeakMicros.end(),
                           [](std::uint32_t peak) { return peak <= 1000000U; })
            && std::all_of(stemLayerMixMicros.begin(), stemLayerMixMicros.end(),
                           [](std::uint32_t mix) { return mix <= 1000000U; })
            && std::all_of(stemLifeStates.begin(), stemLifeStates.end(),
                           [](std::uint8_t life) {
                               return life < static_cast<std::uint8_t>(ActorLifeState::Count);
                           })
            && std::all_of(stemReadiness.begin(), stemReadiness.end(),
                           [](std::uint8_t readiness) {
                               return readiness < static_cast<std::uint8_t>(PerformanceStemReadiness::Count);
                           })
            && std::all_of(stemMotionFields.begin(), stemMotionFields.end(),
                           [](std::uint8_t field) {
                               return field < static_cast<std::uint8_t>(PerformanceMotionField::Count);
                           })
            && std::all_of(stemMotionDirections.begin(), stemMotionDirections.end(),
                           [](std::uint8_t direction) { return direction <= 1U; });
    }
};

class PerformanceTelemetryMailbox {
public:
    void publish(const PerformanceTelemetrySnapshot& snapshot) noexcept {
        cpuMilliPercent_.store(std::min(snapshot.cpuMilliPercent, 100000U), std::memory_order_relaxed);
        outputPeakMicros_.store(std::min(snapshot.outputPeakMicros, 1000000U), std::memory_order_relaxed);
        truePeakMicros_.store(std::min(snapshot.truePeakMicros, 1000000U), std::memory_order_relaxed);
        momentaryCentiLufs_.store(snapshot.momentaryCentiLufs, std::memory_order_relaxed);
        shortTermCentiLufs_.store(snapshot.shortTermCentiLufs, std::memory_order_relaxed);
        integratedCentiLufs_.store(snapshot.integratedCentiLufs, std::memory_order_relaxed);
        maxTruePeakMicros_.store(snapshot.maxTruePeakMicros, std::memory_order_relaxed);
        limiterMaxReductionCentiDb_.store(snapshot.limiterMaxReductionCentiDb, std::memory_order_relaxed);
        xrunCount_.store(snapshot.xrunCount, std::memory_order_relaxed);
        activeActorCount_.store(snapshot.activeActorCount, std::memory_order_relaxed);
        sceneId_.store(snapshot.sceneId, std::memory_order_relaxed);
        eventQueueDepth_.store(snapshot.eventQueueDepth, std::memory_order_relaxed);
        playing_.store(snapshot.playing, std::memory_order_relaxed);
        holding_.store(snapshot.holding, std::memory_order_relaxed);
        quantizedGestures_.store(snapshot.quantizedGestures, std::memory_order_relaxed);
        gestureGridDivision_.store(snapshot.gestureGridDivision, std::memory_order_relaxed);
        panicSafe_.store(snapshot.panicSafe, std::memory_order_relaxed);
        codaGesture_.store(snapshot.codaGesture < 4U ? snapshot.codaGesture : 0U,
                           std::memory_order_relaxed);
        codaReadinessMicros_.store(std::min(snapshot.codaReadinessMicros, 1000000U),
                                   std::memory_order_relaxed);
        narrativeEventValid_.store(snapshot.narrativeEventValid, std::memory_order_relaxed);
        narrativeTurn_.store(snapshot.narrativeTurn, std::memory_order_relaxed);
        narrativeMove_.store(snapshot.narrativeMove < 7U ? snapshot.narrativeMove : 0U,
                             std::memory_order_relaxed);
        narrativeStrategy_.store(snapshot.narrativeStrategy < 6U ? snapshot.narrativeStrategy : 0U,
                                 std::memory_order_relaxed);
        narrativeRule_.store(snapshot.narrativeRule < 6U ? snapshot.narrativeRule : 0U,
                             std::memory_order_relaxed);
        motionPhaseMicros_.store(std::min(snapshot.motionPhaseMicros, 1000000U),
                                 std::memory_order_relaxed);
        motionField_.store(snapshot.motionField < static_cast<std::uint8_t>(PerformanceMotionField::Count)
                               ? snapshot.motionField
                               : static_cast<std::uint8_t>(PerformanceMotionField::Orbit),
                           std::memory_order_relaxed);
        skyFocusPacked_.store(snapshot.skyFocusPacked, std::memory_order_relaxed);
        skyTensionMicros_.store(std::min(snapshot.skyTensionMicros, 1000000U), std::memory_order_relaxed);
        skySection_.store(snapshot.skySection < 4U ? snapshot.skySection : 0U, std::memory_order_relaxed);
        skyGrammar_.store(snapshot.skyGrammar, std::memory_order_relaxed);
        skyFormPass_.store(snapshot.skyFormPass, std::memory_order_relaxed);
        skyGuests_.store(snapshot.skyGuests, std::memory_order_relaxed);
        skyCut_.store(snapshot.skyCut, std::memory_order_relaxed);
        skyTempoMillis_.store(snapshot.skyTempoMillis, std::memory_order_relaxed);
        skySwingPercent_.store(snapshot.skySwingPercent, std::memory_order_relaxed);
        skyGroupingMask_.store(snapshot.skyGroupingMask, std::memory_order_relaxed);
        skyDriftBody_.store(snapshot.skyDriftBody, std::memory_order_relaxed);
        skyEclipseBody_.store(snapshot.skyEclipseBody, std::memory_order_relaxed);
        for (std::size_t index = 0U; index < kPerformanceStemActorCapacity; ++index)
            starBrightnessMicros_[index].store(std::min(snapshot.starBrightnessMicros[index], 1000000U),
                                               std::memory_order_relaxed);
        for (std::size_t index = 0U; index < kPerformanceStemActorCapacity; ++index)
            skyChorusPhase_[index].store(snapshot.skyChorusPhase[index], std::memory_order_relaxed);
        skyDriftSlot_.store(snapshot.skyDriftSlot, std::memory_order_relaxed);
        skyEclipseSlot_.store(snapshot.skyEclipseSlot, std::memory_order_relaxed);
        skyMotionValid_.store(snapshot.skyMotionValid, std::memory_order_relaxed);
        for (std::size_t index = 0U; index < kPerformanceStemActorCapacity; ++index) {
            skyPositionX_[index].store(snapshot.skyPositionX[index], std::memory_order_relaxed);
            skyPositionY_[index].store(snapshot.skyPositionY[index], std::memory_order_relaxed);
        }
        stemCount_.store(std::min<std::uint16_t>(snapshot.stemCount,
                         static_cast<std::uint16_t>(kPerformanceStemActorCapacity)),
                         std::memory_order_relaxed);
        for (std::size_t index = 0U; index < kPerformanceStemActorCapacity; ++index) {
            stemActorIds_[index].store(snapshot.stemActorIds[index], std::memory_order_relaxed);
            stemMotionFields_[index].store(
                snapshot.stemMotionFields[index] < static_cast<std::uint8_t>(PerformanceMotionField::Count)
                    ? snapshot.stemMotionFields[index]
                    : static_cast<std::uint8_t>(PerformanceMotionField::Orbit),
                std::memory_order_relaxed);
            stemMotionDirections_[index].store(snapshot.stemMotionDirections[index] <= 1U
                    ? snapshot.stemMotionDirections[index] : 1U, std::memory_order_relaxed);
            stemPeakMicros_[index].store(std::min(snapshot.stemPeakMicros[index], 1000000U),
                                         std::memory_order_relaxed);
            stemLayerMixMicros_[index].store(std::min(snapshot.stemLayerMixMicros[index], 1000000U),
                                             std::memory_order_relaxed);
            stemLifeStates_[index].store(
                snapshot.stemLifeStates[index] < static_cast<std::uint8_t>(ActorLifeState::Count)
                    ? snapshot.stemLifeStates[index] : static_cast<std::uint8_t>(ActorLifeState::Dormant),
                std::memory_order_relaxed);
            stemReadiness_[index].store(
                snapshot.stemReadiness[index] < static_cast<std::uint8_t>(PerformanceStemReadiness::Count)
                    ? snapshot.stemReadiness[index]
                    : static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
                std::memory_order_relaxed);
        }
        modalReturnPeakMicros_.store(std::min(snapshot.modalReturnPeakMicros, 1000000U),
                                     std::memory_order_relaxed);
        scarMicros_.store(std::min(snapshot.scarMicros, 1000000U), std::memory_order_relaxed);
        repairMicros_.store(std::min(snapshot.repairMicros, 1000000U), std::memory_order_relaxed);
        wildMicros_.store(std::min(snapshot.wildMicros, 1000000U), std::memory_order_relaxed);
    }

    PerformanceTelemetrySnapshot snapshot() const noexcept {
        PerformanceTelemetrySnapshot out;
        out.cpuMilliPercent = cpuMilliPercent_.load(std::memory_order_relaxed);
        out.outputPeakMicros = outputPeakMicros_.load(std::memory_order_relaxed);
        out.truePeakMicros = truePeakMicros_.load(std::memory_order_relaxed);
        out.momentaryCentiLufs = momentaryCentiLufs_.load(std::memory_order_relaxed);
        out.shortTermCentiLufs = shortTermCentiLufs_.load(std::memory_order_relaxed);
        out.integratedCentiLufs = integratedCentiLufs_.load(std::memory_order_relaxed);
        out.maxTruePeakMicros = maxTruePeakMicros_.load(std::memory_order_relaxed);
        out.limiterMaxReductionCentiDb = limiterMaxReductionCentiDb_.load(std::memory_order_relaxed);
        out.xrunCount = xrunCount_.load(std::memory_order_relaxed);
        out.activeActorCount = activeActorCount_.load(std::memory_order_relaxed);
        out.sceneId = sceneId_.load(std::memory_order_relaxed);
        out.eventQueueDepth = eventQueueDepth_.load(std::memory_order_relaxed);
        out.playing = playing_.load(std::memory_order_relaxed);
        out.holding = holding_.load(std::memory_order_relaxed);
        out.quantizedGestures = quantizedGestures_.load(std::memory_order_relaxed);
        out.gestureGridDivision = gestureGridDivision_.load(std::memory_order_relaxed);
        out.panicSafe = panicSafe_.load(std::memory_order_relaxed);
        out.codaGesture = codaGesture_.load(std::memory_order_relaxed);
        out.codaReadinessMicros = codaReadinessMicros_.load(std::memory_order_relaxed);
        out.narrativeEventValid = narrativeEventValid_.load(std::memory_order_relaxed);
        out.narrativeTurn = narrativeTurn_.load(std::memory_order_relaxed);
        out.narrativeMove = narrativeMove_.load(std::memory_order_relaxed);
        out.narrativeStrategy = narrativeStrategy_.load(std::memory_order_relaxed);
        out.narrativeRule = narrativeRule_.load(std::memory_order_relaxed);
        out.motionPhaseMicros = motionPhaseMicros_.load(std::memory_order_relaxed);
        out.motionField = motionField_.load(std::memory_order_relaxed);
        out.skyFocusPacked = skyFocusPacked_.load(std::memory_order_relaxed);
        out.skyTensionMicros = skyTensionMicros_.load(std::memory_order_relaxed);
        out.skySection = skySection_.load(std::memory_order_relaxed);
        out.skyGrammar = skyGrammar_.load(std::memory_order_relaxed);
        out.skyFormPass = skyFormPass_.load(std::memory_order_relaxed);
        out.skyGuests = skyGuests_.load(std::memory_order_relaxed);
        out.skyCut = skyCut_.load(std::memory_order_relaxed);
        out.skyTempoMillis = skyTempoMillis_.load(std::memory_order_relaxed);
        out.skySwingPercent = skySwingPercent_.load(std::memory_order_relaxed);
        out.skyGroupingMask = skyGroupingMask_.load(std::memory_order_relaxed);
        out.skyDriftBody = skyDriftBody_.load(std::memory_order_relaxed);
        out.skyEclipseBody = skyEclipseBody_.load(std::memory_order_relaxed);
        for (std::size_t index = 0U; index < kPerformanceStemActorCapacity; ++index)
            out.starBrightnessMicros[index] = starBrightnessMicros_[index].load(std::memory_order_relaxed);
        for (std::size_t index = 0U; index < kPerformanceStemActorCapacity; ++index)
            out.skyChorusPhase[index] = skyChorusPhase_[index].load(std::memory_order_relaxed);
        out.skyDriftSlot = skyDriftSlot_.load(std::memory_order_relaxed);
        out.skyEclipseSlot = skyEclipseSlot_.load(std::memory_order_relaxed);
        out.skyMotionValid = skyMotionValid_.load(std::memory_order_relaxed);
        for (std::size_t index = 0U; index < kPerformanceStemActorCapacity; ++index) {
            out.skyPositionX[index] = skyPositionX_[index].load(std::memory_order_relaxed);
            out.skyPositionY[index] = skyPositionY_[index].load(std::memory_order_relaxed);
        }
        out.stemCount = stemCount_.load(std::memory_order_relaxed);
        for (std::size_t index = 0U; index < kPerformanceStemActorCapacity; ++index) {
            out.stemActorIds[index] = stemActorIds_[index].load(std::memory_order_relaxed);
            out.stemMotionFields[index] = stemMotionFields_[index].load(std::memory_order_relaxed);
            out.stemMotionDirections[index] = stemMotionDirections_[index].load(std::memory_order_relaxed);
            out.stemPeakMicros[index] = stemPeakMicros_[index].load(std::memory_order_relaxed);
            out.stemLayerMixMicros[index] = stemLayerMixMicros_[index].load(std::memory_order_relaxed);
            out.stemLifeStates[index] = stemLifeStates_[index].load(std::memory_order_relaxed);
            out.stemReadiness[index] = stemReadiness_[index].load(std::memory_order_relaxed);
        }
        out.modalReturnPeakMicros = modalReturnPeakMicros_.load(std::memory_order_relaxed);
        out.scarMicros = scarMicros_.load(std::memory_order_relaxed);
        out.repairMicros = repairMicros_.load(std::memory_order_relaxed);
        out.wildMicros = wildMicros_.load(std::memory_order_relaxed);
        return out;
    }

private:
    std::atomic<std::uint32_t> cpuMilliPercent_{0U};
    std::atomic<std::uint32_t> outputPeakMicros_{0U};
    std::atomic<std::uint32_t> truePeakMicros_{0U};
    std::atomic<std::int16_t> momentaryCentiLufs_{-7000};
    std::atomic<std::int16_t> shortTermCentiLufs_{-7000};
    std::atomic<std::int16_t> integratedCentiLufs_{-7000};
    std::atomic<std::uint32_t> maxTruePeakMicros_{0U};
    std::atomic<std::uint16_t> limiterMaxReductionCentiDb_{0U};
    std::atomic<std::uint32_t> xrunCount_{0U};
    std::atomic<std::uint16_t> activeActorCount_{0U};
    std::atomic<std::uint16_t> sceneId_{0U};
    std::atomic<std::uint16_t> eventQueueDepth_{0U};
    std::atomic<bool> playing_{false};
    std::atomic<bool> holding_{false};
    std::atomic<bool> quantizedGestures_{false};
    std::atomic<std::uint8_t> gestureGridDivision_{static_cast<std::uint8_t>(GestureGridDivision::Beat)};
    std::atomic<bool> panicSafe_{false};
    std::atomic<std::uint8_t> codaGesture_{0U};
    std::atomic<std::uint32_t> codaReadinessMicros_{0U};
    std::atomic<bool> narrativeEventValid_{false};
    std::atomic<std::uint32_t> narrativeTurn_{0U};
    std::atomic<std::uint8_t> narrativeMove_{0U};
    std::atomic<std::uint8_t> narrativeStrategy_{0U};
    std::atomic<std::uint8_t> narrativeRule_{0U};
    std::atomic<std::uint32_t> motionPhaseMicros_{0U};
    std::atomic<std::uint8_t> motionField_{static_cast<std::uint8_t>(PerformanceMotionField::Orbit)};
    std::atomic<std::uint32_t> skyFocusPacked_{0U};
    std::atomic<std::uint32_t> skyTensionMicros_{0U};
    std::atomic<std::uint8_t> skySection_{0U};
    std::atomic<std::uint8_t> skyGrammar_{0U};
    std::atomic<std::uint8_t> skyFormPass_{0U};
    std::atomic<std::uint32_t> skyGuests_{0U};
    std::atomic<std::uint8_t> skyCut_{0U};
    std::atomic<std::uint16_t> skyTempoMillis_{1000U};
    std::atomic<std::uint8_t> skySwingPercent_{50U};
    std::atomic<std::uint16_t> skyGroupingMask_{0U};
    std::atomic<std::uint8_t> skyDriftBody_{255U};
    std::atomic<std::uint8_t> skyEclipseBody_{255U};
    std::array<std::atomic<std::uint32_t>, kPerformanceStemActorCapacity> starBrightnessMicros_{};
    std::array<std::atomic<std::uint8_t>, kPerformanceStemActorCapacity> skyChorusPhase_{};
    std::atomic<std::uint8_t> skyDriftSlot_{255U};
    std::atomic<std::uint8_t> skyEclipseSlot_{255U};
    std::atomic<bool> skyMotionValid_{false};
    std::array<std::atomic<std::uint16_t>, kPerformanceStemActorCapacity> skyPositionX_{};
    std::array<std::atomic<std::uint16_t>, kPerformanceStemActorCapacity> skyPositionY_{};
    std::atomic<std::uint16_t> stemCount_{0U};
    std::array<std::atomic<std::uint16_t>, kPerformanceStemActorCapacity> stemActorIds_{{
        kInvalidActorId, kInvalidActorId, kInvalidActorId, kInvalidActorId, kInvalidActorId}};
    std::array<std::atomic<std::uint8_t>, kPerformanceStemActorCapacity> stemMotionFields_{{
        static_cast<std::uint8_t>(PerformanceMotionField::Orbit),
        static_cast<std::uint8_t>(PerformanceMotionField::Orbit),
        static_cast<std::uint8_t>(PerformanceMotionField::Orbit),
        static_cast<std::uint8_t>(PerformanceMotionField::Orbit),
        static_cast<std::uint8_t>(PerformanceMotionField::Orbit)}};
    std::array<std::atomic<std::uint8_t>, kPerformanceStemActorCapacity> stemMotionDirections_{{1U, 1U, 1U, 1U, 1U}};
    std::array<std::atomic<std::uint32_t>, kPerformanceStemActorCapacity> stemPeakMicros_{{
        0U, 0U, 0U, 0U, 0U}};
    std::array<std::atomic<std::uint32_t>, kPerformanceStemActorCapacity> stemLayerMixMicros_{{
        0U, 0U, 0U, 0U, 0U}};
    std::array<std::atomic<std::uint8_t>, kPerformanceStemActorCapacity> stemLifeStates_{{
        static_cast<std::uint8_t>(ActorLifeState::Dormant),
        static_cast<std::uint8_t>(ActorLifeState::Dormant),
        static_cast<std::uint8_t>(ActorLifeState::Dormant),
        static_cast<std::uint8_t>(ActorLifeState::Dormant),
        static_cast<std::uint8_t>(ActorLifeState::Dormant)}};
    std::array<std::atomic<std::uint8_t>, kPerformanceStemActorCapacity> stemReadiness_{{
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved)}};
    std::atomic<std::uint32_t> modalReturnPeakMicros_{0U};
    std::atomic<std::uint32_t> scarMicros_{0U};
    std::atomic<std::uint32_t> repairMicros_{0U};
    std::atomic<std::uint32_t> wildMicros_{0U};
};

struct PerformanceScene {
    std::uint16_t id = 0U;
    std::array<char, 48> name{};
    PerformanceSessionState session{};
    PerformanceConstellation constellation{};

    bool valid(const PerformanceTerritory& territory) const noexcept {
        return session.territorySeed == territory.seed && session.valid()
            && constellation.valid(territory);
    }
};

struct MorphedActorState {
    std::uint16_t actorId = kInvalidActorId;
    PerformanceRole sourceRole = PerformanceRole::Corpo;
    PerformanceRole targetRole = PerformanceRole::Corpo;
    ActorLifeState sourceLife = ActorLifeState::Dormant;
    ActorLifeState targetLife = ActorLifeState::Dormant;
    double roleMorph = 0.0;
    double presence = 0.0;
    double nourishment = 0.0;
    double observedAudibility = 0.0;
    double gestureBudget = 0.0;
};

struct MorphedRelation {
    PerformanceRelation relation{};
    double activity = 0.0;
};

struct PerformanceSceneMorph {
    PerformanceSessionState session{};
    std::array<MorphedActorState, kMorphActorCapacity> actors{};
    std::size_t actorCount = 0;
    std::array<MorphedRelation, kMorphRelationCapacity> relations{};
    std::size_t relationCount = 0;
    double amount = 0.0;
    bool compatibleTerritory = true;
};

class PerformanceSceneMorpher {
public:
    static PerformanceSceneMorph morph(const PerformanceScene& source,
                                       const PerformanceScene& target,
                                       double amount) noexcept {
        PerformanceSceneMorph out;
        out.amount = clampUnit(amount);
        out.compatibleTerritory = source.session.territorySeed == target.session.territorySeed;
        if (!out.compatibleTerritory) {
            out.amount = 0.0;
            out.session = source.session;
            appendActors(source.constellation, source.constellation, 0.0, out);
            appendRelations(source.constellation, source.constellation, 0.0, out);
            return out;
        }
        out.session = morphSession(source.session, target.session, out.amount);
        appendActors(source.constellation, target.constellation, out.amount, out);
        appendRelations(source.constellation, target.constellation, out.amount, out);
        return out;
    }

private:
    static PerformanceSessionState morphSession(const PerformanceSessionState& source,
                                                const PerformanceSessionState& target,
                                                double amount) noexcept {
        PerformanceSessionState out = source;
        out.territorySeed = source.territorySeed;
        out.tempoBpm = lerp(source.tempoBpm, target.tempoBpm, amount);
        out.clock = amount < 0.5 ? source.clock : target.clock;
        out.playing = amount < 0.5 ? source.playing : target.playing;
        out.autonomy = lerp(source.autonomy, target.autonomy, amount);
        out.intervention = lerp(source.intervention, target.intervention, amount);
        out.density = lerp(source.density, target.density, amount);
        out.tension = lerp(source.tension, target.tension, amount);
        out.memory = lerp(source.memory, target.memory, amount);
        out.risk = lerp(source.risk, target.risk, amount);
        out.space = lerp(source.space, target.space, amount);
        out.depth = lerp(source.depth, target.depth, amount);
        out.currentSceneId = amount < 1.0 ? source.currentSceneId : target.currentSceneId;
        out.targetSceneId = target.targetSceneId;
        out.morphSeconds = lerp(source.morphSeconds, target.morphSeconds, amount);
        out.outputTrimDb = lerp(source.outputTrimDb, target.outputTrimDb, amount);
        out.panicSafe = source.panicSafe || target.panicSafe;
        return out;
    }

    static const PerformanceActorState* findActor(const PerformanceConstellation& constellation,
                                                  std::uint16_t id) noexcept {
        return constellation.findActor(id);
    }

    static void appendActors(const PerformanceConstellation& source,
                             const PerformanceConstellation& target,
                             double amount, PerformanceSceneMorph& out) noexcept {
        std::array<std::uint16_t, kMorphActorCapacity> ids{};
        std::size_t idCount = 0;
        const auto appendId = [&ids, &idCount](std::uint16_t id) noexcept {
            for (std::size_t i = 0; i < idCount; ++i) if (ids[i] == id) return;
            ids[idCount++] = id;
        };
        for (std::size_t i = 0; i < source.actorCount; ++i) appendId(source.actors[i].actorId);
        for (std::size_t i = 0; i < target.actorCount; ++i) appendId(target.actors[i].actorId);
        std::sort(ids.begin(), ids.begin() + static_cast<std::ptrdiff_t>(idCount));
        for (std::size_t i = 0; i < idCount; ++i) {
            const auto* from = findActor(source, ids[i]);
            const auto* to = findActor(target, ids[i]);
            MorphedActorState actor;
            actor.actorId = ids[i];
            actor.sourceRole = from != nullptr ? from->role : to->role;
            actor.targetRole = to != nullptr ? to->role : actor.sourceRole;
            actor.sourceLife = from != nullptr ? from->life : ActorLifeState::Dormant;
            actor.targetLife = to != nullptr ? to->life : ActorLifeState::Dormant;
            actor.roleMorph = from != nullptr && to != nullptr && from->role != to->role ? amount : 0.0;
            actor.presence = lerp(from != nullptr ? from->presence : 0.0,
                                  to != nullptr ? to->presence : 0.0, amount);
            actor.nourishment = lerp(from != nullptr ? from->nourishment : 0.0,
                                     to != nullptr ? to->nourishment : 0.0, amount);
            actor.observedAudibility = lerp(from != nullptr ? from->observedAudibility : 0.0,
                                             to != nullptr ? to->observedAudibility : 0.0, amount);
            actor.gestureBudget = lerp(from != nullptr ? from->gestureBudget : 0.0,
                                       to != nullptr ? to->gestureBudget : 0.0, amount);
            out.actors[out.actorCount++] = actor;
        }
    }

    static void appendRelations(const PerformanceConstellation& source,
                                const PerformanceConstellation& target,
                                double amount, PerformanceSceneMorph& out) noexcept {
        std::array<bool, kConstellationRelationCapacity> targetConsumed{};
        for (std::size_t sourceIndex = 0; sourceIndex < source.relationCount; ++sourceIndex) {
            const auto& from = source.relations[sourceIndex];
            const PerformanceRelation* to = nullptr;
            std::size_t targetIndex = 0;
            for (; targetIndex < target.relationCount; ++targetIndex) {
                if (from.sameIdentity(target.relations[targetIndex])) {
                    to = &target.relations[targetIndex];
                    targetConsumed[targetIndex] = true;
                    break;
                }
            }
            appendRelationFrame(&from, to, amount, out);
        }
        for (std::size_t targetIndex = 0; targetIndex < target.relationCount; ++targetIndex)
            if (!targetConsumed[targetIndex]) appendRelationFrame(nullptr, &target.relations[targetIndex], amount, out);
    }

    static void appendRelationFrame(const PerformanceRelation* from,
                                    const PerformanceRelation* to,
                                    double amount, PerformanceSceneMorph& out) noexcept {
        MorphedRelation frame;
        frame.relation = from != nullptr ? *from : *to;
        if (from != nullptr && to != nullptr) {
            frame.relation.intensity = lerp(from->intensity, to->intensity, amount);
            frame.relation.latencyMilliseconds = lerp(from->latencyMilliseconds, to->latencyMilliseconds, amount);
            frame.relation.durationSeconds = lerp(from->durationSeconds, to->durationSeconds, amount);
            frame.relation.limit = lerp(from->limit, to->limit, amount);
            frame.activity = 1.0;
        } else if (from != nullptr) {
            frame.activity = 1.0 - amount;
        } else {
            frame.activity = amount;
        }
        out.relations[out.relationCount++] = frame;
    }
};

} // namespace rasgo::performance
