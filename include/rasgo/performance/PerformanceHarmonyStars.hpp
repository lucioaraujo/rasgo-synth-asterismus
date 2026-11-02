#pragma once

// Duas estrelas reescritas a partir do conceito (2/11/2026) — no Rasgo Synth
// Studio elas eram comportamentos dentro do renderizador de peças
// (PieceRenderer.hpp, fases 77 e 79, commit cfe1698, só lido), sem voz
// separável. Nada do código de lá foi copiado; os conceitos são citados:
//
// - HARMONAIG: pad de quatro vozes raiz/3ª/5ª/7ª, a partir do quantizador de
//   acordes de 4 vozes harmonàig da Instruō (Jason Lim, Glasgow) — no Studio,
//   "Harmonaig-inspired chord pad", com inversão e abertura escolhidas pela
//   seed. Aqui a inversão e a abertura são controles, e o acorde anda sozinho
//   pelo modo do céu, em quartas, a cada tantas batidas.
// - BLOOM: melodia fractal Tronco/Galho/Caminho, a partir do sequenciador
//   Bloom da Qu-Bit Electronix (Andrew Ikenberry) — no Studio, um tronco de 5
//   intervalos e um galho de 25 (o tronco aplicado a si mesmo), com o
//   caminho alternando entre os dois níveis ao fim de cada volta. Aqui o
//   CAMINHO é a chance de trocar de nível, e as alturas caem no modo do céu.
//
// Sem alocação, sem <random>; cada voz guarda o próprio estado.

#include <dsp/Envelope.hpp>
#include <dsp/Oscillator.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace rasgo::performance {

// O modo do céu, completo (dórico): os acordes se empilham em terças dentro
// dele, e a qualidade de cada acorde (m7, maj7, 7, m7♭5) vem do modo.
inline constexpr std::array<int, 7> kPerformanceSkyMode{{0, 2, 3, 5, 7, 9, 10}};

inline double performanceModeSemitones(int degree) noexcept {
    const int octave = degree >= 0 ? degree / 7 : -((-degree + 6) / 7);
    const int index = degree - octave * 7;
    return static_cast<double>(octave * 12 + kPerformanceSkyMode[static_cast<std::size_t>(index)]);
}

class PerformanceHarmonaigPad {
public:
    void prepare(double sampleRate) noexcept {
        sampleRate_ = sampleRate;
        glide_ = 1.0 - std::exp(-1.0 / (0.09 * sampleRate_));
        for (std::size_t voice = 0U; voice < 4U; ++voice) {
            envelopes_[voice].setSampleRate(sampleRate_);
            // Ataque lento e soltura longa, cada voz um pouco defasada: as
            // quatro não incham juntas.
            envelopes_[voice].setADSR(0.45 + 0.13 * static_cast<double>(voice), 0.25, 0.85,
                                      1.3 + 0.27 * static_cast<double>(voice));
            phase_[voice] = 0.25 * static_cast<double>(voice);
            hz_[voice] = target_[voice] = 0.0;
        }
        chord_ = 0;
    }

    // Um acorde novo sobre a fundamental: grau da raiz no modo (andando em
    // quartas), inversão 0–3 (que tom vai ao grave) e abertura 0–1 (uma ou
    // duas oitavas entre as vozes de cima).
    void voice(double fundamentalHz, int rootDegree, int inversion, double spread) noexcept {
        for (std::size_t voice = 0U; voice < 4U; ++voice) {
            const int tone = static_cast<int>((voice + static_cast<std::size_t>(inversion)) % 4U);
            double semitones = performanceModeSemitones(rootDegree + tone * 2);
            // As vozes acima do grave sobem uma oitava; com abertura, as duas
            // de cima sobem mais uma.
            if (voice > 0U && tone < inversion % 4) semitones += 12.0;
            if (voice >= 2U) semitones += 12.0 * std::clamp(spread, 0.0, 1.0);
            target_[voice] = std::clamp(fundamentalHz * std::pow(2.0, semitones / 12.0), 30.0, 3000.0);
            if (hz_[voice] <= 0.0) hz_[voice] = target_[voice];
            envelopes_[voice].noteOn();
        }
        ++chord_;
    }
    void release() noexcept {
        for (auto& envelope : envelopes_) envelope.noteOff();
    }

