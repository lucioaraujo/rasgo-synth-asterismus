#pragma once

#include "PerformanceSessionDocument.hpp"
#include "PerformanceSky.hpp"
#include "PerformanceStemDistribution.hpp"

#include <array>
#include <cstdint>

namespace rasgo::performance {

// Control-side territory maker. A seed does not select a cosmetic preset: it
// establishes bounded identity, actor potential and the first four playable
// constellations. No random source, allocation or I/O enters the audio path.
class PerformanceTerritoryFactory {
public:
    // The opening controls are part of a territory's musical identity.  They
    // must not reset to one cosmetic panel layout every time the performer
    // rolls a new seed.
    static std::array<double, 3> initialModuleControls(std::uint32_t seed,
                                                        std::uint16_t actorId) noexcept {
        const auto salt = 160U + static_cast<std::uint32_t>(actorId) * 3U;
        return {{between(seed, salt, 0.24, 0.80),
                 between(seed, salt + 1U, 0.28, 0.76),
                 between(seed, salt + 2U, 0.30, 0.74)}};
    }

    static PerformanceSessionDocument make(std::uint32_t seed, bool playing = false) noexcept {
        PerformanceSessionDocument document;
        document.territory.seed = seed;
        document.territory.identity = {
            unit(seed, 1U), unit(seed, 2U), unit(seed, 3U), unit(seed, 4U),
            between(seed, 5U, 0.58, 0.94), between(seed, 6U, 0.46, 0.92)};
        document.territory.gamePlan = nativeGamePlan(seed);

        document.territory.addActor({1U, PerformanceFamily::Tone, 0x03U,
                                     roleBit(PerformanceRole::Chao) | roleBit(PerformanceRole::Corpo),
                                     between(seed, 11U, 0.48, 0.82), 0.04, 0.92});
        document.territory.addActor({2U, PerformanceFamily::Texture, 0x14U,
                                     roleBit(PerformanceRole::Atrito) | roleBit(PerformanceRole::Vestigio),
                                     between(seed, 12U, 0.35, 0.72), 0.0, 1.0});
        document.territory.addActor({3U, PerformanceFamily::Pattern, 0x22U,
                                     roleBit(PerformanceRole::Contagio) | roleBit(PerformanceRole::Rasgo),
                                     between(seed, 13U, 0.30, 0.68), 0.0, 0.94});

        auto opening = scene(seed, 1U, playing);
        opening.constellation.addActor({1U, PerformanceRole::Chao, ActorLifeState::Active,
                                        between(seed, 21U, 0.42, 0.74),
                                        between(seed, 22U, 0.45, 0.72),
                                        between(seed, 23U, 0.32, 0.62),
                                        between(seed, 24U, 0.62, 0.88)});
        opening.constellation.addActor({2U, PerformanceRole::Atrito,
                                        ActorLifeState::Emerging,
                                        between(seed, 25U, 0.42, 0.68),
                                        between(seed, 26U, 0.54, 0.78),
                                        between(seed, 27U, 0.30, 0.52),
                                        between(seed, 28U, 0.52, 0.82)});
        opening.constellation.addActor({3U, PerformanceRole::Contagio,
                                        ActorLifeState::Emerging,
                                        between(seed, 29U, 0.34, 0.58),
                                        between(seed, 30U, 0.46, 0.68),
                                        between(seed, 31U, 0.24, 0.46),
                                        between(seed, 32U, 0.42, 0.72)});
        opening.constellation.addRelation({1U, 2U, RelationKind::Friction, RelationDomain::Matter,
                                           between(seed, 33U, 0.28, 0.68),
                                           between(seed, 34U, 8.0, 28.0),
                                           between(seed, 35U, 0.8, 3.0),
                                           between(seed, 36U, 0.52, 0.86)});
        opening.constellation.addRelation({2U, 3U, RelationKind::Contagion, RelationDomain::Time,
                                           between(seed, 37U, 0.24, 0.64),
                                           between(seed, 38U, 6.0, 22.0),
                                           between(seed, 39U, 0.6, 2.2),
                                           between(seed, 40U, 0.46, 0.78)});
        opening.constellation.addRelation({1U, 3U, RelationKind::Orbit, RelationDomain::Space,
                                           between(seed, 56U, 0.18, 0.58),
                                           between(seed, 57U, 10.0, 34.0),
                                           between(seed, 58U, 1.2, 3.6),
                                           between(seed, 59U, 0.42, 0.80)});
        document.addScene(opening);

        auto spiral = opening;
        configureScene(spiral, 2U, seed, 41U, playing);
        spiral.session.populationPhenomenon = PerformancePopulationPhenomenon::CollectiveRotate;
        spiral.session.populationPhenomenonIntensity = between(seed, 126U, 0.52, 0.82);
        spiral.session.populationPhenomenonEnabled = true;
        spiral.constellation.actors[0].role = PerformanceRole::Corpo;
        spiral.constellation.actors[1].presence = between(seed, 42U, 0.38, 0.68);
        spiral.constellation.actors[2].presence = between(seed, 43U, 0.36, 0.66);
        spiral.constellation.relations[1].intensity = between(seed, 44U, 0.46, 0.82);
        document.addScene(spiral);

        auto rupture = spiral;
        configureScene(rupture, 3U, seed, 45U, playing);
        rupture.session.populationPhenomenon = PerformancePopulationPhenomenon::Fragment;
        rupture.session.populationPhenomenonIntensity = between(seed, 127U, 0.60, 0.90);
        rupture.session.populationPhenomenonEnabled = true;
        rupture.constellation.actors[1].role = PerformanceRole::Vestigio;
        rupture.constellation.actors[2].role = PerformanceRole::Rasgo;
        rupture.constellation.actors[2].presence = between(seed, 46U, 0.48, 0.84);
        rupture.constellation.relations[0].intensity = between(seed, 47U, 0.66, 0.94);
        document.addScene(rupture);

        auto balance = opening;
        configureScene(balance, 4U, seed, 48U, playing);
        balance.session.populationPhenomenon = PerformancePopulationPhenomenon::Align;
        balance.session.populationPhenomenonIntensity = between(seed, 128U, 0.38, 0.68);
        balance.session.populationPhenomenonEnabled = true;
        balance.session.autonomy = between(seed, 49U, 0.42, 0.70);
        balance.session.intervention = between(seed, 50U, 0.40, 0.70);
        balance.session.space = between(seed, 51U, 0.45, 0.84);
        balance.constellation.actors[0].presence = between(seed, 52U, 0.36, 0.62);
        balance.constellation.actors[1].presence = between(seed, 53U, 0.30, 0.58);
        balance.constellation.actors[2].presence = between(seed, 54U, 0.20, 0.46);
        balance.constellation.relations[2].intensity = between(seed, 60U, 0.46, 0.82);
        document.addScene(balance);
        applyTerritoryLimits(document);
        return document;
    }

