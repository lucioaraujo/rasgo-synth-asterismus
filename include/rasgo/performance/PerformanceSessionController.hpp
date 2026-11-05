#pragma once

#include "PerformanceSessionDocument.hpp"
#include "PerformanceSessionEngine.hpp"
#include "PerformanceSky.hpp"
#include "PerformanceCumulativeDesk.hpp"
#include "PerformanceModuleLibrary.hpp"

#include <algorithm>
#include <atomic>
#include <limits>

namespace rasgo::performance {

// Control-thread facade for a future standalone host. Loading/preparing remains
// quiescent-only; scene recall crosses to the audio side through a fixed SPSC
// queue, so a GUI/MIDI producer does not mutate engine state during a block.
class PerformanceSessionController {
private:
    struct StemMixCommand {
        enum class Kind : std::uint8_t { Gain, Mute, Solo };
        Kind kind = Kind::Gain;
        std::uint16_t actorId = kInvalidActorId;
        double gain = 1.0;
        bool enabled = false;
    };

public:
    static constexpr std::size_t kSceneRecallQueueCapacity = 32U;
    static constexpr std::size_t kSceneCaptureQueueCapacity = 8U;
    static constexpr std::size_t kMacroQueueCapacity = 64U;
    static constexpr std::size_t kTempoQueueCapacity = 16U;
    static constexpr std::size_t kQuantizeQueueCapacity = 8U;
    static constexpr std::size_t kEcologyQueueCapacity = 64U;
    static constexpr std::size_t kTransportQueueCapacity = 8U;
    static constexpr std::size_t kSafetyQueueCapacity = 8U;
    static constexpr std::size_t kMasterQueueCapacity = 32U;
    static constexpr std::size_t kStemMixQueueCapacity = 64U;
    static constexpr std::size_t kModuleControlQueueCapacity = 96U;
    static constexpr std::size_t kTerritoryQueueCapacity = 2U;

    bool prepare(double sampleRate, std::uint32_t maximumBlockSize,
                 std::uint32_t channels) noexcept {
        loaded_ = false;
        activeSceneIndex_ = kNoSceneIndex;
        recallCommands_.reset();
        captureCommands_.reset();
        macroCommands_.reset();
        tempoCommands_.reset();
        quantizeCommands_.reset();
        ecologyCommands_.reset();
        transportCommands_.reset();
        safetyCommands_.reset();
        masterCommands_.reset();
        stemMixCommands_.reset();
        moduleControlCommands_.reset();
        territoryCommands_.reset();
        morceauState_.store(static_cast<std::uint8_t>(PerformanceMorceauState::Prepared),
                            std::memory_order_relaxed);
        cumulativeDesk_.clear();
        currentSceneId_.store(kNoScene, std::memory_order_relaxed);
        return engine_.prepare(sampleRate, maximumBlockSize, channels);
    }

    bool loadDocument(const PerformanceSessionDocument& document,
                      std::uint16_t initialSceneId = kNoScene) noexcept {
        if (!document.valid() || document.sceneCount == 0U || !engine_.prepared()) return false;
        const auto selected = findScene(document, initialSceneId);
        if (selected == kNoSceneIndex
            || !engine_.setTerritoryScene(document.scenes[selected], document.territory))
            return false;
        controlDocument_ = document;
        audioDocument_ = document;
        activeSceneIndex_ = selected;
        currentSceneId_.store(document.scenes[selected].id, std::memory_order_release);
        loaded_ = true;
        cumulativeDesk_.clear();
        return true;
    }

    bool recallScene(std::uint16_t sceneId, double morphSeconds) noexcept {
        if (!loaded_) return false;
        const auto target = findScene(controlDocument_, sceneId);
        return target != kNoSceneIndex && finite(morphSeconds)
            && morphSeconds >= 0.02 && morphSeconds <= 30.0
            && recallCommands_.push({sceneId, morphSeconds});
    }

    // Capture is a control-side snapshot. The bounded command updates only the
    // controller's future recall document on the audio side; it never mutates
    // the currently sounding engine state directly.
    bool captureScene(const PerformanceScene& scene) noexcept {
        if (!loaded_ || !scene.valid(controlDocument_.territory)) return false;
        const auto target = findScene(controlDocument_, scene.id);
        if (target == kNoSceneIndex) return false;
        auto bounded = scene;
        boundSceneToTerritory(bounded, controlDocument_.territory);
        if (!captureCommands_.push({bounded})) return false;
        controlDocument_.scenes[target] = bounded;
        return true;
    }

