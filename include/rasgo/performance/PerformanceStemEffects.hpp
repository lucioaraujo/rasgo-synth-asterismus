#pragma once

// Filtros por stem e os envios do céu (6/11/2026). Pedido do autor:
// "precisamos fazer os filtros para cada stem, vcf, etc"; "eco, delay";
// "sim, faça os filtros e os envios, com os botões"; "os filtros também podem
// ser submetidos às escolhas da seed". Peças da família (family/PROVENANCE.md):
//
// - PerformanceStemFilter: cinco filtros à escolha da seed — SVF GRAVE,
//   BANDA e AGUDO (o núcleo TPT do Filter do Rasgo Modular, com a saturação
//   no laço que prende a auto-oscilação), CMOS (o CmosVcf do Antitotem, com a
//   máscara de saídas) e MATÉRIA (o MaterialFilter do Antitotem, carregado e
//   assimétrico).
// - PerformanceEchoBus: o eco do céu (núcleo do Space do Modular): tomadas na
//   linha, realimentação com filtro de tom, all-pass de difusão na saída; os
//   tempos presos ao andamento.
// - PerformanceHallBus: a sala do céu (núcleo do Hall do Modular): oito
//   linhas, matriz de Householder, amortecimento e modulação por linha.
//
// Toda memória é alocada no prepare; nada aloca no callback.

#include "family/antitotem/CmosVcf.h"
#include "family/antitotem/MaterialFilter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace rasgo::performance {

class PerformanceStemFilter {
public:
    enum class Type : std::uint8_t { SvfLow = 0, SvfBand, SvfHigh, Cmos, Material, Count };

    void prepare(double sampleRate) noexcept {
        rate_ = std::max(8000.0, sampleRate);
        cmos_.prepare(rate_);
        material_.prepare(rate_);
        reset();
    }
    void reset() noexcept { ic1_ = ic2_ = 0.0f; cmos_.reset(); material_.reset(); }

    // cutoff 0–1 (25 Hz a ~18 kHz, exponencial), resonance 0–1, character 0–1
    // (a máscara do CMOS ou o drive/assimetria do MATÉRIA).
    void set(Type type, double cutoff, double resonance, double character) noexcept {
        if (type != type_) reset();
        type_ = type;
        cutoff_ = static_cast<float>(std::clamp(cutoff, 0.0, 1.0));
        resonance_ = static_cast<float>(std::clamp(resonance, 0.0, 1.0));
        character_ = static_cast<float>(std::clamp(character, 0.0, 1.0));
    }

