#pragma once

// O diretor do céu: a trama de relações entre as estrelas acesas.
//
// Decisões do autor (2026-10-28): "não haverão estrelas sozinhas, todas as
// estrelas se conectam para compor generativamente"; "não que todas as
// estrelas precisem se relacionar ao mesmo tempo, mas criar recursos
// inteligentes, sobretudo no quesito composição"; inspirar-se na teia do
// Aquorbium; "cálculos das leis físicas". Proposta completa em
// docs/TRAMA_DE_RELACOES.md.
//
// Primeira fatia, num relógio de controle (a cada kControlInterval amostras),
// fora do trabalho por amostra:
// - gravitação de N corpos (Newton, integrador leapfrog simplético): cada
//   estrela acesa é atraída pelas outras e pelo poço do seu lugar desenhado;
// - percepção: cada estrela sente as outras pela massa e pela distância
//   calculadas (nenhuma sente o céu inteiro com a mesma força);
// - teia: o tipo de relação de cada par emerge da função das duas estrelas e
//   da seed; a intensidade sobe rápido e decai devagar (como no Aquorbium);
// - foco: a cada compasso, só poucas relações ficam em primeiro plano, com
//   preferência pelas que não soaram há pouco (pressão por novidade);
// - atos: resposta (com "tempo-luz" proporcional à distância e a altura
//   ouvida, transformada), contágio, sombra (aumentação, mais grave), hoquetus
//   (no vão entre dois ataques) e apoio (pedal grave).
//
// Determinístico pela seed, sem alocação, lock ou I/O. Influência 0 deixa o
// som idêntico ao de antes desta camada.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "PerformanceContracts.hpp"
#include "PerformanceSky.hpp"
#include "PerformanceSkyMotion.hpp"

namespace rasgo::performance {

class PerformanceSkyDirector {
public:
    static constexpr std::size_t kBodies = 8U;
    static constexpr std::uint32_t kControlInterval = 64U;
    static constexpr std::size_t kMaxFocus = 3U;
    static constexpr std::size_t kCueCapacity = 32U;

    struct Body {
        bool lit = false;
        PerformanceModuleBody star = PerformanceModuleBody::Count;
        double anchorX = 0.5;
        double anchorY = 0.5;
        double presence = 0.0;
        // Posição viva no Campo (o movimento da Constelação: órbita, pulso,
        // figura-8), usada pela variação contínua do mixer.
        double liveX = 0.5;
        double liveY = 0.5;
    };

    struct Cue {
        std::size_t slot = kBodies;
        double pitchHz = 0.0;
        double strength = 0.0;
        std::uint64_t dueSample = 0U;
        std::uint8_t depth = 1U;
        // Atrito: a altura fica fora da escala de propósito (não é ajustada).
        bool chromatic = false;
    };
    // Uma resposta pode provocar outra (A chama B, B chama C), mas não mais
    // que kMaxChainDepth elos: sem isso dois contágios recíprocos de atraso
    // zero viravam um eco a cada amostra.
    static constexpr std::uint8_t kMaxChainDepth = 2U;

    struct Focus {
        std::size_t caller = kBodies;
        std::size_t responder = kBodies;
        RelationKind kind = RelationKind::Support;
    };

    void prepare(double sampleRate) noexcept {
        sampleRate_ = std::isfinite(sampleRate) && sampleRate > 0.0 ? sampleRate : 48000.0;
        reset();
    }

    void reset() noexcept {
        sample_ = 0U;
        countdown_ = 0U;
        beatPhase_ = 0.0;
        beat_ = 0U;
        cueCount_ = 0U;
        focusCount_ = 0U;
        intensity_ = {};
        recentFocus_ = {};
        lastAttackSample_ = {};
        previousAttackSample_ = {};
        lastCueSample_ = {};
        answerDepth_ = {};
        brightness_.fill(1.0);
        level_.fill(1.0);
        motif_ = {};
        motifHead_ = {};
        motifCount_ = {};
        pairResponses_ = {};
        tension_ = 0.0;
        tempoFactor_ = 1.0;
        springPosition_ = 1.0;
        tempoVelocity_ = 0.0;
        swing_ = 0.5;
        groove_ = 0.0;
        section_ = Section::Opening;
        sectionOrdinal_ = 0U;
        formStep_ = 0U;
        formPass_ = 0U;
        sectionBarsLeft_ = 0U;
        characterReady_ = false;
        sectionEnergy_.fill(0.0);
        recedeSlot_ = kBodies;
        recedeGain_ = 1.0;
        openingLeader_ = kBodies;
        openingLit_ = {};
        brightnessTarget_.fill(1.0);
        litBars_ = {};
        restBars_ = {};
        heldBars_ = {};
        brightnessReady_ = false;
        heardPitch_ = {};
        attackCount_ = {};
        positionsReady_ = false;
        chorusPhase_.fill(kChorusRest);
        driftSlot_ = kBodies;
        driftRatio_ = 1.0;
        onsets_ = {};
        rhythm_ = {};
        sixteenth_ = 0U;
        // Onde estão as luas quando a peça começa: da seed.
        synodicOffset_ = kSynodicBeats * static_cast<double>(mix(seed_ ^ 0x5A405U) & 1023U) / 1024.0;
        draconicOffset_ = static_cast<double>(mix(seed_ ^ 0xD4AC0U) & 1023U) / 1024.0;
        lunation_ = static_cast<std::uint64_t>(std::floor(synodicOffset_ / kSynodicBeats));
        eclipseSun_ = kBodies;
        eclipseBeatsLeft_ = 0U;
        eclipseCount_ = 0U;
        dance_ = seededChoreography(Section::Opening);
        motionTime_ = 0.0;
        rotationSign_ = (mix(seed_ ^ 0x601AU) & 1U) != 0U ? 1.0 : -1.0;
        spin_ = {};
        chain_.fill(kBodies);
        collisionCount_ = 0U;
        openingCount_ = 0U;
        openingSlot_ = kBodies;
        recapitulations_ = 0U;
        pendulumTurns_ = 0U;
        pendulumOver_ = {};
        for (std::size_t slot = 0U; slot < kBodies; ++slot)
            charge_[slot] = (mix(seed_ ^ static_cast<std::uint32_t>(0xC4A26EU + slot)) & 1U) != 0U ? 1.0 : -1.0;
        flowShape_ = {};
        flowPhase_ = 0.0;
        for (std::size_t slot = 0U; slot < kBodies; ++slot)
            chorusOmega_[slot] = 0.25 * (static_cast<double>(mix(seed_ ^ static_cast<std::uint32_t>(0xC0A0U + slot)) & 1023U)
                                         / 511.5 - 1.0);
    }

    void setSeed(std::uint32_t seed) noexcept {
        seed_ = seed;
        grammar_ = static_cast<std::uint8_t>(performanceSeedStream(seed, 0xF200U) % kGrammars.size());
        // As regiões harmônicas (5/11, auditoria da seed, item D): o contraste
        // vai para outra região (subdominante, dominante, relativa, mediantes,
        // trítono, vizinhas), o desenvolvimento às vezes para uma vizinha, a
        // abertura e o retorno ficam na tônica. A escala da seed é a mesma; o
        // centro se move. Era o que a especificação de setembro pedia
        // ("regiões, progressões, pivôs") e o código não tinha.
        static constexpr std::array<int, 9> kContrastRegions{{5, 7, -3, 4, -4, 6, -5, 2, 3}};
        static constexpr std::array<int, 6> kDevelopmentRegions{{0, 0, 0, 2, -2, 5}};
        contrastRegion_ = kContrastRegions[performanceSeedStream(seed, 0xF300U) % kContrastRegions.size()];
        developmentRegion_ = kDevelopmentRegions[performanceSeedStream(seed, 0xF301U) % kDevelopmentRegions.size()];
        chooseSeedMotion();
        chooseDensityGenes();
        reset();
    }
    // A gramática da forma desta seed (0 ciclo, 1 arco, 2 rondó, 3 ondas,
    // 4 kishōtenketsu, 5 choque inicial) e quantas voltas ela já completou.
    std::uint8_t grammar() const noexcept { return grammar_; }
    std::uint8_t formPass() const noexcept { return static_cast<std::uint8_t>(formPass_ & 0xffU); }
    // Os gestos do céu (5/11, auditoria da seed, item E): o céu também faz
    // STUTTER e DESLOCAR, com um orçamento por parte (0 a 2, pela energia do
    // caráter e pela seed; nenhum na abertura, no máximo um no retorno), num
    // compasso e numa estrela acesa escolhidos pela seed. A ecologia não o
    // aplica em cima de um gesto do músico.
    struct SkyGesture {
        std::size_t slot = 0U;
        bool displace = false;
        double intensity = 0.5;
        double duration = 0.4;
        std::uint32_t token = 0U;
    };
    bool popSkyGesture(SkyGesture& out) noexcept {
        if (!gesturePending_) return false;
        out = pendingGesture_;
        gesturePending_ = false;
        return true;
    }
    std::uint32_t skyGestureCount() const noexcept { return skyGestures_; }
    // A faixa de densidade desta seed (estrelas acesas e atividade).
    double litLow() const noexcept { return litLow_; }
    double litHigh() const noexcept { return litHigh_; }
    double activityLow() const noexcept { return activityLow_; }

    // A região harmônica da parte que soa, em semitons sobre a raiz da seed.
    double regionSemitones() const noexcept {
        if (influence_ <= 0.0) return 0.0;
        switch (section_) {
        case Section::Contrast: return static_cast<double>(contrastRegion_);
        case Section::Development: return static_cast<double>(developmentRegion_);
        default: return 0.0;
        }
    }
    static const char* grammarName(std::uint8_t grammar) noexcept {
        static constexpr std::array<const char*, 6> names{{"ciclo", "arco", "rondo", "ondas", "kishotenketsu", "choque"}};
        return grammar < names.size() ? names[grammar] : "ciclo";
    }
    // Centro tonal da seed (a fundamental da escala com que os corpos são
    // afinados), usado pela cadência.
    void setTonalCenter(double hz) noexcept {
        if (std::isfinite(hz) && hz > 0.0) tonalCenterHz_ = hz;
    }
    // A intenção da peça: a estratégia do jogo (GamePlan do Studio) dá
    // caráter à trama — metade das relações em foco segue a estratégia, a
    // outra metade o tipo natural do par (função das duas estrelas).
    void setStrategy(PerformanceGameStrategy strategy) noexcept { strategy_ = strategy; }

    // A forma (passo 4 da trama): seções vivas que dão direção à peça.
    enum class Section : std::uint8_t { Opening, Development, Contrast, Return, Count };
    // A gramática por dentro (para os testes e o registro da tomada).
    static Section grammarPart(std::uint8_t grammar, std::size_t index) noexcept {
        const auto& g = kGrammars[grammar < kGrammars.size() ? grammar : 0U];
        return g.parts[std::min<std::size_t>(index, 7U)];
    }
    static std::size_t grammarCount(std::uint8_t grammar) noexcept {
        return kGrammars[grammar < kGrammars.size() ? grammar : 0U].count;
    }
    static std::size_t grammarLoop(std::uint8_t grammar) noexcept {
        return kGrammars[grammar < kGrammars.size() ? grammar : 0U].loop;
    }
    Section section() const noexcept { return section_; }
    // A métrica das contagens: a de casa (0) na abertura, no desenvolvimento
    // e no retorno; uma nova a cada contraste (modulação métrica).
    std::uint32_t countingOrdinal() const noexcept {
        return section_ == Section::Contrast ? sectionOrdinal_ : 0U;
    }
    std::uint32_t sectionOrdinal() const noexcept { return sectionOrdinal_; }
    PerformanceGameStrategy strategy() const noexcept { return strategy_; }
    // Tempo elástico, swing e groove (passo do tempo maleável, 29/10).
    double tempoFactor() const noexcept { return tempoFactor_; }
    double swing() const noexcept { return swing_; }
    double groove() const noexcept { return groove_; }
    // Coro de fases (29/10): a fase de cada corpo dentro do pulso, como
    // fração de um passo em [0, 1); 0 com a trama desligada.
    // Fourier do ritmo (29/10): a transformada discreta dos ataques recentes
    // (48 semicolcheias, 3 compassos — 48 é o mínimo múltiplo comum dos
    // períodos, assim a volta do anel não quebra a fase de nenhum deles).
    // Do espectro de potência |X(k)|², a transformada inversa dá a
    // autocorrelação circular R (teorema de Wiener–Khinchin): R(P)/R(0) = 1
    // quando o padrão se repete exatamente a cada P semicolcheias. O período
    // dominante é o MENOR período musical cuja periodicidade chega a 90% da
    // melhor (os múltiplos de um ciclo também se repetem); a clareza é essa
    // periodicidade. Sem ataques suficientes, {0, 0}.
    struct RhythmPeriod { std::uint32_t period = 0U; double clarity = 0.0; };
    static constexpr std::array<std::uint32_t, 7> kRhythmPeriods{{2U, 3U, 4U, 6U, 8U, 12U, 16U}};
    static constexpr std::uint32_t kRhythmRing = 48U;
    static RhythmPeriod dominantPeriod(std::uint64_t onsets) noexcept {
        std::uint32_t count = 0U;
        for (std::uint64_t bits = onsets; bits != 0U; bits &= bits - 1U) ++count;
        if (count < 3U) return {};
        std::array<double, kRhythmRing> cosine{};
        std::array<double, kRhythmRing> sine{};
        for (std::uint32_t n = 0U; n < kRhythmRing; ++n) {
            cosine[n] = std::cos(kChorusTwoPi * n / kRhythmRing);
            sine[n] = std::sin(kChorusTwoPi * n / kRhythmRing);
        }
        std::array<double, kRhythmRing> power{};
        for (std::uint32_t k = 0U; k < kRhythmRing; ++k) {
            double re = 0.0;
            double im = 0.0;
            for (std::uint32_t n = 0U; n < kRhythmRing; ++n) {
                if (((onsets >> n) & 1U) == 0U) continue;
                const auto index = (k * n) % kRhythmRing;
                re += cosine[index];
                im -= sine[index];
            }
            power[k] = re * re + im * im;
        }
        const auto autocorrelation = [&](std::uint32_t lag) {
            double sum = 0.0;
            for (std::uint32_t k = 0U; k < kRhythmRing; ++k) sum += power[k] * cosine[(k * lag) % kRhythmRing];
            return sum / static_cast<double>(kRhythmRing);
        };
        const double zero = autocorrelation(0U);
        std::array<double, kRhythmPeriods.size()> periodicity{};
        double best = 0.0;
        for (std::size_t index = 0U; index < kRhythmPeriods.size(); ++index) {
            periodicity[index] = std::clamp(autocorrelation(kRhythmPeriods[index]) / zero, 0.0, 1.0);
            best = std::max(best, periodicity[index]);
        }
        for (std::size_t index = 0U; index < kRhythmPeriods.size(); ++index)
            if (periodicity[index] >= best * 0.9 && best > 0.0) return {kRhythmPeriods[index], periodicity[index]};
        return {};
    }
    RhythmPeriod rhythm(std::size_t slot) const noexcept { return slot < kBodies ? rhythm_[slot] : RhythmPeriod{}; }
    // Os ataques recentes de um corpo (bit n = semicolcheia n do anel).
    std::uint64_t onsetMask(std::size_t slot) const noexcept { return slot < kBodies ? onsets_[slot] : 0U; }

    // Eclipses (29/10): duas luas medidas em tempos — o mês sinódico (de lua
    // nova a lua nova, 29,530589) e o dracônico (de nodo a nodo, 27,212221).
    // Em cada lua nova há eclipse se a lua está perto de um nodo (fase
    // dracônica a menos de 0,051 do ciclo de 0 ou de ½: os ±18,5° do limite
    // dos eclipses solares). Como os dois meses diferem, os eclipses vêm em
    // estações; 223 meses sinódicos quase igualam 242 dracônicos (o Saros),
    // e a sequência inteira se repete. `n` é o número da lunação.
    static constexpr double kSynodicBeats = 29.530589;
    static constexpr double kDraconicBeats = 27.212221;
    static constexpr double kEclipseLimit = 0.051;
    static bool eclipseAtLunation(std::uint64_t n, double draconicOffset) noexcept {
        double phase = static_cast<double>(n) * kSynodicBeats / kDraconicBeats + draconicOffset;
        phase -= std::floor(phase);
        const double fromNode = std::min({phase, std::fabs(phase - 0.5), 1.0 - phase});
        return fromNode < kEclipseLimit;
    }
    // O corpo encoberto agora (kBodies: nenhum) e quantos eclipses já houve.
    std::size_t eclipsedSlot() const noexcept {
        return influence_ > 0.0 && eclipseBeatsLeft_ > 0U ? eclipseSun_ : kBodies;
    }
    std::uint32_t eclipseCount() const noexcept { return eclipseCount_; }