    bool pushGesture(const PerformanceGestureEvent& event) noexcept {
        return loaded_ && engine_.pushGesture(event);
    }

    bool setMacro(PerformanceMacro macro, double value) noexcept {
        return loaded_ && unit(value)
            && static_cast<std::uint8_t>(macro) < static_cast<std::uint8_t>(PerformanceMacro::Count)
            && macroCommands_.push({macro, value});
    }

    bool setTempoBpm(double tempoBpm) noexcept {
        return loaded_ && finite(tempoBpm) && tempoBpm >= 20.0 && tempoBpm <= 300.0
            && tempoCommands_.push({tempoBpm});
    }

    bool setQuantizedGestures(bool enabled) noexcept {
        return setGestureGrid(enabled, GestureGridDivision::Beat);
    }

    bool setGestureGrid(bool enabled, GestureGridDivision division) noexcept {
        return loaded_ && validGestureGridDivision(division)
            && quantizeCommands_.push({enabled, division});
    }

    bool setActorPresence(std::uint16_t actorId, double presence) noexcept {
        const bool queued = loaded_ && engine_.hasActor(actorId)
            && controlDocument_.territory.find(actorId) != nullptr && unit(presence)
            && ecologyCommands_.push({EcologyCommand::Kind::Presence, actorId, kInvalidActorId, presence});
        if (queued && cumulativeDesk_.contains(actorId))
            cumulativeDesk_.setPresence(actorId, presence, presence > 0.0);
        return queued;
    }

    // Control-side admission for a module already prepared by the current
    // seed. The desk records the musician's explicit insertion before the
    // bounded presence/position commands cross to the audio boundary.
    bool addDeskModule(std::uint16_t actorId, std::uint16_t libraryEntry,
                       double x, double y, double presence) noexcept {
        if (!loaded_ || !engine_.hasActor(actorId)
            || controlDocument_.territory.find(actorId) == nullptr
            || !unit(x) || !unit(y) || !unit(presence)) return false;
        if (!cumulativeDesk_.add(actorId, libraryEntry, {x, y}, presence, presence > 0.0))
            return false;
        const bool queued = ecologyCommands_.push({EcologyCommand::Kind::Position,
                                                   actorId, kInvalidActorId, x, y})
            && ecologyCommands_.push({EcologyCommand::Kind::Presence,
                                       actorId, kInvalidActorId, presence});
        if (!queued) cumulativeDesk_.remove(actorId);
        return queued;
    }

    bool addLibraryModule(std::uint16_t actorId, std::string_view moduleId,
                          double x, double y, double presence) noexcept {
        const auto* entry = findPerformanceModule(moduleId);
        if (entry == nullptr || entry->state != PerformanceLibraryState::ActiveCohort)
            return false;
        // A library entry is not an invitation to reinterpret an existing
        // actor: a body lights only the star the seed's asterism gives it.
        if (actorId < 1U || actorId > 8U
            || performanceStarLibraryId(performanceInitialAsterism(
                   controlDocument_.territory.seed)[actorId - 1U]) != moduleId)
            return false;
        return addDeskModule(actorId,
                             static_cast<std::uint16_t>(performanceModuleLibraryIndex(moduleId)),
                             x, y, presence);
    }

    bool removeDeskModule(std::uint16_t actorId) noexcept {
        if (!loaded_ || !cumulativeDesk_.contains(actorId)) return false;
        if (!setActorPresence(actorId, 0.0)) return false;
        return cumulativeDesk_.remove(actorId);
    }

    const PerformanceCumulativeDesk& cumulativeDesk() const noexcept {
        return cumulativeDesk_;
    }

    bool setModuleControl(std::uint16_t actorId, PerformanceModuleControl control,
                          double value) noexcept {
        const bool declaredBody = controlDocument_.territory.find(actorId) != nullptr;
        const bool internalLane = actorId >= 5U && actorId <= 8U && !declaredBody;
        return loaded_ && (internalLane ? engine_.supportsModuleControl(actorId)
                                        : engine_.hasActor(actorId)) && unit(value)
            && static_cast<std::uint8_t>(control)
                < static_cast<std::uint8_t>(PerformanceModuleControl::Count)
            && moduleControlCommands_.push({actorId, control, value});
    }

    bool setRelationIntensity(std::uint16_t sourceActorId, std::uint16_t destinationActorId,
                              double intensity) noexcept {
        return loaded_ && engine_.hasActor(sourceActorId) && engine_.hasActor(destinationActorId)
            && hasRelationEndpoints(sourceActorId, destinationActorId) && unit(intensity)
            && ecologyCommands_.push({EcologyCommand::Kind::RelationIntensity,
                                      sourceActorId, destinationActorId, intensity});
    }

