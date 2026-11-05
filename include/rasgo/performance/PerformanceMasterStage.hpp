#pragma once

// O master do Asterismus (30/10/2026). Pedido do autor: "verifique a
// excelência de saída de áudio, parece que está baixo o som"; "seguir o
// padrão dos outros instrumentos, como o Navalha 2 ou o Antitotem — padrão de
// qualidade de saída de som, já pensando nas gravações prontas, padrão
// álbum".
//
// Medido antes (ffmpeg ebur128, BS.1770): as tomadas de 29–30/10 saíam entre
// −50 e −44 LUFS, pico de −31 a −28 dBFS — cerca de 30 dB abaixo de um
// álbum (~−14 LUFS, true peak ≤ −1 dBTP). O ganho de saída do motor
// (0,06 + densidade·0,19 + intervenção·0,05) vem de quando o instrumento era
// uma camada de senoides; não havia nivelador nem limitador, só um teto suave
// em 0,98.
//
// Cadeia, depois da soma estéreo, segundo a arquitetura comum de saída RASGO
// (RASGO_DOCUMENTATION/architecture/SAIDA_AUDIO_COMUM.md): DC fora → orçamento
// de ganho fixo (+26 dB; sem normalização automática da performance ao vivo)
// → MASTER (o trim do performer) → limitador com antecipação de 2 ms e
// detecção de true peak 4× → joelho suave até o teto (0,85 ≈ −1,41 dBTP). Os
// stems gravados continuam antes do master (crus, para mixagem).
//
// Proveniência: técnica adaptada de ANTITOTEM/src/core/OutputStage.h (DC,
// limitador com antecipação, joelho suave), repositório próprio do
// Antitotem, commit 645e5ad ("Antitotem — Objeto Sonoro v0.1.0"), autoria
// Lúcio de Araújo / RASGO. Licença: este arquivo herda a GNU
// AGPL-3.0-or-later do Antitotem; o código autoral do Asterismus é GPLv3, e a
// combinação é permitida (GPLv3 §13), como já é com o JUCE pela via AGPLv3.
// O autor, titular dos dois, pode relicenciar este trecho (ver
// THIRD_PARTY_NOTICES.md). Adaptações: precisão dupla; ganho fixo no lugar
// do SignalLeveler (usado na primeira versão e retirado no mesmo dia — a
// arquitetura comum proíbe normalizar a performance ao vivo); detector de
// true peak 4× no limitador; o MASTER entre o ganho e o limitador.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

#include "PerformanceLoudnessMeter.hpp"

namespace rasgo::performance {

class PerformanceMasterStage {
public:
    static constexpr double kCeiling = 0.85;   // ≈ −1,4 dBFS antes da conversão
    static constexpr double kKneeStart = 0.72;
    static constexpr std::size_t kMaxLookahead = 1024U;

    void prepare(double sampleRate) noexcept {
        sampleRate_ = std::max(8000.0, sampleRate);
        dcCoefficient_ = std::exp(-2.0 * 3.14159265358979323846 * 18.0 / sampleRate_);
        lookahead_ = std::clamp<std::size_t>(static_cast<std::size_t>(sampleRate_ * 0.002), 1U, kMaxLookahead);
        attack_ = coefficient(0.35);
        release_ = coefficient(90.0);
        reset();
    }

    void reset() noexcept {
        delayLeft_.fill(0.0);
        delayRight_.fill(0.0);
        position_ = 0U;
        previousInLeft_ = previousInRight_ = previousOutLeft_ = previousOutRight_ = 0.0;
        envelope_ = 0.0;
        limiterGain_ = 1.0;
        for (auto& detector : truePeak_) detector.reset();
    }

    // O MASTER do performer, linear (0–1); age depois do ganho fixo.
    void setMasterGain(double gain) noexcept { master_ = std::clamp(gain, 0.0, 1.0); }
    // O ganho de compensação (orçamento de ganho), em dB.
    void setMakeupDb(double decibels) noexcept { makeup_ = std::pow(10.0, std::clamp(decibels, -24.0, 36.0) / 20.0); }
    double makeupGain() const noexcept { return makeup_; }
    double limiterGain() const noexcept { return limiterGain_; }