    // Os cabos desenhados (29/10). As 16 relações fixas do território e o
    // fenômeno de cada cabo antes chegavam ao céu somados em cinco pressões
    // globais — quem se relaciona com quem se perdia. Agora cada cabo entra
    // na trama: a intensidade aumenta a chance de o par entrar em foco, o
    // fenômeno escolhe o ato musical do par e vira uma força física entre os
    // dois corpos (docs/CODIGO_SUBAPROVEITADO_2026-10-29.md §3.1).
    static constexpr std::uint8_t kNoPhenomenon = 255U;
    void clearCables() noexcept {
        for (auto& row : cable_) row.fill(0.0);
        for (auto& row : cablePhenomenon_) row.fill(kNoPhenomenon);
    }
    void setCable(std::size_t a, std::size_t b, double intensity, std::uint8_t phenomenon) noexcept {
        if (a >= kBodies || b >= kBodies || a == b || !std::isfinite(intensity)) return;
        cable_[a][b] = std::max(cable_[a][b], std::clamp(intensity, 0.0, 1.0));
        if (phenomenon < static_cast<std::uint8_t>(PerformanceRelationalPhenomenon::Count))
            cablePhenomenon_[a][b] = phenomenon;
    }
    double cable(std::size_t a, std::size_t b) const noexcept {
        return a < kBodies && b < kBodies ? cable_[a][b] : 0.0;
    }
    // O ato musical que cada fenômeno pede (kNoPhenomenon e LIBERAR: o ato
    // natural do par).
    static RelationKind phenomenonKind(std::uint8_t phenomenon, RelationKind natural) noexcept {
        using P = PerformanceRelationalPhenomenon;
        switch (static_cast<P>(phenomenon)) {
        case P::Attract: case P::Parallel: case P::Converge: case P::Equilibrium: case P::Resonate:
            return RelationKind::Support;
        case P::Repel: return RelationKind::Friction;
        case P::Orbit: case P::EllipticalOrbit: case P::Oscillate: case P::Swap:
            return RelationKind::CallResponse;
        case P::BarycentricDance: case P::Encounter: case P::Merge: case P::Lock: case P::Synchronize:
            return RelationKind::Contagion;
        case P::Flyby: case P::Chase: case P::SpiralIn: case P::SpiralOut:
            return RelationKind::Shadow;
        case P::Scatter: case P::Escape: case P::Diverge:
            return RelationKind::Hocket;
        default: return natural;
        }
    }
    // A força física do fenômeno entre a (quem chama) e b, somada às
    // acelerações dos dois: elásticos, repulsão, giro, perseguição/fuga,
    // alinhamento. `strength` é a intensidade do cabo.
    static void phenomenonForce(std::uint8_t phenomenon, double strength,
                                const SkyBodyState& a, const SkyBodyState& b,
                                double& aax, double& aay, double& bax, double& bay) noexcept {
        using P = PerformanceRelationalPhenomenon;
        const double dx = b.x - a.x;
        const double dy = b.y - a.y;
        const double distance = std::max(std::sqrt(dx * dx + dy * dy), 0.02);
        const double ux = dx / distance;
        const double uy = dy / distance;
        const auto spring = [&](double rest, double stiffness) {
            const double pull = (distance - rest) * stiffness * strength;
            aax += pull * ux; aay += pull * uy;
            bax -= pull * ux; bay -= pull * uy;
        };
        const auto push = [&](double amount) {
            const double force = amount * strength / (distance * distance);
            aax -= force * ux; aay -= force * uy;
            bax += force * ux; bay += force * uy;
        };
        const auto swirl = [&](double amount) {
            // Força tangencial: os dois giram em torno do meio do par.
            aax += -uy * amount * strength; aay += ux * amount * strength;
            bax -= -uy * amount * strength; bay -= ux * amount * strength;
        };
        switch (static_cast<P>(phenomenon)) {
        case P::Attract: case P::Converge: case P::Merge: case P::Encounter: spring(0.07, 2.4); break;
        case P::Repel: case P::Diverge: case P::Scatter: push(0.0035); break;
        case P::Orbit: spring(0.18, 1.2); swirl(0.35); break;
        case P::EllipticalOrbit: spring(0.22, 0.7); swirl(0.45); break;
        case P::BarycentricDance: spring(0.14, 1.6); swirl(0.25); break;
        case P::SpiralIn: spring(0.05, 0.8); swirl(0.4); break;
        case P::SpiralOut: push(0.0015); swirl(-0.4); break;
        case P::Oscillate: case P::Resonate: spring(0.26, 3.0); break;
        case P::Lock: case P::Synchronize: spring(0.12, 6.0); break;
        case P::Chase: aax += ux * 0.5 * strength; aay += uy * 0.5 * strength; break;
        case P::Escape: bax += ux * 0.5 * strength; bay += uy * 0.5 * strength; break;
        case P::Parallel:
            aax += (b.vx - a.vx) * 1.5 * strength; aay += (b.vy - a.vy) * 1.5 * strength;
            bax += (a.vx - b.vx) * 1.5 * strength; bay += (a.vy - b.vy) * 1.5 * strength;
            break;
        case P::Flyby: swirl(0.2); break;
        default: break;  // EQUILÍBRIO, TROCAR, LIBERAR: só o ato musical
        }
    }

    // O movimento da constelação inteira (30/10): GIRA — os lugares de
    // repouso rodam em torno do centro da constelação, uma volta a cada 16
    // compassos (sentido da seed); VIAJA — o centro passeia numa órbita larga
    // pelo campo, uma volta a cada 24 compassos. As estrelas seguem os
    // lugares pela física (inércia, molas, choques). FIXA: o giro e a viagem
    // voltam devagar a zero — o desenho é memória autoral.
    void setConstellationMotion(PerformanceConstellationMotion motion) noexcept { constellationMotion_ = motion; }
    PerformanceConstellationMotion constellationMotion() const noexcept { return constellationMotion_; }
    double constellationAngle() const noexcept { return rotationAngle_; }

    // Famílias de movimento (30/10). Prioridade: a escolhida para o corpo; a
    // escolhida para o campo; a que vem do campo de órbita antigo (ÓRBITA →
    // LIVRE, PULSO → ESPUMA, FIGURA-8 → PÊNDULOS), para que esse comando
    // continue audível. `Count` = não escolhida.
    void setCollectiveFamily(PerformanceMotionFamily family) noexcept { collectiveFamily_ = family; }
    void setBodyFamily(std::size_t slot, PerformanceMotionFamily family) noexcept {
        if (slot < kBodies) bodyFamily_[slot] = family;
    }
    // A forma encaixada numa duração (2/11, o dispositivo de 1/2/3/5 min):
    // as quatro primeiras seções — abertura, desenvolvimento, contraste,
    // retorno — com os compassos dados, e a forma recomeça na abertura. Todos
    // zero desfazem o plano (a forma volta aos 8–20 compassos da seed). Como
    // no Studio, a forma da peça se ajusta à duração pedida.
    void setFormPlan(const std::array<std::uint32_t, 4>& bars) noexcept {
        formPlanned_ = bars[0] + bars[1] + bars[2] + bars[3] > 0U;
        formPlan_ = bars;
        if (!formPlanned_) return;
        planGrammar(bars[0] + bars[1] + bars[2] + bars[3]);
        section_ = Section::Opening;
        sectionOrdinal_ = 0U;
        formStep_ = 0U;
        formPass_ = 0U;
        sectionBarsLeft_ = sectionLength(0U);
        characterReady_ = false;
        stepCharacter();
    }
    bool formPlanned() const noexcept { return formPlanned_; }

    // O caráter de cada parte da forma (4/11, pedido do autor: "contrastes de
    // modo generativo, sem padrões que se repitam em faixas diferentes"). Não
    // há cardápio de gestos: cada parte é um ponto num espaço contínuo de
    // nove dimensões em [0, 1] — quantas estrelas soam, quanto elas tocam, o
    // andamento e o swing sentidos, e o peso de cada família (tom, textura,
    // voz, pulso, controle) no elenco. A seed desenha o caminho: a abertura é
    // um ponto qualquer; o desenvolvimento dá um passo curto; o contraste,
    // um salto longo que rebate nas bordas (nunca fica perto); o retorno cai
    // entre a abertura e o contraste. Os ciclos seguintes continuam o
    // caminho. Mesma ideia leve dos genes do Studio e da "mão" do Modular:
    // poucas restrições, o resto em movimento.
    static constexpr std::size_t kCharacterDims = 9U;
    struct SectionCharacter {
        // 0 estrelas, 1 atividade, 2 andamento, 3 swing, 4..8 famílias
        std::array<double, kCharacterDims> v{{0.4, 0.7, 0.5, 0.3, 0.6, 0.6, 0.6, 0.6, 0.15}};
        // A densidade da seed (6/11, pedido do autor: "faça a densidade como
        // gene, mas desejo muitas possibilidades"): a faixa de estrelas acesas
        // e a faixa de atividade são da seed; o caráter de cada parte anda
        // dentro delas. Valores de antes: 2–7 estrelas, atividade 0,35–1,6.
        double litLow = 2.0, litHigh = 7.0;
        double activityLow = 0.35, activityHigh = 1.6;
        double chorus = 0.0;  // chamadas extras por tempo às acesas (gene, 6/11)
        std::size_t litTarget() const noexcept {
            return static_cast<std::size_t>(std::lround(litLow + (litHigh - litLow) * v[0]));
        }
        // Abaixo de 1 a porta rareia os ataques; acima de 1, o céu chama as
        // estrelas acesas a tocar mais.
        double rawActivity() const noexcept { return activityLow + (activityHigh - activityLow) * v[1]; }
        double activity() const noexcept { return std::min(1.0, rawActivity()); }
        double densify() const noexcept { return std::max(0.0, rawActivity() - 1.0) + chorus * (0.5 + v[1]); }
        // A energia de uma parte: quantas estrelas soam e quanto tocam.
        double energy() const noexcept { return 0.5 * (v[0] + v[1]); }
        // ±6 %, a mesma ordem da variação antiga (0,95–1,05): a faixa com
        // duração calcula os compassos pelo BPM do controle.
        double tempo() const noexcept { return 0.94 + 0.12 * v[2]; }
        double swing() const noexcept { return 0.5 + 0.14 * v[3]; }
        double family(PerformanceStarFunction f) const noexcept {
            return v[4U + std::min<std::size_t>(static_cast<std::size_t>(f), 4U)];
        }
    };
    const SectionCharacter& character() const noexcept { return character_; }
    // A liderança que gira (4/11, "meio homogêneo": uma estrela levava 66–90 %
    // da energia em todas as partes). A ecologia soma aqui a energia de cada
    // estrela; ao trocar de parte, quem liderou a parte que acabou recua na
    // proporção do salto do caráter (muito no contraste, pouco num
    // desenvolvimento curto); no retorno, o líder da abertura volta.
    void addStemEnergy(std::size_t slot, double energy) noexcept {
        if (slot < kBodies && std::isfinite(energy)) sectionEnergy_[slot] += energy;
    }
    std::size_t recedingLeader() const noexcept { return recedeSlot_; }
    double recedeGain() const noexcept { return recedeGain_; }
    // Quanto as estrelas tocam agora (a fração de ataques que passa), com a
    // trama; sem ela, tudo passa.
    double activity() const noexcept { return influence_ > 0.0 ? character_.activity() : 1.0; }
    static double characterDistance(const SectionCharacter& a, const SectionCharacter& b) noexcept {
        double sum = 0.0;
        for (std::size_t i = 0U; i < kCharacterDims; ++i) {
            const double w = i < 4U ? 1.0 : 0.5;
            sum += w * (a.v[i] - b.v[i]) * (a.v[i] - b.v[i]);
        }
        return std::sqrt(sum);
    }
    // Uma estrela fixada pelo performer (2/11) fica acesa: a VARIAÇÃO e os
    // eclipses não a apagam.
    void setPinned(std::size_t slot, bool pinned) noexcept {
        if (slot < kBodies) pinned_[slot] = pinned;
    }
    bool pinned(std::size_t slot) const noexcept { return slot < kBodies && pinned_[slot]; }
    void setFieldFamily(std::size_t slot, PerformanceMotionField field) noexcept {
        if (slot >= kBodies) return;
        fieldFamily_[slot] = field == PerformanceMotionField::Pulse ? PerformanceMotionFamily::Foam
                           : field == PerformanceMotionField::FigureEight ? PerformanceMotionFamily::Pendulum
                           : PerformanceMotionFamily::Free;
    }
    PerformanceMotionFamily family(std::size_t slot) const noexcept {
        if (slot >= kBodies) return PerformanceMotionFamily::Free;
        if (bodyFamily_[slot] != PerformanceMotionFamily::Count) return bodyFamily_[slot];
        if (collectiveFamily_ != PerformanceMotionFamily::Count) return collectiveFamily_;
        // Sem escolha do músico, a família de cada parte vem da seed (5/11).
        if (fieldFamily_[slot] == PerformanceMotionFamily::Free && influence_ > 0.0)
            return sectionFamily_[std::min<std::size_t>(static_cast<std::size_t>(section_), 3U)];
        return fieldFamily_[slot];
    }
    // O que cada família faz às forças da estrela (multiplicadores sobre a
    // coreografia da seção; `stadium`, `magnetFloor` e `pendulum` impõem).
    struct FamilyShape {
        double anchor = 1.0, damping = 1.0, flow = 1.0, magnet = 1.0, kick = 1.0;
        double bubble = 1.0, gravity = 1.0, stadium = 0.0, magnetFloor = 0.0;
        bool pendulum = false;
    };
    static FamilyShape familyShape(PerformanceMotionFamily family) noexcept {
        switch (family) {
        // BILHAR: pouco atrito e pouca âncora, impulsos fortes, estádio.
        case PerformanceMotionFamily::Billiard: return {0.25, 0.25, 0.3, 0.3, 2.2, 0.6, 1.0, 1.0, 0.0, false};
        // ESPUMA: bolhas grandes e viscosas que se empurram e se arrumam.
        case PerformanceMotionFamily::Foam: return {0.7, 2.0, 0.6, 0.2, 0.8, 1.9, 0.5, 0.0, 0.0, false};
        // PÊNDULOS: a estrela pende do seu lugar e balança.
        case PerformanceMotionFamily::Pendulum: return {1.0, 0.4, 0.2, 0.3, 1.3, 1.0, 1.0, 0.0, 0.0, true};
        // MAGNÉTICO: campo forte e sempre presente — espirais.
        case PerformanceMotionFamily::Magnetic: return {0.6, 0.5, 0.5, 1.6, 1.0, 1.0, 1.0, 0.0, 1.0, false};
        // GRAVIDADE: atração forte — aglomerados que se formam e desfazem.
        case PerformanceMotionFamily::Gravity: return {0.6, 0.8, 0.5, 0.5, 1.0, 0.8, 2.0, 0.0, 0.0, false};
        default: return {};
        }
    }

