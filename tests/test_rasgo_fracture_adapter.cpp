#include <cstdlib>
#include <iostream>

#include <rasgo/performance/RasgoFractureAdapter.hpp>

namespace {
using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

int mainImpl() {
    RasgoFractureAdapter first;
    RasgoFractureAdapter second;
    expect(first.prepare(48000.0, 512U, 2U), "fracture adapter prepares");
    expect(second.prepare(48000.0, 512U, 2U), "second fracture adapter prepares");
    RasgoFractureParameters parameters;
    parameters.seed = 20260830U;
    parameters.pressure = 0.8;
    expect(first.setParameters(parameters), "valid fracture parameters accepted");
    expect(second.setParameters(parameters), "same parameters accepted");
    first.applyStress(1.0);
    second.applyStress(1.0);
    for (int i = 0; i < 128; ++i) {
        expect(first.process() == second.process(), "same seed is deterministic");
    }
    expect(first.scar() > 0.0, "stress creates a scar");
    const auto scarBeforeRepair = first.scar();
    first.setRepair(1.0);
    for (int i = 0; i < 4000; ++i) first.process();
    expect(first.scar() < scarBeforeRepair, "repair reduces presented scar");
    expect(first.totalBreaks() >= 0 && first.totalBreaks() <= 5,
           "fracture count remains bounded");
    expect(first.descriptor().kContractVersion == 5U, "v5 descriptor is published");
    first.reset();
    expect(first.scar() == 0.0 && first.totalBreaks() == 0,
           "reset clears the session boundary");
    return 0;
}
}

int main() { return mainImpl(); }
