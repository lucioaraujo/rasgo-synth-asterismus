#pragma once

#include "PerformanceMotionGeometry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace rasgo::performance {

constexpr PerformanceRelationalPhenomenon nextRelationalPhenomenon(
    PerformanceRelationalPhenomenon value) noexcept {
    const auto next = (static_cast<std::uint8_t>(value) + 1U)
        % static_cast<std::uint8_t>(PerformanceRelationalPhenomenon::Count);
    return static_cast<PerformanceRelationalPhenomenon>(next);
}

inline const char* relationalPhenomenonLabel(PerformanceRelationalPhenomenon value) noexcept {
    switch (value) {
    case PerformanceRelationalPhenomenon::Attract: return "ATRAIR";
    case PerformanceRelationalPhenomenon::Repel: return "REPELIR";
    case PerformanceRelationalPhenomenon::Orbit: return "ORBITAR";
    case PerformanceRelationalPhenomenon::EllipticalOrbit: return "ORBITA ELIPTICA";
    case PerformanceRelationalPhenomenon::BarycentricDance: return "DANCA BARICENTRICA";
    case PerformanceRelationalPhenomenon::Encounter: return "ENCONTRO";
    case PerformanceRelationalPhenomenon::Merge: return "FUSIONAR";
    case PerformanceRelationalPhenomenon::Flyby: return "SOBREVOAR";
    case PerformanceRelationalPhenomenon::Scatter: return "ESPALHAR";
    case PerformanceRelationalPhenomenon::Chase: return "PERSEGUIR";
    case PerformanceRelationalPhenomenon::Escape: return "FUGIR";
    case PerformanceRelationalPhenomenon::Parallel: return "PARALELO";
    case PerformanceRelationalPhenomenon::Converge: return "CONVERGIR";
    case PerformanceRelationalPhenomenon::Diverge: return "DIVERGIR";
    case PerformanceRelationalPhenomenon::Oscillate: return "OSCILAR";
    case PerformanceRelationalPhenomenon::SpiralIn: return "ESPIRAL >";
    case PerformanceRelationalPhenomenon::SpiralOut: return "ESPIRAL <";
    case PerformanceRelationalPhenomenon::Equilibrium: return "EQUILIBRIO";
    case PerformanceRelationalPhenomenon::Lock: return "TRAVAR";
    case PerformanceRelationalPhenomenon::Synchronize: return "SINCRONIZAR";
    case PerformanceRelationalPhenomenon::Resonate: return "RESSOAR";
    case PerformanceRelationalPhenomenon::Swap: return "TROCAR";
    case PerformanceRelationalPhenomenon::Release: return "LIBERAR";
    case PerformanceRelationalPhenomenon::Count: break;
    }
    return "RELACIONAR";
}

struct PerformanceRelationObservation {
    double distance = 0.0;
    double angleRadians = 0.0;
    double radialVelocity = 0.0;
    double angularVelocity = 0.0;
    double proximity = 1.0;
    double encounter = 0.0;

    bool valid() const noexcept {
        return unit(distance) && finite(angleRadians) && finite(radialVelocity)
            && radialVelocity >= -1.0 && radialVelocity <= 1.0
            && finite(angularVelocity) && angularVelocity >= -1.0
            && angularVelocity <= 1.0 && unit(proximity) && unit(encounter);
    }
};

inline double wrapRelationAngle(double angle) noexcept {
    constexpr double kPi = 3.14159265358979323846;
    constexpr double kTwoPi = kPi * 2.0;
    while (angle > kPi) angle -= kTwoPi;
    while (angle < -kPi) angle += kTwoPi;
    return angle;
}

inline PerformanceRelationObservation observePerformanceRelation(
    const PerformanceMotionPoint& previousSource,
    const PerformanceMotionPoint& previousDestination,
    const PerformanceMotionPoint& source,
    const PerformanceMotionPoint& destination,
    double elapsedSeconds) noexcept {
    constexpr double kPi = 3.14159265358979323846;
    const auto dx = destination.x - source.x;
    const auto dy = destination.y - source.y;
    const auto previousDx = previousDestination.x - previousSource.x;
    const auto previousDy = previousDestination.y - previousSource.y;
    const auto distance = performanceMotionDistance(source, destination);
    const auto previousDistance = performanceMotionDistance(previousSource, previousDestination);
    const auto angle = std::atan2(dy, dx);
    const auto previousAngle = std::atan2(previousDy, previousDx);
    const auto dt = finite(elapsedSeconds) && elapsedSeconds > 1.0e-6
        ? elapsedSeconds : 1.0;
    const auto radial = std::clamp((distance - previousDistance) / dt, -1.0, 1.0);
    const auto angular = std::clamp(wrapRelationAngle(angle - previousAngle) / (dt * kPi), -1.0, 1.0);
    const auto proximity = clampUnit(1.0 - distance);
    // Encounters become compositionally significant before points overlap.
    const auto encounter = clampUnit((0.24 - distance) / 0.24);
    return {distance, angle, radial, angular, proximity, encounter};
}