    float process(float x) noexcept {
        if (!std::isfinite(x)) x = 0.0f;
        switch (type_) {
        case Type::SvfLow:
        case Type::SvfBand:
        case Type::SvfHigh: {
            // os coeficientes só mudam com o corte e a ressonância (6/11: a
            // tangente por amostra em oito estrelas pesava no callback)
            if (cutoff_ != coefCutoff_ || resonance_ != coefResonance_) {
                coefCutoff_ = cutoff_;
                coefResonance_ = resonance_;
                const float fc = std::clamp(25.0f * std::exp2(cutoff_ * 9.5f), 15.0f, static_cast<float>(rate_ * 0.45));
                // o Filter do Modular: Q de 0,5 a ~13, um fio negativo perto
                // do máximo (auto-oscilação presa pela saturação do estado)
                const float r = resonance_ * 0.85f;
                const float q = 0.5f * std::exp2(r * 4.7f);
                const float r4 = r * r * r * r;
                k_ = 1.0f / q - r4 * 0.012f;
                g_ = std::tan(3.14159265f * fc / static_cast<float>(rate_));
                a1_ = 1.0f / (1.0f + g_ * (g_ + k_));
                gainBand_ = 1.0f + r;
            }
            const float g = g_, k = k_, a1 = a1_;
            const float v1 = a1 * (ic1_ + g * (x - ic2_));
            const float v2 = ic2_ + g * v1;
            ic1_ = 2.0f * saturate(v1) - ic1_;
            ic2_ = 2.0f * saturate(v2) - ic2_;
            if (!std::isfinite(ic1_) || !std::isfinite(ic2_)) { ic1_ = ic2_ = 0.0f; return 0.0f; }
            const float out = type_ == Type::SvfLow ? v2 : type_ == Type::SvfBand ? v1 * gainBand_ : x - k * v1 - v2;
            return softLimit(out);
        }
        case Type::Cmos: {
            static constexpr std::array<unsigned char, 5> masks{{
                antitotem::CmosVcf::lowpass, antitotem::CmosVcf::lowpass | antitotem::CmosVcf::bandpass,
                antitotem::CmosVcf::bandpass, antitotem::CmosVcf::notch,
                antitotem::CmosVcf::bandpass | antitotem::CmosVcf::highpass}};
            cmos_.setModeMask(masks[std::min<std::size_t>(static_cast<std::size_t>(character_ * 5.0f), 4U)]);
            cmos_.setCutoff(cutoff_);
            cmos_.setResonance(resonance_ * 0.88f);
            cmos_.setCvDepth(0.0f);
            return softLimit(cmos_.process(x, 0.5f));
        }
        case Type::Material:
            material_.setCutoff(cutoff_);
            material_.setResonance(resonance_ * 0.9f);
            material_.setDrive(0.15f + character_ * 0.6f);
            material_.setAsymmetry(character_);
            material_.setMix(1.0f);
            // o MaterialFilter satura o que entra: o stem chega baixo (antes
            // do ganho fixo), então ele trabalha numa escala e volta
            return material_.process(x * 12.0f, 0.5f) / 12.0f;
        case Type::Count: break;
        }
        return x;
    }

    static const char* name(Type type) noexcept {
        switch (type) {
        case Type::SvfLow: return "SVF GRAVE";
        case Type::SvfBand: return "SVF BANDA";
        case Type::SvfHigh: return "SVF AGUDO";
        case Type::Cmos: return "CMOS";
        case Type::Material: return "MATÉRIA";
        case Type::Count: break;
        }
        return "";
    }

private:
    static float saturate(float v) noexcept {
        if (v > 1.0f) return 1.0f + std::tanh(v - 1.0f);
        if (v < -1.0f) return -1.0f + std::tanh(v + 1.0f);
        return v;
    }
    static float softLimit(float v) noexcept {
        if (v > 0.8f) return 0.8f + 0.2f * std::tanh((v - 0.8f) * 5.0f);
        if (v < -0.8f) return -0.8f + 0.2f * std::tanh((v + 0.8f) * 5.0f);
        return v;
    }
    double rate_ = 48000.0;
    Type type_ = Type::SvfLow;
    float cutoff_ = 1.0f, resonance_ = 0.0f, character_ = 0.0f;
    float ic1_ = 0.0f, ic2_ = 0.0f;
    float coefCutoff_ = -1.0f, coefResonance_ = -1.0f, g_ = 0.1f, k_ = 1.0f, a1_ = 1.0f, gainBand_ = 1.0f;
    antitotem::CmosVcf cmos_{};
    antitotem::MaterialFilter material_{};
};

