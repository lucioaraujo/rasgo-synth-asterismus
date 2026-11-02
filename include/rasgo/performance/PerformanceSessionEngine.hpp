#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "PerformanceActorEcology.hpp"
#include "PerformanceContracts.hpp"
#include "PerformanceLoudnessMeter.hpp"
#include "PerformanceMasterStage.hpp"
#include "PerformanceMorceauLifecycle.hpp"

namespace rasgo::performance {

// Marco 2: a deliberately small native voice. It proves the realtime contract
// and gesture lifecycle; it is not an adapter for, or a replacement of, Studio DSP.
class PerformanceSessionEngine {
public:
    static constexpr std::size_t kEventQueueCapacity = 128;

    bool prepare(double sampleRate, std::uint32_t maximumBlockSize,
                 std::uint32_t channels) noexcept {
        if (!finite(sampleRate) || sampleRate < 8000.0 || sampleRate > 384000.0
            || maximumBlockSize == 0U || maximumBlockSize > 8192U
            || channels == 0U || channels > 2U)
            return false;
        sampleRate_ = sampleRate;
        maximumBlockSize_ = maximumBlockSize;
        channels_ = channels;
        prepared_ = false;
        if (!ecology_.prepare(sampleRate, maximumBlockSize, channels)) return false;
        master_.prepare(sampleRate);
        meter_.prepare(sampleRate);
        prepared_ = true;
        reset();
        if (!morceau_.prepare(sampleRate_, session_.territorySeed)) {
            prepared_ = false;
            return false;
        }
        return true;
    }

    void reset() noexcept {
        phase_ = 0.0;
        sampleCursor_ = 0U;
        spatialGameSampleAccumulator_ = 0U;
        performerPauseSamples_ = 0U;
        codaReadiness_ = 0.0;
        observedManualActivity_ = 0U;
        manualActivitySerial_.store(0U, std::memory_order_relaxed);
        gain_ = 0.0;
        targetGain_ = 0.0;
        frequency_ = 110.0;
        targetFrequency_ = 110.0;
        mutate_ = 0.0;
        wild_ = 0.0;
        wildToken_ = 0U;
        wildTargetActorId_ = kInvalidActorId;
        tearTargetActorId_ = kInvalidActorId;
        repairTargetActorId_ = kInvalidActorId;
        tear_ = 0.0;
        scar_ = 0.0;
        repair_ = 0.0;
        silence_ = 0.0;
        silenceTarget_ = 0.0;
        silenceHoldSamples_ = 0U;
        stopDeClickSamples_ = 0U;
        stopDeClickRemaining_ = 0U;
        holding_ = false;
        quantizedGestures_ = false;
        gestureGridDivision_ = GestureGridDivision::Beat;
        ecology_.setGestureGrid(false, GestureGridDivision::Beat);
        ecology_.setHolding(false);
        panicSafe_ = false;
        masterTrimOverride_ = false;
        masterTrimDb_ = 0.0;
        deferred_ = false;
        morphActive_ = false;
        ecology_.reset();
        xrunCount_ = 0U;
        lastStemCount_ = 0U;
        lastStemActorIds_.fill(kInvalidActorId);
        lastStemPeaks_.fill(0.0);
        lastStemLayerMix_.fill(0.0);
        lastStemReadiness_.fill(static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved));
        lastModalReturnPeak_ = 0.0;
        publishTelemetry();
    }

    bool setSessionSnapshot(const PerformanceSessionState& session) noexcept {
        if (!session.valid()) return false;
        const bool wasPlaying = session_.playing;
        session_ = session;
        synchronizeMorceauTransport(wasPlaying, session_.playing);
        targetFrequency_ = frequencyFor(session_);
        targetGain_ = targetGainFor(session_);
        ecology_.setMacroState(session_);
        noteManualInteraction();
        return true;
    }

    bool setMacro(PerformanceMacro macro, double value) noexcept {
        if (!unit(value) || static_cast<std::uint8_t>(macro) >= static_cast<std::uint8_t>(PerformanceMacro::Count))
            return false;
        value = constrainedMacroValue(macro, value);
        applyMacro(session_, macro, value);
        if (morphActive_) {
            applyMacro(morphSource_.session, macro, value);
            applyMacro(morphTarget_.session, macro, value);
        }
        targetFrequency_ = frequencyFor(session_);
        targetGain_ = targetGainFor(session_);
        ecology_.setMacroState(session_);
        noteManualInteraction();
        return true;
    }

    bool setTempoBpm(double tempoBpm) noexcept {
        if (!prepared_ || !finite(tempoBpm) || tempoBpm < 20.0 || tempoBpm > 300.0) return false;
        session_.tempoBpm = tempoBpm;
        if (morphActive_) {
            morphSource_.session.tempoBpm = tempoBpm;
            morphTarget_.session.tempoBpm = tempoBpm;
        }
        ecology_.setMacroState(session_);
        noteManualInteraction();
        return true;
    }

    bool setQuantizedGestures(bool enabled) noexcept {
        if (!prepared_) return false;
        quantizedGestures_ = enabled;
        noteManualInteraction();
        return true;
    }

    bool setGestureGrid(bool enabled, GestureGridDivision division) noexcept {
        if (!prepared_ || !validGestureGridDivision(division)) return false;
        quantizedGestures_ = enabled;
        gestureGridDivision_ = division;
        ecology_.setGestureGrid(enabled, division);
        noteManualInteraction();
        return true;
    }

    bool setPlaying(bool playing) noexcept {
        if (!prepared_) return false;
        const bool wasPlaying = session_.playing;
        session_.playing = playing;
        synchronizeMorceauTransport(wasPlaying, playing);
        // A medição da peça recomeça com o PLAY (a tomada abre junto).
        if (playing && !wasPlaying) {
            meter_.resetIntegrated();
            limiterMaxReduction_ = 0.0;
        }
        if (playing) {
            stopDeClickRemaining_ = 0U;
            silenceTarget_ = 0.0;
        } else {
            // STOP is still an interruption, not a musical coda. A tiny
            // output-boundary ramp prevents a discontinuity in the device and
            // the WAV, then reaches absolute silence; it introduces no tail
            // and no new musical material.
            stopDeClickSamples_ = static_cast<std::uint32_t>(std::clamp(
                std::llround(sampleRate_ * 0.008), 16LL, 2048LL));
            stopDeClickRemaining_ = stopDeClickSamples_;
            silenceTarget_ = 1.0;
        }
        targetGain_ = targetGainFor(session_);
        noteManualInteraction();
        return true;
    }

    // This is the intentional musical ending. STOP remains an interruption:
    // it closes the output immediately and must never be presented as coda.
    bool beginMorceauCoda() noexcept {
        if (!prepared_ || !session_.playing || !morceau_.beginCoda()) return false;
        // A transport-only engine still needs a valid technical coda for
        // tests, device transitions and an empty field. When bodies exist,
        // the ecology adds its seed-derived compositional ending.
        if (ecology_.voiceCount() > 0U) ecology_.beginCoda();
        noteManualInteraction();
        return true;
    }