    bool setRelationPhenomenon(std::uint16_t sourceActorId,
                               std::uint16_t destinationActorId,
                               PerformanceRelationalPhenomenon phenomenon,
                               double resonanceRatio = 1.0) noexcept {
        if (!loaded_ || !engine_.hasActor(sourceActorId) || !engine_.hasActor(destinationActorId)
            || !hasRelationEndpoints(sourceActorId, destinationActorId)
            || !validRelationalPhenomenon(phenomenon) || !finite(resonanceRatio)
            || resonanceRatio < 0.25 || resonanceRatio > 4.0) return false;
        if (!ecologyCommands_.push({EcologyCommand::Kind::RelationPhenomenon,
                                    sourceActorId, destinationActorId,
                                    static_cast<double>(static_cast<std::uint8_t>(phenomenon)),
                                    resonanceRatio})) return false;
        for (std::size_t sceneIndex = 0U; sceneIndex < controlDocument_.sceneCount; ++sceneIndex) {
            if (controlDocument_.scenes[sceneIndex].id
                != currentSceneId_.load(std::memory_order_acquire)) continue;
            auto& constellation = controlDocument_.scenes[sceneIndex].constellation;
            for (std::size_t relationIndex = 0U; relationIndex < constellation.relationCount; ++relationIndex) {
                auto& relation = constellation.relations[relationIndex];
                if (relation.sourceActorId == sourceActorId && relation.destinationActorId == destinationActorId) {
                    relation.phenomenon = phenomenon;
                    relation.resonanceRatio = resonanceRatio;
                    relation.phenomenonEnabled = true;
                }
            }
        }
        return true;
    }

    bool setMotionField(PerformanceMotionField field) noexcept {
        return loaded_ && validPerformanceMotionField(field)
            && ecologyCommands_.push({EcologyCommand::Kind::MotionField,
                                      kInvalidActorId, kInvalidActorId,
                                      static_cast<double>(static_cast<std::uint8_t>(field))});
    }

    bool setPopulationPhenomenon(PerformancePopulationPhenomenon phenomenon,
                                 double intensity) noexcept {
        if (!loaded_ || !validPopulationPhenomenon(phenomenon) || !unit(intensity)
            || !ecologyCommands_.push({EcologyCommand::Kind::PopulationPhenomenon,
                                       kInvalidActorId, kInvalidActorId,
                                       static_cast<double>(static_cast<std::uint8_t>(phenomenon)),
                                       intensity})) return false;
        for (std::size_t index = 0U; index < controlDocument_.sceneCount; ++index) {
            auto& scene = controlDocument_.scenes[index];
            if (scene.id != currentSceneId_.load(std::memory_order_acquire)) continue;
            scene.session.populationPhenomenon = phenomenon;
            scene.session.populationPhenomenonIntensity = intensity;
            scene.session.populationPhenomenonEnabled = intensity > 0.0;
            break;
        }
        return true;
    }

    bool setActorMotionField(std::uint16_t actorId, PerformanceMotionField field) noexcept {
        return loaded_ && engine_.hasActor(actorId) && validPerformanceMotionField(field)
            && ecologyCommands_.push({EcologyCommand::Kind::ActorMotionField,
                                      actorId, kInvalidActorId,
                                      static_cast<double>(static_cast<std::uint8_t>(field))});
    }

    bool setActorMotionDirection(std::uint16_t actorId, bool forward) noexcept {
        return loaded_ && engine_.hasActor(actorId)
            && ecologyCommands_.push({EcologyCommand::Kind::ActorMotionDirection,
                                      actorId, kInvalidActorId, forward ? 1.0 : 0.0});
    }

    bool setSeedCollage(std::uint32_t sourceSeed, double amount) noexcept {
        return loaded_ && unit(amount)
            && ecologyCommands_.push({EcologyCommand::Kind::SeedCollage,
                                      kInvalidActorId, kInvalidActorId, amount, 0.5, sourceSeed});
    }

    bool setActorSeedVariation(std::uint16_t actorId, double amount) noexcept {
        return loaded_ && engine_.supportsActorSeedVariation(actorId) && unit(amount)
            && ecologyCommands_.push({EcologyCommand::Kind::ActorSeedVariation,
                                      actorId, kInvalidActorId, amount});
    }

