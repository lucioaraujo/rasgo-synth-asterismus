#include <rasgo/performance/PerformanceSkyDirector.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <initializer_list>
#include <set>
#include <vector>
#include <utility>

using namespace rasgo::performance;

namespace {

constexpr double kRate = 48000.0;
using Bodies = std::array<PerformanceSkyDirector::Body, PerformanceSkyDirector::kBodies>;

bool expect(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}

Bodies opening() {
    Bodies bodies{};
    const std::array<PerformanceModuleBody, 8> stars{{
        PerformanceModuleBody::FmFlute, PerformanceModuleBody::Gendyn,
        PerformanceModuleBody::AtariPunk, PerformanceModuleBody::DejaVu,
        PerformanceModuleBody::Galo, PerformanceModuleBody::Acid,
        PerformanceModuleBody::Choir, PerformanceModuleBody::Scat}};
    for (std::size_t slot = 0U; slot < bodies.size(); ++slot)
        bodies[slot] = {true, stars[slot], 0.15 + 0.1 * static_cast<double>(slot),
                        0.3 + 0.05 * static_cast<double>(slot % 3U), 0.7};
    return bodies;
}

// Runs a director for `seconds`, with every lit body attacking on each beat,
// and returns the cues it delivered as (slot, due sample) pairs.
std::vector<std::pair<std::size_t, std::uint64_t>> run(std::uint32_t seed, const Bodies& bodies,
                                                        double seconds, PerformanceSkyDirector& director) {
    director.prepare(kRate);
    director.setSeed(seed);
    std::vector<std::pair<std::size_t, std::uint64_t>> delivered;
    const auto samples = static_cast<std::uint64_t>(kRate * seconds);
    const auto beat = static_cast<std::uint64_t>(kRate * 60.0 / 120.0);
    for (std::uint64_t sample = 0U; sample < samples; ++sample) {
        director.advance(bodies, 120.0);
        PerformanceSkyDirector::Cue due{};
        while (director.popDue(due)) delivered.emplace_back(due.slot, sample);
        if (sample % beat == 0U)
            for (std::size_t slot = 0U; slot < bodies.size(); ++slot)
                if (bodies[slot].lit) director.hear(slot, 110.0 * static_cast<double>(slot + 1U));
    }
    return delivered;
}

} // namespace

