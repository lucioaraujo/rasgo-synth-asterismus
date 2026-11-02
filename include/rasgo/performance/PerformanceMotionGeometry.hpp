#pragma once

#include "PerformanceContracts.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace rasgo::performance {

// One shared geometry for rendering and audio. The anchor belongs to the
// performer; this point is the live musical body moving around it.
struct PerformanceMotionPoint {
    double x = 0.5;
    double y = 0.5;
    double z = 0.5;
};

inline PerformanceMotionPoint performanceMotionPoint(
    double anchorX, double anchorY, PerformanceMotionField field,
    double phaseRadians, std::size_t actorIndex, double presence) noexcept {
    constexpr double kTwoPi = 6.28318530717958647692;
    constexpr double kHalfPi = 1.57079632679489661923;
    const auto boundedPresence = std::clamp(presence, 0.0, 1.0);
    const auto phase = phaseRadians + static_cast<double>(actorIndex) * kHalfPi;
    const auto radiusX = 0.052 + boundedPresence * 0.030;
    const auto radiusY = 0.044 + boundedPresence * 0.026;
    double offsetX = std::cos(phase) * radiusX;
    double offsetY = std::sin(phase) * radiusY;
    double depth = 0.5 + std::sin(phase + kHalfPi) * (0.22 + boundedPresence * 0.14);

    if (field == PerformanceMotionField::Pulse) {
        const auto pulse = 0.18 + 0.82 * (0.5 + 0.5 * std::sin(phase));
        const auto axis = static_cast<double>(actorIndex) * kHalfPi + kHalfPi * 0.5;
        offsetX = std::cos(axis) * radiusX * pulse;
        offsetY = std::sin(axis) * radiusY * pulse;
        depth = 0.5 + (pulse - 0.5) * (0.42 + boundedPresence * 0.18);
    } else if (field == PerformanceMotionField::FigureEight) {
        offsetX = std::sin(phase) * radiusX;
        offsetY = std::sin(phase * 2.0) * radiusY;
        depth = 0.5 + std::cos(phase * 2.0) * (0.20 + boundedPresence * 0.12);
    }

    // Keep the live body inside the normalized field while preserving the
    // anchor separately. kTwoPi is referenced here to keep phase handling
    // explicit and finite for arbitrarily long sessions.
    const auto finitePhase = std::fmod(phaseRadians, kTwoPi);
    if (!std::isfinite(finitePhase)) return {anchorX, anchorY, 0.5};
    return {std::clamp(anchorX + offsetX, 0.0, 1.0),
            std::clamp(anchorY + offsetY, 0.0, 1.0),
            std::clamp(depth, 0.0, 1.0)};
}

// The collective field applies one transform to every authored point. Orbit is
// a rigid-body rotation plus translation: pairwise distances are invariant.
// Pulse/Figure-8 may add a common scale as an explicit breathing/deformation
// layer, while local forces are handled separately by the ecology.
inline PerformanceMotionPoint performanceCollectiveMotionPoint(
    double anchorX, double anchorY, double centreX, double centreY,
    PerformanceMotionField field, double phaseRadians) noexcept {
    if (!std::isfinite(phaseRadians)) return {anchorX, anchorY, 0.5};
    const auto dx = anchorX - centreX;
    const auto dy = anchorY - centreY;
    double angle = std::sin(phaseRadians) * 0.42;
    double scale = 1.0;
    if (field == PerformanceMotionField::Pulse) {
        angle = std::sin(phaseRadians) * 0.20;
        scale = 0.82 + 0.18 * (0.5 + 0.5 * std::sin(phaseRadians));
    } else if (field == PerformanceMotionField::FigureEight) {
        angle = std::sin(phaseRadians * 2.0) * 0.50;
        scale = 0.90 + 0.10 * (0.5 + 0.5 * std::cos(phaseRadians));
    }
    const auto movedX = centreX + (dx * std::cos(angle) - dy * std::sin(angle)) * scale
        + std::cos(phaseRadians) * 0.028;
    const auto movedY = centreY + (dx * std::sin(angle) + dy * std::cos(angle)) * scale
        + std::sin(phaseRadians) * 0.022;
    return {std::clamp(movedX, 0.0, 1.0), std::clamp(movedY, 0.0, 1.0),
            std::clamp(0.5 + std::sin(phaseRadians) * 0.16, 0.0, 1.0)};
}

inline double performanceMotionDistance(const PerformanceMotionPoint& first,
                                        const PerformanceMotionPoint& second) noexcept {
    const auto dx = first.x - second.x;
    const auto dy = first.y - second.y;
    const auto dz = first.z - second.z;
    return std::clamp(std::sqrt(dx * dx + dy * dy + dz * dz) / std::sqrt(3.0), 0.0, 1.0);
}

} // namespace rasgo::performance