    bool setActorPosition(std::uint16_t actorId, double x, double y) noexcept {
        const bool internalLane = actorId >= 5U && actorId <= 8U
            && controlDocument_.territory.find(actorId) == nullptr;
        const bool queued = loaded_ && (internalLane ? engine_.supportsActorPosition(actorId)
                                                     : engine_.hasActor(actorId))
            && (internalLane || controlDocument_.territory.find(actorId) != nullptr) && unit(x) && unit(y)
            && ecologyCommands_.push({EcologyCommand::Kind::Position, actorId, kInvalidActorId, x, y});
        if (queued && cumulativeDesk_.contains(actorId))
            cumulativeDesk_.setPosition(actorId, {x, y});
        return queued;
    }

    // A forma encaixada numa duração (compassos de abertura, desenvolvimento,
    // contraste e retorno; todos zero desfazem). Cada número cabe em 16 bits.
    bool setFormPlan(const std::array<std::uint32_t, 4>& bars) noexcept {
        for (const auto value : bars) if (value > 0xFFFFU) return false;
        return loaded_ && ecologyCommands_.push({EcologyCommand::Kind::FormPlan, static_cast<std::uint16_t>(bars[0]),
                                                 static_cast<std::uint16_t>(bars[1]), static_cast<double>(bars[2]),
                                                 static_cast<double>(bars[3])});
    }

    // Fixar a estrela de um corpo: a VARIAÇÃO não a apaga.
    bool setActorPinned(std::uint16_t actorId, bool pinned) noexcept {
        return loaded_ && actorId != kInvalidActorId
            && ecologyCommands_.push({EcologyCommand::Kind::ActorPin, actorId, kInvalidActorId,
                                      pinned ? 1.0 : 0.0});
    }

    // Trocar a estrela de um corpo (Count desfaz a troca).
    bool setActorStar(std::uint16_t actorId, PerformanceModuleBody star) noexcept {
        return loaded_ && actorId != kInvalidActorId
            && (star == PerformanceModuleBody::Count || PerformanceSky::isStar(star))
            && ecologyCommands_.push({EcologyCommand::Kind::ActorStar, actorId, kInvalidActorId,
                                      static_cast<double>(static_cast<std::uint8_t>(star))});
    }

    // O giro e a viagem da constelação inteira.
    bool setConstellationMotion(PerformanceConstellationMotion motion) noexcept {
        return loaded_ && static_cast<std::uint8_t>(motion) < static_cast<std::uint8_t>(PerformanceConstellationMotion::Count)
            && ecologyCommands_.push({EcologyCommand::Kind::ConstellationMotion, kInvalidActorId, kInvalidActorId,
                                      static_cast<double>(static_cast<std::uint8_t>(motion))});
    }

    // A família de movimento da DANÇA (kInvalidActorId: o campo inteiro).
    bool setMotionFamily(std::uint16_t actorId, PerformanceMotionFamily family) noexcept {
        return loaded_ && static_cast<std::uint8_t>(family) <= static_cast<std::uint8_t>(PerformanceMotionFamily::Count)
            && ecologyCommands_.push({EcologyCommand::Kind::MotionFamily, actorId, kInvalidActorId,
                                      static_cast<double>(static_cast<std::uint8_t>(family))});
    }

    // O lugar desenhado, só para a física do céu (não muda a ecologia).
    bool setSkyAnchor(std::uint16_t actorId, double x, double y) noexcept {
        return loaded_ && actorId != kInvalidActorId && unit(x) && unit(y)
            && ecologyCommands_.push({EcologyCommand::Kind::SkyAnchor, actorId, kInvalidActorId, x, y});
    }

    bool setPlaying(bool playing) noexcept {
        return loaded_ && transportCommands_.push({TransportCommand::Kind::Playing, playing});
    }

    bool beginMorceauCoda() noexcept {
        return loaded_ && transportCommands_.push({TransportCommand::Kind::Coda, false});
    }

    // Depois da coda de uma tomada, uma nova peça abre sem STOP (5/11).
    bool continueAfterMorceau() noexcept {
        return loaded_ && transportCommands_.push({TransportCommand::Kind::Continue, true});
    }

    PerformanceMorceauState morceauState() const noexcept {
        return static_cast<PerformanceMorceauState>(morceauState_.load(std::memory_order_acquire));
    }

    bool setPanicSafe(bool enabled) noexcept {
        return loaded_ && safetyCommands_.push({enabled});
    }

    bool setOutputTrimDb(double decibels) noexcept {
        return loaded_ && finite(decibels) && decibels >= -24.0 && decibels <= 12.0
            && masterCommands_.push({decibels});
    }