    void process(double& left, double& right) noexcept {
        // DC fora antes de tudo.
        const double cleanLeft = dcBlock(finite(left), previousInLeft_, previousOutLeft_);
        const double cleanRight = dcBlock(finite(right), previousInRight_, previousOutRight_);
        // Orçamento de ganho fixo (30/10): a arquitetura comum de saída RASGO
        // (RASGO_DOCUMENTATION/architecture/SAIDA_AUDIO_COMUM.md §3) pede que a
        // loudness informe a decisão, mas que uma performance ao vivo não seja
        // normalizada automaticamente — a primeira versão deste master tinha
        // um nivelador automático (AGC) e foi corrigida. O ganho de
        // compensação é constante; a dinâmica da performance fica verdadeira.
        const double gain = makeup_ * master_;
        const double inLeft = cleanLeft * gain;
        const double inRight = cleanRight * gain;
        // Limitador com antecipação: o ganho reage ao pico que ainda vai
        // sair do atraso de 2 ms. O detector é o true peak 4× (30/10): os
        // picos entre amostras também ficam abaixo do teto (latência do
        // detector ~8 amostras, dentro da antecipação de ~96).
        const double peak = std::max(truePeak_[0].process(inLeft), truePeak_[1].process(inRight));
        // Envelope de pico: sobe na hora (o detector oscila de amostra para
        // amostra — uma subida suave fazia média e deixava o true peak passar
        // ~0,35 dB do teto); desce devagar. A suavização fica no ganho, que
        // tem os 2 ms de antecipação para chegar antes do pico.
        envelope_ = peak > envelope_ ? peak : envelope_ + (peak - envelope_) * release_;
        const double target = envelope_ > kCeiling ? kCeiling / envelope_ : 1.0;
        limiterGain_ += (target - limiterGain_) * (target < limiterGain_ ? attack_ : release_);
        left = softCeiling(delayLeft_[position_] * limiterGain_);
        right = softCeiling(delayRight_[position_] * limiterGain_);
        delayLeft_[position_] = inLeft;
        delayRight_[position_] = inRight;
        position_ = (position_ + 1U) % lookahead_;
    }

private:
    static double finite(double value) noexcept { return std::isfinite(value) ? value : 0.0; }
    double coefficient(double milliseconds) const noexcept {
        return 1.0 - std::exp(-1.0 / std::max(1.0, milliseconds * sampleRate_ / 1000.0));
    }
    double dcBlock(double value, double& previousInput, double& previousOutput) const noexcept {
        const double result = value - previousInput + dcCoefficient_ * previousOutput;
        previousInput = value;
        previousOutput = result;
        return result;
    }
    static double softCeiling(double value) noexcept {
        const double magnitude = std::fabs(value);
        if (magnitude <= kKneeStart) return value;
        const double curved = kKneeStart + (kCeiling - kKneeStart)
            * (1.0 - std::exp(-(magnitude - kKneeStart) / (kCeiling - kKneeStart)));
        return std::copysign(curved, value);
    }

    double sampleRate_ = 48000.0;
    double dcCoefficient_ = 0.997;
    std::size_t lookahead_ = 96U;
    double attack_ = 0.5, release_ = 0.01;
    double previousInLeft_ = 0.0, previousInRight_ = 0.0, previousOutLeft_ = 0.0, previousOutRight_ = 0.0;
    double envelope_ = 0.0, limiterGain_ = 1.0;
    double master_ = 1.0;
    // +26 dB (30/10): o motor somava ~−44 LUFS com o MASTER em −6 dB; com
    // +26 dB a peça típica ficava perto de −18 LUFS e os picos medidos
    // (−33 dBTP) chegavam a ~−7 dBTP — o limitador só age em excessos.
    // +35 dB (5/11): a forma por partes, a atividade e o nível de referência
    // por estrela (PerformanceStarLevels.hpp) deixaram a peça mais rala — as
    // tomadas da escuta guiada saíam entre −36 e −30 LUFS, o limitador sem
    // agir; a sonda `niveis` (48 seeds × 90 s, MASTER −6 dB) media −28,0
    // LUFS de média. +9 dB a devolve à faixa de −19 LUFS.
    double makeup_ = 56.234132519034908;
    std::array<double, kMaxLookahead> delayLeft_{};
    std::array<double, kMaxLookahead> delayRight_{};
    std::size_t position_ = 0U;
    std::array<PerformanceTruePeakDetector, 2> truePeak_{};
};

} // namespace rasgo::performance
