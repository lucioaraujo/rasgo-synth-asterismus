#include <cmath>
#include <cstdlib>
#include <iostream>

#include <rasgo/performance/PerformanceAudioMeasurement.hpp>

int main() {
    rasgo::performance::PerformanceAudioMeasurementAccumulator accumulator;
    const float left[]{0.0F, 0.5F, -0.5F, 0.0F};
    const float right[]{0.0F, -0.25F, 0.25F, 0.0F};
    accumulator.push(left, right, 4U);
    const auto result = accumulator.result();
    if (result.samplesPerChannel != 4U || std::fabs(result.samplePeak - 0.5) > 1.0e-6
        || result.estimatedInterSamplePeak < result.samplePeak
        || std::fabs(result.integratedRms - std::sqrt(0.078125)) > 1.0e-6
        || result.dbfs(result.samplePeak) > -6.01 || result.dbfs(result.samplePeak) < -6.03) {
        std::cerr << "measurement accumulator failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "rasgo_synth_performance_audio_measurement_tests: OK\n";
    return EXIT_SUCCESS;
}