    bool setStemGain(std::uint16_t actorId, double gain) noexcept {
        return loaded_ && controlDocument_.territory.find(actorId) != nullptr && unit(gain)
            && stemMixCommands_.push({StemMixCommand::Kind::Gain, actorId, gain, false});
    }

    bool setStemMuted(std::uint16_t actorId, bool muted) noexcept {
        return loaded_ && controlDocument_.territory.find(actorId) != nullptr
            && stemMixCommands_.push({StemMixCommand::Kind::Mute, actorId, 0.0, muted});
    }

    bool setStemSolo(std::uint16_t actorId, bool soloed) noexcept {
        return loaded_ && controlDocument_.territory.find(actorId) != nullptr
            && stemMixCommands_.push({StemMixCommand::Kind::Solo, actorId, 0.0, soloed});
    }

    // Called only by the audio-device host after it measures one callback
    // exceeding its sample budget. It never crosses a control queue.
    void reportDeadlineMiss() noexcept { engine_.reportDeadlineMiss(); }

    // A territory is a bounded live transition. The control side may prepare it
    // freely and enqueue it while audio is running; the audio boundary applies
    // the complete value object without pointers, allocations or I/O.
    bool queueTerritory(const PerformanceSessionDocument& document,
                        std::uint16_t initialSceneId = kNoScene) noexcept {
        if (!loaded_ || !document.valid() || document.sceneCount == 0U
            || findScene(document, initialSceneId) == kNoSceneIndex)
            return false;
        if (!territoryCommands_.push({document, initialSceneId})) return false;
        controlDocument_ = document;
        return true;
    }

    // Variação do mixer, ligada ou desligada pelo músico (qualquer thread);
    // o áudio a aplica no começo do bloco seguinte.
    // O POLIFONIA (qualquer thread; 6/11), −1 a +1.
    void setPolyphonyBias(double bias) noexcept { polyphonyBias_.store(bias, std::memory_order_release); }
    // A duração da próxima coda, em segundos (qualquer thread; 5/11).
    void setCodaSeconds(double seconds) noexcept { codaSeconds_.store(seconds, std::memory_order_release); }
    void setMixerVariation(bool enabled) noexcept {
        mixerVariation_.store(enabled, std::memory_order_release);
    }
    bool mixerVariation() const noexcept { return mixerVariation_.load(std::memory_order_acquire); }
    // A trama do céu (relações entre estrelas), ligada ou desligada.
    void setSkyWeb(bool enabled) noexcept { skyWeb_.store(enabled, std::memory_order_release); }
    bool skyWeb() const noexcept { return skyWeb_.load(std::memory_order_acquire); }
    // Guarda de capacidade medida pelo app (qualquer thread).
    void setStarCapacity(std::size_t capacity) noexcept {
        starCapacity_.store(static_cast<std::uint8_t>(std::clamp<std::size_t>(capacity, 1U, 8U)),
                            std::memory_order_release);
    }
    std::size_t starCapacity() const noexcept { return starCapacity_.load(std::memory_order_acquire); }

    // Chamar fora do áudio (antes de o dispositivo começar).
    void setStemTap(float* const* buffers, std::size_t count, std::uint32_t capacity) noexcept {
        engine_.setStemTap(buffers, count, capacity);
    }

    void processBlock(float* left, float* right, std::uint32_t samples) noexcept {
        engine_.setMixerVariation(mixerVariation_.load(std::memory_order_acquire));
        engine_.setPolyphonyBias(polyphonyBias_.load(std::memory_order_acquire));
        {
            const auto coda = codaSeconds_.load(std::memory_order_acquire);
            if (coda != appliedCodaSeconds_ && engine_.setCodaSeconds(coda)) appliedCodaSeconds_ = coda;
        }
        engine_.setStarCapacity(starCapacity_.load(std::memory_order_acquire));
        engine_.setSkyInfluence(skyWeb_.load(std::memory_order_acquire) ? 1.0 : 0.0);
        // Territory replacement is the boundary transaction. Commands queued
        // immediately afterwards, including PLAY, target the installed seed.
        applyTerritoryCommands();
        applyMacroCommands();
        applyTempoCommands();
        applyQuantizeCommands();
        applyEcologyCommands();
        applyModuleControlCommands();
        applyTransportCommands();
        applySafetyCommands();
        applyMasterCommands();
        applyStemMixCommands();
        applyCaptureCommands();
        applyRecallCommands();
        engine_.processBlock(left, right, samples);
        morceauState_.store(static_cast<std::uint8_t>(engine_.morceauState()),
                            std::memory_order_release);
    }

