#include <cstdlib>
#include <iostream>
#include <limits>

#include <rasgo/performance/PerformanceCumulativeDesk.hpp>

namespace {
using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}

int mainImpl() {
    PerformanceCumulativeDesk desk;
    expect(desk.add(11U, 0U, {0.20, 0.30}, 0.70, true), "first module enters desk");
    expect(desk.add(22U, 1U, {0.50, 0.30}, 0.60, true), "second module enters desk");
    expect(desk.add(33U, 2U, {0.80, 0.60}, 0.50, true), "third module enters desk");
    const auto before = *desk.slot(0U);
    const auto beforeSecond = *desk.slot(1U);
    const auto beforeThird = *desk.slot(2U);
    expect(desk.add(44U, 8U, {0.50, 0.78}, 0.68, true), "new module enters without replacing the trio");
    expect(desk.occupiedCount() == 4U && desk.audibleCount() == 4U,
           "four cumulative stems remain represented and audible");
    expect(desk.slot(0U)->actorId == before.actorId
               && desk.slot(0U)->position.x == before.position.x
               && desk.slot(0U)->position.y == before.position.y
               && desk.slot(0U)->libraryEntry == before.libraryEntry
               && desk.slot(0U)->presence == before.presence
               && desk.slot(0U)->audible == before.audible
               && desk.slot(1U)->actorId == beforeSecond.actorId
               && desk.slot(1U)->position.x == beforeSecond.position.x
               && desk.slot(2U)->actorId == beforeThird.actorId
               && desk.slot(2U)->position.y == beforeThird.position.y,
           "existing module identity and field survive insertion");
    expect(desk.setPosition(44U, {0.91, 0.12}) && desk.slot(0U)->position.x == before.position.x,
           "moving the new module does not move existing modules");
    expect(!desk.add(11U, 9U, {0.1, 0.1}, 0.5, true), "duplicate module is rejected");
    for (std::uint16_t id = 55U; id < 59U; ++id)
        expect(desk.add(id, id, {0.5, 0.5}, 0.4, false), "desk accepts structural slots up to capacity");
    expect(desk.occupiedCount() == PerformanceCumulativeDesk::kCapacity,
           "desk has explicit bounded structural capacity");
    expect(!desk.add(99U, 99U, {0.5, 0.5}, 0.4, false), "desk rejects overflow safely");
    expect(desk.remove(22U) && desk.occupiedCount() == PerformanceCumulativeDesk::kCapacity - 1U,
           "module can be recollected without clearing the desk");
    expect(desk.slot(0U)->actorId == before.actorId
               && desk.slot(0U)->position.x == before.position.x
               && desk.slot(2U)->actorId == beforeThird.actorId,
           "recollection leaves unrelated modules and their positions intact");
    return 0;
}
}

int main() { return mainImpl(); }
