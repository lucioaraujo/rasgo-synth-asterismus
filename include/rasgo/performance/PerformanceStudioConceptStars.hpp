#pragma once

// As últimas quatro estrelas da biblioteca (2/11/2026), reescritas a partir
// do conceito. No Rasgo Synth Studio eram comportamentos dentro do
// renderizador de peças (PieceRenderer.hpp, commit cfe1698, só lido), sem voz
// separável; nada do código de lá foi copiado.
//
// - THERMAL STATIC: no Studio, o fim térmico da peça — "o som sufocando e
//   perdendo harmônicos, como uma máquina ficando sem energia"; mais raro,
//   uma "morte abrupta" (um corte rápido, nunca um salto a zero) — e a
//   estática que o acompanha. Aqui vira uma vida térmica que se repete: um
//   zumbido com estática que esquenta, morre (devagar ou de repente), esfria
//   e volta.
// - CORE: no Studio, a voz melódica principal (o par de osciladores em FM
//   cruzado). Aqui, uma voz de FM cruzado que segue a frase do céu.
// - TEXTURE: no Studio, o "clima de ruído", a cama sempre presente. Aqui, um
//   ruído colorido (branco → marrom) por uma banda ressonante que anda.
// - BASS FALLBACK: no Studio, o grave "core-sub" que assume quando não há
//   Tuba nem Acid. Aqui, um baixo fraseado em registro sub/grave.
//
// Sem alocação, sem <random>; cada voz guarda o próprio estado.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace rasgo::performance {