    const PerformanceSessionDocument* document() const noexcept {
        return loaded_ ? &controlDocument_ : nullptr;
    }

    std::uint16_t currentSceneId() const noexcept {
        return currentSceneId_.load(std::memory_order_acquire);
    }

    PerformanceTelemetrySnapshot readTelemetry() const noexcept { return engine_.readTelemetry(); }

    // Read-only views used by the desk and by continuity/regression checks.
    // They expose the live field without giving the controller a second path
    // that could silently redraw it.
    PerformanceSpatialPoint actorPosition(std::uint16_t actorId) const noexcept {
        return engine_.actorPosition(actorId);
    }

    PerformanceSpatialGameEvent lastSpatialGameEvent() const noexcept {
        return engine_.lastSpatialGameEvent();
    }

private:
    struct SceneRecallCommand {
        std::uint16_t sceneId = kNoScene;
        double morphSeconds = 1.0;
    };

    struct SceneCaptureCommand { PerformanceScene scene{}; };

    struct MacroCommand {
        PerformanceMacro macro = PerformanceMacro::Autonomy;
        double value = 0.5;
    };
    struct TempoCommand { double tempoBpm = 120.0; };
    struct QuantizeCommand {
        bool enabled = false;
        GestureGridDivision division = GestureGridDivision::Beat;
    };

    struct EcologyCommand {
        enum class Kind : std::uint8_t { Presence, RelationIntensity, RelationPhenomenon, Position, MotionField, PopulationPhenomenon, ActorMotionField, ActorMotionDirection, SeedCollage, ActorSeedVariation, SkyAnchor, MotionFamily, ConstellationMotion, ActorStar, ActorPin, FormPlan };
        Kind kind = Kind::Presence;
        std::uint16_t sourceActorId = kInvalidActorId;
        std::uint16_t destinationActorId = kInvalidActorId;
        double value = 0.5;
        double secondaryValue = 0.5;
        std::uint32_t seed = 0U;
    };

    struct ModuleControlCommand {
        std::uint16_t actorId = kInvalidActorId;
        PerformanceModuleControl control = PerformanceModuleControl::Primary;
        double value = 0.5;
    };

    struct TransportCommand {
        enum class Kind : std::uint8_t { Playing, Coda, Continue };
        Kind kind = Kind::Playing;
        bool playing = false;
    };
    struct SafetyCommand { bool enabled = false; };
    struct MasterCommand { double decibels = 0.0; };
    struct TerritoryCommand {
        PerformanceSessionDocument document{};
        std::uint16_t initialSceneId = kNoScene;
    };

    static constexpr std::uint16_t kNoScene = std::numeric_limits<std::uint16_t>::max();
    static constexpr std::size_t kNoSceneIndex = std::numeric_limits<std::size_t>::max();

    static std::size_t findScene(const PerformanceSessionDocument& document,
                                 std::uint16_t sceneId) noexcept {
        if (sceneId == kNoScene) return 0U;
        for (std::size_t index = 0; index < document.sceneCount; ++index)
            if (document.scenes[index].id == sceneId) return index;
        return kNoSceneIndex;
    }

    static void boundSceneToTerritory(PerformanceScene& scene,
                                      const PerformanceTerritory& territory) noexcept {
        scene.session.memory = std::min(scene.session.memory, territory.identity.memoryLimit);
        scene.session.risk = std::min(scene.session.risk, territory.identity.riskLimit);
    }

    bool hasRelationEndpoints(std::uint16_t sourceActorId, std::uint16_t destinationActorId) const noexcept {
        for (std::size_t sceneIndex = 0; sceneIndex < controlDocument_.sceneCount; ++sceneIndex) {
            const auto& constellation = controlDocument_.scenes[sceneIndex].constellation;
            for (std::size_t relationIndex = 0; relationIndex < constellation.relationCount; ++relationIndex) {
                const auto& relation = constellation.relations[relationIndex];
                if (relation.sourceActorId == sourceActorId
                    && relation.destinationActorId == destinationActorId) return true;
            }
        }
        return false;
    }