    PerformanceMorceauState morceauState() const noexcept { return morceau_.state(); }

    bool setPanicSafe(bool enabled) noexcept {
        if (!prepared_) return false;
        panicSafe_ = enabled;
        if (!enabled) silenceTarget_ = std::min(silenceTarget_, 0.15);
        targetGain_ = targetGainFor(session_);
        noteManualInteraction();
        return true;
    }

    bool setOutputTrimDb(double decibels) noexcept {
        if (!prepared_ || !finite(decibels) || decibels < -24.0 || decibels > 12.0) return false;
        masterTrimDb_ = decibels;
        masterTrimOverride_ = true;
        session_.outputTrimDb = decibels;
        if (morphActive_) {
            morphSource_.session.outputTrimDb = decibels;
            morphTarget_.session.outputTrimDb = decibels;
        }
        targetGain_ = targetGainFor(session_);
        noteManualInteraction();
        return true;
    }

    double outputTrimDb() const noexcept { return masterTrimOverride_ ? masterTrimDb_ : session_.outputTrimDb; }

    bool setActorPresence(std::uint16_t actorId, double presence) noexcept {
        const bool changed = prepared_ && ecology_.setPresence(actorId, presence);
        if (changed) noteManualInteraction();
        return changed;
    }

    bool setModuleControl(std::uint16_t actorId, PerformanceModuleControl control,
                          double value) noexcept {
        const bool changed = prepared_ && ecology_.setModuleControl(actorId, control, value);
        if (changed) noteManualInteraction();
        return changed;
    }

    bool setMotionField(PerformanceMotionField field) noexcept {
        const bool changed = prepared_ && ecology_.setMotionField(field);
        if (changed) noteManualInteraction();
        return changed;
    }

    bool setPopulationPhenomenon(PerformancePopulationPhenomenon phenomenon,
                                 double intensity) noexcept {
        const bool changed = prepared_ && ecology_.setPopulationPhenomenon(phenomenon, intensity);
        if (changed) noteManualInteraction();
        return changed;
    }

    PerformancePopulationPhenomenon populationPhenomenon() const noexcept {
        return ecology_.populationPhenomenon();
    }

    double populationPhenomenonIntensity() const noexcept {
        return ecology_.populationPhenomenonIntensity();
    }

    bool setActorMotionField(std::uint16_t actorId, PerformanceMotionField field) noexcept {
        const bool changed = prepared_ && ecology_.setActorMotionField(actorId, field);
        if (changed) noteManualInteraction();
        return changed;
    }

    bool setActorMotionDirection(std::uint16_t actorId, bool forward) noexcept {
        const bool changed = prepared_ && ecology_.setActorMotionDirection(actorId, forward);
        if (changed) noteManualInteraction();
        return changed;
    }

    bool setSeedCollage(std::uint32_t sourceSeed, double amount) noexcept {
        const bool changed = prepared_ && ecology_.setSeedCollage(sourceSeed, amount);
        if (changed) noteManualInteraction();
        return changed;
    }

    bool setActorSeedVariation(std::uint16_t actorId, double amount) noexcept {
        const bool changed = prepared_ && ecology_.setActorSeedVariation(actorId, amount);
        if (changed) noteManualInteraction();
        return changed;
    }

    bool setRelationIntensity(std::uint16_t sourceActorId, std::uint16_t destinationActorId,
                              double intensity) noexcept {
        const bool changed = prepared_ && ecology_.setRelationIntensity(sourceActorId, destinationActorId, intensity);
        if (changed) noteManualInteraction();
        return changed;
    }

    bool setRelationPhenomenon(std::uint16_t sourceActorId,
                               std::uint16_t destinationActorId,
                               PerformanceRelationalPhenomenon phenomenon,
                               double resonanceRatio = 1.0) noexcept {
        const bool changed = prepared_ && ecology_.setRelationPhenomenon(
            sourceActorId, destinationActorId, phenomenon, resonanceRatio);
        if (changed) noteManualInteraction();
        return changed;
    }

    PerformanceRelationalPhenomenon relationPhenomenon(
        std::uint16_t sourceActorId, std::uint16_t destinationActorId) const noexcept {
        return ecology_.relationPhenomenon(sourceActorId, destinationActorId);
    }

    bool setActorStar(std::uint16_t actorId, PerformanceModuleBody star) noexcept {
        return prepared_ && ecology_.setActorStar(actorId, star);
    }
    PerformanceModuleBody actorStar(std::uint16_t actorId) const noexcept { return ecology_.actorStar(actorId); }
    bool setConstellationMotion(PerformanceConstellationMotion motion) noexcept {
        return prepared_ && ecology_.setConstellationMotion(motion);
    }
    bool setMotionFamily(std::uint16_t actorId, PerformanceMotionFamily family) noexcept {
        return prepared_ && ecology_.setMotionFamily(actorId, family);
    }
    bool setSkyAnchor(std::uint16_t actorId, double x, double y) noexcept {
        return prepared_ && ecology_.setSkyAnchor(actorId, x, y);
    }
    bool setActorPosition(std::uint16_t actorId, double x, double y) noexcept {
        const bool changed = prepared_ && ecology_.setActorPosition(actorId, x, y);
        if (changed) noteManualInteraction();
        return changed;
    }

    bool setStemGain(std::uint16_t actorId, double gain) noexcept {
        const bool changed = prepared_ && ecology_.setStemGain(actorId, gain);
        if (changed) noteManualInteraction();
        return changed;
    }

    // Saída de stems: um buffer mono por corpo (até kPerformanceStemActorCapacity),
    // pré-alocado por quem chama e definido fora do áudio. Cada amostra é o
    // corpo antes da espacialização global, com o mesmo ganho da mistura: a
    // soma dos stems reconstrói a mistura mono. nullptr desliga.
    void setStemTap(float* const* buffers, std::size_t count, std::uint32_t capacity) noexcept {
        stemTap_ = buffers;
        stemTapCount_ = std::min(count, kPerformanceStemActorCapacity);
        stemTapCapacity_ = capacity;
    }

    // Variação do mixer: brilho e mistura conduzidos pela seed e pelo
    // movimento da Constelação. Desligada, o mixer é só o do músico.
    void setMixerVariation(bool enabled) noexcept { ecology_.setMixerVariation(enabled); }
    void setStarCapacity(std::size_t capacity) noexcept { ecology_.setStarCapacity(capacity); }
    bool mixerVariation() const noexcept { return ecology_.mixerVariation(); }
    double starBrightness(std::size_t index) const noexcept { return ecology_.starBrightness(index); }