class PerformanceEchoBus {
public:
    void prepare(double sampleRate) {
        rate_ = std::max(8000.0, sampleRate);
        buffer_.assign(static_cast<std::size_t>(rate_ * 2.2), 0.0f);
        write_ = 0U;
        lp_ = 0.0f;
        lfo_ = 0.0f;
        for (std::size_t i = 0U; i < kAllpass; ++i) {
            ap_[i].assign(kApLen[i], 0.0f);
            apPos_[i] = 0U;
        }
        smoothTime_ = -1.0f;
    }
    // seconds (0,02–2), feedback 0–0,9, tone 0–1, diffusion 0–1, taps 1–4
    void set(double seconds, double feedback, double tone, double diffusion, int taps) noexcept {
        time_ = static_cast<float>(std::clamp(seconds, 0.02, 2.0));
        feedback_ = static_cast<float>(std::clamp(feedback, 0.0, 0.9));
        tone_ = static_cast<float>(std::clamp(tone, 0.0, 1.0));
        diffusion_ = static_cast<float>(std::clamp(diffusion, 0.0, 1.0));
        taps_ = std::clamp(taps, 1, 4);
    }
    void process(float in, float& outLeft, float& outRight) noexcept {
        if (buffer_.empty()) { outLeft = outRight = 0.0f; return; }
        if (smoothTime_ < 0.0f) smoothTime_ = time_;
        smoothTime_ += (time_ - smoothTime_) * 0.0005f;  // sem salto de tomada
        lfo_ += 0.13f / static_cast<float>(rate_);
        if (lfo_ >= 1.0f) lfo_ -= 1.0f;
        const float mod = std::sin(6.2831853f * lfo_) * 0.002f * static_cast<float>(rate_) * 0.15f;
        const float base = smoothTime_ * static_cast<float>(rate_);
        float left = 0.0f, right = 0.0f;
        for (int k = 0; k < taps_; ++k) {
            const float frac = static_cast<float>(k + 1) / static_cast<float>(taps_);
            const float d = base * (0.4f + 0.6f * frac) + mod * (k % 2 == 0 ? 1.0f : -1.0f);
            const float v = read(d) * (1.0f - 0.12f * static_cast<float>(k));
            (k % 2 == 0 ? left : right) += v;
        }
        if (taps_ == 1) right = left;
        const float wet = 0.5f * (left + right);
        lp_ += (0.08f + tone_ * 0.9f) * (wet - lp_);
        buffer_[write_] = std::tanh((std::isfinite(in) ? in : 0.0f) + feedback_ * lp_);
        write_ = (write_ + 1U) % buffer_.size();
        float diffused = wet;
        for (std::size_t i = 0U; i < kAllpass; ++i) diffused = allpass(i, diffused);
        outLeft = (1.0f - diffusion_) * left + diffusion_ * diffused;
        outRight = (1.0f - diffusion_) * right + diffusion_ * diffused;
    }

private:
    static constexpr std::size_t kAllpass = 4U;
    static constexpr std::array<std::size_t, kAllpass> kApLen{{223U, 353U, 523U, 739U}};
    float read(float delay) const noexcept {
        const auto n = static_cast<float>(buffer_.size());
        float d = std::clamp(delay, 1.0f, n - 2.0f);
        float p = static_cast<float>(write_) - d;
        while (p < 0.0f) p += n;
        const auto i0 = static_cast<std::size_t>(p) % buffer_.size();
        const auto i1 = (i0 + 1U) % buffer_.size();
        const float f = p - std::floor(p);
        return buffer_[i0] * (1.0f - f) + buffer_[i1] * f;
    }
    float allpass(std::size_t i, float x) noexcept {
        const float delayed = ap_[i][apPos_[i]];
        const float v = x - 0.6f * delayed;
        ap_[i][apPos_[i]] = v;
        apPos_[i] = (apPos_[i] + 1U) % ap_[i].size();
        return delayed + 0.6f * v;
    }
    std::vector<float> buffer_;
    std::array<std::vector<float>, kAllpass> ap_{};
    std::array<std::size_t, kAllpass> apPos_{};
    std::size_t write_ = 0U;
    double rate_ = 48000.0;
    float time_ = 0.375f, smoothTime_ = -1.0f, feedback_ = 0.35f, tone_ = 0.5f, diffusion_ = 0.3f, lp_ = 0.0f, lfo_ = 0.0f;
    int taps_ = 2;
};