    double tick() noexcept {
        double out = 0.0;
        for (std::size_t voice = 0U; voice < 4U; ++voice) {
            hz_[voice] += (target_[voice] - hz_[voice]) * glide_;
            phase_[voice] += hz_[voice] / sampleRate_;
            phase_[voice] -= std::floor(phase_[voice]);
            // Seno com um pouco de 2º harmônico: corpo sem brilho áspero.
            const double angle = phase_[voice] * 6.283185307179586;
            const double tone = std::sin(angle) + 0.12 * std::sin(2.0 * angle);
            out += tone * static_cast<double>(envelopes_[voice].tick());
        }
        return out * 0.2;
    }
    std::uint32_t chords() const noexcept { return chord_; }

private:
    double sampleRate_ = 48000.0;
    double glide_ = 0.0002;
    std::array<rasgo::Envelope, 4> envelopes_{};
    std::array<double, 4> phase_{}, hz_{}, target_{};
    std::uint32_t chord_ = 0U;
};

class PerformanceBloomMelody {
public:
    void prepare(double sampleRate, std::uint32_t seed) noexcept {
        oscillator_.setSampleRate(sampleRate);
        oscillator_.setWaveform(rasgo::Oscillator::Waveform::Saw);
        envelope_.setSampleRate(sampleRate);
        envelope_.setADSR(0.012, 0.09, 0.32, 0.14);
        lowpass_ = 0.0;
        lowpassCoefficient_ = 1.0 - std::exp(-6.283185307179586 * 2400.0 / sampleRate);
        reseed(seed);
    }
    // O tronco: cinco passos de −4 a +4 graus do modo; o galho: o tronco
    // aplicado a si mesmo (25 passos, cada um o tronco[i] + tronco[j]).
    void reseed(std::uint32_t seed) noexcept {
        state_ = seed == 0U ? 0xB100Fu : seed;
        for (auto& step : trunk_) step = static_cast<int>(draw() * 9.0) - 4;
        for (std::size_t i = 0U; i < 5U; ++i)
            for (std::size_t j = 0U; j < 5U; ++j) branch_[i * 5U + j] = trunk_[i] + trunk_[j];
        level_ = draw() < 0.5 ? 0 : 1;
        step_ = 0U;
        switches_ = 0U;
    }

    // Uma nota: o próximo passo do nível atual. Ao fim de uma volta, troca de
    // nível com a chance do CAMINHO. Devolve o grau tocado.
    int step(double fundamentalHz, double path, double octave) noexcept {
        const std::size_t length = level_ == 0 ? trunk_.size() : branch_.size();
        const int degree = level_ == 0 ? trunk_[step_ % trunk_.size()] : branch_[step_ % branch_.size()];
        if (++step_ >= length) {
            step_ = 0U;
            if (draw() < std::clamp(path, 0.0, 1.0)) {
                level_ = 1 - level_;
                ++switches_;
            }
        }
        const double semitones = performanceModeSemitones(degree) + 12.0 * octave;
        oscillator_.setFrequency(std::clamp(fundamentalHz * std::pow(2.0, semitones / 12.0), 40.0, 2400.0));
        envelope_.noteOn();
        gateLeft_ = gateSamples_;
        return degree;
    }
    void setGateSamples(std::uint32_t samples) noexcept { gateSamples_ = std::max<std::uint32_t>(1U, samples); }

    double tick() noexcept {
        if (gateLeft_ > 0U && --gateLeft_ == 0U) envelope_.noteOff();
        const double raw = static_cast<double>(oscillator_.tick() * envelope_.tick());
        lowpass_ += (raw - lowpass_) * lowpassCoefficient_;
        return lowpass_ * 0.55;
    }
    int level() const noexcept { return level_; }
    std::uint32_t levelSwitches() const noexcept { return switches_; }
    const std::array<int, 5>& trunk() const noexcept { return trunk_; }
    const std::array<int, 25>& branch() const noexcept { return branch_; }

private:
    double draw() noexcept {
        state_ ^= state_ << 13U;
        state_ ^= state_ >> 17U;
        state_ ^= state_ << 5U;
        return static_cast<double>(state_ >> 8U) / 16777216.0;
    }

    rasgo::Oscillator oscillator_{};
    rasgo::Envelope envelope_{};
    double lowpass_ = 0.0, lowpassCoefficient_ = 0.25;
    std::array<int, 5> trunk_{};
    std::array<int, 25> branch_{};
    int level_ = 0;
    std::size_t step_ = 0U;
    std::uint32_t switches_ = 0U, gateLeft_ = 0U, gateSamples_ = 4800U;
    std::uint32_t state_ = 0xB100Fu;
};

} // namespace rasgo::performance
