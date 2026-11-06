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
//
// Mais duas na mesma noite (passo 5, os últimos da auditoria):
// - PerformanceSpectralGhost: o "fantasma espectral" do Studio
//   (rasgo-synth-core/src/dsp/SpectralGhost.hpp): FFT de 1024 com Hann e
//   50 % de sobreposição, mantém a magnitude de cada faixa e troca a fase
//   por uma que deriva. O do Studio aloca no callback (std::vector, deque)
//   e sorteia com std::mt19937; aqui tudo é alocado no prepare, as fases
//   avançam com a faixa (deriva 0 = um fantasma coerente) e a magnitude
//   pode demorar (MEMÓRIA) e escurecer (BRILHO).
// - PerformanceStochasticWave: a onda de pontos que andam ao acaso (a ideia
//   do Gendyn de Xenakis), canto da estrela VECTOR — o Gendyn copiado do
//   Studio usa distribuições da biblioteca padrão.

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
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

class PerformanceSpectralGhost {
public:
    static constexpr std::size_t kSize = 1024U;
    static constexpr std::size_t kHop = kSize / 2U;

    void prepare() {
        constexpr double pi = 3.14159265358979323846;
        window_.assign(kSize, 0.0F);
        // Hann periódica (denominador N): soma constante a 50 % (a lição do Studio).
        for (std::size_t i = 0U; i < kSize; ++i)
            window_[i] = static_cast<float>(0.5 - 0.5 * std::cos(2.0 * pi * static_cast<double>(i) / kSize));
        twiddle_.assign(kSize / 2U, {});
        for (std::size_t k = 0U; k < kSize / 2U; ++k)
            twiddle_[k] = std::polar(1.0F, static_cast<float>(-2.0 * pi * static_cast<double>(k) / kSize));
        reverse_.assign(kSize, 0U);
        for (std::size_t i = 0U, j = 0U; i < kSize; ++i) {
            reverse_[i] = static_cast<std::uint16_t>(j);
            std::size_t bit = kSize >> 1U;
            for (; (j & bit) != 0U; bit >>= 1U) j ^= bit;
            j ^= bit;
        }
        spectrum_.assign(kSize, {});
        history_.assign(kHop, 0.0F);
        input_.assign(kHop, 0.0F);
        output_.assign(kHop, 0.0F);
        accum_.assign(kSize, 0.0F);
        magnitude_.assign(kSize / 2U, 0.0F);
        phase_.assign(kSize / 2U, 0.0F);
        fill_ = 0U;
    }
    void seed(std::uint32_t s) noexcept { random_.seed(s); }
    // 0: as fases só seguem a faixa (fantasma coerente); 1: deriva larga.
    void setDrift(double amount) noexcept { drift_ = static_cast<float>(std::clamp(amount, 0.0, 1.0)); }
    // 0: a magnitude de agora; 1: demora ~0,4 s para esquecer.
    void setMemory(double amount) noexcept { memory_ = static_cast<float>(std::clamp(amount, 0.0, 1.0) * 0.965); }
    // 0: escuro (corte nas faixas graves); 1: aberto.
    void setBrightness(double amount) noexcept {
        const double b = std::clamp(amount, 0.0, 1.0);
        cutoffBin_ = static_cast<float>(6.0 + b * b * 420.0);
    }