    // O corpo que deriva neste desenvolvimento (kBodies: nenhum).
    std::size_t driftSlot() const noexcept { return influence_ > 0.0 ? driftSlot_ : kBodies; }
    double chorusDelay(std::size_t slot) const noexcept {
        if (influence_ <= 0.0 || slot >= kBodies) return 0.0;
        return chorusPhase_[slot] / kChorusTwoPi;
    }
    // A defasagem que cada relação pede entre quem chama e quem responde
    // (em radianos: 2π é um passo inteiro).
    static constexpr double chorusLag(RelationKind kind) noexcept {
        switch (kind) {
        case RelationKind::CallResponse:
        case RelationKind::Hocket: return kChorusTwoPi * 0.5;
        case RelationKind::Shadow:
        case RelationKind::Friction: return kChorusTwoPi * 0.25;
        default: return 0.0;
        }
    }
    // Um passo do modelo de Kuramoto com defasagens: cada fase anda na sua
    // frequência própria, é puxada fracamente para o pulso comum (âncora em
    // θ0) e, em cada par acoplado, para a defasagem pedida pela relação:
    // dθa/dt = ωa − K0·sen(θa − θ0) + K·sen(θb − θa − φ),
    // dθb/dt = ωb − K0·sen(θb − θ0) + K·sen(θa − θb + φ).
    // O pulso comum fica em θ0 = 0,1 passo (e não em 0) para que inclinar-se
    // um pouco à frente seja um atraso menor, e não quase um passo inteiro.
    template <std::size_t N>
    static void kuramotoStep(std::array<double, N>& phase, const std::array<double, N>& omega,
                             const std::size_t* callers, const std::size_t* responders, const double* lags,
                             std::size_t pairs, double coupling, double anchor, double dt,
                             const std::array<double, N>* restShift = nullptr) noexcept {
        std::array<double, N> rate{};
        for (std::size_t i = 0U; i < N; ++i)
            rate[i] = omega[i] - anchor * std::sin(phase[i] - kChorusRest - (restShift != nullptr ? (*restShift)[i] : 0.0));
        for (std::size_t p = 0U; p < pairs; ++p) {
            const auto a = callers[p];
            const auto b = responders[p];
            if (a >= N || b >= N || a == b) continue;
            rate[a] += coupling * std::sin(phase[b] - phase[a] - lags[p]);
            rate[b] += coupling * std::sin(phase[a] - phase[b] + lags[p]);
        }
        for (std::size_t i = 0U; i < N; ++i) {
            phase[i] += rate[i] * dt;
            phase[i] -= kChorusTwoPi * std::floor(phase[i] / kChorusTwoPi);
        }
    }
    // O swing como órbita: excentricidade (0 = reto) e periélio (qual passo
    // do tempo aperta) para o relógio orbital do céu.
    // A primeira metade do tempo vale ~0,5 + 0,63·e (e = 0,1 → 56%, 0,2 → 63%,
    // 0,36 → 72%): a excentricidade é escolhida para dar o swing pedido.
    double eccentricity() const noexcept { return std::clamp((swing_ - 0.5) / 0.63, 0.0, 0.36); }
    // Ângulo verdadeiro no início do tempo. Começando num ápside, cada metade
    // da órbita dura exatamente meio período (simetria) e não há swing; a π/2
    // a primeira metade atravessa o afélio (lenta): colcheia longa, curta.
    // Uma seed em quatro usa 3π/2: o swing invertido.
    double perihelion() const noexcept {
        return (seed_ & 3U) == 0U ? 4.71238898038469 : 1.5707963267948966;
    }
    // Tensão harmônica acumulada pelo atrito, 0..1.
    double tension() const noexcept { return tension_; }

    // Variação do mixer (pedido do autor, 28/10): com ela ligada, a seed
    // escolhe quais estrelas abrem acesas e, a cada compasso, a dança do
    // brilho apaga, reacende e substitui estrelas. Desligada, todo corpo com
    // fader acima de zero brilha inteiro. O fader continua sendo do músico:
    // o brilho só multiplica por cima dele.
    void setMixerVariation(bool enabled) noexcept { mixerVariation_ = enabled; }
    // Guarda de capacidade: quantas estrelas a máquina sustenta acesas ao
    // mesmo tempo, medido pelo app no pior bloco real. A variação do mixer
    // nunca acende além disso e apaga primeiro quem tem menos vida; os
    // faders do músico não são bloqueados.
    void setStarCapacity(std::size_t capacity) noexcept {
        starCapacity_ = std::clamp<std::size_t>(capacity, 1U, kBodies);
    }
    std::size_t starCapacity() const noexcept { return starCapacity_; }
    bool mixerVariation() const noexcept { return mixerVariation_; }
    // O músico subiu o fader de um corpo: ele acende e fica aceso por um tempo.
    void holdLit(std::size_t slot) noexcept {
        if (slot >= kBodies) return;
        brightnessTarget_[slot] = 1.0;
        heldBars_[slot] = 8U;
        restBars_[slot] = 0U;
    }
    // Brilho suavizado de um corpo, 0..1, por amostra.
    double brightness(std::size_t slot) const noexcept { return slot < kBodies ? brightness_[slot] : 1.0; }
    double level(std::size_t slot) const noexcept { return slot < kBodies ? level_[slot] : 1.0; }
    double brightnessTarget(std::size_t slot) const noexcept {
        return slot < kBodies && !pinned_[slot] ? brightnessTarget_[slot] : 1.0;
    }
    void setInfluence(double influence) noexcept {
        influence_ = std::isfinite(influence) ? std::clamp(influence, 0.0, 1.0) : 0.0;
    }
    double influence() const noexcept { return influence_; }
    // O empurrão do ABRIR (3/11, R7): o "grão" do token do gesto escolhe a
    // direção; a intensidade, a força (≈ 3 ataques). Sem a trama, nada.
    // slot = kBodies empurra todas as acesas, cada uma num ângulo (áureo).
    void push(std::size_t slot, std::uint8_t grain, double intensity) noexcept {
        if (influence_ <= 0.0 || !std::isfinite(intensity)) return;
        const double base = static_cast<double>(grain) / 256.0 * 6.283185307179586;
        for (std::size_t body = 0U; body < kBodies; ++body) {
            if (slot < kBodies ? body != slot : !bodies_[body].lit) continue;
            const double angle = base + (slot < kBodies ? 0.0 : static_cast<double>(body) * 2.399963229728653);
            const double impulse = kKick * 3.0 * std::clamp(intensity, 0.0, 1.0) / massOf(bodies_[body]);
            vx_[body] += std::cos(angle) * impulse;
            vy_[body] += std::sin(angle) * impulse;
        }
    }

    // Chamado uma vez por amostra, antes das estrelas, com o estado dos corpos
    // e o andamento. Roda o relógio de controle quando é a vez.
    void advance(const std::array<Body, kBodies>& bodies, double tempoBpm) noexcept {
        ++sample_;
        // O tempo elástico: os tempos do céu correm no andamento-base vezes o
        // fator elástico (1 com a trama desligada).
        const double beatsPerSample = std::clamp(tempoBpm * tempoFactor_, 20.0, 300.0) / 60.0 / sampleRate_;
        // Entradas e saídas de brilho levam cerca de dois tempos.
        const double fade = std::min(1.0, beatsPerSample * 0.5);
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            double target = mixerVariation_ ? brightnessTarget_[slot] * level_[slot] : 1.0;
            // O plano de cada parte (4/11, "meio homogêneo"): a família forte no
            // caráter da parte vem para a frente, as outras recuam (até 45 %);
            // muda de parte para parte, então quem lidera também muda.
            if (mixerVariation_ && characterReady_ && influence_ > 0.0) target *= foregroundOf(slot);
            if (mixerVariation_ && influence_ > 0.0 && slot == recedeSlot_) target *= recedeGain_;
            // O sol encoberto escurece durante o eclipse.
            if (mixerVariation_ && eclipseBeatsLeft_ > 0U && slot == eclipseSun_) target *= 0.15;
            if (pinned_[slot]) target = 1.0;
            brightness_[slot] += (target - brightness_[slot]) * fade;
        }
        if (influence_ <= 0.0) {
            tempoFactor_ = 1.0;
            springPosition_ = 1.0;
            tempoVelocity_ = 0.0;
            swing_ = 0.5;
            groove_ = 0.0;
            if (!mixerVariation_) return;
        }
        beatPhase_ += beatsPerSample;
        bool barTurn = false;
        if (beatPhase_ >= 1.0) {
            beatPhase_ -= std::floor(beatPhase_);
            ++beat_;
            barTurn = (beat_ % 4U) == 0U;
            if (eclipseBeatsLeft_ > 0U) --eclipseBeatsLeft_;
            if (influence_ > 0.0) watchTheMoon();
            if (influence_ > 0.0 && brightnessReady_) densifyBeat();
        }
        // O anel de semicolcheias do Fourier do ritmo: ao entrar numa nova
        // casa, esquece o que estava nela há três compassos.
        const auto sixteenth = beat_ * 4U + static_cast<std::uint64_t>(std::min(beatPhase_ * 4.0, 3.0));
        if (sixteenth != sixteenth_) {
            sixteenth_ = sixteenth;
            const auto clear = ~(std::uint64_t{1} << (sixteenth_ % kRhythmRing));
            for (auto& mask : onsets_) mask &= clear;
        }
        if (countdown_ > 0U) {
            --countdown_;
            if (!barTurn) return;
        }
        countdown_ = kControlInterval - 1U;
        bodies_ = bodies;
        if (barTurn) {
            advanceForm();
            checkSkyGesture();
        }
        if (mixerVariation_) {
            if (barTurn || !brightnessReady_) danceBrightness(barTurn);
            mixByConstellation();
        }
        if (influence_ <= 0.0) {
            tempoFactor_ = 1.0;
            springPosition_ = 1.0;
            tempoVelocity_ = 0.0;
            swing_ = 0.5;
            groove_ = 0.0;
            return;
        }
        stepElasticTime(static_cast<double>(kControlInterval) / sampleRate_);
        if (!positionsReady_) placeAtAnchors();
        stepGravity(static_cast<double>(kControlInterval) / sampleRate_);
        updateWeb();
        if (barTurn)
            for (std::size_t slot = 0U; slot < kBodies; ++slot) rhythm_[slot] = dominantPeriod(onsets_[slot]);
        if (barTurn || focusCount_ == 0U) chooseFocus();
        stepChorus(static_cast<double>(kControlInterval) / sampleRate_);
        samplesPerBeat_ = 1.0 / beatsPerSample;
    }

    // As estrelas que atacaram nesta amostra são ouvidas pelo céu.
    void hear(std::size_t slot, double pitchHz) noexcept {
        if (influence_ <= 0.0 || slot >= kBodies || !std::isfinite(pitchHz) || pitchHz <= 0.0) return;
        // O motivo da estrela: as últimas alturas e o ritmo entre elas.
        const double ioiBeats = lastAttackSample_[slot] > 0U
            ? std::clamp(static_cast<double>(sample_ - lastAttackSample_[slot]) / samplesPerBeat_, 0.125, 2.0)
            : 0.5;
        auto& motif = motif_[slot];
        motif[motifHead_[slot]] = {pitchHz, ioiBeats};
        motifHead_[slot] = static_cast<std::uint8_t>((motifHead_[slot] + 1U) % kMotifLength);
        motifCount_[slot] = static_cast<std::uint8_t>(std::min<std::size_t>(motifCount_[slot] + 1U, kMotifLength));
        previousAttackSample_[slot] = lastAttackSample_[slot];
        lastAttackSample_[slot] = sample_;
        onsets_[slot] |= std::uint64_t{1} << (sixteenth_ % kRhythmRing);
        heardPitch_[slot] = pitchHz;
        ++attackCount_[slot];
        kickOnAttack(slot);
        // Um ataque que é ele mesmo resposta carrega a profundidade da cadeia.
        const auto depth = static_cast<std::uint8_t>(answerDepth_[slot] + 1U);
        answerDepth_[slot] = 0U;
        if (depth > kMaxChainDepth) return;
        for (std::size_t index = 0U; index < focusCount_; ++index) {
            const auto& focus = focus_[index];
            if (focus.caller == slot) respond(focus, pitchHz, depth);
        }
    }

    // Entrega um chamado vencido, se houver. Chamar em laço até falso.
    bool popDue(Cue& due) noexcept {
        for (std::size_t index = 0U; index < cueCount_; ++index) {
            if (cues_[index].dueSample > sample_) continue;
            due = cues_[index];
            cues_[index] = cues_[--cueCount_];
            answerDepth_[due.slot] = due.depth;
            return true;
        }
        return false;
    }

    // Telemetria e testes.
    double positionX(std::size_t slot) const noexcept { return slot < kBodies ? x_[slot] : 0.0; }
    // O lugar na tela (o espaço deformado pela matriz móvel) e o giro.
    double shownX(std::size_t slot) const noexcept { return slot < kBodies ? shownX_[slot] : 0.5; }
    double shownY(std::size_t slot) const noexcept { return slot < kBodies ? shownY_[slot] : 0.5; }
    double spin(std::size_t slot) const noexcept { return slot < kBodies ? spin_[slot] : 0.0; }
    bool positionsReady() const noexcept { return positionsReady_ && influence_ > 0.0; }
    // Choques audíveis e voltas completas do pêndulo até aqui.
    std::uint32_t collisionCount() const noexcept { return collisionCount_; }
    std::uint32_t pendulumTurns() const noexcept { return pendulumTurns_; }
    // Quantas vezes o retorno trouxe o motivo da abertura.
    std::uint32_t recapitulations() const noexcept { return recapitulations_; }
    std::size_t pendulumChainLength() const noexcept {
        std::size_t length = 0U;
        for (const auto slot : chain_) length += slot < kBodies ? 1U : 0U;
        return influence_ > 0.0 ? length : 0U;
    }
    double positionY(std::size_t slot) const noexcept { return slot < kBodies ? y_[slot] : 0.0; }
    double intensity(std::size_t a, std::size_t b) const noexcept {
        return a < kBodies && b < kBodies ? intensity_[a][b] : 0.0;
    }
    RelationKind kind(std::size_t a, std::size_t b) const noexcept { return kindFor(a, b); }
    std::size_t focusCount() const noexcept { return focusCount_; }
    const Focus& focus(std::size_t index) const noexcept { return focus_[index < kMaxFocus ? index : 0U]; }
    std::size_t pendingCues() const noexcept { return cueCount_; }

private:
    // A coreografia (29/10): o peso de cada força por seção. A abertura
    // reúne devagar; o desenvolvimento gira pares (elásticos, magnetismo,
    // pêndulos); o contraste espalha com energia (bilhar no estádio, a pista
    // de correntes forte); o retorno assenta nos lugares desenhados. Os
    // pesos mudam devagar, sem degrau.
    struct Choreography {
        double anchor = 4.0;     // mola ao lugar desenhado
        double damping = 1.2;    // atrito do ar
        double flow = 0.0;       // arrasto da pista de correntes
        double magnet = 0.0;     // campo magnético
        double stadium = 0.0;    // paredes do estádio
        double bands = 0.0;      // elásticos das relações em foco
        double kick = 0.5;       // impulso de cada ataque
        double spin = 0.4;       // efeito Magnus (curvas)
        double foam = 0.5;       // pressão das bolhas
        double epicycle = 0.04;  // raio do passeio do lugar de repouso
        double distortion = 0.2; // a matriz móvel que deforma o espaço
    };
    // A seed escolhe a física de cada parte (5/11, auditoria da seed, item
    // B): uma família de movimento por parte (antes, só o músico, pela DANÇA)
    // e um desvio da coreografia em torno da tabela (cada peso ×0,7–1,35) —
    // a tabela vira centro, não regra; o contraste deixa de ser sempre o
    // mesmo bilhar no estádio.
    void chooseSeedMotion() noexcept {
        constexpr auto families = static_cast<std::uint32_t>(PerformanceMotionFamily::Count);
        for (std::size_t section = 0U; section < sectionFamily_.size(); ++section) {
            sectionFamily_[section] = static_cast<PerformanceMotionFamily>(
                performanceSeedStream(seed_, 0xF000U + static_cast<std::uint32_t>(section)) % families);
            for (std::size_t weight = 0U; weight < choreographyScale_[section].size(); ++weight)
                choreographyScale_[section][weight] = 0.7 + 0.65 * static_cast<double>(
                    performanceSeedStream(seed_, 0xF100U + static_cast<std::uint32_t>(section * 16U + weight))
                    & 0xffffU) / 65535.0;
        }
    }
    Choreography seededChoreography(Section section) const noexcept {
        auto c = choreographyFor(section);
        const auto& k = choreographyScale_[std::min<std::size_t>(static_cast<std::size_t>(section), 3U)];
        c.anchor *= k[0]; c.damping *= k[1]; c.flow *= k[2]; c.magnet *= k[3]; c.stadium *= k[4];
        c.bands *= k[5]; c.kick *= k[6]; c.spin *= k[7]; c.foam *= k[8]; c.epicycle *= k[9];
        c.distortion *= k[10];
        return c;
    }
    static Choreography choreographyFor(Section section) noexcept {
        switch (section) {
        case Section::Opening: return {3.2, 1.3, 0.35, 0.0, 0.0, 0.6, 0.6, 0.4, 0.6, 0.04, 0.2};
        case Section::Development: return {1.6, 0.7, 0.7, 1.0, 0.0, 1.6, 1.0, 1.0, 0.3, 0.06, 0.5};
        case Section::Contrast: return {0.55, 0.32, 1.3, 0.5, 1.0, 0.6, 1.6, 1.4, 1.0, 0.03, 1.0};
        case Section::Return: return {4.5, 1.6, 0.2, 0.0, 0.0, 0.4, 0.5, 0.3, 0.4, 0.03, 0.1};
        default: return {};
        }
    }