    // The fourth seed-selected matter belongs to the opening ensemble. It is
    // not a silent reserve: the performer may still reposition, mute or
    // recollect it, but a seed must never present its volume as permanently
    // zero merely because it occupies the fourth slot.
    static PerformanceSessionDocument makeWithOptionalModule(std::uint32_t seed,
                                                             bool playing = false) noexcept {
        auto document = make(seed, playing);
        // Every currently adapted ecology exposes exactly one complementary
        // fourth matter chosen by its seed. The performer may position or
        // recollect it, but cannot turn the opening into a random catalogue.
        if (performanceStarLibraryId(performanceInitialAsterism(seed)[3]).empty()) return document;
        document.territory.addActor({4U, PerformanceFamily::Tone, 0x03U,
                                     roleBit(PerformanceRole::Corpo)
                                         | roleBit(PerformanceRole::Vestigio),
                                     between(seed, 61U, 0.22, 0.58), 0.0, 1.0});
        for (std::size_t index = 0U; index < document.sceneCount; ++index) {
            auto& constellation = document.scenes[index].constellation;
            constellation.addActor({4U, PerformanceRole::Corpo, ActorLifeState::Emerging,
                                    between(seed, 62U + static_cast<std::uint32_t>(index), 0.34, 0.64),
                                    between(seed, 66U + static_cast<std::uint32_t>(index), 0.28, 0.54),
                                    between(seed, 69U + static_cast<std::uint32_t>(index), 0.08, 0.34), 0.50});
            constellation.addRelation({1U, 4U, RelationKind::Orbit, RelationDomain::Space,
                                       between(seed, 70U + static_cast<std::uint32_t>(index), 0.18, 0.62),
                                       between(seed, 71U + static_cast<std::uint32_t>(index), 8.0, 30.0),
                                       between(seed, 72U + static_cast<std::uint32_t>(index), 0.8, 3.0),
                                       between(seed, 73U + static_cast<std::uint32_t>(index), 0.42, 0.82)});
            // The seed opens one invitation to the fourth body. The remaining
            // pairs begin closed, so the musician can explicitly form a
            // square, chain or star without replacing the territory.
            constellation.addRelation({2U, 4U, RelationKind::Contagion, RelationDomain::Time,
                                       0.0, 12.0, 1.4, 0.78});
            constellation.addRelation({3U, 4U, RelationKind::Shadow, RelationDomain::Memory,
                                       0.0, 18.0, 2.0, 0.74});
        }
        return document;
    }

