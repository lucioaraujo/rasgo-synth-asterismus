#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

#include "PerformanceContracts.hpp"
#include "PerformanceMotionGeometry.hpp"
#include "PerformanceRelationalPhenomena.hpp"
#include "PerformanceSky.hpp"
#include "PerformanceSkyDirector.hpp"
#include "PerformanceStemFrame.hpp"
#include "PerformanceSpatialGame.hpp"
#include "PerformanceSeedTrajectory.hpp"
#include "PerformanceTerritoryFactory.hpp"
#include "ModalMatterAdapter.hpp"
#include "RasgoFractureAdapter.hpp"

namespace rasgo::performance {

// Native temporary bodies for Marco 2. Their purpose is to prove roles and
// relations in continuous audio before any Studio material is adapted.
class PerformanceActorEcology {
public:
    static constexpr std::size_t kVoiceCapacity = kPerformanceStemActorCapacity;
    enum class CodaGesture : std::uint8_t { Gather, Relay, Memory, Disperse };

    bool prepare(double sampleRate, std::uint32_t maximumBlockSize,
                 std::uint32_t channels) noexcept {
        if (!sky_.prepare(sampleRate)) return false;
        director_.prepare(sampleRate);
        sampleRate_ = sampleRate;
        fracturePrepared_ = fracture_.prepare(sampleRate, maximumBlockSize, channels);
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
        modalPrepared_ = modalMatter_.prepare(sampleRate, maximumBlockSize, channels);
        return modalPrepared_ && fracturePrepared_;
#else
        (void) sampleRate;
        (void) maximumBlockSize;
        (void) channels;
        return fracturePrepared_;
#endif
    }

    // O próximo território recomeça a peça mesmo com a mesma seed (território
    // novo, faixa recomeçada); chamar uma cena não passa por aqui.
    void restartSeed() noexcept { seedConfigured_ = false; }
    void reset() noexcept {
        seedConfigured_ = false;
        voices_ = {};
        tornActorId_ = kInvalidActorId;
        lastFractureBreaks_ = 0;
        relations_ = {};
        voiceCount_ = 0;
        relationCount_ = 0;
        soloActorId_ = kInvalidActorId;
        timePressure_ = 0.0;
        matterPressure_ = 0.0;
        frictionPressure_ = 0.0;
        spacePressure_ = 0.0;
        memoryPressure_ = 0.0;
        relationalPulseCoupling_ = 0.0;
        relationalTemporalFracture_ = 0.0;
        relationalMatterExchange_ = 0.0;
        relationalMatterFusion_ = 0.0;
        relationalSpatialCirculation_ = 0.0;
        relationalDynamicTension_ = 0.0;
        relationalMemoryTrace_ = 0.0;
        relationalSeedDeviation_ = 0.0;
        relationalSilence_ = 0.0;
        pressureRefreshCountdown_ = 0U;
        pitchRefreshCountdown_ = 0U;
        macroEnergyCompensation_ = 1.0;
        autonomy_ = intervention_ = density_ = tension_ = memory_ = risk_ = space_ = depth_ = 0.5;
        tempoBpm_ = 120.0;
        sky_.reset();
        director_.reset();
        lastFractureTear_ = 0.0;
        if (fracturePrepared_) fracture_.reset();
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
        modalClock_ = 0.0;
        modalLevel_ = 0.0;
        lastTear_ = 0.0;
        modalLastPhraseStep_ = std::numeric_limits<std::uint32_t>::max();
        if (modalPrepared_) modalMatter_.reset();
#endif
        spatialGameTurn_ = 0U;
        lastSpatialGameEvent_ = {};
        motionField_ = PerformanceMotionField::Orbit;
        populationPhenomenon_ = PerformancePopulationPhenomenon::CollectiveRotate;
        populationPhenomenonEnabled_ = false;
        populationPhenomenonIntensity_ = 0.65;
        motionPhase_ = 0.0;
        motionSeedOffset_ = 0.0;
        orchestrationPhase_ = 0.0;
        collageAmount_ = 0.0;
        stutterIntensity_ = 0.0;
        stutterPhase_ = 0.0;
        stutterRemainingSamples_ = 0U;
        stutterTargetActorId_ = kInvalidActorId;
        tempoShiftIntensity_ = 0.0;
        tempoShiftRemainingSamples_ = 0U;
        tempoShiftTargetActorId_ = kInvalidActorId;
        tempoShiftRatio_ = 1.0;
        codaActive_ = false;
        codaProgress_ = 0.0;
        codaLeaderIndex_ = 0U;
        codaGesture_ = CodaGesture::Gather;
        internalLayerMix_ = {};
        internalModuleSeedVariation_.fill(0.5);
        for (auto& controls : internalModuleControls_) controls.fill(0.5);
        internalModulePositions_ = {{{0.66, 0.66}, {0.33, 0.69}, {0.73, 0.38}, {0.42, 0.54}}};
    }

    void setMorph(const PerformanceSceneMorph& morph) noexcept {
        const auto previousRelations = relations_;
        const auto previousRelationCount = relationCount_;
        std::array<Voice, kVoiceCapacity> next{};
        const std::size_t wanted = std::min({morph.actorCount, next.size(), kPerformanceSafeBodyLimit});
        [[maybe_unused]] bool modalControlsTaken = false;
        for (std::size_t index = 0; index < wanted; ++index) {
            const auto& actor = morph.actors[index];
            auto& voice = next[index];
            voice.actorId = actor.actorId;
            voice.star = starForSlot(index);
            // A troca do performer (30/10) vale por cima da escolha da seed.
            for (const auto& swap : starSwaps_)
                if (swap.actorId == voice.actorId && swap.star != PerformanceModuleBody::Count) voice.star = swap.star;
            voice.role = actor.roleMorph >= 0.5 ? actor.targetRole : actor.sourceRole;
            voice.presence = actor.presence;
            voice.nourishment = actor.nourishment;
            voice.life = morph.amount >= 0.5 ? actor.targetLife : actor.sourceLife;
            voice.targetFrequency = frequencyFor(actor, voice.role);
            voice.x = index == 0U ? 0.16 : index == 1U ? 0.50 : index == 2U ? 0.84 : 0.50;
            voice.y = index == 0U ? 0.48 : index == 1U ? 0.28 : index == 2U ? 0.58 : 0.78;
            for (std::size_t previous = 0; previous < voiceCount_; ++previous) {
                if (voices_[previous].actorId != voice.actorId) continue;
                voice.phase = voices_[previous].phase;
                voice.gain = voices_[previous].gain;
                // A dança escolhida e o lugar desenhado seguem o mesmo ator.
                voice.family = voices_[previous].family;
                voice.skyAnchorX = voices_[previous].skyAnchorX;
                voice.skyAnchorY = voices_[previous].skyAnchorY;
                voice.skyAnchorSet = voices_[previous].skyAnchorSet;
                voice.frequency = voices_[previous].frequency;
                voice.pulsePhase = voices_[previous].pulsePhase;
                voice.pulseEnvelope = voices_[previous].pulseEnvelope;
                voice.phrasePhase = voices_[previous].phrasePhase;
                voice.phraseEnvelope = voices_[previous].phraseEnvelope;
                voice.phraseDuty = voices_[previous].phraseDuty;
                voice.phrasePitchSemitones = voices_[previous].phrasePitchSemitones;
                voice.phraseStep = voices_[previous].phraseStep;
                voice.phraseActive = voices_[previous].phraseActive;
                voice.motionEnvelope = voices_[previous].motionEnvelope;
                voice.motionField = voices_[previous].motionField;
                voice.motionForward = voices_[previous].motionForward;
                voice.seedVariationAmount = voices_[previous].seedVariationAmount;
                voice.stutterEnvelope = voices_[previous].stutterEnvelope;
                voice.depthFiltered = voices_[previous].depthFiltered;
                voice.localControls = voices_[previous].localControls;
                voice.orchestrationGain = voices_[previous].orchestrationGain;
                voice.populationEnvelope = voices_[previous].populationEnvelope;
                voice.presenceOverridden = voices_[previous].presenceOverridden;
                if (voice.presenceOverridden) voice.presence = voices_[previous].presence;
                voice.lifeOverridden = voices_[previous].lifeOverridden;
                if (voice.lifeOverridden) voice.life = voices_[previous].life;
                voice.stemGain = voices_[previous].stemGain;
                voice.targetStemGain = voices_[previous].targetStemGain;
                voice.stemMuted = voices_[previous].stemMuted;
                voice.x = voices_[previous].x;
                voice.y = voices_[previous].y;
                break;
            }
            if (voice.frequency == 0.0)
                voice.localControls = PerformanceTerritoryFactory::initialModuleControls(seed_, voice.actorId);
            voice.targetGain = targetGainFor(voice);
            if (voice.frequency == 0.0) voice.frequency = voice.targetFrequency;
            for (std::size_t control = 0U; control < voice.localControls.size(); ++control)
                sky_.setControl(voice.star, static_cast<PerformanceModuleControl>(control),
                                voice.localControls[control]);
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
            if (voice.star == PerformanceModuleBody::ModalResonator && !modalControlsTaken) {
                modalControls_ = voice.localControls;
                modalControlsTaken = true;
            }
#endif
        }
        voices_ = next;
        voiceCount_ = wanted;
        if (soloActorId_ != kInvalidActorId) {
            bool soloStillPresent = false;
            for (std::size_t index = 0; index < voiceCount_; ++index)
                soloStillPresent = soloStillPresent || voices_[index].actorId == soloActorId_;
            if (!soloStillPresent) soloActorId_ = kInvalidActorId;
        }
        relationCount_ = std::min(morph.relationCount, relations_.size());
        for (std::size_t index = 0; index < morph.relationCount; ++index) {
            if (index >= relations_.size()) break;
            const auto& frame = morph.relations[index];
            auto& relation = relations_[index];
            relation.sourceActorId = frame.relation.sourceActorId;
            relation.destinationActorId = frame.relation.destinationActorId;
            relation.kind = frame.relation.kind;
            relation.domain = frame.relation.domain;
            relation.phenomenon = frame.relation.phenomenonEnabled
                ? frame.relation.phenomenon : phenomenonForRelationKind(frame.relation.kind);
            relation.resonanceRatio = frame.relation.resonanceRatio;
            relation.phenomenonEnabled = frame.relation.phenomenonEnabled;
            relation.activity = frame.activity;
            relation.intensity = frame.relation.intensity;
            for (std::size_t previous = 0; previous < previousRelationCount; ++previous) {
                const auto& old = previousRelations[previous];
                if (old.sourceActorId == relation.sourceActorId
                    && old.destinationActorId == relation.destinationActorId
                    && old.kind == relation.kind && old.domain == relation.domain) {
                    relation.intensity = old.intensity;
                    relation.intensityOverridden = old.intensityOverridden;
                    if (!frame.relation.phenomenonEnabled) {
                        relation.phenomenon = old.phenomenon;
                        relation.resonanceRatio = old.resonanceRatio;
                        relation.phenomenonEnabled = old.phenomenonEnabled;
                    }
                    relation.previousSource = old.previousSource;
                    relation.previousDestination = old.previousDestination;
                    relation.observationReady = old.observationReady;
                    if (!relation.intensityOverridden)
                        relation.intensity = frame.relation.intensity;
                    break;
                }
            }
        }
        for (std::size_t index = 0U; index < relationCount_; ++index) {
            auto& relation = relations_[index];
            relation.sourceIndex = kVoiceCapacity;
            relation.destinationIndex = kVoiceCapacity;
            for (std::size_t voiceIndex = 0U; voiceIndex < voiceCount_; ++voiceIndex) {
                if (voices_[voiceIndex].actorId == relation.sourceActorId) relation.sourceIndex = voiceIndex;
                if (voices_[voiceIndex].actorId == relation.destinationActorId) relation.destinationIndex = voiceIndex;
            }
        }
        updatePressures();
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
        configureModalMatter();
#endif
    }

    std::size_t voiceCount() const noexcept { return voiceCount_; }

    // Immutable territory identity colours every subsequent realization.
    void setTerritoryIdentity(const TerritoryIdentity& identity,
                              std::uint32_t seed = 181U) noexcept {
        setTerritoryIdentity(identity, seed, PerformanceGamePlan{});
    }

    void setTerritoryIdentity(const TerritoryIdentity& identity,
                              std::uint32_t seed,
                              const PerformanceGamePlan& gamePlan) noexcept {
        if (!identity.valid()) return;
        // Chamar uma cena reaplica o território; com a mesma seed, o céu e as
        // trajetórias seguem de onde estão (2/11: antes toda troca de cena
        // reiniciava a forma do céu na ABERTURA — o PERCURSO ficava preso
        // entre abertura e desenvolvimento, e o clique numa cena também
        // voltava à abertura). Só uma seed nova recomeça a peça.
        const bool newSeed = !seedConfigured_ || seed != seed_;
        seedConfigured_ = true;
        territoryIdentity_ = identity;
        seed_ = seed;
        composition_ = compositionProfileForSeed(seed);
        ++seedCacheVersion_;
        motionSeedOffset_ = static_cast<double>(seed % 37U) * 0.17;
        if (newSeed) {
            spatialGame_.configure(seed, gamePlan);
            seedTrajectory_.configure(seed);
            collageTrajectory_.configure(seed);
            sky_.setSeed(seed);
            director_.setSeed(seed);
            director_.setTonalCenter(440.0 * seedTonalScale()
                * std::pow(2.0, (static_cast<double>(seedRootMidi()) - 69.0) / 12.0));
        }
        director_.setStrategy(gamePlan.strategy);
        if (newSeed) sky_.setSeedEvent(seedTrajectory_.event(0U));
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            voices_[index].star = starForSlot(index);
        if (fracturePrepared_ && newSeed) {
            fractureParameters_.seed = seed;
            fracture_.setParameters(fractureParameters_);
        }
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            voices_[index].targetFrequency = frequencyForVoice(voices_[index].actorId, voices_[index].role);
        updatePressures();
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
        configureModalMatter();
#endif
    }

    // One slow autonomous turn lets the seed's game answer the live field
    // during a pause in hand manipulation. It changes only musical pressures
    // and an existing relation intensity; it never writes actor positions.
    PerformanceSpatialGameEvent advanceSpatialGame() noexcept {
        PerformanceSpatialGameState state;
        state.seed = spatialGame_.seed();
        state.turn = spatialGameTurn_++;
        state.autonomy = autonomy_;
        state.density = density_;
        state.tension = tension_;
        state.memory = memory_;
        state.risk = risk_;
        state.field.actorCount = voiceCount_;
        state.field.relationCount = relationCount_;
        for (std::size_t index = 0U; index < voiceCount_; ++index) {
            state.field.actorIds[index] = voices_[index].actorId;
            // The narrative answers the bodies that are actually dancing now,
            // never a stale anchor. The anchor itself remains untouched.
            // O jogo espacial continua lendo a órbita (30/10): com a física
            // as estrelas ficam mais quietas e o jogo lia isso como pausa
            // longa — a peça calava. A física alimenta as relações e o céu.
            const auto live = livePosition(index, false);
            state.field.points[index] = {live.x, live.y};
        }
        for (std::size_t index = 0U; index < relationCount_; ++index)
            state.field.relationIntensities[index] = relations_[index].intensity;
        if (voiceCount_ > 0U) state.field.focusActorId = voices_[0].actorId;

        lastSpatialGameEvent_ = spatialGame_.propose(state);
        const auto& event = lastSpatialGameEvent_;
        if (!event.valid()) return event;
        const auto seededEvent = currentSeedEvent(event.turn);
        // The spatial game chooses the situated action; the seed trajectory
        // supplies the history around it. Both are bounded and control-only:
        // neither can redraw the musician's field or allocate in the callback.
        density_ = clampUnit(density_ + event.densityDelta
            + seededEvent.densityBias * 0.35);
        tension_ = clampUnit(tension_ + event.tensionDelta
            + seededEvent.tensionBias * 0.35);
        memory_ = clampUnit(memory_ + event.memoryDelta
            + seededEvent.memoryBias * 0.30);
        for (std::size_t index = 0U; index < relationCount_; ++index) {
            auto& relation = relations_[index];
            if (relation.sourceActorId == event.sourceActorId
                && relation.destinationActorId == event.targetActorId) {
                relation.intensity = clampUnit(relation.intensity + event.relationDelta);
                relation.intensityOverridden = false;
                break;
            }
        }
        // The turn must reach an existing musical body immediately, not only
        // a slow global pressure or an invisible return. It changes that
        // body's local seed colour; the performer drawing remains intact and
        // the next phrase can then carry the consequence further.
        for (std::size_t index = 0U; index < voiceCount_; ++index) {
            if (voices_[index].actorId != event.targetActorId) continue;
            voices_[index].seedVariationAmount = clampUnit(
                voices_[index].seedVariationAmount * 0.58
                + std::fabs(event.spatialInfluence) * 0.30
                + std::fabs(event.relationDelta) * 0.18
                + std::fabs(seededEvent.matterBias) * 0.12);
            break;
        }
        updatePressures();
        matterPressure_ = clampUnit(matterPressure_ + seededEvent.matterBias * 0.18);
        sky_.setSeedEvent(seededEvent);
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
        // A situated narrative turn can recall Modal matter once. This is an
        // event response to the actual field (not an audio-rate or periodic
        // feeder), and the resulting sound remains owned by a live stem.
        const auto modalOwner = modalLayerOwnerIndex();
        configureModalMatter(modalOwner < voiceCount_ ? &voices_[modalOwner] : nullptr);
        if (modalPrepared_)
            modalMatter_.excite(clampUnit(0.14 + std::fabs(event.spatialInfluence) * 0.32
                + std::fabs(event.relationDelta) * 0.20 + memoryPressure_ * 0.18));
#endif
        return event;
    }