    // Atração suave e poço firme: a dança desloca as estrelas alguns
    // centésimos do Campo sem que pares próximos se capturem.
    static constexpr double kGravity = 0.004;
    static constexpr double kSoftening = 0.12;
    // O movimento físico (29/10; PerformanceSkyMotion.hpp).
    static constexpr double kFoamStiffness = 40.0;
    static constexpr double kMagnus = 0.9;
    static constexpr double kSpinDecay = 0.15;
    static constexpr double kServeSpin = 1.2;
    static constexpr double kGrip = 0.3;
    static constexpr double kEpicycleOmega = 0.21;
    static constexpr double kGolden = 1.618033988749895;
    // φ^n e φ^(n/2), para não chamar pow a cada passo.
    static constexpr std::array<double, 5> kGoldenPowers{{1.0, kGolden, kGolden * kGolden,
                                                          kGolden * kGolden * kGolden, kGolden * kGolden * kGolden * kGolden}};
    static constexpr std::array<double, 4> kGoldenRoots{{1.0, 1.272019649514069, kGolden, 2.058171027271492}};
    static constexpr double kContact = 0.05;
    static constexpr double kAudibleImpact = 0.18;
    static constexpr double kMaxSpeed = 0.9;
    static constexpr double kFlowSpeed = 0.22;
    static constexpr double kFlowDrag = 1.6;
    static constexpr double kMagnetField = 3.0;
    static constexpr double kStadiumHalf = 0.16;
    static constexpr double kStadiumRadius = 0.40;
    static constexpr double kStadiumPush = 60.0;
    static constexpr double kBandStiffness = 2.2;
    static constexpr double kKick = 0.05;
    // Pêndulo elástico na ressonância 2:1 (mola com o dobro da frequência
    // do balanço: k = 4·g/L).
    static constexpr double kPendulumRest = 0.13;
    static constexpr double kPendulumGravity = 1.1;
    static constexpr double kPendulumStiffness = 4.0 * kPendulumGravity / kPendulumRest;
    Choreography dance_{};
    double motionTime_ = 0.0;
    sky_motion::FlowShape flowShape_{};
    double flowPhase_ = 0.0;
    static constexpr std::size_t kChainLength = 3U;
    std::array<std::size_t, kChainLength> chain_{{kBodies, kBodies, kBodies}};
    std::array<bool, kChainLength> pendulumOver_{};
    std::array<double, kBodies> charge_{};
    PerformanceMotionFamily collectiveFamily_ = PerformanceMotionFamily::Count;
    std::array<PerformanceMotionFamily, 4> sectionFamily_{};
    std::array<std::array<double, 11>, 4> choreographyScale_{{
        {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}, {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
        {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}, {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}}};
    PerformanceConstellationMotion constellationMotion_ = PerformanceConstellationMotion::Still;
    double rotationAngle_ = 0.0;
    double rotationSign_ = 1.0;
    double travelPhase_ = 0.0;
    double travelAmount_ = 0.0;
    std::array<PerformanceMotionFamily, kBodies> bodyFamily_ = [] {
        std::array<PerformanceMotionFamily, kBodies> none{};
        none.fill(PerformanceMotionFamily::Count);
        return none;
    }();
    std::array<PerformanceMotionFamily, kBodies> fieldFamily_{};
    std::array<std::array<double, kBodies>, kBodies> cable_{};
    std::array<std::array<std::uint8_t, kBodies>, kBodies> cablePhenomenon_ = [] {
        std::array<std::array<std::uint8_t, kBodies>, kBodies> none{};
        for (auto& row : none) row.fill(kNoPhenomenon);
        return none;
    }();
    std::array<double, kBodies> spin_{};
    std::array<double, kBodies> bubble_{};
    std::uint32_t collisionCount_ = 0U;
    std::uint32_t pendulumTurns_ = 0U;
    std::array<double, kBodies> restX_{};
    std::array<double, kBodies> restY_{};
    std::array<double, kBodies> shownX_{};
    std::array<double, kBodies> shownY_{};

    double sampleRate_ = 48000.0;
    std::uint32_t seed_ = 181U;
    double influence_ = 1.0;
    std::uint64_t sample_ = 0U;
    std::uint32_t countdown_ = 0U;
    double beatPhase_ = 0.0;
    std::uint32_t beat_ = 0U;
    double samplesPerBeat_ = 24000.0;
    std::array<Body, kBodies> bodies_{};
    std::array<double, kBodies> x_{};
    std::array<double, kBodies> y_{};
    std::array<double, kBodies> vx_{};
    std::array<double, kBodies> vy_{};
    bool positionsReady_ = false;
    std::array<std::array<double, kBodies>, kBodies> intensity_{};
    std::array<std::array<double, kBodies>, kBodies> recentFocus_{};
    std::array<Focus, kMaxFocus> focus_{};
    std::size_t focusCount_ = 0U;
    std::array<Cue, kCueCapacity> cues_{};
    std::size_t cueCount_ = 0U;
    std::array<std::uint64_t, kBodies> lastAttackSample_{};
    std::array<std::uint64_t, kBodies> previousAttackSample_{};
    std::array<double, kBodies> heardPitch_{};
    std::array<std::uint32_t, kBodies> attackCount_{};
    bool mixerVariation_ = false;
    std::size_t starCapacity_ = kBodies;
    bool brightnessReady_ = false;
    std::array<double, kBodies> brightness_{{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
    std::array<double, kBodies> brightnessTarget_{{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
    std::array<double, kBodies> level_{{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
    std::array<std::uint32_t, kBodies> litBars_{};
    std::array<std::uint32_t, kBodies> restBars_{};
    std::array<std::uint32_t, kBodies> heldBars_{};
    std::array<std::uint64_t, kBodies> lastCueSample_{};
    static constexpr std::size_t kMotifLength = 8U;
    struct MotifNote {
        double pitchHz = 0.0;
        double ioiBeats = 0.5;
    };
    std::array<std::array<MotifNote, kMotifLength>, kBodies> motif_{};
    std::array<MotifNote, kMotifLength> openingMotif_{};
    std::size_t openingCount_ = 0U;
    std::size_t openingSlot_ = kBodies;
    std::uint32_t recapitulations_ = 0U;
    std::array<std::uint8_t, kBodies> motifHead_{};
    std::array<std::uint8_t, kBodies> motifCount_{};
    std::array<std::array<std::uint32_t, kBodies>, kBodies> pairResponses_{};
    double tension_ = 0.0;
    double tonalCenterHz_ = 110.0;
    PerformanceGameStrategy strategy_ = PerformanceGameStrategy::Follow;
    Section section_ = Section::Opening;
    double tempoFactor_ = 1.0;
    double tempoVelocity_ = 0.0;
    double swing_ = 0.5;
    double groove_ = 0.0;
    static constexpr double kChorusTwoPi = 6.283185307179586;
    static constexpr double kChorusRest = kChorusTwoPi * 0.1;
    static constexpr double kChorusAnchor = 0.35;
    std::size_t driftSlot_ = kBodies;
    double synodicOffset_ = 0.0;
    double draconicOffset_ = 0.0;
    std::uint64_t lunation_ = 0U;
    std::size_t eclipseSun_ = kBodies;
    std::array<bool, kBodies> pinned_{};
    std::array<std::uint32_t, 4> formPlan_{};
    std::uint8_t grammar_ = 0U;
    std::uint32_t sectionBars_ = 0U;
    std::array<std::uint32_t, 2> gestureBar_{{0U, 0U}};
    std::uint32_t gestureBudget_ = 0U;
    bool gesturePending_ = false;
    SkyGesture pendingGesture_{};
    std::uint32_t skyGestures_ = 0U;
    int contrastRegion_ = 0;
    int developmentRegion_ = 0;
    std::uint8_t formStep_ = 0U;
    std::uint32_t formPass_ = 0U;
    std::array<std::uint32_t, 8> plannedBars_{};
    std::uint8_t plannedParts_ = 0U;
    bool formPlanned_ = false;
    std::uint32_t eclipseBeatsLeft_ = 0U;
    std::uint32_t eclipseCount_ = 0U;
    std::array<std::uint64_t, kBodies> onsets_{};
    std::array<RhythmPeriod, kBodies> rhythm_{};
    std::uint64_t sixteenth_ = 0U;
    double driftRatio_ = 1.0;
    std::array<double, kBodies> chorusPhase_{};
    std::array<double, kBodies> chorusOmega_{};

    // O andamento como sistema dinâmico. Uma mola amortecida de segunda ordem
    // (x'' = -w²(x - alvo) - 2·zeta·w·x') leva o fator de andamento ao alvo da
    // seção e da harmonia — acelera e freia como um regente, com uma pequena
    // ultrapassagem, sem degraus. Por cima, um rubato em série de Fourier (4
    // harmônicos, amplitudes da seed caindo com 1/k², período da frase) e o
    // gesto: um arco por frase, que avança até o meio e relaxa no fim.
    // O coro de fases: só as relações em foco acoplam; quem não está em foco
    // segue a sua frequência própria, preso de leve ao pulso comum — cada
    // corpo tem o seu jeito de estar um pouco à frente ou atrás. Conceito do
    // ChorusPhase do Aquorbium (a partir de Aihara et al., "Spatio-temporal
    // dynamics in collective frog choruses examined by mathematical modeling
    // and field observations", Sci Rep 4:3891, 2014) e de Y. Kuramoto (1975);
    // nenhum código copiado.
    void stepChorus(double dt) noexcept {
        std::array<std::size_t, kMaxFocus> callers{};
        std::array<std::size_t, kMaxFocus> responders{};
        std::array<double, kMaxFocus> lags{};
        for (std::size_t index = 0U; index < focusCount_; ++index) {
            callers[index] = focus_[index].caller;
            responders[index] = focus_[index].responder;
            lags[index] = chorusLag(focus_[index].kind);
        }
        auto omega = chorusOmega_;
        std::array<double, kBodies> restShift{};
        if (driftSlot_ < kBodies) {
            omega[driftSlot_] = kChorusAnchor * driftRatio_;
            // O gargalo da equação de Adler fica a ¼ de volta do repouso;
            // girar a âncora do planeta põe a demora em cima do pulso.
            restShift[driftSlot_] = driftRatio_ > 0.0 ? -0.25 * kChorusTwoPi : 0.25 * kChorusTwoPi;
        }
        kuramotoStep(chorusPhase_, omega, callers.data(), responders.data(), lags.data(),
                     focusCount_, 2.5 * influence_, kChorusAnchor, dt, &restShift);
    }

    void stepElasticTime(double dt) noexcept {
        // O andamento sentido de cada parte vem do caráter dela (4/11); antes,
        // 0,95 / 1,05 / 1,0 / 1,0 em toda faixa.
        double target = character_.tempo();
        // A tensão harmônica puxa um ritardando; a cadência o desfaz.
        target *= 1.0 - 0.07 * tension_;
        constexpr double kOmega = 1.1;   // rad/s: a mola leva alguns segundos
        constexpr double kZeta = 0.62;   // levemente subamortecida
        const double acceleration = -kOmega * kOmega * (springPosition_ - target)
            - 2.0 * kZeta * kOmega * tempoVelocity_;
        tempoVelocity_ += acceleration * dt;
        springPosition_ += tempoVelocity_ * dt;
        // Fourier e gesto, sobre a posição dentro da frase (2 compassos).
        const double beats = static_cast<double>(beat_) + beatPhase_;
        const double phrase = beats / 8.0;
        const double width = section_ == Section::Contrast ? 0.045 : 0.018;
        double rubato = 0.0;
        for (std::uint32_t k = 1U; k <= 4U; ++k) {
            const auto word = mix(seed_ ^ (k * 0x9e3779b9U));
            const double amplitude = width / static_cast<double>(k * k)
                * (0.5 + static_cast<double>(word & 255U) / 510.0);
            const double phase = static_cast<double>((word >> 8U) & 1023U) / 1024.0 * 6.283185307179586;
            rubato += amplitude * std::sin(6.283185307179586 * static_cast<double>(k) * phrase + phase);
        }
        const double inPhrase = phrase - std::floor(phrase);
        const double gesture = 0.03 * (std::sin(3.141592653589793 * inPhrase) * std::sin(3.141592653589793 * inPhrase) - 0.5);
        // A equação do tempo: o relógio de sol se adianta e se atrasa pela
        // órbita elíptica (um ciclo por ano) e pela inclinação do eixo (dois
        // ciclos). E(B) ≈ 9,87·sen 2B − 7,53·cos B − 1,5·sen B minutos. O
        // andamento usa a DERIVADA (a velocidade do adiantamento), com o
        // "ano" valendo 16 compassos.
        const double yearAngle = 6.283185307179586 * beats / 64.0;
        const double equationRate = 19.74 * std::cos(2.0 * yearAngle) + 7.53 * std::sin(yearAngle)
            - 1.5 * std::cos(yearAngle);
        const double equationOfTime = 0.025 * equationRate / 29.0;
        const double factor = springPosition_ + rubato + gesture + equationOfTime;
        if (!std::isfinite(factor)) {
            springPosition_ = 1.0;
            tempoVelocity_ = 0.0;
        }
        tempoFactor_ = std::isfinite(factor) ? std::clamp(factor, 0.7, 1.35) : 1.0;
        // O swing também vem do caráter da parte (4/11).
        swing_ = character_.swing();
        groove_ = section_ == Section::Opening ? 0.5 : 1.0;
    }
    double springPosition_ = 1.0;
    std::uint32_t sectionOrdinal_ = 0U;
    std::uint32_t sectionBarsLeft_ = 0U;
    SectionCharacter character_{};
    std::array<double, kBodies> sectionEnergy_{};
    std::size_t recedeSlot_ = kBodies;
    double recedeGain_ = 1.0;
    std::size_t openingLeader_ = kBodies;
    std::size_t energyLeader() const noexcept {
        std::size_t best = kBodies;
        double total = 0.0;
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            total += sectionEnergy_[slot];
            if (sectionEnergy_[slot] > 0.0 && (best == kBodies || sectionEnergy_[slot] > sectionEnergy_[best])) best = slot;
        }
        return total > 0.0 ? best : kBodies;
    }
    SectionCharacter openingCharacter_{};
    SectionCharacter contrastCharacter_{};
    bool characterReady_ = false;

    double characterUnit(std::uint32_t salt) const noexcept {
        return static_cast<double>(mix(seed_ ^ (salt * 0x9e3779b9U) ^ (sectionOrdinal_ * 0x85ebca6bU)))
            / 4294967296.0;
    }
    static double reflect(double x) noexcept {
        while (x < 0.0 || x > 1.0) x = x < 0.0 ? -x : 2.0 - x;
        return x;
    }
    // Sobe a energia de uma parte até `target`, repartindo entre as estrelas
    // que soam e a atividade.
    static void raiseEnergy(SectionCharacter& c, double target) noexcept {
        double missing = std::clamp(target, 0.0, 1.0) - c.energy();
        for (int pass = 0; pass < 2 && missing > 1.0e-6; ++pass) {
            for (std::size_t i = 0U; i < 2U; ++i) {
                const double room = 1.0 - c.v[i];
                const double add = std::min(room, missing);
                c.v[i] += add;
                missing -= add * 0.5;
            }
        }
    }
    bool peakInDevelopment_ = true;
    // O próximo ponto do caminho, ao entrar numa parte.
    void stepCharacter() noexcept {
        SectionCharacter next = character_;
        if (!characterReady_ || section_ == Section::Opening) {
            for (std::size_t i = 0U; i < kCharacterDims; ++i) next.v[i] = characterUnit(101U + static_cast<std::uint32_t>(i));
            next.v[8] *= 0.4;  // o controle (que não soa) pesa menos no elenco
            openingCharacter_ = next;
            // Um retorno antes de qualquer contraste (rondó, 5/11) volta perto
            // da própria abertura.
            contrastCharacter_ = next;
            characterReady_ = true;
        } else if (section_ == Section::Return) {
            const double r = 0.1 + 0.35 * characterUnit(211U);
            for (std::size_t i = 0U; i < kCharacterDims; ++i)
                next.v[i] = openingCharacter_.v[i] + r * (contrastCharacter_.v[i] - openingCharacter_.v[i]);
            raiseEnergy(next, openingCharacter_.energy());
        } else {
            // Uma direção qualquer (soma de três sorteios: perto de uma normal).
            std::array<double, kCharacterDims> d{};
            double norm = 0.0;
            for (std::size_t i = 0U; i < kCharacterDims; ++i) {
                const auto k = static_cast<std::uint32_t>(i) * 3U;
                d[i] = characterUnit(301U + k) + characterUnit(302U + k) + characterUnit(303U + k) - 1.5;
                norm += d[i] * d[i];
            }
            norm = std::sqrt(std::max(norm, 1.0e-9));
            const bool contrast = section_ == Section::Contrast;
            const double step = contrast ? 0.55 + 0.35 * characterUnit(401U) : 0.15 + 0.2 * characterUnit(402U);
            for (std::size_t i = 0U; i < kCharacterDims; ++i)
                next.v[i] = reflect(character_.v[i] + step * d[i] / norm * (i < 4U ? 1.0 : 1.4));
            // Uma única regra leve (4/11): o salto do contraste move de verdade
            // o que mais se ouve — quantas estrelas soam e quanto tocam. A
            // direção segue a da seed (o contraste pode ser mais cheio ou mais
            // vazio); só a distância nessas duas dimensões tem um mínimo.
            // A peça não murcha (4/11, "a peça pareceu murchar"): em cada ciclo
            // a seed escolhe onde fica o pico — no desenvolvimento ou no
            // contraste — e ele passa da abertura; o retorno (abaixo) também
            // não cai abaixo dela. O resto do caminho segue livre.
            const double floorEnergy = openingCharacter_.energy();
            if (!contrast) {
                peakInDevelopment_ = characterUnit(501U) < 0.5;
                raiseEnergy(next, peakInDevelopment_ ? floorEnergy + 0.2 : floorEnergy - 0.05);
            }
            if (contrast) {
                const double audible = std::hypot(next.v[0] - character_.v[0], next.v[1] - character_.v[1]);
                if (audible < 0.45) {
                    for (std::size_t i = 0U; i < 2U; ++i) {
                        const double from = character_.v[i];
                        // Para o lado com mais espaço, se a seed não apontou.
                        double sign = next.v[i] - from;
                        if (std::fabs(sign) < 1.0e-6) sign = from < 0.5 ? 1.0 : -1.0;
                        const double want = from + (sign > 0.0 ? 0.35 : -0.35);
                        next.v[i] = reflect(want);
                    }
                }
                if (!peakInDevelopment_) raiseEnergy(next, std::max(floorEnergy, character_.energy()) + 0.15);
                contrastCharacter_ = next;
            }
        }
        applyDensity(next);
        character_ = next;
    }
    // Os genes de densidade (6/11): cada um num fluxo próprio da seed, em
    // faixas contínuas, para haver muitas combinações e não dois tipos:
    // - o piso de estrelas acesas, de 2 a 4,5;
    // - a largura da faixa, de 1,5 a 5,5 estrelas acima do piso (até 8);
    // - o piso da atividade, de 0,5 a 0,85 (acesas tocam de fato);
    // - o teto da atividade, de 1,15 a 1,8 (quanto o céu adensa);
    // - quanto o líder que sai recua, no mínimo 0,3 a 0,7 do seu brilho.
    void chooseDensityGenes() noexcept {
        const auto gene = [this](std::uint32_t domain) {
            return static_cast<double>(performanceSeedStream(seed_, domain) & 0xffffU) / 65535.0;
        };
        litLow_ = 2.0 + 2.5 * gene(0xDE00U);
        litHigh_ = std::min(8.0, litLow_ + 1.5 + 4.0 * gene(0xDE01U));
        activityLow_ = 0.5 + 0.35 * gene(0xDE02U);
        activityHigh_ = 1.15 + 0.65 * gene(0xDE03U);
        recedeFloor_ = 0.3 + 0.4 * gene(0xDE04U);
        // - o coro: quanto o céu chama as acesas a cada tempo, de 0 a 0,6
        //   (vezes a atividade da parte): de céus de solistas a céus corais.
        chorus_ = 0.6 * gene(0xDE05U);
    }
    void applyDensity(SectionCharacter& character) const noexcept {
        character.litLow = litLow_;
        character.litHigh = litHigh_;
        character.activityLow = activityLow_;
        character.activityHigh = activityHigh_;
        character.chorus = chorus_;
    }
    double litLow_ = 2.0, litHigh_ = 7.0, activityLow_ = 0.35, activityHigh_ = 1.6, recedeFloor_ = 0.0, chorus_ = 0.0;
    std::array<bool, kBodies> openingLit_{};

    // As gramáticas da forma (5/11, auditoria da seed, item C). Antes havia
    // uma só (abertura, depois desenvolvimento → contraste → retorno em
    // ciclo). Cada gramática é uma sequência de partes, o ponto para onde ela
    // volta depois da primeira passagem e o peso de cada parte na duração.
    // O caráter de cada parte continua gerado (stepCharacter): a gramática dá
    // a ordem e as proporções, não o som. Conceito do "morceau/arc" do
    // rasgo-synth (arco, Freytag, kishōtenketsu), sem código.
    struct Grammar {
        std::array<Section, 8> parts;
        std::uint8_t count;
        std::uint8_t loop;
        std::array<double, 8> weight;
    };
    static constexpr Section O = Section::Opening, D = Section::Development, C = Section::Contrast,
                             R = Section::Return;
    static constexpr std::array<Grammar, 6> kGrammars{{
        // ciclo: a forma de antes
        {{{O, D, C, R, O, O, O, O}}, 4U, 1U, {{1.0, 1.0, 1.0, 1.0, 1, 1, 1, 1}}},
        // arco: subida longa em dois desenvolvimentos, clímax no contraste
        {{{O, D, D, C, R, O, O, O}}, 5U, 1U, {{0.8, 1.0, 1.1, 1.3, 1.0, 1, 1, 1}}},
        // rondó: o retorno volta entre os episódios (A B A C A)
        {{{O, D, R, C, R, O, O, O}}, 5U, 1U, {{0.9, 1.0, 0.7, 1.1, 0.9, 1, 1, 1}}},
        // ondas: duas subidas, a segunda chega ao contraste
        {{{O, D, R, D, C, R, O, O}}, 6U, 1U, {{0.8, 0.9, 0.6, 1.0, 1.2, 0.9, 1, 1}}},
        // kishōtenketsu: apresentar, desenvolver, a virada curta, concluir
        {{{O, D, C, R, O, O, O, O}}, 4U, 1U, {{1.0, 1.3, 0.5, 1.4, 1, 1, 1, 1}}},
        // choque inicial: o contraste logo depois da abertura, depois o caminho
        {{{O, C, D, C, R, O, O, O}}, 5U, 2U, {{0.7, 0.6, 1.1, 1.0, 1.1, 1, 1, 1}}},
    }};

    // Numa faixa com duração, os compassos da faixa repartidos pela primeira
    // passagem da gramática, cada parte com ao menos 2 compassos; se a faixa
    // for curta demais para a gramática, o ciclo de sempre com o plano dado.
    // A abertura fica com os compassos que o plano lhe dá; o resto da faixa
    // se reparte pelas outras partes da gramática.
    void planGrammar(std::uint32_t totalBars) noexcept {
        const auto& g = kGrammars[grammar_];
        plannedParts_ = 0U;
        const auto opening = std::max<std::uint32_t>(1U, formPlan_[0]);
        if (totalBars < opening + 2U * (g.count - 1U)) return;
        plannedBars_[0] = opening;
        totalBars -= opening;
        double weights = 0.0;
        for (std::size_t i = 1U; i < g.count; ++i) weights += g.weight[i];
        std::uint32_t used = 0U;
        for (std::size_t i = 1U; i < g.count; ++i) {
            const auto bars = i + 1U == g.count
                ? std::max<std::uint32_t>(2U, totalBars > used ? totalBars - used : 2U)
                : std::max<std::uint32_t>(2U, static_cast<std::uint32_t>(std::lround(
                      static_cast<double>(totalBars) * g.weight[i] / weights)));
            plannedBars_[i] = bars;
            used += bars;
        }
        plannedParts_ = g.count;
    }

    void planSkyGestures() noexcept {
        gestureBudget_ = 0U;
        if (section_ == Section::Opening || sectionBars_ < 2U) return;
        const double energy = character_.energy();
        const int base = energy > 0.62 ? 2 : energy > 0.38 ? 1 : 0;
        const auto word = performanceSeedStream(seed_ ^ (sectionOrdinal_ * 0x9E3779B9U), 0xE000U);
        int budget = std::clamp(base + static_cast<int>(word % 3U) - 1, 0, 2);
        if (section_ == Section::Return) budget = std::min(budget, 1);
        gestureBudget_ = static_cast<std::uint32_t>(budget);
        for (std::uint32_t k = 0U; k < gestureBudget_; ++k)
            gestureBar_[k] = 1U + (word >> (8U + k * 8U)) % std::max<std::uint32_t>(1U, sectionBars_ - 1U);
    }
    void checkSkyGesture() noexcept {
        if (influence_ <= 0.0 || gestureBudget_ == 0U || sectionBars_ < sectionBarsLeft_) return;
        const auto bar = sectionBars_ - sectionBarsLeft_;
        for (std::uint32_t k = 0U; k < gestureBudget_; ++k) {
            if (gestureBar_[k] != bar) continue;
            const auto word = performanceSeedStream(seed_ ^ (sectionOrdinal_ * 0x85EBCA6BU) ^ (k * 0x27D4EB2DU), 0xE100U);
            std::array<std::size_t, kBodies> lit{};
            std::size_t count = 0U;
            for (std::size_t slot = 0U; slot < kBodies; ++slot)
                if (brightnessTarget_[slot] > 0.5
                    && performanceStarFunction(bodies_[slot].star) != PerformanceStarFunction::Control)
                    lit[count++] = slot;
            if (count == 0U) return;
            pendingGesture_.slot = lit[word % count];
            pendingGesture_.displace = ((word >> 8U) & 1U) != 0U;
            pendingGesture_.intensity = 0.35 + 0.35 * static_cast<double>((word >> 12U) & 0xffU) / 255.0;
            pendingGesture_.duration = 0.2 + 0.4 * static_cast<double>((word >> 20U) & 0xffU) / 255.0;
            pendingGesture_.token = word;
            gesturePending_ = true;
            ++skyGestures_;
            return;
        }
    }

    std::uint32_t sectionLength(std::uint32_t ordinal) const noexcept {
        if (formPlanned_) {
            if (plannedParts_ > 0U && formPass_ == 0U && formStep_ < plannedParts_) return plannedBars_[formStep_];
            if (plannedParts_ == 0U && ordinal < formPlan_.size()) return std::max<std::uint32_t>(1U, formPlan_[ordinal]);
        }
        const auto& g = kGrammars[grammar_];
        const double weight = g.weight[std::min<std::size_t>(formStep_, 7U)];
        const auto base = 8U + mix(seed_ ^ (ordinal * 0x632be5abU)) % 13U;
        return std::clamp<std::uint32_t>(static_cast<std::uint32_t>(std::lround(base * weight)), 3U, 28U);
    }

    // A cada compasso: quando a seção acaba, a seguinte começa. Abertura uma
    // vez; depois desenvolvimento → contraste → retorno, em ciclo.
    void advanceForm() noexcept {
        if (sectionBarsLeft_ == 0U) {
            sectionBarsLeft_ = sectionLength(0U);
            if (!characterReady_) stepCharacter();
            return;
        }
        if (--sectionBarsLeft_ > 0U) return;
        if (section_ == Section::Opening) rememberOpening();
        ++sectionOrdinal_;
        {
            // A parte seguinte vem da gramática da seed; no fim de uma
            // passagem, a forma volta ao ponto de retorno dela e conta a volta.
            const bool cycleOnly = formPlanned_ && plannedParts_ == 0U;
            const auto& g = kGrammars[cycleOnly ? 0U : grammar_];
            auto next = static_cast<std::uint32_t>(formStep_) + 1U;
            if (next >= g.count) { next = g.loop; ++formPass_; }
            formStep_ = static_cast<std::uint8_t>(next);
            section_ = g.parts[formStep_];
        }
        sectionBarsLeft_ = sectionLength(sectionOrdinal_);
        sectionBars_ = sectionBarsLeft_;
        const auto previousLeader = energyLeader();
        const auto previousCharacter = character_;
        stepCharacter();
        planSkyGestures();
        // A liderança que gira: quem levou a energia da parte que acabou recua
        // na proporção do salto; no retorno, o líder da abertura volta.
        if (section_ == Section::Development && sectionOrdinal_ == 1U) openingLeader_ = previousLeader;
        const double jump = characterDistance(character_, previousCharacter);
        recedeSlot_ = previousLeader;
        // Um salto de 0,8 ou mais (o contraste, em geral) tira o líder de cena;
        // um passo curto (o desenvolvimento) só o faz recuar um pouco. O
        // líder natural costuma estar 10–20 dB acima das outras estrelas: um
        // recuo pela metade não muda quem lidera.
        recedeGain_ = std::max(recedeFloor_, 1.0 - std::clamp(jump / 0.8, 0.0, 1.0));
        if (section_ == Section::Return && recedeSlot_ == openingLeader_) recedeGain_ = 1.0;
        sectionEnergy_.fill(0.0);
        if (mixerVariation_ && brightnessReady_) reshapeForSection();
        chooseDrift();
        reshapeMotion();
        if (section_ == Section::Return) recapitulate();
    }

    // A deriva (29/10): em cada desenvolvimento um corpo aceso vira o
    // "planeta" — a sua frequência própria passa um pouco do que a âncora
    // segura (|ω| > K0) e a fase escorrega contra o pulso das outras: demora
    // perto do alinhamento, atravessa um passo inteiro e se realinha um passo
    // adiante (equação de Adler, R. Adler, "A study of locking phenomena in
    // oscillators", Proc. IRE 34(6), 1946; o "phasing" de S. Reich como
    // referência musical). Como o dia sideral contra o solar (366,25 :
    // 365,25), um ciclo a mais a cada tantos: aqui a razão ω/K0 fica entre
    // 1,01 e 1,06 — uma volta inteira em ~50 a ~180 s.
    // A cada lua nova, o eclipse (se houver): um corpo aceso — o "sol" — é
    // encoberto por dois tempos, e outro — a "lua" — responde com o motivo
    // dele (reflete a sua luz). Sem a variação do mixer, fica só a resposta.
    void watchTheMoon() noexcept {
        const auto lunation = static_cast<std::uint64_t>(
            std::floor((static_cast<double>(beat_) + synodicOffset_) / kSynodicBeats));
        if (lunation == lunation_) return;
        lunation_ = lunation;
        if (!eclipseAtLunation(lunation, draconicOffset_)) return;
        const auto word = mix(seed_ ^ static_cast<std::uint32_t>(lunation * 0x2545F491U));
        std::size_t sun = kBodies;
        std::size_t moon = kBodies;
        for (std::size_t offset = 0U; offset < kBodies; ++offset) {
            const auto slot = (word + offset) % kBodies;
            if (!bodies_[slot].lit || bodies_[slot].presence <= 0.0 || brightnessTarget_[slot] < 0.5) continue;
            if (sun == kBodies) sun = slot;
            else if (moon == kBodies) { moon = slot; break; }
        }
        if (sun == kBodies || moon == kBodies) return;
        eclipseSun_ = sun;
        eclipseBeatsLeft_ = 2U;
        ++eclipseCount_;
        if (motifCount_[sun] >= 2U) scheduleMotif(sun, moon, 0.8, 0.5, 1U, false);
        else schedule(moon, heardPitch_[sun], 0.8, 0.5, 1U);
    }

    // A recapitulação (29/10): como no ciclo metônico — 19 anos trazem a
    // mesma fase da Lua na mesma data —, o retorno traz o motivo da
    // abertura de volta, literal (sem transformação), pela mesma estrela. No
    // fim da abertura o céu guarda o motivo da estrela mais ativa.
    void rememberOpening() noexcept {
        std::size_t best = kBodies;
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            if (!bodies_[slot].lit || motifCount_[slot] < 3U) continue;
            if (best == kBodies || attackCount_[slot] > attackCount_[best]) best = slot;
        }
        openingSlot_ = best;
        openingCount_ = 0U;
        if (best == kBodies) return;
        openingCount_ = std::min<std::size_t>(motifCount_[best], 6U);
        for (std::size_t index = 0U; index < openingCount_; ++index)
            openingMotif_[index] = motif_[best][(motifHead_[best] + kMotifLength - openingCount_ + index) % kMotifLength];
    }
    void recapitulate() noexcept {
        if (influence_ <= 0.0 || openingCount_ < 2U || openingSlot_ >= kBodies) return;
        auto voice = openingSlot_;
        if (!bodies_[voice].lit) {
            voice = kBodies;
            for (std::size_t slot = 0U; slot < kBodies && voice == kBodies; ++slot)
                if (bodies_[slot].lit) voice = slot;
        }
        if (voice == kBodies) return;
        double when = 0.5;
        for (std::size_t index = 0U; index < openingCount_; ++index) {
            if (index > 0U) when += std::clamp(openingMotif_[index].ioiBeats, 0.125, 4.0);
            schedule(voice, std::clamp(openingMotif_[index].pitchHz, 30.0, 2000.0), 0.9, when, kMaxChainDepth);
        }
        ++recapitulations_;
    }

    void chooseDrift() noexcept {
        driftSlot_ = kBodies;
        if (section_ != Section::Development) return;
        const auto word = mix(seed_ ^ (sectionOrdinal_ * 0x51ED27U) ^ 0xD71F7U);
        for (std::size_t offset = 0U; offset < kBodies; ++offset) {
            const auto slot = (word + offset) % kBodies;
            if (!bodies_[slot].lit || bodies_[slot].presence <= 0.0 || brightnessTarget_[slot] < 0.5) continue;
            driftSlot_ = slot;
            break;
        }
        driftRatio_ = (1.01 + 0.05 * static_cast<double>((word >> 8U) & 255U) / 255.0)
            * (((word >> 16U) & 1U) != 0U ? 1.0 : -1.0);
    }

    // O quanto uma estrela fica na frente na parte atual: o peso da família
    // dela relativo à família mais forte do caráter (de 0,45 a 1).
    double foregroundOf(std::size_t slot) const noexcept {
        double strongest = 1.0e-6;
        for (std::size_t f = 0U; f < 4U; ++f) strongest = std::max(strongest, character_.v[4U + f]);
        const auto function = performanceStarFunction(bodies_[slot].star);
        if (function == PerformanceStarFunction::Control) return 1.0;
        return 0.45 + 0.55 * std::clamp(character_.family(function) / strongest, 0.0, 1.0);
    }

    // O adensamento (4/11): numa parte de atividade acima de 1, a cada tempo
    // cada estrela acesa (que soa) tem a chance de ser chamada a tocar de
    // novo, num lugar do tempo e com uma força que a seed escolhe.
    void densifyBeat() noexcept {
        const double extra = character_.densify();
        if (extra <= 0.0) return;
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            if (!bodies_[slot].lit || bodies_[slot].presence <= 0.0 || brightnessTarget_[slot] < 0.5) continue;
            if (performanceStarFunction(bodies_[slot].star) == PerformanceStarFunction::Control) continue;
            const auto word = mix(seed_ ^ static_cast<std::uint32_t>(beat_ * 0x27d4eb2dU) ^ static_cast<std::uint32_t>(slot * 0x165667b1U));
            if (static_cast<double>(word & 0xffffU) / 65536.0 >= extra) continue;
            static constexpr std::array<double, 4> places{{0.0, 0.5, 0.25, 0.75}};
            const double strength = 0.45 + 0.35 * static_cast<double>((word >> 16U) & 255U) / 255.0;
            schedule(slot, 0.0, strength, places[(word >> 24U) & 3U], kMaxChainDepth);
        }
    }

    // O elenco de cada parte (4/11): ao entrar numa parte, as estrelas que
    // soam são escolhidas pelo peso da família delas no caráter da parte, com
    // um pouco de acaso da seed; no retorno, quem estava na abertura tem
    // vantagem (tanto mais quanto mais o retorno se parece com ela). Antes
    // eram regras fixas, iguais em toda faixa (a abertura nos corpos 1–3, o
    // corpo 1 recuando no contraste, o retorno literal).
    double castScore(std::size_t slot) const noexcept {
        double score = character_.family(performanceStarFunction(bodies_[slot].star))
            + 0.35 * static_cast<double>(mix(seed_ ^ static_cast<std::uint32_t>(slot * 977U + sectionOrdinal_ * 131U)) & 1023U)
                / 1023.0;
        if (section_ == Section::Return && openingLit_[slot])
            score += 0.6 * (1.0 - characterDistance(character_, openingCharacter_) / 2.0);
        return score;
    }
    void reshapeForSection() noexcept {
        std::array<std::size_t, kBodies> order{};
        std::size_t count = 0U;
        for (std::size_t slot = 0U; slot < kBodies; ++slot)
            if (bodies_[slot].presence > 0.0 && PerformanceSky::isStar(bodies_[slot].star)) order[count++] = slot;
        // Inserção (no máximo 8 itens; o std::sort sobre parte do array dava um
        // falso alarme de -Warray-bounds no GCC 13 com -O3).
        std::array<double, kBodies> scores{};
        for (std::size_t index = 0U; index < count; ++index) scores[index] = castScore(order[index]);
        for (std::size_t index = 1U; index < count && index < kBodies; ++index) {
            for (std::size_t back = index; back > 0U && scores[back] > scores[back - 1U]; --back) {
                std::swap(scores[back], scores[back - 1U]);
                std::swap(order[back], order[back - 1U]);
            }
        }
        const std::size_t wanted = std::min({character_.litTarget(), count, starCapacity_});
        std::size_t chosen = 0U;
        for (std::size_t rank = 0U; rank < count; ++rank) {
            const auto slot = order[rank];
            if (heldBars_[slot] > 0U) continue;
            const bool wasLit = brightnessTarget_[slot] > 0.5;
            const bool on = chosen < wanted || pinned_[slot];
            if (on) {
                ++chosen;
                brightnessTarget_[slot] = 1.0;
                litBars_[slot] = sectionBarsLeft_ + 2U;
                if (!wasLit && influence_ > 0.0) schedule(slot, 0.0, 0.95, 0.0, kMaxChainDepth);
            } else {
                brightnessTarget_[slot] = 0.0;
                restBars_[slot] = sectionBarsLeft_;
            }
        }
    }

    RelationKind sectionKind(std::size_t a, std::size_t b, std::uint32_t word) const noexcept {
        const auto fa = performanceStarFunction(bodies_[a].star);
        const auto fb = performanceStarFunction(bodies_[b].star);
        const bool tonal = (fa == PerformanceStarFunction::Tone || fa == PerformanceStarFunction::Voice)
            && (fb == PerformanceStarFunction::Tone || fb == PerformanceStarFunction::Voice);
        switch (section_) {
        case Section::Opening: return (word & 4U) != 0U ? RelationKind::Support : RelationKind::Contagion;
        case Section::Development: return RelationKind::CallResponse;
        case Section::Contrast: return tonal ? RelationKind::Friction : RelationKind::Hocket;
        case Section::Return:
        default: return RelationKind::CallResponse;
        }
    }

    RelationKind strategicKind(std::size_t a, std::size_t b, RelationKind natural) const noexcept {
        const auto word = mix(seed_ ^ (beat_ * 0x27d4eb2dU) ^ static_cast<std::uint32_t>(a * 53U + b * 29U));
        // Um terço segue o tipo natural do par, um terço a seção da forma, um
        // terço a estratégia do jogo.
        const auto share = word % 3U;
        if (share == 0U) return natural;
        if (share == 1U) return sectionKind(a, b, word);
        const auto fa = performanceStarFunction(bodies_[a].star);
        const auto fb = performanceStarFunction(bodies_[b].star);
        const bool tonal = (fa == PerformanceStarFunction::Tone || fa == PerformanceStarFunction::Voice)
            && (fb == PerformanceStarFunction::Tone || fb == PerformanceStarFunction::Voice);
        switch (strategy_) {
        case PerformanceGameStrategy::Follow:
            return (word & 2U) != 0U ? RelationKind::Contagion : RelationKind::Support;
        case PerformanceGameStrategy::Transform:
            return RelationKind::CallResponse;
        case PerformanceGameStrategy::Counter:
            return tonal ? RelationKind::Friction : RelationKind::Hocket;
        case PerformanceGameStrategy::Interrupt:
            return RelationKind::Hocket;
        case PerformanceGameStrategy::Conceal:
            return RelationKind::Shadow;
        case PerformanceGameStrategy::Negotiate:
        default:
            return RelationKind::CallResponse;
        }
    }
    std::array<std::uint8_t, kBodies> answerDepth_{};

    static std::uint32_t mix(std::uint32_t value) noexcept {
        value ^= value >> 16U;
        value *= 0x7feb352dU;
        value ^= value >> 15U;
        value *= 0x846ca68bU;
        return value ^ (value >> 16U);
    }

    static double massOf(const Body& body) noexcept {
        double factor = 1.0;
        switch (performanceStarFunction(body.star)) {
        case PerformanceStarFunction::Pulse: factor = 1.25; break;
        case PerformanceStarFunction::Tone: factor = 1.0; break;
        case PerformanceStarFunction::Voice: factor = 0.8; break;
        case PerformanceStarFunction::Texture: factor = 0.65; break;
        case PerformanceStarFunction::Control: factor = 0.5; break;
        }
        return (0.25 + std::clamp(body.presence, 0.0, 1.0)) * factor;
    }

    void placeAtAnchors() noexcept {
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            x_[slot] = restX_[slot] = shownX_[slot] = bodies_[slot].anchorX;
            y_[slot] = restY_[slot] = shownY_[slot] = bodies_[slot].anchorY;
            vx_[slot] = vy_[slot] = 0.0;
        }
        positionsReady_ = true;
    }

    // Leapfrog (kick-drift-kick): simplético, estável com passo fixo.
    // Um passo do movimento: forças → velocidades → posições (Euler
    // semi-implícito, estável para molas), depois os choques e as paredes.
    void stepGravity(double dt) noexcept {
        const auto target = seededChoreography(section_);
        const double blend = std::min(1.0, dt * 0.6);
        dance_.anchor += (target.anchor - dance_.anchor) * blend;
        dance_.damping += (target.damping - dance_.damping) * blend;
        dance_.flow += (target.flow - dance_.flow) * blend;
        dance_.magnet += (target.magnet - dance_.magnet) * blend;
        dance_.stadium += (target.stadium - dance_.stadium) * blend;
        dance_.bands += (target.bands - dance_.bands) * blend;
        dance_.kick += (target.kick - dance_.kick) * blend;
        dance_.spin += (target.spin - dance_.spin) * blend;
        dance_.foam += (target.foam - dance_.foam) * blend;
        dance_.epicycle += (target.epicycle - dance_.epicycle) * blend;
        dance_.distortion += (target.distortion - dance_.distortion) * blend;
        motionTime_ += dt;
        // Giro e viagem da constelação inteira, contados em tempos musicais.
        const double beatsPerSecond = sampleRate_ / std::max(samplesPerBeat_, 1.0);
        const bool rotating = constellationMotion_ == PerformanceConstellationMotion::Rotate
            || constellationMotion_ == PerformanceConstellationMotion::RotateTravel;
        const bool travelling = constellationMotion_ == PerformanceConstellationMotion::Travel
            || constellationMotion_ == PerformanceConstellationMotion::RotateTravel;
        if (rotating) {
            rotationAngle_ += rotationSign_ * kChorusTwoPi / 64.0 * beatsPerSecond * dt;
            rotationAngle_ -= kChorusTwoPi * std::floor(rotationAngle_ / kChorusTwoPi);
        } else {
            // Volta ao desenho pelo caminho mais curto, em ~4 s.
            const double back = rotationAngle_ > kChorusTwoPi * 0.5 ? rotationAngle_ - kChorusTwoPi : rotationAngle_;
            rotationAngle_ = back * (1.0 - std::min(1.0, dt * 0.25));
            if (rotationAngle_ < 0.0) rotationAngle_ += kChorusTwoPi;
        }
        if (travelling) travelPhase_ += kChorusTwoPi / 96.0 * beatsPerSecond * dt;
        travelAmount_ += ((travelling ? 1.0 : 0.0) - travelAmount_) * std::min(1.0, dt * 0.25);
        double centreX = 0.0, centreY = 0.0;
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            centreX += bodies_[slot].anchorX;
            centreY += bodies_[slot].anchorY;
        }
        centreX /= static_cast<double>(kBodies);
        centreY /= static_cast<double>(kBodies);
        const double cosA = std::cos(rotationAngle_), sinA = std::sin(rotationAngle_);
        const double shiftX = travelAmount_ * 0.16 * std::sin(travelPhase_);
        const double shiftY = travelAmount_ * 0.11 * std::sin(2.0 * travelPhase_ + 0.7);
        // O lugar de repouso passeia em epiciclos de Fibonacci (cada corpo
        // com a sua velocidade, em razão áurea com os outros).
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            double ex = 0.0, ey = 0.0;
            sky_motion::fibonacciEpicycle(motionTime_, kEpicycleOmega * kGoldenRoots[slot % 4U],
                                          dance_.epicycle, flowPhase_ + static_cast<double>(slot) * 2.399963, ex, ey);
            const double ax = bodies_[slot].anchorX - centreX, ay = bodies_[slot].anchorY - centreY;
            const double turnedX = centreX + ax * cosA - ay * sinA + shiftX;
            const double turnedY = centreY + ax * sinA + ay * cosA + shiftY;
            restX_[slot] = std::clamp(turnedX + ex, 0.02, 0.98);
            restY_[slot] = std::clamp(turnedY + ey, 0.02, 0.98);
            spin_[slot] *= 1.0 - kSpinDecay * dt;
            bubble_[slot] = bubbleRadius(slot) * familyShape(family(slot)).bubble;
        }
        std::array<double, kBodies> ax{}, ay{};
        accelerations(ax, ay);
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            vx_[slot] += ax[slot] * dt;
            vy_[slot] += ay[slot] * dt;
            // Rapidez limitada: o céu dança, não explode.
            const double speed = std::hypot(vx_[slot], vy_[slot]);
            if (speed > kMaxSpeed) {
                vx_[slot] *= kMaxSpeed / speed;
                vy_[slot] *= kMaxSpeed / speed;
            }
            x_[slot] += vx_[slot] * dt;
            y_[slot] += vy_[slot] * dt;
        }
        // Bilhar: choques elásticos entre corpos acesos que se tocam.
        for (std::size_t a = 0U; a < kBodies; ++a) {
            if (!bodies_[a].lit) continue;
            for (std::size_t b = a + 1U; b < kBodies; ++b) {
                if (!bodies_[b].lit) continue;
                SkyBodyState first{x_[a], y_[a], vx_[a], vy_[a]};
                SkyBodyState second{x_[b], y_[b], vx_[b], vy_[b]};
                const double impact = std::hypot(vx_[a] - vx_[b], vy_[a] - vy_[b]);
                if (sky_motion::collide(first, second, massOf(bodies_[a]), massOf(bodies_[b]), kContact)) {
                    sky_motion::spinExchange(first, second, spin_[a], spin_[b], kContact * 0.5, kGrip);
                    // O choque soa: as duas estrelas atacam juntas, com a
                    // força do impacto (o ritmo que nasce do bilhar).
                    if (impact > kAudibleImpact) {
                        const double strength = std::clamp(0.45 + impact, 0.45, 1.0);
                        schedule(a, 0.0, strength, 0.0, kMaxChainDepth);
                        schedule(b, 0.0, strength, 0.0, kMaxChainDepth);
                        ++collisionCount_;
                    }
                    x_[a] = first.x; y_[a] = first.y; vx_[a] = first.vx; vy_[a] = first.vy;
                    x_[b] = second.x; y_[b] = second.y; vx_[b] = second.vx; vy_[b] = second.vy;
                }
            }
        }
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            SkyBodyState body{x_[slot], y_[slot], vx_[slot], vy_[slot]};
            sky_motion::reflectBox(body);
            x_[slot] = body.x; y_[slot] = body.y; vx_[slot] = body.vx; vy_[slot] = body.vy;
            if (!std::isfinite(x_[slot] + y_[slot] + vx_[slot] + vy_[slot] + spin_[slot])) {
                x_[slot] = bodies_[slot].anchorX;
                y_[slot] = bodies_[slot].anchorY;
                vx_[slot] = vy_[slot] = 0.0;
                spin_[slot] = 0.0;
            }
            // A separatriz do pêndulo: quando o corpo pendurado passa por
            // cima do ponto de suspensão (balanço vira volta completa), ataca.
            const auto link = chainLink(slot);
            if (link < kChainLength) {
                const double hangY = link == 0U ? restY_[slot] : y_[chain_[link - 1U]];
                const bool over = y_[slot] < hangY - kPendulumRest * 0.5;
                if (over && !pendulumOver_[link]) {
                    schedule(slot, 0.0, 0.9, 0.0, kMaxChainDepth);
                    ++pendulumTurns_;
                }
                pendulumOver_[link] = over;
            }
            // O que a Constelação vê: o espaço deformado pela matriz móvel.
            sky_motion::distort(motionTime_, dance_.distortion, flowPhase_, x_[slot], y_[slot],
                                shownX_[slot], shownY_[slot]);
            shownX_[slot] = sky_motion::softEdge(shownX_[slot]);
            shownY_[slot] = sky_motion::softEdge(shownY_[slot]);
        }
    }

    void accelerations(std::array<double, kBodies>& ax, std::array<double, kBodies>& ay) const noexcept {
        // O campo magnético (por corpo, abaixo) se inverte devagar — um ciclo
        // a cada ~40 s: as espirais trocam de sentido.
        const double phase = motionTime_ * 0.11 + flowPhase_;
        // A reação das molas da cadeia sobre o corpo de cima.
        std::array<double, kBodies> chainReactionX{}, chainReactionY{};
        for (std::size_t a = 0U; a < kBodies; ++a) {
            const SkyBodyState body{x_[a], y_[a], vx_[a], vy_[a]};
            const double mass = massOf(bodies_[a]);
            const auto shape = familyShape(family(a));
            ax[a] = -dance_.damping * shape.damping * vx_[a];
            ay[a] = -dance_.damping * shape.damping * vy_[a];
            // Elástico ao lugar desenhado; o pêndulo pendura dele por uma mola
            // com gravidade.
            const auto link = chainLink(a);
            if ((link < kChainLength && dance_.anchor < 3.0) || shape.pendulum) {
                // A cadeia de pêndulos: o primeiro pende do seu lugar, cada
                // um dos outros pende do anterior.
                const double hangX = link == 0U || link >= kChainLength ? restX_[a] : x_[chain_[link - 1U]];
                const double hangY = link == 0U || link >= kChainLength ? restY_[a] : y_[chain_[link - 1U]];
                double springX = 0.0, springY = 0.0;
                sky_motion::swingingSpring(body, hangX, hangY, kPendulumRest, kPendulumStiffness, 0.0,
                                           springX, springY);
                ax[a] += springX;
                ay[a] += springY + kPendulumGravity;
                // Ação e reação: a mola puxa também o corpo de cima.
                if (link > 0U && link < kChainLength) {
                    const auto parent = chain_[link - 1U];
                    const double ratio = mass / massOf(bodies_[parent]);
                    chainReactionX[parent] -= springX * ratio;
                    chainReactionY[parent] -= springY * ratio;
                }
            } else {
                ax[a] += dance_.anchor * shape.anchor * (restX_[a] - x_[a]);
                ay[a] += dance_.anchor * shape.anchor * (restY_[a] - y_[a]);
            }
            if (!bodies_[a].lit) continue;
            // A pista de correntes: o corpo é arrastado para a velocidade do
            // redemoinho onde está.
            double u = 0.0, v = 0.0;
            sky_motion::flow(flowShape_, x_[a], y_[a], phase, u, v);
            ax[a] += dance_.flow * shape.flow * kFlowDrag * (u * kFlowSpeed - vx_[a]);
            ay[a] += dance_.flow * shape.flow * kFlowDrag * (v * kFlowSpeed - vy_[a]);
            const double bodyField = std::max(dance_.magnet * shape.magnet, shape.magnetFloor * 1.6)
                * kMagnetField * std::sin(motionTime_ * 0.157 + flowPhase_);
            sky_motion::lorentz(body, charge_[a] / mass, bodyField, ax[a], ay[a]);
            sky_motion::magnus(body, spin_[a], kMagnus * dance_.spin, ax[a], ay[a]);
            const double bubbleA = bubble_[a];
            // As paredes do estádio, macias: só no contraste empurram para
            // dentro (o bilhar caótico de Bunimovich).
            const double stadium = std::max(dance_.stadium, shape.stadium);
            if (stadium > 0.01) {
                SkyBodyState probe = body;
                if (sky_motion::reflectStadium(probe, 0.5, 0.5, kStadiumHalf, kStadiumRadius)) {
                    ax[a] += stadium * kStadiumPush * (probe.x - x_[a]);
                    ay[a] += stadium * kStadiumPush * (probe.y - y_[a]);
                }
            }
            for (std::size_t b = 0U; b < kBodies; ++b) {
                if (a == b || !bodies_[b].lit) continue;
                const double dx = x_[b] - x_[a];
                const double dy = y_[b] - y_[a];
                const double r2 = dx * dx + dy * dy + kSoftening * kSoftening;
                // Gravidade (atração) e a pressão das bolhas (repulsão de
                // contato: a espuma).
                const double inv = kGravity * shape.gravity * massOf(bodies_[b]) / (r2 * std::sqrt(r2));
                ax[a] += dx * inv;
                ay[a] += dy * inv;
                sky_motion::bubblePressure(body, {x_[b], y_[b], vx_[b], vy_[b]}, bubbleA, bubble_[b],
                                           kFoamStiffness * (0.3 + dance_.foam) / mass, ax[a], ay[a]);
            }
        }
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            ax[slot] += chainReactionX[slot];
            ay[slot] += chainReactionY[slot];
        }
        // Os fenômenos dos cabos desenhados viram forças entre os dois corpos.
        for (std::size_t a = 0U; a < kBodies; ++a) {
            for (std::size_t b = 0U; b < kBodies; ++b) {
                if (a == b || cablePhenomenon_[a][b] == kNoPhenomenon || cable_[a][b] < 0.02
                    || !bodies_[a].lit || !bodies_[b].lit) continue;
                double aax = 0.0, aay = 0.0, bax = 0.0, bay = 0.0;
                phenomenonForce(cablePhenomenon_[a][b], cable_[a][b], {x_[a], y_[a], vx_[a], vy_[a]},
                                {x_[b], y_[b], vx_[b], vy_[b]}, aax, aay, bax, bay);
                ax[a] += aax / massOf(bodies_[a]);
                ay[a] += aay / massOf(bodies_[a]);
                ax[b] += bax / massOf(bodies_[b]);
                ay[b] += bay / massOf(bodies_[b]);
            }
        }
        // Os elásticos: cada relação em foco é uma mola com o seu comprimento
        // (pedal e contágio curtos; resposta média; hocket e atrito longos).
        for (std::size_t index = 0U; index < focusCount_; ++index) {
            const auto a = focus_[index].caller;
            const auto b = focus_[index].responder;
            if (a >= kBodies || b >= kBodies || a == b) continue;
            const double dx = x_[b] - x_[a];
            const double dy = y_[b] - y_[a];
            const double length = std::max(std::sqrt(dx * dx + dy * dy), 1.0e-6);
            const double stretch = (length - bandLength(focus_[index].kind)) * dance_.bands * kBandStiffness;
            ax[a] += stretch * dx / length / massOf(bodies_[a]);
            ay[a] += stretch * dy / length / massOf(bodies_[a]);
            ax[b] -= stretch * dx / length / massOf(bodies_[b]);
            ay[b] -= stretch * dy / length / massOf(bodies_[b]);
        }
    }
    // A bolha de cada corpo respira: raio pela presença, pulsando numa
    // frequência própria (potências da razão áurea, sem período comum).
    // Calculado uma vez por passo (não dentro do laço de pares).
    double bubbleRadius(std::size_t slot) const noexcept {
        const double breath = std::sin(motionTime_ * 0.37 * kGoldenPowers[slot % 5U] + static_cast<double>(slot) * 1.7);
        return (0.035 + 0.025 * std::clamp(bodies_[slot].presence, 0.0, 1.0)) * (0.55 + 0.45 * dance_.foam)
            * (1.0 + 0.35 * dance_.foam * breath);
    }
    static double bandLength(RelationKind kind) noexcept {
        switch (kind) {
        case RelationKind::Support:
        case RelationKind::Contagion: return 0.12;
        case RelationKind::Hocket:
        case RelationKind::Friction: return 0.36;
        default: return 0.22;
        }
    }

    // A forma do movimento muda a cada seção: nova pista de correntes, novo
    // pêndulo, cargas da seed.
    void reshapeMotion() noexcept {
        const auto word = mix(seed_ ^ (sectionOrdinal_ * 0x6C8E9CF5U) ^ 0x3A11CEU);
        const auto unit = [&](std::uint32_t shift) { return static_cast<double>((word >> shift) & 255U) / 255.0; };
        flowShape_ = {1.0 + 1.6 * unit(0U), -(1.0 + 1.6 * unit(8U)), 0.6 + 1.8 * unit(16U), -(0.6 + 1.8 * unit(24U))};
        flowPhase_ = unit(4U) * 6.283185307179586;
        // O desenvolvimento pendura uma cadeia de três pêndulos ligados por
        // molas (conceito do pêndulo triplo acoplado estudado no Batruc); o
        // contraste, um pêndulo só.
        chain_.fill(kBodies);
        pendulumOver_ = {};
        const std::size_t wanted = section_ == Section::Development ? kChainLength
                                 : section_ == Section::Contrast ? 1U : 0U;
        std::size_t found = 0U;
        for (std::size_t offset = 0U; offset < kBodies && found < wanted; ++offset) {
            const auto slot = (word + offset) % kBodies;
            if (bodies_[slot].lit && slot != driftSlot_) chain_[found++] = slot;
        }
    }

    // A posição do corpo na cadeia de pêndulos (kChainLength: fora dela).
    std::size_t chainLink(std::size_t slot) const noexcept {
        for (std::size_t link = 0U; link < kChainLength; ++link)
            if (chain_[link] == slot) return link;
        return kChainLength;
    }

    // Cada ataque dá um impulso no corpo: o som move a estrela. A direção
    // gira com a seed e a contagem; a força cresce com a coreografia.
    void kickOnAttack(std::size_t slot) noexcept {
        if (slot >= kBodies) return;
        const auto word = mix(seed_ ^ static_cast<std::uint32_t>(slot * 977U + attackCount_[slot] * 31U));
        const double angle = static_cast<double>(word & 1023U) / 1024.0 * 6.283185307179586;
        const double impulse = kKick * dance_.kick * familyShape(family(slot)).kick / massOf(bodies_[slot]);
        vx_[slot] += std::cos(angle) * impulse;
        vy_[slot] += std::sin(angle) * impulse;
        // O saque com efeito: top ou backspin, conforme a seed.
        spin_[slot] += ((word >> 10U) & 1U ? 1.0 : -1.0) * (0.5 + static_cast<double>((word >> 11U) & 255U) / 255.0)
            * kServeSpin * dance_.kick;
    }

    // Quanto tempo uma estrela brilha, em compassos: massa–luminosidade
    // (t ∝ M^−2,5) — pulso e tom, mais "massivos", vivem menos; texturas e
    // vozes, mais. A seed varia cada vida.
    std::uint32_t lifeBars(std::size_t slot, std::uint32_t salt) const noexcept {
        const double mass = std::max(0.3, massOf({true, bodies_[slot].star, 0.0, 0.0, 0.8}));
        const double base = 14.0 * std::pow(mass, -2.5) * 0.55;
        const auto spread = static_cast<double>(mix(seed_ ^ (salt * 0x9e3779b9U) ^ static_cast<std::uint32_t>(slot * 977U)) % 9U);
        return static_cast<std::uint32_t>(std::clamp(base + spread, 4.0, 40.0));
    }

    // A dança do brilho, a cada compasso: quem esgotou a vida apaga e
    // descansa; quem descansou reacende (as estrelas que reacendem); o céu
    // mantém entre três e seis estrelas acesas entre os corpos com fader.
    void danceBrightness(bool barTurn) noexcept {
        std::array<bool, kBodies> available{};
        std::size_t availableCount = 0U;
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            available[slot] = bodies_[slot].presence > 0.0 && PerformanceSky::isStar(bodies_[slot].star);
            availableCount += available[slot] ? 1U : 0U;
        }
        if (!brightnessReady_) {
            // Abertura (4/11): o elenco vem do caráter da abertura, como em
            // toda parte; as estrelas acendem direto, sem fade.
            if (!characterReady_) stepCharacter();
            reshapeForSection();
            for (std::size_t slot = 0U; slot < kBodies; ++slot) {
                brightness_[slot] = pinned_[slot] ? 1.0 : brightnessTarget_[slot];
                litBars_[slot] = brightnessTarget_[slot] > 0.5 ? lifeBars(slot, 1U) : 0U;
                restBars_[slot] = brightnessTarget_[slot] > 0.5 ? 0U : 2U + mix(seed_ ^ static_cast<std::uint32_t>(slot)) % 6U;
                openingLit_[slot] = brightnessTarget_[slot] > 0.5;
            }
            brightnessReady_ = true;
            return;
        }
        if (!barTurn || availableCount == 0U) return;
        const auto bar = static_cast<std::uint32_t>(beat_ / 4U);
        std::size_t litCount = 0U;
        for (std::size_t slot = 0U; slot < kBodies; ++slot)
            litCount += available[slot] && brightnessTarget_[slot] > 0.5 ? 1U : 0U;
        // Quantas estrelas soam vem do caráter da parte (4/11): o alvo da parte,
        // com folga de uma para cada lado; a capacidade medida da máquina
        // continua sendo o teto.
        const std::size_t target = character_.litTarget();
        const std::size_t maxLit = std::min({std::min<std::size_t>(7U, target + 1U), availableCount, starCapacity_});
        const std::size_t minLit = std::min({std::max<std::size_t>(2U, target > 0U ? target - 1U : 0U), availableCount, maxLit});

        // Acima da capacidade medida, apaga já quem tem menos vida pela frente.
        while (litCount > maxLit) {
            std::size_t weakest = kBodies;
            for (std::size_t slot = 0U; slot < kBodies; ++slot) {
                if (!available[slot] || brightnessTarget_[slot] < 0.5 || heldBars_[slot] > 0U) continue;
                if (weakest == kBodies || litBars_[slot] < litBars_[weakest]) weakest = slot;
            }
            if (weakest == kBodies) break;
            brightnessTarget_[weakest] = 0.0;
            restBars_[weakest] = 4U;
            --litCount;
        }
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            if (!available[slot]) continue;
            if (heldBars_[slot] > 0U) { --heldBars_[slot]; continue; }
            if (brightnessTarget_[slot] > 0.5) {
                if (litBars_[slot] > 0U) --litBars_[slot];
                if (litBars_[slot] == 0U && litCount > minLit) {
                    brightnessTarget_[slot] = 0.0;
                    --litCount;
                    restBars_[slot] = 2U + mix(seed_ ^ (bar * 131U) ^ static_cast<std::uint32_t>(slot)) % 10U;
                }
            } else if (restBars_[slot] > 0U) {
                --restBars_[slot];
            }
        }
        // Quem acende (no máximo uma por compasso): entre as que descansaram,
        // a de família mais forte no caráter da parte — ou, faltando estrelas,
        // a melhor entre todas as apagadas.
        if (litCount < maxLit) {
            std::size_t best = kBodies;
            for (std::size_t slot = 0U; slot < kBodies; ++slot) {
                if (!available[slot] || heldBars_[slot] > 0U || brightnessTarget_[slot] > 0.5) continue;
                if (restBars_[slot] > 0U && litCount >= minLit) continue;
                if (best == kBodies || castScore(slot) > castScore(best)) best = slot;
            }
            if (best < kBodies) {
                brightnessTarget_[best] = 1.0;
                litBars_[best] = lifeBars(best, bar + 7U);
                // Reacender é acontecimento: a estrela entra com um chamado
                // forte (uma "nova").
                if (influence_ > 0.0) schedule(best, 0.0, 0.95, 0.0, kMaxChainDepth);
            }
        }
    }

    // A mistura que se move com a Constelação (pedido do autor, 28/10: "uma
    // mistura de sons também dada pelas variações dos sliders do mixer",
    // "pelos movimentos do campo constelação"). O centro de massa das
    // estrelas acesas é o centro da figura; quem, na dança, se aproxima dele
    // sobe, quem se afasta desce — de 30% a 100% do seu fader.
    void mixByConstellation() noexcept {
        double cx = 0.0, cy = 0.0, total = 0.0;
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            if (bodies_[slot].presence <= 0.0 || brightnessTarget_[slot] < 0.5) continue;
            const double mass = massOf(bodies_[slot]);
            cx += bodies_[slot].liveX * mass;
            cy += bodies_[slot].liveY * mass;
            total += mass;
        }
        if (total <= 0.0) { level_.fill(1.0); return; }
        cx /= total;
        cy /= total;
        constexpr double kWidth = 0.24;
        for (std::size_t slot = 0U; slot < kBodies; ++slot) {
            const double dx = bodies_[slot].liveX - cx;
            const double dy = bodies_[slot].liveY - cy;
            const double closeness = std::exp(-(dx * dx + dy * dy) / (2.0 * kWidth * kWidth));
            level_[slot] = std::isfinite(closeness) ? 0.30 + 0.70 * closeness : 1.0;
        }
    }

    double distance(std::size_t a, std::size_t b) const noexcept {
        const double dx = x_[b] - x_[a];
        const double dy = y_[b] - y_[a];
        return std::sqrt(dx * dx + dy * dy);
    }

    // O que a estrela a sente de b: massa sobre distância ao quadrado,
    // comprimido em 0..1.
    double perception(std::size_t a, std::size_t b) const noexcept {
        const double d = distance(a, b);
        const double raw = massOf(bodies_[b]) / (d * d + 0.03);
        return raw / (raw + 6.0);
    }

    // O tipo de relação nasce das funções das duas estrelas, com a seed
    // escolhendo entre alternativas plausíveis para cada par.
    RelationKind kindFor(std::size_t a, std::size_t b) const noexcept {
        return naturalKind(seed_, a, b, bodies_[a].star, bodies_[b].star);
    }