    void applyRecallCommands() noexcept {
        SceneRecallCommand command;
        while (recallCommands_.pop(command)) {
            const auto target = findScene(audioDocument_, command.sceneId);
            if (target == kNoSceneIndex || activeSceneIndex_ == kNoSceneIndex) continue;
            if (target == activeSceneIndex_) {
                if (engine_.recallScene(audioDocument_.scenes[target], audioDocument_.territory))
                    currentSceneId_.store(command.sceneId, std::memory_order_release);
                continue;
            }
            if (engine_.beginSceneMorph(audioDocument_.scenes[activeSceneIndex_],
                                        audioDocument_.scenes[target],
                                        audioDocument_.territory, command.morphSeconds)) {
                activeSceneIndex_ = target;
                currentSceneId_.store(command.sceneId, std::memory_order_release);
            }
        }
    }

    void applyCaptureCommands() noexcept {
        SceneCaptureCommand command;
        while (captureCommands_.pop(command)) {
            const auto target = findScene(audioDocument_, command.scene.id);
            if (target != kNoSceneIndex && command.scene.valid(audioDocument_.territory))
                audioDocument_.scenes[target] = command.scene;
        }
    }

    void applyMacroCommands() noexcept {
        MacroCommand command;
        while (macroCommands_.pop(command)) engine_.setMacro(command.macro, command.value);
    }

    void applyTempoCommands() noexcept {
        TempoCommand command;
        while (tempoCommands_.pop(command)) engine_.setTempoBpm(command.tempoBpm);
    }

    void applyQuantizeCommands() noexcept {
        QuantizeCommand command;
        while (quantizeCommands_.pop(command)) engine_.setGestureGrid(command.enabled, command.division);
    }

    void applyEcologyCommands() noexcept {
        EcologyCommand command;
        while (ecologyCommands_.pop(command)) {
            if (command.kind == EcologyCommand::Kind::Presence)
                engine_.setActorPresence(command.sourceActorId, command.value);
            else if (command.kind == EcologyCommand::Kind::RelationIntensity)
                engine_.setRelationIntensity(command.sourceActorId, command.destinationActorId, command.value);
            else if (command.kind == EcologyCommand::Kind::RelationPhenomenon)
                engine_.setRelationPhenomenon(command.sourceActorId, command.destinationActorId,
                    static_cast<PerformanceRelationalPhenomenon>(
                        static_cast<std::uint8_t>(command.value)), command.secondaryValue);
            else if (command.kind == EcologyCommand::Kind::Position)
                engine_.setActorPosition(command.sourceActorId, command.value, command.secondaryValue);
            else if (command.kind == EcologyCommand::Kind::FormPlan)
                engine_.setFormPlan({{command.sourceActorId, command.destinationActorId,
                                      static_cast<std::uint32_t>(command.value),
                                      static_cast<std::uint32_t>(command.secondaryValue)}});
            else if (command.kind == EcologyCommand::Kind::ActorPin)
                engine_.setActorPinned(command.sourceActorId, command.value > 0.5);
            else if (command.kind == EcologyCommand::Kind::ActorStar)
                engine_.setActorStar(command.sourceActorId,
                    static_cast<PerformanceModuleBody>(static_cast<std::uint8_t>(command.value)));
            else if (command.kind == EcologyCommand::Kind::ConstellationMotion)
                engine_.setConstellationMotion(static_cast<PerformanceConstellationMotion>(
                    static_cast<std::uint8_t>(command.value)));
            else if (command.kind == EcologyCommand::Kind::MotionFamily)
                engine_.setMotionFamily(command.sourceActorId,
                    static_cast<PerformanceMotionFamily>(static_cast<std::uint8_t>(command.value)));
            else if (command.kind == EcologyCommand::Kind::SkyAnchor)
                engine_.setSkyAnchor(command.sourceActorId, command.value, command.secondaryValue);
            else if (command.kind == EcologyCommand::Kind::MotionField)
                engine_.setMotionField(static_cast<PerformanceMotionField>(
                    static_cast<std::uint8_t>(command.value)));
            else if (command.kind == EcologyCommand::Kind::PopulationPhenomenon)
                engine_.setPopulationPhenomenon(static_cast<PerformancePopulationPhenomenon>(
                    static_cast<std::uint8_t>(command.value)), command.secondaryValue);
            else if (command.kind == EcologyCommand::Kind::ActorMotionField)
                engine_.setActorMotionField(command.sourceActorId,
                    static_cast<PerformanceMotionField>(static_cast<std::uint8_t>(command.value)));
            else if (command.kind == EcologyCommand::Kind::ActorMotionDirection)
                engine_.setActorMotionDirection(command.sourceActorId, command.value >= 0.5);
            else if (command.kind == EcologyCommand::Kind::SeedCollage)
                engine_.setSeedCollage(command.seed, command.value);
            else if (command.kind == EcologyCommand::Kind::ActorSeedVariation)
                engine_.setActorSeedVariation(command.sourceActorId, command.value);
        }
    }