    // Compatibility name retained for existing sessions and tests. The
    // optional body may be Walsh or Hard Sync depending on the seed.
    static PerformanceSessionDocument makeWithWalsh(std::uint32_t seed,
                                                    bool playing = false) noexcept {
        return makeWithOptionalModule(seed, playing);
    }

    // The startup/seed path must construct only the stems that were released
    // before the territory is queued.  The performer therefore never sees an
    // eight-body opening when the current host may sound only four bodies.
    static PerformanceSessionDocument makeForReleasedStems(
        std::uint32_t seed, std::size_t releasedStemCount, bool playing = false) noexcept {
        const auto distribution = PerformanceStemDistributor::make(seed, releasedStemCount);
        auto document = make(seed, playing);
        if (distribution.audibleStemCount >= 4U)
            document = makeWithOptionalModule(seed, playing);
        if (distribution.audibleStemCount >= 5U)
            document = makeWithModalReserve(seed, playing);
        // Com os 8 stems liberados (2026-10-28), todas as estrelas do
        // asterismo estão no céu desde a abertura, com a presença da seed. Quem
        // abre aceso ou apagado é a variação do mixer (PerformanceSkyDirector):
        // a seed escolhe a figura, e o autor pediu que os corpos 6–8 entrem
        // nessa escolha em vez de começarem sempre zerados.
        if (distribution.audibleStemCount >= 8U)
            document = makeExpandedEnsemble(seed, playing);
        seedPhenomena(document, seed);
        return document;
    }

