#include <cstdlib>
#include <iostream>

#include <rasgo/performance/StudioTerritoryImporter.hpp>

namespace {
void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
}

int main() {
    using namespace rasgo::performance;
    constexpr const char* studio =
        "RSTM 1\nterritory 181 123\nactors 3\n"
        "actor 0 1 88 0.78 0.62 1\n"
        "actor 1 2 13 0.46 0.48 0.72\n"
        "actor 2 0 51 0.64 0.50 0.68\n";
    PerformanceSessionDocument document;
    const auto result = StudioTerritoryImporter::import(studio, document);
    expect(result.valid() && result.actorCount == 3U, "Studio territory imports its manifested actors");
    expect(document.valid() && document.territory.seed == 181U && document.sceneCount == 1U,
           "import produces a bounded Performance session");
    expect(document.scenes[0].session.tempoBpm == 123.0
           && document.scenes[0].constellation.actorCount == 3U
           && document.scenes[0].constellation.relationCount == 2U,
           "Studio tempo and actor relations enter the first playable scene");
    expect(document.territory.actors[1].family == PerformanceFamily::Texture
           && roleAllowed(document.territory.actors[1].allowedRoles, PerformanceRole::Atrito),
           "Studio texture retains an intelligible Performance role");
    constexpr const char* studioV2 =
        "RSTM 2\nterritory 182 128\n"
        "game 3 5 0.71 0.32 0.88 0.65 0.41\nactors 2\n"
        "actor 0 1 88 0.78 0.62 1\n"
        "actor 2 0 51 0.64 0.50 0.68\n";
    const auto enriched = StudioTerritoryImporter::import(studioV2, document);
    expect(enriched.valid() && enriched.version == 2U && enriched.hasGamePlan,
           "RSTM 2 parses as an enriched Studio territory");
    expect(document.territory.gamePlan.strategy == PerformanceGameStrategy::Interrupt
           && document.territory.gamePlan.rule == PerformanceGameRule::Break,
           "RSTM 2 imports the Studio strategy and rule");
    expect(document.territory.gamePlan.energy == 0.71
           && document.territory.gamePlan.hiddenInformation == 0.41,
           "RSTM 2 imports the Studio game resources");
    const auto bad = StudioTerritoryImporter::import("RSTM 3\n", document);
    expect(bad.error == StudioTerritoryImportError::Version, "unknown manifest version is rejected");
    std::cout << "StudioTerritoryImporter tests passed\n";
}