    // 0 desliga a trama do céu (som idêntico ao anterior a ela); 1 é o padrão.
    void setSkyInfluence(double influence) noexcept { ecology_.setSkyInfluence(influence); }
    const PerformanceSkyDirector& skyDirector() const noexcept { return ecology_.skyDirector(); }
    double fractureLevel() const noexcept { return ecology_.fractureLevel(); }
    std::uint32_t tearCuts() const noexcept { return ecology_.tearCuts(); }
    std::uint16_t tornActorId() const noexcept { return ecology_.tornActorId(); }

    bool setStemMuted(std::uint16_t actorId, bool muted) noexcept {
        const bool changed = prepared_ && ecology_.setStemMuted(actorId, muted);
        if (changed) noteManualInteraction();
        return changed;
    }

    bool setStemSolo(std::uint16_t actorId, bool soloed) noexcept {
        const bool changed = prepared_ && ecology_.setStemSolo(actorId, soloed);
        if (changed) noteManualInteraction();
        return changed;
    }

    double actorPresence(std::uint16_t actorId) const noexcept { return ecology_.presence(actorId); }

    // Control-side capability query. The ecology may intentionally cap a
    // larger document to the realtime-safe body limit; callers must be able
    // to distinguish a represented actor from a merely declared one.
    bool hasActor(std::uint16_t actorId) const noexcept {
        return ecology_.presence(actorId) >= 0.0;
    }
    bool supportsActorSeedVariation(std::uint16_t actorId) const noexcept {
        return ecology_.supportsActorSeedVariation(actorId);
    }
    bool supportsActorPosition(std::uint16_t actorId) const noexcept {
        return ecology_.supportsActorPosition(actorId);
    }
    bool supportsModuleControl(std::uint16_t actorId) const noexcept {
        return ecology_.supportsModuleControl(actorId);
    }
    double relationIntensity(std::uint16_t sourceActorId, std::uint16_t destinationActorId) const noexcept {
        return ecology_.relationIntensity(sourceActorId, destinationActorId);
    }
    double stemGain(std::uint16_t actorId) const noexcept { return ecology_.stemGain(actorId); }
    bool stemMuted(std::uint16_t actorId) const noexcept { return ecology_.stemMuted(actorId); }
    std::uint16_t soloActorId() const noexcept { return ecology_.soloActorId(); }
    PerformanceSpatialPoint actorPosition(std::uint16_t actorId) const noexcept {
        return ecology_.actorPosition(actorId);
    }
    PerformanceMotionPoint actorLivePosition(std::uint16_t actorId) const noexcept {
        return ecology_.actorLivePosition(actorId);
    }
    PerformanceSpatialGameEvent lastSpatialGameEvent() const noexcept {
        return ecology_.lastSpatialGameEvent();
    }
    const PerformanceGamePlan& gamePlan() const noexcept { return gamePlan_; }
    bool playing() const noexcept { return session_.playing; }

    bool setScene(const PerformanceScene& scene, const PerformanceTerritory& territory,
                  bool preserveLiveOverrides = false,
                  bool preserveTransport = false) noexcept {
        if (!scene.valid(territory)) return false;
        const bool wasPlaying = session_.playing;
        session_ = scene.session;
        // A scene is compositional material.  During a recall its stored
        // PLAY/STOP value must never interrupt the running morceau: a scene
        // can have been captured while stopped, but recalling it must not
        // close the audio envelope under an active performer.
        if (!preserveTransport)
            synchronizeMorceauTransport(wasPlaying, session_.playing);
        territoryIdentity_ = territory.identity;
        gamePlan_ = territory.gamePlan;
        session_.memory = std::min(session_.memory, territoryIdentity_.memoryLimit);
        session_.risk = std::min(session_.risk, territoryIdentity_.riskLimit);
        if (!masterTrimOverride_) masterTrimDb_ = session_.outputTrimDb;
        else session_.outputTrimDb = masterTrimDb_;
        targetFrequency_ = frequencyFor(session_);
        targetGain_ = targetGainFor(session_);
        // A territory installation is allowed to establish a fresh state.
        // A scene recall is not: the performer owns live presences, positions
        // and cable intensities until an explicit action changes them.
        if (!preserveLiveOverrides) ecology_.clearOverrides();
        ecology_.setTerritoryIdentity(territoryIdentity_, territory.seed, gamePlan_);
        ecology_.setMorph(PerformanceSceneMorpher::morph(scene, scene, 1.0));
        preparedReservedStemActorId_ = kInvalidActorId;
        for (std::size_t index = 0U; index < scene.constellation.actorCount; ++index) {
            const auto& actor = scene.constellation.actors[index];
            // Reserve telemetry only when the actor was actually excluded by
            // the active safety limit. Comparing two compile-time capacities
            // duplicated actor 5 once the limit itself had already admitted it.
            if (actor.actorId == 5U && ecology_.presence(actor.actorId) < 0.0) {
                preparedReservedStemActorId_ = actor.actorId;
                break;
            }
        }
        ecology_.setMacroState(session_);
        ecology_.setPopulationPhenomenon(session_.populationPhenomenon,
            session_.populationPhenomenonEnabled ? session_.populationPhenomenonIntensity : 0.0);
        morphActive_ = false;
        return true;
    }

    bool setTerritoryScene(const PerformanceScene& scene,
                           const PerformanceTerritory& territory) noexcept {
        const bool wasPlaying = session_.playing;
        if (!setScene(scene, territory)) return false;
        const bool sceneRequestedPlaying = session_.playing;
        session_.playing = wasPlaying;
        synchronizeMorceauTransport(sceneRequestedPlaying, wasPlaying);
        targetGain_ = targetGainFor(session_);
        ecology_.resetLiveControls();
        return true;
    }

    // Scene recall changes composition, never the explicit transport decision
    // made by the performer. A prepared territory is the only path allowed to
    // establish a new initial PLAY/STOP state.
    bool recallScene(const PerformanceScene& scene, const PerformanceTerritory& territory) noexcept {
        const bool wasPlaying = session_.playing;
        if (!setScene(scene, territory, true, true)) return false;
        session_.playing = wasPlaying;
        targetGain_ = targetGainFor(session_);
        return true;
    }

    bool beginSceneMorph(const PerformanceScene& source, const PerformanceScene& target,
                         const PerformanceTerritory& territory, double seconds) noexcept {
        if (!source.valid(territory) || !target.valid(territory) || source.session.territorySeed != target.session.territorySeed
            || !finite(seconds) || seconds < 0.02 || seconds > 30.0)
            return false;
        const bool wasPlaying = session_.playing;
        if (!recallScene(source, territory)) return false;
        morphSource_ = source;
        morphTarget_ = target;
        morphSource_.session.playing = wasPlaying;
        morphTarget_.session.playing = wasPlaying;
        morphSeconds_ = seconds;
        morphProgress_ = 0.0;
        morphActive_ = true;
        return true;
    }