public:
    // O ato natural do par a → b (pela função das duas estrelas e a seed);
    // público para a interface mostrar no rótulo do cabo o ato que ele pede
    // (3/11, R1). Mesmo cálculo de antes — som idêntico.
    static RelationKind naturalKind(std::uint32_t seed, std::size_t a, std::size_t b,
                                    PerformanceModuleBody starA, PerformanceModuleBody starB) noexcept {
        const auto fa = performanceStarFunction(starA);
        const auto fb = performanceStarFunction(starB);
        const auto word = mix(seed ^ static_cast<std::uint32_t>(a * 31U + b * 131U + 7U));
        const bool alternative = (word & 3U) == 0U;
        // Entre estrelas de altura (tom e voz), parte dos pares vive em atrito
        // harmônico (passo 3 da trama: a harmonia que trabalha).
        const bool tonalA = fa == PerformanceStarFunction::Tone || fa == PerformanceStarFunction::Voice;
        const bool tonalB = fb == PerformanceStarFunction::Tone || fb == PerformanceStarFunction::Voice;
        if (tonalA && tonalB && (word >> 8U) % 5U == 1U) return RelationKind::Friction;
        if (fa == PerformanceStarFunction::Pulse)
            return alternative ? RelationKind::Hocket : RelationKind::Contagion;
        if (fa == PerformanceStarFunction::Texture)
            return alternative ? RelationKind::CallResponse : RelationKind::Shadow;
        if (fb == PerformanceStarFunction::Pulse)
            return alternative ? RelationKind::Contagion : RelationKind::Hocket;
        if (fa == PerformanceStarFunction::Voice && fb == PerformanceStarFunction::Voice)
            return alternative ? RelationKind::CallResponse : RelationKind::Hocket;
        if (fb == PerformanceStarFunction::Texture)
            return alternative ? RelationKind::Support : RelationKind::Shadow;
        return alternative ? RelationKind::Support : RelationKind::CallResponse;
    }

