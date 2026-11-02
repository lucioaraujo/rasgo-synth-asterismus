#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iterator>

namespace rasgo::performance {

struct PerformanceConstellationPoint {
    double x = 0.5;
    double y = 0.5;
};

inline constexpr double kConstellationRelationDistance = 0.90;

inline double constellationRelationFromDistance(double distance) noexcept {
    return std::clamp(1.0 - distance / kConstellationRelationDistance, 0.0, 1.0);
}

inline std::array<PerformanceConstellationPoint, 3> projectConstellationRelations(
    const std::array<double, 3>& relationIntensities) noexcept {
    // Pair order is the public Performance contract: 0-1 matter, 1-2 time,
    // 0-2 space. Independent intensities are not always a valid Euclidean
    // triangle, so share any triangle-inequality excess equally. This is the
    // least-squares projection of that violating edge onto the boundary.
    std::array<double, 3> distances{};
    for (std::size_t index = 0U; index < distances.size(); ++index)
        distances[index] = kConstellationRelationDistance
            * (1.0 - std::clamp(relationIntensities[index], 0.0, 1.0));

    for (std::size_t pass = 0U; pass < 3U; ++pass) {
        const auto longest = static_cast<std::size_t>(
            std::distance(distances.begin(), std::max_element(distances.begin(), distances.end())));
        const auto first = (longest + 1U) % 3U;
        const auto second = (longest + 2U) % 3U;
        const auto excess = distances[longest] - distances[first] - distances[second];
        if (excess <= 0.0) break;
        const auto correction = excess / 3.0;
        distances[longest] -= correction;
        distances[first] += correction;
        distances[second] += correction;
    }

    static constexpr std::array<std::array<std::size_t, 2>, 3> pairs{{
        {{0U, 1U}}, {{1U, 2U}}, {{0U, 2U}}
    }};
    const auto longest = static_cast<std::size_t>(
        std::distance(distances.begin(), std::max_element(distances.begin(), distances.end())));
    const auto source = pairs[longest][0U];
    const auto destination = pairs[longest][1U];
    const auto remaining = 3U - source - destination;
    const auto base = distances[longest];

    std::array<PerformanceConstellationPoint, 3> points{};
    if (base <= 1.0e-12) return points;

    const auto pairIndex = [](std::size_t left, std::size_t right) noexcept {
        if ((left == 0U && right == 1U) || (left == 1U && right == 0U)) return 0U;
        if ((left == 1U && right == 2U) || (left == 2U && right == 1U)) return 1U;
        return 2U;
    };
    const auto sourceDistance = distances[pairIndex(source, remaining)];
    const auto destinationDistance = distances[pairIndex(destination, remaining)];
    const auto remainingX = (sourceDistance * sourceDistance + base * base
                             - destinationDistance * destinationDistance) / (2.0 * base);
    const auto remainingY = std::sqrt(std::max(
        0.0, sourceDistance * sourceDistance - remainingX * remainingX));

    const auto left = 0.5 - base * 0.5;
    const auto baseline = 0.5 + remainingY * 0.5;
    points[source] = {left, baseline};
    points[destination] = {left + base, baseline};
    points[remaining] = {left + remainingX, baseline - remainingY};
    return points;
}

} // namespace rasgo::performance