    void applyModuleControlCommands() noexcept {
        ModuleControlCommand command;
        while (moduleControlCommands_.pop(command))
            engine_.setModuleControl(command.actorId, command.control, command.value);
    }

    void applyTransportCommands() noexcept {
        TransportCommand command;
        while (transportCommands_.pop(command)) {
            if (command.kind == TransportCommand::Kind::Playing) engine_.setPlaying(command.playing);
            else if (command.kind == TransportCommand::Kind::Continue) engine_.continueAfterMorceau();
            else engine_.beginMorceauCoda();
        }
    }

    void applySafetyCommands() noexcept {
        SafetyCommand command;
        while (safetyCommands_.pop(command)) engine_.setPanicSafe(command.enabled);
    }

    void applyMasterCommands() noexcept {
        MasterCommand command;
        while (masterCommands_.pop(command)) engine_.setOutputTrimDb(command.decibels);
    }

    void applyStemMixCommands() noexcept {
        StemMixCommand command;
        while (stemMixCommands_.pop(command)) {
            switch (command.kind) {
            case StemMixCommand::Kind::Gain: engine_.setStemGain(command.actorId, command.gain); break;
            case StemMixCommand::Kind::Mute: engine_.setStemMuted(command.actorId, command.enabled); break;
            case StemMixCommand::Kind::Solo: engine_.setStemSolo(command.actorId, command.enabled); break;
            }
        }
    }

    void applyTerritoryCommands() noexcept {
        TerritoryCommand command;
        while (territoryCommands_.pop(command)) {
            if (!command.document.valid()) continue;
            const auto selected = findScene(command.document, command.initialSceneId);
            if (selected == kNoSceneIndex
                || !engine_.setTerritoryScene(command.document.scenes[selected],
                                              command.document.territory))
                continue;
            audioDocument_ = command.document;
            activeSceneIndex_ = selected;
            currentSceneId_.store(command.document.scenes[selected].id, std::memory_order_release);
        }
    }

    PerformanceSessionEngine engine_{};
    PerformanceCumulativeDesk cumulativeDesk_{};
    // The producer and consumer never mutate the same document. Commands carry
    // bounded copies across the SPSC boundary; GUI reads only controlDocument_
    // and audio reads/writes only audioDocument_.
    PerformanceSessionDocument controlDocument_{};
    PerformanceSessionDocument audioDocument_{};
    FixedEventQueue<SceneRecallCommand, kSceneRecallQueueCapacity> recallCommands_{};
    FixedEventQueue<SceneCaptureCommand, kSceneCaptureQueueCapacity> captureCommands_{};
    FixedEventQueue<MacroCommand, kMacroQueueCapacity> macroCommands_{};
    FixedEventQueue<TempoCommand, kTempoQueueCapacity> tempoCommands_{};
    FixedEventQueue<QuantizeCommand, kQuantizeQueueCapacity> quantizeCommands_{};
    FixedEventQueue<EcologyCommand, kEcologyQueueCapacity> ecologyCommands_{};
    FixedEventQueue<TransportCommand, kTransportQueueCapacity> transportCommands_{};
    std::atomic<std::uint8_t> morceauState_{
        static_cast<std::uint8_t>(PerformanceMorceauState::Prepared)};
    FixedEventQueue<SafetyCommand, kSafetyQueueCapacity> safetyCommands_{};
    FixedEventQueue<MasterCommand, kMasterQueueCapacity> masterCommands_{};
    FixedEventQueue<StemMixCommand, kStemMixQueueCapacity> stemMixCommands_{};
    FixedEventQueue<ModuleControlCommand, kModuleControlQueueCapacity> moduleControlCommands_{};
    FixedEventQueue<TerritoryCommand, kTerritoryQueueCapacity> territoryCommands_{};
    std::size_t activeSceneIndex_ = kNoSceneIndex; // audio thread only after load.
    std::atomic<std::uint16_t> currentSceneId_{kNoScene};
    bool loaded_ = false;
    std::atomic<bool> mixerVariation_{false};
    std::atomic<double> codaSeconds_{4.0};
    std::atomic<double> polyphonyBias_{0.0};
    double appliedCodaSeconds_ = 4.0;
    std::atomic<std::uint8_t> starCapacity_{8U};
    std::atomic<bool> skyWeb_{true};
};

} // namespace rasgo::performance
