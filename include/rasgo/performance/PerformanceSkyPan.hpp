#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

#include "PerformanceContracts.hpp"

namespace rasgo::performance {

// O pan pela física (3/11/2026, aprovado pelo autor): com a TRAMA ligada,
// cada estrela soa onde a Constelação a mostra — a posição x da física vira
// o lugar do stem no estéreo. ESPAÇO abre ou fecha esse campo (0 = mono); nenhuma
// estrela vai além de 70 % para um lado. O grave de cada stem (abaixo de
// ~150 Hz, separado por um filtro de um polo; agudo = sinal − grave, então
// a soma é exata) fica no centro, para o grave não pender nem enfraquecer
// em mono. No centro, o ganho de cada canal é o mesmo da mistura de antes
// (0,94); a lei é de potência igual. O lugar de cada estrela é alisado
// (~30 ms): a física anda a cada 64 amostras, o ouvido não ouve degraus.
class PerformanceSkyPan {
public:
    static constexpr std::size_t kStems = kPerformanceStemActorCapacity;
    static constexpr double kMaxPan = 0.70;
    static constexpr double kCentreGain = 0.94;
    static constexpr double kCrossoverHz = 150.0;
    static constexpr double kSmoothingSeconds = 0.030;

    void prepare(double sampleRate) noexcept {
        const double rate = std::isfinite(sampleRate) && sampleRate > 0.0 ? sampleRate : 48000.0;
        lowCoefficient_ = 1.0 - std::exp(-2.0 * kPi * kCrossoverHz / rate);
        smoothCoefficient_ = 1.0 - std::exp(-1.0 / (kSmoothingSeconds * rate));
        reset();
    }

    void reset() noexcept {
        low_.fill(0.0);
        pan_.fill(0.0);
        primed_.fill(false);
    }

    // x: lugar mostrado na Constelação (0 = esquerda, 1 = direita).
    // width: 0..1, vindo de ESPAÇO; em 0 a saída é mono exato (o contrato
    // de antes: ESPAÇO 0 = centro), em 1 o campo abre até 70 %.
    static double targetPan(double x, double width) noexcept {
        if (!std::isfinite(x)) return 0.0;
        const double side = std::clamp((x - 0.5) * 2.0, -1.0, 1.0);
        const double open = std::clamp(std::isfinite(width) ? width : 0.0, 0.0, 1.0);
        return side * kMaxPan * open;
    }

    // Acrescenta um stem à mistura. slot identifica o stem (lugar no frame).
    void add(std::size_t slot, double sample, double target, double& left, double& right) noexcept {
        if (slot >= kStems || !std::isfinite(sample)) return;
        if (!primed_[slot]) {
            pan_[slot] = target;
            primed_[slot] = true;
        } else {
            pan_[slot] += (target - pan_[slot]) * smoothCoefficient_;
        }
        low_[slot] += (sample - low_[slot]) * lowCoefficient_;
        const double low = low_[slot];
        const double high = sample - low;
        // Potência igual, normalizada para que o centro dê kCentreGain.
        // No centro exato, o mesmo ganho nos dois canais (mono bit a bit).
        double leftGain = kCentreGain;
        double rightGain = kCentreGain;
        if (pan_[slot] != 0.0) {
            const double angle = (pan_[slot] + 1.0) * 0.25 * kPi;
            const double scale = kCentreGain * std::sqrt(2.0);
            leftGain = std::cos(angle) * scale;
            rightGain = std::sin(angle) * scale;
        }
        left += low * kCentreGain + high * leftGain;
        right += low * kCentreGain + high * rightGain;
    }

    // Um stem que saiu do céu recomeça do lugar novo, sem deslizar.
    void release(std::size_t slot) noexcept {
        if (slot >= kStems) return;
        primed_[slot] = false;
        low_[slot] = 0.0;
    }

    double pan(std::size_t slot) const noexcept { return slot < kStems ? pan_[slot] : 0.0; }

private:
    static constexpr double kPi = 3.14159265358979323846;
    std::array<double, kStems> low_{};
    std::array<double, kStems> pan_{};
    std::array<bool, kStems> primed_{};
    double lowCoefficient_ = 0.0;
    double smoothCoefficient_ = 0.0;
};

} // namespace rasgo::performance
