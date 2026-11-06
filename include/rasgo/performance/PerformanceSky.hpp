#pragma once

// O céu do Asterismus: cada módulo é uma estrela independente.
//
// Decisão do autor (2026-10-28): "cada módulo como sendo uma estrela, que
// podem ser acionados ou não no asterismo". Esta camada substitui, para o
// som, as cinco coortes fixas de PerformanceModuleCohortAdapter: ali os
// módulos só existiam em trios sorteados por seed % 5, presos a três lanes e
// a estados compartilhados (um passo comum, misturas cruzadas Acid×Gendyn).
//
// Aqui cada estrela tem seu DSP próprio e só gasta CPU quando acesa (quando
// renderStar é chamado). O que as estrelas compartilham é o tempo do céu —
// três relógios (pulso de circuito, frase de sopro/voz, memória), o Rungler e
// a fonte aleatória Buchla —, avançados uma vez por amostra em advance(),
// acesas ou não, para que uma estrela acesa no meio da peça entre no tempo.
// A saída de uma estrela não depende de quais outras estão acesas; o
// acoplamento entre estrelas pertence às relações da ecologia, não ao DSP.
//
// Proveniência: o código de cada estrela é o da sua família de origem em
// PerformanceModuleCohortAdapter.hpp (commit 244654a deste repositório), com
// os controles da própria estrela no lugar dos controles da vaga da coorte.
// Diferenças intencionais, todas registradas em docs/DIAGNOSTICO_2026-10-28.md:
// - os termos cruzados Acid×Gendyn (crossMutation, gendyn no Acid, acid no
//   Gendyn, gendyn nas Drums) saíram: eram acoplamento escondido entre lanes;
// - os relógios compartilhados de frase e memória usavam um controle de uma
//   vaga (VOSIM, Galo) para todos; aqui usam o valor neutro dessa conta;
// - cada estrela recebe a sua frequência (a do seu corpo no Campo).

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string_view>

#include "PerformanceModuleTypes.hpp"
#include "PerformanceSeedTrajectory.hpp"

// Os pacotes de DSP copiados do Studio (vendor/studio-dsp, com proveniência)
// ainda são ligados por quatro alvos de CMake herdados das antigas coortes;
// o céu só existe com os quatro presentes.
#if defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V1) \
    && defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V2) \
    && defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V3) \
    && defined(RASGO_PERFORMANCE_WITH_MODULE_COHORT_V4)
#define RASGO_PERFORMANCE_SKY_AVAILABLE 1
#include <dsp/AcidBasslineVoice.hpp>
#include <dsp/AtariPunkConsole.hpp>
#include <dsp/AtropeloVoice.hpp>
#include <dsp/BuchlaRandomSource.hpp>
#include <dsp/ChoirEnsemble.hpp>
#include <dsp/DejaVuVoice.hpp>
#include <dsp/DrumVoices.hpp>
#include <dsp/FmFluteVoice.hpp>
#include <dsp/GaloVoice.hpp>
#include <dsp/GendynOscillator.hpp>
#include <dsp/HardSyncVoice.hpp>
#include <dsp/PolyrhythmDivider.hpp>
#include <dsp/Rungler.hpp>
#include <dsp/ScatVoice.hpp>
#include <dsp/ThereminVoice.hpp>
#include "TubaBreathVoice.hpp"
#include <dsp/FootnotePluck.hpp>
#include <dsp/AY8910Chip.hpp>
#include <dsp/MorphOscillator.hpp>
#include <dsp/RosslerMap.hpp>
#include <dsp/EnvelopeLoopPulse.hpp>
#include "PerformanceStudioVoices.hpp"
#include "PerformanceBluffSignal.hpp"
#include "PerformanceSpectralObject.hpp"
#include "PerformanceHarmonyStars.hpp"
#include "PerformanceStudioConceptStars.hpp"
#include <dsp/VosimOscillator.hpp>
#include <dsp/WalshOscillator.hpp>
#endif

namespace rasgo::performance {

// As 14 estrelas que a seed escolhia até 3/11/2026 (histórico: as tomadas
// gravadas antes disso nasceram desta lista). Desde 3/11 a seed escolhe
// entre todas as estrelas do céu (kPerformanceSkyStars) — decisão do autor:
// "é um instrumento generativo que oferta um bom leque de variações".
inline constexpr std::array<PerformanceModuleBody, 14> kPerformanceSeedStars{{
    PerformanceModuleBody::Acid, PerformanceModuleBody::Gendyn,
    PerformanceModuleBody::Percussion, PerformanceModuleBody::FmFlute,
    PerformanceModuleBody::Theremin, PerformanceModuleBody::Scat,
    PerformanceModuleBody::Choir, PerformanceModuleBody::Vosim,
    PerformanceModuleBody::Walsh, PerformanceModuleBody::HardSync,
    PerformanceModuleBody::AtariPunk, PerformanceModuleBody::DejaVu,
    PerformanceModuleBody::Atropelo, PerformanceModuleBody::Galo}};

// As estrelas que o céu sabe tocar — e, desde 3/11/2026, as que a seed
// escolhe: as 14 primeiras e as da biblioteca (30/10 a 2/11). A ordem não
// muda: mudá-la mudaria o asterismo de toda seed.
inline constexpr std::array<PerformanceModuleBody, 28> kPerformanceSkyStars{{
    PerformanceModuleBody::Acid, PerformanceModuleBody::Gendyn,
    PerformanceModuleBody::Percussion, PerformanceModuleBody::FmFlute,
    PerformanceModuleBody::Theremin, PerformanceModuleBody::Scat,
    PerformanceModuleBody::Choir, PerformanceModuleBody::Vosim,
    PerformanceModuleBody::Walsh, PerformanceModuleBody::HardSync,
    PerformanceModuleBody::AtariPunk, PerformanceModuleBody::DejaVu,
    PerformanceModuleBody::Atropelo, PerformanceModuleBody::Galo,
    PerformanceModuleBody::Tuba, PerformanceModuleBody::Footnote, PerformanceModuleBody::Bluff,
    PerformanceModuleBody::Spectral, PerformanceModuleBody::Sieve, PerformanceModuleBody::Polymeter,
    PerformanceModuleBody::Hocket, PerformanceModuleBody::Rupture, PerformanceModuleBody::Harmonaig,
    PerformanceModuleBody::Bloom, PerformanceModuleBody::ThermalStatic, PerformanceModuleBody::Core,
    PerformanceModuleBody::Texture, PerformanceModuleBody::BassFallback}};

// As estrelas trazidas do Studio em 6/11 (auditoria dos módulos, passo 2):
// AY-8910, MORPH, RÖSSLER, ENVELOPE PULSE. Ficam fora de kPerformanceSkyStars
// de propósito — o sorteio das 8 estrelas de cada seed (a ordem acima) não
// muda, e as seeds da 0.2.0 continuam com o mesmo céu. Tocam pela BIBLIOTECA
// e como convidadas do contraste.
inline constexpr std::array<PerformanceModuleBody, 6> kPerformanceExtraStars{{
    PerformanceModuleBody::AyChip, PerformanceModuleBody::Morph,
    PerformanceModuleBody::Rossler, PerformanceModuleBody::EnvelopePulse,
    PerformanceModuleBody::Granular, PerformanceModuleBody::Turing}};
// Todas as estrelas que o céu sabe tocar: as 28 do sorteio e as de fora.
inline constexpr std::array<PerformanceModuleBody, 34> kPerformanceAllStars{{
    PerformanceModuleBody::Acid, PerformanceModuleBody::Gendyn,
    PerformanceModuleBody::Percussion, PerformanceModuleBody::FmFlute,
    PerformanceModuleBody::Theremin, PerformanceModuleBody::Scat,
    PerformanceModuleBody::Choir, PerformanceModuleBody::Vosim,
    PerformanceModuleBody::Walsh, PerformanceModuleBody::HardSync,
    PerformanceModuleBody::AtariPunk, PerformanceModuleBody::DejaVu,
    PerformanceModuleBody::Atropelo, PerformanceModuleBody::Galo,
    PerformanceModuleBody::Tuba, PerformanceModuleBody::Footnote, PerformanceModuleBody::Bluff,
    PerformanceModuleBody::Spectral, PerformanceModuleBody::Sieve, PerformanceModuleBody::Polymeter,
    PerformanceModuleBody::Hocket, PerformanceModuleBody::Rupture, PerformanceModuleBody::Harmonaig,
    PerformanceModuleBody::Bloom, PerformanceModuleBody::ThermalStatic, PerformanceModuleBody::Core,
    PerformanceModuleBody::Texture, PerformanceModuleBody::BassFallback,
    PerformanceModuleBody::AyChip, PerformanceModuleBody::Morph,
    PerformanceModuleBody::Rossler, PerformanceModuleBody::EnvelopePulse,
    PerformanceModuleBody::Granular, PerformanceModuleBody::Turing}};

// A função musical de cada estrela, a mesma do catálogo
// (PerformanceModuleLibrary): é ela que dá a cada corpo do primeiro
// asterismo um lugar audível na figura.
// Control (2/11): estrelas que não soam — filtram os ataques das outras.
enum class PerformanceStarFunction : std::uint8_t { Tone, Texture, Voice, Pulse, Control };

inline constexpr PerformanceStarFunction performanceStarFunction(
    PerformanceModuleBody body) noexcept {
    switch (body) {
    case PerformanceModuleBody::Gendyn:
    case PerformanceModuleBody::DejaVu:
    case PerformanceModuleBody::Spectral:
    case PerformanceModuleBody::ThermalStatic:
    case PerformanceModuleBody::Rossler:
    case PerformanceModuleBody::Granular:
    case PerformanceModuleBody::Texture: return PerformanceStarFunction::Texture;
    case PerformanceModuleBody::Sieve:
    case PerformanceModuleBody::Polymeter:
    case PerformanceModuleBody::Hocket:
    case PerformanceModuleBody::Rupture: return PerformanceStarFunction::Control;
    case PerformanceModuleBody::Scat:
    case PerformanceModuleBody::Choir:
    case PerformanceModuleBody::Atropelo: return PerformanceStarFunction::Voice;
    case PerformanceModuleBody::Percussion:
    case PerformanceModuleBody::AtariPunk:
    case PerformanceModuleBody::Bluff: return PerformanceStarFunction::Pulse;
    default: return PerformanceStarFunction::Tone;
    }
}

// O primeiro asterismo que a seed propõe para os oito corpos do Campo
// (2026-10-28, passo 3: "a seed escolhe corpos, não coortes"). A seed
// embaralha as 28 estrelas do céu (até 3/11, só as 14 primeiras); a figura de abertura pede, nos papéis do
// Campo, um chão tonal (corpo 1), uma matéria de atrito — textura ou voz —
// (corpo 2) e um pulso (corpo 3); o corpo 4 e os corpos 6..8 recebem as
// estrelas seguintes da ordem da seed. Qualquer estrela pode encontrar
// qualquer outra: não há mais famílias fechadas (seed % 5). O corpo 5 é o
// Modal, que ainda toca pelo ModalMatterAdapter. Nenhuma estrela acende duas
// vezes. No máximo uma estrela de CONTROLE por céu (elas não soam: duas ou
// três deixariam o céu ralo) — a única regra acrescentada em 3/11.
inline constexpr std::array<PerformanceModuleBody, 8> performanceInitialAsterism(
    std::uint32_t seed) noexcept {
    std::array<PerformanceModuleBody, kPerformanceSkyStars.size()> order = kPerformanceSkyStars;
    std::uint32_t state = seed ^ 0x2545f491U;
    for (std::size_t index = order.size() - 1U; index > 0U; --index) {
        state ^= state << 13U;
        state ^= state >> 17U;
        state ^= state << 5U;
        const auto other = static_cast<std::size_t>(state % static_cast<std::uint32_t>(index + 1U));
        const auto held = order[index];
        order[index] = order[other];
        order[other] = held;
    }
    std::array<bool, kPerformanceSkyStars.size()> used{};
    bool controlTaken = false;
    const auto take = [&order, &used, &controlTaken](auto accepts) {
        const auto admit = [&controlTaken](PerformanceModuleBody star) {
            return !controlTaken || performanceStarFunction(star) != PerformanceStarFunction::Control;
        };
        const auto mark = [&controlTaken](PerformanceModuleBody star) {
            controlTaken = controlTaken || performanceStarFunction(star) == PerformanceStarFunction::Control;
            return star;
        };
        for (std::size_t index = 0U; index < order.size(); ++index) {
            if (used[index] || !admit(order[index]) || !accepts(performanceStarFunction(order[index]))) continue;
            used[index] = true;
            return mark(order[index]);
        }
        for (std::size_t index = 0U; index < order.size(); ++index) {
            if (used[index] || !admit(order[index])) continue;
            used[index] = true;
            return mark(order[index]);
        }
        return PerformanceModuleBody::Count;
    };
    std::array<PerformanceModuleBody, 8> result{};
    result[0] = take([](PerformanceStarFunction f) { return f == PerformanceStarFunction::Tone; });
    result[1] = take([](PerformanceStarFunction f) {
        return f == PerformanceStarFunction::Texture || f == PerformanceStarFunction::Voice; });
    result[2] = take([](PerformanceStarFunction f) { return f == PerformanceStarFunction::Pulse; });
    result[3] = take([](PerformanceStarFunction) { return true; });
    result[4] = PerformanceModuleBody::ModalResonator;
    for (std::size_t index = 5U; index < result.size(); ++index)
        result[index] = take([](PerformanceStarFunction) { return true; });
    return result;
}

// O id de catálogo (PerformanceModuleLibrary) de cada estrela.
inline constexpr std::string_view performanceStarLibraryId(PerformanceModuleBody body) noexcept {
    switch (body) {
    case PerformanceModuleBody::Acid: return "acid";
    case PerformanceModuleBody::Gendyn: return "gendyn";
    case PerformanceModuleBody::Percussion: return "drums";
    case PerformanceModuleBody::FmFlute: return "fm-flute";
    case PerformanceModuleBody::Theremin: return "theremin";
    case PerformanceModuleBody::Scat: return "scat";
    case PerformanceModuleBody::Choir: return "choir";
    case PerformanceModuleBody::Vosim: return "vosim";
    case PerformanceModuleBody::Walsh: return "walsh";
    case PerformanceModuleBody::HardSync: return "hard-sync";
    case PerformanceModuleBody::AtariPunk: return "atari-punk";
    case PerformanceModuleBody::DejaVu: return "deja-vu";
    case PerformanceModuleBody::Atropelo: return "atropelo";
    case PerformanceModuleBody::Galo: return "galo";
    case PerformanceModuleBody::ModalResonator: return "modal-resonator";
    case PerformanceModuleBody::Tuba: return "tuba";
    case PerformanceModuleBody::Footnote: return "footnote";
    case PerformanceModuleBody::Bluff: return "bluff";
    case PerformanceModuleBody::Spectral: return "spectral";
    case PerformanceModuleBody::Sieve: return "sieve";
    case PerformanceModuleBody::Polymeter: return "polymeter";
    case PerformanceModuleBody::Hocket: return "hocket";
    case PerformanceModuleBody::Rupture: return "rupture";
    case PerformanceModuleBody::Harmonaig: return "harmonaig";
    case PerformanceModuleBody::Bloom: return "bloom";
    case PerformanceModuleBody::ThermalStatic: return "thermal-static";
    case PerformanceModuleBody::Core: return "core";
    case PerformanceModuleBody::Texture: return "texture";
    case PerformanceModuleBody::BassFallback: return "bass-fallback";
    case PerformanceModuleBody::AyChip: return "ay-8910";
    case PerformanceModuleBody::Morph: return "morph";
    case PerformanceModuleBody::Rossler: return "rossler";
    case PerformanceModuleBody::EnvelopePulse: return "envelope-pulse";
    case PerformanceModuleBody::Granular: return "granular";
    case PerformanceModuleBody::Turing: return "turing";
    default: return {};
    }
}

// O que cada estrela faz, em poucas palavras (a função do descritor da sua
// família de origem).
inline constexpr std::string_view performanceStarRole(PerformanceModuleBody body) noexcept {
    switch (body) {
    case PerformanceModuleBody::Acid: return "corpo tonal e trajetória";
    case PerformanceModuleBody::Gendyn: return "matéria estocástica";
    case PerformanceModuleBody::Percussion: return "pulso e contágio";
    case PerformanceModuleBody::FmFlute: return "sopro articulado";
    case PerformanceModuleBody::Theremin: return "linha contínua";
    case PerformanceModuleBody::Scat: return "sílabas sintéticas";
    case PerformanceModuleBody::Choir: return "campo vocal polifônico";
    case PerformanceModuleBody::Vosim: return "formante pulsado";
    case PerformanceModuleBody::Walsh: return "base ortogonal e sequência";
    case PerformanceModuleBody::HardSync: return "ruptura harmônica";
    case PerformanceModuleBody::AtariPunk: return "pulso escalonado";
    case PerformanceModuleBody::DejaVu: return "memória estatística";
    case PerformanceModuleBody::Atropelo: return "voz quase falada";
    case PerformanceModuleBody::Galo: return "chamada ressonante";
    case PerformanceModuleBody::ModalResonator: return "memória e ressonância";
    case PerformanceModuleBody::Tuba: return "sopro grave modal";
    case PerformanceModuleBody::Footnote: return "nota de rodapé inarmônica";
    case PerformanceModuleBody::Bluff: return "sinal, blefe e confiança";
    case PerformanceModuleBody::Spectral: return "objeto espectral: altura é ritmo";
    case PerformanceModuleBody::Sieve: return "peneira de Xenakis nos ataques do céu";
    case PerformanceModuleBody::Polymeter: return "cada estrela no seu próprio ciclo";
    case PerformanceModuleBody::Hocket: return "hoquetus: ninguém ataca duas vezes seguidas";
    case PerformanceModuleBody::Rupture: return "ruptura: o céu se cala de repente";
    case PerformanceModuleBody::Harmonaig: return "acorde de quatro vozes que anda em quartas";
    case PerformanceModuleBody::Bloom: return "melodia fractal: tronco e galho";
    case PerformanceModuleBody::ThermalStatic: return "vida térmica: esquenta, morre, volta";
    case PerformanceModuleBody::Core: return "voz de FM cruzado na frase do céu";
    case PerformanceModuleBody::Texture: return "cama de ruído colorido";
    case PerformanceModuleBody::BassFallback: return "baixo fraseado";
    case PerformanceModuleBody::AyChip: return "chip de som de 8 bits: três tons, ruído e envelope";
    case PerformanceModuleBody::Morph: return "oscilador que passa entre formas de onda";
    case PerformanceModuleBody::Rossler: return "atrator caótico de Rössler, com erupções";
    case PerformanceModuleBody::EnvelopePulse: return "pulso de envelope em laço";
    case PerformanceModuleBody::Granular: return "nuvem de grãos do passado recente";
    case PerformanceModuleBody::Turing: return "melodia em laço que muda aos poucos";
    default: return {};
    }
}

// Os nomes dos três controles de cada estrela, os mesmos da sua família.
inline constexpr std::array<const char*, 3> performanceStarControlNames(
    PerformanceModuleBody body) noexcept {
    switch (body) {
    case PerformanceModuleBody::Acid: return {"CORTE", "RESO", "ARTIC"};
    case PerformanceModuleBody::Gendyn: return {"PASSO", "TEMPO", "FLUXO"};
    case PerformanceModuleBody::Percussion: return {"ATIV.", "FRAG.", "ABERT."};
    case PerformanceModuleBody::FmFlute: return {"CORPO", "BRILHO", "FRASE"};
    case PerformanceModuleBody::Theremin: return {"ALTURA", "PRES.", "DERIVA"};
    case PerformanceModuleBody::Scat: return {"CORPO", "VOGAL", "FRASE"};
    case PerformanceModuleBody::Choir: return {"CORPO", "ABERT.", "FRASE"};
    case PerformanceModuleBody::Vosim: return {"PULSO", "FORM.", "DECAY"};
    case PerformanceModuleBody::Walsh: return {"ALTURA", "SEQUÊNCIA", "CORTE"};
    case PerformanceModuleBody::HardSync: return {"RAZÃO", "CORPO", "VARRED."};
    case PerformanceModuleBody::AtariPunk: return {"CLOCK", "PULSO", "ATAQUE"};
    case PerformanceModuleBody::DejaVu: return {"LEMBRANÇA", "BRILHO", "DURAÇÃO"};
    case PerformanceModuleBody::Atropelo: return {"ALTURA", "CORPO", "SÍLABA"};
    case PerformanceModuleBody::Galo: return {"CHAMADA", "REGISTRO", "CAUDA"};
    case PerformanceModuleBody::ModalResonator: return {"CORPO", "BRILHO", "CAUDA"};
    case PerformanceModuleBody::Tuba: return {"ALTURA", "SOPRO", "BRILHO"};
    case PerformanceModuleBody::Footnote: return {"ALTURA", "INARMONIA", "DECAIM."};
    case PerformanceModuleBody::Bluff: return {"VERDADE", "JANELA", "ALTURA"};
    case PerformanceModuleBody::Spectral: return {"ALTURA", "INARMONIA", "PERÍODO"};
    case PerformanceModuleBody::Sieve: return {"MÓDULO A", "MÓDULO B", "RESÍDUO"};
    case PerformanceModuleBody::Polymeter: return {"CICLO", "PULSOS", "ESPALHA"};
    case PerformanceModuleBody::Hocket: return {"GRUPO", "PACIÊNCIA", "ALCANCE"};
    case PerformanceModuleBody::Rupture: return {"PERÍODO", "DURAÇÃO", "CHANCE"};
    case PerformanceModuleBody::Harmonaig: return {"INVERSÃO", "ABERTURA", "MOVIMENTO"};
    case PerformanceModuleBody::Bloom: return {"CAMINHO", "REGISTRO", "DURAÇÃO"};
    case PerformanceModuleBody::ThermalStatic: return {"CALOR", "MORTE", "ESTÁTICA"};
    case PerformanceModuleBody::Core: return {"FM", "RAZÃO", "DECAIM."};
    case PerformanceModuleBody::Texture: return {"COR", "FOCO", "MOVIMENTO"};
    case PerformanceModuleBody::BassFallback: return {"REGISTRO", "ATAQUE", "DURAÇÃO"};
    case PerformanceModuleBody::AyChip: return {"ARPEJO", "RUÍDO", "ENVELOPE"};
    case PerformanceModuleBody::Morph: return {"FORMA", "DERIVA", "DURAÇÃO"};
    case PerformanceModuleBody::Rossler: return {"TAXA", "CAOS", "BRILHO"};
    case PerformanceModuleBody::EnvelopePulse: return {"VELOCIDADE", "DECAIM.", "ALTURA"};
    case PerformanceModuleBody::Granular: return {"DENSIDADE", "ESPALHAR", "ALTURA"};
    case PerformanceModuleBody::Turing: return {"MUDANÇA", "FORMA", "DURAÇÃO"};
    default: return {"I", "II", "III"};
    }
}

// O estado comum do céu num instante: tempo, campo de forças e gestos.
struct PerformanceSkyContext {
    double tempoBpm = 118.0;
    double density = 0.5;
    double tension = 0.5;
    double memory = 0.5;
    double risk = 0.3;
    double intervention = 0.5;
    double time = 0.0;
    double matter = 0.0;
    double friction = 0.0;
    double mutate = 0.0;
    double tear = 0.0;
    double repair = 0.0;
    double autonomy = 0.5;
    bool holding = false;
};

class PerformanceSky {
public:
    static constexpr std::size_t kStarCount = kPerformanceAllStars.size();
    // As máscaras por estrela (renderizada, atacou) têm 32 bits: até 16
    // estrelas cabiam em 16, e a 17ª (Bluff, 30/10) caía fora — sem ataque e
    // com o DSP avançando duas vezes na mesma amostra.
    static_assert(kPerformanceAllStars.size() <= 64U, "star masks are 64 bits");
    using StarControls = std::array<double, 3>;