    bool pushGesture(const PerformanceGestureEvent& event) noexcept {
        if (!prepared_ || !event.valid(maximumBlockSize_)) return false;
        auto scheduled = event;
        if (quantizedGestures_ && shouldQuantize(event.type)) {
            scheduled.sampleTimestamp = nextGridSample();
            scheduled.blockOffset = 0U;
        }
        const bool queued = events_.push(scheduled);
        if (queued) noteManualInteraction();
        return queued;
    }

    void processBlock(float* left, float* right, std::uint32_t samples) noexcept {
        if (!prepared_ || left == nullptr || samples == 0U || samples > maximumBlockSize_) {
            if (samples > maximumBlockSize_) ++xrunCount_;
            publishTelemetry();
            return;
        }
        if (channels_ == 2U && right == nullptr) {
            ++xrunCount_;
            publishTelemetry();
            return;
        }

        const std::uint64_t blockEnd = sampleCursor_ + samples;
        advanceSceneMorph(samples);
        const auto manualActivity = manualActivitySerial_.load(std::memory_order_relaxed);
        if (manualActivity != observedManualActivity_) {
            observedManualActivity_ = manualActivity;
            spatialGameSampleAccumulator_ = 0U;
            performerPauseSamples_ = 0U;
        }
        spatialGameSampleAccumulator_ += samples;
        performerPauseSamples_ = performerPauseSamples_ > std::numeric_limits<std::uint64_t>::max() - samples
            ? std::numeric_limits<std::uint64_t>::max() : performerPauseSamples_ + samples;
        const auto narrativeBeats = ecology_.nextNarrativePauseBeats();
        const auto safeTempo = std::clamp(session_.tempoBpm, 20.0, 300.0);
        const auto autonomousInterval = static_cast<std::uint64_t>(std::max(
            sampleRate_ * 0.75,
            std::round(sampleRate_ * (60.0 / safeTempo)
                       * static_cast<double>(narrativeBeats))));
        // The seed takes a bounded narrative turn after the performer pauses.
        // Intervention colours the live hand gesture; it must not silence the
        // narrative merely because it is numerically higher than autonomy.
        // Autonomy at zero remains the explicit opt-out.
        if (session_.playing && session_.autonomy > 0.02
            && autonomousInterval > 0U && spatialGameSampleAccumulator_ >= autonomousInterval) {
            spatialGameSampleAccumulator_ %= autonomousInterval;
            ecology_.advanceSpatialGame();
        }
        updateCodaReadiness();
        double peak = 0.0;
        std::array<double, kPerformanceStemActorCapacity> stemPeaks{};
        std::array<double, kPerformanceStemActorCapacity> stemLayerMix{};
        std::array<std::uint16_t, kPerformanceStemActorCapacity> stemActorIds{
            kInvalidActorId, kInvalidActorId, kInvalidActorId, kInvalidActorId, kInvalidActorId};
        std::array<std::uint8_t, kPerformanceStemActorCapacity> stemLifeStates{};
        std::array<std::uint8_t, kPerformanceStemActorCapacity> stemReadiness{};
        stemReadiness.fill(static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved));
        std::size_t stemCount = 0U;
        double modalReturnPeak = 0.0;
        master_.setMasterGain(std::pow(10.0, outputTrimDb() / 20.0));
        for (std::uint32_t index = 0U; index < samples; ++index) {
            dispatchDueEvents(sampleCursor_ + index, blockEnd);
            updateTargets();
            gain_ += (targetGain_ - gain_) * 0.0018;
            frequency_ += (targetFrequency_ - frequency_) * 0.0008;
            tear_ *= 0.9991;
            scar_ *= 0.999988;
            repair_ *= 0.999970;
            mutate_ *= 0.999985;
            wild_ *= 0.999976;
            silence_ += (silenceTarget_ - silence_) * 0.0045;
            if (silenceHoldSamples_ > 0U) --silenceHoldSamples_;

            const double tokenTilt = static_cast<double>((wildToken_ >> 8U) & 0xffU) / 255.0;
            const double tokenGrain = static_cast<double>((wildToken_ >> 16U) & 0xffU) / 255.0;
            const double wildMutate = clampUnit(mutate_ + wild_ * (0.32 + tokenTilt * 0.46));
            const double phaseIncrement = (frequency_ * (1.0 + mutate_ * 0.08
                + wild_ * (0.025 + tokenGrain * 0.07))) / sampleRate_;
            phase_ += phaseIncrement;
            if (phase_ >= 1.0) phase_ -= std::floor(phase_);
            const double scarEnergy = std::max(tear_, scar_ * (0.28 + session_.risk * 0.42));
            // The piece-level envelope is applied only at the final engine
            // boundary. Modules remain free to compose their own opening and
            // coda material; this layer prevents clicks and gives an explicit
            // ending its reliable fade/cauda semantics.
            const double morceauGain = morceau_.nextGain();
            double boundaryGain = morceauGain;
            if (stopDeClickRemaining_ > 0U) {
                const auto elapsed = stopDeClickSamples_ - stopDeClickRemaining_;
                const auto phase = static_cast<double>(elapsed)
                    / static_cast<double>(std::max(1U, stopDeClickSamples_));
                boundaryGain = 0.5 + 0.5 * std::cos(phase * kPi);
                --stopDeClickRemaining_;
                if (stopDeClickRemaining_ == 0U) {
                    silence_ = 1.0;
                    gain_ = 0.0;
                }
            }
            const double amplitude = gain_ * (1.0 - silence_) * boundaryGain;
            // No engine-level safety oscillator: it made an empty or dormant
            // constellation emit a permanent sine, and TENSION was directly
            // changing its pitch. Silence between events is a valid musical
            // state; the ecology is the sole source of audible material.
            double ecological = 0.0;
            // Em STOP, depois do de-click, a saída é exatamente zero: o céu
            // fica parado (não calcula estrelas para jogar fora) e retoma de
            // onde estava no próximo PLAY. Achado da auditoria de 28/10 (4.2).
            // Vale depois de um STOP (silêncio pleno) e também ao abrir, antes
            // do primeiro PLAY (ganho ainda exatamente zero).
            const bool stoppedSilent = !session_.playing && stopDeClickRemaining_ == 0U
                && (silence_ >= 1.0 || gain_ == 0.0);
            if (ecology_.voiceCount() > 0U && !stoppedSilent) {
                const auto stems = ecology_.renderStems(sampleRate_, wildMutate, scarEnergy, repair_,
                                                        holding_ ? 1.0 : 0.0, wildTargetActorId_, tearTargetActorId_,
                                                        repairTargetActorId_);
                ecological = stems.mixedMono();
                stemCount = stems.actorCount;
                for (std::size_t stem = 0U; stem < stems.actorCount; ++stem) {
                    stemActorIds[stem] = stems.actors[stem].actorId;
                    stemPeaks[stem] = std::max(stemPeaks[stem], std::fabs(stems.actors[stem].mono));
                    stemLayerMix[stem] = std::max(stemLayerMix[stem],
                        ecology_.internalLayerMix(stems.actors[stem].actorId));
                    stemLifeStates[stem] = static_cast<std::uint8_t>(stems.actors[stem].life);
                }
                modalReturnPeak = std::max(modalReturnPeak, std::fabs(stems.modalReturn));
                if (stemTap_ != nullptr && index < stemTapCapacity_)
                    for (std::size_t stem = 0U; stem < stemTapCount_; ++stem)
                        if (stemTap_[stem] != nullptr)
                            stemTap_[stem][index] = stem < stems.actorCount
                                ? static_cast<float>(finiteSample(stems.actors[stem].mono * amplitude)) : 0.0f;
            } else {
                if (stemTap_ != nullptr && index < stemTapCapacity_)
                    for (std::size_t stem = 0U; stem < stemTapCount_; ++stem)
                        if (stemTap_[stem] != nullptr) stemTap_[stem][index] = 0.0f;
                // Parado, os corpos continuam existindo para a telemetria.
                stemCount = std::min(ecology_.voiceCount(), stemActorIds.size());
                for (std::size_t stem = 0U; stem < stemCount; ++stem) {
                    stemActorIds[stem] = ecology_.actorIdAt(stem);
                    stemLifeStates[stem] = static_cast<std::uint8_t>(ecology_.lifeAt(stem));
                }
            }
            // Space is both a macro and a live relation. The orbit relation
            // therefore changes the heard stereo field, rather than remaining
            // a graphical cable only.
            const auto spatial = clampUnit(session_.space + ecology_.spacePressure() * 0.62);
            const double mono = finiteSample(ecological * amplitude);
            // ESPAÇO must not inject an independent pitched oscillator. The
            // previous pure sine here was heard as a fixed, tiresome tone.
            // Spatial motion now reshapes the current stem material, using a
            // slow phase only as a stereo weighting source.
            const double spatialMotion = mono * std::sin(
                (phase_ * 0.37 + spatial * 0.11) * kTwoPi) * spatial * 0.16;
            double leftSample = mono * (0.94 - spatial * 0.20) + spatialMotion * (1.0 - spatial * 0.35);
            double rightSample = channels_ == 2U
                ? mono * (0.94 + spatial * 0.20) - spatialMotion * (0.55 + spatial * 0.35)
                : leftSample;
            // O master (30/10): ganho fixo → MASTER → limitador → teto.
            // Parado e já no silêncio do STOP, a saída é zero exato: o master
            // zera o seu estado uma vez (o filtro anti-DC e o atraso não
            // deixam cauda — o contrato de silêncio do STOP).
            if (!session_.playing && amplitude == 0.0) {
                if (!masterSilent_) {
                    master_.reset();
                    masterSilent_ = true;
                }
                leftSample = 0.0;
                rightSample = 0.0;
            } else {
                masterSilent_ = false;
                master_.process(leftSample, rightSample);
                limiterMaxReduction_ = std::max(limiterMaxReduction_, 1.0 - master_.limiterGain());
            }
            leftSample = protectOutputPeak(leftSample);
            rightSample = protectOutputPeak(rightSample);
            meter_.process(leftSample, rightSample);
            left[index] = static_cast<float>(leftSample);
            if (channels_ == 2U) right[index] = static_cast<float>(rightSample);
            peak = std::max(peak, std::fabs(leftSample));
            peak = std::max(peak, std::fabs(rightSample));
        }
        sampleCursor_ += samples;
        lastPeak_ = peak;
        for (std::size_t stem = 0U; stem < stemCount; ++stem) {
            if (stemPeaks[stem] > 0.0001) {
                stemReadiness[stem] = static_cast<std::uint8_t>(PerformanceStemReadiness::Audible);
            } else if (stemLifeStates[stem] == static_cast<std::uint8_t>(ActorLifeState::Dormant)) {
                stemReadiness[stem] = static_cast<std::uint8_t>(PerformanceStemReadiness::Prepared);
            } else {
                stemReadiness[stem] = static_cast<std::uint8_t>(PerformanceStemReadiness::Silent);
            }
        }
        // A reserved fifth stem is an honest part of telemetry even while the
        // measured host limit keeps it outside the audio callback. It has no
        // peak and is explicitly Prepared, never presented as audible.
        if (preparedReservedStemActorId_ != kInvalidActorId
            && stemCount < stemActorIds.size()) {
            stemActorIds[stemCount] = preparedReservedStemActorId_;
            stemLifeStates[stemCount] = static_cast<std::uint8_t>(ActorLifeState::Dormant);
            stemReadiness[stemCount] = static_cast<std::uint8_t>(PerformanceStemReadiness::Prepared);
            ++stemCount;
        }
        lastStemCount_ = stemCount;
        lastStemActorIds_ = stemActorIds;
        lastStemPeaks_ = stemPeaks;
        lastStemLayerMix_ = stemLayerMix;
        lastStemLifeStates_ = stemLifeStates;
        lastStemReadiness_ = stemReadiness;
        lastModalReturnPeak_ = modalReturnPeak;
        publishTelemetry();
    }

    PerformanceTelemetrySnapshot readTelemetry() const noexcept { return telemetry_.snapshot(); }
    bool prepared() const noexcept { return prepared_; }
    double tempoBpm() const noexcept { return session_.tempoBpm; }
    std::size_t activeActorCount() const noexcept { return ecology_.voiceCount(); }
    // The host owns the device deadline. It reports a missed callback budget
    // here; this is deliberately not presented as a hardware-driver underrun.
    void reportDeadlineMiss() noexcept {
        if (prepared_ && xrunCount_ < std::numeric_limits<std::uint32_t>::max()) ++xrunCount_;
    }