    PerformanceSpatialGameEvent lastSpatialGameEvent() const noexcept {
        return lastSpatialGameEvent_;
    }

    const PerformanceGamePlan& gamePlan() const noexcept {
        return spatialGame_.gamePlan();
    }

    std::size_t relationCount() const noexcept { return relationCount_; }

    double activeRelationMass() const noexcept {
        double mass = 0.0;
        for (std::size_t index = 0U; index < relationCount_; ++index)
            mass += relations_[index].intensity;
        return mass;
    }

    // The next wait belongs to the seed trajectory, not to a GUI timer. It is
    // read-only and bounded so the engine can translate it to sample time.
    std::uint8_t nextNarrativePauseBeats() const noexcept {
        return currentSeedEvent(spatialGameTurn_).pauseBeats;
    }

    // The seed chooses a closing verb from the constellation that actually
    // exists: its live cables, focus and accumulated narrative turn. It never
    // changes authored positions, only the musical behaviour of their bodies.
    bool beginCoda() noexcept {
        if (voiceCount_ == 0U) return false;
        codaActive_ = true;
        codaProgress_ = 0.0;
        codaLeaderIndex_ = (static_cast<std::size_t>(seed_)
                            + static_cast<std::size_t>(spatialGameTurn_)
                            + relationCount_) % voiceCount_;
        const auto choice = (seed_ + spatialGameTurn_ * 3U
                             + static_cast<std::uint32_t>(relationCount_ * 5U)) % 4U;
        codaGesture_ = static_cast<CodaGesture>(choice);
        return true;
    }

    CodaGesture codaGesture() const noexcept { return codaGesture_; }

    bool setSeedCollage(std::uint32_t sourceSeed, double amount) noexcept {
        if (!finite(amount) || amount < 0.0 || amount > 1.0) return false;
        collageTrajectory_.configure(sourceSeed);
        ++seedCacheVersion_;
        collageAmount_ = amount;
        sky_.setSeedEvent(currentSeedEvent(spatialGameTurn_));
        return true;
    }

    // A local seed variation is a situated musical force. It reads the same
    // secondary trajectory as the collective Seed Viva. Actors 5..8 are the
    // four internal module lanes folded into the production stems: they keep
    // their own variation amount without becoming voices, stems or outputs.
    bool setActorSeedVariation(std::uint16_t actorId, double amount) noexcept {
        if (!unit(amount)) return false;
        if (isInternalModuleVariationActor(actorId)) {
            internalModuleSeedVariation_[actorId - 5U] = amount;
            return true;
        }
        for (std::size_t index = 0U; index < voiceCount_; ++index) {
            if (voices_[index].actorId != actorId) continue;
            voices_[index].seedVariationAmount = amount;
            return true;
        }
        return false;
    }

    bool supportsActorSeedVariation(std::uint16_t actorId) const noexcept {
        return isInternalModuleVariationActor(actorId) || presence(actorId) >= 0.0;
    }

    bool supportsActorPosition(std::uint16_t actorId) const noexcept {
        return isInternalModuleVariationActor(actorId) || presence(actorId) >= 0.0;
    }

    bool setMotionField(PerformanceMotionField field) noexcept {
        if (!validPerformanceMotionField(field)) return false;
        motionField_ = field;
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            voices_[index].motionField = field;
        return true;
    }

    bool setPopulationPhenomenon(PerformancePopulationPhenomenon phenomenon,
                                 double intensity) noexcept {
        if (!validPopulationPhenomenon(phenomenon) || !unit(intensity)) return false;
        populationPhenomenon_ = phenomenon;
        populationPhenomenonIntensity_ = intensity;
        populationPhenomenonEnabled_ = intensity > 0.0;
        updatePressures();
        return true;
    }

    PerformancePopulationPhenomenon populationPhenomenon() const noexcept {
        return populationPhenomenon_;
    }

    double populationPhenomenonIntensity() const noexcept {
        return populationPhenomenonEnabled_ ? populationPhenomenonIntensity_ : 0.0;
    }

    bool setActorMotionField(std::uint16_t actorId, PerformanceMotionField field) noexcept {
        if (!validPerformanceMotionField(field)) return false;
        for (std::size_t index = 0U; index < voiceCount_; ++index) {
            if (voices_[index].actorId != actorId) continue;
            voices_[index].motionField = field;
            return true;
        }
        return false;
    }

    // Trocar a estrela de um corpo (30/10, gestão dos módulos): o performer
    // põe no corpo uma estrela que o céu sabe tocar — inclusive as da
    // biblioteca, como a Tuba, que a seed não escolhe. Count desfaz (volta à
    // estrela da seed). A troca segue o ator quando as vozes são refeitas.
    bool setActorStar(std::uint16_t actorId, PerformanceModuleBody star) noexcept {
        if (actorId == kInvalidActorId) return false;
        // O Modal também entra (30/10): soa pelo seu adapter, num corpo só.
        if (star != PerformanceModuleBody::Count && star != PerformanceModuleBody::ModalResonator
            && !PerformanceSky::isStar(star)) return false;
        std::size_t slot = starSwaps_.size();
        for (std::size_t index = 0U; index < starSwaps_.size(); ++index) {
            if (starSwaps_[index].actorId == actorId) { slot = index; break; }
            if (slot == starSwaps_.size() && starSwaps_[index].actorId == kInvalidActorId) slot = index;
        }
        if (slot == starSwaps_.size()) return false;
        starSwaps_[slot] = {star == PerformanceModuleBody::Count ? kInvalidActorId : actorId, star};
        for (std::size_t index = 0U; index < voiceCount_; ++index) {
            if (voices_[index].actorId != actorId) continue;
            voices_[index].star = star == PerformanceModuleBody::Count ? starForSlot(index) : star;
            return true;
        }
        return true;
    }
    // Fixar a estrela de um corpo (2/11): a VARIAÇÃO e os eclipses não a
    // apagam. Segue o ator quando as vozes são refeitas.
    bool setActorPinned(std::uint16_t actorId, bool pinned) noexcept {
        if (actorId == kInvalidActorId) return false;
        std::size_t free = pinnedActors_.size();
        for (std::size_t index = 0U; index < pinnedActors_.size(); ++index) {
            if (pinnedActors_[index] == actorId) {
                if (!pinned) pinnedActors_[index] = kInvalidActorId;
                return true;
            }
            if (free == pinnedActors_.size() && pinnedActors_[index] == kInvalidActorId) free = index;
        }
        if (!pinned) return true;
        if (free == pinnedActors_.size()) return false;
        pinnedActors_[free] = actorId;
        return true;
    }
    bool actorPinned(std::uint16_t actorId) const noexcept {
        return actorId != kInvalidActorId
            && std::find(pinnedActors_.begin(), pinnedActors_.end(), actorId) != pinnedActors_.end();
    }
    void setFormPlan(const std::array<std::uint32_t, 4>& bars) noexcept { director_.setFormPlan(bars); }
    bool formPlanned() const noexcept { return director_.formPlanned(); }
    // O giro e a viagem da constelação inteira (30/10).
    bool setConstellationMotion(PerformanceConstellationMotion motion) noexcept {
        if (static_cast<std::uint8_t>(motion) >= static_cast<std::uint8_t>(PerformanceConstellationMotion::Count))
            return false;
        director_.setConstellationMotion(motion);
        return true;
    }

    // A família de movimento (30/10): kInvalidActorId = o campo inteiro;
    // senão, só aquela estrela. `Count` desfaz a escolha.
    bool setMotionFamily(std::uint16_t actorId, PerformanceMotionFamily family) noexcept {
        if (static_cast<std::uint8_t>(family) > static_cast<std::uint8_t>(PerformanceMotionFamily::Count)) return false;
        if (actorId == kInvalidActorId) {
            collectiveFamily_ = family;
            return true;
        }
        for (std::size_t index = 0U; index < voiceCount_; ++index) {
            if (voices_[index].actorId != actorId) continue;
            voices_[index].family = family;
            return true;
        }
        return false;
    }

    bool setActorMotionDirection(std::uint16_t actorId, bool forward) noexcept {
        for (std::size_t index = 0U; index < voiceCount_; ++index) {
            if (voices_[index].actorId != actorId) continue;
            voices_[index].motionForward = forward;
            return true;
        }
        return false;
    }

    // Recuts are temporary transformations of prepared stems. They never
    // construct bodies, write coordinates or replace the session tempo.
    bool triggerRecut(GestureType type, std::uint16_t targetActorId, double intensity,
                      double duration, std::uint32_t variationToken) noexcept {
        if ((type != GestureType::Stutter && type != GestureType::TempoShift)
            || !unit(intensity) || !unit(duration) || !finite(sampleRate_) || sampleRate_ <= 0.0)
            return false;
        if (targetActorId != kInvalidActorId && presence(targetActorId) < 0.0) return false;
        const double seconds = 0.12 + (duration > 0.0 ? duration : intensity) * 0.68;
        const auto boundedSamples = static_cast<std::uint32_t>(std::clamp(
            std::llround(sampleRate_ * seconds), 1LL,
            static_cast<long long>(sampleRate_ * 0.80)));
        if (type == GestureType::Stutter) {
            stutterIntensity_ = std::max(stutterIntensity_, intensity);
            stutterRemainingSamples_ = std::max(stutterRemainingSamples_, boundedSamples);
            stutterTargetActorId_ = targetActorId;
            return true;
        }
        tempoShiftIntensity_ = std::max(tempoShiftIntensity_, intensity);
        tempoShiftRemainingSamples_ = std::max(tempoShiftRemainingSamples_, boundedSamples);
        tempoShiftTargetActorId_ = targetActorId;
        const auto polarity = variationToken != 0U ? variationToken
            : static_cast<std::uint32_t>(spatialGameTurn_ ^ targetActorId);
        tempoShiftRatio_ = (polarity & 1U) == 0U
            ? 1.0 + intensity * 0.62 : 1.0 - intensity * 0.38;
        return true;
    }

    void setMacroState(const PerformanceSessionState& session) noexcept {
        autonomy_ = session.autonomy;
        intervention_ = session.intervention;
        density_ = session.density;
        tension_ = session.tension;
        memory_ = session.memory;
        risk_ = session.risk;
        space_ = session.space;
        depth_ = session.depth;
        tempoBpm_ = session.tempoBpm;
        fractureParameters_.pressure = clampUnit(0.45 * risk_ + 0.35 * tension_
            + 0.20 * frictionPressure_);
        fractureParameters_.repair = 0.0;
        if (fracturePrepared_) fracture_.setParameters(fractureParameters_);
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
        configureModalMatter();
#endif
    }

    void setGestureGrid(bool enabled, GestureGridDivision division) noexcept {
        if (!validGestureGridDivision(division)) return;
        gridEnabled_ = enabled;
        gridDivision_ = division;
    }

    void setHolding(bool holding) noexcept { holding_ = holding; }

    void resetFracture() noexcept {
        if (!fracturePrepared_) return;
        fracture_.reset();
        lastFractureTear_ = 0.0;
        tornActorId_ = kInvalidActorId;
        lastFractureBreaks_ = 0;
        fracture_.setParameters(fractureParameters_);
    }

    void clearOverrides() noexcept {
        for (std::size_t index = 0; index < voiceCount_; ++index) {
            voices_[index].presenceOverridden = false;
            voices_[index].lifeOverridden = false;
        }
        for (std::size_t index = 0; index < relationCount_; ++index)
            relations_[index].intensityOverridden = false;
    }

    // A territory is a new performance session, not a scene recall. Scene
    // changes preserve the live desk; territory changes start with a neutral
    // mixer and the declared module defaults so host and DSP share one policy.
    void resetLiveControls() noexcept {
        soloActorId_ = kInvalidActorId;
        sky_.resetControls();
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
        modalControls_ = {{0.48, 0.50, 0.42}};
        configureModalMatter();
#endif
        for (std::size_t index = 0; index < voiceCount_; ++index) {
            auto& voice = voices_[index];
            voice.stemGain = 1.0;
            voice.targetStemGain = 1.0;
            voice.stemMuted = false;
            voice.seedVariationAmount = 0.5;
            voice.presenceOverridden = false;
            voice.lifeOverridden = false;
        }
        internalModuleSeedVariation_.fill(0.5);
        for (auto& controls : internalModuleControls_) controls.fill(0.5);
        for (std::size_t index = 0; index < relationCount_; ++index)
            relations_[index].intensityOverridden = false;
    }

    bool setPresence(std::uint16_t actorId, double presence) noexcept {
        if (!unit(presence)) return false;
        for (std::size_t index = 0; index < voiceCount_; ++index) {
            auto& voice = voices_[index];
            if (voice.actorId != actorId) continue;
            // Subir o fader acende a estrela e a mantém acesa por um tempo,
            // mesmo com a variação do mixer ligada.
            if (presence > voice.presence) director_.holdLit(index);
            voice.presence = presence;
            voice.presenceOverridden = true;
            voice.life = presence > 0.025 ? ActorLifeState::Active : ActorLifeState::Dormant;
            voice.lifeOverridden = true;
            voice.targetGain = targetGainFor(voice);
            updatePressures();
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
            configureModalMatter();
#endif
            return true;
        }
        return false;
    }

    // Module-local modulation stays distinct from the global session macros.
    // Actor ids are resolved to the already-prepared fixed cohort; activating
    // or modulating a body never constructs DSP on the audio thread.
    bool setModuleControl(std::uint16_t actorId, PerformanceModuleControl control,
                          double value) noexcept {
        if (static_cast<std::uint8_t>(control)
                >= static_cast<std::uint8_t>(PerformanceModuleControl::Count)
            || !unit(value)) return false;
        if (isInternalModuleVariationActor(actorId)) {
            internalModuleControls_[actorId - 5U][static_cast<std::size_t>(control)] = value;
            return true;
        }
        for (std::size_t index = 0; index < voiceCount_; ++index) {
            if (voices_[index].actorId != actorId) continue;
            // Each visible module owns its three local controls. The first
            // four are additionally delegated to a native cohort body; the
            // expanded bodies keep their controls in the fixed voice state
            // instead of silently rejecting them beyond adapter slot four.
            voices_[index].localControls[static_cast<std::size_t>(control)] = value;
            if (actorId >= 5U) {
                // A local intervention is itself a bounded musical event.
                // Re-open the phrase briefly so a control change cannot be
                // hidden behind a resting expanded voice.
                voices_[index].phraseActive = true;
                voices_[index].phraseDuty = clampUnit(
                    0.28 + voices_[index].localControls[2] * 0.64);
                voices_[index].phraseEnvelope = std::max(
                    voices_[index].phraseEnvelope, 0.86);
                if (control == PerformanceModuleControl::Primary)
                    voices_[index].gain = clampUnit(0.22 + value * 0.72);
            }
            sky_.setControl(voices_[index].star, control, value);
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
            if (voices_[index].star == PerformanceModuleBody::ModalResonator && index == modalOwner()) {
                modalControls_ = voices_[index].localControls;
                configureModalMatter(&voices_[index]);
                // MODAL is event matter, not a permanently fed resonance. A
                // deliberate control movement creates one bounded reply.
                if (modalPrepared_)
                    modalMatter_.excite(clampUnit(0.18 + value * 0.52
                        + density_ * 0.16 + matterPressure_ * 0.14));
            }
#endif
            return true;
        }
        return false;
    }

