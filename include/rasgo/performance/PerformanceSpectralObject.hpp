#pragma once

// Objeto espectral como estrela do céu (30/10/2026): uma só estrutura de
// proporções que é ao mesmo tempo altura, ritmo e timbre — o parcial p soa
// na frequência f0·p (esticada pela inarmonicidade) E pulsa p vezes por
// período-base.
//
// Proveniência: cópia adaptada de RASGO_SYNTH_STUDIO/rasgo-synth-core/src/
// dsp/SpectralObject.hpp (monorepo rasgo-instruments, commit cfe1698,
// SHA-256 no vendor/studio-dsp/PROVENANCE.md; mesma autoria e licença). Os
// três eixos são os do Studio:
//   A — harmonicidade ↔ inarmonicidade, f_p = f0·p·√(1 + B·p²) (Fletcher e
//       Rossing, The Physics of Musical Instruments, o modelo de cordas de
//       piano e sinos);
//   B — buracos de peneira (Xenakis) no espectro: um parcial soa se o seu
//       índice satisfaz uma condição módulo/resíduo; o fundamental nunca
//       some;
//   C — ritmo espectral: o parcial p pulsa a cada período-base/p.
// Conjuntos de parciais: série completa 1–6; ímpar 1,3,5,…,11 (tubo
// fechado — Benade, Fundamentals of Musical Acoustics); primos 1,2,3,5,7,11.
//
// Adaptações para tempo real: sem <random>, <sstream>, std::string nem
// std::vector (condições em std::array, xorshift de 32 bits); a afinação só
// é recalculada quando f0 ou B mudam; o período-base vem do relógio da
// estrela (o chamador mede as batidas); as fases dos pulsos se realinham no
// início de cada período, para o objeto respirar junto do céu.

#include <dsp/Envelope.hpp>
#include <dsp/Oscillator.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace rasgo::performance {

class PerformanceSpectralObject {
public:
    static constexpr std::size_t kPartials = 6U;
    enum class PartialSet : std::uint8_t { FullHarmonic, OddHarmonic, PrimeWeighted };

    void prepare(double sampleRate, std::uint32_t seed) noexcept {
        sampleRate_ = sampleRate;
        state_ = seed == 0U ? 0x9E3779B9U : seed;
        for (std::size_t i = 0U; i < kPartials; ++i) {
            oscillators_[i].setSampleRate(sampleRate_);
            oscillators_[i].setWaveform(rasgo::Oscillator::Waveform::Sine);
            envelopes_[i].setSampleRate(sampleRate_);
            envelopes_[i].setADSR(0.008 + unit() * 0.015, 0.18 + unit() * 0.25, 0.0, 0.05 + unit() * 0.05);
            countdown_[i] = 0U;
            admitted_[i] = true;
        }
        set_ = PartialSet::FullHarmonic;
        partials_ = partialNumbers(set_);
        fundamental_ = 0.0;
        inharmonicity_ = -1.0;
        regenerateHoles();
    }