class PerformanceHallBus {
public:
    static constexpr std::size_t kLines = 8U;
    void prepare(double sampleRate) {
        rate_ = std::max(8000.0f, static_cast<float>(sampleRate));
        const float scale = rate_ / 48000.0f;
        std::size_t longest = 0U;
        for (std::size_t i = 0U; i < kLines; ++i) {
            len_[i] = static_cast<float>(kBase[i]) * scale;
            longest = std::max(longest, static_cast<std::size_t>(len_[i] * 1.7f) + 64U);
        }
        for (std::size_t i = 0U; i < kLines; ++i) {
            line_[i].assign(longest, 0.0f);
            write_[i] = 0U;
            lp_[i] = 0.0f;
            phase_[i] = static_cast<float>(i) * 0.1234f;
        }
        set(0.5, 0.5, 0.4);
    }
    // size 0–1, decay 0–1 (RT60 de 0,2 a ~15 s), damping 0–1
    void set(double size, double decay, double damping) noexcept {
        size_ = 0.3f + static_cast<float>(std::clamp(size, 0.0, 1.0)) * 1.4f;
        const float rt60 = 0.2f * std::pow(75.0f, static_cast<float>(std::clamp(decay, 0.0, 1.0)));
        for (std::size_t i = 0U; i < kLines; ++i)
            gain_[i] = std::pow(10.0f, -3.0f * (len_[i] * size_ / rate_) / rt60);
        const float fc = 18000.0f - static_cast<float>(std::clamp(damping, 0.0, 1.0)) * 16000.0f;
        damp_ = 1.0f - std::exp(-6.28318530718f * fc / rate_);
    }
    void process(float in, float& outLeft, float& outRight) noexcept {
        if (line_[0].empty()) { outLeft = outRight = 0.0f; return; }
        static constexpr std::array<float, kLines> rates{{0.53f, 0.61f, 0.74f, 0.83f, 0.97f, 1.09f, 1.21f, 1.37f}};
        static constexpr std::array<float, kLines> inVec{{0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f}};
        std::array<float, kLines> s{}, out{};
        float h = 0.0f;
        // a modulação das linhas é lenta (0,5–1,4 Hz): recalculada a cada 32
        // amostras, não a cada uma (6/11, custo do callback)
        if (modCountdown_ == 0U) {
            modCountdown_ = 32U;
            for (std::size_t i = 0U; i < kLines; ++i) {
                phase_[i] += rates[i] * 32.0f / rate_;
                phase_[i] -= std::floor(phase_[i]);
                mod_[i] = 9.0f * std::sin(6.28318530718f * phase_[i]);
            }
        }
        --modCountdown_;
        for (std::size_t i = 0U; i < kLines; ++i) {
            const float d = len_[i] * size_ + mod_[i];
            float v = read(i, d);
            lp_[i] += (v - lp_[i]) * damp_;
            out[i] = lp_[i];
            s[i] = lp_[i] * gain_[i];
            h += s[i];
        }
        h *= 2.0f / static_cast<float>(kLines);
        const float x = std::isfinite(in) ? in : 0.0f;
        for (std::size_t i = 0U; i < kLines; ++i) {
            float w = x * inVec[i] + (s[i] - h);
            if (w < 1.0e-18f && w > -1.0e-18f) w = 0.0f;
            line_[i][write_[i]] = w;
            write_[i] = (write_[i] + 1U) % line_[i].size();
        }
        outLeft = (out[0] - out[2] + out[4] - out[6]) * 0.6f;
        outRight = (out[1] - out[3] + out[5] - out[7]) * 0.6f;
    }

private:
    static constexpr std::array<int, kLines> kBase{{1237, 1381, 1607, 1777, 1949, 2137, 2273, 2477}};
    float read(std::size_t i, float delay) const noexcept {
        const auto& b = line_[i];
        const auto n = static_cast<float>(b.size());
        float p = static_cast<float>(write_[i]) - std::clamp(delay, 1.0f, n - 2.0f);
        while (p < 0.0f) p += n;
        const auto i0 = static_cast<std::size_t>(p) % b.size();
        const auto i1 = (i0 + 1U) % b.size();
        const float f = p - std::floor(p);
        return b[i0] * (1.0f - f) + b[i1] * f;
    }
    std::array<std::vector<float>, kLines> line_{};
    std::array<std::size_t, kLines> write_{};
    std::array<float, kLines> len_{}, lp_{}, phase_{}, gain_{}, mod_{};
    std::uint32_t modCountdown_ = 0U;
    float rate_ = 48000.0f, size_ = 1.0f, damp_ = 0.5f;
};

} // namespace rasgo::performance