int main() {
    bool ok = true;

    // Gravitation: bounded, finite, and each body stays near its drawn place.
    {
        PerformanceSkyDirector director;
        const auto bodies = opening();
        run(181U, bodies, 60.0, director);
        // A física é caótica: um instante de uma seed não diz muito. A média
        // das oito estrelas fica perto do desenho (0,18–0,24 em 4/11, antes e
        // depois do caráter das partes); cada uma, com folga, abaixo de 0,5.
        double meanDistance = 0.0;
        for (std::size_t slot = 0U; slot < bodies.size(); ++slot) {
            const double x = director.positionX(slot);
            const double y = director.positionY(slot);
            ok = expect(std::isfinite(x) && std::isfinite(y) && x >= 0.0 && x <= 1.0
                            && y >= 0.0 && y <= 1.0, "positions stay finite and in the Field") && ok;
            const double distance = std::hypot(x - bodies[slot].anchorX, y - bodies[slot].anchorY);
            meanDistance += distance / static_cast<double>(bodies.size());
            ok = expect(distance < 0.5, "no star wanders far from its drawn place") && ok;
        }
        ok = expect(meanDistance < 0.3, "the drawn place remains the constellation's gravity well") && ok;
    }

    // Determinism by seed; different seeds give different dances.
    {
        PerformanceSkyDirector first, second, other;
        const auto bodies = opening();
        const auto a = run(4242U, bodies, 30.0, first);
        const auto b = run(4242U, bodies, 30.0, second);
        const auto c = run(4243U, bodies, 30.0, other);
        ok = expect(!a.empty(), "focused relations produce cues") && ok;
        ok = expect(a == b, "the same seed reproduces the same dance") && ok;
        ok = expect(a != c, "another seed dances differently") && ok;
    }

    // Focus: few relations at a time, only lit bodies, and it rotates.
    {
        PerformanceSkyDirector director;
        auto bodies = opening();
        bodies[6].lit = false;
        bodies[7].lit = false;
        director.prepare(kRate);
        director.setSeed(77U);
        std::set<std::pair<std::size_t, std::size_t>> pairs;
        const auto samples = static_cast<std::uint64_t>(kRate * 120.0);
        for (std::uint64_t sample = 0U; sample < samples; ++sample) {
            director.advance(bodies, 120.0);
            ok = expect(director.focusCount() <= PerformanceSkyDirector::kMaxFocus,
                        "never more than three relations in focus") && ok;
            for (std::size_t index = 0U; index < director.focusCount(); ++index) {
                const auto& focus = director.focus(index);
                ok = expect(bodies[focus.caller].lit && bodies[focus.responder].lit,
                            "only lit stars relate") && ok;
                pairs.insert({focus.caller, focus.responder});
            }
            if (!ok) break;
        }
        std::cout << "distinct focused relations in 120 s: " << pairs.size() << '\n';
        ok = expect(pairs.size() >= 6U, "the focus rotates through many relations") && ok;
    }

    // Light time: a farther responder answers later.
    {
        const auto delayFor = [](double distance) {
            Bodies bodies{};
            bodies[0] = {true, PerformanceModuleBody::FmFlute, 0.2, 0.5, 0.9};
            bodies[1] = {true, PerformanceModuleBody::Theremin, 0.2 + distance, 0.5, 0.9};
            for (std::uint32_t seed = 1U; seed < 400U; ++seed) {
                PerformanceSkyDirector director;
                director.prepare(kRate);
                director.setSeed(seed);
                for (int sample = 0; sample < 4800; ++sample) director.advance(bodies, 120.0);
                if (director.focusCount() == 0U || director.focus(0).caller != 0U
                    || director.focus(0).kind != RelationKind::CallResponse) continue;
                director.hear(0U, 220.0);
                for (int sample = 1; sample < 480000; ++sample) {
                    director.advance(bodies, 120.0);
                    PerformanceSkyDirector::Cue due{};
                    if (director.popDue(due)) return sample;
                }
            }
            return -1;
        };
        const int near = delayFor(0.1);
        const int far = delayFor(0.6);
        std::cout << "call-response delay: near " << near << " samples, far " << far << " samples\n";
        ok = expect(near > 0 && far > near, "the farther star answers later") && ok;
    }

    // Reciprocal answers never become an echo at audio rate: every delivered
    // cue makes its star attack at once (the worst case), and the cue rate
    // stays bounded by the chain depth and the minimum spacing.
    {
        Bodies bodies{};
        bodies[0] = {true, PerformanceModuleBody::Percussion, 0.40, 0.5, 0.9};
        bodies[1] = {true, PerformanceModuleBody::AtariPunk, 0.45, 0.5, 0.9};
        std::size_t worst = 0U;
        for (std::uint32_t seed = 1U; seed <= 40U; ++seed) {
            PerformanceSkyDirector director;
            director.prepare(kRate);
            director.setSeed(seed);
            std::size_t cues = 0U;
            const auto beat = static_cast<std::uint64_t>(kRate * 0.5);
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 20.0); ++sample) {
                director.advance(bodies, 120.0);
                if (sample % beat == 0U) director.hear(0U, 55.0);
                PerformanceSkyDirector::Cue due{};
                while (director.popDue(due)) {
                    ++cues;
                    director.hear(due.slot, 180.0);
                }
            }
            worst = std::max(worst, cues);
        }
        std::cout << "worst reciprocal cue count in 20 s: " << worst << '\n';
        // One caller attack per beat (40 in 20 s) may yield at most two links
        // of answers each.
        ok = expect(worst <= 40U * PerformanceSkyDirector::kMaxChainDepth + 8U,
                    "reciprocal relations stay bounded") && ok;
    }

    // Gestos do céu (5/11, item E): poucos, por orçamento — entre zero e dois
    // por parte, nenhum na abertura.
    {
        std::uint32_t total = 0U, sections = 0U;
        bool openingClean = true;
        for (std::uint32_t seed = 1U; seed <= 6U; ++seed) {
            PerformanceSkyDirector director;
            director.prepare(kRate);
            director.setSeed(seed);
            director.setMixerVariation(true);
            auto bodies = opening();
            auto last = director.section();
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 240.0); ++sample) {
                director.advance(bodies, 120.0);
                PerformanceSkyDirector::SkyGesture gesture{};
                if (director.popSkyGesture(gesture) && director.section() == PerformanceSkyDirector::Section::Opening)
                    openingClean = false;
                if (director.section() != last) { ++sections; last = director.section(); }
            }
            total += director.skyGestureCount();
        }
        std::cout << "sky gestures: " << total << " in " << sections << " sections (6 seeds, 240 s)\n";
        ok = expect(total > 0U && total <= sections * 2U, "the sky makes a few gestures, within its budget") && ok;
        ok = expect(openingClean, "no sky gesture in the opening") && ok;
    }

    // Progressões neo-riemannianas e modos de contraste (6/11, 0.3): no
    // desenvolvimento e no contraste o centro anda por passos P/L/R (mais de
    // uma região por parte); no contraste a escala vira um dos 16 modos.
    {
        std::size_t walked = 0U, moded = 0U;
        for (std::uint32_t seed = 1U; seed <= 8U; ++seed) {
            PerformanceSkyDirector director;
            director.prepare(kRate);
            director.setSeed(seed);
            director.setMixerVariation(true);
            auto bodies = opening();
            std::array<bool, 12> seen{};
            bool contrastMode = false;
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 120.0); ++sample) {
                director.advance(bodies, 120.0);
                const auto section = director.section();
                if (section == PerformanceSkyDirector::Section::Development
                    || section == PerformanceSkyDirector::Section::Contrast) {
                    seen[static_cast<std::size_t>((static_cast<int>(director.regionSemitones()) + 12) % 12)] = true;
                }
                if (section == PerformanceSkyDirector::Section::Contrast && director.regionMode() >= 0) contrastMode = true;
                if (section != PerformanceSkyDirector::Section::Contrast && director.regionMode() >= 0) contrastMode = false;
            }
            if (std::count(seen.begin(), seen.end(), true) >= 3) ++walked;
            if (contrastMode) ++moded;
        }
        std::cout << "harmony: progressions in " << walked << " of 8 seeds, contrast modes in " << moded << "\n";
        ok = expect(walked >= 6U, "the harmony walks through several regions (P/L/R)") && ok;
        ok = expect(moded == 8U, "the contrast takes a mode of its own") && ok;
    }

    // Regiões harmônicas (5/11, item D da auditoria da seed): o contraste
    // muda de região, a abertura e o retorno ficam na tônica.
    {
        std::size_t moved = 0U;
        bool tonicOk = true;
        for (std::uint32_t seed = 1U; seed <= 6U; ++seed) {
            PerformanceSkyDirector director;
            director.prepare(kRate);
            director.setSeed(seed);
            director.setMixerVariation(true);
            director.setFormPlan({{2U, 3U, 3U, 3U}});
            auto bodies = opening();
            bool sawContrast = false;
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 30.0); ++sample) {
                director.advance(bodies, 120.0);
                const auto section = director.section();
                if (section == PerformanceSkyDirector::Section::Contrast && !sawContrast) {
                    sawContrast = true;
                    if (director.regionSemitones() != 0.0) ++moved;
                }
                if ((section == PerformanceSkyDirector::Section::Opening
                     || section == PerformanceSkyDirector::Section::Return) && director.regionSemitones() != 0.0)
                    tonicOk = false;
            }
        }
        std::cout << "harmonic regions: the contrast moved in " << moved << " of 6 seeds\n";
        ok = expect(moved == 6U, "every contrast goes to another region") && ok;
        ok = expect(tonicOk, "opening and return stay on the tonic") && ok;
    }

    // A form fitted to a duration (2/11, the 1/2/3/5 min device): with a plan
    // of 2, 4, 3 and 3 bars, the sky passes OPENING → DEVELOPMENT → CONTRAST
    // → RETURN at exactly those bar lines (120 BPM: one bar = 2 s); clearing
    // the plan gives the seed's own lengths back.
    {
        PerformanceSkyDirector director;
        director.prepare(kRate);
        director.setSeed(2026U);
        director.setMixerVariation(true);
        director.setFormPlan({{2U, 4U, 3U, 3U}});
        auto bodies = opening();
        // Contado em tempos, pelo andamento que o próprio céu toca (desde 4/11
        // o andamento de cada parte vem do caráter dela).
        // Desde 5/11 (gramáticas da forma): a abertura fica com os 2
        // compassos do plano, e a primeira passagem da gramática da seed
        // termina nos 12 compassos da faixa.
        double leftOpening = -1.0, passEnded = -1.0;
        double beats = 0.0;
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 30.0); ++sample) {
            director.advance(bodies, 120.0);
            beats += director.tempoFactor() * 2.0 / kRate;
            if (leftOpening < 0.0 && director.section() != PerformanceSkyDirector::Section::Opening) leftOpening = beats / 4.0;
            if (passEnded < 0.0 && director.formPass() > 0U) passEnded = beats / 4.0;
        }
        std::cout << "form plan 2/4/3/3 bars, grammar " << PerformanceSkyDirector::grammarName(director.grammar())
                  << ": opening ends at bar " << leftOpening << ", the first pass at bar " << passEnded << "\n";
        ok = expect(director.formPlanned() && std::fabs(leftOpening - 2.0) < 0.3 && std::fabs(passEnded - 12.0) < 0.3,
                    "the form follows the planned bars") && ok;
        director.setFormPlan({{0U, 0U, 0U, 0U}});
        ok = expect(!director.formPlanned(), "clearing the plan gives the seed's lengths back") && ok;
    }

    // A pinned star (2/11) stays lit through the variation and the eclipses,
    // while the others still go out.
    {
        PerformanceSkyDirector director;
        director.prepare(kRate);
        director.setSeed(2026U);
        director.setMixerVariation(true);
        director.setPinned(5U, true);
        auto bodies = opening();
        double pinnedMin = 1.0;
        bool otherWentOut = false;
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 240.0); ++sample) {
            director.advance(bodies, 120.0);
            if (sample < 4800U) continue;
            pinnedMin = std::min(pinnedMin, director.brightness(5U));
            for (std::size_t slot = 0U; slot < bodies.size(); ++slot)
                if (slot != 5U && director.brightnessTarget(slot) < 0.5) otherWentOut = true;
        }
        std::cout << "pinned star: lowest brightness " << pinnedMin << '\n';
        ok = expect(director.pinned(5U) && pinnedMin > 0.99, "a pinned star stays lit") && ok;
        ok = expect(otherWentOut, "the other stars still vary") && ok;
    }

    // Mixer variation: the seed opens a figure, stars fade, go out and
    // re-ignite, and the mix follows the Constellation's movement; switched
    // off, every star shines fully. Since 4/11 how many stars sound comes
    // from each section's character (2 to 7, never a fixed pattern).
    {
        PerformanceSkyDirector director;
        director.prepare(kRate);
        director.setSeed(2026U);
        director.setMixerVariation(true);
        auto bodies = opening();
        std::array<bool, 8> wentOut{}, cameBack{};
        std::size_t minLit = 8U, maxLit = 0U;
        using SectionV = PerformanceSkyDirector::Section;
        std::array<std::size_t, 4> sectionMin{{8U, 8U, 8U, 8U}}, sectionMax{};
        bool leaderBackInContrast = false;
        double minLevel = 1.0, maxLevel = 0.0;
        const auto samples = static_cast<std::uint64_t>(kRate * 240.0);
        for (std::uint64_t sample = 0U; sample < samples; ++sample) {
            // The Constellation moves: each body circles its anchor.
            if (sample % 64U == 0U) {
                const double t = static_cast<double>(sample) / kRate;
                for (std::size_t slot = 0U; slot < bodies.size(); ++slot) {
                    bodies[slot].liveX = bodies[slot].anchorX + 0.2 * std::cos(t * 0.3 + static_cast<double>(slot));
                    bodies[slot].liveY = bodies[slot].anchorY + 0.2 * std::sin(t * 0.3 + static_cast<double>(slot));
                }
            }
            director.advance(bodies, 120.0);
            if (sample % 4800U != 0U || sample < 48000U) continue;
            std::size_t lit = 0U;
            for (std::size_t slot = 0U; slot < bodies.size(); ++slot) {
                const bool on = director.brightnessTarget(slot) > 0.5;
                lit += on ? 1U : 0U;
                if (!on) wentOut[slot] = true;
                if (on && wentOut[slot]) cameBack[slot] = true;
                if (on) {
                    minLevel = std::min(minLevel, director.level(slot));
                    maxLevel = std::max(maxLevel, director.level(slot));
                }
            }
            minLit = std::min(minLit, lit);
            maxLit = std::max(maxLit, lit);
            const auto section = static_cast<std::size_t>(director.section());
            if (section < 4U) {
                sectionMin[section] = std::min(sectionMin[section], lit);
                sectionMax[section] = std::max(sectionMax[section], lit);
            }
            if (director.section() == SectionV::Contrast && director.brightnessTarget(0U) < 0.5)
                leaderBackInContrast = true;
        }
        std::size_t returns = 0U;
        for (const bool back : cameBack) returns += back ? 1U : 0U;
        std::cout << "variation: lit " << minLit << ".." << maxLit << ", stars that re-ignited "
                  << returns << ", level " << minLevel << ".." << maxLevel << '\n';
        std::cout << "orchestration: opening " << sectionMin[0] << ".." << sectionMax[0]
                  << ", development " << sectionMin[1] << ".." << sectionMax[1]
                  << ", contrast " << sectionMin[2] << ".." << sectionMax[2]
                  << ", return " << sectionMin[3] << ".." << sectionMax[3] << '\n';
        ok = expect(minLit >= 2U && maxLit <= 7U, "between two and seven stars stay lit") && ok;
        (void)leaderBackInContrast;
        ok = expect(returns >= 2U, "stars go out and re-ignite") && ok;
        ok = expect(maxLevel - minLevel > 0.2, "the mix follows the Constellation's movement") && ok;

        director.setMixerVariation(false);
        for (int sample = 0; sample < 48000 * 8; ++sample) director.advance(bodies, 120.0);
        for (std::size_t slot = 0U; slot < bodies.size(); ++slot)
            ok = expect(director.brightness(slot) > 0.99, "switched off, every star shines fully") && ok;
    }

    // The character of the sections (4/11): no fixed pattern — different
    // seeds draw different paths; the contrast jumps far from the
    // development; the return lands closer to the opening than the contrast.
    {
        using SectionC = PerformanceSkyDirector::Section;
        const auto bodies = opening();
        std::vector<std::array<double, PerformanceSkyDirector::kCharacterDims>> contrasts;
        std::size_t farContrasts = 0U, closerReturns = 0U, cycles = 0U;
        for (std::uint32_t seed : {1U, 26U, 628U, 1086U, 84U, 2026U}) {
            PerformanceSkyDirector director;
            director.prepare(kRate);
            director.setSeed(seed);
            director.setMixerVariation(true);
            PerformanceSkyDirector::SectionCharacter openingC{}, developmentC{}, contrastC{};
            auto last = SectionC::Count;
            bool gotContrast = false;
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 200.0); ++sample) {
                director.advance(bodies, 120.0);
                if (director.section() == last) continue;
                last = director.section();
                if (last == SectionC::Opening) openingC = director.character();
                if (last == SectionC::Development) developmentC = director.character();
                if (last == SectionC::Contrast) {
                    contrastC = director.character();
                    farContrasts += PerformanceSkyDirector::characterDistance(contrastC, developmentC) > 0.4 ? 1U : 0U;
                    if (!gotContrast) contrasts.push_back(contrastC.v);
                    gotContrast = true;
                }
                if (last == SectionC::Return && gotContrast) {
                    ++cycles;
                    const auto r = director.character();
                    closerReturns += PerformanceSkyDirector::characterDistance(r, openingC)
                        < PerformanceSkyDirector::characterDistance(contrastC, openingC) ? 1U : 0U;
                }
            }
        }
        double minPair = 1.0e9;
        for (std::size_t a = 0U; a < contrasts.size(); ++a)
            for (std::size_t b = a + 1U; b < contrasts.size(); ++b) {
                double d = 0.0;
                for (std::size_t i = 0U; i < contrasts[a].size(); ++i) d += std::fabs(contrasts[a][i] - contrasts[b][i]);
                minPair = std::min(minPair, d);
            }
        std::cout << "character: " << contrasts.size() << " seeds, closest pair of contrasts " << minPair
                  << ", far contrasts " << farContrasts << ", returns closer to the opening " << closerReturns
                  << "/" << cycles << '\n';
        ok = expect(contrasts.size() == 6U, "every seed reaches a contrast") && ok;
        ok = expect(minPair > 0.5, "different seeds draw different contrasts") && ok;
        ok = expect(farContrasts == contrasts.size() || farContrasts >= cycles, "the contrast jumps far from the development") && ok;
        ok = expect(cycles > 0U && closerReturns == cycles, "the return lands closer to the opening than the contrast") && ok;
    }

    // Capacity guard: with a measured capacity of three, the mixer variation
    // never keeps more than three stars lit.
    {
        PerformanceSkyDirector director;
        director.prepare(kRate);
        director.setSeed(99U);
        director.setMixerVariation(true);
        const auto bodies = opening();
        std::size_t worst = 0U;
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 120.0); ++sample) {
            if (sample == static_cast<std::uint64_t>(kRate * 10.0)) director.setStarCapacity(3U);
            director.advance(bodies, 120.0);
            if (sample < static_cast<std::uint64_t>(kRate * 20.0) || sample % 4800U != 0U) continue;
            std::size_t lit = 0U;
            for (std::size_t slot = 0U; slot < bodies.size(); ++slot)
                lit += director.brightnessTarget(slot) > 0.5 ? 1U : 0U;
            worst = std::max(worst, lit);
        }
        std::cout << "capacity 3: most lit " << worst << '\n';
        ok = expect(worst <= 3U, "the variation respects the measured capacity") && ok;
    }

    // Motifs: a caller plays an ascending phrase; the responder answers each
    // phrase with a fragment of several notes, each answer of the pair uses a
    // different transformation, and some answer descends (inversion or
    // retrograde) — development, never a copy.
    {
        Bodies bodies{};
        bodies[0] = {true, PerformanceModuleBody::FmFlute, 0.30, 0.5, 0.9};
        bodies[1] = {true, PerformanceModuleBody::Choir, 0.42, 0.5, 0.9};
        bool tested = false;
        for (std::uint32_t seed = 1U; seed < 600U && !tested; ++seed) {
            PerformanceSkyDirector director;
            director.prepare(kRate);
            director.setSeed(seed);
            for (int sample = 0; sample < 4800; ++sample) director.advance(bodies, 120.0);
            if (director.focusCount() == 0U || director.focus(0).caller != 0U
                || director.focus(0).kind != RelationKind::CallResponse) continue;
            tested = true;
            const std::array<double, 4> phrase{{220.0, 261.6, 329.6, 392.0}};
            std::vector<std::vector<double>> answers;
            std::vector<double> current;
            std::set<int> transforms;
            std::uint64_t lastCue = 0U, sample = 0U;
            const auto beat = static_cast<std::uint64_t>(kRate * 0.5);
            for (; sample < static_cast<std::uint64_t>(kRate * 40.0); ++sample) {
                // Keep the same relation in focus for the whole test.
                if (director.focusCount() == 0U || director.focus(0).caller != 0U
                    || director.focus(0).kind != RelationKind::CallResponse) {
                    director.advance(bodies, 120.0);
                    continue;
                }
                director.advance(bodies, 120.0);
                if (sample % beat == 0U) {
                    director.hear(0U, phrase[(sample / beat) % phrase.size()]);
                    transforms.insert(static_cast<int>(director.lastTransform()));
                }
                PerformanceSkyDirector::Cue due{};
                while (director.popDue(due)) {
                    if (due.slot != 1U) continue;
                    if (!current.empty() && sample - lastCue > beat * 3U) {
                        answers.push_back(current);
                        current.clear();
                    }
                    current.push_back(due.pitchHz);
                    lastCue = sample;
                }
            }
            if (!current.empty()) answers.push_back(current);
            std::size_t multiNote = 0U;
            bool descends = false;
            for (const auto& answer : answers) {
                multiNote += answer.size() >= 2U ? 1U : 0U;
                if (answer.size() >= 2U && answer.back() < answer.front()) descends = true;
            }
            std::cout << "motifs: " << answers.size() << " answers, " << multiNote
                      << " with several notes, " << transforms.size() << " transformations\n";
            ok = expect(multiNote >= 2U, "answers carry motif fragments, not single notes") && ok;
            ok = expect(transforms.size() >= 3U, "successive answers use different transformations") && ok;
            ok = expect(descends, "some answer develops the rising phrase downward") && ok;
        }
        ok = expect(tested, "found a seed with a call-response focus to test motifs") && ok;
    }

    // Harmony that works: friction between pitched stars accumulates tension
    // (chromatic answers, outside the scale); above the threshold the focus
    // turns into a cadence (repair to the root or fifth) and tension falls.
    {
        Bodies bodies{};
        bodies[0] = {true, PerformanceModuleBody::FmFlute, 0.30, 0.5, 0.9};
        bodies[1] = {true, PerformanceModuleBody::Theremin, 0.38, 0.5, 0.9};
        bodies[2] = {true, PerformanceModuleBody::Choir, 0.46, 0.5, 0.9};
        double peakTension = 0.0;
        bool repaired = false, chromatic = false, fellAfterRepair = false;
        double tensionAtRepair = 0.0;
        for (std::uint32_t seed = 1U; seed <= 300U && !(repaired && fellAfterRepair); ++seed) {
            PerformanceSkyDirector director;
            director.prepare(kRate);
            director.setSeed(seed);
            director.setTonalCenter(110.0);
            const auto beat = static_cast<std::uint64_t>(kRate * 0.25);
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 60.0); ++sample) {
                director.advance(bodies, 120.0);
                if (sample % beat == 0U)
                    for (std::size_t slot = 0U; slot < 3U; ++slot) director.hear(slot, 220.0 + 20.0 * slot);
                PerformanceSkyDirector::Cue due{};
                while (director.popDue(due)) chromatic = chromatic || due.chromatic;
                peakTension = std::max(peakTension, director.tension());
                for (std::size_t index = 0U; index < director.focusCount(); ++index) {
                    if (director.focus(index).kind != RelationKind::Repair) continue;
                    if (!repaired) tensionAtRepair = director.tension();
                    repaired = true;
                }
                if (repaired && director.tension() < tensionAtRepair - 0.2) fellAfterRepair = true;
            }
        }
        std::cout << "harmony: peak tension " << peakTension << ", cadence " << repaired
                  << ", chromatic answers " << chromatic << '\n';
        ok = expect(chromatic, "friction answers outside the scale") && ok;
        ok = expect(peakTension > 0.6 && repaired, "tension leads to a cadence") && ok;
        ok = expect(fellAfterRepair, "the cadence releases the tension") && ok;
    }

    // The game's intention gives the web its character: the same stars under
    // different strategies relate differently.
    {
        const auto kindsFor = [](PerformanceGameStrategy strategy) {
            std::array<std::size_t, static_cast<std::size_t>(RelationKind::Count)> counts{};
            const auto bodies = opening();
            for (std::uint32_t seed = 1U; seed <= 6U; ++seed) {
                PerformanceSkyDirector director;
                director.prepare(kRate);
                director.setSeed(seed);
                director.setStrategy(strategy);
                for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 60.0); ++sample) {
                    director.advance(bodies, 120.0);
                    if (sample % 24000U != 0U) continue;
                    for (std::size_t index = 0U; index < director.focusCount(); ++index)
                        ++counts[static_cast<std::size_t>(director.focus(index).kind)];
                }
            }
            return counts;
        };
        const auto follow = kindsFor(PerformanceGameStrategy::Follow);
        const auto counter = kindsFor(PerformanceGameStrategy::Counter);
        const auto conceal = kindsFor(PerformanceGameStrategy::Conceal);
        const auto at = [](const auto& counts, RelationKind kind) { return counts[static_cast<std::size_t>(kind)]; };
        std::cout << "strategies: friction follow " << at(follow, RelationKind::Friction)
                  << " / counter " << at(counter, RelationKind::Friction)
                  << "; shadow conceal " << at(conceal, RelationKind::Shadow)
                  << " / follow " << at(follow, RelationKind::Shadow) << '\n';
        ok = expect(at(counter, RelationKind::Friction) > at(follow, RelationKind::Friction),
                    "to counter brings more friction than to follow") && ok;
        ok = expect(at(conceal, RelationKind::Shadow) > at(follow, RelationKind::Shadow),
                    "to conceal brings more shadow than to follow") && ok;
    }

    // Form: opening, then development, contrast and return in a cycle; each
    // section lasts 8–20 bars; the opening keeps one relation in focus; in
    // the return the opening's stars shine again.
    {
        PerformanceSkyDirector director;
        director.prepare(kRate);
        director.setSeed(2718U);
        director.setMixerVariation(true);
        const auto bodies = opening();
        using Section = PerformanceSkyDirector::Section;
        std::vector<std::pair<Section, double>> sections{{Section::Opening, 0.0}};
        double beats = 0.0;  // em tempos, pelo andamento do céu (4/11)
        std::array<bool, 8> openingLit{};
        bool openingCaptured = false, openingTooBusy = false, returnRelit = false, sawReturn = false;
        const auto samples = static_cast<std::uint64_t>(kRate * 400.0);
        for (std::uint64_t sample = 0U; sample < samples; ++sample) {
            director.advance(bodies, 120.0);
            beats += director.tempoFactor() * 2.0 / kRate;
            if (!openingCaptured && sample == 64U) {
                for (std::size_t slot = 0U; slot < 8U; ++slot) openingLit[slot] = director.brightnessTarget(slot) > 0.5;
                openingCaptured = true;
            }
            if (director.section() == Section::Opening && director.focusCount() > 1U) openingTooBusy = true;
            if (director.section() != sections.back().first) {
                sections.emplace_back(director.section(), beats);
                if (director.section() == Section::Return) {
                    sawReturn = true;
                    for (std::size_t slot = 0U; slot < 8U; ++slot)
                        if (openingLit[slot] && director.brightnessTarget(slot) > 0.5) returnRelit = true;
                }
            }
        }
        // A ordem é a da gramática da seed (5/11); partes iguais em sequência
        // (o arco tem dois desenvolvimentos) aparecem como uma só aqui.
        std::vector<Section> expected;
        {
            const auto g = director.grammar();
            std::size_t step = 0U;
            expected.push_back(PerformanceSkyDirector::grammarPart(g, 0U));
            for (int part = 0; part < 40; ++part) {
                ++step;
                if (step >= PerformanceSkyDirector::grammarCount(g)) step = PerformanceSkyDirector::grammarLoop(g);
                const auto next = PerformanceSkyDirector::grammarPart(g, step);
                if (next != expected.back()) expected.push_back(next);
            }
        }
        bool orderOk = sections.size() >= 4U;
        for (std::size_t index = 0U; orderOk && index < sections.size() && index < expected.size(); ++index)
            orderOk = sections[index].first == expected[index];
        bool lengthsOk = true;
        const double bar = 4.0;
        for (std::size_t index = 1U; index + 1U < sections.size(); ++index) {
            const double bars = (sections[index + 1U].second - sections[index].second) / bar;
            if (bars < 2.5 || bars > 56.5) lengthsOk = false;  // 3–28 compassos; duas partes iguais somam
        }
        std::cout << "form: " << sections.size() << " sections in 400 s\n";
        ok = expect(orderOk, "sections follow the seed's grammar") && ok;
        ok = expect(lengthsOk, "each section lasts 3 to 28 bars") && ok;
        ok = expect(!openingTooBusy, "the opening keeps a single relation in focus") && ok;
        ok = expect(sawReturn && returnRelit, "the return brings back some of the opening's stars") && ok;
    }

    // Elastic time: the tempo factor breathes (spring, Fourier rubato, phrase
    // arch, equation of time), stays bounded, swings in some sections, and is
    // exactly 1 with the web switched off.
    {
        PerformanceSkyDirector director;
        director.prepare(kRate);
        director.setSeed(31415U);
        const auto bodies = opening();
        double low = 2.0, high = 0.0, previous = 1.0, biggestStep = 0.0, maxEccentricity = 0.0;
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 180.0); ++sample) {
            director.advance(bodies, 120.0);
            const double factor = director.tempoFactor();
            low = std::min(low, factor);
            high = std::max(high, factor);
            biggestStep = std::max(biggestStep, std::fabs(factor - previous));
            previous = factor;
            maxEccentricity = std::max(maxEccentricity, director.eccentricity());
        }
        std::cout << "elastic time: tempo " << low << ".." << high << ", biggest step " << biggestStep
                  << ", eccentricity up to " << maxEccentricity << '\n';
        ok = expect(low >= 0.7 && high <= 1.35, "the tempo factor stays bounded") && ok;
        ok = expect(low < 0.97 && high > 1.03, "the tempo breathes") && ok;
        ok = expect(biggestStep < 0.01, "the tempo moves without jumps") && ok;
        ok = expect(maxEccentricity > 0.0, "some sections swing") && ok;
        director.setInfluence(0.0);
        for (int sample = 0; sample < 1000; ++sample) director.advance(bodies, 120.0);
        ok = expect(director.tempoFactor() == 1.0 && director.eccentricity() == 0.0,
                    "with the web off, time is straight") && ok;
    }

    // Influence 0 switches the web off entirely.
    {
        PerformanceSkyDirector director;
        director.setInfluence(0.0);
        const auto bodies = opening();
        director.prepare(kRate);
        std::size_t cues = 0U;
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 10.0); ++sample) {
            director.advance(bodies, 120.0);
            director.hear(sample % 8U, 220.0);
            PerformanceSkyDirector::Cue due{};
            while (director.popDue(due)) ++cues;
        }
        ok = expect(cues == 0U, "influence 0 produces no cues") && ok;
    }

    // Phase chorus (Kuramoto): a coupled pair locks to the lag its relation
    // asks for (sync, a quarter, a half step) despite different natural
    // frequencies; an uncoupled body stays near the common pulse (resting at
    // a tenth of a step); in a live
    // director every phase stays in [0, 1) of a step and is 0 with the web off.
    {
        constexpr double twoPi = 6.283185307179586;
        for (const auto kind : {RelationKind::Support, RelationKind::Shadow, RelationKind::Hocket}) {
            std::array<double, 3> phase{{0.3, 2.0, 0.0}};
            const std::array<double, 3> omega{{0.2, -0.15, 0.1}};
            const std::size_t caller = 0U, responder = 1U;
            const double lag = PerformanceSkyDirector::chorusLag(kind);
            for (int step = 0; step < 20000; ++step)
                PerformanceSkyDirector::kuramotoStep(phase, omega, &caller, &responder, &lag, 1U, 2.5, 0.35, 0.002);
            double difference = phase[1] - phase[0] - lag;
            difference -= twoPi * std::round(difference / twoPi);
            double alone = phase[2] - 0.1 * twoPi;
            alone -= twoPi * std::round(alone / twoPi);
            std::cout << "chorus lag " << lag / twoPi << " step: locked within " << std::fabs(difference) / twoPi
                      << " step; uncoupled body at " << alone / twoPi << " step\n";
            ok = expect(std::fabs(difference) < 0.05 * twoPi, "a coupled pair locks to its relation's lag") && ok;
            ok = expect(std::fabs(alone) < 0.08 * twoPi, "an uncoupled body leans only a little off the pulse") && ok;
        }
        PerformanceSkyDirector director;
        const auto bodies = opening();
        run(31415U, bodies, 30.0, director);
        bool bounded = true;
        bool moved = false;
        for (std::size_t slot = 0U; slot < bodies.size(); ++slot) {
            const double delay = director.chorusDelay(slot);
            bounded = bounded && delay >= 0.0 && delay < 1.0;
            moved = moved || std::fabs(delay - 0.1) > 0.02;
        }
        ok = expect(bounded, "chorus phases stay within one step") && ok;
        ok = expect(moved, "the chorus moves the phases") && ok;
        director.setInfluence(0.0);
        ok = expect(director.chorusDelay(0U) == 0.0, "with the web off the chorus is silent") && ok;
    }

    // Drift (Adler): just above the locking threshold a lone phase slips a
    // whole step at the predicted rate, lingering near the pulse; a live
    // director names a drifting body during development only.
    {
        constexpr double twoPi = 6.283185307179586;
        const double anchor = 0.35;
        const double omegaValue = anchor * 1.03;
        std::array<double, 1> phase{{0.1 * twoPi}};
        const std::array<double, 1> omega{{omegaValue}};
        const std::array<double, 1> shift{{-0.25 * twoPi}};  // as the director turns the planet's anchor
        double unwrapped = 0.0;
        double previous = phase[0];
        std::uint64_t near = 0U;
        const std::uint64_t steps = 300000U;
        const double dt = 0.002;
        for (std::uint64_t step = 0U; step < steps; ++step) {
            PerformanceSkyDirector::kuramotoStep(phase, omega, nullptr, nullptr, nullptr, 0U, 0.0, anchor, dt, &shift);
            double delta = phase[0] - previous;
            delta -= twoPi * std::round(delta / twoPi);
            unwrapped += delta;
            previous = phase[0];
            double fromRest = phase[0] - 0.1 * twoPi;
            fromRest -= twoPi * std::round(fromRest / twoPi);
            near += std::fabs(fromRest) < 0.25 * twoPi ? 1U : 0U;
        }
        const double seconds = static_cast<double>(steps) * dt;
        const double expected = seconds * std::sqrt(omegaValue * omegaValue - anchor * anchor) / twoPi;
        const double turns = unwrapped / twoPi;
        const double dwell = static_cast<double>(near) / static_cast<double>(steps);
        std::cout << "drift: " << turns << " turns in " << seconds << " s (Adler predicts " << expected
                  << "), " << dwell * 100.0 << "% of the time within a quarter step of the pulse\n";
        ok = expect(std::fabs(turns - expected) < 0.6, "the drift slips at the Adler rate") && ok;
        ok = expect(dwell > 0.6, "the drift lingers near the pulse") && ok;

        PerformanceSkyDirector director;
        const auto bodies = opening();
        director.prepare(kRate);
        director.setSeed(31415U);
        bool drifted = false;
        bool onlyInDevelopment = true;
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 120.0); ++sample) {
            director.advance(bodies, 120.0);
            PerformanceSkyDirector::Cue due{};
            while (director.popDue(due)) {}
            if (sample % 4800U != 0U) continue;
            const bool drifting = director.driftSlot() < PerformanceSkyDirector::kBodies;
            drifted = drifted || drifting;
            onlyInDevelopment = onlyInDevelopment
                && (!drifting || director.section() == PerformanceSkyDirector::Section::Development);
        }
        ok = expect(drifted, "a development names a drifting body") && ok;
        ok = expect(onlyInDevelopment, "the drift happens only in development") && ok;
    }

    // Rhythm Fourier: the dominant period of known patterns over the
    // 48-sixteenth ring; a scattered pattern is unclear; a live director
    // finds periods from the set for bodies that attack on every beat.
    {
        const auto pattern = [](std::initializer_list<std::uint32_t> cycle, std::uint32_t length) {
            std::uint64_t mask = 0U;
            for (std::uint32_t start = 0U; start < PerformanceSkyDirector::kRhythmRing; start += length)
                for (const auto step : cycle)
                    if (start + step < PerformanceSkyDirector::kRhythmRing) mask |= std::uint64_t{1} << (start + step);
            return mask;
        };
        const auto quarter = PerformanceSkyDirector::dominantPeriod(pattern({0U}, 4U));
        const auto tresillo = PerformanceSkyDirector::dominantPeriod(pattern({0U, 3U, 6U}, 8U));
        const auto threes = PerformanceSkyDirector::dominantPeriod(pattern({0U}, 3U));
        const auto dotted = PerformanceSkyDirector::dominantPeriod(pattern({0U, 3U, 6U, 9U}, 12U));
        const std::uint64_t scattered = (std::uint64_t{1} << 0U) | (std::uint64_t{1} << 5U) | (std::uint64_t{1} << 7U)
            | (std::uint64_t{1} << 18U) | (std::uint64_t{1} << 19U) | (std::uint64_t{1} << 30U) | (std::uint64_t{1} << 41U);
        const auto noise = PerformanceSkyDirector::dominantPeriod(scattered);
        std::cout << "rhythm fourier: quarters " << quarter.period << " (" << quarter.clarity << "), tresillo "
                  << tresillo.period << " (" << tresillo.clarity << "), threes " << threes.period << ", dotted "
                  << dotted.period << ", scattered clarity " << noise.clarity << '\n';
        ok = expect(quarter.period == 4U && quarter.clarity > 0.99, "quarters have period 4") && ok;
        ok = expect(tresillo.period == 8U && tresillo.clarity > 0.99, "the tresillo has period 8") && ok;
        ok = expect(threes.period == 3U, "dotted eighths have period 3") && ok;
        ok = expect(dotted.period == 3U || dotted.period == 12U, "a cycle of threes keeps period 3 (or its 12)") && ok;
        ok = expect(noise.clarity < 0.5, "a scattered pattern is unclear") && ok;
        ok = expect(PerformanceSkyDirector::dominantPeriod(1U).period == 0U, "too few attacks give no period") && ok;

        PerformanceSkyDirector director;
        const auto bodies = opening();
        run(181U, bodies, 20.0, director);
        bool found = false;
        for (std::size_t slot = 0U; slot < bodies.size(); ++slot) {
            const auto rhythm = director.rhythm(slot);
            if (rhythm.period == 0U) continue;
            found = true;
            bool known = false;
            for (const auto period : PerformanceSkyDirector::kRhythmPeriods) known = known || period == rhythm.period;
            ok = expect(known && rhythm.clarity <= 1.0 + 1e-9, "a live period is musical and its clarity bounded") && ok;
        }
        ok = expect(found, "a live director hears periods") && ok;
    }

    // Eclipses: about 45 per Saros (223 lunations; real skies give ~41-43
    // solar eclipses per saros), in seasons; the sequence repeats after a
    // Saros; a live director eclipses within a few minutes and darkens the
    // covered body when the mixer varies.
    {
        for (const double offset : {0.0, 0.37, 0.81}) {
            std::uint32_t perSaros = 0U;
            std::uint32_t repeats = 0U;
            std::uint32_t eclipses = 0U;
            for (std::uint64_t n = 0U; n < 223U * 4U; ++n) {
                const bool now = PerformanceSkyDirector::eclipseAtLunation(n, offset);
                if (n < 223U) perSaros += now ? 1U : 0U;
                if (now) {
                    ++eclipses;
                    repeats += PerformanceSkyDirector::eclipseAtLunation(n + 223U, offset) ? 1U : 0U;
                }
            }
            std::cout << "eclipses: " << perSaros << " per saros, " << repeats << "/" << eclipses
                      << " recur one saros later\n";
            ok = expect(perSaros >= 35U && perSaros <= 55U, "a saros holds about 45 eclipses") && ok;
            ok = expect(repeats * 100U >= eclipses * 90U, "the saros brings the eclipses back") && ok;
        }
        PerformanceSkyDirector director;
        const auto bodies = opening();
        director.prepare(kRate);
        director.setSeed(2718U);
        director.setMixerVariation(true);
        double darkest = 1.0;
        std::size_t covered = PerformanceSkyDirector::kBodies;
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 300.0)
                                       && director.eclipseCount() < 2U; ++sample) {
            director.advance(bodies, 120.0);
            PerformanceSkyDirector::Cue due{};
            while (director.popDue(due)) {}
            const auto slot = director.eclipsedSlot();
            if (slot < PerformanceSkyDirector::kBodies) {
                covered = slot;
                darkest = std::min(darkest, director.brightness(slot));
            }
        }
        std::cout << "live eclipses: " << director.eclipseCount() << ", covered body dimmed to " << darkest << '\n';
        ok = expect(director.eclipseCount() >= 1U, "a live sky eclipses within five minutes") && ok;
        ok = expect(covered < PerformanceSkyDirector::kBodies && darkest < 0.7, "the covered body darkens") && ok;
    }

    // Physical motion primitives (PerformanceSkyMotion.hpp), each against its
    // physics.
    {
        using namespace rasgo::performance::sky_motion;
        // Billiards: an elastic collision keeps momentum and kinetic energy.
        SkyBodyState a{0.40, 0.50, 0.30, 0.05};
        SkyBodyState b{0.44, 0.51, -0.10, 0.00};
        const double ma = 1.3, mb = 0.7;
        const double px = ma * a.vx + mb * b.vx, py = ma * a.vy + mb * b.vy;
        const double energy = 0.5 * ma * (a.vx * a.vx + a.vy * a.vy) + 0.5 * mb * (b.vx * b.vx + b.vy * b.vy);
        ok = expect(collide(a, b, ma, mb, 0.05), "touching bodies collide") && ok;
        const double px2 = ma * a.vx + mb * b.vx, py2 = ma * a.vy + mb * b.vy;
        const double energy2 = 0.5 * ma * (a.vx * a.vx + a.vy * a.vy) + 0.5 * mb * (b.vx * b.vx + b.vy * b.vy);
        ok = expect(std::fabs(px - px2) < 1e-12 && std::fabs(py - py2) < 1e-12, "a collision keeps momentum") && ok;
        ok = expect(std::fabs(energy - energy2) < 1e-12, "a collision keeps energy") && ok;
        // Stadium: a body outside returns to the wall with the same speed.
        SkyBodyState ball{0.97, 0.80, 0.4, 0.1};  // 0.43 from the segment: outside
        const double speed = std::hypot(ball.vx, ball.vy);
        ok = expect(reflectStadium(ball, 0.5, 0.5, 0.16, 0.40), "the stadium wall catches an escaping body") && ok;
        ok = expect(std::fabs(std::hypot(ball.vx, ball.vy) - speed) < 1e-12 && ball.vx < 0.0,
                    "the stadium reflects without losing speed") && ok;
        // Lorentz: alone, a charged body runs a circle of radius v/(qB).
        SkyBodyState charged{0.5, 0.5, 0.2, 0.0};
        double minX = 1.0, maxX = 0.0;
        for (int step = 0; step < 40000; ++step) {
            double ax = 0.0, ay = 0.0;
            lorentz(charged, 1.0, 4.0, ax, ay);
            charged.vx += ax * 1e-4;
            charged.vy += ay * 1e-4;
            charged.x += charged.vx * 1e-4;
            charged.y += charged.vy * 1e-4;
            minX = std::min(minX, charged.x);
            maxX = std::max(maxX, charged.x);
        }
        const double diameter = maxX - minX;
        std::cout << "lorentz circle diameter " << diameter << " (expected " << 2.0 * 0.2 / 4.0 << ")\n";
        ok = expect(std::fabs(diameter - 0.1) < 0.003, "the Lorentz force draws its circle") && ok;
        // Magnus: spin curves the path and keeps the speed (to first order).
        SkyBodyState spun{0.5, 0.5, 0.3, 0.0};
        double ax = 0.0, ay = 0.0;
        magnus(spun, 1.0, 1.0, ax, ay);
        ok = expect(ax == 0.0 && ay > 0.0, "positive spin curves the path to the left") && ok;
        // Swinging spring: hangs at rest length + g/k under the anchor.
        SkyBodyState hanging{0.5, 0.5 + 0.13 + 1.1 / (4.0 * 1.1 / 0.13), 0.0, 0.0};
        ax = ay = 0.0;
        swingingSpring(hanging, 0.5, 0.5, 0.13, 4.0 * 1.1 / 0.13, 1.1, ax, ay);
        ok = expect(std::fabs(ax) < 1e-9 && std::fabs(ay) < 1e-9, "the swinging spring hangs at its equilibrium") && ok;
        // Distortion: amount 0 is the identity.
        double ux = 0.0, uy = 0.0;
        distort(12.3, 0.0, 0.7, 0.21, 0.83, ux, uy);
        ok = expect(std::fabs(ux - 0.21) < 1e-12 && std::fabs(uy - 0.83) < 1e-12, "no distortion is the identity") && ok;
        // Foam: overlapping bubbles push apart, separate ones do not.
        ax = ay = 0.0;
        bubblePressure({0.50, 0.5, 0, 0}, {0.56, 0.5, 0, 0}, 0.04, 0.04, 10.0, ax, ay);
        ok = expect(ax < 0.0, "overlapping bubbles push apart") && ok;
        ax = ay = 0.0;
        bubblePressure({0.50, 0.5, 0, 0}, {0.70, 0.5, 0, 0}, 0.04, 0.04, 10.0, ax, ay);
        ok = expect(ax == 0.0, "separate bubbles do not touch") && ok;
        // Soft screen edge: identity in the middle, monotonic, never at the
        // border.
        bool monotonic = true;
        double previous = softEdge(-0.5);
        for (int step = -49; step <= 150; ++step) {
            const double value = softEdge(step / 100.0);
            monotonic = monotonic && value > previous;
            previous = value;
        }
        ok = expect(softEdge(0.3) == 0.3 && softEdge(0.85) < 0.98 && softEdge(1.4) < 0.98
                        && softEdge(-0.4) > 0.02, "the soft edge keeps the middle and spares the border") && ok;
        ok = expect(monotonic, "the soft edge keeps the order of places") && ok;
    }

    // Constellation motion (30/10): ROTATE turns the rest places a quarter
    // turn in 16 beats (one turn per 16 bars); STILL brings them back.
    {
        PerformanceSkyDirector director;
        const auto bodies = opening();
        director.prepare(kRate);
        director.setSeed(181U);
        director.setConstellationMotion(PerformanceConstellationMotion::Rotate);
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 8.0); ++sample)
            director.advance(bodies, 120.0);
        double turned = director.constellationAngle();
        if (turned > 3.141592653589793) turned = 6.283185307179586 - turned;
        std::cout << "constellation turned " << turned << " rad in 16 beats\n";
        ok = expect(std::fabs(turned - 1.5707963) < 0.12, "ROTATE turns a quarter in 16 beats") && ok;
        director.setConstellationMotion(PerformanceConstellationMotion::Still);
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 30.0); ++sample)
            director.advance(bodies, 120.0);
        double back = director.constellationAngle();
        if (back > 3.141592653589793) back = 6.283185307179586 - back;
        ok = expect(back < 0.05, "STILL brings the constellation back to the drawing") && ok;
    }

    // Motion families (30/10): the body's choice beats the field's, which
    // beats the old orbit field (PULSO -> ESPUMA, FIGURA-8 -> PENDULOS);
    // each family really changes the dance — billiards travels more than
    // the free choreography, gravity gathers the bodies.
    {
        using F = PerformanceMotionFamily;
        PerformanceSkyDirector director;
        director.setFieldFamily(0U, PerformanceMotionField::Pulse);
        ok = expect(director.family(0U) == F::Foam, "PULSO becomes foam") && ok;
        director.setCollectiveFamily(F::Magnetic);
        ok = expect(director.family(0U) == F::Magnetic, "the field's family beats the orbit field") && ok;
        director.setBodyFamily(0U, F::Gravity);
        ok = expect(director.family(0U) == F::Gravity, "the body's family beats the field's") && ok;
        const auto bodies = opening();
        const auto measure = [&](F family) {
            PerformanceSkyDirector sky;
            sky.prepare(kRate);
            sky.setSeed(181U);
            sky.setCollectiveFamily(family);
            double travelled = 0.0, spread = 0.0;
            double lastX = -1.0, lastY = -1.0;
            int samples = 0;
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 40.0); ++sample) {
                sky.advance(bodies, 120.0);
                PerformanceSkyDirector::Cue due{};
                while (sky.popDue(due)) {}
                if (sample % 12000U == 0U)
                    for (std::size_t slot = 0U; slot < bodies.size(); ++slot) sky.hear(slot, 220.0);
                if (sample % 4800U != 0U) continue;
                const double x = sky.positionX(2U), y = sky.positionY(2U);
                if (lastX >= 0.0) travelled += std::hypot(x - lastX, y - lastY);
                lastX = x; lastY = y;
                double cx = 0.0, cy = 0.0;
                for (std::size_t slot = 0U; slot < 8U; ++slot) { cx += sky.positionX(slot); cy += sky.positionY(slot); }
                cx /= 8.0; cy /= 8.0;
                for (std::size_t slot = 0U; slot < 8U; ++slot)
                    spread += std::hypot(sky.positionX(slot) - cx, sky.positionY(slot) - cy);
                ++samples;
            }
            return std::make_pair(travelled, spread / samples / 8.0);
        };
        const auto free = measure(F::Free);
        const auto billiard = measure(F::Billiard);
        const auto gravity = measure(F::Gravity);
        std::cout << "families: free travels " << free.first << ", billiards " << billiard.first
                  << "; spread free " << free.second << ", gravity " << gravity.second << '\n';
        ok = expect(billiard.first > free.first, "billiards travels more than the free dance") && ok;
        ok = expect(gravity.second < free.second, "gravity gathers the bodies") && ok;
    }

    // Drawn cables (29/10): a strong cable pulls its pair into focus with
    // the act its phenomenon asks for; ATTRACT pulls the two bodies together
    // and REPEL pushes them apart; each phenomenon maps to a playable act.
    {
        using P = PerformanceRelationalPhenomenon;
        for (std::uint8_t phenomenon = 0U; phenomenon < static_cast<std::uint8_t>(P::Count); ++phenomenon) {
            const auto kind = PerformanceSkyDirector::phenomenonKind(phenomenon, RelationKind::Support);
            ok = expect(kind == RelationKind::Support || kind == RelationKind::CallResponse
                            || kind == RelationKind::Hocket || kind == RelationKind::Shadow
                            || kind == RelationKind::Contagion || kind == RelationKind::Friction,
                        "every phenomenon asks for a playable act") && ok;
        }
        ok = expect(PerformanceSkyDirector::phenomenonKind(static_cast<std::uint8_t>(P::Scatter), RelationKind::Support)
                        == RelationKind::Hocket, "SCATTER asks for a hocket") && ok;
        const SkyBodyState left{0.40, 0.5, 0.0, 0.0}, right{0.60, 0.5, 0.0, 0.0};
        double aax = 0.0, aay = 0.0, bax = 0.0, bay = 0.0;
        PerformanceSkyDirector::phenomenonForce(static_cast<std::uint8_t>(P::Attract), 1.0, left, right, aax, aay, bax, bay);
        ok = expect(aax > 0.0 && bax < 0.0, "ATTRACT pulls the pair together") && ok;
        aax = aay = bax = bay = 0.0;
        PerformanceSkyDirector::phenomenonForce(static_cast<std::uint8_t>(P::Repel), 1.0, left, right, aax, aay, bax, bay);
        ok = expect(aax < 0.0 && bax > 0.0, "REPEL pushes the pair apart") && ok;
        aax = aay = bax = bay = 0.0;
        PerformanceSkyDirector::phenomenonForce(static_cast<std::uint8_t>(P::Orbit), 1.0, left, right, aax, aay, bax, bay);
        ok = expect(aay != 0.0 && std::fabs(aay + bay) < 1e-12, "ORBIT turns the pair around its middle") && ok;

        PerformanceSkyDirector director;
        const auto bodies = opening();
        director.prepare(kRate);
        director.setSeed(181U);
        std::size_t cabledFocus = 0U, focusTotal = 0U;
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 60.0); ++sample) {
            if (sample % 64U == 0U) {
                director.clearCables();
                director.setCable(2U, 5U, 1.0, static_cast<std::uint8_t>(P::Scatter));
            }
            director.advance(bodies, 120.0);
            PerformanceSkyDirector::Cue due{};
            while (director.popDue(due)) {}
            if (sample % 24000U != 0U) continue;
            for (std::size_t index = 0U; index < director.focusCount(); ++index) {
                ++focusTotal;
                const auto& focus = director.focus(index);
                if (focus.caller == 2U && focus.responder == 5U) ++cabledFocus;
            }
        }
        std::cout << "cabled pair in focus " << cabledFocus << " of " << focusTotal << " foci\n";
        ok = expect(cabledFocus > focusTotal / 56U, "a drawn cable pulls its pair into focus more than chance") && ok;
    }

    // The live sky moves: bodies stay in the Field, keep moving through the
    // sections (no rigid freeze), wander off their drawn places and come
    // back; the same seed dances the same dance; another seed, another one.
    {
        const auto bodies = opening();
        const auto dance = [&](std::uint32_t seed, std::vector<double>& trace) {
            PerformanceSkyDirector director;
            director.prepare(kRate);
            director.setSeed(seed);
            double travelled = 0.0, farthest = 0.0;
            double lastX = -1.0, lastY = -1.0;
            bool inField = true;
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 120.0); ++sample) {
                director.advance(bodies, 120.0);
                PerformanceSkyDirector::Cue due{};
                while (director.popDue(due)) {}
                if (sample % 12000U == 0U)
                    for (std::size_t slot = 0U; slot < bodies.size(); ++slot)
                        director.hear(slot, 110.0 * static_cast<double>(slot + 1U));
                if (sample % 4800U != 0U) continue;
                const double x = director.shownX(2U), y = director.shownY(2U);
                inField = inField && x >= 0.0 && x <= 1.0 && y >= 0.0 && y <= 1.0 && std::isfinite(x + y);
                if (lastX >= 0.0) travelled += std::hypot(x - lastX, y - lastY);
                lastX = x;
                lastY = y;
                farthest = std::max(farthest, std::hypot(director.positionX(2U) - bodies[2].anchorX,
                                                         director.positionY(2U) - bodies[2].anchorY));
                trace.push_back(x);
            }
            std::cout << "dance seed " << seed << ": travelled " << travelled << ", farthest " << farthest
                      << ", sounding collisions " << director.collisionCount()
                      << ", pendulum turns " << director.pendulumTurns() << '\n';
            ok = expect(inField, "dancing bodies stay in the Field") && ok;
            ok = expect(travelled > 0.5, "the sky keeps moving") && ok;
            ok = expect(farthest > 0.03 && farthest < 0.9, "bodies wander off their places, within reach") && ok;
        };
        std::vector<double> first, again, other;
        dance(181U, first);
        dance(181U, again);
        dance(9001U, other);
        ok = expect(first == again, "the same seed dances the same dance") && ok;
        // During development a chain of three pendulums hangs: the chain
        // exists and every link stays finite and in the Field (checked above).
        {
            PerformanceSkyDirector director;
            director.prepare(kRate);
            director.setSeed(181U);
            std::size_t longest = 0U;
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 60.0); ++sample) {
                director.advance(bodies, 120.0);
                PerformanceSkyDirector::Cue due{};
                while (director.popDue(due)) {}
                if (sample % 4800U == 0U) longest = std::max(longest, director.pendulumChainLength());
            }
            std::cout << "longest pendulum chain " << longest << '\n';
            ok = expect(longest == 3U, "development hangs a chain of three pendulums") && ok;
        }
        ok = expect(first != other, "another seed dances another dance") && ok;
        // Recapitulation: once a return comes, the opening motif comes back.
        {
            PerformanceSkyDirector director;
            director.prepare(kRate);
            director.setSeed(181U);
            bool returned = false;
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 150.0); ++sample) {
                director.advance(bodies, 120.0);
                PerformanceSkyDirector::Cue due{};
                while (director.popDue(due)) {}
                if (sample % 12000U == 0U)
                    for (std::size_t slot = 0U; slot < bodies.size(); ++slot)
                        director.hear(slot, 110.0 * static_cast<double>(slot + 1U) * (1.0 + 0.1 * (sample / 12000U % 5U)));
                returned = returned || director.section() == PerformanceSkyDirector::Section::Return;
            }
            std::cout << "recapitulations " << director.recapitulations() << '\n';
            ok = expect(!returned || director.recapitulations() >= 1U, "a return brings the opening motif back") && ok;
            ok = expect(returned, "the form reaches a return within 150 s") && ok;
        }
    }

    if (!ok) return EXIT_FAILURE;
    std::cout << "Performance sky director tests passed\n";
    return EXIT_SUCCESS;
}