    static constexpr bool available() noexcept {
#if defined(RASGO_PERFORMANCE_SKY_AVAILABLE)
        return true;
#else
        return false;
#endif
    }

    static constexpr std::size_t starIndex(PerformanceModuleBody body) noexcept {
        for (std::size_t index = 0U; index < kPerformanceAllStars.size(); ++index)
            if (kPerformanceAllStars[index] == body) return index;
        return kStarCount;
    }
    static constexpr bool isStar(PerformanceModuleBody body) noexcept {
        return starIndex(body) < kStarCount;
    }

    bool prepare(double sampleRate) noexcept {
        if (!std::isfinite(sampleRate) || sampleRate < 8000.0 || sampleRate > 384000.0)
            return false;
        sampleRate_ = sampleRate;
#if defined(RASGO_PERFORMANCE_SKY_AVAILABLE)
        prepareStars();
        prepared_ = true;
        configureSeed(seed_);
#endif
        return true;
    }

    void reset() noexcept {
#if defined(RASGO_PERFORMANCE_SKY_AVAILABLE)
        if (prepared_) {
            prepareStars();
            configureSeed(seed_);
        }
#endif
    }

    // O kit da DRUMS desta seed (bits: 1 cowbell, 2 palmas, 4 prato, 8 maracas).
    std::uint8_t drumKit() const noexcept { return drumKit_; }
    void setSeed(std::uint32_t seed) noexcept {
        seed_ = seed;
        seedMix_ = performanceSeedStream(seed, 0x5EEDU);
#if defined(RASGO_PERFORMANCE_SKY_AVAILABLE)
        if (prepared_) configureSeed(seed_);
#endif
    }
    std::uint32_t seed() const noexcept { return seed_; }

    // O tempo dentro do compasso (29/10). Swing kepleriano: os 4 passos de cada
    // tempo são ângulos iguais de uma órbita; o instante de cada passo sai da
    // equação de Kepler (M = E - e·sen E), de modo que a excentricidade é a
    // quantidade de swing (0 = reto) e o periélio decide qual passo aperta.
    // Groove: um desvio pequeno por posição dos 16 passos, gerado pela seed
    // (o conceito de microtempo por posição medido no Trioio, Groove.h,
    // commit c73ede4; Danielsen 2010; Benadon 2006 para o swing como razão de
    // duração). Com excentricidade e groove zero, o relógio é o de antes.
    void setTiming(double eccentricity, double perihelion, double grooveDepth) noexcept {
        eccentricity = std::isfinite(eccentricity) ? std::clamp(eccentricity, 0.0, 0.6) : 0.0;
        perihelion = std::isfinite(perihelion) ? perihelion : 0.0;
        grooveDepth = std::isfinite(grooveDepth) ? std::clamp(grooveDepth, 0.0, 1.0) : 0.0;
        if (eccentricity == eccentricity_ && perihelion == perihelion_ && grooveDepth == grooveDepth_) return;
        eccentricity_ = eccentricity;
        perihelion_ = perihelion;
        grooveDepth_ = grooveDepth;
        rebuildStepTimes();
    }
    // Contagens variáveis (29/10): sobre o pulso comum, cada estrela de evento
    // conta o seu próprio ciclo — um padrão euclidiano E(k, n, r) — e só age
    // nos ataques dele. Ciclos de comprimentos diferentes se reencontram no
    // mínimo múltiplo comum (polimetria). `ordinal` 0 é a métrica de casa;
    // outro valor sorteia novos ciclos (modulação métrica). Desligado, os
    // relógios são os de antes. Conceitos: os padrões euclidianos, agrupamentos
    // e peneiras da especificação de ritmos do Batruc (RITMOS_E_CONTAGENS.md,
    // só lida; nenhum código copiado); G. Toussaint, "The Euclidean algorithm
    // generates traditional musical rhythms", BRIDGES 2005; a polimetria por
    // ciclos coprimos do PolymeterEnsemble do Studio.
    // A atividade da parte da forma (4/11): a fração de ataques que passa,
    // de 0,35 a 1. Escolha determinística por estrela e por ataque (a mesma
    // seed soa igual). 1 = tudo passa, como antes.
    void setActivity(double activity) noexcept {
        activity_ = std::isfinite(activity) ? std::clamp(activity, 0.0, 1.0) : 1.0;
    }
    double activity() const noexcept { return activity_; }
    void setCounting(bool enabled, std::uint32_t ordinal) noexcept {
        if (enabled == counting_ && ordinal == countingOrdinal_) return;
        counting_ = enabled;
        countingOrdinal_ = ordinal;
        if (counting_) buildEuclid();
    }
    // Peneira (Xenakis): a união de duas classes de resíduos, (m1, 0) ∪
    // (m2, r2) — ataca no passo n se n mod m1 = 0 ou n mod m2 = r2. O ciclo é
    // o mínimo múltiplo comum dos módulos (3 e 5: 15 passos), irregular por
    // dentro. I. Xenakis, "Sieves", Perspectives of New Music 28(1), 1990.
    static constexpr bool sieveOnset(std::uint32_t step, std::uint32_t m1, std::uint32_t m2,
                                     std::uint32_t r2) noexcept {
        return m1 > 0U && m2 > 0U && (step % m1 == 0U || step % m2 == r2);
    }
    static constexpr std::array<std::array<std::uint8_t, 2>, 14> kSievePairs{{
        {{2, 3}}, {{2, 5}}, {{3, 4}}, {{3, 5}}, {{4, 5}}, {{3, 7}}, {{4, 7}},
        {{5, 7}}, {{5, 8}}, {{7, 8}}, {{7, 9}}, {{9, 11}}, {{11, 13}}, {{13, 16}}}};
    bool usesSieve(PerformanceModuleBody body) const noexcept {
        const auto star = starIndex(body);
        return counting_ && star < kStarCount && sieve_[star];
    }
    static constexpr bool euclidOnset(std::uint32_t step, std::uint32_t pulses,
                                      std::uint32_t length, std::uint32_t rotation) noexcept {
        return length > 0U && ((step + rotation) * pulses) % length < pulses;
    }
    std::uint32_t euclidLength(PerformanceModuleBody body) const noexcept {
        const auto star = starIndex(body);
        return star < kStarCount ? euclidN_[star] : 0U;
    }
    std::uint32_t euclidPulses(PerformanceModuleBody body) const noexcept {
        const auto star = starIndex(body);
        return star < kStarCount ? euclidK_[star] : 0U;
    }
    // Agrupamentos aditivos (29/10): o compasso de 16 pulsos dividido em
    // grupos desiguais (3+3+2+3+3+2, 5+5+6, 2+3+2+3+3+3…). O início de cada
    // grupo é um acento métrico; o primeiro, mais forte. A métrica de casa
    // sai da seed; cada contraste sorteia outro agrupamento. Conceito dos
    // agrupamentos da especificação de ritmos do Batruc (só lida) e do aksak
    // (C. Brăiloiu, "Le rythme aksak", Revue de Musicologie 33, 1951).
    static constexpr std::array<std::uint16_t, 8> kGroupings{{
        0x1111U,  // 4+4+4+4
        0x1U | (1U << 3U) | (1U << 6U) | (1U << 8U) | (1U << 11U) | (1U << 14U),  // 3+3+2+3+3+2
        0x249U | (1U << 12U),  // 3+3+3+3+4
        0x1U | (1U << 2U) | (1U << 5U) | (1U << 7U) | (1U << 10U) | (1U << 13U),  // 2+3+2+3+3+3
        0x1U | (1U << 5U) | (1U << 10U),  // 5+5+6
        0x1U | (1U << 3U) | (1U << 6U) | (1U << 10U) | (1U << 13U),  // 3+3+4+3+3
        0x1U | (1U << 2U) | (1U << 4U) | (1U << 7U) | (1U << 9U) | (1U << 11U) | (1U << 14U),  // 2+2+3+2+2+3+2
        0x1U | (1U << 3U) | (1U << 5U) | (1U << 8U) | (1U << 11U) | (1U << 13U),  // 3+2+3+3+2+3
    }};
    std::uint16_t groupingMask() const noexcept { return counting_ ? groupingMask_ : 0U; }
    // Sequências de durações (29/10): o terceiro controle de cada estrela que
    // conta — a duração da nota nas vozes em que ele é duração (portão da
    // flauta, sílaba do scat e do atropelo, frase do coro, galo, deja vu) — é
    // multiplicado, a cada ataque, pelo próximo termo de uma sequência de
    // proporções normalizada pela média: frases com durações longas e curtas
    // em proporção, não só ataques em lugares diferentes. Conceito das
    // sequências de durações da especificação de ritmos do Batruc (só lida).
    static constexpr std::array<std::array<std::uint8_t, 5>, 6> kDurationSequences{{
        {{1, 2, 3, 5, 8}}, {{3, 1, 2, 0, 0}}, {{2, 1, 1, 2, 3}}, {{1, 1, 2, 0, 0}},
        {{5, 3, 2, 1, 1}}, {{2, 3, 0, 0, 0}}}};
    // As vozes em que o terceiro controle é a duração da nota (conferido em
    // applyCue: portão, sílaba, frase, galo, gatilho).
    static constexpr bool shapesDuration(PerformanceModuleBody body) noexcept {
        return body == PerformanceModuleBody::FmFlute || body == PerformanceModuleBody::Scat
            || body == PerformanceModuleBody::Choir || body == PerformanceModuleBody::DejaVu
            || body == PerformanceModuleBody::Atropelo || body == PerformanceModuleBody::Galo;
    }
    double durationFactor(PerformanceModuleBody body) const noexcept {
        const auto star = starIndex(body);
        if (!counting_ || star >= kStarCount || !counts_[star] || !shapesDuration(body)) return 1.0;
        const auto& sequence = kDurationSequences[durationSequence_[star]];
        std::size_t length = 0U;
        double sum = 0.0;
        for (const auto term : sequence) {
            if (term == 0U) break;
            ++length;
            sum += term;
        }
        return static_cast<double>(sequence[durationStep_[star] % length]) * static_cast<double>(length) / sum;
    }
    // Coro de fases (29/10): o ataque de cada estrela que conta cai uma
    // fração do passo depois do pulso comum (a fase do seu oscilador no
    // diretor). 0 é o pulso; só vale com as contagens ligadas.
    void setPhaseDelay(PerformanceModuleBody body, double fraction) noexcept {
        const auto star = starIndex(body);
        if (star < kStarCount)
            phaseDelay_[star] = std::isfinite(fraction) ? std::clamp(fraction, 0.0, 0.9) : 0.0;
    }

