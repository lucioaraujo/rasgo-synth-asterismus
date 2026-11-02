#include <cstdlib>
#include <iostream>

#include <rasgo/performance/PerformanceLayout.hpp>

namespace {

using namespace rasgo::performance;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

int runLayoutTests() {
    constexpr auto layout = performanceGraphLayout();
    expect(performanceGraphLowerStripIsDisjoint(), "graph lower strip contract is disjoint");
    expect(layout.interactionBottomInset == 94, "interaction area keeps the reserved strip");

    for (const auto panelHeight : {180, 240, 360, 720, 1080}) {
        const auto rackTop = panelHeight - layout.interactionBottomInset - 28;
        const auto rackBottom = panelHeight - layout.interactionBottomInset;
        const auto libraryTop = panelHeight - layout.libraryTopInset;
        const auto libraryBottom = libraryTop + layout.libraryHeight;
        const auto cueTop = panelHeight - layout.cueTopInset;
        const auto cueBottom = cueTop + layout.cueHeight;
        const auto logTop = panelHeight - layout.logTopInset;
        expect(rackTop >= 0, "rack remains inside supported graph heights");
        expect(rackBottom <= libraryTop, "fourth rack card stays above library");
        expect(libraryBottom <= cueTop, "library stays above cue");
        expect(cueBottom <= logTop, "cue stays above log");
    }

    for (const auto width : {1280, 1366, 1440, 1920, 2560})
        expect(performanceHeaderTargetsAreDisjoint(width), "header targets remain disjoint");
    constexpr auto header = performanceHeaderLayout(1280);
    expect(header.seedLabel.y == header.seedEditor.y
               && header.alea.y == header.tempo.y
               && header.tempo.y == header.master.y,
           "first header row shares one vertical alignment");
    expect(header.alea.x - (header.seedEditor.x + header.seedEditor.width) == 6,
           "ALEA stays close to the seed number box");
    expect(header.tempo.width == 210 && header.master.width == 210,
           "tempo and master sliders keep the same header geometry");
    expect(header.tutorial.width == 86 && header.about.width == 86
               && header.tutorial.y == header.master.y && header.about.y == header.master.y,
           "tutorial and about occupy their own first-row system targets");
    expect(header.language.width == 38 && header.language.y == header.master.y,
           "language icon has a compact first-row system target");
    expect(header.record.y >= 56 && header.transport.y >= 56,
           "capture controls remain below the first header row");
    expect(header.record.y < header.recordingBox.y + header.recordingBox.height
               && header.record.y + header.record.height <= header.recordingBox.y + header.recordingBox.height,
           "record control remains inside capture box");
    expect(header.transport.y < header.recordingBox.y + header.recordingBox.height
               && header.transport.y + header.transport.height <= header.recordingBox.y + header.recordingBox.height,
           "transport control remains inside capture box");
    expect(header.recordingBox.right() == header.language.right()
               && header.record.x - header.recordingBox.x == 8
               && header.recordingBox.right() - header.transport.right() == 8,
           "recording box and its controls share the header right margin");
    expect(header.recordingBox.height == 50 && header.record.y - header.recordingBox.y == 18,
           "recording box remains compact with controls close to its title");
    expect(performanceSceneUtilityReserve() == 584, "scene utility reserve remains explicit");
    return 0;
}

} // namespace

int main() { return runLayoutTests(); }