    bool supportsModuleControl(std::uint16_t actorId) const noexcept {
        return isInternalModuleVariationActor(actorId) || presence(actorId) >= 0.0;
    }

    bool setRelationIntensity(std::uint16_t sourceActorId, std::uint16_t destinationActorId,
                              double intensity) noexcept {
        if (!unit(intensity)) return false;
        for (std::size_t index = 0; index < relationCount_; ++index) {
            auto& relation = relations_[index];
            if (relation.sourceActorId != sourceActorId || relation.destinationActorId != destinationActorId)
                continue;
            relation.intensity = intensity;
            relation.intensityOverridden = true;
            updatePressures();
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
            configureModalMatter();
#endif
            return true;
        }
        return false;
    }

    bool setRelationPhenomenon(std::uint16_t sourceActorId,
                               std::uint16_t destinationActorId,
                               PerformanceRelationalPhenomenon phenomenon,
                               double resonanceRatio = 1.0) noexcept {
        if (!validRelationalPhenomenon(phenomenon) || !finite(resonanceRatio)
            || resonanceRatio < 0.25 || resonanceRatio > 4.0) return false;
        for (std::size_t index = 0U; index < relationCount_; ++index) {
            auto& relation = relations_[index];
            if (relation.sourceActorId != sourceActorId
                || relation.destinationActorId != destinationActorId) continue;
            relation.phenomenon = phenomenon;
            relation.resonanceRatio = resonanceRatio;
            relation.phenomenonEnabled = true;
            relation.observationReady = false;
            updatePressures();
            return true;
        }
        return false;
    }

    PerformanceRelationalPhenomenon relationPhenomenon(
        std::uint16_t sourceActorId, std::uint16_t destinationActorId) const noexcept {
        for (std::size_t index = 0U; index < relationCount_; ++index)
            if (relations_[index].sourceActorId == sourceActorId
                && relations_[index].destinationActorId == destinationActorId)
                return relations_[index].phenomenon;
        return PerformanceRelationalPhenomenon::Attract;
    }

    // Spatial position is a bounded control value. It is stored with the
    // voice and folded into relation pressure; no GUI or allocation touches
    // the audio callback.
    bool setActorPosition(std::uint16_t actorId, double x, double y) noexcept {
        if (!unit(x) || !unit(y)) return false;
        if (isInternalModuleVariationActor(actorId)) {
            internalModulePositions_[actorId - 5U] = {{x, y}};
            return true;
        }
        for (std::size_t index = 0; index < voiceCount_; ++index) {
            if (voices_[index].actorId != actorId) continue;
            voices_[index].x = x;
            voices_[index].y = y;
            voices_[index].skyAnchorX = x;
            voices_[index].skyAnchorY = y;
            voices_[index].skyAnchorSet = true;
            updatePressures();
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
            configureModalMatter();
#endif
            return true;
        }
        return false;
    }

    // O lugar desenhado na tela, só para a física do céu (29/10): a âncora
    // de cada corpo. Não muda a posição da ecologia — com a trama desligada
    // o som é o mesmo. Corrige as estrelas 5–8 nunca arrastadas, que no motor
    // nasciam empilhadas em (0,50; 0,78) enquanto a tela as desenha
    // separadas (docs/MOVIMENTO_FISICO.md §6, opção 2).
    bool setSkyAnchor(std::uint16_t actorId, double x, double y) noexcept {
        if (!unit(x) || !unit(y)) return false;
        for (std::size_t index = 0; index < voiceCount_; ++index) {
            if (voices_[index].actorId != actorId) continue;
            voices_[index].skyAnchorX = x;
            voices_[index].skyAnchorY = y;
            voices_[index].skyAnchorSet = true;
            return true;
        }
        return false;
    }

    double presence(std::uint16_t actorId) const noexcept {
        for (std::size_t index = 0; index < voiceCount_; ++index)
            if (voices_[index].actorId == actorId) return voices_[index].presence;
        return -1.0;
    }

    double relationIntensity(std::uint16_t sourceActorId, std::uint16_t destinationActorId) const noexcept {
        for (std::size_t index = 0; index < relationCount_; ++index)
            if (relations_[index].sourceActorId == sourceActorId
                && relations_[index].destinationActorId == destinationActorId)
                return relations_[index].intensity;
        return -1.0;
    }

    bool setStemGain(std::uint16_t actorId, double gain) noexcept {
        if (!unit(gain)) return false;
        for (std::size_t index = 0; index < voiceCount_; ++index) {
            auto& voice = voices_[index];
            if (voice.actorId != actorId) continue;
            voice.targetStemGain = gain;
            return true;
        }
        return false;
    }

    bool setStemMuted(std::uint16_t actorId, bool muted) noexcept {
        for (std::size_t index = 0; index < voiceCount_; ++index) {
            auto& voice = voices_[index];
            if (voice.actorId != actorId) continue;
            voice.stemMuted = muted;
            return true;
        }
        return false;
    }

    bool setStemSolo(std::uint16_t actorId, bool soloed) noexcept {
        bool knownActor = false;
        for (std::size_t index = 0; index < voiceCount_; ++index)
            knownActor = knownActor || voices_[index].actorId == actorId;
        if (!knownActor) return false;
        if (soloed) soloActorId_ = actorId;
        else if (soloActorId_ == actorId) soloActorId_ = kInvalidActorId;
        return true;
    }

    double stemGain(std::uint16_t actorId) const noexcept {
        for (std::size_t index = 0; index < voiceCount_; ++index)
            if (voices_[index].actorId == actorId) return voices_[index].targetStemGain;
        return -1.0;
    }

    bool stemMuted(std::uint16_t actorId) const noexcept {
        for (std::size_t index = 0; index < voiceCount_; ++index)
            if (voices_[index].actorId == actorId) return voices_[index].stemMuted;
        return false;
    }

    std::uint16_t soloActorId() const noexcept { return soloActorId_; }
    double spacePressure() const noexcept { return spacePressure_; }

    PerformanceSpatialPoint actorPosition(std::uint16_t actorId) const noexcept {
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            if (voices_[index].actorId == actorId) return {voices_[index].x, voices_[index].y};
        return {};
    }

    PerformanceMotionPoint actorLivePosition(std::uint16_t actorId) const noexcept {
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            if (voices_[index].actorId == actorId) return livePosition(index);
        return {};
    }

    double motionPhase() const noexcept { return motionPhase_; }
    void setSkyInfluence(double influence) noexcept { director_.setInfluence(influence); }
    // O empurrão do ABRIR na estrela `actorId` (ou em todas, sem alvo).
    void pushStar(std::uint16_t actorId, std::uint8_t grain, double intensity) noexcept {
        std::size_t slot = PerformanceSkyDirector::kBodies;
        if (actorId != kInvalidActorId) {
            for (std::size_t index = 0U; index < voiceCount_; ++index)
                if (voices_[index].actorId == actorId) slot = index;
            if (slot >= PerformanceSkyDirector::kBodies) return;
        }
        director_.push(slot, grain, intensity);
    }
    // Identidade e vida de cada corpo, para a telemetria quando o céu está
    // parado e não renderiza.
    std::uint16_t actorIdAt(std::size_t index) const noexcept {
        return index < voiceCount_ ? voices_[index].actorId : kInvalidActorId;
    }
    ActorLifeState lifeAt(std::size_t index) const noexcept {
        return index < voiceCount_ ? voices_[index].life : ActorLifeState::Dormant;
    }
    void setMixerVariation(bool enabled) noexcept { director_.setMixerVariation(enabled); }
    void setStarCapacity(std::size_t capacity) noexcept { director_.setStarCapacity(capacity); }
    bool mixerVariation() const noexcept { return director_.mixerVariation(); }
    double starBrightness(std::size_t index) const noexcept { return director_.brightness(index); }
    const PerformanceSkyDirector& skyDirector() const noexcept { return director_; }
    // O lugar x do stem `index` na Constelação, pela física do céu (pan pela
    // física, 3/11/2026). Falso sem a trama: o estéreo segue o de antes.
    bool physicsPanX(std::size_t index, double& x) const noexcept {
        if (index >= voiceCount_ || index >= PerformanceSkyDirector::kBodies
            || !director_.positionsReady())
            return false;
        x = director_.shownX(index);
        return true;
    }
    // O nível da fratura (cicatriz + impulso, 0–1) e o corpo rasgado agora.
    double fractureLevel() const noexcept { return fractureLevel_; }
    // Quantos cortes o rasgo já abriu (para testes e diagnóstico).
    std::uint32_t tearCuts() const noexcept { return tearCuts_; }
    std::uint16_t tornActorId() const noexcept { return tornActorId_; }
    // O corpo que toca o Modal: o primeiro cuja estrela é o Modal (só há um
    // Modal — um ressonador, um stem). voiceCount_ quando nenhum.
    std::size_t modalOwner() const noexcept {
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            if (voices_[index].star == PerformanceModuleBody::ModalResonator) return index;
        return voiceCount_;
    }
    std::uint16_t skyGroupingMask() const noexcept { return sky_.groupingMask(); }
    // O módulo do corpo que deriva (255: nenhum).
    std::uint8_t skyDriftBody() const noexcept { return bodyAt(director_.driftSlot()); }
    // O módulo encoberto por um eclipse (255: nenhum).
    std::uint8_t skyEclipseBody() const noexcept { return bodyAt(director_.eclipsedSlot()); }
    std::uint8_t bodyAt(std::size_t slot) const noexcept {
        return slot < std::min(voiceCount_, voices_.size()) ? static_cast<std::uint8_t>(voices_[slot].star) : 255U;
    }
    PerformanceModuleBody actorStar(std::uint16_t actorId) const noexcept {
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            if (voices_[index].actorId == actorId) return voices_[index].star;
        return PerformanceModuleBody::Count;
    }
    // Diagnostic of the bounded internal orchestration layer. This is a
    // coefficient, not a new field position or an implicit connection.
    double internalLayerMix(std::uint16_t actorId) const noexcept {
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            if (voices_[index].actorId == actorId) return internalLayerMix_[index];
        return 0.0;
    }
    PerformanceMotionField motionField() const noexcept { return motionField_; }
    PerformanceMotionField motionField(std::uint16_t actorId) const noexcept {
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            if (voices_[index].actorId == actorId) return voices_[index].motionField;
        return PerformanceMotionField::Orbit;
    }
    bool motionForward(std::uint16_t actorId) const noexcept {
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            if (voices_[index].actorId == actorId) return voices_[index].motionForward;
        return true;
    }