    // Instante (fração do compasso) em que o passo k começa; 16 passos.
    double stepTime(std::size_t step) const noexcept { return stepTimes_[step < 17U ? step : 16U]; }

    // Kepler: fração do período gasta para ir do periélio + offset até o
    // ângulo verdadeiro dado, contínua e crescente.
    static double meanAnomalyFromTrue(double trueAnomaly, double eccentricity) noexcept {
        constexpr double kTau = 6.283185307179586;
        const double turns = std::floor((trueAnomaly + kTau * 0.5) / kTau);
        const double nu = trueAnomaly - turns * kTau;
        const double eccentric = 2.0 * std::atan2(std::sqrt(1.0 - eccentricity) * std::sin(nu * 0.5),
                                                  std::sqrt(1.0 + eccentricity) * std::cos(nu * 0.5));
        return eccentric - eccentricity * std::sin(eccentric) + turns * kTau;
    }
    static double keplerWarp(double fraction, double eccentricity, double perihelion) noexcept {
        constexpr double kTau = 6.283185307179586;
        const double start = meanAnomalyFromTrue(perihelion, eccentricity);
        return (meanAnomalyFromTrue(perihelion + kTau * fraction, eccentricity) - start) / kTau;
    }

    void setSeedEvent(const PerformanceSeedEvent& event) noexcept {
        if (event.ordinal != lastSeedEventOrdinal_ || event.variation != seedVariation_)
            seedEventPending_ = event.ordinal > 0U;
        lastSeedEventOrdinal_ = event.ordinal;
        seedVariation_ = event.variation;
        seedDensityBias_ = event.densityBias;
        seedTensionBias_ = event.tensionBias;
        seedMatterBias_ = event.matterBias;
        seedMemoryBias_ = event.memoryBias;
    }

    bool setControl(PerformanceModuleBody body, PerformanceModuleControl control,
                    double value) noexcept {
        const auto star = starIndex(body);
        const auto controlIndex = static_cast<std::size_t>(control);
        if (star >= kStarCount || controlIndex >= 3U || !std::isfinite(value)
            || value < 0.0 || value > 1.0) return false;
        controls_[star][controlIndex] = value;
#if defined(RASGO_PERFORMANCE_SKY_AVAILABLE)
        if (prepared_) configureStarControls();
#endif
        return true;
    }
    const StarControls& controls(PerformanceModuleBody body) const noexcept {
        const auto star = starIndex(body);
        return controls_[star < kStarCount ? star : 0U];
    }
    void resetControls() noexcept {
        controls_ = defaultControls();
#if defined(RASGO_PERFORMANCE_SKY_AVAILABLE)
        if (prepared_) configureStarControls();
#endif
    }

    // Avança o tempo comum do céu: uma vez por amostra, antes de renderStar.
    void advance(const PerformanceSkyContext& sky) noexcept {
        renderedMask_ = 0U;
        attackedMask_ = 0U;
#if defined(RASGO_PERFORMANCE_SKY_AVAILABLE)
        if (!prepared_) return;
        context_ = sky;
        context_.density = unit(sky.density + seedDensityBias_ * 0.82);
        context_.tension = unit(sky.tension + seedTensionBias_ * 0.70);
        context_.memory = unit(sky.memory + seedMemoryBias_ * 0.72);
        context_.matter = unit(sky.matter + seedMatterBias_ * 0.78);
        context_.risk = unit(sky.risk);
        context_.friction = unit(sky.friction);
        context_.time = unit(sky.time);
        context_.mutate = unit(sky.mutate);
        context_.tear = unit(sky.tear);
        context_.repair = unit(sky.repair);
        context_.intervention = unit(sky.intervention);
        const double tempo = std::clamp(sky.tempoBpm, 20.0, 300.0) / 60.0;

        seedCue_ = -1;
        if (seedEventPending_) {
            seedCue_ = static_cast<int>(seedVariation_ & 3U);
            seedEventPending_ = false;
            seedEventPulse_ = 1.0;
        }
        seedEventPulse_ *= 0.99935;
        mutateResponse_ += (context_.mutate - mutateResponse_) * 0.0035;
        tearResponse_ += (context_.tear - tearResponse_) * 0.0045;
        holdResponse_ += ((sky.holding ? 1.0 : 0.0) - holdResponse_) * 0.0035;
        mutateEdge_ = context_.mutate > lastMutate_ + 0.08;
        tearEdge_ = context_.tear > lastTear_ + 0.08;
        lastMutate_ = context_.mutate;
        lastTear_ = context_.tear;
        scorePhase_ += scoreRate_ / sampleRate_;
        if (scorePhase_ >= 1.0) scorePhase_ -= std::floor(scorePhase_);
        scoreArc_ = 0.5 + 0.5 * std::sin(scorePhase_ * kTwoPi);
        counterArc_ = 0.5 + 0.5 * std::sin((scorePhase_ * 1.61803398875 + scoreOffset_) * kTwoPi);
        autonomyEvent_ = std::sin(scorePhase_ * kTwoPi) * (unit(sky.autonomy) - 0.5) * 0.22;

        rungler_.setFeedbackDepth(unit(0.12 + context_.risk * 0.52
                                      + context_.friction * 0.30 - context_.memory * 0.08));
        runglerValue_ = rungler_.next();
        buchlaPhase_ += (0.45 + context_.time * 5.5 + context_.density * 2.0) / sampleRate_;
        if (!sky.holding && buchlaPhase_ >= 1.0) {
            buchlaPhase_ -= std::floor(buchlaPhase_);
            buchla_.clock();
        }
        random_ = buchla_.outputs();

        const double circuitRate = tempo * (0.72 + profile_.rhythmicDrive * 0.72
            + unit(sky.autonomy) * 1.6 + context_.time * 3.0);
        if (eccentricity_ > 0.0 || grooveDepth_ > 0.0) {
            // Relógio orbital: a fase do compasso anda uniforme; os passos
            // caem nos instantes da órbita e do groove.
            if (!sky.holding) barPhase_ += circuitRate / 16.0 / sampleRate_;
            circuitTick_ = false;
            if (barPhase_ >= stepTimes_[nextStep_ + 1U]) {
                circuitTick_ = true;
                ++nextStep_;
                if (nextStep_ >= 16U) {
                    nextStep_ = 0U;
                    barPhase_ -= std::floor(barPhase_);
                }
                barStep_ = static_cast<std::uint32_t>(nextStep_);
            }
        } else {
            circuitTick_ = advanceClock(circuitPhase_, circuitRate, sky.holding);
            if (circuitTick_) barStep_ = (barStep_ + 1U) & 15U;
        }
        if (circuitTick_) {
            divider_.clock();
            ++circuitStep_;
        }
        if (counting_) {
            const double stepSamples = sampleRate_ / std::max(circuitRate, 0.05);
            for (std::size_t star = 0U; star < kStarCount; ++star) {
                bool onset = circuitTick_
                    && (sieve_[star] ? sieveOnset(countStep_[star], sieveM1_[star], sieveM2_[star], sieveR2_[star])
                                     : euclidOnset(countStep_[star], euclidK_[star], euclidN_[star], euclidR_[star]));
                if (circuitTick_) ++countStep_[star];
                const double accent = onset && counts_[star] && (groupingMask_ & (1U << barStep_)) != 0U
                    ? (barStep_ == 0U ? 0.34 : 0.2) : 0.0;
                // Um ataque atrasado pela fase espera a sua vez; se o
                // próximo ataque chega antes, o que esperava sai já.
                bool due = false;
                if (delayLeft_[star] > 0U && (onset || --delayLeft_[star] == 0U)) {
                    delayLeft_[star] = 0U;
                    due = true;
                    accent_[star] = std::max(accent_[star], delayedAccent_[star]);
                }
                const auto delay = onset && counts_[star]
                    ? static_cast<std::uint32_t>(phaseDelay_[star] * stepSamples) : 0U;
                if (onset && delay > 0U) {
                    delayLeft_[star] = delay;
                    delayedAccent_[star] = accent;
                } else if (onset) {
                    due = true;
                    accent_[star] = std::max(accent_[star], accent);
                }
                ownTick_[star] = due;
                // Sequência de durações: cada ataque leva o próximo termo.
                if (due && counts_[star]) ++durationStep_[star];
            }
        }
        phraseTick_ = advanceClock(phrasePhase_, tempo * (0.28 + context_.time * 0.92 + 0.175
            + context_.mutate * 0.34 + context_.tear * 0.16), sky.holding);
        if (phraseTick_) ++phraseStep_;
        memoryTick_ = advanceClock(memoryPhase_, tempo * (0.18 + context_.time * 0.62
            + context_.density * 0.24 + 0.11), sky.holding);
        if (memoryTick_) ++memoryStep_;
        // As estrelas de controle acesas na amostra anterior valem nesta.
        controlActive_ = controlPending_;
        ++hocketClock_;
        controlPending_ = ControlState{};
        if (controlActive_.rupture) {
            if (memoryTick_) {
                if (ruptureBeatsLeft_ > 0U) --ruptureBeatsLeft_;
                else if (++ruptureBeat_ >= controlActive_.rupturePeriod) {
                    ruptureBeat_ = 0U;
                    if (hocketDraw() < controlActive_.ruptureChance) {
                        ruptureBeatsLeft_ = controlActive_.ruptureLength;
                        rupturePending_ = true;
                    }
                }
            }
        } else {
            ruptureBeatsLeft_ = 0U;
        }
        // A Ruptura corta em ~3 ms e volta em ~20 ms.
        const double ruptureTarget = ruptureBeatsLeft_ > 0U ? 0.0 : 1.0;
        ruptureGain_ += (ruptureTarget - ruptureGain_)
            * (ruptureTarget < ruptureGain_ ? 1.0 - std::exp(-1.0 / (0.003 * sampleRate_))
                                            : 1.0 - std::exp(-1.0 / (0.020 * sampleRate_)));
#else
        (void) sky;
#endif
    }

    // A voz de uma estrela acesa nesta amostra. Uma segunda chamada para a
    // mesma estrela na mesma amostra devolve o mesmo valor sem avançar o DSP.
    double renderStar(PerformanceModuleBody body, double frequency) noexcept {
        const auto star = starIndex(body);
        if (star >= kStarCount || !std::isfinite(frequency)) return 0.0;
        const auto bit = std::uint64_t{1} << star;
        if ((renderedMask_ & bit) != 0U) return lastOutput_[star];
        renderedMask_ |= bit;
        double value = 0.0;
#if defined(RASGO_PERFORMANCE_SKY_AVAILABLE)
        if (prepared_) {
            currentStar_ = star;
            const double base = std::max(20.0, frequency);
            if (cuePending_[star]) {
                cuePending_[star] = false;
                // Os chamados da trama também passam pelas estrelas de controle.
                if (controlGate())
                    applyCue(body, shapedControls(star), base, cuePitch_[star], cueStrength_[star]);
            }
            value = renderPrepared(body, shapedControls(star), std::max(20.0, base * bend_[star]))
                * (1.0 + accent_[star] * 0.9);
            accent_[star] *= 0.9997;
            bend_[star] += (1.0 - bend_[star]) * 1.4e-5;
        }
        if (seedEventPulse_ > 0.00001)
            value = std::tanh(value + seedEventPulse_ * kSeedPulseWeights[star % kSeedPulseWeights.size()]
                                  * (0.72 + seedVariation_ * 0.035));
#endif
#if defined(RASGO_PERFORMANCE_SKY_AVAILABLE)
        if (ruptureGain_ < 1.0) value *= ruptureGain_;
#endif
        lastOutput_[star] = std::isfinite(value) ? value : 0.0;
        return lastOutput_[star];
    }

    // Chamado de outra estrela (pela trama do céu): a estrela ataca do seu
    // próprio jeito na próxima amostra em que for renderizada. pitchHz 0 usa
    // a altura do próprio corpo. Estrelas de som contínuo respondem com um
    // acento e uma curva de afinação que volta devagar à sua altura.
    void cue(PerformanceModuleBody body, double pitchHz, double strength) noexcept {
        const auto star = starIndex(body);
        if (star >= kStarCount || !std::isfinite(pitchHz) || !std::isfinite(strength)) return;
        cuePending_[star] = true;
        cuePitch_[star] = std::clamp(pitchHz, 0.0, 4000.0);
        cueStrength_[star] = unit(strength);
    }
    // Se a estrela atacou na amostra corrente (por si ou por chamado) e em
    // que altura — o que as outras estrelas "ouvem".
    bool attacked(PerformanceModuleBody body) const noexcept {
        const auto star = starIndex(body);
        return star < kStarCount && (attackedMask_ & (std::uint64_t{1} << star)) != 0U;
    }
    double attackPitch(PerformanceModuleBody body) const noexcept {
        const auto star = starIndex(body);
        return star < kStarCount ? attackPitch_[star] : 0.0;
    }

private:
    static constexpr double kTwoPi = 6.283185307179586;
    static constexpr std::array<double, 5> kSeedPulseWeights{{0.015, 0.010, 0.018, 0.012, 0.008}};