// Orthogonal outputs keep the operators distinguishable in sound. A consumer
// may route them differently for each module, but must never collapse all of
// them into one generic gain value.
struct PerformanceCompositionalForces {
    double pulseCoupling = 0.0;
    double temporalFracture = 0.0;
    double matterExchange = 0.0;
    double matterFusion = 0.0;
    double spatialCirculation = 0.0;
    double dynamicTension = 0.0;
    double memoryTrace = 0.0;
    double seedDeviation = 0.0;
    double silence = 0.0;
    double sourceBias = 0.5;
    double destinationBias = 0.5;

    bool valid() const noexcept {
        return unit(pulseCoupling) && unit(temporalFracture)
            && unit(matterExchange) && unit(matterFusion)
            && unit(spatialCirculation) && unit(dynamicTension)
            && unit(memoryTrace) && unit(seedDeviation) && unit(silence)
            && unit(sourceBias) && unit(destinationBias);
    }
};

inline PerformanceCompositionalForces composeRelationalPhenomenon(
    PerformanceRelationalPhenomenon phenomenon,
    const PerformanceRelationObservation& observation,
    double phaseRadians,
    double intensity,
    double resonanceRatio = 1.0) noexcept {
    if (!validRelationalPhenomenon(phenomenon) || !observation.valid()) return {};
    const auto amount = clampUnit(intensity);
    const auto near = observation.proximity;
    const auto encounter = observation.encounter;
    const auto motion = clampUnit(std::fabs(observation.radialVelocity)
                                  + std::fabs(observation.angularVelocity));
    const auto wave = 0.5 + 0.5 * std::sin(finite(phaseRadians) ? phaseRadians : 0.0);
    const auto ratioDistance = clampUnit(std::fabs(
        std::clamp(resonanceRatio, 0.25, 4.0) - 1.0) / 3.0);
    PerformanceCompositionalForces out;
    switch (phenomenon) {
    case PerformanceRelationalPhenomenon::Attract:
        out = {0.28 * near, 0.02, 0.54 * near, 0.18 * near, 0.20, 0.26, 0.12, 0.08, 0.0, 0.45, 0.55}; break;
    case PerformanceRelationalPhenomenon::Repel:
        out = {0.06, 0.34 * near, 0.05, 0.0, 0.30, 0.72 * near, 0.18, 0.24, 0.14 * near, 0.62, 0.38}; break;
    case PerformanceRelationalPhenomenon::Orbit:
        out = {0.38, 0.04, 0.18, 0.04, 0.82 * (0.45 + 0.55 * motion), 0.22, 0.26, 0.10, 0.0, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::EllipticalOrbit:
        out = {0.46 * wave, 0.08, 0.26 * near, 0.08, 0.86, 0.34 * (1.0 - wave), 0.32, 0.18 * wave, 0.02, 0.56, 0.44}; break;
    case PerformanceRelationalPhenomenon::BarycentricDance:
        out = {0.62, 0.06, 0.34, 0.14, 0.88, 0.28, 0.40, 0.16, 0.0, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::Encounter:
        out = {0.16, 0.76 * encounter, 0.48 * encounter, 0.18, 0.22, 0.70 * encounter, 0.42 * encounter, 0.34, 0.62 * encounter, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::Merge:
        out = {0.66 * near, 0.05, 0.78 * near, 0.92 * near, 0.16, 0.18, 0.70 * near, 0.30, 0.0, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::Flyby:
        out = {0.12, 0.44 * wave, 0.28 * encounter, 0.0, 0.74 * motion, 0.52 * encounter, 0.34 * encounter, 0.40 * wave, 0.24 * encounter, 0.58, 0.42}; break;
    case PerformanceRelationalPhenomenon::Scatter:
        out = {0.04, 0.68, 0.10, 0.0, 0.58, 0.82, 0.22, 0.66, 0.18, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::Chase:
        out = {0.58, 0.20, 0.24, 0.0, 0.46, 0.48, 0.38, 0.20, 0.0, 0.72, 0.28}; break;
    case PerformanceRelationalPhenomenon::Escape:
        out = {0.20, 0.56, 0.06, 0.0, 0.54, 0.76, 0.28, 0.58, 0.12, 0.30, 0.70}; break;
    case PerformanceRelationalPhenomenon::Parallel:
        out = {0.52, 0.02, 0.16, 0.0, 0.34, 0.10, 0.28, 0.06, 0.0, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::Converge:
        out = {0.46 * near, 0.14, 0.52 * near, 0.34 * near, 0.20, 0.34, 0.24, 0.18, 0.0, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::Diverge:
        out = {0.08, 0.40, 0.08, 0.0, 0.42, 0.68, 0.20, 0.48, 0.10, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::Oscillate:
        out = {0.42 + 0.30 * wave, 0.14 * (1.0 - wave), 0.30 * wave, 0.08, 0.52, 0.38 * (1.0 - wave), 0.26, 0.22 * wave, 0.04, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::SpiralIn:
        out = {0.48 * near, 0.12, 0.56 * near, 0.38 * near, 0.72, 0.42, 0.34, 0.30, 0.0, 0.46, 0.54}; break;
    case PerformanceRelationalPhenomenon::SpiralOut:
        out = {0.14, 0.46, 0.08, 0.0, 0.76, 0.64, 0.30, 0.54, 0.12, 0.54, 0.46}; break;
    case PerformanceRelationalPhenomenon::Equilibrium:
        out = {0.44, 0.0, 0.20, 0.08, 0.18, 0.06, 0.42, 0.02, 0.0, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::Lock:
        out = {0.88, 0.0, 0.34, 0.22, 0.06, 0.08, 0.62, 0.0, 0.0, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::Synchronize:
        out = {0.96, 0.02, 0.22, 0.10, 0.12, 0.08, 0.46, 0.04, 0.0, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::Resonate:
        out = {0.72, 0.08 * ratioDistance, 0.48, 0.24, 0.30, 0.20 + 0.30 * ratioDistance, 0.58, 0.16, 0.0, 0.50, 0.50}; break;
    case PerformanceRelationalPhenomenon::Swap:
        out = {0.36, 0.52 * wave, 0.58, 0.08, 0.44, 0.46, 0.40, 0.34, 0.12, wave, 1.0 - wave}; break;
    case PerformanceRelationalPhenomenon::Release:
        out = {0.05, 0.36, 0.02, 0.0, 0.62, 0.54, 0.72, 0.46, 0.18, 0.42, 0.58}; break;
    case PerformanceRelationalPhenomenon::Count:
        return {};
    }
    auto scale = [amount](double value) { return clampUnit(value * amount); };
    out.pulseCoupling = scale(out.pulseCoupling);
    out.temporalFracture = scale(out.temporalFracture);
    out.matterExchange = scale(out.matterExchange);
    out.matterFusion = scale(out.matterFusion);
    out.spatialCirculation = scale(out.spatialCirculation);
    out.dynamicTension = scale(out.dynamicTension);
    out.memoryTrace = scale(out.memoryTrace);
    out.seedDeviation = scale(out.seedDeviation);
    out.silence = scale(out.silence);
    return out;
}

struct PerformanceRelationalDisplacement {
    double sourceX = 0.0;
    double sourceY = 0.0;
    double destinationX = 0.0;
    double destinationY = 0.0;
};

// The displacement is always relative to the authored points and bounded to
// 7.5% of the field. Stopping or disabling a phenomenon therefore returns to
// the drawing instead of replacing it.
inline PerformanceRelationalDisplacement relationalDisplacement(
    PerformanceRelationalPhenomenon phenomenon,
    const PerformanceMotionPoint& source,
    const PerformanceMotionPoint& destination,
    double phaseRadians,
    double intensity) noexcept {
    const auto dx = destination.x - source.x;
    const auto dy = destination.y - source.y;
    const auto length = std::max(std::hypot(dx, dy), 1.0e-6);
    const auto ux = dx / length;
    const auto uy = dy / length;
    const auto tx = -uy;
    const auto ty = ux;
    const auto amount = 0.075 * clampUnit(intensity);
    const auto wave = std::sin(finite(phaseRadians) ? phaseRadians : 0.0);
    PerformanceRelationalDisplacement out;
    auto symmetric = [&](double radial, double tangent) {
        out.sourceX = ux * radial + tx * tangent;
        out.sourceY = uy * radial + ty * tangent;
        out.destinationX = -ux * radial - tx * tangent;
        out.destinationY = -uy * radial - ty * tangent;
    };
    switch (phenomenon) {
    case PerformanceRelationalPhenomenon::Attract: symmetric(amount, 0.0); break;
    case PerformanceRelationalPhenomenon::Repel: symmetric(-amount, 0.0); break;
    case PerformanceRelationalPhenomenon::Orbit: symmetric(0.0, amount * wave); break;
    case PerformanceRelationalPhenomenon::EllipticalOrbit:
        symmetric(amount * wave * 0.34, amount * std::cos(phaseRadians)); break;
    case PerformanceRelationalPhenomenon::BarycentricDance:
        out.sourceX = tx * amount * wave * 0.72;
        out.sourceY = ty * amount * wave * 0.72;
        out.destinationX = -tx * amount * wave;
        out.destinationY = -ty * amount * wave; break;
    case PerformanceRelationalPhenomenon::Encounter: symmetric(amount * std::max(0.0, wave), 0.0); break;
    case PerformanceRelationalPhenomenon::Merge: symmetric(amount * (0.70 + 0.30 * wave), 0.0); break;
    case PerformanceRelationalPhenomenon::Flyby: symmetric(amount * wave * 0.30, amount * wave); break;
    case PerformanceRelationalPhenomenon::Scatter: symmetric(-amount * (0.55 + 0.45 * std::fabs(wave)), amount * wave * 0.25); break;
    case PerformanceRelationalPhenomenon::Chase:
        out.sourceX = ux * amount; out.sourceY = uy * amount;
        out.destinationX = tx * amount * wave * 0.35; out.destinationY = ty * amount * wave * 0.35; break;
    case PerformanceRelationalPhenomenon::Escape:
        out.sourceX = ux * amount * 0.45; out.sourceY = uy * amount * 0.45;
        out.destinationX = ux * amount; out.destinationY = uy * amount; break;
    case PerformanceRelationalPhenomenon::Parallel:
        out.sourceX = out.destinationX = tx * amount * wave;
        out.sourceY = out.destinationY = ty * amount * wave; break;
    case PerformanceRelationalPhenomenon::Converge: symmetric(amount * (0.35 + 0.65 * std::fabs(wave)), 0.0); break;
    case PerformanceRelationalPhenomenon::Diverge: symmetric(-amount * (0.35 + 0.65 * std::fabs(wave)), 0.0); break;
    case PerformanceRelationalPhenomenon::Oscillate: symmetric(amount * wave, 0.0); break;
    case PerformanceRelationalPhenomenon::SpiralIn: symmetric(amount * (0.45 + 0.25 * wave), amount * wave); break;
    case PerformanceRelationalPhenomenon::SpiralOut: symmetric(-amount * (0.45 + 0.25 * wave), amount * wave); break;
    case PerformanceRelationalPhenomenon::Equilibrium: symmetric(amount * wave * 0.12, amount * wave * 0.08); break;
    case PerformanceRelationalPhenomenon::Lock: break;
    case PerformanceRelationalPhenomenon::Synchronize:
        out.destinationX = tx * amount * wave * 0.25;
        out.destinationY = ty * amount * wave * 0.25; break;
    case PerformanceRelationalPhenomenon::Resonate: symmetric(amount * wave * 0.18, amount * wave * 0.42); break;
    case PerformanceRelationalPhenomenon::Swap: symmetric(amount * wave, amount * std::cos(phaseRadians)); break;
    case PerformanceRelationalPhenomenon::Release: symmetric(-amount * std::max(0.0, wave), amount * wave * 0.20); break;
    case PerformanceRelationalPhenomenon::Count: break;
    }
    return out;
}

inline PerformanceRelationalPhenomenon phenomenonForRelationKind(RelationKind kind) noexcept {
    switch (kind) {
    case RelationKind::Support: return PerformanceRelationalPhenomenon::Parallel;
    case RelationKind::CallResponse: return PerformanceRelationalPhenomenon::Chase;
    case RelationKind::Hocket: return PerformanceRelationalPhenomenon::Swap;
    case RelationKind::Shadow: return PerformanceRelationalPhenomenon::Chase;
    case RelationKind::Contagion: return PerformanceRelationalPhenomenon::Attract;
    case RelationKind::Friction: return PerformanceRelationalPhenomenon::Repel;
    case RelationKind::Infiltration: return PerformanceRelationalPhenomenon::SpiralIn;
    case RelationKind::Repair: return PerformanceRelationalPhenomenon::Equilibrium;
    case RelationKind::Orbit: return PerformanceRelationalPhenomenon::Orbit;
    case RelationKind::Count: break;
    }
    return PerformanceRelationalPhenomenon::Attract;
}

inline const char* populationPhenomenonLabel(PerformancePopulationPhenomenon value) noexcept {
    switch (value) {
    case PerformancePopulationPhenomenon::Align: return "ALINHAR";
    case PerformancePopulationPhenomenon::Cluster: return "AGRUPAR";
    case PerformancePopulationPhenomenon::Scatter: return "DISPERSAR";
    case PerformancePopulationPhenomenon::CollectiveRotate: return "GIRAR CONJUNTO";
    case PerformancePopulationPhenomenon::Expand: return "EXPANDIR";
    case PerformancePopulationPhenomenon::Contract: return "CONTRAIR";
    case PerformancePopulationPhenomenon::Pulse: return "PULSAR";
    case PerformancePopulationPhenomenon::Fragment: return "FRAGMENTAR";
    case PerformancePopulationPhenomenon::MergeGroups: return "FUNDIR GRUPOS";
    case PerformancePopulationPhenomenon::Drift: return "DERIVAR";
    case PerformancePopulationPhenomenon::Count: break;
    }
    return "CAMPO";
}

inline PerformanceCompositionalForces composePopulationPhenomenon(
    PerformancePopulationPhenomenon phenomenon, double phaseRadians,
    double intensity, double cohesion) noexcept {
    if (!validPopulationPhenomenon(phenomenon)) return {};
    const auto amount = clampUnit(intensity);
    const auto together = clampUnit(cohesion);
    const auto wave = 0.5 + 0.5 * std::sin(finite(phaseRadians) ? phaseRadians : 0.0);
    PerformanceCompositionalForces out;
    switch (phenomenon) {
    case PerformancePopulationPhenomenon::Align:
        out = {0.78, 0.02, 0.18, 0.06, 0.28, 0.10, 0.32, 0.04, 0.0, 0.5, 0.5}; break;
    case PerformancePopulationPhenomenon::Cluster:
        out = {0.48, 0.04, 0.58 * together, 0.44 * together, 0.22, 0.24, 0.46, 0.18, 0.0, 0.5, 0.5}; break;
    case PerformancePopulationPhenomenon::Scatter:
        out = {0.06, 0.64, 0.08, 0.0, 0.58, 0.84, 0.24, 0.68, 0.16, 0.5, 0.5}; break;
    case PerformancePopulationPhenomenon::CollectiveRotate:
        out = {0.42, 0.04, 0.18, 0.02, 0.92, 0.22, 0.30, 0.12, 0.0, 0.5, 0.5}; break;
    case PerformancePopulationPhenomenon::Expand:
        out = {0.10, 0.34, 0.08, 0.0, 0.62, 0.56, 0.26, 0.40, 0.08, 0.5, 0.5}; break;
    case PerformancePopulationPhenomenon::Contract:
        out = {0.50, 0.06, 0.50, 0.32, 0.34, 0.28, 0.38, 0.16, 0.0, 0.5, 0.5}; break;
    case PerformancePopulationPhenomenon::Pulse:
        out = {0.64 * wave, 0.18 * (1.0 - wave), 0.28 * wave, 0.12, 0.46, 0.36, 0.28, 0.22 * wave, 0.04, 0.5, 0.5}; break;
    case PerformancePopulationPhenomenon::Fragment:
        out = {0.08, 0.88, 0.12, 0.0, 0.54, 0.90, 0.48, 0.76, 0.28, 0.5, 0.5}; break;
    case PerformancePopulationPhenomenon::MergeGroups:
        out = {0.60, 0.10, 0.72 * together, 0.86 * together, 0.26, 0.30, 0.68, 0.34, 0.0, 0.5, 0.5}; break;
    case PerformancePopulationPhenomenon::Drift:
        out = {0.20, 0.10, 0.16, 0.0, 0.68, 0.24, 0.54, 0.46, 0.02, 0.5, 0.5}; break;
    case PerformancePopulationPhenomenon::Count:
        return {};
    }
    auto scale = [amount](double value) { return clampUnit(value * amount); };
    out.pulseCoupling = scale(out.pulseCoupling);
    out.temporalFracture = scale(out.temporalFracture);
    out.matterExchange = scale(out.matterExchange);
    out.matterFusion = scale(out.matterFusion);
    out.spatialCirculation = scale(out.spatialCirculation);
    out.dynamicTension = scale(out.dynamicTension);
    out.memoryTrace = scale(out.memoryTrace);
    out.seedDeviation = scale(out.seedDeviation);
    out.silence = scale(out.silence);
    return out;
}

// Population movement reads the musician's authored point every time. No
// internal integration accumulates drift, so disabling the operator restores
// the original constellation exactly.
inline PerformanceMotionPoint populationMotionPoint(
    PerformancePopulationPhenomenon phenomenon,
    const PerformanceMotionPoint& authored,
    const PerformanceMotionPoint& centre,
    double phaseRadians, double intensity,
    std::size_t actorIndex, std::size_t actorCount) noexcept {
    if (!validPopulationPhenomenon(phenomenon) || !finite(phaseRadians)) return authored;
    const auto amount = clampUnit(intensity);
    const auto dx = authored.x - centre.x;
    const auto dy = authored.y - centre.y;
    const auto radius = std::hypot(dx, dy);
    const auto angle = std::atan2(dy, dx);
    const auto wave = std::sin(phaseRadians);
    const auto indexedPhase = phaseRadians
        + static_cast<double>(actorIndex) * 6.28318530717958647692
            / static_cast<double>(std::max<std::size_t>(actorCount, 1U));
    double nextX = authored.x;
    double nextY = authored.y;
    switch (phenomenon) {
    case PerformancePopulationPhenomenon::Align:
        nextX += std::cos(phaseRadians) * 0.035 * amount;
        nextY += std::sin(phaseRadians) * 0.035 * amount; break;
    case PerformancePopulationPhenomenon::Cluster:
        nextX = centre.x + dx * (1.0 - 0.24 * amount);
        nextY = centre.y + dy * (1.0 - 0.24 * amount); break;
    case PerformancePopulationPhenomenon::Scatter:
        nextX = centre.x + dx * (1.0 + 0.22 * amount);
        nextY = centre.y + dy * (1.0 + 0.22 * amount); break;
    case PerformancePopulationPhenomenon::CollectiveRotate: {
        const auto rotation = phaseRadians * (0.08 + 0.22 * amount);
        nextX = centre.x + std::cos(rotation) * dx - std::sin(rotation) * dy;
        nextY = centre.y + std::sin(rotation) * dx + std::cos(rotation) * dy;
        break;
    }
    case PerformancePopulationPhenomenon::Expand:
        nextX = centre.x + dx * (1.0 + 0.20 * amount * (0.5 + 0.5 * wave));
        nextY = centre.y + dy * (1.0 + 0.20 * amount * (0.5 + 0.5 * wave)); break;
    case PerformancePopulationPhenomenon::Contract:
        nextX = centre.x + dx * (1.0 - 0.20 * amount * (0.5 + 0.5 * wave));
        nextY = centre.y + dy * (1.0 - 0.20 * amount * (0.5 + 0.5 * wave)); break;
    case PerformancePopulationPhenomenon::Pulse: {
        const auto scale = 1.0 + wave * 0.14 * amount;
        nextX = centre.x + dx * scale;
        nextY = centre.y + dy * scale; break;
    }
    case PerformancePopulationPhenomenon::Fragment: {
        const auto fragment = std::sin(indexedPhase * (1.0 + actorIndex % 3U));
        nextX = centre.x + std::cos(angle) * (radius + fragment * 0.065 * amount);
        nextY = centre.y + std::sin(angle) * (radius + fragment * 0.065 * amount); break;
    }
    case PerformancePopulationPhenomenon::MergeGroups: {
        const auto side = actorIndex % 2U == 0U ? -1.0 : 1.0;
        const auto targetX = centre.x + side * 0.09;
        nextX = authored.x + (targetX - authored.x) * 0.28 * amount;
        nextY = authored.y + (centre.y - authored.y) * 0.20 * amount; break;
    }
    case PerformancePopulationPhenomenon::Drift:
        nextX += std::cos(indexedPhase * 0.37) * 0.045 * amount;
        nextY += std::sin(indexedPhase * 0.29) * 0.045 * amount; break;
    case PerformancePopulationPhenomenon::Count: break;
    }
    return {std::clamp(nextX, 0.0, 1.0), std::clamp(nextY, 0.0, 1.0), authored.z};
}

} // namespace rasgo::performance