    PerformanceStemFrame renderStems(double sampleRate, double mutate, double tear,
                                     double repair = 0.0, double hold = 0.0,
                                     std::uint16_t mutateTargetActorId = kInvalidActorId,
                                     std::uint16_t tearTargetActorId = kInvalidActorId,
                                     std::uint16_t repairTargetActorId = kInvalidActorId) noexcept {
        PerformanceStemFrame frame;
        if (!finite(sampleRate) || sampleRate <= 0.0) return frame;
        const auto activeVoiceCount = std::min(voiceCount_, kVoiceCapacity);
        frame.actorCount = activeVoiceCount;
        std::array<double, kVoiceCapacity> cohortFrequencies{{110.0, 165.0, 55.0, 110.0, 110.0}};
        for (std::size_t index = 0; index < std::min(activeVoiceCount, cohortFrequencies.size()); ++index) {
            cohortFrequencies[index] = std::max(20.0, voices_[index].frequency);
            const bool shifted = tempoShiftRemainingSamples_ > 0U
                && (tempoShiftTargetActorId_ == kInvalidActorId
                    || tempoShiftTargetActorId_ == voices_[index].actorId);
            if (shifted)
                cohortFrequencies[index] *= tempoShiftRatio_;
        }
        const double backgroundMutate = mutateTargetActorId == kInvalidActorId ? mutate : mutate * 0.14;
        const double backgroundTear = tearTargetActorId == kInvalidActorId ? tear : tear * 0.18;
        const double backgroundRepair = repairTargetActorId == kInvalidActorId ? repair : repair * 0.20;
        fractureParameters_.pressure = clampUnit(0.42 * risk_ + 0.34 * tension_
            + 0.24 * frictionPressure_);
        fractureParameters_.repair = clampUnit(repair);
        if (fracturePrepared_) {
            fracture_.setParameters(fractureParameters_);
            if (tear > lastFractureTear_ + 0.04)
                fracture_.applyStress(clampUnit(tear + tension_ * 0.28 + frictionPressure_ * 0.22));
            lastFractureTear_ = tear;
        }
        const double fractureSignal = fracturePrepared_ ? fracture_.process() : 0.0;
        fractureLevel_ = fractureSignal;
        // A fratura no céu (29/10): até aqui o nível da fratura só entrava na
        // camada provisória de senoides, que não soa desde que as estrelas
        // têm voz própria — a ruptura era calculada e jogada fora. Com a
        // trama, ela rasga o stem da estrela atingida (ver tearStem). O
        // alvo do último RASGAR fica marcado enquanto houver cicatriz.
        if (tearTargetActorId != kInvalidActorId && tear > 0.05) tornActorId_ = tearTargetActorId;
        if (fractureSignal < 0.004) tornActorId_ = kInvalidActorId;
        // Cada fibra nova que se rompe é um estalo: a estrela rasgada ataca.
        if (fracturePrepared_) {
            const auto breaks = fracture_.totalBreaks();
            if (breaks > lastFractureBreaks_ && director_.influence() > 0.0) {
                for (std::size_t index = 0U; index < voiceCount_; ++index)
                    if (fractureWeight(voices_[index]) >= 0.5 && PerformanceSky::isStar(voices_[index].star))
                        sky_.cue(voices_[index].star, 0.0, 1.0);
            }
            lastFractureBreaks_ = breaks;
        }
        // Densidade, tensão e risco são forças de composição, não três
        // controles de volume ocultos. Compensar a energia coletiva preserva
        // o MASTER como referência enquanto os três mudam quantidade de
        // eventos, atrito, registro e ruptura.
        const double macroEnergyTarget = 1.0 / (1.0 + density_ * 0.20
            + tension_ * 0.16 + risk_ * 0.18);
        macroEnergyCompensation_ += (macroEnergyTarget - macroEnergyCompensation_) * 0.0008;
        // Movimento de Campo deve respirar e deixar o desenho legível. O
        // relógio anterior (0,90–1,75 multiplicado pelo BPM) transformava os
        // astros em um tremor contínuo. Pulso ainda é o mais enérgico e
        // The field is a trajectory, not a screen LFO. At 120 BPM a complete
        // orbit now takes about 9 s; Figura-8 and Pulse remain successively
        // more active, but still leave time to hear a relation transform.
        // ESPAÇO is the performer's spatial-speed macro. It changes the rate
        // of the shared audio/visual phase, never the authored coordinates.
        const auto& composition = composition_;
        // Two performances with the same manual ESPAÇO can still inhabit
        // different spatial worlds. The seed's spatial affinity changes the
        // bounded rate and the amount of musical matter exposed by a movement;
        // it never writes over the performer's anchors or chosen motion mode.
        const double spatialSpeed = lerp(0.35, 1.25, space_)
            * (0.72 + composition.spatialAffinity * 0.56);
        const double motionRate = (motionField_ == PerformanceMotionField::Pulse ? 0.12
            : motionField_ == PerformanceMotionField::FigureEight ? 0.08 : 0.055) * spatialSpeed;
        motionPhase_ += (tempoBpm_ / 60.0) * motionRate / sampleRate;
        if (motionPhase_ >= 1.0) motionPhase_ -= std::floor(motionPhase_);
        // Relações espaciais não precisam ser reavaliadas para cada sample.
        // A taxa anterior multiplicava buscas, raízes e trigonometria por
        // 48 mil e provocava picos em sessões longas. A 750 Hz (48 kHz / 64)
        // a ligação continua mais rápida que qualquer gesto humano, mas o
        // callback deixa de pagar uma geometria completa por sample.
        if (pressureRefreshCountdown_ == 0U) {
            updatePressures();
            pressureRefreshCountdown_ = kPressureRefreshSamples;
        } else {
            --pressureRefreshCountdown_;
        }
        // Pitch trajectories are compositional control signals. Re-evaluating
        // their exponentials for every voice at audio rate was needlessly
        // expensive and could exhaust a short device callback. 1.5 kHz is
        // far above a performed parameter change while keeping the oscillator
        // itself sample-accurate and smooth.
        const bool refreshPitchTargets = pitchRefreshCountdown_ == 0U;
        if (refreshPitchTargets) {
            pitchRefreshCountdown_ = kPitchRefreshSamples;
        } else {
            --pitchRefreshCountdown_;
        }
        const double motionPhase = motionPhase_ * kTwoPi;
        const double motionWave = motionField_ == PerformanceMotionField::Pulse
            ? std::max(0.0, std::sin(motionPhase))
            : motionField_ == PerformanceMotionField::FigureEight
                ? std::sin(motionPhase) * std::sin(motionPhase * 2.0)
                : std::sin(motionPhase);
        const double motionColour = motionField_ == PerformanceMotionField::Pulse
            ? motionWave * 0.48
            : motionField_ == PerformanceMotionField::FigureEight
                ? motionWave * 0.38 : motionWave * 0.30;
        // O céu avança uma vez por amostra; cada corpo toca a sua estrela
        // abaixo, num stem só dele (PerformanceSky.hpp).
        // O tempo maleável (29/10): andamento elástico e swing orbital vêm da
        // trama; desligada, fator 1 e relógio reto (som idêntico).
        sky_.setTiming(director_.eccentricity(), director_.perihelion(), director_.groove());
        sky_.setCounting(director_.influence() > 0.0, director_.countingOrdinal());
        sky_.advance({tempoBpm_ * director_.tempoFactor(), density_, tension_, memory_, risk_, intervention_,
                      timePressure_, matterPressure_, frictionPressure_, backgroundMutate,
                      backgroundTear, backgroundRepair, autonomy_, holding_});
        // A trama do céu: o diretor move as estrelas pela gravitação, escolhe
        // as relações em foco e entrega os chamados vencidos às estrelas.
        // Um chamado também abre os portões de frase e pulso de quem responde,
        // para que a resposta seja ouvida.
        {
            // O estado dos corpos para o diretor muda devagar: refeito a cada
            // intervalo de controle (as posições vivas custam trigonometria).
            if (skyBodiesCountdown_ == 0U) {
                skyBodiesCountdown_ = PerformanceSkyDirector::kControlInterval;
                skyBodies_ = {};
                for (std::size_t index = 0U; index < std::min(activeVoiceCount, skyBodies_.size()); ++index) {
                    const auto& voice = voices_[index];
                    const auto live = livePosition(index);
                    skyBodies_[index] = {!isDarkAt(index) && PerformanceSky::isStar(voice.star), voice.star,
                                         voice.skyAnchorSet ? voice.skyAnchorX : voice.x,
                                         voice.skyAnchorSet ? voice.skyAnchorY : voice.y,
                                         voice.presence, live.x, live.y};
                    // O coro de fases: o ataque da estrela cai na fase do
                    // seu oscilador (0 com a trama desligada).
                    if (PerformanceSky::isStar(voice.star))
                        sky_.setPhaseDelay(voice.star, director_.chorusDelay(index));
                }
                // As famílias de movimento escolhidas na DANÇA (e as que vêm
                // dos campos de órbita antigos).
                director_.setCollectiveFamily(collectiveFamily_);
                for (std::size_t index = 0U; index < std::min(activeVoiceCount, skyBodies_.size()); ++index) {
                    director_.setBodyFamily(index, voices_[index].family);
                    // O limite explícito: sem ele o GCC 13 vê um índice 8
                    // possível depois de expandir actorPinned (falso alarme).
                    const auto pinnedId = index < voices_.size() ? voices_[index].actorId : kInvalidActorId;
                    director_.setPinned(index, actorPinned(pinnedId));
                    director_.setFieldFamily(index, voices_[index].motionField);
                }
                // Os cabos desenhados (as relações do território e os seus
                // fenômenos) entram na trama par a par.
                director_.clearCables();
                for (std::size_t relation = 0U; relation < relationCount_; ++relation) {
                    const auto& cable = relations_[relation];
                    if (cable.sourceIndex >= activeVoiceCount || cable.destinationIndex >= activeVoiceCount) continue;
                    director_.setCable(cable.sourceIndex, cable.destinationIndex, cable.intensity,
                                       cable.phenomenonEnabled ? static_cast<std::uint8_t>(cable.phenomenon)
                                                               : PerformanceSkyDirector::kNoPhenomenon);
                }
            }
            --skyBodiesCountdown_;
            director_.advance(skyBodies_, tempoBpm_);
            PerformanceSkyDirector::Cue due{};
            while (director_.popDue(due)) {
                if (due.slot >= activeVoiceCount) continue;
                auto& voice = voices_[due.slot];
                if (isDarkAt(due.slot) || !PerformanceSky::isStar(voice.star)) continue;
                sky_.cue(voice.star, due.pitchHz > 0.0 && !due.chromatic ? quantizeToSeedScale(due.pitchHz)
                                                                         : due.pitchHz,
                         due.strength);
                voice.phraseActive = true;
                voice.phraseEnvelope = std::max(voice.phraseEnvelope, 0.86);
                voice.pulseEnvelope = std::max(voice.pulseEnvelope, 0.90);
            }
        }
        if (codaActive_)
            codaProgress_ = clampUnit(codaProgress_ + 1.0 / std::max(sampleRate * 4.0, 1.0));
        if (!holding_) {
            orchestrationPhase_ += (0.018 + autonomy_ * 0.020 + density_ * 0.014) / sampleRate;
            if (orchestrationPhase_ >= 1.0)
                orchestrationPhase_ -= std::floor(orchestrationPhase_);
        }
        const auto orchestrationLeader = voiceCount_ > 0U
            ? (static_cast<std::size_t>(orchestrationPhase_ * static_cast<double>(voiceCount_))
               + static_cast<std::size_t>(spatialGameTurn_)) % voiceCount_
            : 0U;
        // The constellation is an orchestration score, not merely a drawing.
        // A cable that reaches the present focus selects a musical reply; the
        // remaining live cables form its local ensemble. This replaces the old
        // purely index-based "neighbour" rule, so moving, connecting or
        // separating bodies changes who answers whom in the audible ensemble.
        std::array<double, kVoiceCapacity> relationalReply{};
        std::array<double, kVoiceCapacity> relationalEnsemble{};
        if (voiceCount_ > 0U && soloActorId_ == kInvalidActorId) {
            const auto leaderId = voices_[orchestrationLeader].actorId;
            for (std::size_t relationIndex = 0U; relationIndex < relationCount_; ++relationIndex) {
                const auto& relation = relations_[relationIndex];
                // Índices dos dois corpos guardados quando a cena muda (antes
                // procurados por amostra, duas vezes por relação).
                const auto sourceIndex = relation.sourceIndex;
                const auto destinationIndex = relation.destinationIndex;
                const double sourcePresence = sourceIndex < voiceCount_ ? voices_[sourceIndex].presence : -1.0;
                const double destinationPresence = destinationIndex < voiceCount_
                    ? voices_[destinationIndex].presence : -1.0;
                const double membership = std::sqrt(std::max(0.0, sourcePresence)
                                                    * std::max(0.0, destinationPresence));
                const double strength = clampUnit(relation.activity * relation.intensity * membership);
                if (strength <= 0.001) continue;
                if (sourceIndex >= voiceCount_ || destinationIndex >= voiceCount_) continue;
                relationalEnsemble[sourceIndex] = std::max(relationalEnsemble[sourceIndex], strength);
                relationalEnsemble[destinationIndex] = std::max(relationalEnsemble[destinationIndex], strength);
                if (relation.sourceActorId == leaderId)
                    relationalReply[destinationIndex] = std::max(relationalReply[destinationIndex], strength);
                if (relation.destinationActorId == leaderId)
                    relationalReply[sourceIndex] = std::max(relationalReply[sourceIndex], strength);
            }
        }
        const double motionSine = std::sin(motionPhase);
        const double motionSineDouble = std::sin(motionPhase * 2.0);
        for (std::size_t index = 0; index < activeVoiceCount; ++index) {
            auto& voice = voices_[index];
            if (isDarkAt(index)) {
                // Estrela apagada: continua no céu sem gastar CPU (decisão do
                // autor, 2026-10-28). Nem a estrela nem a ecologia do corpo
                // rodam até a presença subir de novo.
                auto& dark = frame.actors[index];
                dark.actorId = voice.actorId;
                dark.role = voice.role;
                dark.life = voice.life;
                dark.mono = 0.0;
                continue;
            }
            const bool stuttering = stutterRemainingSamples_ > 0U
                && (stutterTargetActorId_ == kInvalidActorId
                    || stutterTargetActorId_ == voice.actorId);
            const bool tempoShifted = tempoShiftRemainingSamples_ > 0U
                && (tempoShiftTargetActorId_ == kInvalidActorId
                    || tempoShiftTargetActorId_ == voice.actorId);
            const double localTempoRatio = tempoShifted ? tempoShiftRatio_ : 1.0;
            const bool mutateTargetsVoice = mutateTargetActorId == kInvalidActorId
                || mutateTargetActorId == voice.actorId;
            const bool tearTargetsVoice = tearTargetActorId == kInvalidActorId
                || tearTargetActorId == voice.actorId;
            const bool repairTargetsVoice = repairTargetActorId == kInvalidActorId
                || repairTargetActorId == voice.actorId;
            const double localMutate = mutate * (mutateTargetsVoice ? 1.0 : 0.14);
            const double localTear = tear * (tearTargetsVoice ? 1.0 : 0.18);
            // A focused repair belongs to the selected body. Letting 20% leak
            // into every other stem made Recall sound more active elsewhere
            // than on its declared target.
            const double localRepair = repair * (repairTargetsVoice ? 1.0 : 0.0);
            // The local seed only changes on a narrative turn, a new
            // trajectory or a new variation amount; recomputing it for every
            // body at audio rate was the ecology's largest repeated cost.
            if (voice.localSeedTurn != spatialGameTurn_ || voice.localSeedVersion != seedCacheVersion_
                || voice.localSeedVariation != voice.seedVariationAmount) {
                voice.localSeed = PerformanceSeedTrajectory::collage(
                    seedTrajectory_.event(spatialGameTurn_), collageTrajectory_.event(spatialGameTurn_),
                    voice.seedVariationAmount);
                voice.localSeedTurn = spatialGameTurn_;
                voice.localSeedVersion = seedCacheVersion_;
                voice.localSeedVariation = voice.seedVariationAmount;
            }
            const auto& localSeed = voice.localSeed;
            const double localSeedColour = voice.seedVariationAmount
                * (localSeed.matterBias * 0.48 + localSeed.tensionBias * 0.32
                   + localSeed.densityBias * 0.20);
            // Cada corpo tem uma frase própria. O pulso comum ainda pode
            // aproximar os módulos, mas não pode reduzir uma constelação a
            // oito relógios idênticos. A cada limiar de frase, a seed decide
            // presença, duração e grau: assim há respiração real, chamadas,
            // respostas e pausas -- nunca uma nota alimentada sem fim.
            const double phraseRate = phraseRateFor(voice.actorId, index)
                * (0.68 + density_ * 0.46 + timePressure_ * 0.34
                   + localSeed.densityBias * 0.18);
            if (!holding_) {
                voice.phrasePhase += (tempoBpm_ / 60.0) * phraseRate
                    * localTempoRatio / sampleRate;
                if (voice.phrasePhase >= 1.0) {
                    voice.phrasePhase -= std::floor(voice.phrasePhase);
                    ++voice.phraseStep;
                    const auto word = phraseWord(seed_, voice.phraseStep
                        + spatialGameTurn_ * 17U, voice.actorId);
                    voice.phraseActive = phraseSpeaks(voice.actorId, word,
                        localSeed, relationalEnsemble[index]);
                    voice.phraseDuty = phraseDutyFor(voice.actorId, word,
                        localSeed, relationalReply[index]);
                    voice.phrasePitchSemitones = phrasePitchFor(voice.actorId, word,
                        localSeed, relationalReply[index]);
                }
            }
            const double phraseTarget = voice.phraseActive
                && voice.phrasePhase < voice.phraseDuty ? 1.0 : 0.0;
            voice.phraseEnvelope += (phraseTarget - voice.phraseEnvelope) * 0.0048;
            // The module's entry ramp must become audible within the first
            // musical gesture; 0.0017 left the real stem effectively silent
            // during the mixer/solo observation window.
            voice.gain += (voice.targetGain - voice.gain) * 0.008;
            // A fixed target would turn an unattended performance into a
            // stationary drone. The seed supplies a slow pitch contour while
            // the shared motion supplies a bounded vibrato; both are smoothed
            // so the change remains musical rather than a click or retune.
            const auto& localControl = voice.localControls;
            // Expanded modules 5–8 have a real local pitch/register lever.
            // It is intentionally bounded to one octave either side, so it
            // changes a musical trajectory instead of behaving as a level.
            // The expanded Field actors are indices 4..7 (actor ids 5..8).
            // Comparing against cohort.bodyCount was wrong once the adapter
            // began publishing all eight internal lanes: every expanded
            // actor then missed its register control entirely.
            if (refreshPitchTargets) {
                // Só quando o alvo é renovado (antes calculado a cada amostra).
                const double seedPitchContour = localSeed.matterBias * 0.62
                    + localSeed.tensionBias * 0.38
                    + std::sin(motionPhase * 0.37 + static_cast<double>(index) * 0.83)
                        * (0.12 + autonomy_ * 0.16);
                const double localRegister = index >= 4U
                    ? (localControl[0] - 0.5) * 24.0
                        + (localControl[2] - 0.5) * 12.0 : 0.0;
                voice.liveTargetFrequency = voice.targetFrequency
                    * std::pow(2.0, (seedPitchContour + voice.phrasePitchSemitones
                                      + localRegister) / 12.0);
            }
            const double codaRegister = !codaActive_ ? 1.0
                : codaGesture_ == CodaGesture::Gather ? 0.992
                : codaGesture_ == CodaGesture::Relay ? (index == codaLeaderIndex_ ? 1.006 : 0.986)
                : codaGesture_ == CodaGesture::Memory ? 0.997 : 1.012;
            voice.frequency += (voice.liveTargetFrequency * codaRegister - voice.frequency) * 0.0009;
            // These macros are played from a stage surface, not inspected in
            // a lab: each must leave a plainly audible consequence. Autonomy
            // changes the organism's internal drift, while Intervention
            // introduces a controllable cross-colour into every active stem.
            const double drift = 1.0 + (autonomy_ - 0.5) * 0.042
                + localMutate * (0.040 + timePressure_ * 0.070)
                + timePressure_ * (0.010 + static_cast<double>(index) * 0.006)
                + territoryIdentity_.temporalDna * (0.001 + static_cast<double>(index) * 0.002)
                + spacePressure_ * (0.002 + static_cast<double>(index) * 0.0015)
                + localSeed.densityBias * voice.seedVariationAmount * 0.046
                + motionColour * (0.010 + static_cast<double>(index) * 0.004)
                + static_cast<double>(index) * 0.0017;
            voice.phase += voice.frequency * drift * localTempoRatio / sampleRate;
            if (voice.phase >= 1.0) voice.phase -= std::floor(voice.phase);
            const double gridRate = !gridEnabled_ ? 1.0
                : gridDivision_ == GestureGridDivision::Quarter ? 1.24
                : gridDivision_ == GestureGridDivision::Half ? 1.12 : 1.0;
            const double pulseRate = pulseRateFor(voice.role, index) * gridRate
                * (1.0 - relationalPulseCoupling_ * 0.22
                   + relationalTemporalFracture_ * 0.38);
            if (!holding_) {
                voice.pulsePhase += (tempoBpm_ / 60.0) * pulseRate * localTempoRatio / sampleRate;
                if (voice.pulsePhase >= 1.0) voice.pulsePhase -= std::floor(voice.pulsePhase);
            }
            const double gridAccent = gridEnabled_ && gridDivision_ != GestureGridDivision::Beat
                ? std::max(0.0, std::sin(voice.pulsePhase * kTwoPi
                    * static_cast<double>(static_cast<std::uint8_t>(gridDivision_)))) * 0.08
                : 0.0;
            const double pulseTarget = voice.pulsePhase < pulseDutyFor(voice.role) + gridAccent
                ? 1.0 : 0.0;
            voice.pulseEnvelope += (pulseTarget - voice.pulseEnvelope) * 0.0065;
            // MOVIMENTO must be heard through the actual stem, not only as a
            // small colour in the fallback contour. The three fields keep
            // their own temporal character while this one-pole response avoids
            // a discontinuity when the performer changes field.
            const auto localMotionField = voice.motionField;
            const double localMotionPhase = (voice.motionForward ? 1.0 : -1.0) * motionPhase;
            // O seno é ímpar: o sentido inverso só troca o sinal; os dois
            // senos são calculados uma vez por amostra, fora do laço.
            const double localSine = voice.motionForward ? motionSine : -motionSine;
            const double localSineDouble = voice.motionForward ? motionSineDouble : -motionSineDouble;
            const double localMotionWave = localMotionField == PerformanceMotionField::Pulse
                ? std::max(0.0, localSine)
                : localMotionField == PerformanceMotionField::FigureEight
                    ? localSine * localSineDouble
                    : localSine;
            const double localMotionColour = localMotionField == PerformanceMotionField::Pulse
                ? localMotionWave * 0.72
                : localMotionField == PerformanceMotionField::FigureEight
                    ? localMotionWave * 0.58 : localMotionWave * 0.46;
            const double motionTarget = localMotionField == PerformanceMotionField::Pulse
                ? 0.48 + localMotionWave * 0.52
                : localMotionField == PerformanceMotionField::FigureEight
                    ? 0.72 + (localMotionWave + 1.0) * 0.14
                    : 0.86 + (localMotionWave + 1.0) * 0.07;
            voice.motionEnvelope += (motionTarget - voice.motionEnvelope) * 0.0065;
            const double stutterGate = stuttering
                && std::fmod(stutterPhase_ + static_cast<double>(index) * 0.173, 1.0)
                    > (0.18 + (1.0 - stutterIntensity_) * 0.34) ? 0.12 : 1.0;
            const double stutterTarget = stuttering ? stutterGate : 1.0;
            voice.stutterEnvelope += (stutterTarget - voice.stutterEnvelope) * 0.024;
            const double gridTexture = gridEnabled_
                ? std::sin(voice.pulsePhase * kTwoPi
                    * static_cast<double>(static_cast<std::uint8_t>(gridDivision_)))
                    * (gridDivision_ == GestureGridDivision::Quarter ? 0.18 : 0.10)
                : 0.0;
            const bool excludedBySolo = soloActorId_ != kInvalidActorId
                && voice.actorId != soloActorId_;
            const double mixTarget = voice.stemMuted || excludedBySolo ? 0.0 : voice.targetStemGain;
            voice.stemGain += (mixTarget - voice.stemGain) * 0.0032;
            auto& stem = frame.actors[index];
            stem.actorId = voice.actorId;
            stem.role = voice.role;
            stem.life = voice.life;
            // A matéria provisória em senoides só existe para corpos sem
            // estrela (o fallback). Calculá-la para toda estrela a cada
            // amostra custava ~8 senos e 2 potências por corpo, jogados fora.
            const auto provisionalBody = [&]() noexcept {
                const double base = std::sin(voice.phase * kTwoPi);
            const double harmonic = std::sin(voice.phase * kTwoPi
                * (2.0 + matterPressure_ * 4.8 + frictionPressure_ * 2.4
                   + tension_ * 0.7 + risk_ * 1.6 + localSeed.matterBias
                   * voice.seedVariationAmount * 1.8
                   + relationalMatterExchange_ * 1.6
                   + relationalMatterFusion_ * 2.8
                   + localMotionColour * (1.15 + static_cast<double>(index) * 0.12)));
            const double friction = harmonic * frictionPressure_ * 0.24;
            const double scar = std::sin(voice.phase * kTwoPi * 9.0)
                * (localTear + risk_ * 0.24) * 0.16;
            const double intervention = harmonic * intervention_
                * (0.11 + localMutate * 0.24 + static_cast<double>(index) * 0.025
                   + localSeed.tensionBias * voice.seedVariationAmount * 0.18
                   + relationalSeedDeviation_ * 0.20);
            const double movement = std::sin((voice.phase + localMotionPhase * (1.0 + index * 0.17)) * kTwoPi)
                * localMotionColour * (0.18 + static_cast<double>(index) * 0.03);
            const double residue = std::sin((voice.phase - memory_ * 0.18) * kTwoPi)
                * memory_ * (0.08 + memory_ * 0.22);
            const double fractureTexture = fractureSignal
                * (voice.role == PerformanceRole::Rasgo ? 0.32
                   : voice.role == PerformanceRole::Atrito ? 0.10 : 0.025)
                // Ruptura só existe como acontecimento articulado. Antes,
                // TENSÃO/RISCO mantinham uma camada longa por baixo dos
                // módulos, contrariando o caráter generativo do instrumento.
                * std::pow(clampUnit(voice.pulseEnvelope), 1.65)
                * std::pow(clampUnit(voice.phraseEnvelope), 1.20);
            const double repairedHarmonic = harmonic * (1.0 - clampUnit(localRepair) * 0.78
                - clampUnit(hold) * 0.38);
            const double relationalGate = 1.0 - relationalSilence_
                * (0.18 + 0.32 * std::max(0.0, std::sin(
                    motionPhase + static_cast<double>(index) * 1.31)));
            const double provisional = (waveform(voice.role, base, repairedHarmonic, scar)
                + friction + residue + intervention + movement + gridTexture + fractureTexture
                + std::sin(voice.phase * kTwoPi * (3.0 + localSeedColour * 5.0))
                    * localSeedColour * 0.16) * relationalGate;
                return provisional;
            };
            if (PerformanceSky::available() && PerformanceSky::isStar(voice.star)) {
                // The real audited module owns the audible identity. The
                // provisional sine ecology is intentionally excluded here;
                // mixing it underneath a real body was the source of an
                // unrelated continuous pitched layer.
                const double realBody = sky_.renderStar(voice.star, cohortFrequencies[index])
                    * (1.35 + localMutate * 0.32 + localTear * 0.16
                       + localSeedColour * 0.28);
                // RASGAR no stem focal precisa ferir a matéria, não apenas
                // subir seu nível. RECALL devolve uma componente estável da
                // mesma voz; ambos permanecem limitados por evento.
                // Fora do gesto, RASGAR e REPARAR têm peso zero: as contas deles
                // são puladas (o resultado é exatamente o mesmo número).
                const double tornBody = std::tanh(localTear != 0.0
                    ? realBody + std::sin(voice.phase * kTwoPi * (5.0 + tension_ * 8.0))
                        * localTear * (0.20 + risk_ * 0.18)
                    : realBody);
                // Recall acts on the new bodies too: it pulls their energetic
                // foreground back while the remembered provisional contour
                // briefly resurfaces. Hold slightly concentrates the body
                // while its event clocks are frozen.
                const double repairedBody = clampUnit(localRepair) != 0.0
                    ? lerp(tornBody, std::tanh(tornBody * 0.58 + std::sin(voice.phase * kTwoPi) * 0.22),
                           clampUnit(localRepair))
                    : tornBody;
                const double heldBody = tearStem(voice, repairedBody, fractureSignal, sampleRate)
                    * (1.0 + clampUnit(hold) * 0.12);
                const double movingBody = heldBody * (0.46 + voice.motionEnvelope * 0.54);
                // GRID must reach the native body itself. A provisional
                // side-layer can disappear inside a dense module; this
                // bounded accent makes each division performable.
                const double gridBodyAccent = gridTexture
                    * (gridDivision_ == GestureGridDivision::Quarter ? 0.62 : 0.42);
                // Native cohort bodies must breathe as events. A continuous
                // oscillator underneath the generative layer reads as a stuck
                // note; this bounded gate creates real openings. The smoothed
                // pulse envelope supplies the release, so no pitched residue
                // is left between events.
                // Envelope curves run for every body at audio rate. Polynomial
                // curves keep the same quiet attack/release character as the
                // former fractional powers without repeatedly invoking pow()
                // inside the device callback.
                const double pulseGate = curveTowardCube(voice.pulseEnvelope, 0.80);
                const double phraseGate = curveTowardSquare(voice.phraseEnvelope, 0.35);
                const double nativeEventGate = (soloActorId_ != kInvalidActorId ? 1.0 : pulseGate)
                    * phraseGate;
                stem.mono = (movingBody * 3.0 + gridBodyAccent)
                    * nativeEventGate * voice.gain * voice.stemGain;
            } else if (voice.star == PerformanceModuleBody::ModalResonator) {
                // O Modal soa pelo seu próprio adapter, logo abaixo; nunca
                // por uma imitação em senoide com o nome dele.
                stem.mono = 0.0;
            } else {
                // Slots beyond the native cohort must not collapse into the
                // same generic sine. Give each visible body a distinct,
                // bounded musical behaviour driven by slot, seed and pulse.
                const auto fallbackSlot = index + 1U;
                const double slot = static_cast<double>(fallbackSlot);
                const double seedBend = localSeedColour * (1.4 + slot * 0.35);
                const double ratio = index == 3U ? 1.50 + seedBend
                    : index == 4U ? 1.18 + localControl[1] * 2.46 + seedBend * 0.24
                    : index == 5U ? 0.42 + localControl[1] * 1.52 - seedBend * 0.18
                    : index == 6U ? 1.85 + localControl[1] * 3.10 + seedBend * 0.30
                    : 0.28 + localControl[1] * 1.06 + seedBend * 0.12;
                const double orbit = std::sin(voice.phase * kTwoPi * ratio
                    + localMotionPhase * (0.65 + slot * 0.21));
                const double collision = std::sin(voice.phase * kTwoPi
                    * (ratio + 1.0 + frictionPressure_ * 2.0)) * (0.22 + tension_ * 0.28);
                const double body = index == 3U ? orbit * 0.72 + collision
                    // 5 MODAL: modos cruzados e cauda curta; CORPO muda a
                    // razão, BRILHO o atrito e CAUDA a duração da frase.
                    : index == 4U ? std::sin(voice.phase * kTwoPi * ratio)
                                      * (0.48 + localControl[1] * 0.48)
                                    + std::sin(voice.phase * kTwoPi * (ratio * 1.618))
                                      * (0.18 + matterPressure_ * 0.22)
                    // 6 GALO: chamada em inarmônicos; REGISTRO muda a
                    // relação interna, CAUDA decide quanto a chamada persiste.
                    : index == 5U ? std::sin(voice.phase * kTwoPi * ratio)
                                      * std::sin(voice.phase * kTwoPi * (ratio * 1.37))
                                      * (0.92 + localControl[2] * 0.76)
                                    + collision * (0.16 + localControl[2] * 0.98)
                    // 7 ATROPELO: fala/rasgo com corpo assimétrico, nunca
                    // uma senoide contínua sob o rótulo de sílaba.
                    : index == 6U ? std::tanh(orbit * (0.88 + localControl[1] * 1.34)
                                      + collision * (0.28 + localControl[2] * 1.08)
                                      + std::sin(voice.phase * kTwoPi * ratio
                                          * (1.18 + localControl[2] * 1.72))
                                          * (0.14 + localControl[2] * 0.92))
                    // 8 DEJA VU: vestígio lento, com memória de fase e
                    // duração local; permanece articulado pelo gate abaixo.
                    : orbit * (0.32 + localControl[1] * 0.52)
                      + std::sin((voice.phase * (ratio * 0.53) - memory_ * 0.21) * kTwoPi)
                        * (0.18 + localControl[2] * 0.76);
                const double pulseGate = index == 5U
                    ? curveTowardSquare(voice.pulseEnvelope, 0.80)
                    : curveTowardSquare(voice.pulseEnvelope, 0.15);
                const double phraseCurve = index >= 4U
                    ? clampUnit(localControl[2] * 0.72 - 0.06)
                    : 0.20;
                const double eventGate = pulseGate
                    * curveTowardSquare(voice.phraseEnvelope, phraseCurve);
                // Expanded local controls must remain audible even when a
                // phrase is quiet. Secondary changes the internal colour;
                // Motion/CAUDA changes the event articulation. Both are
                // bounded and still pass through the phrase gate.
                const double localModuleArticulation = index >= 4U
                    ? (std::sin(voice.phase * kTwoPi * (1.0 + slot * 0.23))
                          * (localControl[0] - 0.5) * 1.80
                       + std::sin(voice.phase * kTwoPi * (2.0 + slot * 0.37))
                          * (localControl[1] - 0.5) * 0.84
                       + std::sin(voice.phase * kTwoPi * (3.0 + slot * 0.29))
                          * (localControl[2] - 0.5) * 0.72)
                    : 0.0;
                stem.mono = std::tanh(body + provisionalBody() * 0.12 + localModuleArticulation)
                    * voice.gain * voice.stemGain * eventGate;
            }
        }
        // A stem is an orchestral submix, not a prison for one module. The
        // native cohort has already rendered its bounded real bodies above;
        // this pass lets one related body enter a different stem as a layer.
        // It does not instantiate a second DSP voice, write the performer's
        // positions or make/change a cable. The layer must be invited by a
        // seed event and/or live relation, and it is gated by both phrases so
        // it cannot turn into a permanent carrier.
        internalLayerMix_ = {};
        if (director_.influence() > 0.0) {
            for (std::size_t index = 0U; index < activeVoiceCount; ++index) {
                auto& voice = voices_[index];
                if (isDarkAt(index) || !sky_.attacked(voice.star)) continue;
                director_.hear(index, sky_.attackPitch(voice.star));
                // A estrela é ouvida quando age: o evento do próprio módulo
                // abre os seus portões de frase e pulso. Antes uma estrela acesa
                // podia disparar e ficar muda por 9–10 s com os portões fechados.
                voice.phraseActive = true;
                voice.phraseEnvelope = std::max(voice.phraseEnvelope, 0.70);
                voice.pulseEnvelope = std::max(voice.pulseEnvelope, 0.85);
            }
        }
        stutterPhase_ += (tempoBpm_ / 60.0) * (3.0 + stutterIntensity_ * 13.0) / sampleRate;
        if (stutterPhase_ >= 1.0) stutterPhase_ -= std::floor(stutterPhase_);
        if (tempoShiftRemainingSamples_ > 0U && --tempoShiftRemainingSamples_ == 0U) {
            tempoShiftIntensity_ = 0.0;
            tempoShiftTargetActorId_ = kInvalidActorId;
            tempoShiftRatio_ = 1.0;
        }
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
        if (modalPrepared_ && modalLevel_ > 0.0) {
            const Voice* modalVoice = nullptr;
            std::size_t modalOwnerIndex = kVoiceCapacity;
            // O dono do Modal é o corpo cuja estrela é o Modal (30/10): por
            // padrão o corpo 5, mas o performer pode trocá-lo — antes o som
            // do Modal sobrescrevia o ator 5 qualquer que fosse a estrela.
            if (const auto owner = modalOwner(); owner < activeVoiceCount) {
                modalVoice = &voices_[owner];
                modalOwnerIndex = owner;
            }
            // Um módulo, um stem (2026-10-28): sem o corpo 5 no céu o Modal
            // não soa. Antes ele era dobrado dentro do stem de outro corpo,
            // e o MUTE desse corpo deixava de calar só o seu próprio som.
            // Actor 5 follows its own phrase, not a fixed territory
            // fundamental.  Parameter changes and excitation happen only at
            // phrase boundaries: a changing melodic line, never continuous
            // feeding of a resonance.
            if (modalVoice != nullptr && modalVoice->phraseStep != modalLastPhraseStep_) {
                configureModalMatter(modalVoice);
                if (modalVoice->phraseActive)
                    modalMatter_.excite(clampUnit(0.20 + modalVoice->presence * 0.28
                        + density_ * 0.18 + matterPressure_ * 0.16 + risk_ * 0.18));
                modalLastPhraseStep_ = modalVoice->phraseStep;
            }
            if (tear > lastTear_ + 0.08)
                modalMatter_.excite(clampUnit(0.24 + tear * 0.58 + risk_ * 0.34));
            lastTear_ = tear;
            // Only an explicitly released fifth Modal stem gets a sparse
            // autonomous clock. The internal layer is articulated by host
            // phrases and ruptures, so it cannot become a free-running drone.
            if (modalOwnerIndex < voiceCount_) {
                modalClock_ += (0.35 + autonomy_ * 1.4 + intervention_ * 0.6
                    + timePressure_ * 2.0 + mutate * 1.4) / sampleRate;
                if (modalClock_ >= 1.0) {
                    modalClock_ -= std::floor(modalClock_);
                    modalMatter_.excite(clampUnit(0.12 + density_ * 0.18
                        + matterPressure_ * 0.30 + risk_ * 0.34 + tear * 0.32));
                }
            }
            const double modalSample = static_cast<double>(modalMatter_.process()) * modalLevel_;
            bool claimedByFifthStem = false;
            for (std::size_t index = 0U; index < frame.actorCount; ++index) {
                if (index != modalOwnerIndex) continue;
                // The fifth body owns the same already-audited modal DSP. It
                // enters through the ordinary voice gain/mute/solo path, so
                // it is a real stem once capacity permits it, not a hidden
                // return mixed beside the desk.
                const auto& voice = voices_[index];
                const bool excludedBySolo = soloActorId_ != kInvalidActorId
                    && voice.actorId != soloActorId_;
                const double mix = voice.stemMuted || excludedBySolo ? 0.0 : voice.stemGain;
                // The modal tail remains audible after an attack, but it no
                // longer occupies the field as an unbroken carrier between
                // phrases.  CAUDA changes both its physical decay and the
                // audible residual level.
                const double modalPhraseGate = 0.12 + 0.88
                    * std::pow(clampUnit(voice.phraseEnvelope), 0.72);
                frame.actors[index].mono = modalSample * voice.gain * mix * modalPhraseGate;
                claimedByFifthStem = true;
                break;
            }
            (void) claimedByFifthStem;
        }
#endif
        // Post-process the final owned stems after modal replacement so every
        // actor receives the same orchestration and stutter stage.
        for (std::size_t index = 0U; index < frame.actorCount; ++index) {
            auto& voice = voices_[index];
            auto& stem = frame.actors[index];
            if (isDarkAt(index)) continue;
            // Orchestration is a moving hierarchy across the ensemble. One
            // body comes forward, neighbours answer and the others retain a
            // role-dependent floor instead of every stem speaking equally.
            double roleFloor = 0.34;
            switch (voice.role) {
            case PerformanceRole::Chao: roleFloor = 0.78; break;
            case PerformanceRole::Corpo: roleFloor = 0.62; break;
            case PerformanceRole::Atrito: roleFloor = 0.40; break;
            case PerformanceRole::Contagio: roleFloor = 0.52; break;
            case PerformanceRole::Rasgo: roleFloor = 0.34; break;
            case PerformanceRole::Vestigio: roleFloor = 0.58; break;
            case PerformanceRole::Count: break;
            }
            const auto distance = voiceCount_ > 0U
                ? std::min((index + voiceCount_ - orchestrationLeader) % voiceCount_,
                           (orchestrationLeader + voiceCount_ - index) % voiceCount_)
                : 0U;
            const auto& composition = composition_;
            const double replyTarget = std::max(roleFloor,
                0.56 + composition.collectiveAffinity * 0.12
                    + relationalReply[index] * 0.34 + memory_ * 0.04);
            const double ensembleTarget = std::max(roleFloor * (0.82 + memory_ * 0.10
                                                               + composition.collectiveAffinity * 0.12),
                0.40 + composition.collectiveAffinity * 0.10
                    + relationalEnsemble[index] * 0.28);
            const double legacyNeighbourTarget = distance == 1U
                ? std::max(0.56, roleFloor * 0.92) : roleFloor * (0.78 + memory_ * 0.22);
            const double orchestrationTarget = soloActorId_ != kInvalidActorId ? 1.0
                : distance == 0U ? 1.0
                : relationalReply[index] > 0.001 ? replyTarget
                : relationalEnsemble[index] > 0.001 ? ensembleTarget
                : legacyNeighbourTarget;
            voice.orchestrationGain += (orchestrationTarget - voice.orchestrationGain) * 0.0007;
            stem.mono *= voice.orchestrationGain;

            // EIXO · FENÔMENO is a performative orchestration operator, not a
            // decorative displacement. Each population movement therefore
            // owns a direct, recognisable articulation on the final stems.
            // The short smoothing protects the audio boundary from clicks.
            double populationTarget = 1.0;
            double populationDrive = 0.0;
            if (populationPhenomenonEnabled_) {
                const double amount = populationPhenomenonIntensity_;
                const double commonPhase = motionPhase_ * kTwoPi * 8.0;
                const double indexedPhase = commonPhase
                    + static_cast<double>(index) * kTwoPi
                        / static_cast<double>(std::max<std::size_t>(voiceCount_, 1U));
                const double commonWave = 0.5 + 0.5 * std::sin(commonPhase);
                const double indexedWave = 0.5 + 0.5 * std::sin(indexedPhase);
                switch (populationPhenomenon_) {
                case PerformancePopulationPhenomenon::Align:
                    populationTarget = commonWave > 0.34 ? 1.0 : 0.22; break;
                case PerformancePopulationPhenomenon::Cluster:
                    populationTarget = 0.78 + commonWave * 0.22;
                    populationDrive = amount * 1.6; break;
                case PerformancePopulationPhenomenon::Scatter:
                    populationTarget = indexedWave > 0.58 ? 1.0 : 0.08; break;
                case PerformancePopulationPhenomenon::CollectiveRotate:
                    populationTarget = 0.18 + indexedWave * 1.02; break;
                case PerformancePopulationPhenomenon::Expand:
                    populationTarget = 0.46 + commonWave * 0.82;
                    populationDrive = amount * 0.65; break;
                case PerformancePopulationPhenomenon::Contract:
                    populationTarget = 0.30 + (1.0 - commonWave) * 0.42; break;
                case PerformancePopulationPhenomenon::Pulse:
                    populationTarget = 0.10 + commonWave * commonWave * 1.05; break;
                case PerformancePopulationPhenomenon::Fragment: {
                    const auto cut = std::sin(voice.pulsePhase * kTwoPi
                        * static_cast<double>(3U + index % 3U));
                    populationTarget = cut > 0.18 ? 1.0 : 0.035; break;
                }
                case PerformancePopulationPhenomenon::MergeGroups: {
                    const auto pairPhase = commonPhase
                        + static_cast<double>(index / 2U) * 1.73;
                    populationTarget = 0.16 + (0.5 + 0.5 * std::sin(pairPhase)) * 0.94;
                    populationDrive = amount * 0.9; break;
                }
                case PerformancePopulationPhenomenon::Drift:
                    populationTarget = 0.34 + (0.5 + 0.5 * std::sin(
                        commonPhase * 0.37 + static_cast<double>(index) * 1.91)) * 0.82;
                    break;
                case PerformancePopulationPhenomenon::Count: break;
                }
                populationTarget = lerp(1.0, populationTarget, amount);
            }
            voice.populationEnvelope += (populationTarget - voice.populationEnvelope) * 0.008;
            if (populationDrive > 0.0)
                stem.mono = std::tanh(stem.mono * (1.0 + populationDrive))
                    / (1.0 + populationDrive * 0.30);
            stem.mono *= voice.populationEnvelope;

            if (codaActive_) {
                const double relationMemory = relationalEnsemble[index];
                double codaWeight = 1.0;
                switch (codaGesture_) {
                case CodaGesture::Gather:
                    codaWeight = index == codaLeaderIndex_ ? 1.0 - codaProgress_ * 0.18
                                                            : 1.0 - codaProgress_ * 0.72;
                    break;
                case CodaGesture::Relay: {
                    const auto distance = std::min((index + voiceCount_ - codaLeaderIndex_) % voiceCount_,
                                                   (codaLeaderIndex_ + voiceCount_ - index) % voiceCount_);
                    codaWeight = 1.0 - codaProgress_ * (0.24 + static_cast<double>(distance) * 0.24);
                    break;
                }
                case CodaGesture::Memory:
                    codaWeight = 1.0 - codaProgress_ * (0.62 - relationMemory * 0.30);
                    break;
                case CodaGesture::Disperse:
                    codaWeight = 1.0 - codaProgress_ * (0.36 + static_cast<double>(index) * 0.16);
                    break;
                }
                stem.mono *= clampUnit(codaWeight);
            }

            const bool stuttering = stutterRemainingSamples_ > 0U
                && (stutterTargetActorId_ == kInvalidActorId
                    || stutterTargetActorId_ == voice.actorId);
            const double stutterMix = stuttering ? stutterIntensity_ : 0.0;
            stem.mono *= lerp(1.0, voice.stutterEnvelope, stutterMix);
            // PROFUNDIDADE é o eixo Z audível: cada corpo pode recuar com
            // filtragem e atenuação leves, sem criar oscilador contínuo.
            const double depthWeight = static_cast<double>(index + 1U)
                / static_cast<double>(std::max<std::size_t>(voiceCount_, 1U));
            const double localDepth = clampUnit(depth_ * (0.36 + depthWeight * 0.64));
            voice.depthFiltered += (stem.mono - voice.depthFiltered)
                * (0.18 - localDepth * 0.145);
            stem.mono = lerp(stem.mono, voice.depthFiltered, localDepth * 0.78)
                * (1.0 - localDepth * 0.20);
            if (index >= 4U) {
                // Expanded-module controls survive the final phrase/depth
                // stage as a bounded articulation weight. This keeps a
                // resting Galo/Atropelo/Deja Vu responsive without adding a
                // continuous carrier or changing the four-bus topology.
                stem.mono *= 0.46 + voice.localControls[0] * 0.42
                    + voice.localControls[1] * 0.46
                    + voice.localControls[2] * 0.38;
            }
            stem.mono *= macroEnergyCompensation_ * director_.brightness(index);
        }
        if (stutterRemainingSamples_ > 0U && --stutterRemainingSamples_ == 0U) {
            stutterIntensity_ = 0.0;
            stutterTargetActorId_ = kInvalidActorId;
        }
        // Um corpo, um stem: nenhum corpo é dobrado dentro de outro.
        return frame;
    }

