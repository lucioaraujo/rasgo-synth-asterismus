#pragma once

// Blefe e confiança como estrela do céu (30/10/2026).
//
// Proveniência: cópia adaptada de RASGO_SYNTH_STUDIO/rasgo-synth-core/src/
// sequencer/BluffSignal.hpp (monorepo rasgo-instruments, commit cfe1698,
// SHA-256 no vendor/studio-dsp/PROVENANCE.md; mesma autoria e licença).
// O modelo é o do Studio: um SINAL soa igual sempre, genuíno ou blefe
// (informação incompleta); o genuíno é seguido, depois de uma janela de
// expectativa, por um gesto de CONFIRMAÇÃO; o blefe, por nada. A CONFIANÇA é
// uma média móvel exponencial das resoluções; a TRAIÇÃO só existe depois de
// uma confiança construída, e depois dela a resposta se cala.
//
// Adaptações ao Asterismus (instrumento ao vivo, tocado):
// - sem std::mt19937 nem <random>: um xorshift de 32 bits, sem alocação;
// - a janela de expectativa conta batidas da estrela (não compassos de uma
//   peça renderizada) e o sinal pendente se resolve sozinho;
// - REORGANIZAÇÃO: o Studio parava na traição (a peça acabava); aqui, depois
//   de uma traição, oito sinais depois a confiança recomeça neutra — a
//   dramaturgia do autor ("aproximação → cooperação → dependência → traição →
//   reorganização") inteira, porque a performance continua.

#include <algorithm>
#include <cstdint>

namespace rasgo::performance {

class PerformanceBluffSignal {
public:
    struct Beat {
        bool signal = false;        // tocar o gesto de sinal (sempre o mesmo)
        bool confirmation = false;  // tocar a confirmação (só de genuíno)
    };

    void configure(double truthfulness, std::uint32_t seed) noexcept {
        truthfulness_ = std::clamp(truthfulness, 0.0, 1.0);
        state_ = seed == 0U ? 0x6D2B79F5U : seed;
        trust_ = 0.5;
        everTrusted_ = betrayed_ = pending_ = false;
        pendingGenuine_ = false;
        wait_ = 0U;
        sinceBetrayal_ = 0U;
    }
    void setTruthfulness(double value) noexcept { truthfulness_ = std::clamp(value, 0.0, 1.0); }

    // Uma batida da estrela. chance: probabilidade de um sinal novo nesta
    // batida (quando não há um pendente); window: batidas de expectativa.
    Beat beat(double chance, std::uint32_t window) noexcept {
        Beat out{};
        if (pending_) {
            if (wait_ > 0U) --wait_;
            if (wait_ == 0U) {
                out.confirmation = pendingGenuine_ && !betrayed_;
                resolve();
            }
            return out;
        }
        if (unit() >= chance) return out;
        pendingGenuine_ = unit() < truthfulness_;
        pending_ = true;
        wait_ = std::max<std::uint32_t>(1U, window);
        out.signal = true;
        if (betrayed_ && ++sinceBetrayal_ >= kReorganizeAfter) {
            betrayed_ = everTrusted_ = false;
            trust_ = 0.5;
            sinceBetrayal_ = 0U;
        }
        return out;
    }

    double trust() const noexcept { return trust_; }
    bool betrayed() const noexcept { return betrayed_; }
    // O ganho da resposta: a confiança atual; zero depois da traição.
    double responseGain() const noexcept { return betrayed_ ? 0.0 : trust_; }

private:
    static constexpr double kAlpha = 0.25;
    static constexpr double kTrusted = 0.65;
    static constexpr double kBetrayal = 0.35;
    static constexpr std::uint32_t kReorganizeAfter = 8U;

    void resolve() noexcept {
        trust_ = std::clamp(trust_ + kAlpha * ((pendingGenuine_ ? 1.0 : 0.0) - trust_), 0.0, 1.0);
        pending_ = false;
        if (trust_ > kTrusted) everTrusted_ = true;
        if (!betrayed_ && everTrusted_ && trust_ < kBetrayal) betrayed_ = true;
    }
    double unit() noexcept {
        state_ ^= state_ << 13U;
        state_ ^= state_ >> 17U;
        state_ ^= state_ << 5U;
        return static_cast<double>(state_ >> 8U) / 16777216.0;
    }

    double truthfulness_ = 0.7;
    double trust_ = 0.5;
    bool everTrusted_ = false, betrayed_ = false, pending_ = false, pendingGenuine_ = false;
    std::uint32_t wait_ = 0U, sinceBetrayal_ = 0U;
    std::uint32_t state_ = 0x6D2B79F5U;
};

} // namespace rasgo::performance
