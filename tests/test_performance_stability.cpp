#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <new>
#include <string_view>

#if defined(RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT) && defined(__linux__)
#include <sys/resource.h>
#endif

#include <rasgo/performance/PerformanceSessionEngine.hpp>
#include <rasgo/performance/PerformanceTerritoryFactory.hpp>

namespace {
std::atomic<std::size_t> allocations{0U};
}

void* operator new(std::size_t size) {
    allocations.fetch_add(1U, std::memory_order_relaxed);
    if (void* result = std::malloc(size)) return result;
    throw std::bad_alloc{};
}

void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { std::free(pointer); }
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete[](void* pointer) noexcept { ::operator delete(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { ::operator delete(pointer); }

namespace {
using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

#if defined(RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT)
std::uint64_t maximumResidentSetKiB() noexcept {
#if defined(__linux__)
    rusage usage{};
    return getrusage(RUSAGE_SELF, &usage) == 0
        ? static_cast<std::uint64_t>(usage.ru_maxrss) : 0U;
#else
    return 0U;
#endif
}
#endif

PerformanceTerritory territory() {
    PerformanceTerritory out;
    out.seed = 181U;
    for (std::uint16_t id = 11U; id <= 13U; ++id) {
        PerformanceActorPotential actor;
        actor.id = id;
        actor.capabilities = 1U;
        expect(out.addActor(actor), "actor enters stress territory");
    }
    return out;
}

PerformanceScene scene() {
    PerformanceScene out;
    out.id = 31U;
    out.session.territorySeed = 181U;
    out.session.playing = true;
    out.session.currentSceneId = out.id;
    out.session.density = 0.62;
    out.session.tension = 0.57;
    expect(out.constellation.addActor({11U, PerformanceRole::Chao, ActorLifeState::Active,
                                       0.78, 0.72, 0.60, 0.80}), "ground actor enters stress scene");
    expect(out.constellation.addActor({12U, PerformanceRole::Corpo, ActorLifeState::Active,
                                       0.64, 0.66, 0.55, 0.72}), "body actor enters stress scene");
    expect(out.constellation.addActor({13U, PerformanceRole::Atrito, ActorLifeState::Emerging,
                                       0.42, 0.48, 0.32, 0.68}), "friction actor enters stress scene");
    expect(out.constellation.addRelation({11U, 12U, RelationKind::Support, RelationDomain::Time,
                                          0.42, 10.0, 1.0, 0.7}), "time relation enters stress scene");
    expect(out.constellation.addRelation({12U, 13U, RelationKind::Friction, RelationDomain::Matter,
                                          0.55, 9.0, 0.8, 0.7}), "matter relation enters stress scene");
    return out;
}

void runTwentyMinuteSimulation() {
    constexpr std::uint32_t sampleRate = 48000U;
    constexpr std::uint32_t blockSize = 256U;
    constexpr std::uint64_t simulatedSamples = 20ULL * 60ULL * sampleRate;
    constexpr std::uint64_t blockCount = simulatedSamples / blockSize;

    const auto performanceTerritory = territory();
    const auto performanceScene = scene();
    expect(performanceScene.valid(performanceTerritory), "stress scene is valid");

    PerformanceSessionEngine engine;
    expect(engine.prepare(static_cast<double>(sampleRate), blockSize, 2U), "stress engine prepares");
    expect(engine.setScene(performanceScene, performanceTerritory), "stress scene enters engine");
    std::array<float, blockSize> left{};
    std::array<float, blockSize> right{};

    const std::size_t allocationsBefore = allocations.load(std::memory_order_relaxed);
    const auto started = std::chrono::steady_clock::now();
    double largestSample = 0.0;
    for (std::uint64_t block = 0U; block < blockCount; ++block) {
        engine.processBlock(left.data(), right.data(), blockSize);
        largestSample = std::max(largestSample, std::fabs(static_cast<double>(left[block % blockSize])));
    }
    const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    const std::size_t allocationsAfter = allocations.load(std::memory_order_relaxed);
    const auto telemetry = engine.readTelemetry();

    expect(allocationsAfter == allocationsBefore, "processBlock performs no dynamic allocations in 20-minute simulation");
    expect(std::isfinite(largestSample) && largestSample > 1.0e-5, "20-minute output stays finite and audible");
    expect(telemetry.valid() && telemetry.xrunCount == 0U && telemetry.activeActorCount == 3U,
           "20-minute simulation keeps telemetry stable");
    expect(elapsed > 0.0, "throughput measurement is meaningful");
    std::cout << "20-minute simulated session in " << elapsed << " s ("
              << (1200.0 / elapsed) << "x realtime), allocations in processBlock: 0\n";
}

void runFourBodySimulation(std::uint32_t seed, std::uint64_t simulatedSeconds,
                           std::string_view label) {
    constexpr std::uint32_t sampleRate = 48000U;
    constexpr std::uint32_t blockSize = 256U;
    const std::uint64_t simulatedSamples = simulatedSeconds * sampleRate;
    const std::uint64_t blockCount = simulatedSamples / blockSize;
    const auto document = PerformanceTerritoryFactory::makeWithWalsh(seed, true);
    expect(document.valid(), "four-body stress document is valid");
    PerformanceSessionEngine engine;
    expect(engine.prepare(static_cast<double>(sampleRate), blockSize, 2U)
               && engine.setScene(document.scenes[0], document.territory)
               && engine.setActorPresence(4U, 0.82)
               && engine.setActorPosition(4U, 0.20, 0.46),
           "four-body stress engine prepares with the selected cumulative body");
    std::array<float, blockSize> left{};
    std::array<float, blockSize> right{};
    const std::size_t allocationsBefore = allocations.load(std::memory_order_relaxed);
    const auto started = std::chrono::steady_clock::now();
    double largestSample = 0.0;
    for (std::uint64_t block = 0U; block < blockCount; ++block) {
        engine.processBlock(left.data(), right.data(), blockSize);
        largestSample = std::max(largestSample, std::fabs(static_cast<double>(left[block % blockSize])));
    }
    const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    const std::size_t allocationsAfter = allocations.load(std::memory_order_relaxed);
    const auto telemetry = engine.readTelemetry();
    expect(allocationsAfter == allocationsBefore, "four-body processBlock performs no dynamic allocations");
    expect(std::isfinite(largestSample) && largestSample > 1.0e-5, "four-body output stays finite and audible");
    expect(telemetry.valid() && telemetry.xrunCount == 0U
               && telemetry.activeActorCount == 4U && telemetry.stemCount == 4U,
           "four-body simulation keeps telemetry and stem table stable");
    expect(elapsed > 0.0, "four-body throughput measurement is meaningful");
    std::cout << "four-body " << label << " simulated session in " << elapsed << " s ("
              << (static_cast<double>(simulatedSeconds) / elapsed)
              << "x realtime), allocations in processBlock: 0\n";
}

void runGaloOneMinuteSimulation() {
    constexpr std::uint32_t sampleRate = 48000U;
    constexpr std::uint32_t blockSize = 256U;
    constexpr std::uint64_t simulatedSamples = 60ULL * sampleRate;
    // A seed 41 põe o Galo no corpo 4 (desde 3/11 a seed escolhe entre todas
    // as estrelas; antes era a 200).
    const auto document = PerformanceTerritoryFactory::makeWithOptionalModule(41U, true);
    expect(document.valid(), "Galo stress document is valid");
    PerformanceSessionEngine engine;
    expect(engine.prepare(static_cast<double>(sampleRate), blockSize, 2U)
               && engine.setScene(document.scenes[0], document.territory)
               && engine.setActorPresence(4U, 0.78)
               && engine.setActorPosition(4U, 0.64, 0.72)
               && engine.setModuleControl(4U, PerformanceModuleControl::Primary, 0.84)
               && engine.setRelationIntensity(1U, 4U, 0.71),
           "Galo one-minute engine prepares with live controls");
    std::array<float, blockSize> left{};
    std::array<float, blockSize> right{};
    const auto allocationsBefore = allocations.load(std::memory_order_relaxed);
    double largestSample = 0.0;
    // O pico do stem 4 é o maior dos últimos ~5 s (uma estrela pode estar
    // numa pausa da frase no instante final; ler um bloco só era frágil).
    std::uint32_t galoPeak = 0U;
    for (std::uint64_t sample = 0U; sample < simulatedSamples; sample += blockSize) {
        engine.processBlock(left.data(), right.data(), blockSize);
        largestSample = std::max(largestSample,
            std::fabs(static_cast<double>(left[(sample / blockSize) % blockSize])));
        if (sample + 5ULL * sampleRate >= simulatedSamples)
            galoPeak = std::max(galoPeak, engine.readTelemetry().stemPeakMicros[3]);
    }
    const auto allocationsAfter = allocations.load(std::memory_order_relaxed);
    const auto telemetry = engine.readTelemetry();
    expect(allocationsAfter == allocationsBefore,
           "Galo one-minute processBlock performs no dynamic allocations");
    expect(std::isfinite(largestSample) && largestSample > 1.0e-5,
           "Galo one-minute output stays finite and audible");
    expect(telemetry.valid() && telemetry.xrunCount == 0U
               && telemetry.activeActorCount == 4U && telemetry.stemCount == 4U
               && telemetry.stemActorIds[3] == 4U && galoPeak > 100U,
           "Galo one-minute stem telemetry remains audible and stable");
    std::cout << "Galo one-minute simulated session: stable, allocations in processBlock: 0\n";
}

void runExpandedEightModuleFourStemSimulation() {
    constexpr std::uint32_t sampleRate = 48000U;
    constexpr std::uint32_t blockSize = 256U;
    constexpr std::uint64_t blockCount = (60ULL * sampleRate) / blockSize;
    const auto document = PerformanceTerritoryFactory::makeForReleasedStems(181U, 4U, true);
    expect(document.valid() && document.territory.actorCount == 4U,
           "eight-module four-stem ensemble document is valid");
    PerformanceSessionEngine engine;
    expect(engine.prepare(static_cast<double>(sampleRate), blockSize, 2U)
               && engine.setScene(document.scenes[0], document.territory)
               && engine.setActorMotionField(3U, PerformanceMotionField::Pulse)
               && engine.setActorMotionField(4U, PerformanceMotionField::FigureEight)
               && engine.setActorMotionDirection(4U, false),
           "eight-module four-stem ensemble accepts motion and prepares");
    std::array<float, blockSize> left{};
    std::array<float, blockSize> right{};
    const auto allocationsBefore = allocations.load(std::memory_order_relaxed);
    double largestSample = 0.0;
    for (std::uint64_t block = 0U; block < blockCount; ++block) {
        engine.processBlock(left.data(), right.data(), blockSize);
        largestSample = std::max(largestSample, std::fabs(static_cast<double>(left[block % blockSize])));
    }
    const auto allocationsAfter = allocations.load(std::memory_order_relaxed);
    const auto telemetry = engine.readTelemetry();
    expect(allocationsAfter == allocationsBefore,
           "eight-module four-stem processBlock performs no dynamic allocations");
    expect(std::isfinite(largestSample) && largestSample > 1.0e-5,
           "eight-module four-stem output stays finite and audible");
    expect(telemetry.valid() && telemetry.xrunCount == 0U
               && telemetry.activeActorCount == 4U && telemetry.stemCount == 4U
               && telemetry.stemActorIds[3] == 4U,
           "eight-module four-stem telemetry remains complete and stable");
    std::cout << "eight-module/four-stem 60-second simulated session: stable, allocations in processBlock: 0\n";
}

#if defined(RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT)
void runModalFiveBodySimulation(std::uint64_t simulatedSeconds, const char* label) {
    constexpr std::uint32_t sampleRate = 48000U;
    constexpr std::uint32_t blockSize = 256U;
    const std::uint64_t simulatedSamples = simulatedSeconds * sampleRate;
    const std::uint64_t blockCount = simulatedSamples / blockSize;
    const auto document = PerformanceTerritoryFactory::makeWithModalReserve(184U, true);
    expect(document.valid() && document.territory.actorCount == 5U,
           "five-body ModalResonator stress document is valid");
    PerformanceSessionEngine engine;
    expect(engine.prepare(static_cast<double>(sampleRate), blockSize, 2U)
               && engine.setScene(document.scenes[0], document.territory)
               && engine.setActorPresence(4U, 0.78)
               && engine.setActorPresence(5U, 0.72)
               && engine.setActorPosition(4U, 0.20, 0.46)
               && engine.setActorPosition(5U, 0.64, 0.72)
               && engine.setModuleControl(5U, PerformanceModuleControl::Primary, 0.84)
               && engine.setModuleControl(5U, PerformanceModuleControl::Secondary, 0.66)
               && engine.setRelationIntensity(1U, 5U, 0.78),
           "five-body ModalResonator stress engine prepares with modal controls and memory relation");
    std::array<float, blockSize> left{};
    std::array<float, blockSize> right{};
    const std::size_t allocationsBefore = allocations.load(std::memory_order_relaxed);
    const auto started = std::chrono::steady_clock::now();
    double largestSample = 0.0;
    double maximumBlockSeconds = 0.0;
    bool exercisedGain = false;
    bool exercisedMute = false;
    bool restoredMute = false;
    bool exercisedSolo = false;
    bool releasedSolo = false;
    for (std::uint64_t block = 0U; block < blockCount; ++block) {
        if (block == blockCount / 4U) exercisedGain = engine.setStemGain(5U, 0.58);
        if (block == blockCount / 2U) exercisedMute = engine.setStemMuted(5U, true);
        if (block == blockCount / 2U + 128U) restoredMute = engine.setStemMuted(5U, false);
        if (block == (blockCount * 3U) / 4U) exercisedSolo = engine.setStemSolo(5U, true);
        if (block == (blockCount * 3U) / 4U + 128U) releasedSolo = engine.setStemSolo(5U, false);
        const auto blockStarted = std::chrono::steady_clock::now();
        engine.processBlock(left.data(), right.data(), blockSize);
        maximumBlockSeconds = std::max(maximumBlockSeconds,
            std::chrono::duration<double>(std::chrono::steady_clock::now() - blockStarted).count());
        largestSample = std::max(largestSample, std::fabs(static_cast<double>(left[block % blockSize])));
    }
    const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    const std::size_t allocationsAfter = allocations.load(std::memory_order_relaxed);
    const auto telemetry = engine.readTelemetry();
    expect(allocationsAfter == allocationsBefore,
           "five-body ModalResonator processBlock performs no dynamic allocations");
    expect(std::isfinite(largestSample) && largestSample > 1.0e-5,
           "five-body ModalResonator output stays finite and audible");
    if (!(telemetry.valid() && telemetry.xrunCount == 0U
          && telemetry.activeActorCount == 5U && telemetry.stemCount == 5U
          && telemetry.stemActorIds[4] == 5U && telemetry.stemPeakMicros[4] > 100U)) {
        std::cerr << "modal-five telemetry: valid=" << telemetry.valid()
                  << " xruns=" << telemetry.xrunCount
                  << " active=" << telemetry.activeActorCount
                  << " stems=" << telemetry.stemCount
                  << " fifth-id=" << telemetry.stemActorIds[4]
                  << " fifth-peak=" << telemetry.stemPeakMicros[4] << '\n';
    }
    expect(telemetry.valid() && telemetry.xrunCount == 0U
               && telemetry.activeActorCount == 5U && telemetry.stemCount == 5U
               && telemetry.stemActorIds[4] == 5U && telemetry.stemPeakMicros[4] > 100U,
           "five-body ModalResonator simulation keeps fifth stem audible and telemetry stable");
    expect(exercisedGain && exercisedMute && restoredMute && exercisedSolo && releasedSolo
               && engine.stemGain(5U) == 0.58 && !engine.stemMuted(5U)
               && engine.soloActorId() == kInvalidActorId,
           "five-body benchmark exercises gain, mute and solo without leaving the mixer altered");
    expect(elapsed > 0.0, "five-body ModalResonator throughput measurement is meaningful");
    std::cout << "five-body ModalResonator " << label << " simulated session in " << elapsed << " s ("
              << (static_cast<double>(simulatedSeconds) / elapsed)
              << "x realtime), peak offline block: " << (maximumBlockSeconds * 1000.0)
              << " ms, RSS max: " << maximumResidentSetKiB()
              << " KiB, allocations in processBlock: 0\n";
}
#endif
} // namespace

int main(int argc, char** argv) {
    if (argc > 1 && std::string_view(argv[1]) == "four") {
        runFourBodySimulation(181U, 20ULL * 60ULL, "20-minute");
        return 0;
    }
    if (argc > 1 && std::string_view(argv[1]) == "four-smoke") {
        runFourBodySimulation(181U, 60ULL, "60-second smoke");
        return 0;
    }
    if (argc > 1 && std::string_view(argv[1]) == "hard-sync") {
        runFourBodySimulation(186U, 20ULL * 60ULL, "20-minute");
        return 0;
    }
    if (argc > 1 && std::string_view(argv[1]) == "digital") {
        runFourBodySimulation(184U, 20ULL * 60ULL, "20-minute");
        return 0;
    }
    if (argc > 1 && std::string_view(argv[1]) == "galo") {
        runGaloOneMinuteSimulation();
        return 0;
    }
    if (argc > 1 && std::string_view(argv[1]) == "expanded-eight") {
        runExpandedEightModuleFourStemSimulation();
        return 0;
    }
#if defined(RASGO_PERFORMANCE_TEST_SAFE_BODY_LIMIT)
    if (argc > 1 && std::string_view(argv[1]) == "modal-five") {
        runModalFiveBodySimulation(20ULL * 60ULL, "20-minute");
        return 0;
    }
    if (argc > 1 && std::string_view(argv[1]) == "modal-five-smoke") {
        runModalFiveBodySimulation(60ULL, "60-second smoke");
        return 0;
    }
#endif
    runTwentyMinuteSimulation();
    return 0;
}