    double render(double sampleRate, double mutate, double tear, double repair = 0.0,
                  double hold = 0.0) noexcept {
        return renderStems(sampleRate, mutate, tear, repair, hold).mixedMono();
    }

private:
    // Actors 5..8 keep their stored controls while they are not voices of
    // the current asterism; once lit they are ordinary bodies with a stem.
    bool isInternalModuleVariationActor(std::uint16_t actorId) const noexcept {
        return actorId >= 5U && actorId <= 8U && presence(actorId) < 0.0;
    }

    PerformanceSeedEvent currentSeedEvent(std::uint32_t ordinal) const noexcept {
        return PerformanceSeedTrajectory::collage(seedTrajectory_.event(ordinal),
                                                   collageTrajectory_.event(ordinal),
                                                   collageAmount_);
    }

    static constexpr double kTwoPi = 6.28318530717958647692;
    static constexpr std::uint32_t kPressureRefreshSamples = 64U;
    static constexpr std::uint32_t kPitchRefreshSamples = 32U;

    static double curveTowardSquare(double value, double amount) noexcept {
        const double x = clampUnit(value);
        const double bounded = clampUnit(amount);
        return x * (1.0 - bounded + bounded * x);
    }

    static double curveTowardCube(double value, double amount) noexcept {
        const double x = clampUnit(value);
        const double bounded = clampUnit(amount);
        const double square = x * x;
        return square * (1.0 - bounded + bounded * x);
    }