    // A seed escolhe os fenômenos (5/11, auditoria da seed, item B). Antes,
    // todo cabo nascia ATRAIR, sem fenômeno ligado, e cada cena tinha o seu
    // fenômeno de população fixo (ESPIRAL girar, RASGO fragmentar, BALANÇO
    // alinhar). Agora, por cena: cada cabo tem uma chance de nascer com um
    // dos 23 fenômenos relacionais (menor na abertura, que reúne devagar), e
    // as cenas 2–4 tiram o fenômeno de população entre os 10. O músico
    // continua trocando ou desligando qualquer um.
    static void seedPhenomena(PerformanceSessionDocument& document, std::uint32_t seed) noexcept {
        constexpr auto relational = static_cast<std::uint32_t>(PerformanceRelationalPhenomenon::Count);
        constexpr auto population = static_cast<std::uint32_t>(PerformancePopulationPhenomenon::Count);
        for (std::size_t sceneIndex = 0U; sceneIndex < document.sceneCount; ++sceneIndex) {
            auto& scene = document.scenes[sceneIndex];
            const double chance = sceneIndex == 0U ? 0.35 : 0.6;
            for (std::size_t index = 0U; index < scene.constellation.relationCount; ++index) {
                auto& relation = scene.constellation.relations[index];
                const auto salt = 0xB000U + static_cast<std::uint32_t>(sceneIndex * 64U + index * 2U);
                if (unit(seed, salt) >= chance) continue;
                relation.phenomenon = static_cast<PerformanceRelationalPhenomenon>(
                    mix((static_cast<std::uint64_t>(seed) << 32U) | (salt + 1U)) % relational);
                relation.phenomenonEnabled = true;
            }
            if (sceneIndex > 0U && scene.session.populationPhenomenonEnabled)
                scene.session.populationPhenomenon = static_cast<PerformancePopulationPhenomenon>(
                    mix((static_cast<std::uint64_t>(seed) << 32U) | (0xB800U + sceneIndex)) % population);
        }
    }

    // The ModalResonator is the next audited matter. This function declares
    // its fifth structural identity only after the seed has prepared the four
    // currently safe bodies. The audited fifth-stem benchmark now admits it as
    // an emerging, audible memory body rather than a dormant reserve.
    static PerformanceSessionDocument makeWithModalReserve(std::uint32_t seed,
                                                            bool playing = false) noexcept {
        auto document = makeWithOptionalModule(seed, playing);
        if (document.territory.actorCount != 4U) return document;
        document.territory.addActor({5U, PerformanceFamily::Tone, 0x11U,
                                     roleBit(PerformanceRole::Chao)
                                         | roleBit(PerformanceRole::Vestigio),
                                     between(seed, 81U, 0.20, 0.52), 0.0, 0.88});
        for (std::size_t index = 0U; index < document.sceneCount; ++index) {
            auto& constellation = document.scenes[index].constellation;
            constellation.addActor({5U, PerformanceRole::Vestigio, ActorLifeState::Emerging,
                                    between(seed, 82U + static_cast<std::uint32_t>(index), 0.42, 0.60),
                                    between(seed, 83U + static_cast<std::uint32_t>(index), 0.30, 0.60),
                                    0.0, 0.48});
            constellation.addRelation({1U, 5U, RelationKind::Shadow, RelationDomain::Memory,
                                       0.54, 24.0, 2.8, 0.70});
        }
        return document;
    }

