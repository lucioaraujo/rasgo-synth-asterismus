#pragma once

// Os cortes do céu (6/11/2026, auditoria dos módulos do Studio, passo 4: os
// glitches como gestos do céu). Conceitos do Rasgo Synth Studio
// (rasgo-synth-core/src/dsp/ParenthesisGlitch.hpp, MontageEditCut.hpp,
// ApocryphalTear.hpp e SampleHoldGlitch.hpp — mesmo autor, GPLv3, só lidos),
// reescritos para o tempo real do Asterismus, com acaso determinístico:
//
// - PARÊNTESE: o instante de agora (0,12–0,45 s) preso e repetido por
//   0,8–2,2 s; o céu continua por baixo e a saída volta para onde ele está.
// - CORTE DE MONTAGEM: um salto limpo para 6–18 s atrás, tocado reto por
//   0,8–2 s, e o salto de volta.
// - APÓCRIFO: um fragmento de 8–19 s atrás, ao contrário e corroído (5–7
//   bits), sobre o som ao vivo abafado — a memória rasgada.
// - DECIMAÇÃO: a saída só aceita um valor novo a cada 4–40 amostras (com
//   jitter) por 0,8–2 s; cada valor é do próprio sinal, nunca amplificado.
//
// Estágio na soma estéreo, antes do master (o limitador vem depois). A
// memória (20 s estéreo) é alocada no prepare, nunca no callback. Os stems
// gravados não passam por aqui: durante um corte a soma dos stems não
// reconstrói a mistura.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace rasgo::performance {

class PerformanceSkyCuts {
public:
    enum class Kind : std::uint8_t { None = 0, Parenthesis, Montage, Apocryphal, Decimate };

    void prepare(double sampleRate, double memorySeconds = 20.0) {
        rate_ = std::max(8000.0, sampleRate);
        const auto size = std::max<std::size_t>(1024U, static_cast<std::size_t>(memorySeconds * rate_));
        left_.assign(size, 0.0F);
        right_.assign(size, 0.0F);
        write_ = 0U;
        filled_ = 0U;
        kind_ = Kind::None;
    }
    void reset() noexcept {
        std::fill(left_.begin(), left_.end(), 0.0F);
        std::fill(right_.begin(), right_.end(), 0.0F);
        write_ = 0U;
        filled_ = 0U;
        kind_ = Kind::None;
    }

    // Um corte começa (do céu). `word` é a palavra da seed para ele.
    bool trigger(Kind kind, std::uint32_t word) noexcept {
        if (kind == Kind::None || left_.empty() || kind_ != Kind::None) return false;
        random_ = word == 0U ? 0x2545F491U : word;
        const double seconds = 0.8 + unit() * 1.4;
        length_ = static_cast<std::uint64_t>(seconds * rate_);
        age_ = 0U;
        const auto size = static_cast<double>(left_.size());
        switch (kind) {
        case Kind::Parenthesis: {
            loop_ = std::max<std::uint64_t>(256U, static_cast<std::uint64_t>((0.12 + unit() * 0.33) * rate_));
            if (filled_ < loop_ + 16U) return false;
            origin_ = static_cast<double>(write_) - static_cast<double>(loop_);
            break;
        }
        case Kind::Montage: {
            const double back = (6.0 + unit() * 12.0) * rate_;
            if (static_cast<double>(filled_) < back + 16.0) return false;
            origin_ = static_cast<double>(write_) - back;
            length_ = static_cast<std::uint64_t>((0.8 + unit() * 1.2) * rate_);
            break;
        }
        case Kind::Apocryphal: {
            const double back = (8.0 + unit() * 11.0) * rate_;
            if (static_cast<double>(filled_) < back + 16.0) return false;
            origin_ = static_cast<double>(write_) - back;
            length_ = static_cast<std::uint64_t>((0.6 + unit() * 0.8) * rate_);
            bits_ = 5 + static_cast<int>(unit() * 3.0);
            break;
        }
        case Kind::Decimate:
            holdMin_ = 4U;
            holdMax_ = 6U + static_cast<std::uint32_t>(unit() * 34.0);
            holdLeft_ = 0U;
            break;
        case Kind::None: break;
        }
        while (origin_ < 0.0) origin_ += size;
        kind_ = kind;
        return true;
    }