    PerformanceMotionPoint baseLivePosition(std::size_t index, bool physics = true) const noexcept {
        if (index >= voiceCount_) return {};
        const auto& voice = voices_[index];
        const auto direction = voice.motionForward ? 1.0 : -1.0;
        const auto collectivePhase = motionPhase_ * kTwoPi + motionSeedOffset_;
        double centreX = 0.0;
        double centreY = 0.0;
        for (std::size_t body = 0U; body < voiceCount_; ++body) {
            centreX += voices_[body].x;
            centreY += voices_[body].y;
        }
        const auto count = std::max<std::size_t>(voiceCount_, 1U);
        const auto collective = performanceCollectiveMotionPoint(
            voice.x, voice.y, centreX / static_cast<double>(count), centreY / static_cast<double>(count),
            motionField_, collectivePhase);
        // Equal fields follow the constellation as one polygon. A distinct
        // field or reverse direction is an explicit local force and may bend
        // that formation around its collectively moving point.
        auto point = voice.motionField == motionField_ && voice.motionForward
            ? collective
            : performanceMotionPoint(collective.x, collective.y, voice.motionField,
                                     direction * motionPhase_ * kTwoPi + motionSeedOffset_,
                                     index, voice.presence);
        // Com a trama (30/10), o corpo está onde a física do céu o pôs — o
        // mesmo lugar que a Constelação desenha; as distâncias e relações da
        // ecologia ouvem esse movimento. Os campos de órbita viram famílias
        // de movimento no céu (ÓRBITA → LIVRE, PULSO → ESPUMA, FIGURA-8 →
        // PÊNDULOS), então continuam mudando o som. A profundidade segue a da
        // órbita. Sem a trama, a órbita de antes (som idêntico).
        if (physics && director_.positionsReady() && index < PerformanceSkyDirector::kBodies) {
            point.x = director_.shownX(index);
            point.y = director_.shownY(index);
        }
        return point;
    }

    PerformanceMotionPoint livePosition(std::size_t index, bool physics = true) const noexcept {
        const auto base = baseLivePosition(index, physics);
        if (index >= voiceCount_ || !populationPhenomenonEnabled_) return base;
        PerformanceMotionPoint centre{};
        for (std::size_t body = 0U; body < voiceCount_; ++body) {
            const auto point = baseLivePosition(body, physics);
            centre.x += point.x;
            centre.y += point.y;
            centre.z += point.z;
        }
        const auto count = static_cast<double>(std::max<std::size_t>(voiceCount_, 1U));
        centre.x /= count;
        centre.y /= count;
        centre.z /= count;
        return populationMotionPoint(populationPhenomenon_, base, centre,
                                     motionPhase_ * kTwoPi + motionSeedOffset_,
                                     populationPhenomenonIntensity_, index, voiceCount_);
    }