    // Uma amostra entra; sai o fantasma, com kHop amostras de atraso.
    float process(float x) noexcept {
        if (window_.empty()) return 0.0F;
        input_[fill_] = std::isfinite(x) ? x : 0.0F;
        const float y = output_[fill_];
        if (++fill_ == kHop) {
            block();
            fill_ = 0U;
        }
        return y;
    }

private:
    void fft(bool inverse) noexcept {
        for (std::size_t i = 0U; i < kSize; ++i)
            if (i < reverse_[i]) std::swap(spectrum_[i], spectrum_[reverse_[i]]);
        for (std::size_t len = 2U; len <= kSize; len <<= 1U) {
            const std::size_t step = kSize / len;
            for (std::size_t i = 0U; i < kSize; i += len)
                for (std::size_t k = 0U; k < len / 2U; ++k) {
                    const auto w = inverse ? std::conj(twiddle_[k * step]) : twiddle_[k * step];
                    const auto u = spectrum_[i + k];
                    const auto v = spectrum_[i + k + len / 2U] * w;
                    spectrum_[i + k] = u + v;
                    spectrum_[i + k + len / 2U] = u - v;
                }
        }
    }
    void block() noexcept {
        constexpr float pi = 3.14159265F;
        for (std::size_t i = 0U; i < kHop; ++i) {
            spectrum_[i] = {history_[i] * window_[i], 0.0F};
            spectrum_[kHop + i] = {input_[i] * window_[kHop + i], 0.0F};
            history_[i] = input_[i];
        }
        fft(false);
        spectrum_[0] = {};
        spectrum_[kSize / 2U] = {};
        for (std::size_t b = 1U; b < kSize / 2U; ++b) {
            const float mag = std::abs(spectrum_[b]);
            magnitude_[b] = magnitude_[b] * memory_ + mag * (1.0F - memory_);
            const float ratio = static_cast<float>(b) / cutoffBin_;
            const float gain = 1.0F / (1.0F + ratio * ratio);
            // A fase avança com a faixa (π·b por salto de N/2) e deriva ao acaso.
            float phase = phase_[b] + pi * static_cast<float>(b & 1U)
                + (static_cast<float>(random_.unit()) * 2.0F - 1.0F) * drift_ * pi;
            phase -= 2.0F * pi * std::floor(phase / (2.0F * pi));
            phase_[b] = phase;
            spectrum_[b] = std::polar(magnitude_[b] * gain, phase);
            spectrum_[kSize - b] = std::conj(spectrum_[b]);
        }
        fft(true);
        const float scale = 1.0F / static_cast<float>(kSize);
        for (std::size_t i = 0U; i < kSize; ++i) accum_[i] += spectrum_[i].real() * scale;
        for (std::size_t i = 0U; i < kHop; ++i) {
            output_[i] = std::isfinite(accum_[i]) ? accum_[i] : 0.0F;
            accum_[i] = accum_[kHop + i];
            accum_[kHop + i] = 0.0F;
        }
    }

    std::vector<float> window_, history_, input_, output_, accum_, magnitude_, phase_;
    std::vector<std::complex<float>> twiddle_, spectrum_;
    std::vector<std::uint16_t> reverse_;
    std::size_t fill_ = 0U;
    float drift_ = 0.3F, memory_ = 0.5F, cutoffBin_ = 120.0F;
    PerformanceStudioRandom random_{};
};

// Uma onda de oito pontos que andam ao acaso a cada ciclo: amplitude e
// duração de cada trecho num passeio limitado (reflete nas bordas).
class PerformanceStochasticWave {
public:
    static constexpr std::size_t kPoints = 8U;
    void seed(std::uint32_t s) noexcept {
        random_.seed(s);
        for (std::size_t i = 0U; i < kPoints; ++i) {
            amplitude_[i] = random_.unit() * 2.0 - 1.0;
            duration_[i] = 0.6 + random_.unit() * 0.8;
        }
    }
    void setSampleRate(double sampleRate) noexcept { sampleRate_ = std::max(8000.0, sampleRate); }
    void setFrequency(double hz) noexcept { hz_ = std::clamp(hz, 10.0, 4000.0); }
    void setStep(double step) noexcept { step_ = std::clamp(step, 0.0, 0.6); }
    float tick() noexcept {
        const std::size_t next = (point_ + 1U) % kPoints;
        const double segment = duration_[point_] * sampleRate_ / (hz_ * static_cast<double>(kPoints));
        const double t = std::min(1.0, position_ / std::max(1.0, segment));
        const double out = amplitude_[point_] + (amplitude_[next] - amplitude_[point_]) * t;
        position_ += 1.0;
        if (position_ >= segment) {
            position_ -= segment;
            point_ = next;
            amplitude_[point_] = reflect(amplitude_[point_] + (random_.unit() * 2.0 - 1.0) * step_, -1.0, 1.0);
            duration_[point_] = reflect(duration_[point_] + (random_.unit() * 2.0 - 1.0) * step_ * 0.5, 0.4, 1.6);
        }
        return static_cast<float>(out);
    }

private:
    static double reflect(double value, double low, double high) noexcept {
        if (value > high) value = 2.0 * high - value;
        if (value < low) value = 2.0 * low - value;
        return std::clamp(value, low, high);
    }
    std::array<double, kPoints> amplitude_{}, duration_{};
    std::size_t point_ = 0U;
    double position_ = 0.0, sampleRate_ = 48000.0, hz_ = 110.0, step_ = 0.15;
    PerformanceStudioRandom random_{};
};

} // namespace rasgo::performance