private:
    void updateWeb() noexcept {
        for (std::size_t a = 0U; a < kBodies; ++a) {
            for (std::size_t b = 0U; b < kBodies; ++b) {
                double target = 0.0;
                if (a != b && bodies_[a].lit && bodies_[b].lit)
                    target = std::sqrt(perception(a, b) * perception(b, a));
                auto& value = intensity_[a][b];
                value += (target - value) * (target > value ? 0.08 : 0.004);
            }
        }
    }

    // A cada compasso, poucas relações em primeiro plano: as mais intensas,
    // descontadas pelo quanto já estiveram em foco há pouco.
    void chooseFocus() noexcept {
        for (auto& row : recentFocus_)
            for (auto& value : row) value *= 0.7;
        tension_ *= 0.92;
        const auto draw = mix(seed_ ^ (beat_ * 0x9e3779b9U));
        // A seção decide quantas relações cabem em primeiro plano.
        const std::uint32_t wanted = section_ == Section::Opening ? 1U
            : section_ == Section::Return ? 1U + draw % 2U
            : 2U + draw % 2U;
        focusCount_ = 0U;
        std::array<std::array<bool, kBodies>, kBodies> taken{};
        for (std::size_t round = 0U; round < wanted; ++round) {
            double best = 0.05;
            Focus chosen{};
            for (std::size_t a = 0U; a < kBodies; ++a) {
                for (std::size_t b = 0U; b < kBodies; ++b) {
                    if (a == b || taken[a][b] || !bodies_[a].lit || !bodies_[b].lit) continue;
                    const double novelty = 1.0 / (1.0 + recentFocus_[a][b] * 6.0);
                    const double jitter = 0.85 + 0.30 * static_cast<double>(
                        mix(seed_ ^ static_cast<std::uint32_t>(beat_ * 97U + a * 13U + b)) & 255U) / 255.0;
                    // A raiz comprime a vantagem dos pares mais próximos, para
                    // que a novidade possa trazer relações mais distantes.
                    // Um cabo desenhado puxa o par para o foco (a direção
                    // desenhada pesa mais que a inversa).
                    const double drawn = std::max(cable_[a][b], cable_[b][a] * 0.5);
                    const double score = std::sqrt(intensity_[a][b]) * novelty * jitter * (1.0 + 1.5 * drawn);
                    if (score <= best) continue;
                    best = score;
                    const auto phenomenon = cablePhenomenon_[a][b] != kNoPhenomenon ? cablePhenomenon_[a][b]
                                                                                    : cablePhenomenon_[b][a];
                    chosen = {a, b, strategicKind(a, b, phenomenon != kNoPhenomenon
                                                            ? phenomenonKind(phenomenon, kindFor(a, b))
                                                            : kindFor(a, b))};
                }
            }
            if (chosen.caller >= kBodies) break;
            taken[chosen.caller][chosen.responder] = true;
            recentFocus_[chosen.caller][chosen.responder] += 1.0;
            focus_[focusCount_++] = chosen;
        }
        // Cadência: com a tensão alta, a primeira relação entre estrelas de
        // altura vira reparo — a resposta vai à fundamental ou à quinta.
        // Negociar busca o acordo: a cadência chega com menos tensão.
        const double cadenceThreshold = strategy_ == PerformanceGameStrategy::Negotiate ? 0.4 : 0.6;
        if (tension_ > cadenceThreshold) {
            for (std::size_t index = 0U; index < focusCount_; ++index) {
                const auto responder = performanceStarFunction(bodies_[focus_[index].responder].star);
                if (responder == PerformanceStarFunction::Tone || responder == PerformanceStarFunction::Voice) {
                    focus_[index].kind = RelationKind::Repair;
                    break;
                }
            }
        }
    }

    // Intervalos de transformação da altura ouvida: a resposta nunca é cópia
    // literal; o par e a contagem de ataques escolhem a transformação.
    double transformed(std::size_t a, std::size_t b, double heard) const noexcept {
        static constexpr std::array<double, 8> ratios{{1.0, 1.5, 4.0 / 3.0, 0.75, 1.2, 5.0 / 6.0, 2.0, 0.5}};
        const auto word = mix(seed_ ^ static_cast<std::uint32_t>(a * 17U + b * 101U + attackCount_[a] * 7U));
        return heard * ratios[word % ratios.size()];
    }

    void schedule(std::size_t slot, double pitchHz, double strength, double delayBeats,
                  std::uint8_t depth, bool chromatic = false) noexcept {
        if (cueCount_ >= cues_.size() || slot >= kBodies) return;
        const auto delay = static_cast<std::uint64_t>(std::max(0.0, delayBeats) * samplesPerBeat_);
        const auto due = sample_ + delay;
        // Intervalo mínimo de um oitavo de tempo entre chamados à mesma estrela.
        const auto spacing = static_cast<std::uint64_t>(samplesPerBeat_ * 0.125);
        if (lastCueSample_[slot] > 0U && due < lastCueSample_[slot] + spacing
            && due + spacing > lastCueSample_[slot]) return;
        lastCueSample_[slot] = due;
        cues_[cueCount_++] = {slot, pitchHz, std::clamp(strength * influence_, 0.0, 1.0), due, depth, chromatic};
    }