    void setPartialSet(PartialSet set) noexcept {
        if (set == set_) return;
        set_ = set;
        partials_ = partialNumbers(set_);
        retune();
    }
    // f0 em Hz e B (0 a 0,004): só reafina se algo mudou de fato.
    void setTuning(double fundamentalHz, double inharmonicity) noexcept {
        const double b = std::clamp(inharmonicity, 0.0, 0.004);
        if (std::fabs(fundamentalHz - fundamental_) < 1.0e-6 && std::fabs(b - inharmonicity_) < 1.0e-9) return;
        fundamental_ = fundamentalHz;
        inharmonicity_ = b;
        retune();
    }
    // Novo período-base (amostras): os pulsos recomeçam alinhados.
    void beginPeriod(std::size_t basePeriodSamples) noexcept {
        for (std::size_t i = 0U; i < kPartials; ++i) {
            period_[i] = std::max<std::size_t>(1U, basePeriodSamples / static_cast<std::size_t>(partials_[i]));
            countdown_[i] = 0U;
        }
    }
    // Eixo B: uma ou duas condições módulo/resíduo novas (módulo 2–4).
    void regenerateHoles() noexcept {
        const std::size_t conditions = 1U + (unit() < 0.5 ? 1U : 0U);
        std::array<std::array<std::size_t, 2>, 2> rule{};
        for (std::size_t c = 0U; c < conditions; ++c) {
            const auto modulus = 2U + static_cast<std::size_t>(unit() * 3.0);
            rule[c] = {modulus, static_cast<std::size_t>(unit() * static_cast<double>(modulus))};
        }
        admittedCount_ = 0U;
        for (std::size_t i = 0U; i < kPartials; ++i) {
            bool open = i == 0U;
            for (std::size_t c = 0U; c < conditions; ++c) open = open || i % rule[c][0] == rule[c][1];
            admitted_[i] = open;
            admittedCount_ += open ? 1U : 0U;
        }
    }

    double tick() noexcept {
        double out = 0.0;
        for (std::size_t i = 0U; i < kPartials; ++i) {
            if (admitted_[i] && period_[i] > 0U) {
                if (countdown_[i] == 0U) {
                    envelopes_[i].noteOn();
                    countdown_[i] = period_[i];
                }
                --countdown_[i];
            }
            const double voice = static_cast<double>(oscillators_[i].tick() * envelopes_[i].tick());
            if (audible_[i]) out += voice / partials_[i];
        }
        return out;
    }

    std::size_t admittedCount() const noexcept { return admittedCount_; }
    bool admitted(std::size_t i) const noexcept { return i < kPartials && admitted_[i]; }
    double partialFrequency(std::size_t i) const noexcept {
        const double p = partials_[std::min(i, kPartials - 1U)];
        return fundamental_ * p * std::sqrt(1.0 + std::max(0.0, inharmonicity_) * p * p);
    }

    static constexpr std::array<double, kPartials> partialNumbers(PartialSet set) noexcept {
        switch (set) {
        case PartialSet::OddHarmonic: return {{1.0, 3.0, 5.0, 7.0, 9.0, 11.0}};
        case PartialSet::PrimeWeighted: return {{1.0, 2.0, 3.0, 5.0, 7.0, 11.0}};
        default: return {{1.0, 2.0, 3.0, 4.0, 5.0, 6.0}};
        }
    }

private:
    void retune() noexcept {
        const double nyquistGuard = sampleRate_ * 0.45;
        for (std::size_t i = 0U; i < kPartials; ++i) {
            const double frequency = partialFrequency(i);
            // Um parcial acima de 0,45·fs cala (não dobra nem vira DC).
            audible_[i] = frequency > 0.0 && frequency < nyquistGuard;
            if (audible_[i]) oscillators_[i].setFrequency(frequency);
        }
    }
    double unit() noexcept {
        state_ ^= state_ << 13U;
        state_ ^= state_ >> 17U;
        state_ ^= state_ << 5U;
        return static_cast<double>(state_ >> 8U) / 16777216.0;
    }

    double sampleRate_ = 48000.0;
    double fundamental_ = 0.0;
    double inharmonicity_ = -1.0;
    PartialSet set_ = PartialSet::FullHarmonic;
    std::array<double, kPartials> partials_{{1.0, 2.0, 3.0, 4.0, 5.0, 6.0}};
    std::array<rasgo::Oscillator, kPartials> oscillators_{};
    std::array<rasgo::Envelope, kPartials> envelopes_{};
    std::array<bool, kPartials> admitted_{};
    std::array<bool, kPartials> audible_{};
    std::array<std::size_t, kPartials> period_{};
    std::array<std::size_t, kPartials> countdown_{};
    std::size_t admittedCount_ = 0U;
    std::uint32_t state_ = 0x9E3779B9U;
};

} // namespace rasgo::performance