    // The expanded ensemble is a seed-selected opening constellation, not a
    // palette of silent widgets. Every actor enters the ecology with a role,
    // presence and relation, while the performer remains free to reshape the
    // field during the piece.
    static PerformanceSessionDocument makeExpandedEnsemble(std::uint32_t seed,
                                                            bool playing = false) noexcept {
        auto document = makeWithModalReserve(seed, playing);
        if (document.territory.actorCount != 5U) return document;
        document.territory.addActor({6U, PerformanceFamily::Tone, 0x11U,
                                     roleBit(PerformanceRole::Chao) | roleBit(PerformanceRole::Corpo),
                                     between(seed, 91U, 0.38, 0.66), 0.0, 0.90});
        document.territory.addActor({7U, PerformanceFamily::Rupture, 0x44U,
                                     roleBit(PerformanceRole::Rasgo) | roleBit(PerformanceRole::Atrito),
                                     between(seed, 92U, 0.34, 0.62), 0.0, 0.88});
        document.territory.addActor({8U, PerformanceFamily::Texture, 0x22U,
                                     roleBit(PerformanceRole::Atrito) | roleBit(PerformanceRole::Vestigio),
                                     between(seed, 93U, 0.32, 0.60), 0.0, 0.86});
        for (std::size_t index = 0U; index < document.sceneCount; ++index) {
            auto& constellation = document.scenes[index].constellation;
            constellation.addActor({6U, PerformanceRole::Corpo, ActorLifeState::Emerging,
                                    between(seed, 94U + static_cast<std::uint32_t>(index), 0.42, 0.62),
                                    between(seed, 95U + static_cast<std::uint32_t>(index), 0.30, 0.62), 0.0, 0.52});
            constellation.addActor({7U, PerformanceRole::Rasgo, ActorLifeState::Emerging,
                                    between(seed, 96U + static_cast<std::uint32_t>(index), 0.38, 0.58),
                                    between(seed, 97U + static_cast<std::uint32_t>(index), 0.28, 0.60), 0.0, 0.50});
            constellation.addActor({8U, PerformanceRole::Atrito, ActorLifeState::Emerging,
                                    between(seed, 98U + static_cast<std::uint32_t>(index), 0.34, 0.56),
                                    between(seed, 99U + static_cast<std::uint32_t>(index), 0.30, 0.58), 0.0, 0.50});
            constellation.addRelation({5U, 6U, RelationKind::Support, RelationDomain::Matter, 0.46, 18.0, 2.2, 0.62});
            constellation.addRelation({6U, 7U, RelationKind::Friction, RelationDomain::Time, 0.50, 16.0, 2.4, 0.68});
            constellation.addRelation({7U, 8U, RelationKind::Infiltration, RelationDomain::Space, 0.48, 20.0, 2.6, 0.66});
            // Additional dormant diagonals keep the expanded constellation
            // open to performer-authored triangles and closed polygons.
            // They are declared in the contract but begin at zero intensity.
            constellation.addRelation({1U, 6U, RelationKind::Orbit, RelationDomain::Space, 0.0, 18.0, 1.0, 0.60});
            constellation.addRelation({2U, 5U, RelationKind::Contagion, RelationDomain::Matter, 0.0, 16.0, 1.0, 0.60});
            constellation.addRelation({2U, 6U, RelationKind::Friction, RelationDomain::Time, 0.0, 16.0, 1.0, 0.60});
            constellation.addRelation({3U, 7U, RelationKind::Shadow, RelationDomain::Memory, 0.0, 20.0, 1.0, 0.60});
            constellation.addRelation({4U, 8U, RelationKind::Infiltration, RelationDomain::Space, 0.0, 20.0, 1.0, 0.60});
            constellation.addRelation({5U, 7U, RelationKind::Support, RelationDomain::Matter, 0.0, 18.0, 1.0, 0.60});
        }
        return document;
    }

private:
    static std::uint64_t mix(std::uint64_t value) noexcept {
        value += 0x9e3779b97f4a7c15ULL;
        value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
        return value ^ (value >> 31U);
    }

    static double unit(std::uint32_t seed, std::uint32_t salt) noexcept {
        const auto value = mix((static_cast<std::uint64_t>(seed) << 32U) | salt);
        return static_cast<double>(value >> 11U) * (1.0 / 9007199254740991.0);
    }

    static double between(std::uint32_t seed, std::uint32_t salt,
                          double minimum, double maximum) noexcept {
        return minimum + (maximum - minimum) * unit(seed, salt);
    }

