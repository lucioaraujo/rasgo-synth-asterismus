#include <cstdlib>
#include <iostream>

#include <rasgo/performance/PerformanceStemDistribution.hpp>

namespace {
using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testProductionDistributionOnlyOpensReleasedStems() {
    // Since 2026-10-28 every body of the asterism owns an audible stem: the
    // four-stem gate came from an audio thread without realtime priority.
    const auto distribution = PerformanceStemDistributor::make(181U, 8U);
    expect(distribution.requestedStemCount == 8U && distribution.audibleStemCount == 8U,
           "production distribution opens one stem per body");
    for (std::uint16_t actorId = 1U; actorId <= 8U; ++actorId) {
        const auto* assignment = distribution.assignmentFor(actorId);
        expect(assignment != nullptr && !assignment->primaryModuleId().empty()
                   && assignment->disposition == PerformanceStemDisposition::Audible,
               "each body receives one seed-selected audible module");
    }
}

void testDistributionIsSeedStableAndCohortAware() {
    const auto circuit = PerformanceStemDistributor::make(181U, 4U);
    const auto repeat = PerformanceStemDistributor::make(181U, 4U);
    expect(circuit.assignmentFor(1U)->primaryModuleId() == repeat.assignmentFor(1U)->primaryModuleId()
               && !circuit.assignmentFor(1U)->primaryModuleId().empty(),
           "the seed recreates a concrete opening orchestration");
    expect(circuit.assignmentFor(4U)->primaryModuleId() == repeat.assignmentFor(4U)->primaryModuleId(),
           "the same seed recreates its complementary fourth module");
}

void testEachStemHoldsOneDistinctStar() {
    for (const auto seed : {181U, 182U, 183U, 184U, 185U, 3898605111U}) {
        const auto distribution = PerformanceStemDistributor::make(seed, 8U);
        for (std::uint16_t actorId = 1U; actorId <= 8U; ++actorId) {
            const auto* assignment = distribution.assignmentFor(actorId);
            expect(assignment != nullptr && assignment->moduleCount == 1U,
                   "a stem is one module, not a group of alternatives");
            for (std::uint16_t other = 1U; other < actorId; ++other)
                expect(distribution.assignmentFor(other)->primaryModuleId()
                           != assignment->primaryModuleId(),
                       "a star never lights twice in one asterism");
        }
    }
}

} // namespace

int main() {
    testProductionDistributionOnlyOpensReleasedStems();
    testDistributionIsSeedStableAndCohortAware();
    testEachStemHoldsOneDistinctStar();
    std::cout << "Performance stem distribution tests passed\n";
}
