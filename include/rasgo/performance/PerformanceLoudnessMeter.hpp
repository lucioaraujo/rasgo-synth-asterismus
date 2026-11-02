#pragma once

// Medidor de saída do Asterismus (30/10/2026): loudness BS.1770 (momentânea,
// 400 ms; curto prazo, 3 s) e true peak 4×, na saída depois do master —
// o padrão de medição dos instrumentos RASGO (docs/SAIDA_DE_AUDIO.md).
//
// Proveniência: cópia adaptada do Navalha 2 (repositório próprio, GPLv3, a
// mesma licença do Asterismus, autoria Lúcio de Araújo / RASGO):
// - ponderação K (os dois biquads da ITU-R BS.1770-4 pela transformação
//   bilinear na taxa real) de NAVALHA2_JUCE/src/core/MasteringAnalysis.cpp,
//   commit b1a9614;
// - detector de true peak (FIR polifásico 4×, janela Blackman-Harris de 4
//   termos, ramos normalizados) de NAVALHA2_JUCE/src/core/TruePeakDetector.*,
//   commit 34f8fd7, validado lá pelos casos EBU Tech 3341.
// Adaptações: um cabeçalho só, sem exceção (prepare limita a taxa), sem
// alocação; blocos de 100 ms num anel de 30 (3 s) para as janelas da norma
// (sem a integração com portas — essa é feita por ffmpeg nas tomadas).

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace rasgo::performance {

// Detector de true peak de um canal: FIR polifásico 4× (16 taps por fase),
// janela Blackman-Harris de 4 termos, ramos normalizados (ganho DC unitário em
// cada fase). Usado pelo medidor e pelo limitador do master. Latência: ~8
// amostras.
class PerformanceTruePeakDetector {
public:
    static constexpr std::size_t kOversampling = 4U;
    static constexpr std::size_t kTapsPerPhase = 16U;

    PerformanceTruePeakDetector() noexcept { prepare(); }

    void prepare() noexcept {
        constexpr double pi = 3.14159265358979323846;
        constexpr auto total = kOversampling * kTapsPerPhase;
        constexpr double centre = (static_cast<double>(total) - 1.0) * 0.5;
        std::array<double, total> prototype{};
        for (std::size_t tap = 0U; tap < total; ++tap) {
            const double position = (static_cast<double>(tap) - centre) / static_cast<double>(kOversampling);
            const double sinc = std::fabs(position) < 1.0e-12 ? 1.0 : std::sin(pi * position) / (pi * position);
            const double phase = 2.0 * pi * static_cast<double>(tap) / static_cast<double>(total - 1U);
            const double window = 0.35875 - 0.48829 * std::cos(phase) + 0.14128 * std::cos(2.0 * phase)
                - 0.01168 * std::cos(3.0 * phase);
            prototype[tap] = sinc * window;
        }
        for (std::size_t branch = 0U; branch < kOversampling; ++branch) {
            double sum = 0.0;
            for (std::size_t tap = 0U; tap < kTapsPerPhase; ++tap) sum += prototype[branch + tap * kOversampling];
            for (std::size_t tap = 0U; tap < kTapsPerPhase; ++tap)
                coefficients_[branch][tap] = prototype[branch + tap * kOversampling] / sum;
        }
        reset();
    }
    void reset() noexcept { history_.fill(0.0); writeIndex_ = 0U; }

    // O maior valor absoluto entre as quatro fases interpoladas.
    double process(double input) noexcept {
        history_[writeIndex_] = std::isfinite(input) ? input : 0.0;
        writeIndex_ = (writeIndex_ + 1U) % kTapsPerPhase;
        double peak = 0.0;
        for (const auto& branch : coefficients_) {
            double value = 0.0;
            auto read = writeIndex_;
            for (std::size_t tap = 0U; tap < kTapsPerPhase; ++tap) {
                read = read == 0U ? kTapsPerPhase - 1U : read - 1U;
                value += branch[tap] * history_[read];
            }
            peak = std::max(peak, std::fabs(value));
        }
        return peak;
    }

private:
    std::array<std::array<double, kTapsPerPhase>, kOversampling> coefficients_{};
    std::array<double, kTapsPerPhase> history_{};
    std::size_t writeIndex_ = 0U;
};

class PerformanceLoudnessMeter {
public:
    static constexpr std::size_t kBlocks = 30U;  // 30 × 100 ms = 3 s

    void prepare(double sampleRate) noexcept {
        sampleRate_ = std::clamp(std::isfinite(sampleRate) ? sampleRate : 48000.0, 8000.0, 384000.0);
        for (auto& filter : weighting_) filter.prepare(sampleRate_);
        blockLength_ = std::max<std::size_t>(1U, static_cast<std::size_t>(sampleRate_ * 0.1));
        // Retenção do true peak: decai a 1/e em 1 s (~8,7 dB/s), como num
        // medidor de mesa.
        peakDecay_ = std::exp(-1.0 / sampleRate_);
        reset();
    }

    void reset() noexcept {
        for (auto& filter : weighting_) filter.clear();
        for (auto& detector : truePeakDetectors_) detector.reset();
        blocks_.fill(0.0);
        blockCount_ = 0U;
        blockHead_ = 0U;
        blockSum_ = 0.0;
        blockFill_ = 0U;
        truePeak_ = 0.0;
        resetIntegrated();
    }

    // A loudness integrada recomeça (no PLAY, com a tomada).
    void resetIntegrated() noexcept {
        histogramCount_.fill(0U);
        histogramPower_.fill(0.0);
        maxTruePeak_ = 0.0;
    }

    void process(double left, double right) noexcept {
        const double l = weighting_[0].process(finite(left));
        const double r = weighting_[1].process(finite(right));
        blockSum_ += l * l + r * r;
        if (++blockFill_ >= blockLength_) {
            blocks_[blockHead_] = blockSum_ / static_cast<double>(blockLength_);
            blockHead_ = (blockHead_ + 1U) % kBlocks;
            blockCount_ = std::min(blockCount_ + 1U, kBlocks);
            blockSum_ = 0.0;
            blockFill_ = 0U;
            // Integrada (BS.1770-4): blocos de 400 ms com 75% de
            // sobreposição — um a cada 100 ms —, guardados num histograma
            // de 0,1 dB (memória fixa, qualquer duração).
            if (blockCount_ >= 4U) {
                double power = 0.0;
                for (std::size_t back = 1U; back <= 4U; ++back) power += blocks_[(blockHead_ + kBlocks - back) % kBlocks];
                power /= 4.0;
                const double loudness = power > 1.0e-12 ? -0.691 + 10.0 * std::log10(power) : -100.0;
                if (loudness > kAbsoluteGate) {
                    const auto bin = std::min<std::size_t>(kBins - 1U,
                        static_cast<std::size_t>((loudness - kAbsoluteGate) * 10.0));
                    ++histogramCount_[bin];
                    histogramPower_[bin] += power;
                }
            }
        }
        truePeak_ *= peakDecay_;
        truePeak_ = std::max({truePeak_, truePeakDetectors_[0].process(left), truePeakDetectors_[1].process(right)});
        maxTruePeak_ = std::max(maxTruePeak_, truePeak_);
    }

    // Loudness (LUFS) das últimas n janelas de 100 ms: 4 = momentânea,
    // 30 = curto prazo. −70 quando não há sinal.
    double loudness(std::size_t windows) const noexcept {
        const auto count = std::min(windows, blockCount_);
        if (count == 0U) return -70.0;
        double sum = 0.0;
        for (std::size_t back = 1U; back <= count; ++back)
            sum += blocks_[(blockHead_ + kBlocks - back) % kBlocks];
        const double mean = sum / static_cast<double>(count);
        return mean > 1.0e-12 ? std::max(-70.0, -0.691 + 10.0 * std::log10(mean)) : -70.0;
    }
    double momentary() const noexcept { return loudness(4U); }
    double shortTerm() const noexcept { return loudness(kBlocks); }
    // Loudness integrada desde o último recomeço, com as duas portas da
    // BS.1770-4: absoluta (−70 LUFS) e relativa (−10 LU abaixo da média dos
    // blocos acima da absoluta). −70 sem sinal.
    double integrated() const noexcept {
        double sum = 0.0;
        std::uint64_t count = 0U;
        for (std::size_t bin = 0U; bin < kBins; ++bin) {
            sum += histogramPower_[bin];
            count += histogramCount_[bin];
        }
        if (count == 0U) return -70.0;
        const double relativeGate = -0.691 + 10.0 * std::log10(sum / static_cast<double>(count)) - 10.0;
        double gatedSum = 0.0;
        std::uint64_t gatedCount = 0U;
        for (std::size_t bin = 0U; bin < kBins; ++bin) {
            const double binLoudness = kAbsoluteGate + (static_cast<double>(bin) + 0.5) / 10.0;
            if (binLoudness < relativeGate) continue;
            gatedSum += histogramPower_[bin];
            gatedCount += histogramCount_[bin];
        }
        if (gatedCount == 0U) return -70.0;
        return -0.691 + 10.0 * std::log10(gatedSum / static_cast<double>(gatedCount));
    }
    // O maior true peak desde o último recomeço (linear).
    double maxTruePeak() const noexcept { return maxTruePeak_; }

    // O true peak com retenção (linear; 1 = 0 dBTP).
    double truePeakLinear() const noexcept { return truePeak_; }

private:
    struct Biquad {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0, z1 = 0.0, z2 = 0.0;
        double process(double input) noexcept {
            const double output = input * b0 + z1;
            z1 = input * b1 - output * a1 + z2;
            z2 = input * b2 - output * a2;
            return output;
        }
    };
    struct KWeighting {
        Biquad pre, rlb;
        void prepare(double rate) noexcept {
            constexpr double pi = 3.14159265358979323846;
            {   // pré-filtro (difração da cabeça): +4 dB acima de ~1,7 kHz
                constexpr double f0 = 1681.9744509555319, gainDb = 3.99984385397, q = 0.7071752369554193;
                const double k = std::tan(pi * f0 / rate);
                const double vh = std::pow(10.0, gainDb / 20.0);
                const double vb = std::pow(vh, 0.4996667741545416);
                const double a0 = 1.0 + k / q + k * k;
                pre.b0 = (vh + vb * k / q + k * k) / a0;
                pre.b1 = 2.0 * (k * k - vh) / a0;
                pre.b2 = (vh - vb * k / q + k * k) / a0;
                pre.a1 = 2.0 * (k * k - 1.0) / a0;
                pre.a2 = (1.0 - k / q + k * k) / a0;
            }
            {   // RLB: passa-altas perto de 38 Hz
                constexpr double f0 = 38.13547087602444, q = 0.5003270373238773;
                const double k = std::tan(pi * f0 / rate);
                const double a0 = 1.0 + k / q + k * k;
                rlb.b0 = 1.0 / a0;
                rlb.b1 = -2.0 / a0;
                rlb.b2 = 1.0 / a0;
                rlb.a1 = 2.0 * (k * k - 1.0) / a0;
                rlb.a2 = (1.0 - k / q + k * k) / a0;
            }
        }
        void clear() noexcept { pre.z1 = pre.z2 = rlb.z1 = rlb.z2 = 0.0; }
        double process(double input) noexcept { return rlb.process(pre.process(input)); }
    };

    static double finite(double value) noexcept { return std::isfinite(value) ? value : 0.0; }

    double sampleRate_ = 48000.0;
    std::array<KWeighting, 2> weighting_{};
    std::array<PerformanceTruePeakDetector, 2> truePeakDetectors_{};
    std::array<double, kBlocks> blocks_{};
    std::size_t blockCount_ = 0U, blockHead_ = 0U, blockLength_ = 4800U, blockFill_ = 0U;
    double blockSum_ = 0.0;
    double truePeak_ = 0.0;
    double peakDecay_ = 0.99998;
    static constexpr double kAbsoluteGate = -70.0;
    static constexpr std::size_t kBins = 800U;  // −70 a +10 LUFS, 0,1 dB
    std::array<std::uint64_t, kBins> histogramCount_{};
    std::array<double, kBins> histogramPower_{};
    double maxTruePeak_ = 0.0;
};

} // namespace rasgo::performance
