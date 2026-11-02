#include <cmath>
#include <cstdlib>
#include <iostream>

#include <rasgo/performance/PerformanceMorceauLifecycle.hpp>

namespace {
using rasgo::performance::PerformanceMorceauLifecycle;
using rasgo::performance::PerformanceMorceauPlan;
using rasgo::performance::PerformanceMorceauState;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testLifecycleIsBoundedAndMusical() {
    PerformanceMorceauLifecycle lifecycle;
    const PerformanceMorceauPlan plan{0.02, 0.25, 0.01};
    expect(lifecycle.prepare(8000.0, 717U, plan), "lifecycle accepts a valid bounded plan");
    expect(lifecycle.begin(), "prepared lifecycle starts once");
    expect(!lifecycle.begin(), "opening cannot restart silently");

    double previous = -1.0;
    for (int sample = 0; sample < 160; ++sample) {
        const auto gain = lifecycle.nextGain();
        expect(std::isfinite(gain) && gain >= 0.0 && gain <= 1.0,
               "opening envelope stays finite and unit bounded");
        expect(gain + 1.0e-12 >= previous, "opening never clicks upward then downward");
        previous = gain;
    }
    expect(lifecycle.state() == PerformanceMorceauState::Developing,
           "opening becomes development at its defined edge");
    expect(std::fabs(lifecycle.nextGain() - 1.0) < 1.0e-12,
           "development keeps the musical body at unity");

    expect(lifecycle.beginCoda(), "development accepts one intentional coda");
    previous = 1.0;
    for (int sample = 0; sample < 2000; ++sample) {
        const auto gain = lifecycle.nextGain();
        expect(std::isfinite(gain) && gain >= 0.0 && gain <= 1.0,
               "coda envelope stays finite and unit bounded");
        expect(gain <= previous + 1.0e-12, "coda never rises into a click");
        previous = gain;
    }
    expect(lifecycle.state() == PerformanceMorceauState::Tail,
           "coda enters a recorder-safe tail");
    expect(lifecycle.recordingShouldRemainOpen(), "tail keeps recording alive");
    for (int sample = 0; sample < 80; ++sample)
        expect(lifecycle.nextGain() == 0.0, "tail introduces no new audio energy");
    expect(lifecycle.state() == PerformanceMorceauState::Finalized,
           "tail finalizes deterministically");
    expect(!lifecycle.recordingShouldRemainOpen() && lifecycle.finished(),
           "finalized lifecycle releases the recorder exactly once");
}

void testInterruptIsAnExplicitEmergencyBoundary() {
    PerformanceMorceauLifecycle lifecycle;
    expect(lifecycle.prepare(48000.0, 1U), "default plan prepares");
    expect(lifecycle.begin(), "default plan starts");
    lifecycle.interrupt();
    expect(lifecycle.state() == PerformanceMorceauState::Interrupted,
           "interrupt is visible and never disguised as a coda");
    expect(lifecycle.nextGain() == 0.0 && lifecycle.finished(),
           "interruption emits no residual lifecycle gain");
}

void testInvalidPlansFailClosed() {
    PerformanceMorceauLifecycle lifecycle;
    expect(!lifecycle.prepare(48000.0, 1U, PerformanceMorceauPlan{0.0, 4.0, 1.0}),
           "zero opening is rejected rather than causing an edge click");
    expect(!lifecycle.prepare(48000.0, 1U, PerformanceMorceauPlan{0.2, 0.1, 1.0}),
           "too-short coda is rejected rather than masquerading as a musical ending");
}

} // namespace

int main() {
    testLifecycleIsBoundedAndMusical();
    testInterruptIsAnExplicitEmergencyBoundary();
    testInvalidPlansFailClosed();
    std::cout << "Performance morceau lifecycle tests passed\n";
    return 0;
}