    Kind active() const noexcept { return kind_; }

    // Uma amostra estéreo passa: entra na memória e, se há corte, sai cortada.
    void process(double& left, double& right) noexcept {
        if (left_.empty()) return;
        left_[write_] = static_cast<float>(std::isfinite(left) ? left : 0.0);
        right_[write_] = static_cast<float>(std::isfinite(right) ? right : 0.0);
        write_ = (write_ + 1U) % left_.size();
        if (filled_ < left_.size()) ++filled_;
        if (kind_ == Kind::None) return;
        const auto size = left_.size();
        // Bordas: 8 ms de passagem (2 ms no apócrifo, que rasga).
        const double edgeSamples = rate_ * (kind_ == Kind::Apocryphal ? 0.002 : 0.008);
        const double fromStart = static_cast<double>(age_);
        const double toEnd = static_cast<double>(length_ - std::min(age_, length_));
        const double mixIn = std::clamp(std::min(fromStart, toEnd) / std::max(1.0, edgeSamples), 0.0, 1.0);
        double cutLeft = left, cutRight = right;
        switch (kind_) {
        case Kind::Parenthesis: {
            const auto at = static_cast<std::size_t>(origin_ + static_cast<double>(age_ % loop_)) % size;
            cutLeft = left_[at];
            cutRight = right_[at];
            break;
        }
        case Kind::Montage: {
            const auto at = static_cast<std::size_t>(origin_ + static_cast<double>(age_)) % size;
            cutLeft = left_[at];
            cutRight = right_[at];
            break;
        }
        case Kind::Apocryphal: {
            double at = origin_ + static_cast<double>(length_) - static_cast<double>(age_);
            while (at < 0.0) at += static_cast<double>(size);
            const auto i = static_cast<std::size_t>(at) % size;
            const double steps = std::pow(2.0, bits_ - 1);
            const auto crush = [steps](double x) { return std::round(std::clamp(x, -1.0, 1.0) * steps) / steps; };
            cutLeft = left * 0.25 + crush(left_[i] * 1.0);
            cutRight = right * 0.25 + crush(right_[i] * 1.0);
            break;
        }
        case Kind::Decimate: {
            if (holdLeft_ == 0U) {
                heldLeft_ = left;
                heldRight_ = right;
                holdLeft_ = holdMin_ + static_cast<std::uint32_t>(unit() * static_cast<double>(holdMax_ - holdMin_));
            }
            --holdLeft_;
            cutLeft = heldLeft_;
            cutRight = heldRight_;
            break;
        }
        case Kind::None: break;
        }
        left = left * (1.0 - mixIn) + cutLeft * mixIn;
        right = right * (1.0 - mixIn) + cutRight * mixIn;
        if (++age_ >= length_) kind_ = Kind::None;
    }

private:
    double unit() noexcept {
        random_ ^= random_ << 13U;
        random_ ^= random_ >> 17U;
        random_ ^= random_ << 5U;
        return static_cast<double>(random_ >> 8U) / 16777216.0;
    }
    std::vector<float> left_, right_;
    std::size_t write_ = 0U, filled_ = 0U;
    double rate_ = 48000.0, origin_ = 0.0, heldLeft_ = 0.0, heldRight_ = 0.0;
    std::uint64_t length_ = 0U, age_ = 0U, loop_ = 1024U;
    std::uint32_t random_ = 0x2545F491U, holdMin_ = 4U, holdMax_ = 20U, holdLeft_ = 0U;
    int bits_ = 6;
    Kind kind_ = Kind::None;
};

} // namespace rasgo::performance