namespace concept_detail {
inline constexpr double kTwoPi = 6.283185307179586;

struct Noise {
    std::uint32_t state = 0x7E57A71Cu;
    double white() noexcept {
        state ^= state << 13U;
        state ^= state >> 17U;
        state ^= state << 5U;
        return static_cast<double>(state >> 8U) / 8388608.0 - 1.0;
    }
};

// Filtro de variáveis de estado (Chamberlin, com o ganho trapezoidal de
// Simper): passa-baixas e passa-banda estáveis em toda a faixa útil.
struct StateVariable {
    double ic1 = 0.0, ic2 = 0.0, g = 0.1, k = 1.0, a1 = 1.0, a2 = 0.1, a3 = 0.01;
    void set(double hz, double q, double rate) noexcept {
        g = std::tan(3.141592653589793 * std::clamp(hz, 20.0, rate * 0.45) / rate);
        k = 1.0 / std::max(0.5, q);
        a1 = 1.0 / (1.0 + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }
    void process(double input, double& low, double& band) noexcept {
        const double v3 = input - ic2;
        const double v1 = a1 * ic1 + a2 * v3;
        const double v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0 * v1 - ic1;
        ic2 = 2.0 * v2 - ic2;
        low = v2;
        band = v1;
    }
};
} // namespace concept_detail

class PerformanceThermalStatic {
public:
    void prepare(double sampleRate, std::uint32_t seed) noexcept {
        rate_ = sampleRate;
        noise_.state = seed == 0U ? 0x7E57A71Cu : seed;
        heat_ = 0.0;
        life_ = 1.0;
        dying_ = abrupt_ = false;
        cooling_ = 0.0;
        deaths_ = abruptDeaths_ = 0U;
        phase_ = 0.0;
    }
    // CALOR: quão depressa esquenta (uma vida de ~40 s a ~6 s); MORTE: a
    // chance de a morte ser abrupta; ESTÁTICA: a densidade dos estalos.
    double tick(double hz, double heating, double abruptChance, double staticDensity) noexcept {
        if (!dying_) {
            heat_ += (0.025 + heating * 0.14) / rate_;
            if (heat_ >= 1.0) {
                dying_ = true;
                abrupt_ = noise_.white() * 0.5 + 0.5 < abruptChance;
                ++deaths_;
                if (abrupt_) ++abruptDeaths_;
            }
        } else if (life_ > 0.001) {
            // Morte lenta: ~3 s perdendo harmônicos; abrupta: ~0,25 s.
            life_ *= std::exp(-1.0 / ((abrupt_ ? 0.25 : 3.0) * rate_));
        } else if ((cooling_ += 1.0 / rate_) > 1.5) {
            // Esfriou: volta a viver.
            dying_ = false;
            heat_ = 0.0;
            life_ = 1.0;
            cooling_ = 0.0;
        }
        // O zumbido: fundamental com harmônicos que somem com a vida
        // (passa-baixas que desce) e um tremor que cresce com o calor.
        phase_ += hz * (1.0 + heat_ * heat_ * 0.015 * noise_.white()) / rate_;
        phase_ -= std::floor(phase_);
        const double saw = 2.0 * phase_ - 1.0;
        if ((++counter_ & 15U) == 0U) filter_.set(80.0 + 3800.0 * life_ * life_, 0.8, rate_);
        double low = 0.0, band = 0.0;
        filter_.process(saw, low, band);
        // A estática: estalos esparsos, mais densos quanto mais quente.
        const double crackleChance = (0.0004 + staticDensity * 0.004) * (0.3 + heat_);
        const double crackle = noise_.white() * 0.5 + 0.5 < crackleChance ? noise_.white() : 0.0;
        crackle_ = crackle_ * 0.6 + crackle;
        return (low * 0.42 + crackle_ * 0.5 * (0.4 + staticDensity)) * life_;
    }
    double heat() const noexcept { return heat_; }
    double life() const noexcept { return life_; }
    std::uint32_t deaths() const noexcept { return deaths_; }
    std::uint32_t abruptDeaths() const noexcept { return abruptDeaths_; }

private:
    double rate_ = 48000.0, heat_ = 0.0, life_ = 1.0, cooling_ = 0.0, phase_ = 0.0, crackle_ = 0.0;
    bool dying_ = false, abrupt_ = false;
    std::uint32_t deaths_ = 0U, abruptDeaths_ = 0U, counter_ = 0U;
    concept_detail::Noise noise_{};
    concept_detail::StateVariable filter_{};
};

class PerformanceCoreVoice {
public:
    void prepare(double sampleRate) noexcept {
        rate_ = sampleRate;
        carrier_ = modulator_ = 0.0;
        envelope_ = 0.0;
        feedback_ = 0.0;
    }
    void trigger(double hz) noexcept {
        hz_ = std::clamp(hz, 40.0, 2000.0);
        envelope_ = 1.0;
    }
    // FM: o índice cruzado (0–1); RAZÃO: a razão do modulador (1 a 3,5, em
    // meios: razões harmônicas); DECAIM.: 0,12 a 1,6 s.
    double tick(double index, double ratio, double decaySeconds) noexcept {
        const double r = 1.0 + std::round(std::clamp(ratio, 0.0, 1.0) * 5.0) * 0.5;
        // O par cruzado: o modulador recebe a saída anterior da portadora.
        modulator_ += (hz_ * r) / rate_;
        modulator_ -= std::floor(modulator_);
        const double mod = std::sin(concept_detail::kTwoPi * modulator_ + feedback_ * index * 0.8);
        carrier_ += hz_ / rate_;
        carrier_ -= std::floor(carrier_);
        const double out = std::sin(concept_detail::kTwoPi * carrier_ + mod * index * 3.0 * envelope_);
        feedback_ = out;
        envelope_ *= std::exp(-1.0 / (std::clamp(decaySeconds, 0.05, 4.0) * rate_));
        return out * envelope_ * 0.5;
    }

private:
    double rate_ = 48000.0, hz_ = 220.0, carrier_ = 0.0, modulator_ = 0.0, envelope_ = 0.0, feedback_ = 0.0;
};

class PerformanceTextureBed {
public:
    void prepare(double sampleRate, std::uint32_t seed) noexcept {
        rate_ = sampleRate;
        noise_.state = seed == 0U ? 0x7E87u : seed;
        pinkState_.fill(0.0);
        brown_ = 0.0;
        drift_ = 0.0;
    }
    // COR: branco → rosa → marrom; FOCO: a ressonância da banda; MOVIMENTO:
    // a velocidade com que a banda anda (com a densidade do céu).
    double tick(double centreHz, double colour, double focus, double motion, double density) noexcept {
        const double white = noise_.white();
        // Rosa (Paul Kellet, versão econômica) e marrom (integrador com fuga).
        pinkState_[0] = 0.99765 * pinkState_[0] + white * 0.0990460;
        pinkState_[1] = 0.96300 * pinkState_[1] + white * 0.2965164;
        pinkState_[2] = 0.57000 * pinkState_[2] + white * 1.0526913;
        const double pink = (pinkState_[0] + pinkState_[1] + pinkState_[2] + white * 0.1848) * 0.2;
        brown_ = std::clamp(brown_ * 0.995 + white * 0.04, -1.0, 1.0);
        const double c = std::clamp(colour, 0.0, 1.0);
        const double coloured = c < 0.5 ? white * (1.0 - 2.0 * c) * 0.35 + pink * 2.0 * c
                                         : pink * (2.0 - 2.0 * c) + brown_ * (2.0 * c - 1.0) * 2.2;
        drift_ += (0.05 + motion * 0.6) * (0.5 + density) / rate_;
        drift_ -= std::floor(drift_);
        const double sweep = std::pow(2.0, 1.6 * std::sin(concept_detail::kTwoPi * drift_));
        if ((++counter_ & 15U) == 0U) band_.set(std::clamp(centreHz * sweep, 60.0, 9000.0), 0.7 + focus * 9.0, rate_);
        double low = 0.0, band = 0.0;
        band_.process(coloured, low, band);
        return (band * (0.5 + focus * 0.4) + coloured * (1.0 - focus) * 0.25) * 0.8;
    }

private:
    double rate_ = 48000.0, brown_ = 0.0, drift_ = 0.0;
    std::array<double, 3> pinkState_{};
    std::uint32_t counter_ = 0U;
    concept_detail::Noise noise_{};
    concept_detail::StateVariable band_{};
};

class PerformanceBassFallback {
public:
    void prepare(double sampleRate) noexcept {
        rate_ = sampleRate;
        phase_ = 0.0;
        level_ = 0.0;
        gateLeft_ = 0U;
    }
    // REGISTRO: sub (duas oitavas abaixo), grave (uma) ou médio-grave.
    void trigger(double hz, double registerChoice, double gateSeconds) noexcept {
        const double down = registerChoice < 0.33 ? 0.25 : registerChoice < 0.66 ? 0.5 : 1.0;
        target_ = std::clamp(hz * down, 28.0, 220.0);
        if (hz_ <= 0.0) hz_ = target_;
        gateLeft_ = static_cast<std::uint32_t>(std::max(0.03, gateSeconds) * rate_);
    }
    // ATAQUE: 2 a 80 ms. Seno com saturação suave (corpo sem sub sujo).
    double tick(double attackShape) noexcept {
        hz_ += (target_ - hz_) * 0.0015;
        const double attack = 1.0 - std::exp(-1.0 / ((0.002 + attackShape * 0.078) * rate_));
        const double release = 1.0 - std::exp(-1.0 / (0.09 * rate_));
        if (gateLeft_ > 0U) {
            --gateLeft_;
            level_ += (1.0 - level_) * attack;
        } else {
            level_ += (0.0 - level_) * release;
        }
        phase_ += hz_ / rate_;
        phase_ -= std::floor(phase_);
        const double sine = std::sin(concept_detail::kTwoPi * phase_);
        return std::tanh(sine * 1.6) * 0.62 * level_;
    }

private:
    double rate_ = 48000.0, phase_ = 0.0, level_ = 0.0, hz_ = 0.0, target_ = 55.0;
    std::uint32_t gateLeft_ = 0U;
};

} // namespace rasgo::performance