    static double unit(double value) noexcept { return std::clamp(value, 0.0, 1.0); }

    bool advanceClock(double& phase, double rate, bool holding) const noexcept {
        if (!holding) phase += rate / sampleRate_;
        if (phase < 1.0) return false;
        phase -= std::floor(phase);
        return true;
    }

    // Os valores iniciais de cada estrela são os da vaga que ela ocupava na
    // sua família de origem (defaultModuleControls do adapter de coortes).
    static constexpr std::array<StarControls, kStarCount> defaultControls() noexcept {
        constexpr StarControls first{{0.52, 0.58, 0.54}};
        constexpr StarControls second{{0.38, 0.34, 0.56}};
        constexpr StarControls third{{0.62, 0.42, 0.48}};
        // As 14 da seed como sempre; as da biblioteca (2/10–2/11) seguem o
        // mesmo ciclo — antes ficavam com os três controles em zero.
        std::array<StarControls, kStarCount> controls{{first, second, third, first, second, first, second, third,
                                                      first, second, third, first, second, third}};
        for (std::size_t star = 14U; star < kStarCount; ++star)
            controls[star] = star % 3U == 0U ? first : star % 3U == 1U ? second : third;
        return controls;
    }

    double sampleRate_ = 48000.0;
    std::uint32_t seed_ = 181U;
    // A seed misturada (5/11): as fatias de bits em tempo de execução saem daqui.
    std::uint32_t seedMix_ = performanceSeedStream(181U, 0x5EEDU);
    std::array<StarControls, kStarCount> controls_{defaultControls()};
    std::array<double, kStarCount> lastOutput_{};
    std::uint64_t renderedMask_ = 0U;  // 64 bits desde 6/11 (mais de 32 estrelas)
    PerformanceSkyContext context_{};
    PerformanceCompositionProfile profile_{};
    std::uint8_t seedVariation_ = 0U;
    double seedDensityBias_ = 0.0;
    double seedTensionBias_ = 0.0;
    double seedMatterBias_ = 0.0;
    double seedMemoryBias_ = 0.0;
    std::uint32_t lastSeedEventOrdinal_ = std::numeric_limits<std::uint32_t>::max();
    bool seedEventPending_ = false;
    double seedEventPulse_ = 0.0;
    int seedCue_ = -1;
    double mutateResponse_ = 0.0;
    double tearResponse_ = 0.0;
    double holdResponse_ = 0.0;
    double lastMutate_ = 0.0;
    double lastTear_ = 0.0;
    bool mutateEdge_ = false;
    bool tearEdge_ = false;
    double scorePhase_ = 0.0;
    double scoreRate_ = 0.01;
    double scoreOffset_ = 0.0;
    double scoreArc_ = 0.5;
    double counterArc_ = 0.5;
    double autonomyEvent_ = 0.0;
    double runglerValue_ = 0.0;
    double buchlaPhase_ = 0.0;
    double circuitPhase_ = 1.0;
    double phrasePhase_ = 1.0;
    double memoryPhase_ = 1.0;
    bool circuitTick_ = false;
    bool phraseTick_ = false;
    bool memoryTick_ = false;
    std::uint32_t circuitStep_ = 0U;
    std::uint32_t phraseStep_ = 0U;
    std::uint32_t memoryStep_ = 0U;
    double gendynPitchPhase_ = 0.0;
    double gendynPitchHz_ = 165.0;
    double gendynTargetHz_ = 165.0;
    std::uint32_t fluteGateSamples_ = 0U;
    std::uint32_t galoBeats_ = 0U;
    double eccentricity_ = 0.0;
    double perihelion_ = 0.0;
    double grooveDepth_ = 0.0;
    double barPhase_ = 0.0;
    std::size_t nextStep_ = 0U;
    std::array<double, 17> stepTimes_{{0.0, 0.0625, 0.125, 0.1875, 0.25, 0.3125, 0.375, 0.4375, 0.5,
                                       0.5625, 0.625, 0.6875, 0.75, 0.8125, 0.875, 0.9375, 1.0}};
    std::array<double, 16> grooveOffsets_{};
    bool counting_ = false;
    std::uint32_t countingOrdinal_ = 0U;
    std::array<std::uint8_t, kStarCount> euclidK_{};
    std::array<std::uint8_t, kStarCount> euclidN_{};
    std::array<std::uint8_t, kStarCount> euclidR_{};
    std::array<std::uint32_t, kStarCount> countStep_{};
    std::array<bool, kStarCount> ownTick_{};
    std::array<bool, kStarCount> counts_{};
    std::array<double, kStarCount> phaseDelay_{};
    std::array<std::uint8_t, kStarCount> durationSequence_{};
    std::array<std::uint32_t, kStarCount> durationStep_{};
    // Os controles da estrela com a duração da sequência (só com contagens).
    StarControls shapedControls(std::size_t star) const noexcept {
        auto control = controls_[star];
        if (counting_ && counts_[star] && shapesDuration(kPerformanceAllStars[star]))
            control[2] = std::clamp(control[2] * durationFactor(kPerformanceAllStars[star]), 0.0, 1.0);
        return control;
    }
    std::array<bool, kStarCount> sieve_{};
    std::array<std::uint8_t, kStarCount> sieveM1_{};
    std::array<std::uint8_t, kStarCount> sieveM2_{};
    std::array<std::uint8_t, kStarCount> sieveR2_{};
    std::array<std::uint32_t, kStarCount> delayLeft_{};
    std::array<double, kStarCount> delayedAccent_{};
    std::uint16_t groupingMask_ = 0x1111U;
    std::uint32_t barStep_ = 0U;

    // O tique da estrela corrente: o do seu ciclo próprio quando há contagens,
    // o do relógio compartilhado da sua família quando não.
    // Com uma estrela de controle acesa (2/11), o tique passa pelo filtro dela.
    bool starTick(bool shared) noexcept {
        const bool tick = counting_ ? ownTick_[currentStar_] : shared;
        return tick && controlGate();
    }

    // As estrelas de controle: cada uma, acesa, filtra os ataques das outras
    // (as de som contínuo não atacam e seguem soando; a Ruptura cala todas).
    // Os parâmetros valem já na amostra em que a estrela de controle é
    // renderizada (para as estrelas renderizadas depois dela) e seguem na
    // amostra seguinte (para as anteriores): no pior caso, uma amostra de
    // atraso, só ao acender.
    struct ControlState {
        bool sieve = false, polymeter = false, hocket = false, rupture = false;
        std::uint32_t sieveM1 = 3U, sieveM2 = 5U, sieveR2 = 1U;
        std::uint32_t polyCycle = 5U, polySpread = 2U;
        double polyPulses = 0.5;
        std::uint32_t hocketGroup = 2U, hocketPatience = 2U;
        double hocketReach = 1.0;
        std::uint32_t rupturePeriod = 8U, ruptureLength = 2U;
        double ruptureChance = 0.6;
    };
    bool controlGate() noexcept {
        const auto star = currentStar_;
        if (performanceStarFunction(kPerformanceAllStars[star]) == PerformanceStarFunction::Control) return true;
        if (activity_ < 1.0) {
            std::uint32_t h = static_cast<std::uint32_t>(star) * 0x9e3779b9U ^ (activityStep_[star]++ * 0x85ebca6bU);
            h ^= h >> 16U; h *= 0x7feb352dU; h ^= h >> 15U; h *= 0x846ca68bU; h ^= h >> 16U;
            if (static_cast<double>(h) / 4294967296.0 >= activity_) return false;
        }
        const auto& g = controlActive_;
        if (g.rupture && ruptureBeatsLeft_ > 0U) return false;
        if (g.sieve && !sieveOnset(sieveStep_[star]++, g.sieveM1, g.sieveM2, g.sieveR2)) return false;
        if (g.polymeter) {
            // Polimetria: o mesmo número de ataques k (PULSOS sobre o CICLO)
            // em ciclos de comprimentos diferentes por estrela (CICLO + o
            // espalhamento pelo índice) — as estrelas defasam entre si.
            const auto n = g.polyCycle + (static_cast<std::uint32_t>(star) * g.polySpread) % 5U;
            const auto k = std::min(n, std::max<std::uint32_t>(1U,
                static_cast<std::uint32_t>(std::lround(g.polyPulses * g.polyCycle))));
            const auto step = polyStep_[star]++ % n;
            if ((step * k) % n >= k) return false;
        }
        if (g.hocket && hocketDraw() < g.hocketReach) {
            // Hoquetus: um ataque por instante — quem chega no mesmo instante
            // de outra cede — e uma estrela entre as últimas (GRUPO − 1) que
            // atacaram espera a vez. A PACIÊNCIA é quantas vezes ela cede
            // antes de atacar mesmo assim (sem isso, uma estrela sozinha
            // calaria).
            // No mesmo instante, sempre cede (uma estrela sozinha nunca colide
            // consigo mesma, então isso não cala ninguém).
            if (hocketLastClock_ == hocketClock_) return false;
            bool recent = false;
            for (std::uint32_t back = 0U; back + 1U < g.hocketGroup; ++back)
                recent = recent || hocketRecent_[back] == star;
            if (recent && hocketYield_[star] < g.hocketPatience) {
                ++hocketYield_[star];
                return false;
            }
            hocketYield_[star] = 0U;
            hocketLastClock_ = hocketClock_;
            for (std::size_t back = hocketRecent_.size() - 1U; back > 0U; --back)
                hocketRecent_[back] = hocketRecent_[back - 1U];
            hocketRecent_[0] = star;
        }
        return true;
    }
    double hocketDraw() noexcept {
        hocketState_ = hocketState_ * 1664525U + 1013904223U;
        return static_cast<double>(hocketState_ >> 8U) / 16777216.0;
    }
    ControlState controlActive_{}, controlPending_{};
    std::array<std::uint32_t, kStarCount> sieveStep_{}, polyStep_{}, hocketYield_{};
    std::array<std::uint32_t, kStarCount> activityStep_{};
    double activity_ = 1.0;
    std::array<std::size_t, 4> hocketRecent_{{kStarCount, kStarCount, kStarCount, kStarCount}};
    std::uint32_t hocketState_ = 0x5EEDU;
    std::uint64_t hocketClock_ = 0U, hocketLastClock_ = ~std::uint64_t{0};
    std::uint32_t ruptureBeat_ = 0U, ruptureBeatsLeft_ = 0U;
    double ruptureGain_ = 1.0;
    bool rupturePending_ = false;

    // Ciclos por estrela: pulso denso para quem marca o tempo, ciclos ímpares
    // e esparsos para linhas e vozes, longos e raros para memória.
    void buildEuclid() noexcept {
        struct Range { std::array<std::uint8_t, 4> lengths; double minDensity; double maxDensity; };
        for (std::size_t star = 0U; star < kStarCount; ++star) {
            Range range{{{8, 12, 16, 16}}, 0.0, 0.0};
            switch (kPerformanceAllStars[star]) {
            case PerformanceModuleBody::AyChip: range = {{{8, 12, 16, 12}}, 0.30, 0.55}; break;
            case PerformanceModuleBody::Morph: range = {{{7, 9, 11, 9}}, 0.20, 0.40}; break;
            case PerformanceModuleBody::Rossler: range = {{{11, 13, 16, 13}}, 0.10, 0.20}; break;
            case PerformanceModuleBody::EnvelopePulse: range = {{{8, 12, 16, 16}}, 0.30, 0.60}; break;
            case PerformanceModuleBody::Granular: range = {{{11, 13, 16, 13}}, 0.12, 0.25}; break;
            case PerformanceModuleBody::Turing: range = {{{8, 16, 8, 16}}, 0.35, 0.60}; break;
            case PerformanceModuleBody::Acid: range = {{{8, 12, 16, 12}}, 0.40, 0.70}; break;
            case PerformanceModuleBody::Percussion: range = {{{12, 16, 16, 12}}, 0.50, 0.75}; break;
            case PerformanceModuleBody::Gendyn: range = {{{5, 7, 9, 7}}, 0.25, 0.45}; break;
            case PerformanceModuleBody::FmFlute: range = {{{7, 9, 11, 13}}, 0.18, 0.34}; break;
            case PerformanceModuleBody::Scat: range = {{{5, 7, 9, 7}}, 0.30, 0.50}; break;
            case PerformanceModuleBody::Choir: range = {{{11, 13, 16, 13}}, 0.08, 0.16}; break;
            case PerformanceModuleBody::DejaVu: range = {{{13, 16, 13, 16}}, 0.08, 0.15}; break;
            case PerformanceModuleBody::Atropelo: range = {{{7, 9, 11, 9}}, 0.18, 0.32}; break;
            case PerformanceModuleBody::Galo: range = {{{11, 13, 11, 13}}, 0.10, 0.20}; break;
            case PerformanceModuleBody::Footnote: range = {{{13, 16, 11, 16}}, 0.06, 0.13}; break;
            case PerformanceModuleBody::Bluff: range = {{{8, 12, 8, 12}}, 0.18, 0.30}; break;
            case PerformanceModuleBody::Spectral: range = {{{8, 12, 8, 12}}, 0.25, 0.40}; break;
            case PerformanceModuleBody::Harmonaig: range = {{{4, 6, 4, 6}}, 0.25, 0.50}; break;
            case PerformanceModuleBody::Bloom: range = {{{8, 11, 8, 11}}, 0.40, 0.65}; break;
            case PerformanceModuleBody::Core: range = {{{7, 9, 7, 9}}, 0.40, 0.60}; break;
            case PerformanceModuleBody::BassFallback: range = {{{8, 12, 8, 12}}, 0.25, 0.45}; break;
            default: range = {{{8, 8, 8, 8}}, 0.25, 0.25}; break;
            }
            counts_[star] = range.minDensity != range.maxDensity;
            durationSequence_[star] = static_cast<std::uint8_t>(
                scoreWordStatic(seed_ ^ (countingOrdinal_ * 0x9e3779b9U), static_cast<std::uint32_t>(star), 29U)
                % kDurationSequences.size());
            const auto word = scoreWordStatic(seed_ ^ (countingOrdinal_ * 0x9e3779b9U),
                                              static_cast<std::uint32_t>(star), 21U);
            const auto length = range.lengths[word & 3U];
            const double density = range.minDensity
                + (range.maxDensity - range.minDensity) * static_cast<double>((word >> 4U) & 255U) / 255.0;
            euclidN_[star] = length;
            euclidK_[star] = static_cast<std::uint8_t>(std::clamp(
                static_cast<int>(std::lround(density * length)), 1, static_cast<int>(length) - 1));
            // A rotação é sorteada entre as posições de ataque: todo ciclo
            // começa num ataque (a estrela fala já no primeiro pulso).
            const auto onsetChoice = (word >> 12U) % euclidK_[star];
            std::uint32_t found = 0U;
            for (std::uint32_t step = 0U; step < length; ++step) {
                if (!euclidOnset(step, euclidK_[star], length, 0U)) continue;
                if (found++ == onsetChoice) { euclidR_[star] = static_cast<std::uint8_t>(step); break; }
            }
            // No contraste, metade das estrelas que contam troca o padrão
            // euclidiano por uma peneira de densidade próxima: a modulação
            // métrica fica mais estranha (ciclos de 15, 21, 35 passos).
            sieve_[star] = counts_[star] && countingOrdinal_ != 0U && ((word >> 20U) & 1U) != 0U;
            if (sieve_[star]) {
                std::size_t bestPair = 0U;
                double bestError = 2.0;
                for (std::size_t index = 0U; index < kSievePairs.size(); ++index) {
                    const auto m1 = kSievePairs[index][0];
                    const auto m2 = kSievePairs[index][1];
                    const double sieveDensity = 1.0 / m1 + 1.0 / m2 - 1.0 / std::lcm(static_cast<unsigned>(m1),
                                                                                       static_cast<unsigned>(m2));
                    const double error = std::fabs(sieveDensity - density);
                    if (error < bestError) { bestError = error; bestPair = index; }
                }
                // Qual classe abre o ciclo também varia com a seed.
                const bool swap = ((word >> 21U) & 1U) != 0U;
                sieveM1_[star] = kSievePairs[bestPair][swap ? 1 : 0];
                sieveM2_[star] = kSievePairs[bestPair][swap ? 0 : 1];
                sieveR2_[star] = static_cast<std::uint8_t>(1U + (word >> 22U) % (sieveM2_[star] - 1U));
            }
        }
        // O agrupamento da casa sai só da seed; um contraste sorteia outro,
        // sempre diferente do da casa.
        const auto home = scoreWordStatic(seed_, 0U, 23U) % kGroupings.size();
        auto pick = home;
        if (countingOrdinal_ != 0U) {
            pick = scoreWordStatic(seed_ ^ (countingOrdinal_ * 0x9e3779b9U), 1U, 23U) % (kGroupings.size() - 1U);
            if (pick >= home) ++pick;
        }
        groupingMask_ = kGroupings[pick];
    }
    static std::uint32_t scoreWordStatic(std::uint32_t seed, std::uint32_t step, std::uint32_t lane) noexcept {
        std::uint32_t word = seed ^ (0x9e3779b9U * (step + 0x7f4a7c15U)) ^ (0x85ebca6bU * (lane + 1U));
        word ^= word >> 16U;
        word *= 0x7feb352dU;
        word ^= word >> 15U;
        word *= 0x846ca68bU;
        return word ^ (word >> 16U);
    }

