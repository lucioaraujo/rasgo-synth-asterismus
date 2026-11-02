#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

#include <rasgo/performance/ModalMatterAdapter.hpp>

namespace {
using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testModalMatterBoundary() {
    NativeModalMatterDouble adapter;
    expect(!adapter.prepare(48000.0, 0U, 2U), "invalid block size is rejected");
    expect(adapter.prepare(48000.0, 256U, 2U), "double prepares independently");

    ModalMatterParameters parameters;
    parameters.fundamentalHz = 73.4;
    parameters.brightness = 0.78;
    parameters.relationPressure = 0.40;
    parameters.role = PerformanceRole::Chao;
    expect(adapter.setParameters(parameters), "bounded modal parameters enter adapter");
    parameters.fundamentalHz = 9000.0;
    expect(!adapter.setParameters(parameters), "unsafe fundamental is rejected");

    adapter.excite(0.9);
    double peak = 0.0;
    for (int sample = 0; sample < 4096; ++sample) {
        const float output = adapter.process();
        expect(std::isfinite(output), "modal double stays finite");
        peak = std::max(peak, std::fabs(static_cast<double>(output)));
    }
    expect(peak > 1.0e-5 && peak <= 0.90, "excited modal double is audible and bounded");

    const auto descriptor = adapter.descriptor();
    expect(descriptor.kContractVersion == 1U && descriptor.latencySamples == 0U,
           "adapter declares its stable contract and latency");
    expect(std::string(descriptor.auditedCoreRevision) == "ae33cba5a1104bccf9427b9d7cf229250cc4dcd5",
           "adapter pins audited core provenance without including Studio code");
}

#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
void testAuditedCoreRealisation() {
    static_assert(RASGO_SYNTH_MODAL_MATTER_VERSION == 10000,
                  "Performance requires modal-matter package v1.0.0");
    CoreModalMatterAdapter adapter;
    expect(adapter.prepare(48000.0, 256U, 2U), "audited core adapter prepares");
    ModalMatterParameters parameters;
    parameters.fundamentalHz = 55.0;
    parameters.brightness = 0.68;
    parameters.relationPressure = 0.61;
    parameters.role = PerformanceRole::Vestigio;
    expect(adapter.setParameters(parameters), "audited core accepts bounded performance parameters");
    NativeModalMatterDouble contractDouble;
    expect(contractDouble.prepare(48000.0, 256U, 2U)
           && contractDouble.setParameters(parameters),
           "contract double prepares for technical A/B against the audited body");
    adapter.excite(0.82);
    contractDouble.excite(0.82);
    double peak = 0.0;
    double energy = 0.0;
    double differenceEnergy = 0.0;
    for (int sample = 0; sample < 24000; ++sample) {
        const auto output = static_cast<double>(adapter.process());
        const auto doubleOutput = static_cast<double>(contractDouble.process());
        expect(std::isfinite(output), "audited core output stays finite");
        peak = std::max(peak, std::fabs(output));
        energy += output * output;
        const auto difference = output - doubleOutput;
        differenceEnergy += difference * difference;
    }
    expect(peak > 1.0e-5 && peak <= 0.90 && energy > 1.0e-6,
           "audited ModalResonator realisation rings audibly and remains bounded");
    expect(differenceEnergy > 1.0e-5,
           "technical A/B proves the real core body is not the provisional double");
    const auto descriptor = adapter.descriptor();
    expect(std::string(descriptor.implementationId) == "rasgo-core-modal-resonator-v1",
           "descriptor distinguishes real core body from the contract double");
}
#endif
} // namespace

int main() {
    testModalMatterBoundary();
#if defined(RASGO_PERFORMANCE_WITH_MODAL_CORE)
    testAuditedCoreRealisation();
#endif
    std::cout << "rasgo_synth_performance_modal_adapter_tests: OK\n";
    return 0;
}