    // Standalone fallback for a field created directly in the Performance.
    // An imported RSTM 2 plan replaces this snapshot with the exact Studio
    // values; the fallback keeps the same bounded vocabulary and stable policy.
    static PerformanceGamePlan nativeGamePlan(std::uint32_t seed) noexcept {
        PerformanceGamePlan plan;
        plan.strategy = static_cast<PerformanceGameStrategy>(
            mix((static_cast<std::uint64_t>(seed) << 32U) | 125U) % 6U);
        const auto ruleRoll = unit(seed, 126U);
        switch (plan.strategy) {
            case PerformanceGameStrategy::Follow:
                plan.rule = ruleRoll < 0.68 ? PerformanceGameRule::Preserve
                                            : PerformanceGameRule::Exchange;
                break;
            case PerformanceGameStrategy::Transform:
                plan.rule = ruleRoll < 0.72 ? PerformanceGameRule::Mutate
                                            : PerformanceGameRule::Exchange;
                break;
            case PerformanceGameStrategy::Counter:
                plan.rule = ruleRoll < 0.70 ? PerformanceGameRule::Oppose
                                            : PerformanceGameRule::Exchange;
                break;
            case PerformanceGameStrategy::Interrupt:
                plan.rule = ruleRoll < 0.74 ? PerformanceGameRule::Break
                                            : PerformanceGameRule::Withhold;
                break;
            case PerformanceGameStrategy::Conceal:
                plan.rule = ruleRoll < 0.72 ? PerformanceGameRule::Withhold
                                            : PerformanceGameRule::Mutate;
                break;
            case PerformanceGameStrategy::Negotiate:
                plan.rule = ruleRoll < 0.70 ? PerformanceGameRule::Exchange
                                            : PerformanceGameRule::Preserve;
                break;
            case PerformanceGameStrategy::Count: break;
        }
        plan.energy = between(seed, 120U, 0.20, 0.82);
        plan.memory = between(seed, 121U, 0.18, 0.82);
        plan.risk = between(seed, 122U, 0.10, 0.85);
        plan.freedom = between(seed, 123U, 0.12, 0.84);
        plan.hiddenInformation = between(seed, 124U, 0.08, 0.78);
        return plan;
    }

    static PerformanceScene scene(std::uint32_t seed, std::uint16_t id,
                                  bool playing) noexcept {
        PerformanceScene out;
        out.id = id;
        out.session.territorySeed = seed;
        out.session.currentSceneId = id;
        out.session.targetSceneId = id;
        out.session.playing = playing;
        out.session.tempoBpm = between(seed, 14U, 96.0, 148.0);
        out.session.autonomy = between(seed, 15U, 0.35, 0.68);
        out.session.intervention = between(seed, 16U, 0.34, 0.70);
        out.session.density = between(seed, 17U, 0.28, 0.64);
        out.session.tension = between(seed, 18U, 0.22, 0.56);
        out.session.memory = between(seed, 19U, 0.42, 0.78);
        out.session.risk = between(seed, 20U, 0.10, 0.44);
        out.session.space = between(seed, 55U, 0.18, 0.62);
        out.session.depth = between(seed, 125U, 0.18, 0.72);
        out.session.outputTrimDb = -6.0;
        return out;
    }

    static void configureScene(PerformanceScene& scene, std::uint16_t id,
                               std::uint32_t seed, std::uint32_t salt,
                               bool playing) noexcept {
        scene.id = id;
        scene.session.currentSceneId = id;
        scene.session.targetSceneId = id;
        scene.session.playing = playing;
        scene.session.density = between(seed, salt, 0.48, 0.84);
        scene.session.tension = between(seed, salt + 1U, 0.38, 0.88);
        scene.session.memory = between(seed, salt + 2U, 0.42, 0.86);
        scene.session.risk = between(seed, salt + 3U, 0.24, 0.90);
        scene.session.space = between(seed, salt + 4U, 0.26, 0.86);
        scene.session.depth = between(seed, salt + 5U, 0.18, 0.82);
    }

    static void applyTerritoryLimits(PerformanceSessionDocument& document) noexcept {
        for (std::size_t index = 0U; index < document.sceneCount; ++index) {
            auto& session = document.scenes[index].session;
            session.memory = std::min(session.memory, document.territory.identity.memoryLimit);
            session.risk = std::min(session.risk, document.territory.identity.riskLimit);
        }
    }
};

} // namespace rasgo::performance