    void rebuildStepTimes() noexcept {
        stepTimes_[0] = 0.0;
        for (std::size_t step = 1U; step < 16U; ++step) {
            const auto beat = static_cast<double>(step / 4U);
            const double within = keplerWarp(static_cast<double>(step % 4U) / 4.0, eccentricity_, perihelion_);
            double time = (beat + within) / 4.0 + grooveOffsets_[step] * grooveDepth_;
            time = std::clamp(time, stepTimes_[step - 1U] + 0.02, 1.0 - 0.02 * static_cast<double>(16U - step));
            stepTimes_[step] = time;
        }
        stepTimes_[16] = 1.0;
    }
    std::size_t currentStar_ = 0U;
    std::uint64_t attackedMask_ = 0U;
    std::array<double, kStarCount> attackPitch_{};
    std::array<bool, kStarCount> cuePending_{};
    std::array<double, kStarCount> cuePitch_{};
    std::array<double, kStarCount> cueStrength_{};
    std::array<double, kStarCount> accent_{};
    std::array<double, kStarCount> bend_{{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                          1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};

    void markAttack(double pitchHz) noexcept {
        attackedMask_ |= std::uint64_t{1} << currentStar_;
        attackPitch_[currentStar_] = pitchHz;
    }

#if defined(RASGO_PERFORMANCE_SKY_AVAILABLE)
    bool prepared_ = false;
    std::array<float, rasgo::BuchlaRandomSource::kNumOutputs> random_{};
    rasgo::PolyrhythmDivider divider_{};
    rasgo::Rungler rungler_{};
    rasgo::BuchlaRandomSource buchla_{};
    rasgo::AcidBasslineVoice acid_{};
    rasgo::GendynOscillator gendyn_{};
    rasgo::KickVoice kick_{};
    rasgo::SnareVoice snare_{};
    rasgo::HiHatVoice hat_{};
    // O kit da DRUMS (6/11, auditoria dos módulos do Studio, passo 1): as
    // quatro vozes que já estavam em vendor/studio-dsp/src/dsp/DrumVoices.hpp
    // (cópia fixada do Studio, PROVENANCE.md) e não tocavam. A seed escolhe
    // quais entram no kit e como soam.
    rasgo::CowbellVoice cowbell_{};
    rasgo::ClapVoice clap_{};
    rasgo::CymbalVoice cymbal_{};
    rasgo::MaracasVoice maracas_{};
    std::uint8_t drumKit_ = 0U;  // bits: 1 cowbell, 2 clap, 4 cymbal, 8 maracas
    rasgo::FmFluteVoice flute_{};
    // As vozes do Studio de 6/11 (vendor/studio-dsp/src/dsp, PROVENANCE.md).
    rasgo::AY8910Chip ay_{};
    rasgo::MorphOscillator morph_{};
    double morphEnv_ = 0.0, morphDecay_ = 0.999, morphHz_ = 220.0, morphDrift_ = 0.0;
    rasgo::RosslerMap rossler_{};
    rasgo::EnvelopeLoopPulse pulseLoop_{};
    double pulseLoopRate_ = -1.0, pulseLoopDecay_ = -1.0;
    int ayStep_ = 0;
    // GRANULAR e TURING (6/11): reescritas do Studio, PerformanceStudioVoices.hpp.
    PerformanceGranularCloud granular_{};
    double granularPhase_ = 0.0, granularHz_ = 220.0, granularEnv_ = 0.0;
    PerformanceTuringRegister turing_{};
    rasgo::MorphOscillator turingVoice_{};
    double turingEnv_ = 0.0, turingDecay_ = 0.999;
    rasgo::ThereminVoice theremin_{};
    // A Tuba (30/10): banco modal grave excitado por sopro
    // (TubaBreathVoice.hpp, proveniência em PROVENIENCIA_TUBA_BREATH.md).
    TubaBreathVoice tuba_{};
    double tubaHz_ = 0.0;
    // O Footnote (30/10): duas parciais senoidais em razão inarmônica,
    // disparadas (vendor/studio-dsp/src/dsp/FootnotePluck.hpp, commit cfe1698).
    rasgo::FootnotePluck footnote_{};
    // Ganho das estrelas da biblioteca que soavam baixo (2/11), medido pela
    // intensidade de quando soam (janelas de 400 ms), não pela média — elas
    // são raras e de ataque. Só ganho, nunca timbre, e só até o pico de cada
    // uma caber em 1. Footnote, Bluff, Spectral, Bloom, Thermal, Texture.
    static constexpr std::array<double, 6> kLibraryTrim{{1.0, 1.25, 1.35, 1.75, 1.0, 1.25}};
    // O Bluff (30/10): o sinal e a confirmação são dois plucks harmônicos
    // (oitava limpa, ao contrário do Footnote); o jogo de blefe e confiança
    // fica em PerformanceBluffSignal.hpp.
    PerformanceBluffSignal bluff_{};
    rasgo::FootnotePluck bluffCall_{}, bluffAnswer_{};
    double bluffPitch_ = 0.0;
    // O Spectral (30/10): o período-base é medido entre as batidas da
    // estrela (PerformanceSpectralObject.hpp).
    PerformanceSpectralObject spectral_{};
    // Harmonaig e Bloom (2/11): reescritas em PerformanceHarmonyStars.hpp.
    PerformanceHarmonaigPad harmonaig_{};
    PerformanceBloomMelody bloom_{};
    // Thermal Static, Core, Texture, Bass Fallback (2/11):
    // PerformanceStudioConceptStars.hpp.
    PerformanceThermalStatic thermal_{};
    PerformanceCoreVoice core_{};
    PerformanceTextureBed texture_{};
    PerformanceBassFallback bass_{};
    int harmonaigRoot_ = 0;
    std::uint32_t harmonaigBeat_ = 0U;
    std::size_t spectralSinceBeat_ = 0U, spectralInterval_ = 0U;
    std::uint32_t spectralBeat_ = 0U;
    rasgo::ScatVoice scat_{};
    rasgo::ChoirEnsemble choir_{};
    rasgo::VosimOscillator vosim_{};
    rasgo::WalshOscillator walsh_{};
    rasgo::HardSyncVoice hardSync_{};
    rasgo::AtariPunkConsole atariPunk_{};
    rasgo::DejaVuVoice dejaVu_{};
    rasgo::AtropeloVoice atropelo_{};
    rasgo::GaloVoice galo_{};

    void prepareStars() noexcept {
        tuba_.prepare(sampleRate_, 0x7AB0U);
        tubaHz_ = 0.0;
        footnote_.setSampleRate(sampleRate_);
        bluffCall_.setSampleRate(sampleRate_);
        bluffCall_.setPartialRatio(2.0);
        bluffCall_.setDecayTime(0.35);
        bluffAnswer_.setSampleRate(sampleRate_);
        bluffAnswer_.setPartialRatio(3.0);
        bluffAnswer_.setDecayTime(0.8);
        spectral_.prepare(sampleRate_, 0x5EC7A1U);
        harmonaig_.prepare(sampleRate_);
        harmonaigRoot_ = 0;
        harmonaigBeat_ = 0U;
        bloom_.prepare(sampleRate_, 0xB100F5U);
        thermal_.prepare(sampleRate_, 0x7E57A71CU);
        core_.prepare(sampleRate_);
        texture_.prepare(sampleRate_, 0x7E87U);
        bass_.prepare(sampleRate_);
        spectralSinceBeat_ = spectralInterval_ = 0U;
        spectralBeat_ = 0U;
        acid_.setSampleRate(sampleRate_);
        gendyn_.setSampleRate(sampleRate_);
        kick_.setSampleRate(sampleRate_);
        snare_.setSampleRate(sampleRate_);
        hat_.setSampleRate(sampleRate_);
        cowbell_.setSampleRate(sampleRate_);
        ay_.setSampleRate(sampleRate_);
        morph_.setSampleRate(sampleRate_);
        rossler_.setSampleRate(sampleRate_);
        pulseLoop_.setSampleRate(sampleRate_);
        granular_.prepare(sampleRate_);
        turingVoice_.setSampleRate(sampleRate_);
        clap_.setSampleRate(sampleRate_);
        cymbal_.setSampleRate(sampleRate_);
        maracas_.setSampleRate(sampleRate_);
        rungler_.setSampleRate(sampleRate_);
        flute_.setSampleRate(sampleRate_);
        theremin_.setSampleRate(sampleRate_);
        scat_.setSampleRate(sampleRate_);
        choir_.setSampleRate(sampleRate_);
        vosim_.setSampleRate(sampleRate_);
        walsh_.setSampleRate(sampleRate_);
        walsh_.configure({1.0, 0.62, 0.34, 0.16});
        hardSync_.setSampleRate(sampleRate_);
        hardSync_.setSlaveWaveform(rasgo::Oscillator::Waveform::Saw);
        atariPunk_.setSampleRate(sampleRate_);
        dejaVu_.setSampleRate(sampleRate_);
        atropelo_.setSampleRate(sampleRate_);
        galo_.setSampleRate(sampleRate_);
    }

    void configureSeed(std::uint32_t seed) noexcept {
        profile_ = compositionProfileForSeed(seed);
        // Cada parâmetro de timbre da seed num fluxo próprio (5/11, auditoria
        // da seed): antes, fatias de bits do número — as seeds pequenas davam
        // o mesmo bumbo, o mesmo chimbal, o mesmo rungler.
        const auto v = [seed](std::uint32_t domain) { return performanceSeedStream(seed, domain); };
        const auto byte = static_cast<std::uint8_t>(v(102U) & 0xffU);
        divider_.reset((v(121U) & 15U));
        rungler_.reset(byte == 0U ? 0x5dU : byte);
        rungler_.setRates(2.1 + static_cast<double>(v(103U) & 7U) * 0.41,
                          3.4 + static_cast<double>(v(104U) & 7U) * 0.53);
        buchla_ = rasgo::BuchlaRandomSource(byte == 0U ? 0xb5U : byte);
        gendyn_.seed(seed ^ 0xa511e9b3U);
        gendyn_.configure(5 + static_cast<int>(v(119U) % 8U),
                          0.00035 + static_cast<double>(v(105U) & 3U) * 0.00008,
                          0.0024 + static_cast<double>(v(106U) & 7U) * 0.00024);
        gendyn_.setDistribution(static_cast<rasgo::GendynOscillator::Distribution>(v(107U) & 3U));
        snare_.seedNoise(seed ^ 0x7f4a7c15U);
        kick_.setTuning(42.0 + static_cast<double>(v(120U) % 17U),
                        72.0 + static_cast<double>(v(108U) % 67U));
        snare_.setTone(145.0 + static_cast<double>(v(109U) % 83U));
        // As vozes de 6/11: o timbre de partida pela seed.
        ay_.setToneEnabled(0, true); ay_.setToneEnabled(1, true); ay_.setToneEnabled(2, (v(160U) & 1U) != 0U);
        ay_.setVolume(0, 0.8f); ay_.setVolume(1, 0.55f); ay_.setVolume(2, 0.4f);
        ay_.setEnvelopeShape(static_cast<rasgo::AY8910Chip::EnvelopeShape>(v(161U) % 4U));
        morph_.setShape(static_cast<float>(v(162U) % 1000U) / 1000.0f);
        rossler_.setParams(0.2, 0.2, 4.5 + static_cast<double>(v(163U) % 250U) / 100.0, 0.1);
        rossler_.setInitial(0.5 + static_cast<double>(v(164U) % 100U) / 100.0, 1.0, 1.0);
        pulseLoopRate_ = -1.0;
        granular_.seed(v(165U));
        turing_.seed(v(166U));
        // O kit da seed: cada peça com a sua chance, afinação e cauda.
        drumKit_ = static_cast<std::uint8_t>((v(140U) % 100U < 35U ? 1U : 0U) | (v(141U) % 100U < 45U ? 2U : 0U)
            | (v(142U) % 100U < 30U ? 4U : 0U) | (v(143U) % 100U < 45U ? 8U : 0U));
        cowbell_.setFundamental(480.0 + static_cast<double>(v(144U) % 360U));
        cowbell_.setDecay(0.08 + static_cast<double>(v(145U) % 100U) * 0.0016);
        clap_.seedNoise(seed ^ 0xC1A9U);
        clap_.setDecay(0.10 + static_cast<double>(v(146U) % 100U) * 0.002);
        clap_.setFilter(900.0 + static_cast<double>(v(147U) % 1400U), 0.25);
        cymbal_.setFundamental(280.0 + static_cast<double>(v(148U) % 220U));
        cymbal_.setDecay(0.35 + static_cast<double>(v(149U) % 100U) * 0.009);
        cymbal_.setFilter(6000.0 + static_cast<double>(v(150U) % 4000U), 0.2);
        maracas_.seedNoise(seed ^ 0x3A2AU);
        maracas_.setDecay(0.025 + static_cast<double>(v(151U) % 100U) * 0.0005);
        maracas_.setFilter(5000.0 + static_cast<double>(v(152U) % 4000U), 0.3);
        hat_.setFundamental(325.0 + static_cast<double>(v(110U) % 190U));
        flute_.setGain(0.72F);
        theremin_.setGain(0.48F);
        scat_.seed(seed ^ 0x9e3779b9U);
        choir_.seed(seed ^ 0x85ebca6bU);
        vosim_.setPulseCount(3 + static_cast<int>(v(111U) & 3U));
        atropelo_.configureFormants(seed ^ 0x6d2b79f5U);
        static constexpr std::array<double, rasgo::ModalResonator::kNumModes> galoRatios{{
            1.0, 2.01, 3.06, 4.15, 5.35, 6.7}};
        static constexpr std::array<double, rasgo::ModalResonator::kNumModes> galoDecays{{
            1.2, 0.9, 0.7, 0.5, 0.35, 0.25}};
        static constexpr std::array<double, rasgo::ModalResonator::kNumModes> galoGains{{
            1.0, 0.72, 0.48, 0.32, 0.20, 0.12}};
        galo_.configure(seed ^ 0x9e3779b9U, galoRatios, galoDecays, galoGains);
        bluff_.configure(0.7, seed ^ 0xB10FF00DU);
        spectral_.setPartialSet(static_cast<PerformanceSpectralObject::PartialSet>(v(112U) % 3U));
        bloom_.reseed(seed ^ 0xB100F5U);
        harmonaigRoot_ = static_cast<int>(v(113U) % 7U);
        configureStarControls();

        circuitPhase_ = 1.0; // a seed fala já na próxima amostra
        phrasePhase_ = 1.0;
        memoryPhase_ = 1.0;
        buchlaPhase_ = 0.0;
        circuitStep_ = (v(122U) & 15U);
        phraseStep_ = (v(123U) & 15U);
        memoryStep_ = (v(124U) & 15U);
        mutateResponse_ = 0.0;
        tearResponse_ = 0.0;
        holdResponse_ = 0.0;
        lastMutate_ = 0.0;
        lastTear_ = 0.0;
        seedEventPending_ = false;
        seedEventPulse_ = 0.0;
        lastSeedEventOrdinal_ = std::numeric_limits<std::uint32_t>::max();
        scorePhase_ = static_cast<double>(v(114U) & 255U) / 256.0;
        scoreRate_ = (0.004 + profile_.eventAffinity * 0.008)
            + static_cast<double>(v(115U) & 15U) * 0.00042;
        scoreOffset_ = static_cast<double>(v(116U) & 255U) / 256.0;
        gendynPitchPhase_ = static_cast<double>(v(117U) & 255U) / 256.0;
        gendynTargetHz_ = std::max(35.0, 165.0 + static_cast<double>(v(118U) & 63U));
        gendynPitchHz_ = gendynTargetHz_;
        fluteGateSamples_ = 0U;
        galoBeats_ = 0U;
        // Groove da seed: os tempos (passos 0, 4, 8, 12) ficam no lugar; as
        // subdivisões se adiantam ou atrasam até 1% do compasso (~20 ms a 120).
        for (std::size_t step = 0U; step < grooveOffsets_.size(); ++step)
            grooveOffsets_[step] = step % 4U == 0U ? 0.0
                : (static_cast<double>(scoreWord(seed, static_cast<std::uint32_t>(step), 9U) % 201U) - 100.0) / 10000.0;
        barPhase_ = 0.0;
        nextStep_ = 0U;
        barStep_ = 0U;
        rebuildStepTimes();
        countStep_.fill(0U);
        delayLeft_.fill(0U);
        durationStep_.fill(0U);
        if (counting_) buildEuclid();
    }

    void configureStarControls() noexcept {
        const auto fluteBrightness = controls_[starIndex(PerformanceModuleBody::FmFlute)][1];
        flute_.configure(0.94 + static_cast<double>(seed_ & 7U) * 0.015,
                         0.45 + fluteBrightness * 3.6,
                         0.08 + fluteBrightness * 0.34, 0.018, 0.24);
        const auto thereminDrift = controls_[starIndex(PerformanceModuleBody::Theremin)][2];
        theremin_.configure(0.08 + static_cast<double>((seedMix_ >> 3U) & 7U) * 0.018,
                            4.1 + thereminDrift * 3.2,
                            static_cast<float>(0.03 + thereminDrift * 0.48),
                            seed_ ^ 0x41c64e6dU);
        const auto choirSpread = static_cast<int>(std::lround(
            controls_[starIndex(PerformanceModuleBody::Choir)][1] * 8.0));
        choir_.setRegisterOffsets({12 + choirSpread, 5 + choirSpread / 2,
                                   -5 - choirSpread / 2, -12 - choirSpread});
    }

    double renderPrepared(PerformanceModuleBody body, const StarControls& control,
                          double frequency) noexcept {
        const auto& c = context_;
        const double holdGain = c.holding ? 0.42 : 1.0;
        switch (body) {
        case PerformanceModuleBody::Acid: return renderAcid(control, frequency);
        case PerformanceModuleBody::Gendyn: return renderGendyn(control, frequency);
        case PerformanceModuleBody::Percussion: return renderDrums(control);
        case PerformanceModuleBody::FmFlute: {
            if (fluteGateSamples_ > 0U && --fluteGateSamples_ == 0U) flute_.noteOff();
            if (starTick(phraseTick_)) {
                const double pitch = phrasePitch(frequency);
                flute_.setFrequency(pitch);
                flute_.noteOn();
                markAttack(pitch);
                fluteGateSamples_ = static_cast<std::uint32_t>(sampleRate_
                    * (0.11 + control[2] * 0.68 + c.memory * 0.28));
            }
            return std::tanh(static_cast<double>(flute_.tick())
                * (0.72 + control[0] * 0.86 + c.tension * 0.34 + c.mutate * 0.34
                   + c.repair * 0.14)) * (1.0 + holdResponse_ * 0.30);
        }
        case PerformanceModuleBody::Tuba: {
            // Sopro grave: a altura fica no registro baixo; o sopro respira com
            // o arco da partitura e a densidade; o filtro só é recalculado
            // quando a altura muda de verdade (cos e exp por parcial).
            const double target = std::clamp(frequency * (0.25 + control[0] * 0.75), 28.0, 440.0);
            if (tubaHz_ <= 0.0 || std::fabs(target / tubaHz_ - 1.0) > 0.004) {
                tubaHz_ = target;
                tuba_.setFrequency(tubaHz_);
            }
            const double breath = std::clamp(0.18 + control[1] * 0.72 + scoreArc_ * 0.16 + c.density * 0.12
                                             - holdResponse_ * 0.3, 0.0, 1.0);
            // As ressonâncias da Tuba são muito agudas (ganho de pico na casa
            // dos milhares): o sopro entra a 0,004 da escala para a voz ficar
            // na região linear — medido: com o sopro cheio saturava (RMS 0,81,
            // onda quase quadrada); a 0,004, pico 0,66 e fator de crista 3,1.
            return tuba_.process(breath * 0.004, 0.2 + control[2] * 0.8) * holdGain;
        }
        case PerformanceModuleBody::Footnote:
            // A nota de rodapé: um sininho levemente "fora", raro. A razão das
            // parciais fica sempre longe de intervalos limpos (1,18–1,94,
            // pulando 1,5); o decaimento vai de 0,15 a 1,6 s.
            if (starTick(memoryTick_)) {
                const double pitch = phrasePitch(frequency * (1.0 + control[0] * 2.0));
                double ratio = 1.18 + control[1] * 0.76;
                if (std::fabs(ratio - 1.5) < 0.04) ratio += 0.08;
                footnote_.setPartialRatio(ratio);
                footnote_.setDecayTime(0.15 + control[2] * 1.45);
                footnote_.trigger(std::clamp(pitch, 80.0, 2400.0));
                markAttack(pitch);
            }
            return static_cast<double>(footnote_.tick()) * kLibraryTrim[0] * holdGain;
        case PerformanceModuleBody::Bluff:
            // VERDADE: quanto dos sinais é genuíno (0,2–1). JANELA: a
            // expectativa, de 1 a 4 batidas. O sinal soa sempre igual; a
            // confirmação (uma quinta acima) só vem do genuíno, com o ganho da
            // confiança — e se cala depois de uma traição, até a
            // reorganização.
            if (starTick(memoryTick_)) {
                bluff_.setTruthfulness(0.2 + control[0] * 0.8);
                const auto window = 1U + static_cast<std::uint32_t>(std::lround(control[1] * 3.0));
                const auto beat = bluff_.beat(0.42 + c.risk * 0.3, window);
                if (beat.signal) {
                    bluffPitch_ = std::clamp(phrasePitch(frequency * (0.8 + control[2] * 2.2)), 80.0, 1800.0);
                    bluffCall_.trigger(bluffPitch_);
                    markAttack(bluffPitch_);
                }
                if (beat.confirmation && bluffPitch_ > 0.0) {
                    bluffAnswer_.trigger(std::clamp(bluffPitch_ * 1.5, 80.0, 2700.0));
                    markAttack(bluffPitch_ * 1.5);
                }
            }
            return (static_cast<double>(bluffCall_.tick()) * 0.8
                    + static_cast<double>(bluffAnswer_.tick()) * (0.25 + bluff_.responseGain() * 0.75))
                * kLibraryTrim[1] * holdGain;
        case PerformanceModuleBody::Harmonaig: {
            // Um acorde de quatro vozes (raiz, 3ª, 5ª, 7ª no modo do céu) que
            // anda em quartas a cada 1, 2 ou 4 batidas (MOVIMENTO). INVERSÃO:
            // que tom vai ao grave; ABERTURA: uma ou duas oitavas em cima.
            const std::uint32_t span = control[2] > 0.66 ? 1U : control[2] < 0.33 ? 4U : 2U;
            if (starTick(memoryTick_) && harmonaigBeat_++ % span == 0U) {
                harmonaigRoot_ = (harmonaigRoot_ + 3) % 7;
                harmonaig_.voice(std::clamp(frequency * 0.5, 40.0, 330.0), harmonaigRoot_,
                                 static_cast<int>(std::lround(control[0] * 3.0)), control[1]);
                markAttack(frequency * 0.5);
            }
            return harmonaig_.tick() * holdGain;
        }
        case PerformanceModuleBody::Bloom: {
            // A melodia fractal: cada batida da frase toca o próximo passo do
            // tronco (5) ou do galho (25); ao fim da volta, CAMINHO é a chance
            // de trocar de nível. REGISTRO: uma oitava abaixo a uma acima.
            if (starTick(phraseTick_)) {
                bloom_.setGateSamples(static_cast<std::uint32_t>(sampleRate_ * (0.06 + control[2] * 0.5)));
                bloom_.step(std::clamp(frequency, 50.0, 880.0), control[0],
                            std::round(control[1] * 2.0) - 1.0);
                markAttack(frequency);
            }
            return bloom_.tick() * kLibraryTrim[3] * holdGain;
        }
        case PerformanceModuleBody::ThermalStatic:
            // A vida térmica: esquenta (CALOR), morre — devagar ou, com a
            // chance de MORTE, de repente — esfria e volta. ESTÁTICA: os
            // estalos, mais densos quanto mais quente.
            return thermal_.tick(std::clamp(frequency * 0.5, 30.0, 400.0), control[0], control[1], control[2])
                * kLibraryTrim[4] * holdGain;
        case PerformanceModuleBody::Core:
            if (starTick(phraseTick_)) {
                const double pitch = phrasePitch(frequency);
                core_.trigger(pitch);
                markAttack(pitch);
            }
            return core_.tick(control[0], control[1], 0.12 + control[2] * 1.5) * holdGain;
        case PerformanceModuleBody::Texture:
            return texture_.tick(std::clamp(frequency * 4.0, 120.0, 4000.0), control[0], control[1], control[2],
                                 c.density) * kLibraryTrim[5] * holdGain;
        case PerformanceModuleBody::BassFallback:
            if (starTick(memoryTick_)) {
                const double pitch = phrasePitch(frequency);
                bass_.trigger(pitch, control[0], 0.08 + control[2] * 0.9);
                markAttack(pitch);
            }
            return bass_.tick(control[1]) * holdGain;
        case PerformanceModuleBody::AyChip: {
            // O chip de 8 bits (AY-3-8910): três tons em arpejo (ARPEJO: da
            // oitava à tríade aberta), o ruído do registrador (RUÍDO) e o
            // envelope de hardware em laço (ENVELOPE: a sua velocidade).
            if (starTick(circuitTick_)) {
                static constexpr std::array<double, 4> steps{{1.0, 1.25, 1.5, 2.0}};
                const double base = phrasePitch(std::clamp(frequency, 60.0, 900.0));
                const auto span = 1 + static_cast<int>(std::lround(control[0] * 3.0));
                ayStep_ = (ayStep_ + 1) % std::max(1, span);
                ay_.setToneFrequency(0, base * steps[static_cast<std::size_t>(ayStep_)]);
                ay_.setToneFrequency(1, base * 0.5);
                ay_.setToneFrequency(2, base * 2.0);
                markAttack(base);
            }
            const bool noise = control[1] > 0.35;
            ay_.setNoiseEnabled(1, noise);
            ay_.setNoiseFrequency(800.0 + control[1] * 9000.0);
            ay_.useEnvelope(0, true);
            ay_.setEnvelopeFrequency(0.5 + control[2] * 11.5);
            return static_cast<double>(ay_.tick()) * 0.55 * holdGain;
        }
        case PerformanceModuleBody::Morph: {
            // Um tom que passa entre seno, triângulo, serra e quadrada
            // (FORMA), com a forma andando sozinha (DERIVA); cada frase
            // reacende o envelope, de 0,1 a 1,8 s (DURAÇÃO).
            if (starTick(phraseTick_)) {
                morphHz_ = phrasePitch(std::clamp(frequency, 40.0, 1400.0));
                morph_.setFrequency(morphHz_);
                morphEnv_ = 1.0;
                morphDecay_ = std::exp(-1.0 / (sampleRate_ * (0.1 + control[2] * 1.7)));
                markAttack(morphHz_);
            }
            morphDrift_ += (0.05 + control[1] * 0.6) / sampleRate_;
            if (morphDrift_ >= 1.0) morphDrift_ -= 1.0;
            const double drift = control[1] * 0.35 * std::sin(morphDrift_ * 6.283185307179586);
            morph_.setShape(static_cast<float>(std::clamp(control[0] + drift, 0.0, 1.0)));
            morphEnv_ *= morphDecay_;
            return static_cast<double>(morph_.tick()) * morphEnv_ * 0.6 * holdGain;
        }
        case PerformanceModuleBody::Rossler: {
            // O atrator de Rössler como fonte: TAXA é a velocidade do passeio
            // (o tom do ruído caótico), CAOS o parâmetro c (de órbita quase
            // periódica a caos pleno), BRILHO o quanto do sinal passa; cada
            // erupção do eixo z é um ataque.
            rossler_.setRate(std::clamp(frequency * (0.25 + control[0] * 2.0), 20.0, 4000.0));
            rossler_.setParams(0.2, 0.2, 3.5 + control[1] * 4.5, 0.1);
            const double out = static_cast<double>(rossler_.tick());
            if (rossler_.justErupted()) markAttack(frequency);
            return std::tanh(out * (0.6 + control[2] * 1.4)) * 0.7 * holdGain;
        }
        case PerformanceModuleBody::Granular: {
            // Uma nuvem de grãos lidos do passado recente de uma frase própria
            // (o GRANULAR do Studio): DENSIDADE de 2 a 40 grãos por segundo,
            // ESPALHAR o quanto atrás eles vão (até 1,6 s), ALTURA o desvio
            // de cada grão (até 7 semitons).
            if (starTick(phraseTick_)) {
                granularHz_ = phrasePitch(std::clamp(frequency, 60.0, 1200.0));
                granularEnv_ = 1.0;
                markAttack(granularHz_);
            }
            granularPhase_ += granularHz_ / sampleRate_;
            if (granularPhase_ >= 1.0) granularPhase_ -= std::floor(granularPhase_);
            granularEnv_ *= 0.99996;
            const double source = std::sin(6.283185307179586 * granularPhase_) * granularEnv_
                + 0.3 * std::sin(12.566370614359172 * granularPhase_) * granularEnv_;
            granular_.setDensity(2.0 + control[0] * 38.0);
            granular_.setSpread(0.05 + control[1] * 1.55);
            granular_.setPitchJitter(control[2] * 7.0);
            granular_.setGrainSeconds(0.03, 0.06 + (1.0 - control[0]) * 0.22);
            return std::tanh(static_cast<double>(granular_.process(static_cast<float>(source * 0.6))) * 1.4) * 0.8 * holdGain;
        }
        case PerformanceModuleBody::Turing: {
            // A melodia do registrador "Turing Machine" (o KS Randomizer do
            // Studio): MUDANÇA é a chance de um bit novo a cada passo (0: o
            // laço trava e repete; 0,6: quase nunca repete), FORMA o timbre
            // (seno a quadrada), DURAÇÃO a cauda de cada nota.
            if (starTick(circuitTick_)) {
                static constexpr std::array<int, 8> degrees{{0, 2, 4, 7, 9, 12, 14, 16}};
                turing_.setChange(control[0] * 0.6);
                const auto value = turing_.step();
                const double hz = phrasePitch(std::clamp(frequency, 60.0, 900.0))
                    * std::pow(2.0, degrees[value & 7U] / 12.0);
                turingVoice_.setFrequency(hz);
                turingEnv_ = (value & 0x80U) != 0U ? 1.0 : 0.55;
                turingDecay_ = std::exp(-1.0 / (sampleRate_ * (0.06 + control[2] * 0.9)));
                markAttack(hz);
            }
            turingVoice_.setShape(static_cast<float>(control[1]));
            turingEnv_ *= turingDecay_;
            return static_cast<double>(turingVoice_.tick()) * turingEnv_ * 0.55 * holdGain;
        }
        case PerformanceModuleBody::EnvelopePulse: {
            // Um pulso que se repete sozinho (o envelope em laço do Studio):
            // VELOCIDADE do laço, DECAIM. de cada golpe, ALTURA do tom.
            const double rate = 0.6 + control[0] * 7.4;
            const double decay = 0.03 + control[1] * 0.5;
            if (std::fabs(rate - pulseLoopRate_) > 0.05 || std::fabs(decay - pulseLoopDecay_) > 0.01) {
                pulseLoopRate_ = rate;
                pulseLoopDecay_ = decay;
                pulseLoop_.configure(std::max(0.002, 1.0 / rate - decay) * 0.08, std::min(decay, 0.95 / rate));
            }
            pulseLoop_.setFrequency(std::clamp(frequency * (0.5 + control[2] * 3.5), 40.0, 3000.0));
            const double out = static_cast<double>(pulseLoop_.tick());
            if (pulseLoop_.justPulsed()) markAttack(frequency);
            return out * 0.8 * holdGain;
        }
        case PerformanceModuleBody::Sieve:
            // Peneira (Xenakis, "Sieves", 1990): (A, 0) ∪ (B, R) sobre a
            // contagem de ataques de cada estrela.
            controlPending_.sieve = true;
            controlPending_.sieveM1 = 2U + static_cast<std::uint32_t>(std::lround(control[0] * 5.0));
            controlPending_.sieveM2 = 3U + static_cast<std::uint32_t>(std::lround(control[1] * 8.0));
            controlPending_.sieveR2 = static_cast<std::uint32_t>(
                std::lround(control[2] * static_cast<double>(controlPending_.sieveM2 - 1U)));
            controlActive_.sieve = true;
            controlActive_.sieveM1 = controlPending_.sieveM1;
            controlActive_.sieveM2 = controlPending_.sieveM2;
            controlActive_.sieveR2 = controlPending_.sieveR2;
            return 0.0;
        case PerformanceModuleBody::Polymeter:
            controlPending_.polymeter = true;
            controlPending_.polyCycle = 3U + static_cast<std::uint32_t>(std::lround(control[0] * 6.0));
            controlPending_.polyPulses = 0.2 + control[1] * 0.7;
            controlPending_.polySpread = static_cast<std::uint32_t>(std::lround(control[2] * 4.0));
            controlActive_.polymeter = true;
            controlActive_.polyCycle = controlPending_.polyCycle;
            controlActive_.polyPulses = controlPending_.polyPulses;
            controlActive_.polySpread = controlPending_.polySpread;
            return 0.0;
        case PerformanceModuleBody::Hocket:
            controlPending_.hocket = true;
            controlPending_.hocketGroup = 2U + static_cast<std::uint32_t>(std::lround(control[0] * 2.0));
            controlPending_.hocketPatience = 1U + static_cast<std::uint32_t>(std::lround(control[1] * 3.0));
            controlPending_.hocketReach = 0.3 + control[2] * 0.7;
            controlActive_.hocket = true;
            controlActive_.hocketGroup = controlPending_.hocketGroup;
            controlActive_.hocketPatience = controlPending_.hocketPatience;
            controlActive_.hocketReach = controlPending_.hocketReach;
            return 0.0;
        case PerformanceModuleBody::Rupture:
            controlPending_.rupture = true;
            controlPending_.rupturePeriod = 4U + static_cast<std::uint32_t>(std::lround(control[0] * 12.0));
            controlPending_.ruptureLength = 1U + static_cast<std::uint32_t>(std::lround(control[1] * 3.0));
            controlPending_.ruptureChance = 0.3 + control[2] * 0.7;
            controlActive_.rupture = true;
            controlActive_.rupturePeriod = controlPending_.rupturePeriod;
            controlActive_.ruptureLength = controlPending_.ruptureLength;
            controlActive_.ruptureChance = controlPending_.ruptureChance;
            if (rupturePending_) {
                rupturePending_ = false;
                markAttack(0.0);
            }
            return 0.0;
        case PerformanceModuleBody::Spectral: {
            // Um objeto, três leituras: o parcial p soa em f0·p (esticado pela
            // INARMONIA, até B = 0,004) e pulsa p vezes por período. PERÍODO:
            // 1, 2 ou 4 batidas da estrela. A cada oito períodos, a peneira
            // abre buracos novos no espectro.
            spectral_.setTuning(std::clamp(frequency * (0.5 + control[0] * 1.5), 40.0, 440.0),
                                control[1] * 0.004);
            ++spectralSinceBeat_;
            if (starTick(memoryTick_)) {
                spectralInterval_ = spectralSinceBeat_;
                spectralSinceBeat_ = 0U;
                const std::uint32_t span = control[2] > 0.66 ? 4U : control[2] < 0.33 ? 1U : 2U;
                if (spectralBeat_++ % span == 0U) {
                    if ((spectralBeat_ / span) % 8U == 7U) spectral_.regenerateHoles();
                    spectral_.beginPeriod(spectralInterval_ * span);
                    markAttack(spectral_.partialFrequency(0U));
                }
            }
            // O ganho da biblioteca entra antes da saturação (5/11): com os
            // parciais ímpares (alinhados como uma onda quadrada) a saída
            // passava de 1; assim o teto é o do tanh, para qualquer conjunto.
            return std::tanh(spectral_.tick() * 0.55 * kLibraryTrim[2]) * holdGain;
        }
        case PerformanceModuleBody::Theremin:
            theremin_.setGain(static_cast<float>(0.18 + control[1] * 1.12));
            theremin_.setTargetFrequency(std::clamp(frequency
                * (0.66 + control[0] * 1.7 + scoreArc_ * 0.12
                   + c.intervention * 0.24 + c.mutate * 0.16), 38.0, 1760.0));
            return std::tanh(static_cast<double>(theremin_.tick())
                * (0.44 + control[1] * 1.10 + c.friction * 0.38
                   + c.intervention * 0.26 + c.tear * 0.18)) * (1.0 - holdResponse_ * 0.55);
        case PerformanceModuleBody::Scat:
            if (starTick(phraseTick_)) {
                const auto vowel = static_cast<int>(std::lround(control[1] * 4.0));
                markAttack(phrasePitch(frequency));
                scat_.beginSyllable(phrasePitch(frequency), 0.12 + control[2] * 0.58
                                        + c.memory * 0.24 + c.mutate * 0.16,
                                    static_cast<int>((phraseStep_ + static_cast<std::uint32_t>(vowel)) % 5U));
            }
            return std::tanh(static_cast<double>(scat_.tick())
                * (0.88 + control[0] * 1.38 + c.tension * 0.42 + c.mutate * 0.48))
                * (1.0 + holdResponse_ * 0.30);
        case PerformanceModuleBody::Choir:
            if (starTick(phraseTick_)) {
                static constexpr std::array<int, 4> chord{{0, 3, 7, 10}};
                const auto vowel = static_cast<int>(std::lround(control[1] * 4.0));
                markAttack(440.0 * std::pow(2.0, (42.0 + static_cast<double>(seed_ % 12U)
                    + std::round(control[0] * 18.0) - 69.0) / 12.0));
                choir_.triggerPhrase(42 + static_cast<int>(seed_ % 12U)
                                         + static_cast<int>(std::lround(control[0] * 18.0)), chord,
                                     static_cast<int>((phraseStep_ + 2U
                                         + static_cast<std::uint32_t>(vowel)
                                         + static_cast<std::uint32_t>(std::lround(control[2] * 3.0))) % 5U),
                                     0.20 + control[2] * 0.78 + c.memory * 0.22 + c.repair * 0.18);
            }
            return std::tanh(static_cast<double>(choir_.tick())
                * (0.22 + control[0] * 1.18 + control[2] * 0.54 + c.friction * 0.34
                   + c.memory * 0.30 + c.repair * 0.32)) * (1.0 - holdResponse_ * 0.55);
        case PerformanceModuleBody::Vosim: {
            vosim_.setFrequency(std::clamp(frequency * (0.7 + control[0] * 1.8), 28.0, 440.0));
            vosim_.setFormant(240.0 + control[1] * 1800.0 + c.matter * 520.0
                              + c.memory * 680.0 + c.tear * 420.0);
            vosim_.setDecay(0.24 + control[2] * 0.64 + c.memory * 0.16);
            const double formant = static_cast<double>(vosim_.tick()) * 2.0 - 0.62;
            return std::tanh(formant * (0.22 + control[2] * 0.42 + c.density * 0.24
                                        + counterArc_ * 0.12 + c.tear * 0.38))
                * (1.0 - holdResponse_ * 0.42 + c.risk * 0.12);
        }
        case PerformanceModuleBody::Walsh:
            walsh_.setFrequency(std::clamp(frequency * (0.45 + control[0] * 1.15
                + control[2] * 0.72 + c.matter * 0.35 + c.mutate * 0.45), 28.0, 2400.0));
            return std::tanh(static_cast<double>(walsh_.tick())
                * (0.28 + control[1] * 0.65 + control[2] * 0.24 + c.mutate * 0.36))
                * (c.holding ? 0.38 : 1.0);
        case PerformanceModuleBody::HardSync:
            hardSync_.setMasterFrequency(std::clamp(frequency, 28.0, 1800.0));
            hardSync_.setSlaveFrequency(std::clamp(frequency * (1.15 + control[0] * 7.8
                + control[2] * 5.6 + c.tension * 2.2 + c.tear * 3.2), 35.0, 8000.0));
            return std::tanh(static_cast<double>(hardSync_.tick())
                * (0.20 + control[1] * 0.62 + control[2] * 0.18 + c.tension * 0.30
                   + c.tear * 0.42)) * (c.holding ? 0.38 : 1.0);
        case PerformanceModuleBody::AtariPunk:
            atariPunk_.setClockFrequency(std::clamp(frequency * (0.5 + control[0] * 3.0
                + std::clamp(c.tempoBpm, 20.0, 300.0) / 60.0 * 0.22 + c.time * 1.4), 18.0, 3400.0));
            atariPunk_.setPulseWidth(0.0006 + control[1] * 0.011 + c.risk * 0.004
                                     + c.friction * 0.003);
            return std::tanh(static_cast<double>(atariPunk_.tick())
                * (0.14 + control[2] * 0.68 + c.risk * 0.34 + c.density * 0.22))
                * (c.holding ? 0.38 : 1.0);
        case PerformanceModuleBody::DejaVu:
            dejaVu_.trackMemory(std::clamp(frequency, 40.0, 1600.0),
                                std::clamp(0.25 + c.matter * 0.55 + control[1] * 0.30, 0.0, 1.0),
                                std::clamp(c.density + c.memory * 0.25, 0.0, 1.0));
            if (starTick(memoryTick_)) {
                dejaVu_.trigger(0.42 + control[2] * 0.86 + c.memory * 0.42);
                markAttack(std::clamp(frequency, 40.0, 1600.0));
            }
            return std::tanh(static_cast<double>(dejaVu_.tick())
                * (0.92 + control[0] * 0.68 + c.memory * 0.40 + c.friction * 0.12) * holdGain);
        case PerformanceModuleBody::Atropelo:
            if (starTick(memoryTick_)) {
                const double pitch = std::clamp(frequency * (0.72 + control[0] * 1.8
                    + c.tension * 0.32 + c.mutate * 0.24), 70.0, 1100.0);
                atropelo_.beginSyllable(pitch, 0.10 + control[2] * 0.44 + c.tear * 0.16);
                markAttack(pitch);
            }
            return std::tanh(static_cast<double>(atropelo_.tick())
                * (0.86 + control[1] * 0.82 + c.tension * 0.32 + c.tear * 0.28) * holdGain);
        case PerformanceModuleBody::Galo:
            // CHAMADA decide quantas batidas da memória chamam o Galo: todas,
            // uma em duas (o valor da família Memória) ou uma em quatro. A
            // contagem começa na entrada da seed, então a primeira batida chama.
            if (starTick(memoryTick_) && (galoBeats_++ & (control[0] > 0.66 ? 0U : control[0] < 0.33 ? 3U : 1U)) == 0U) {
                const double pitch = std::clamp(frequency * (0.72 + control[1] * 1.4), 80.0, 1200.0);
                galo_.trigger(pitch, 0.72 + control[2] * 0.72 + c.risk * 0.24);
                markAttack(pitch);
            }
            return std::tanh(static_cast<double>(galo_.tick())
                * (0.72 + control[2] * 0.70 + c.matter * 0.22 + c.mutate * 0.18) * holdGain);
        default:
            return 0.0;
        }
    }

    // Cada estrela atende a um chamado do seu jeito: as que têm ataque
    // (Acid, Drums, FM Flute, Scat, Choir, Deja Vu, Atropelo, Galo, Gendyn)
    // disparam o seu gesto na altura pedida; as contínuas (Theremin, VOSIM,
    // Walsh, Hard Sync, Atari Punk) curvam a afinação e acentuam.
    void applyCue(PerformanceModuleBody body, const StarControls& control, double base,
                  double requested, double strength) noexcept {
        const double pitch = requested > 0.0 ? requested : base;
        switch (body) {
        case PerformanceModuleBody::Acid:
            acid_.trigger(std::clamp(pitch, 28.0, 880.0), strength > 0.6, false);
            break;
        case PerformanceModuleBody::Gendyn:
            gendynTargetHz_ = std::clamp(pitch, 35.0, 2200.0);
            accent_[currentStar_] = strength * 0.6;
            break;
        case PerformanceModuleBody::Percussion:
            if (pitch < 130.0) kick_.trigger(); else snare_.trigger();
            if (strength > 0.7) hat_.trigger(false);
            break;
        case PerformanceModuleBody::FmFlute:
            flute_.setFrequency(std::clamp(pitch, 38.0, 1320.0));
            flute_.noteOn();
            fluteGateSamples_ = static_cast<std::uint32_t>(sampleRate_ * (0.11 + control[2] * 0.68));
            break;
        case PerformanceModuleBody::Scat:
            scat_.beginSyllable(std::clamp(pitch, 38.0, 1320.0), 0.12 + control[2] * 0.58,
                                static_cast<int>(phraseStep_ % 5U));
            break;
        case PerformanceModuleBody::Choir: {
            static constexpr std::array<int, 4> chord{{0, 3, 7, 10}};
            const auto midi = static_cast<int>(std::lround(std::clamp(
                69.0 + 12.0 * std::log2(std::max(pitch, 20.0) / 440.0), 36.0, 84.0)));
            choir_.triggerPhrase(midi, chord, static_cast<int>(phraseStep_ % 5U),
                                 0.20 + control[2] * 0.78);
            break;
        }
        case PerformanceModuleBody::DejaVu:
            dejaVu_.trackMemory(std::clamp(pitch, 40.0, 1600.0), 0.6, 0.6);
            dejaVu_.trigger(0.42 + control[2] * 0.86);
            break;
        case PerformanceModuleBody::Atropelo:
            atropelo_.beginSyllable(std::clamp(pitch, 70.0, 1100.0), 0.10 + control[2] * 0.44);
            break;
        case PerformanceModuleBody::Galo:
            galo_.trigger(std::clamp(pitch, 80.0, 1200.0), 0.72 + control[2] * 0.72);
            break;
        case PerformanceModuleBody::Footnote:
            footnote_.trigger(std::clamp(pitch, 80.0, 2400.0));
            break;
        case PerformanceModuleBody::Bluff:
            bluffPitch_ = std::clamp(pitch, 80.0, 1800.0);
            bluffCall_.trigger(bluffPitch_);
            break;
        case PerformanceModuleBody::Harmonaig:
            harmonaigRoot_ = (harmonaigRoot_ + 3) % 7;
            harmonaig_.voice(std::clamp(pitch * 0.5, 40.0, 330.0), harmonaigRoot_,
                             static_cast<int>(std::lround(control[0] * 3.0)), control[1]);
            break;
        case PerformanceModuleBody::Core:
            core_.trigger(pitch);
            break;
        case PerformanceModuleBody::BassFallback:
            bass_.trigger(pitch, control[0], 0.08 + control[2] * 0.9);
            break;
        case PerformanceModuleBody::AyChip:
            ay_.setToneFrequency(0, std::clamp(pitch, 60.0, 1800.0));
            break;
        case PerformanceModuleBody::Morph:
            morphHz_ = std::clamp(pitch, 40.0, 1400.0);
            morph_.setFrequency(morphHz_);
            morphEnv_ = std::max(morphEnv_, 0.4 + strength * 0.6);
            break;
        case PerformanceModuleBody::Bloom:
            bloom_.step(std::clamp(pitch, 50.0, 880.0), control[0], std::round(control[1] * 2.0) - 1.0);
            break;
        case PerformanceModuleBody::Spectral:
            if (spectralInterval_ > 0U) spectral_.beginPeriod(spectralInterval_ * 2U);
            break;
        default:
            bend_[currentStar_] = std::clamp(pitch / base, 0.25, 4.0);
            accent_[currentStar_] = strength;
            break;
        }
        markAttack(pitch);
    }

    double phrasePitch(double frequency) const noexcept {
        static constexpr std::array<int, 7> intervals{{0, 2, 3, 5, 7, 10, 12}};
        const auto degree = static_cast<std::size_t>((phraseStep_ + seed_) % 7U);
        return std::clamp(frequency * std::pow(2.0, (intervals[degree]
            + (context_.intervention - 0.5) * 2.4 + context_.mutate * 1.8
            - context_.repair * 0.8) / 12.0), 38.0, 1320.0);
    }

    double renderAcid(const StarControls& control, double frequency) noexcept {
        const auto& c = context_;
        if (seedCue_ == 0) acid_.trigger(std::clamp(frequency * 0.5, 28.0, 440.0), false, false);
        if (seedCue_ == 3) acid_.trigger(std::clamp(frequency * 1.5, 28.0, 880.0), true, true);
        if (mutateEdge_)
            acid_.trigger(std::clamp(frequency * (random_[1] > 0.0F ? 1.5 : 0.75), 28.0, 880.0),
                          true, false);
        if (tearEdge_) acid_.trigger(std::clamp(frequency * 0.5, 28.0, 440.0), true, false);
        acid_.setBaseCutoff(55.0 + control[0] * 2100.0 + frequency * (0.8 + c.matter * 2.8)
                            + (random_[0] + 1.0) * 190.0
                            + scoreArc_ * (180.0 + c.density * 520.0)
                            + profile_.timbralBrightness * 460.0);
        acid_.setEnvDepth(280.0 + control[2] * 3200.0 + c.tension * 2400.0
                          + c.matter * 1800.0 + c.tear * 1200.0);
        acid_.setResonance(unit(0.38 + control[1] * 0.42 + c.tension * 0.16 + c.risk * 0.08));
        if (starTick(circuitTick_)) {
            // A frase de 16 passos da família Circuito: o vocabulário dá a
            // identidade; a palavra da partitura dá contorno, aproximação e
            // registro.
            static constexpr std::array<std::array<int, 16>, 6> phrases{{
                {{0, 0, 3, 5, 7, 5, 3, 2, 0, 3, 5, 7, 10, 7, 5, 3}},
                {{0, 2, 4, 7, 9, 7, 4, 2, 0, -2, 0, 4, 7, 9, 11, 7}},
                {{0, 0, 1, 3, 4, 7, 6, 4, 3, 1, 0, -1, 0, 3, 6, 4}},
                {{0, 5, 3, 7, 10, 7, 12, 10, 5, 3, 0, 3, 7, 12, 10, 5}},
                {{0, 2, 3, 5, 8, 7, 5, 3, 0, 2, 5, 8, 10, 8, 5, 2}},
                {{0, 4, 8, 6, 3, 7, 10, 12, 8, 6, 3, 0, -2, 3, 7, 5}}}};
            const auto phraseIndex = static_cast<std::size_t>((seedMix_ ^ (seedMix_ >> 9U)
                ^ (seedVariation_ * 17U)) % phrases.size());
            const auto phraseStep = static_cast<std::size_t>((circuitStep_ + seedVariation_) & 15U);
            const auto melodicStep = (seed_ & 1U) != 0U ? 15U - phraseStep : phraseStep;
            const auto word = scoreWord(seed_, circuitStep_ + seedVariation_ * 19U, 0U);
            const int contour = static_cast<int>((word >> 4U) % 9U) - 4;
            const int approach = (word & 0x1fU) < 5U ? ((word & 0x20U) != 0U ? -1 : 1) : 0;
            const double octaveLift = ((word >> 12U) & 7U) == 0U
                ? (random_[1] > 0.0F ? 12.0 : -12.0) : 0.0;
            const double note = frequency * std::pow(2.0,
                (static_cast<double>(phrases[phraseIndex][melodicStep] + contour + approach)
                    + octaveLift) / 12.0);
            const bool accent = divider_.q(2) != divider_.q(0)
                || random_[2] > static_cast<float>(0.50 - c.risk * 0.35 - control[2] * 0.30);
            const bool slide = divider_.q(1)
                && random_[0] > static_cast<float>(0.20 + c.tension * 0.20 - control[2] * 0.55);
            acid_.trigger(std::clamp(note, 28.0, 880.0), accent, slide);
            markAttack(std::clamp(note, 28.0, 880.0));
        }
        const double acid = static_cast<double>(acid_.tick());
        return std::tanh(acid * (0.62 + control[2] * 0.98 + c.matter * 0.42) + autonomyEvent_)
            * (1.0 + holdResponse_ * 0.24);
    }

    double renderGendyn(const StarControls& control, double frequency) noexcept {
        const auto& c = context_;
        gendyn_.setStepScale(static_cast<float>(unit(0.015 + control[0] * 0.46
                                                     + c.friction * 0.22 + c.mutate * 0.24)),
                             static_cast<float>(unit(0.012 + control[1] * 0.38
                                                     + c.time * 0.18 + c.risk * 0.16)));
        if (starTick(circuitTick_)) {
            static constexpr std::array<std::array<int, 8>, 6> phrases{{
                {{0, 5, 3, 7, 10, 12, 7, 2}}, {{0, 3, 7, 5, 10, 8, 12, 5}},
                {{0, 7, 2, 10, 3, 12, 5, 8}}, {{0, 2, 5, 9, 7, 3, 10, 12}},
                {{0, 10, 7, 3, 5, 12, 8, 2}}, {{0, 5, 8, 12, 7, 10, 3, 6}}}};
            const auto word = scoreWord(seed_, circuitStep_ + seedVariation_ * 23U, 1U);
            const auto phrase = static_cast<std::size_t>((seed_ ^ (word >> 11U)) % phrases.size());
            const auto degree = static_cast<std::size_t>((circuitStep_ + (seedMix_ >> 4U)
                + seedVariation_ * 3U + (random_[0] > 0.0F ? 2U : 0U)) & 7U);
            const int contour = static_cast<int>((word >> 3U) % 7U) - 3;
            gendynTargetHz_ = std::clamp(frequency * std::pow(2.0,
                static_cast<double>(phrases[phrase][degree] + contour) / 12.0), 35.0, 2200.0);
            markAttack(gendynTargetHz_);
        }
        const double gendyn = static_cast<double>(gendyn_.tick());
        gendynPitchHz_ += (gendynTargetHz_ - gendynPitchHz_) * 0.0018;
        gendynPitchPhase_ += gendynPitchHz_ / sampleRate_;
        if (gendynPitchPhase_ >= 1.0) gendynPitchPhase_ -= std::floor(gendynPitchPhase_);
        const double melody = std::sin(gendynPitchPhase_ * kTwoPi)
            * (0.16 + control[0] * 0.26 + scoreArc_ * 0.10);
        return std::tanh(gendyn * (0.24 + control[2] * 0.42 + c.density * 0.30 + scoreArc_ * 0.20)
                         + melody + runglerValue_ * (0.04 + c.friction * 0.10)
                         + gendyn * tearResponse_ * 0.28)
            * (1.0 - holdResponse_ * 0.66);
    }

    double renderDrums(const StarControls& control) noexcept {
        const auto& c = context_;
        if (seedCue_ == 1) kick_.trigger();
        if (seedCue_ == 2) { snare_.trigger(); hat_.trigger(true); }
        if (mutateEdge_) hat_.trigger(false);
        if (tearEdge_) { kick_.trigger(); snare_.trigger(); hat_.trigger(true); }
        if (starTick(circuitTick_)) {
            // As células rítmicas da família Circuito, com a palavra da seed.
            static constexpr std::array<std::uint16_t, 6> kickCells{{
                0b1000100010001010U, 0b1001001010001000U, 0b1010000010100100U,
                0b1000101000101000U, 0b1000001010100010U, 0b1010100001001000U}};
            static constexpr std::array<std::uint16_t, 6> snareCells{{
                0b0000100000001000U, 0b0000010000000100U, 0b0000100001001000U,
                0b0001000000000100U, 0b0000010010000000U, 0b0000100000101000U}};
            static constexpr std::array<std::uint16_t, 6> hatCells{{
                0b1010101010101010U, 0b1101010110101010U, 0b1011011010110110U,
                0b1110101011101010U, 0b1010111010101110U, 0b1100110011001100U}};
            const auto cell = static_cast<std::uint32_t>((circuitStep_ + seedVariation_ * 3U
                + ((seedMix_ >> 14U) & 3U)) & 15U);
            const auto pattern = static_cast<std::size_t>((seedMix_ ^ (seedMix_ >> 6U)
                ^ (seedVariation_ * 11U)) % kickCells.size());
            const auto at = [cell](std::uint16_t bits) noexcept {
                return (bits & static_cast<std::uint16_t>(1U << cell)) != 0U;
            };
            const auto kickWord = scoreWord(seed_, cell + seedVariation_ * 31U, 2U);
            const auto snareWord = scoreWord(seed_, cell + seedVariation_ * 31U, 3U);
            const auto hatWord = scoreWord(seed_, cell + seedVariation_ * 31U, 4U);
            const bool kickHit = (cell & 3U) == 0U ? (kickWord & 0xffU) < 232U
                : (kickWord & 0xffU) < 42U + static_cast<std::uint32_t>(c.density * 44.0);
            const bool snareHit = (cell & 3U) == 2U ? (snareWord & 0xffU) < 214U
                : (snareWord & 0xffU) < 18U + static_cast<std::uint32_t>(c.risk * 40.0);
            const bool hatHit = (hatWord & 0xffU) < 84U + static_cast<std::uint32_t>(c.density * 104.0);
            if (at(kickCells[pattern]) || kickHit || divider_.risingEdge(0, 1)
                || (c.density + control[0] * 0.55 > 0.92 && (circuitStep_ & 3U) == 0U)) {
                kick_.trigger();
                markAttack(55.0);
            }
            if (at(snareCells[pattern]) || snareHit || divider_.risingEdge(1, 2)
                || (c.time + control[1] * 0.55 > 0.78 && (circuitStep_ & 7U) == 5U)) {
                snare_.trigger();
                markAttack(180.0);
            }
            if (at(hatCells[pattern]) || hatHit || divider_.risingEdge(0, 2)
                || c.density + c.risk + control[1] * 0.45 > 1.18)
                hat_.trigger((c.risk + control[2] * 0.55) > 0.72 && divider_.q(3));
            // O kit da seed (6/11): o palmas reforça os tempos fracos, o
            // cowbell pontua, o prato abre a frase, as maracas nos
            // contratempos — cada um com a sua palavra da seed.
            if ((drumKit_ & 2U) != 0U) {
                const auto word = scoreWord(seed_, cell + seedVariation_ * 31U, 5U);
                if (((cell & 7U) == 4U && (word & 0xffU) < 180U) || (word & 0xffU) < 12U + static_cast<std::uint32_t>(c.risk * 30.0))
                    clap_.trigger();
            }
            if ((drumKit_ & 1U) != 0U) {
                const auto word = scoreWord(seed_, cell + seedVariation_ * 31U, 6U);
                if ((cell & 1U) == 1U && (word & 0xffU) < 30U + static_cast<std::uint32_t>(c.density * 40.0))
                    cowbell_.trigger();
            }
            if ((drumKit_ & 4U) != 0U) {
                const auto word = scoreWord(seed_, cell + seedVariation_ * 31U, 7U);
                if (cell == 0U && (word & 0xffU) < 110U) cymbal_.trigger();
            }
            if ((drumKit_ & 8U) != 0U) {
                const auto word = scoreWord(seed_, cell + seedVariation_ * 31U, 8U);
                if ((cell & 1U) == 1U && (word & 0xffU) < 90U + static_cast<std::uint32_t>(c.density * 90.0))
                    maracas_.trigger();
            }
        }
        if (tearEdge_ && (drumKit_ & 4U) != 0U) cymbal_.trigger();
        const double drums = (static_cast<double>(kick_.process()) * (0.38 + control[0] * 0.88)
            + static_cast<double>(snare_.process()) * (0.20 + control[1] * 0.82)
            + static_cast<double>(hat_.process()) * (0.06 + control[2] * 0.88)
            + ((drumKit_ & 2U) != 0U ? static_cast<double>(clap_.process()) * (0.18 + control[1] * 0.40) : 0.0)
            + ((drumKit_ & 1U) != 0U ? static_cast<double>(cowbell_.process()) * (0.10 + control[2] * 0.28) : 0.0)
            + ((drumKit_ & 4U) != 0U ? static_cast<double>(cymbal_.process()) * (0.06 + control[2] * 0.22) : 0.0)
            + ((drumKit_ & 8U) != 0U ? static_cast<double>(maracas_.process()) * (0.05 + control[2] * 0.30) : 0.0))
            * (0.72 + counterArc_ * 0.42);
        return std::tanh(drums * (0.54 + control[1] * 0.72 + control[2] * 0.46 + c.tension * 0.30)
                         + runglerValue_ * (mutateResponse_ * 0.10 + control[1] * 0.28)
                         + autonomyEvent_ * 0.72)
            * (1.0 - holdResponse_ * 0.42);
    }

    static std::uint32_t scoreWord(std::uint32_t seed, std::uint32_t step,
                                   std::uint32_t lane) noexcept {
        std::uint32_t word = seed ^ (0x9e3779b9U * (step + 0x7f4a7c15U))
            ^ (0x85ebca6bU * (lane + 1U));
        word ^= word >> 16U;
        word *= 0x7feb352dU;
        word ^= word >> 15U;
        word *= 0x846ca68bU;
        return word ^ (word >> 16U);
    }
#endif
};

} // namespace rasgo::performance