    struct Voice {
        std::uint16_t actorId = kInvalidActorId;
        PerformanceModuleBody star = PerformanceModuleBody::Count;
        PerformanceSeedEvent localSeed{};
        std::uint32_t localSeedTurn = 0U;
        std::uint32_t localSeedVersion = 0U;
        double localSeedVariation = -1.0;
        PerformanceRole role = PerformanceRole::Corpo;
        double phase = 0.0;
        double gain = 0.0;
        double targetGain = 0.0;
        double frequency = 0.0;
        double targetFrequency = 110.0;
        double liveTargetFrequency = 110.0;
        double x = 0.5;
        double y = 0.5;
        double pulsePhase = 0.0;
        double pulseEnvelope = 0.0;
        // Entrada audível primeiro; a primeira decisão de frase acontece
        // depois de uma pequena abertura, não no primeiro sample da sessão.
        double phrasePhase = 0.0;
        double phraseEnvelope = 1.0;
        double phraseDuty = 0.42;
        double phrasePitchSemitones = 0.0;
        std::uint32_t phraseStep = 0U;
        bool phraseActive = true;
        PerformanceMotionField motionField = PerformanceMotionField::Orbit;
        bool motionForward = true;
        double seedVariationAmount = 0.0;
        double motionEnvelope = 1.0;
        double stutterEnvelope = 1.0;
        double depthFiltered = 0.0;
        // A família de movimento escolhida para esta estrela (Count: nenhuma).
        PerformanceMotionFamily family = PerformanceMotionFamily::Count;
        // A âncora do céu: o lugar desenhado na tela (setSkyAnchor).
        double skyAnchorX = 0.5;
        double skyAnchorY = 0.5;
        bool skyAnchorSet = false;
        // O rasgo no stem: corte em andamento, portão com rampa, sorteio.
        std::uint32_t tearCutSamples = 0U;
        double tearGate = 1.0;
        std::uint32_t tearRandom = 0x9E3779B9U;
        std::array<double, 3> localControls{{0.50, 0.50, 0.50}};
        double orchestrationGain = 1.0;
        double populationEnvelope = 1.0;
        double presence = 0.0;
        double nourishment = 0.0;
        ActorLifeState life = ActorLifeState::Dormant;
        double stemGain = 1.0;
        double targetStemGain = 1.0;
        bool stemMuted = false;
        bool presenceOverridden = false;
        bool lifeOverridden = false;
    };

    // Quanto a fratura atinge um corpo: o alvo do último RASGAR inteiro; sem
    // alvo, os corpos de papel RASGO (forte) e ATRITO (leve).
    double fractureWeight(const Voice& voice) const noexcept {
        if (tornActorId_ != kInvalidActorId) return voice.actorId == tornActorId_ ? 1.0 : 0.0;
        return voice.role == PerformanceRole::Rasgo ? 0.55 : voice.role == PerformanceRole::Atrito ? 0.2 : 0.0;
    }

    // O rasgo no stem (29/10), só com a trama: o nível da fratura (cicatriz +
    // impulso de ruptura, 0–1) (a) dobra a onda — o timbre se rompe — e (b)
    // abre cortes curtos e irregulares — o tecido do som rasgado, mais
    // frequentes e longos quanto maior a fratura (até ~25 por segundo, de
    // 3 a 37 ms). Os cortes têm rampa de ~0,25 ms (um estalo macio: é um
    // rasgo, não um fade). REPARAR baixa a cicatriz e o
    // som se recompõe. Sem fratura, o corpo passa intacto.
    double tearStem(Voice& voice, double body, double fracture, double sampleRate) noexcept {
        const double amount = director_.influence() > 0.0 ? fracture * fractureWeight(voice) : 0.0;
        if (amount < 0.002 && voice.tearCutSamples == 0U && voice.tearGate >= 0.999) return body;
        const double folded = std::sin(body * (1.0 + amount * 5.0) * 1.5707963267948966);
        double torn = body + (folded - body) * std::min(1.0, amount * 1.4);
        voice.tearRandom = voice.tearRandom * 1664525U + 1013904223U;
        // Cada corpo sorteia por conta própria: os cortes não caem juntos.
        const auto own = voice.tearRandom ^ (static_cast<std::uint32_t>(voice.actorId) * 2654435761U);
        const double draw = static_cast<double>(own >> 8U) / 16777216.0;
        if (voice.tearCutSamples > 0U) {
            --voice.tearCutSamples;
        } else if (draw < amount * 25.0 / std::max(sampleRate, 1.0)) {
            voice.tearRandom = voice.tearRandom * 1664525U + 1013904223U;
            const double length = 0.003 + 0.034 * amount * static_cast<double>(voice.tearRandom >> 8U) / 16777216.0;
            voice.tearCutSamples = static_cast<std::uint32_t>(length * sampleRate);
            ++tearCuts_;
        }
        const double gateTarget = voice.tearCutSamples > 0U ? 0.0 : 1.0;
        voice.tearGate += (gateTarget - voice.tearGate) * std::min(1.0, 4000.0 / std::max(sampleRate, 1.0));
        torn *= voice.tearGate;
        return torn;
    }


    struct RelationControl {
        std::uint16_t sourceActorId = kInvalidActorId;
        std::uint16_t destinationActorId = kInvalidActorId;
        std::size_t sourceIndex = kVoiceCapacity;
        std::size_t destinationIndex = kVoiceCapacity;
        RelationKind kind = RelationKind::Support;
        RelationDomain domain = RelationDomain::Matter;
        PerformanceRelationalPhenomenon phenomenon = PerformanceRelationalPhenomenon::Attract;
        double resonanceRatio = 1.0;
        bool phenomenonEnabled = false;
        double activity = 0.0;
        double intensity = 0.0;
        bool intensityOverridden = false;
        PerformanceMotionPoint previousSource{};
        PerformanceMotionPoint previousDestination{};
        bool observationReady = false;
    };

    std::array<Voice, kVoiceCapacity> voices_{};
    std::array<double, kVoiceCapacity> internalLayerMix_{};
    std::array<double, 4U> internalModuleSeedVariation_{{0.5, 0.5, 0.5, 0.5}};
    std::array<std::array<double, 3U>, 4U> internalModuleControls_{{
        {{0.5, 0.5, 0.5}}, {{0.5, 0.5, 0.5}}, {{0.5, 0.5, 0.5}}, {{0.5, 0.5, 0.5}}}};
    std::array<std::array<double, 2U>, 4U> internalModulePositions_{{
        {{0.66, 0.66}}, {{0.33, 0.69}}, {{0.73, 0.38}}, {{0.42, 0.54}}}};
    std::array<RelationControl, kMorphRelationCapacity> relations_{};
    std::size_t voiceCount_ = 0;
    std::size_t relationCount_ = 0;
    std::uint16_t soloActorId_ = kInvalidActorId;
    double timePressure_ = 0.0;
    double matterPressure_ = 0.0;
    double frictionPressure_ = 0.0;
    double spacePressure_ = 0.0;
    double memoryPressure_ = 0.0;
    double relationalPulseCoupling_ = 0.0;
    double relationalTemporalFracture_ = 0.0;
    double relationalMatterExchange_ = 0.0;
    double relationalMatterFusion_ = 0.0;
    double relationalSpatialCirculation_ = 0.0;
    double relationalDynamicTension_ = 0.0;
    double relationalMemoryTrace_ = 0.0;
    double relationalSeedDeviation_ = 0.0;
    double relationalSilence_ = 0.0;
    std::uint16_t tornActorId_ = kInvalidActorId;
    struct StarSwap {
        std::uint16_t actorId = kInvalidActorId;
        PerformanceModuleBody star = PerformanceModuleBody::Count;
    };
    std::array<StarSwap, 8> starSwaps_{};
    bool seedConfigured_ = false;
    std::array<std::uint16_t, 8> pinnedActors_{{kInvalidActorId, kInvalidActorId, kInvalidActorId, kInvalidActorId,
                                                   kInvalidActorId, kInvalidActorId, kInvalidActorId, kInvalidActorId}};
    PerformanceMotionFamily collectiveFamily_ = PerformanceMotionFamily::Count;
    double fractureLevel_ = 0.0;
    std::uint32_t tearCuts_ = 0U;
    int lastFractureBreaks_ = 0;
    std::uint32_t pressureRefreshCountdown_ = 0U;
    std::uint32_t pitchRefreshCountdown_ = 0U;
    double macroEnergyCompensation_ = 1.0;
    double autonomy_ = 0.5;
    double intervention_ = 0.5;
    double density_ = 0.5;
    double tension_ = 0.5;
    double memory_ = 0.5;
    double risk_ = 0.5;
    double space_ = 0.5;
    double depth_ = 0.5;
    bool gridEnabled_ = false;
    GestureGridDivision gridDivision_ = GestureGridDivision::Beat;
    double tempoBpm_ = 120.0;
    double sampleRate_ = 48000.0;
    PerformanceSpatialGame spatialGame_{};
    PerformanceSeedTrajectory seedTrajectory_{};
    PerformanceSeedTrajectory collageTrajectory_{};
    std::uint32_t spatialGameTurn_ = 0U;
    PerformanceSpatialGameEvent lastSpatialGameEvent_{};
    PerformanceMotionField motionField_ = PerformanceMotionField::Orbit;
    PerformancePopulationPhenomenon populationPhenomenon_
        = PerformancePopulationPhenomenon::CollectiveRotate;
    bool populationPhenomenonEnabled_ = false;
    double populationPhenomenonIntensity_ = 0.65;
    double motionPhase_ = 0.0;
    double motionSeedOffset_ = 0.0;
    double orchestrationPhase_ = 0.0;
    double collageAmount_ = 0.0;
    bool codaActive_ = false;
    double codaProgress_ = 0.0;
    std::size_t codaLeaderIndex_ = 0U;
    CodaGesture codaGesture_ = CodaGesture::Gather;
    double stutterIntensity_ = 0.0;
    double stutterPhase_ = 0.0;
    std::uint32_t stutterRemainingSamples_ = 0U;
    std::uint16_t stutterTargetActorId_ = kInvalidActorId;
    double tempoShiftIntensity_ = 0.0;
    std::uint32_t tempoShiftRemainingSamples_ = 0U;
    std::uint16_t tempoShiftTargetActorId_ = kInvalidActorId;
    double tempoShiftRatio_ = 1.0;
    bool holding_ = false;
    TerritoryIdentity territoryIdentity_{};
    std::uint32_t seed_ = 181U;
    PerformanceCompositionProfile composition_{compositionProfileForSeed(181U)};
    std::uint32_t seedCacheVersion_ = 1U;
    PerformanceSky sky_{};
    PerformanceSkyDirector director_{};
    std::array<PerformanceSkyDirector::Body, PerformanceSkyDirector::kBodies> skyBodies_{};
    std::uint32_t skyBodiesCountdown_ = 0U;
    RasgoFractureAdapter fracture_{};
    RasgoFractureParameters fractureParameters_{};
    bool fracturePrepared_ = false;
    double lastFractureTear_ = 0.0;
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
    CoreModalMatterAdapter modalMatter_{};
    bool modalPrepared_ = false;
    double modalClock_ = 0.0;
    double modalLevel_ = 0.0;
    double lastTear_ = 0.0;
    std::uint32_t modalLastPhraseStep_ = std::numeric_limits<std::uint32_t>::max();
    std::array<double, 3> modalControls_{{0.48, 0.50, 0.42}};
#endif

    double frequencyFor(const MorphedActorState& actor, PerformanceRole role) const noexcept {
        return frequencyForVoice(actor.actorId, role);
    }

    // The Field's bodies 1..8 occupy slots 0..7; the slot, not the actor
    // id, decides the star, as the cohort lanes did before.
    PerformanceModuleBody starForSlot(std::size_t slot) const noexcept {
        const auto asterism = performanceInitialAsterism(seed_);
        return slot < asterism.size() ? asterism[slot] : PerformanceModuleBody::Count;
    }

    std::size_t modalLayerOwnerIndex() const noexcept {
        // First follow the strongest declared memory destination. This gives
        // the Modal layer a legible musical place in the constellation.
        std::size_t selected = kVoiceCapacity;
        double strongestMemory = -1.0;
        for (std::size_t relationIndex = 0U; relationIndex < relationCount_; ++relationIndex) {
            const auto& relation = relations_[relationIndex];
            if (relation.domain != RelationDomain::Memory || relation.intensity <= strongestMemory)
                continue;
            for (std::size_t voiceIndex = 0U; voiceIndex < voiceCount_; ++voiceIndex) {
                if (voices_[voiceIndex].actorId != relation.destinationActorId
                    || voices_[voiceIndex].life == ActorLifeState::Dormant) continue;
                selected = voiceIndex;
                strongestMemory = relation.intensity;
                break;
            }
        }
        if (selected < voiceCount_) return selected;
        // Without a memory cable, Vestigio is the natural archive; otherwise
        // the first audible body owns the bounded layer.
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            if (voices_[index].role == PerformanceRole::Vestigio
                && voices_[index].life != ActorLifeState::Dormant) return index;
        for (std::size_t index = 0U; index < voiceCount_; ++index)
            if (voices_[index].life != ActorLifeState::Dormant) return index;
        return kVoiceCapacity;
    }

    // The seed chooses a tonal dialect for the whole performance. This is
    // the missing Studio lineage: frequencies are realized as degrees of
    // a seed-selected scale, with jazz vocabularies as generative choices.
    static constexpr std::array<std::array<int, 8>, 6> kSeedScales{{
        {{0, 2, 3, 5, 7, 9, 10, 12}}, // Dorian
        {{0, 2, 4, 5, 7, 9, 10, 11}}, // dominant bebop
        {{0, 2, 4, 5, 7, 9, 11, 12}}, // major bebop
        {{0, 1, 3, 4, 6, 8, 10, 12}}, // altered
        {{0, 2, 3, 5, 6, 8, 9, 11}}, // diminished
        {{0, 2, 4, 6, 8, 10, 12, 14}} // whole tone
    }};
    const std::array<int, 8>& seedScale() const noexcept {
        return kSeedScales[static_cast<std::size_t>((composition_.harmonicDialect
            + ((seed_ >> 7U) % kSeedScales.size())) % kSeedScales.size())];
    }
    int seedRootMidi() const noexcept { return 36 + static_cast<int>((seed_ >> 3U) % 12U); }
    double seedTonalScale() const noexcept { return 0.78 + territoryIdentity_.tonalVocabulary * 0.48; }

public:
    // Leva uma altura à nota mais próxima da escala da seed (a mesma raiz,
    // modo e transposição tonal com que a ecologia afina os corpos). Usada
    // nos chamados da trama: uma resposta transformada continua dentro da
    // harmonia da peça.
    double quantizeToSeedScale(double hz) const noexcept {
        if (!finite(hz) || hz <= 0.0) return hz;
        const double tonal = seedTonalScale();
        const double midi = 69.0 + 12.0 * std::log2(hz / (440.0 * tonal));
        const auto& scale = seedScale();
        const int root = seedRootMidi();
        double best = midi;
        double bestDistance = 1.0e9;
        const int octaveBase = static_cast<int>(std::floor((midi - root) / 12.0));
        for (int octave = octaveBase - 1; octave <= octaveBase + 1; ++octave) {
            for (const int degree : scale) {
                const double candidate = static_cast<double>(root + octave * 12 + degree);
                const double distance = std::fabs(candidate - midi);
                if (distance < bestDistance) { bestDistance = distance; best = candidate; }
            }
        }
        return 440.0 * tonal * std::pow(2.0, (best - 69.0) / 12.0);
    }

private:
    double frequencyForVoice(std::uint16_t actorId, PerformanceRole role) const noexcept {
        const auto& composition = composition_;
        const auto& scale = seedScale();
        const auto roleNumber = static_cast<std::size_t>(role);
        const auto degree = static_cast<std::size_t>((actorId * 3U + roleNumber * 5U
            + (seed_ >> 11U)) % 7U);
        const int roleOctave = role == PerformanceRole::Chao ? 0
            : role == PerformanceRole::Vestigio ? 1
            : role == PerformanceRole::Corpo ? 2
            : role == PerformanceRole::Atrito ? 3
            : role == PerformanceRole::Contagio ? 4 : 5;
        const int rootMidi = seedRootMidi();
        const int styleRegister = composition.style == PerformanceCompositionStyle::Cortege ? -3
            : composition.style == PerformanceCompositionStyle::Orbit ? 3 : 0;
        const int midi = rootMidi + styleRegister + roleOctave * 12 + scale[degree];
        const double scaledFrequency = 440.0 * std::pow(2.0,
            (static_cast<double>(midi) - 69.0) / 12.0);
        const double idDetune = static_cast<double>((actorId + seed_) % 9U) * 0.7;
        const double tonalScale = seedTonalScale();
        return scaledFrequency * tonalScale + idDetune;
    }

