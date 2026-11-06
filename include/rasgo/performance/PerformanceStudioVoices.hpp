#pragma once

// Duas vozes do Rasgo Synth Studio reescritas a partir do conceito (6/11/2026,
// auditoria dos módulos, passo 5). As fontes do Studio
// (rasgo-synth-core/src/dsp/GranularCloud.hpp e KSRandomizerCore.hpp, mesmo
// autor, GPLv3, só lidas) usam std::mt19937 com distribuições da biblioteca
// padrão, cujos resultados mudam de um compilador para outro — a mesma seed
// soaria diferente no Linux, no Windows e no macOS. Aqui o acaso é um
// xorshift próprio, idêntico em todo sistema; a ideia é a do Studio.
//
// - PerformanceGranularCloud: grãos com janela de Hann lidos do passado
//   recente de uma fonte (densidade, espalhamento, desvio de altura). O
//   buffer é alocado no prepare, nunca no callback.
// - PerformanceTuringRegister: o registrador de 16 bits do "Turing Machine"
//   (Klee/Whitwell, o KS Randomizer do Studio): a cada passo o bit que sai
//   volta, ou é trocado por um bit novo com a probabilidade dada — de um laço
//   travado a uma sequência que nunca se repete.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace rasgo::performance {

class PerformanceStudioRandom {
public:
    void seed(std::uint32_t s) noexcept { state_ = s == 0U ? 0x9E3779B9U : s; }
    std::uint32_t next() noexcept {
        state_ ^= state_ << 13U;
        state_ ^= state_ >> 17U;
        state_ ^= state_ << 5U;
        return state_;
    }
    double unit() noexcept { return static_cast<double>(next() >> 8U) / 16777216.0; }

private:
    std::uint32_t state_ = 0x9E3779B9U;
};

class PerformanceGranularCloud {
public:
    static constexpr std::size_t kMaxGrains = 12U;

    void prepare(double sampleRate, double bufferSeconds = 2.0) {
        sampleRate_ = std::max(8000.0, sampleRate);
        buffer_.assign(std::max<std::size_t>(1U, static_cast<std::size_t>(bufferSeconds * sampleRate_)), 0.0F);
        write_ = 0U;
        for (auto& grain : grains_) grain.active = false;
    }
    void seed(std::uint32_t s) noexcept { random_.seed(s); }
    void setDensity(double grainsPerSecond) noexcept { density_ = std::clamp(grainsPerSecond, 0.5, 80.0); }
    void setGrainSeconds(double minimum, double maximum) noexcept {
        grainMin_ = std::clamp(minimum, 0.01, 1.0);
        grainMax_ = std::clamp(maximum, grainMin_, 1.2);
    }
    void setSpread(double seconds) noexcept { spread_ = std::clamp(seconds, 0.02, 1.8); }
    void setPitchJitter(double semitones) noexcept { jitter_ = std::clamp(semitones, 0.0, 12.0); }

    // Uma amostra da fonte entra; sai a nuvem (só os grãos).
    float process(float input) noexcept {
        if (buffer_.empty()) return 0.0F;
        buffer_[write_] = std::isfinite(input) ? input : 0.0F;
        write_ = (write_ + 1U) % buffer_.size();
        if (random_.unit() < density_ / sampleRate_) spawn();
        double out = 0.0;
        for (auto& grain : grains_) {
            if (!grain.active) continue;
            const double phase = grain.age / grain.length;
            const double window = 0.5 - 0.5 * std::cos(6.283185307179586 * phase);
            const double size = static_cast<double>(buffer_.size());
            double at = grain.start + grain.age * grain.rate;
            at -= std::floor(at / size) * size;
            const auto i0 = static_cast<std::size_t>(at) % buffer_.size();
            const auto i1 = (i0 + 1U) % buffer_.size();
            const double frac = at - std::floor(at);
            out += (buffer_[i0] * (1.0 - frac) + buffer_[i1] * frac) * window;
            grain.age += 1.0;
            if (grain.age >= grain.length) grain.active = false;
        }
        return static_cast<float>(out * 0.5);
    }

private:
    struct Grain {
        bool active = false;
        double start = 0.0, age = 0.0, length = 1.0, rate = 1.0;
    };
    void spawn() noexcept {
        for (auto& grain : grains_) {
            if (grain.active) continue;
            const double back = (0.01 + random_.unit() * spread_) * sampleRate_;
            const double size = static_cast<double>(buffer_.size());
            grain.start = static_cast<double>(write_) - std::min(back, size - 1.0);
            if (grain.start < 0.0) grain.start += size;
            grain.length = std::max(32.0, (grainMin_ + random_.unit() * (grainMax_ - grainMin_)) * sampleRate_);
            grain.rate = std::pow(2.0, (random_.unit() * 2.0 - 1.0) * jitter_ / 12.0);
            grain.age = 0.0;
            grain.active = true;
            return;
        }
    }
    std::vector<float> buffer_;
    std::size_t write_ = 0U;
    std::array<Grain, kMaxGrains> grains_{};
    PerformanceStudioRandom random_{};
    double sampleRate_ = 48000.0, density_ = 12.0, grainMin_ = 0.04, grainMax_ = 0.18, spread_ = 0.6, jitter_ = 0.0;
};

class PerformanceTuringRegister {
public:
    void seed(std::uint32_t s) noexcept {
        random_.seed(s);
        bits_ = static_cast<std::uint16_t>(random_.next());
    }
    // 0 = laço travado (o bit volta sempre), 1 = sempre um bit novo.
    void setChange(double probability) noexcept { change_ = std::clamp(probability, 0.0, 1.0); }
    // Um passo: o registrador gira; devolve os 8 bits de baixo (0–255).
    std::uint8_t step() noexcept {
        auto out = static_cast<std::uint16_t>((bits_ >> 15U) & 1U);
        if (random_.unit() < change_) out = static_cast<std::uint16_t>(random_.next() & 1U);
        bits_ = static_cast<std::uint16_t>((bits_ << 1U) | out);
        return static_cast<std::uint8_t>(bits_ & 0xFFU);
    }

private:
    std::uint16_t bits_ = 0xA5A5U;
    double change_ = 0.15;
    PerformanceStudioRandom random_{};
};

} // namespace rasgo::performance
