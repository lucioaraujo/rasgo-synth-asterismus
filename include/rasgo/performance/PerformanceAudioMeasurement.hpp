#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace rasgo::performance {

// Offline-only measurement for a completed WAV. It intentionally reports an
// inter-sample estimate and RMS dBFS, never certified true-peak or LUFS.
struct PerformanceAudioMeasurement {
    double samplePeak = 0.0;
    double estimatedInterSamplePeak = 0.0;
    double integratedRms = 0.0;
    std::size_t samplesPerChannel = 0U;

    double dbfs(double value) const noexcept {
        return value > 0.0 ? 20.0 * std::log10(value) : -120.0;
    }
};

class PerformanceAudioMeasurementAccumulator final {
public:
    void push(const float* left, const float* right, std::size_t count) noexcept {
        if (left == nullptr || count == 0U) return;
        for (std::size_t index = 0U; index < count; ++index) {
            const auto currentLeft = sane(left[index]);
            const auto currentRight = sane(right != nullptr ? right[index] : left[index]);
            samplePeak_ = std::max({samplePeak_, std::fabs(currentLeft), std::fabs(currentRight)});
            if (hasPrevious_) {
                for (int phase = 1; phase < 4; ++phase) {
                    const auto fraction = static_cast<double>(phase) * 0.25;
                    estimatedPeak_ = std::max(estimatedPeak_, std::fabs(previousLeft_
                        + (currentLeft - previousLeft_) * fraction));
                    estimatedPeak_ = std::max(estimatedPeak_, std::fabs(previousRight_
                        + (currentRight - previousRight_) * fraction));
                }
            }
            estimatedPeak_ = std::max({estimatedPeak_, std::fabs(currentLeft), std::fabs(currentRight)});
            sumSquares_ += currentLeft * currentLeft + currentRight * currentRight;
            previousLeft_ = currentLeft;
            previousRight_ = currentRight;
            hasPrevious_ = true;
            ++samplesPerChannel_;
        }
    }

    PerformanceAudioMeasurement result() const noexcept {
        PerformanceAudioMeasurement out;
        out.samplePeak = samplePeak_;
        out.estimatedInterSamplePeak = estimatedPeak_;
        out.samplesPerChannel = samplesPerChannel_;
        out.integratedRms = samplesPerChannel_ > 0U
            ? std::sqrt(sumSquares_ / static_cast<double>(samplesPerChannel_ * 2U)) : 0.0;
        return out;
    }

private:
    static double sane(float value) noexcept { return std::isfinite(value) ? static_cast<double>(value) : 0.0; }
    double samplePeak_ = 0.0;
    double estimatedPeak_ = 0.0;
    double sumSquares_ = 0.0;
    double previousLeft_ = 0.0;
    double previousRight_ = 0.0;
    std::size_t samplesPerChannel_ = 0U;
    bool hasPrevious_ = false;
};

} // namespace rasgo::performance