    static std::uint32_t phraseWord(std::uint32_t seed, std::uint32_t step,
                                    std::uint16_t actorId) noexcept {
        std::uint32_t word = seed ^ (0x9e3779b9U * (step + 0x7f4a7c15U))
            ^ (0x85ebca6bU * (static_cast<std::uint32_t>(actorId) + 1U));
        word ^= word >> 16U;
        word *= 0x7feb352dU;
        word ^= word >> 15U;
        word *= 0x846ca68bU;
        return word ^ (word >> 16U);
    }

    static double phraseRateFor(std::uint16_t actorId, std::size_t index) noexcept {
        // 3--5 deixam de sustentar um leito imóvel; 6--8 deixam de repetir
        // uma célula curta. Os valores são deliberadamente lentos: são
        // decisões de frase, não um novo LFO de tela.
        switch (actorId) {
        case 3U: return 0.70; // percussão: células e respiros
        case 4U: return 0.31; // quarto corpo: aparições e silêncio
        case 5U: return 0.22; // matéria modal: ressonâncias raras
        case 6U: return 0.27; // chamada Galo: frase com intervalo amplo
        case 7U: return 0.46; // Atropelo: sílabas irregulares
        case 8U: return 0.19; // Deja Vu: retorno transformado, não metrônomo
        default: return 0.36 + static_cast<double>(index % 3U) * 0.12;
        }
    }

    static bool phraseSpeaks(std::uint16_t actorId, std::uint32_t word,
                             const PerformanceSeedEvent& event,
                             double relation) noexcept {
        const auto draw = static_cast<double>(word & 0xffU) / 255.0;
        const double seedLift = event.densityBias * 0.12 + relation * 0.16;
        const double probability = actorId == 3U ? 0.68
            : actorId == 4U ? 0.47 : actorId == 5U ? 0.36
            : actorId == 6U ? 0.42 : actorId == 7U ? 0.54
            : actorId == 8U ? 0.38 : 0.52;
        return draw < std::min(0.88, probability + seedLift);
    }

    static double phraseDutyFor(std::uint16_t actorId, std::uint32_t word,
                                 const PerformanceSeedEvent& event,
                                 double reply) noexcept {
        const double draw = static_cast<double>((word >> 8U) & 0xffU) / 255.0;
        const double contour = event.memoryBias * 0.12 + reply * 0.10;
        const double base = actorId == 3U ? 0.18
            : actorId == 4U ? 0.32 : actorId == 5U ? 0.46
            : actorId == 6U ? 0.29 : actorId == 7U ? 0.16
            : actorId == 8U ? 0.38 : 0.30;
        return std::clamp(base + draw * 0.28 + contour, 0.10, 0.82);
    }

    static double phrasePitchFor(std::uint16_t actorId, std::uint32_t word,
                                 const PerformanceSeedEvent& event,
                                 double reply) noexcept {
        static constexpr std::array<int, 8> degrees{{0, 2, 3, 5, 7, 9, 10, 12}};
        const auto degree = degrees[(word >> 17U) & 7U];
        const int registerShift = ((word >> 23U) & 3U) == 0U ? -12
            : ((word >> 23U) & 3U) == 3U ? 12 : 0;
        const double relationalBend = (reply - 0.5) * 2.0 + event.tensionBias * 1.4;
        const double roleOffset = actorId == 3U ? -12.0 : actorId == 5U ? -5.0
            : actorId == 6U ? 7.0 : actorId == 7U ? 3.0 : actorId == 8U ? -2.0 : 0.0;
        return static_cast<double>(degree + registerShift) + roleOffset + relationalBend;
    }

    static double lifeGain(ActorLifeState life) noexcept {
        switch (life) {
        case ActorLifeState::Dormant: return 0.0;
        case ActorLifeState::Emerging: return 0.58;
        case ActorLifeState::Active: return 1.0;
        case ActorLifeState::Yielding: return 0.62;
        case ActorLifeState::Ruptured: return 0.82;
        case ActorLifeState::Recovering: return 0.68;
        case ActorLifeState::Count: break;
        }
        return 0.0;
    }

    // A body is dark once its presence is zero and its fade-out has ended,
    // or once the mixer variation has put its star out and its fade ended.
    static bool isDark(const Voice& voice) noexcept {
        return voice.presence <= 0.0 && voice.gain < 1.0e-5;
    }
    bool isDarkAt(std::size_t index) const noexcept {
        if (index >= voiceCount_) return true;
        return isDark(voices_[index])
            || (director_.mixerVariation() && director_.brightnessTarget(index) < 0.5
                && director_.brightness(index) < 1.0e-4);
    }

    static double targetGainFor(const Voice& voice) noexcept {
        return voice.presence * lifeGain(voice.life)
            // Nourishment is an event/ecology input, not a permanent VCA.
            // Keeping it out of the baseline prevents a continuously fed tone.
            * 0.20;
    }

    double pulseRateFor(PerformanceRole role, std::size_t index) const noexcept {
        const double dna = 0.60 + territoryIdentity_.temporalDna * 1.35;
        const double relation = 0.62 + timePressure_ * 1.85;
        const double density = 0.70 + density_ * 0.85;
        double roleRate = 1.0;
        switch (role) {
        case PerformanceRole::Chao: roleRate = 0.25; break;
        case PerformanceRole::Corpo: roleRate = 1.0; break;
        case PerformanceRole::Atrito: roleRate = 1.5; break;
        case PerformanceRole::Contagio: roleRate = 2.0; break;
        case PerformanceRole::Rasgo: roleRate = 3.0; break;
        case PerformanceRole::Vestigio: roleRate = 0.5; break;
        case PerformanceRole::Count: break;
        }
        return roleRate * dna * relation * density * (1.0 + static_cast<double>(index) * 0.025);
    }

    double pulseDutyFor(PerformanceRole role) const noexcept {
        const double openness = 0.20 + density_ * 0.45 + memory_ * 0.16;
        switch (role) {
        case PerformanceRole::Chao: return 0.84;
        case PerformanceRole::Corpo: return clampUnit(openness + 0.10);
        case PerformanceRole::Atrito: return clampUnit(openness - 0.08 + risk_ * 0.18);
        case PerformanceRole::Contagio: return clampUnit(openness - 0.02 + timePressure_ * 0.12);
        case PerformanceRole::Rasgo: return clampUnit(0.12 + risk_ * 0.35 + tension_ * 0.10);
        case PerformanceRole::Vestigio: return clampUnit(openness + memory_ * 0.16);
        case PerformanceRole::Count: break;
        }
        return clampUnit(openness);
    }

    static double pulseWeightFor(PerformanceRole role, double envelope) noexcept {
        const double bounded = clampUnit(envelope);
        return role == PerformanceRole::Chao ? 0.62 + bounded * 0.38
             : role == PerformanceRole::Vestigio ? 0.46 + bounded * 0.54
             : 0.18 + bounded * 0.82;
    }

    void updatePressures() noexcept {
        timePressure_ = 0.0;
        matterPressure_ = 0.0;
        frictionPressure_ = 0.0;
        spacePressure_ = 0.0;
        memoryPressure_ = 0.0;
        relationalPulseCoupling_ = 0.0;
        relationalTemporalFracture_ = 0.0;
        relationalMatterExchange_ = 0.0;
        relationalMatterFusion_ = 0.0;
        relationalSpatialCirculation_ = 0.0;
        relationalDynamicTension_ = 0.0;
        relationalMemoryTrace_ = 0.0;
        relationalSeedDeviation_ = 0.0;
        relationalSilence_ = 0.0;
        if (populationPhenomenonEnabled_) {
            double cohesion = 0.0;
            for (std::size_t voice = 0U; voice < voiceCount_; ++voice)
                cohesion += voices_[voice].presence;
            cohesion /= static_cast<double>(std::max<std::size_t>(voiceCount_, 1U));
            const auto population = composePopulationPhenomenon(
                populationPhenomenon_, motionPhase_ * kTwoPi + motionSeedOffset_,
                populationPhenomenonIntensity_, cohesion);
            relationalPulseCoupling_ += population.pulseCoupling;
            relationalTemporalFracture_ += population.temporalFracture;
            relationalMatterExchange_ += population.matterExchange;
            relationalMatterFusion_ += population.matterFusion;
            relationalSpatialCirculation_ += population.spatialCirculation;
            relationalDynamicTension_ += population.dynamicTension;
            relationalMemoryTrace_ += population.memoryTrace;
            relationalSeedDeviation_ += population.seedDeviation;
            relationalSilence_ += population.silence;
        }
        for (std::size_t index = 0; index < relationCount_; ++index) {
            auto& relation = relations_[index];
            // A cable involving a body still in the reserve is potential, not
            // an audible relation. Pressure grows continuously with both
            // endpoint presences as the bodies enter the constellation.
            const auto sourcePresence = std::max(0.0, presence(relation.sourceActorId));
            const auto destinationPresence = std::max(0.0, presence(relation.destinationActorId));
            const double membership = std::sqrt(sourcePresence * destinationPresence);
            double spatialCoupling = 1.0;
            {
                std::size_t sourceIndex = voiceCount_;
                std::size_t destinationIndex = voiceCount_;
                for (std::size_t voiceIndex = 0U; voiceIndex < voiceCount_; ++voiceIndex) {
                    if (voices_[voiceIndex].actorId == relation.sourceActorId) sourceIndex = voiceIndex;
                    if (voices_[voiceIndex].actorId == relation.destinationActorId) destinationIndex = voiceIndex;
                }
                if (sourceIndex < voiceCount_ && destinationIndex < voiceCount_) {
                    const auto source = livePosition(sourceIndex);
                    const auto destination = livePosition(destinationIndex);
                    if (!relation.observationReady) {
                        relation.previousSource = source;
                        relation.previousDestination = destination;
                        relation.observationReady = true;
                    }
                    const auto observation = observePerformanceRelation(
                        relation.previousSource, relation.previousDestination,
                        source, destination, static_cast<double>(kPressureRefreshSamples)
                            / std::max(sampleRate_, 1.0));
                    relation.previousSource = source;
                    relation.previousDestination = destination;
                    const auto forces = relation.phenomenonEnabled
                        ? composeRelationalPhenomenon(
                            relation.phenomenon, observation,
                            motionPhase_ * kTwoPi + static_cast<double>(index) * 0.71,
                            relation.activity * relation.intensity * membership,
                            relation.resonanceRatio)
                        : PerformanceCompositionalForces{};
                    relationalPulseCoupling_ += forces.pulseCoupling;
                    relationalTemporalFracture_ += forces.temporalFracture;
                    relationalMatterExchange_ += forces.matterExchange;
                    relationalMatterFusion_ += forces.matterFusion;
                    relationalSpatialCirculation_ += forces.spatialCirculation;
                    relationalDynamicTension_ += forces.dynamicTension;
                    relationalMemoryTrace_ += forces.memoryTrace;
                    relationalSeedDeviation_ += forces.seedDeviation;
                    relationalSilence_ += forces.silence;
                    const auto distance = observation.distance;
                    spatialCoupling = 0.35 + 0.65 * clampUnit(1.0 - distance);
                }
            }
            const double pressure = relation.activity * relation.intensity * membership * spatialCoupling;
            if (relation.domain == RelationDomain::Time) timePressure_ += pressure;
            if (relation.domain == RelationDomain::Matter) matterPressure_ += pressure;
            if (relation.domain == RelationDomain::Space) spacePressure_ += pressure;
            if (relation.domain == RelationDomain::Memory) memoryPressure_ += pressure;
            if (relation.kind == RelationKind::Friction) frictionPressure_ += pressure;
        }
        timePressure_ = clampUnit(timePressure_ * (0.55 + territoryIdentity_.relationBias * 0.90));
        matterPressure_ = clampUnit(matterPressure_ * (0.55 + territoryIdentity_.matterBias * 0.90));
        frictionPressure_ = clampUnit(frictionPressure_);
        spacePressure_ = clampUnit(spacePressure_);
        memoryPressure_ = clampUnit(memoryPressure_ * (0.55 + territoryIdentity_.relationBias * 0.65));
        relationalPulseCoupling_ = clampUnit(relationalPulseCoupling_);
        relationalTemporalFracture_ = clampUnit(relationalTemporalFracture_);
        relationalMatterExchange_ = clampUnit(relationalMatterExchange_);
        relationalMatterFusion_ = clampUnit(relationalMatterFusion_);
        relationalSpatialCirculation_ = clampUnit(relationalSpatialCirculation_);
        relationalDynamicTension_ = clampUnit(relationalDynamicTension_);
        relationalMemoryTrace_ = clampUnit(relationalMemoryTrace_);
        relationalSeedDeviation_ = clampUnit(relationalSeedDeviation_);
        relationalSilence_ = clampUnit(relationalSilence_);
        timePressure_ = clampUnit(timePressure_ + relationalPulseCoupling_ * 0.42
                                  + relationalTemporalFracture_ * 0.58);
        matterPressure_ = clampUnit(matterPressure_ + relationalMatterExchange_ * 0.44
                                    + relationalMatterFusion_ * 0.52);
        frictionPressure_ = clampUnit(frictionPressure_ + relationalDynamicTension_ * 0.54
                                      + relationalTemporalFracture_ * 0.34);
        spacePressure_ = clampUnit(spacePressure_ + relationalSpatialCirculation_ * 0.66);
        memoryPressure_ = clampUnit(memoryPressure_ + relationalMemoryTrace_ * 0.60);
    }

#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
    void configureModalMatter(const Voice* liveBody = nullptr) noexcept {
        modalLevel_ = 0.0;
        if (!modalPrepared_ || voiceCount_ == 0U) return;
        const Voice* body = liveBody;
        // When the fifth capacity gate is lifted, the ModalResonator becomes
        // actor 5 rather than a generic global return. Until then this falls
        // back to the first audible body and remains a bounded modal return.
        if (const auto owner = modalOwner(); owner < voiceCount_) body = &voices_[owner];
        for (std::size_t index = 0; index < voiceCount_; ++index) {
            if (body != nullptr) break;
            if (voices_[index].role == PerformanceRole::Chao
                || voices_[index].role == PerformanceRole::Corpo
                || voices_[index].role == PerformanceRole::Vestigio) {
                body = &voices_[index];
                break;
            }
        }
        if (body == nullptr) body = &voices_[0];
        ModalMatterParameters parameters;
        const auto phraseFrequency = body->frequency > 1.0 ? body->frequency : body->targetFrequency;
        parameters.fundamentalHz = std::clamp(phraseFrequency * (0.72 + modalControls_[0] * 0.70), 28.0, 880.0);
        // MODAL's second local control is its timbral articulation. The
        // previous 0.30 span was effectively masked by the bounded internal
        // layer, leaving the visible control technically connected but
        // musically weak. Keep it bounded while giving brightness a real A/B
        // range inside the owned stem.
        parameters.brightness = clampUnit(0.12 + modalControls_[1] * 0.62
            + matterPressure_ * 0.30 + frictionPressure_ * 0.12);
        parameters.relationPressure = clampUnit(timePressure_ * 0.30 + matterPressure_ * 0.40
            + memoryPressure_ * 0.55);
        parameters.memory = clampUnit(memory_ * (0.78 + modalControls_[2] * 0.32)
            + memoryPressure_ * 0.22);
        parameters.role = body->role;
        if (modalMatter_.setParameters(parameters))
            modalLevel_ = clampUnit(0.045 + density_ * 0.10 + modalControls_[1] * 0.16
                + modalControls_[2] * 0.14 + body->presence * 0.22
                + body->nourishment * 0.10);
    }
#endif

    double waveform(PerformanceRole role, double base, double harmonic, double scar) const noexcept {
        switch (role) {
        case PerformanceRole::Chao: return base * 0.9 + harmonic * 0.08;
        case PerformanceRole::Corpo: return base * 0.72 + harmonic * 0.28;
        case PerformanceRole::Atrito: return base * 0.48 + harmonic * 0.52 + scar * (0.4 + frictionPressure_);
        case PerformanceRole::Contagio: return base * (0.62 + matterPressure_ * 0.20) + harmonic * 0.38;
        case PerformanceRole::Rasgo: return harmonic * 0.65 + scar * (0.75 + frictionPressure_ * 0.25);
        case PerformanceRole::Vestigio: return base * 0.45 + harmonic * 0.18;
        case PerformanceRole::Count: break;
        }
        return base;
    }
};

} // namespace rasgo::performance