private:
    static constexpr double kTwoPi = 6.28318530717958647692;
    static constexpr double kPi = 3.14159265358979323846;

    bool prepared_ = false;
    double sampleRate_ = 48000.0;
    std::uint32_t maximumBlockSize_ = 0U;
    std::uint32_t channels_ = 0U;
    PerformanceSessionState session_{};
    TerritoryIdentity territoryIdentity_{};
    PerformanceGamePlan gamePlan_{};
    FixedEventQueue<PerformanceGestureEvent, kEventQueueCapacity> events_{};
    PerformanceGestureEvent deferredEvent_{};
    bool deferred_ = false;
    double phase_ = 0.0;
    std::uint64_t sampleCursor_ = 0U;
    std::uint64_t spatialGameSampleAccumulator_ = 0U;
    std::uint64_t performerPauseSamples_ = 0U;
    std::atomic<std::uint64_t> manualActivitySerial_{0U};
    std::uint64_t observedManualActivity_ = 0U;
    double gain_ = 0.0;
    double targetGain_ = 0.0;
    double codaReadiness_ = 0.0;
    double frequency_ = 110.0;
    double targetFrequency_ = 110.0;
    double mutate_ = 0.0;
    double wild_ = 0.0;
    std::uint32_t wildToken_ = 0U;
    std::uint16_t wildTargetActorId_ = kInvalidActorId;
    std::uint16_t tearTargetActorId_ = kInvalidActorId;
    std::uint16_t repairTargetActorId_ = kInvalidActorId;
    double tear_ = 0.0;
    double scar_ = 0.0;
    double repair_ = 0.0;
    double silence_ = 0.0;
    double silenceTarget_ = 0.0;
    std::uint32_t silenceHoldSamples_ = 0U;
    std::uint32_t stopDeClickSamples_ = 0U;
    std::uint32_t stopDeClickRemaining_ = 0U;
    bool holding_ = false;
    bool quantizedGestures_ = false;
    GestureGridDivision gestureGridDivision_ = GestureGridDivision::Beat;
    bool panicSafe_ = false;
    float* const* stemTap_ = nullptr;
    std::size_t stemTapCount_ = 0U;
    std::uint32_t stemTapCapacity_ = 0U;
    bool masterTrimOverride_ = false;
    double masterTrimDb_ = 0.0;
    double lastPeak_ = 0.0;
    PerformanceMasterStage master_{};
    bool masterSilent_ = false;
    double limiterMaxReduction_ = 0.0;
    PerformanceLoudnessMeter meter_{};
    std::size_t lastStemCount_ = 0U;
    std::array<std::uint16_t, kPerformanceStemActorCapacity> lastStemActorIds_{
        kInvalidActorId, kInvalidActorId, kInvalidActorId, kInvalidActorId, kInvalidActorId};
    std::array<double, kPerformanceStemActorCapacity> lastStemPeaks_{};
    std::array<double, kPerformanceStemActorCapacity> lastStemLayerMix_{};
    std::array<std::uint8_t, kPerformanceStemActorCapacity> lastStemLifeStates_{};
    std::array<std::uint8_t, kPerformanceStemActorCapacity> lastStemReadiness_{{
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved),
        static_cast<std::uint8_t>(PerformanceStemReadiness::Unreserved)}};
    double lastModalReturnPeak_ = 0.0;
    std::uint16_t preparedReservedStemActorId_ = kInvalidActorId;
    std::uint32_t xrunCount_ = 0U;
    PerformanceTelemetryMailbox telemetry_{};
    PerformanceActorEcology ecology_{};
    PerformanceMorceauLifecycle morceau_{};
    PerformanceScene morphSource_{};
    PerformanceScene morphTarget_{};
    double morphProgress_ = 0.0;
    double morphSeconds_ = 1.0;
    bool morphActive_ = false;

    void noteManualInteraction() noexcept {
        manualActivitySerial_.fetch_add(1U, std::memory_order_relaxed);
    }

    void synchronizeMorceauTransport(bool wasPlaying, bool playing) noexcept {
        if (playing && !wasPlaying) {
            // A new PLAY begins a new musical lifetime only from a terminal
            // state. Recalling scenes during an active performance must not
            // reset its envelope.
            morceau_.begin();
        } else if (!playing && wasPlaying && !morceau_.finished()) {
            morceau_.interrupt();
        }
    }

    static double finiteSample(double value) noexcept {
        return finite(value) ? value : 0.0;
    }

    // Transparent below -0.18 dBFS. Only an actual over-ceiling sample is
    // curved into the remaining headroom; this is not a running compressor.
    static double protectOutputPeak(double value) noexcept {
        if (!finite(value)) return 0.0;
        constexpr double ceiling = 0.98;
        const auto magnitude = std::fabs(value);
        if (magnitude <= ceiling) return value;
        const auto over = (magnitude - ceiling) / (1.0 - ceiling);
        const auto protectedMagnitude = ceiling + (1.0 - ceiling) * (over / (1.0 + over));
        return std::copysign(protectedMagnitude, value);
    }

    static void applyMacro(PerformanceSessionState& session, PerformanceMacro macro,
                           double value) noexcept {
        switch (macro) {
        case PerformanceMacro::Autonomy: session.autonomy = value; break;
        case PerformanceMacro::Intervention: session.intervention = value; break;
        case PerformanceMacro::Density: session.density = value; break;
        case PerformanceMacro::Tension: session.tension = value; break;
        case PerformanceMacro::Memory: session.memory = value; break;
        case PerformanceMacro::Risk: session.risk = value; break;
        case PerformanceMacro::Space: session.space = value; break;
        case PerformanceMacro::Depth: session.depth = value; break;
        case PerformanceMacro::Count: break;
        }
    }

    double constrainedMacroValue(PerformanceMacro macro, double value) const noexcept {
        if (macro == PerformanceMacro::Memory) return std::min(value, territoryIdentity_.memoryLimit);
        if (macro == PerformanceMacro::Risk) return std::min(value, territoryIdentity_.riskLimit);
        return value;
    }

    static std::uint32_t toMicros(double value) noexcept {
        return static_cast<std::uint32_t>(std::round(clampUnit(value) * 1000000.0));
    }

    double frequencyFor(const PerformanceSessionState& session) const noexcept {
        // This is a control-rate spatial phase, not an audible pitch. Keeping
        // it sub-audio prevents TENSION from creating a sustained tone while
        // still letting density/risk gently animate stereo displacement.
        const double value = 0.028 + session.density * 0.062 + session.risk * 0.038;
        return std::clamp(value, 0.02, 0.14);
    }

    double targetGainFor(const PerformanceSessionState& session) const noexcept {
        if (!session.playing) return 0.0;
        const double untrimmed = 0.06 + session.density * 0.19 + session.intervention * 0.05;
        // O MASTER (outputTrimDb) age no estágio de master, depois do
        // nivelador (PerformanceMasterStage), para não ser desfeito por ele.
        return panicSafe_ || session.panicSafe ? std::min(untrimmed, 0.08) : untrimmed;
    }

    void updateCodaReadiness() noexcept {
        const auto state = morceau_.state();
        if (!session_.playing || session_.autonomy <= 0.02
            || state == PerformanceMorceauState::Coda || state == PerformanceMorceauState::Tail
            || state == PerformanceMorceauState::Finalized || state == PerformanceMorceauState::Interrupted) {
            codaReadiness_ = 0.0;
            return;
        }
        const auto samplesPerBeat = std::max(1.0, sampleRate_ * 60.0 / std::clamp(session_.tempoBpm, 20.0, 300.0));
        const auto pauseBeats = static_cast<double>(performerPauseSamples_) / samplesPerBeat;
        const auto ageBeats = static_cast<double>(sampleCursor_) / samplesPerBeat;
        const auto& narrative = ecology_.lastSpatialGameEvent();
        const auto turns = narrative.valid() ? static_cast<double>(narrative.turn + 1U) : 0.0;
        const auto relationMean = ecology_.relationCount() > 0U
            ? ecology_.activeRelationMass() / static_cast<double>(ecology_.relationCount()) : 0.0;
        const auto material = std::clamp(session_.memory * 0.34 + session_.risk * 0.18
            + territoryIdentity_.relationBias * 0.22 + relationMean * 0.26, 0.0, 1.0);
        const auto seedPauseBeats = 96.0 + static_cast<double>(session_.territorySeed % 97U);
        const auto pauseTarget = seedPauseBeats * (1.08 - material * 0.32);
        const auto ageTarget = 128.0 + static_cast<double>((session_.territorySeed >> 8U) % 97U);
        const auto turnTarget = 2.0 + static_cast<double>((session_.territorySeed >> 16U) % 3U);
        const auto pauseProgress = std::clamp(pauseBeats / std::max(1.0, pauseTarget), 0.0, 1.0);
        const auto ageProgress = std::clamp(ageBeats / ageTarget, 0.0, 1.0);
        const auto turnProgress = std::clamp(turns / turnTarget, 0.0, 1.0);
        codaReadiness_ = std::clamp(pauseProgress * 0.56 + ageProgress * 0.24
            + turnProgress * 0.20, 0.0, 1.0);
        if (pauseProgress >= 1.0 && ageProgress >= 1.0 && turnProgress >= 1.0)
            codaReadiness_ = 1.0;
    }

    std::uint64_t scheduledSample(const PerformanceGestureEvent& event) const noexcept {
        return event.sampleTimestamp + static_cast<std::uint64_t>(event.blockOffset);
    }

    static bool shouldQuantize(GestureType type) noexcept {
        return type == GestureType::Hold || type == GestureType::Release
            || type == GestureType::Mutate || type == GestureType::Rasgar
            || type == GestureType::Stutter || type == GestureType::TempoShift
            || type == GestureType::Recall;
    }

    std::uint64_t nextGridSample() const noexcept {
        const auto divisions = static_cast<std::uint8_t>(gestureGridDivision_);
        const auto samplesPerBeat = std::max<std::uint64_t>(1U,
            static_cast<std::uint64_t>(std::llround(sampleRate_ * 60.0 / session_.tempoBpm)));
        const auto samplesPerGrid = std::max<std::uint64_t>(1U, samplesPerBeat / divisions);
        return ((sampleCursor_ / samplesPerGrid) + 1U) * samplesPerGrid;
    }

    void dispatchDueEvents(std::uint64_t currentSample, std::uint64_t blockEnd) noexcept {
        if (deferred_ && scheduledSample(deferredEvent_) <= currentSample) {
            applyGesture(deferredEvent_);
            deferred_ = false;
        }
        if (deferred_) return;
        PerformanceGestureEvent event;
        while (events_.pop(event)) {
            if (scheduledSample(event) > blockEnd) {
                deferredEvent_ = event;
                deferred_ = true;
                return;
            }
            if (scheduledSample(event) <= currentSample) applyGesture(event);
            else {
                deferredEvent_ = event;
                deferred_ = true;
                return;
            }
        }
    }

    void applyGesture(const PerformanceGestureEvent& event) noexcept {
        switch (event.type) {
        case GestureType::Hold: holding_ = true; ecology_.setHolding(true); break;
        case GestureType::Release: holding_ = false; ecology_.setHolding(false); break;
        case GestureType::Mutate:
            mutate_ = std::max(mutate_, event.intensity);
            wild_ = std::max(wild_, event.intensity);
            wildToken_ = event.variationToken != 0U ? event.variationToken
                : static_cast<std::uint32_t>(session_.territorySeed ^ sampleCursor_);
            wildTargetActorId_ = event.targetActorId;
            break;
        case GestureType::Rasgar:
            tear_ = std::max(tear_, event.intensity);
            scar_ = std::max(scar_, event.intensity);
            tearTargetActorId_ = event.targetActorId;
            break;
        case GestureType::Interrupt: silenceTarget_ = std::max(silenceTarget_, event.intensity * 0.65); break;
        // A grid choice is otherwise silent configuration. Its cue is a
        // deliberately immediate, bounded recut so the performer hears the
        // timing surface they just selected; it is never quantized itself.
        case GestureType::GridCue:
            ecology_.triggerRecut(GestureType::Stutter, event.targetActorId, event.intensity,
                                  event.duration, event.variationToken);
            // The second, lighter recut makes the grid choice legible inside
            // a dense texture without turning the grid into a permanent LFO.
            ecology_.triggerRecut(GestureType::TempoShift, event.targetActorId,
                                  event.intensity * 0.42, event.duration,
                                  event.variationToken ^ 0x51EDU);
            break;
        case GestureType::Stutter:
        case GestureType::TempoShift:
            ecology_.triggerRecut(event.type, event.targetActorId, event.intensity,
                                  event.duration, event.variationToken);
            break;
        case GestureType::Recall:
            mutate_ *= 0.3; wild_ *= 0.16; tear_ *= 0.2; scar_ *= 0.32;
            repair_ = std::max(repair_, 0.82);
            repairTargetActorId_ = event.targetActorId;
            silenceTarget_ *= 0.4; silenceHoldSamples_ = 0U;
            break;
        case GestureType::Silence:
            silenceTarget_ = std::max(silenceTarget_, event.intensity);
            silenceHoldSamples_ = std::max(silenceHoldSamples_,
                                           static_cast<std::uint32_t>(sampleRate_ * 0.65));
            break;
        case GestureType::PanicSafe:
            panicSafe_ = true;
            holding_ = false;
            ecology_.setHolding(false);
            mutate_ = 0.0;
            wild_ = 0.0;
            wildTargetActorId_ = kInvalidActorId;
            tear_ = 0.0;
            tearTargetActorId_ = kInvalidActorId;
            scar_ = 0.0;
            ecology_.resetFracture();
            repair_ = 0.0;
            repairTargetActorId_ = kInvalidActorId;
            silenceTarget_ = std::max(silenceTarget_, 0.35);
            silenceHoldSamples_ = std::max(silenceHoldSamples_,
                                           static_cast<std::uint32_t>(sampleRate_ * 0.35));
            break;
        case GestureType::Count: break;
        }
    }

    void updateTargets() noexcept {
        if (!holding_) targetFrequency_ = frequencyFor(session_);
        targetGain_ = targetGainFor(session_);
        if (!session_.playing) silenceTarget_ = 1.0;
        else if (silenceTarget_ > 0.0 && silenceHoldSamples_ == 0U) silenceTarget_ *= 0.9994;
    }

    void advanceSceneMorph(std::uint32_t samples) noexcept {
        if (!morphActive_) return;
        morphProgress_ = std::min(1.0, morphProgress_ + static_cast<double>(samples) / (sampleRate_ * morphSeconds_));
        const auto morph = PerformanceSceneMorpher::morph(morphSource_, morphTarget_, morphProgress_);
        if (!morph.compatibleTerritory) {
            morphActive_ = false;
            return;
        }
        session_ = morph.session;
        session_.outputTrimDb = masterTrimDb_;
        targetFrequency_ = frequencyFor(session_);
        targetGain_ = targetGainFor(session_);
        ecology_.setMorph(morph);
        ecology_.setMacroState(session_);
        if (morphProgress_ >= 1.0) morphActive_ = false;
    }

    void publishTelemetry() noexcept {
        PerformanceTelemetrySnapshot snapshot;
        snapshot.cpuMilliPercent = 0U; // timing measurement belongs to the host layer.
        snapshot.outputPeakMicros = toMicros(lastPeak_);
        // True peak 4× (BS.1770, PerformanceLoudnessMeter, 30/10): o maior
        // desde a última publicação.
        snapshot.truePeakMicros = toMicros(meter_.truePeakLinear());
        snapshot.momentaryCentiLufs = static_cast<std::int16_t>(std::lround(meter_.momentary() * 100.0));
        snapshot.shortTermCentiLufs = static_cast<std::int16_t>(std::lround(meter_.shortTerm() * 100.0));
        snapshot.integratedCentiLufs = static_cast<std::int16_t>(std::lround(meter_.integrated() * 100.0));
        snapshot.maxTruePeakMicros = toMicros(meter_.maxTruePeak());
        snapshot.limiterMaxReductionCentiDb = static_cast<std::uint16_t>(std::lround(std::clamp(
            -20.0 * std::log10(std::max(1.0e-6, 1.0 - limiterMaxReduction_)), 0.0, 600.0) * 100.0));
        snapshot.xrunCount = xrunCount_;
        snapshot.activeActorCount = static_cast<std::uint16_t>(ecology_.voiceCount() > 0U
            ? ecology_.voiceCount() : (session_.playing ? 1U : 0U));
        snapshot.sceneId = session_.currentSceneId;
        snapshot.eventQueueDepth = static_cast<std::uint16_t>(events_.size() + (deferred_ ? 1U : 0U));
        snapshot.playing = session_.playing;
        snapshot.holding = holding_;
        snapshot.quantizedGestures = quantizedGestures_;
        snapshot.gestureGridDivision = static_cast<std::uint8_t>(gestureGridDivision_);
        snapshot.panicSafe = panicSafe_ || session_.panicSafe;
        snapshot.codaGesture = static_cast<std::uint8_t>(ecology_.codaGesture());
        snapshot.codaReadinessMicros = toMicros(codaReadiness_);
        const auto narrativeEvent = ecology_.lastSpatialGameEvent();
        snapshot.narrativeEventValid = narrativeEvent.valid();
        snapshot.narrativeTurn = narrativeEvent.turn;
        snapshot.narrativeMove = static_cast<std::uint8_t>(narrativeEvent.move);
        snapshot.narrativeStrategy = static_cast<std::uint8_t>(narrativeEvent.strategy);
        snapshot.narrativeRule = static_cast<std::uint8_t>(narrativeEvent.rule);
        snapshot.motionPhaseMicros = toMicros(ecology_.motionPhase());
        {
            const auto& director = ecology_.skyDirector();
            const auto count = std::min<std::size_t>(director.focusCount(), 3U);
            std::uint32_t packed = static_cast<std::uint32_t>(count);
            for (std::size_t index = 0U; index < count; ++index) {
                const auto& focus = director.focus(index);
                const std::uint32_t entry = (static_cast<std::uint32_t>(focus.caller) & 7U)
                    | ((static_cast<std::uint32_t>(focus.responder) & 7U) << 3U)
                    | ((static_cast<std::uint32_t>(focus.kind) & 15U) << 6U);
                packed |= entry << (2U + static_cast<std::uint32_t>(index) * 10U);
            }
            snapshot.skyFocusPacked = director.influence() > 0.0 ? packed : 0U;
            snapshot.skyTensionMicros = toMicros(director.tension());
            snapshot.skySection = static_cast<std::uint8_t>(director.section());
            snapshot.skyTempoMillis = static_cast<std::uint16_t>(std::lround(director.tempoFactor() * 1000.0));
            snapshot.skySwingPercent = static_cast<std::uint8_t>(std::lround(director.swing() * 100.0));
            snapshot.skyGroupingMask = ecology_.skyGroupingMask();
            snapshot.skyDriftBody = ecology_.skyDriftBody();
            snapshot.skyEclipseBody = ecology_.skyEclipseBody();
            for (std::size_t index = 0U; index < snapshot.skyChorusPhase.size(); ++index)
                snapshot.skyChorusPhase[index] = static_cast<std::uint8_t>(
                    std::clamp(director.chorusDelay(index), 0.0, 1.0) * 255.0);
            const auto drift = director.driftSlot();
            const auto eclipse = director.eclipsedSlot();
            snapshot.skyDriftSlot = drift < PerformanceSkyDirector::kBodies ? static_cast<std::uint8_t>(drift) : 255U;
            snapshot.skyEclipseSlot = eclipse < PerformanceSkyDirector::kBodies ? static_cast<std::uint8_t>(eclipse) : 255U;
            snapshot.skyMotionValid = director.positionsReady();
            for (std::size_t index = 0U; index < std::min<std::size_t>(snapshot.skyPositionX.size(),
                                                                      PerformanceSkyDirector::kBodies); ++index) {
                snapshot.skyPositionX[index] = static_cast<std::uint16_t>(std::clamp(director.shownX(index), 0.0, 1.0) * 65535.0);
                snapshot.skyPositionY[index] = static_cast<std::uint16_t>(std::clamp(director.shownY(index), 0.0, 1.0) * 65535.0);
            }
            for (std::size_t index = 0U; index < snapshot.starBrightnessMicros.size(); ++index)
                snapshot.starBrightnessMicros[index] = toMicros(director.brightness(index));
        }
        snapshot.motionField = static_cast<std::uint8_t>(ecology_.motionField());
        snapshot.stemCount = static_cast<std::uint16_t>(lastStemCount_);
        snapshot.stemActorIds = lastStemActorIds_;
        for (std::size_t index = 0U; index < lastStemPeaks_.size(); ++index) {
            snapshot.stemPeakMicros[index] = toMicros(lastStemPeaks_[index]);
            snapshot.stemLayerMixMicros[index] = toMicros(lastStemLayerMix_[index]);
            snapshot.stemMotionFields[index] = static_cast<std::uint8_t>(
                ecology_.motionField(lastStemActorIds_[index]));
            snapshot.stemMotionDirections[index] = ecology_.motionForward(lastStemActorIds_[index]) ? 1U : 0U;
        }
        snapshot.stemLifeStates = lastStemLifeStates_;
        snapshot.stemReadiness = lastStemReadiness_;
        snapshot.modalReturnPeakMicros = toMicros(lastModalReturnPeak_);
        snapshot.scarMicros = toMicros(scar_);
        snapshot.repairMicros = toMicros(repair_);
        snapshot.wildMicros = toMicros(wild_);
        telemetry_.publish(snapshot);
    }
};

} // namespace rasgo::performance
