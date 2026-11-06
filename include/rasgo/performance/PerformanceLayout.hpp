#pragma once

#include <array>

namespace rasgo::performance {

struct PerformanceLayoutRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    constexpr int right() const noexcept { return x + width; }
    constexpr bool intersects(const PerformanceLayoutRect& other) const noexcept {
        return x < other.right() && other.x < right()
            && y < other.y + other.height && other.y < y + height;
    }
};

struct PerformanceHeaderLayout {
    PerformanceLayoutRect seedLabel;
    PerformanceLayoutRect seedEditor;
    PerformanceLayoutRect alea;
    PerformanceLayoutRect tempo;
    PerformanceLayoutRect master;
    // LEARN (7/11): o botão do LEARN da família, ao lado de TUTORIAL e SOBRE;
    // TEMPO e MASTER cederam 35 px cada (210 → 175) para ele caber.
    PerformanceLayoutRect learn;
    PerformanceLayoutRect tutorial;
    PerformanceLayoutRect about;
    PerformanceLayoutRect language;
    PerformanceLayoutRect recordingBox;
    PerformanceLayoutRect record;
    PerformanceLayoutRect transport;
};

constexpr PerformanceHeaderLayout performanceHeaderLayout(int width) noexcept {
    return {
        {width - 1012, 24, 98, 28}, {width - 910, 24, 104, 28},
        {width - 800, 24, 86, 28}, {width - 694, 24, 175, 28},
        {width - 519, 24, 175, 28},
        {width - 338, 24, 72, 28},
        {width - 258, 24, 86, 28}, {width - 164, 24, 86, 28},
        {width - 70, 24, 38, 28},
        {width - 228, 56, 196, 50},
        {width - 220, 74, 86, 28}, {width - 126, 74, 86, 28}
    };
}

constexpr bool performanceHeaderTargetsAreDisjoint(int width) noexcept {
    const auto targets = performanceHeaderLayout(width);
    const std::array<PerformanceLayoutRect, 11> rectangles{
        targets.seedEditor, targets.alea, targets.tempo, targets.master, targets.learn,
        targets.tutorial, targets.about, targets.record,
        targets.transport, targets.seedLabel, targets.language};
    for (std::size_t left = 0; left < rectangles.size(); ++left)
        for (std::size_t right = left + 1; right < rectangles.size(); ++right)
            if (rectangles[left].intersects(rectangles[right])) return false;
    return true;
}

constexpr int performanceSceneUtilityReserve() noexcept { return 584; }

struct PerformanceGraphLayout {
    int interactionBottomInset = 94;
    int libraryTopInset = 88;
    int libraryHeight = 40;
    int cueTopInset = 36;
    int cueHeight = 16;
    int logTopInset = 18;
};

constexpr PerformanceGraphLayout performanceGraphLayout() noexcept { return {}; }

constexpr bool performanceGraphLowerStripIsDisjoint() noexcept {
    const auto layout = performanceGraphLayout();
    const auto rackTopFromBottom = layout.interactionBottomInset + 28;
    const auto libraryBottomFromBottom = layout.libraryTopInset - layout.libraryHeight;
    const auto cueBottomFromBottom = layout.cueTopInset - layout.cueHeight;
    return rackTopFromBottom >= layout.libraryTopInset
        && libraryBottomFromBottom >= layout.cueTopInset
        && cueBottomFromBottom >= layout.logTopInset;
}

} // namespace rasgo::performance