public:
    // As transformações de um motivo (desenvolvimento motívico): cada resposta
    // de um mesmo par usa a seguinte, para que a resposta nunca seja cópia.
    enum class MotifTransform : std::uint8_t {
        Transpose, Invert, Retrograde, Augment, Diminish, Fragment, Count
    };
    MotifTransform lastTransform() const noexcept { return lastTransform_; }

private:
    MotifTransform lastTransform_ = MotifTransform::Transpose;

    void scheduleMotif(std::size_t a, std::size_t b, double strength, double startBeats,
                       std::uint8_t depth, bool shadow) noexcept {
        const auto available = static_cast<std::size_t>(motifCount_[a]);
        const auto take = std::min<std::size_t>(available,
            2U + mix(seed_ ^ static_cast<std::uint32_t>(a * 13U + b * 5U + pairResponses_[a][b])) % 3U);
        std::array<MotifNote, kMotifLength> notes{};
        for (std::size_t index = 0U; index < take; ++index) {
            const auto ring = (motifHead_[a] + kMotifLength - take + index) % kMotifLength;
            notes[index] = motif_[a][ring];
        }
        auto transform = static_cast<MotifTransform>((pairResponses_[a][b]
            + mix(seed_ ^ static_cast<std::uint32_t>(a * 977U + b)) ) % static_cast<std::uint32_t>(MotifTransform::Count));
        ++pairResponses_[a][b];
        if (shadow) transform = MotifTransform::Augment;
        // No retorno, o motivo volta reconhecível: só transposto.
        if (section_ == Section::Return && !shadow) transform = MotifTransform::Transpose;
        lastTransform_ = transform;
        std::size_t count = take;
        const double first = notes[0].pitchHz;
        switch (transform) {
        case MotifTransform::Transpose: {
            const double ratio = transformed(a, b, 1.0);
            for (std::size_t index = 0U; index < count; ++index) notes[index].pitchHz *= ratio;
            break;
        }
        case MotifTransform::Invert:
            // Espelho em torno da primeira nota (em escala logarítmica).
            for (std::size_t index = 0U; index < count; ++index)
                notes[index].pitchHz = first * first / std::max(notes[index].pitchHz, 1.0);
            break;
        case MotifTransform::Retrograde:
            for (std::size_t index = 0U; index < count / 2U; ++index)
                std::swap(notes[index].pitchHz, notes[count - 1U - index].pitchHz);
            break;
        case MotifTransform::Augment:
            for (std::size_t index = 0U; index < count; ++index) notes[index].ioiBeats *= 2.0;
            break;
        case MotifTransform::Diminish:
            for (std::size_t index = 0U; index < count; ++index) notes[index].ioiBeats *= 0.5;
            break;
        case MotifTransform::Fragment:
            count = std::min<std::size_t>(count, 2U);
            break;
        default:
            break;
        }
        double when = startBeats;
        for (std::size_t index = 0U; index < count; ++index) {
            if (index > 0U) when += std::clamp(notes[index].ioiBeats, 0.125, 4.0);
            const double pitch = std::clamp(notes[index].pitchHz * (shadow ? 0.5 : 1.0), 30.0, 2000.0);
            schedule(b, pitch, strength, when, depth);
        }
    }

    void respond(const Focus& focus, double heard, std::uint8_t depth) noexcept {
        const auto a = focus.caller;
        const auto b = focus.responder;
        const double strength = 0.35 + intensity_[a][b] * 0.65;
        // "Tempo-luz": quanto mais longe, mais tarde a resposta chega.
        const double lightBeats = 0.25 + distance(a, b) * 2.0;
        switch (focus.kind) {
        case RelationKind::CallResponse:
            // Responde à frase, não a cada nota: quando a frase de quem chama
            // fecha, devolve um fragmento do motivo, transformado. Sem motivo
            // ainda, responde à nota ouvida.
            if (motifCount_[a] >= 3U) {
                const auto phrase = 3U + mix(seed_ ^ static_cast<std::uint32_t>(a * 7U + b)) % 3U;
                if (attackCount_[a] % phrase == 0U)
                    scheduleMotif(a, b, strength, lightBeats, depth, false);
            } else {
                schedule(b, transformed(a, b, heard), strength, lightBeats, depth);
            }
            break;
        case RelationKind::Contagion:
            // O ritmo passa adiante; a altura é a do próprio corpo.
            schedule(b, 0.0, strength, 0.0, depth);
            break;
        case RelationKind::Shadow:
            // Aumentação: o motivo volta mais tarde, mais lento e uma oitava
            // abaixo, uma vez por frase de quem chama.
            if (motifCount_[a] >= 3U) {
                if (attackCount_[a] % 4U == 0U) scheduleMotif(a, b, strength * 0.8, lightBeats * 2.0, depth, true);
            } else {
                schedule(b, heard * 0.5, strength * 0.8, lightBeats * 2.0, depth);
            }
            break;
        case RelationKind::Hocket: {
            // No vão: meio período dominante de quem chama, quando o ritmo
            // dele é claro (Fourier); senão, meio vão entre os dois últimos
            // ataques.
            const auto gap = lastAttackSample_[a] - previousAttackSample_[a];
            double gapBeats = previousAttackSample_[a] > 0U
                ? std::clamp(static_cast<double>(gap) / samplesPerBeat_, 0.125, 2.0) : 0.5;
            if (rhythm_[a].clarity >= 0.5) gapBeats = static_cast<double>(rhythm_[a].period) * 0.25;
            schedule(b, 0.0, strength, gapBeats * 0.5, depth);
            break;
        }
        case RelationKind::Friction:
            // Meio-tom acima do que ouviu, fora da escala: a tensão cresce.
            // Com o ritmo de quem chama claro, o atrito também é métrico:
            // entra a 1,5 período (a hemíola, três contra dois).
            schedule(b, heard * (16.0 / 15.0), strength,
                     rhythm_[a].clarity >= 0.5 ? static_cast<double>(rhythm_[a].period) * 0.25 * 1.5
                                               : lightBeats * 0.5,
                     depth, true);
            tension_ = std::min(1.0, tension_ + 0.12);
            break;
        case RelationKind::Repair: {
            // A cadência: a fundamental (ou a quinta, em alternância) na
            // oitava mais próxima do que ouviu; a tensão se desfaz.
            const double target = tonalCenterHz_ * std::pow(2.0, regionSemitones() / 12.0)
                * (attackCount_[a] % 2U == 0U ? 1.0 : 1.5);
            const double octaves = std::round(std::log2(std::max(heard, 1.0) / target));
            schedule(b, target * std::pow(2.0, octaves), strength, lightBeats, depth);
            tension_ = std::max(0.0, tension_ - 0.35);
            break;
        }
        case RelationKind::Support:
            // Pedal grave a cada quatro ataques de quem chama.
            if (attackCount_[a] % 4U == 0U) schedule(b, heard * 0.25, strength, 0.0, depth);
            break;
        default:
            break;
        }
    }
};

} // namespace rasgo::performance
