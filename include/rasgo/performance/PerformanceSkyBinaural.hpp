#pragma once

// A sombra binaural do céu (7/11/2026, decisão do autor: opção B da
// auditoria dos módulos do Studio, §10). O conceito é o do batimento binaural
// caótico do Rasgo Synth Studio (PieceRenderer, fase 73 — mesmo autor, GPLv3,
// só lido): dois senos, cada um audível só no seu canal, quase em uníssono;
// a pequena diferença de frequência entre L e R não está em nenhum dos dois
// canais — o ouvido a ouve como uma pulsação (o batimento binaural). Lá ele
// seguia a voz principal; aqui segue a raiz harmônica do céu, como camada
// (não estrela): as estrelas continuam mono, cada uma onde está.
//
// - Só funciona de fone: em caixas os canais se misturam no ar e viram um
//   batimento comum. Nenhuma promessa de "ondas cerebrais".
// - A portadora desliza (~0,6 s) até a raiz que o diretor dá; a diferença
//   entre os canais anda devagar em volta da base da seed (o "acelera e
//   desacelera" do Studio), com acaso próprio, determinístico.
// - Estágio aditivo na soma estéreo, antes do master; o motor o multiplica
//   pelo ganho que já cala o SILÊNCIO e o fim do STOP.

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace rasgo::performance {

class PerformanceSkyBinaural {
public:
    void prepare(double sampleRate) noexcept {
        sampleRate_ = std::max(8000.0, sampleRate);
        glide_ = 1.0 - std::exp(-1.0 / (0.6 * sampleRate_));
        reset();
    }
    void seed(std::uint32_t s) noexcept { random_ = s == 0U ? 0x9E3779B9U : s; }
    void reset() noexcept {
        phaseLeft_ = phaseRight_ = 0.0;
        carrierHz_ = 0.0;
        drift_ = driftTarget_ = 0.0;
        driftCountdown_ = 0;
    }

    // Uma amostra: carrierHz é a raiz para onde a portadora desliza, beatHz a
    // base da diferença entre os canais, gain o nível (0 cala).
    void process(double carrierHz, double beatHz, double gain, double& left, double& right) noexcept {
        if (!std::isfinite(carrierHz) || carrierHz <= 0.0) return;
        if (carrierHz_ <= 0.0) carrierHz_ = carrierHz;
        carrierHz_ += (carrierHz - carrierHz_) * glide_;
        // A diferença anda: um alvo novo a cada 2–5 s, alcançado devagar.
        if (--driftCountdown_ <= 0) {
            driftTarget_ = unit() * 2.0 - 1.0;
            driftCountdown_ = static_cast<int>((2.0 + unit() * 3.0) * sampleRate_);
        }
        drift_ += (driftTarget_ - drift_) * (1.0 / (1.5 * sampleRate_));
        const double beat = std::max(0.5, beatHz * (1.0 + 0.5 * drift_));
        lastBeatHz_ = beat;
        constexpr double tau = 6.283185307179586;
        phaseLeft_ += carrierHz_ / sampleRate_;
        phaseRight_ += (carrierHz_ + beat) / sampleRate_;
        phaseLeft_ -= std::floor(phaseLeft_);
        phaseRight_ -= std::floor(phaseRight_);
        if (!(gain > 0.0)) return;
        left += std::sin(tau * phaseLeft_) * gain;
        right += std::sin(tau * phaseRight_) * gain;
    }

    double carrierHz() const noexcept { return carrierHz_; }
    double beatHz() const noexcept { return lastBeatHz_; }

private:
    double unit() noexcept {
        random_ ^= random_ << 13U;
        random_ ^= random_ >> 17U;
        random_ ^= random_ << 5U;
        return static_cast<double>(random_ >> 8U) / 16777216.0;
    }
    double sampleRate_ = 48000.0, glide_ = 1.0e-4;
    double phaseLeft_ = 0.0, phaseRight_ = 0.0, carrierHz_ = 0.0;
    double drift_ = 0.0, driftTarget_ = 0.0, lastBeatHz_ = 0.0;
    int driftCountdown_ = 0;
    std::uint32_t random_ = 0xB1A0A11U;
};

} // namespace rasgo::performance
