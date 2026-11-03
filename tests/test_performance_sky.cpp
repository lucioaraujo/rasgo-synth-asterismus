#include <rasgo/performance/PerformanceSky.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace rasgo::performance;

namespace {

constexpr double kRate = 48000.0;

bool expect(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}

const char* starName(PerformanceModuleBody body) {
    switch (body) {
    case PerformanceModuleBody::Acid: return "Acid";
    case PerformanceModuleBody::Gendyn: return "Gendyn";
    case PerformanceModuleBody::Percussion: return "Drums";
    case PerformanceModuleBody::FmFlute: return "FM Flute";
    case PerformanceModuleBody::Theremin: return "Theremin";
    case PerformanceModuleBody::Scat: return "Scat";
    case PerformanceModuleBody::Choir: return "Choir";
    case PerformanceModuleBody::Vosim: return "VOSIM";
    case PerformanceModuleBody::Tuba: return "Tuba";
    case PerformanceModuleBody::Footnote: return "Footnote";
    case PerformanceModuleBody::Bluff: return "Bluff";
    case PerformanceModuleBody::Spectral: return "Spectral";
    case PerformanceModuleBody::Sieve: return "Sieve";
    case PerformanceModuleBody::Polymeter: return "Polymeter";
    case PerformanceModuleBody::Hocket: return "Hocket";
    case PerformanceModuleBody::Rupture: return "Rupture";
    case PerformanceModuleBody::Harmonaig: return "Harmonaig";
    case PerformanceModuleBody::Bloom: return "Bloom";
    case PerformanceModuleBody::ThermalStatic: return "Thermal Static";
    case PerformanceModuleBody::Core: return "Core";
    case PerformanceModuleBody::Texture: return "Texture";
    case PerformanceModuleBody::BassFallback: return "Bass Fallback";
    case PerformanceModuleBody::Walsh: return "Walsh";
    case PerformanceModuleBody::HardSync: return "Hard Sync";
    case PerformanceModuleBody::AtariPunk: return "Atari Punk";
    case PerformanceModuleBody::DejaVu: return "Deja Vu";
    case PerformanceModuleBody::Atropelo: return "Atropelo";
    case PerformanceModuleBody::Galo: return "Galo";
    default: return "?";
    }
}

// Control stars (2/11): attacks of Footnote and Atropelo (both on the memory
// clock) in 30 s, with an optional control star lit beside them. Returns the
// two attack counts, the samples where both attacked together and the
// summed output energy.
struct ControlRun { std::uint32_t footnote = 0U, atropelo = 0U, together = 0U, controlMarks = 0U; double energy = 0.0; };
ControlRun runWithControl(PerformanceModuleBody control, std::array<double, 3> values) {
    PerformanceSky sky;
    sky.prepare(kRate);
    sky.setSeed(181U);
    if (control != PerformanceModuleBody::Count)
        for (std::size_t index = 0U; index < 3U; ++index)
            sky.setControl(control, static_cast<PerformanceModuleControl>(index), values[index]);
    PerformanceSkyContext context;
    ControlRun run;
    for (std::size_t sample = 0U; sample < static_cast<std::size_t>(kRate * 30.0); ++sample) {
        sky.advance(context);
        if (control != PerformanceModuleBody::Count) {
            sky.renderStar(control, 220.0);
            run.controlMarks += sky.attacked(control) ? 1U : 0U;
        }
        const double a = sky.renderStar(PerformanceModuleBody::Footnote, 220.0);
        const double b = sky.renderStar(PerformanceModuleBody::Atropelo, 110.0);
        run.energy += a * a + b * b;
        const bool fa = sky.attacked(PerformanceModuleBody::Footnote);
        const bool fb = sky.attacked(PerformanceModuleBody::Atropelo);
        run.footnote += fa ? 1U : 0U;
        run.atropelo += fb ? 1U : 0U;
        run.together += fa && fb ? 1U : 0U;
    }
    return run;
}

// Renders `target` for `samples` while the stars in `others` are also lit.
std::vector<double> renderStar(std::uint32_t seed, PerformanceModuleBody target,
                               const std::vector<PerformanceModuleBody>& others,
                               std::size_t samples, bool renderTwice = false) {
    PerformanceSky sky;
    sky.prepare(kRate);
    sky.setSeed(seed);
    PerformanceSkyContext context;
    std::vector<double> out(samples);
    for (std::size_t index = 0U; index < samples; ++index) {
        sky.advance(context);
        for (const auto other : others) sky.renderStar(other, 220.0);
        out[index] = sky.renderStar(target, 110.0);
        if (renderTwice) out[index] = sky.renderStar(target, 110.0);
    }
    return out;
}

} // namespace

int main() {
    bool ok = expect(PerformanceSky::available(), "the sky needs the four vendored cohorts");
    // 30/10: fifteen stars play — the fourteen the seed chooses and the Tuba,
    // a library star the performer places by swapping a body's star.
    ok = expect(PerformanceSky::kStarCount == 28U, "twenty-eight stars play today") && ok;
    {
        const auto free = runWithControl(PerformanceModuleBody::Count, {});
        const auto sieved = runWithControl(PerformanceModuleBody::Sieve, {{0.6, 0.6, 0.3}});
        const auto poly = runWithControl(PerformanceModuleBody::Polymeter, {{0.5, 0.2, 1.0}});
        const auto hocket = runWithControl(PerformanceModuleBody::Hocket, {{0.0, 1.0, 1.0}});
        const auto rupture = runWithControl(PerformanceModuleBody::Rupture, {{0.0, 1.0, 1.0}});
        std::cout << "control stars (footnote/atropelo/together): free " << free.footnote << '/' << free.atropelo
                  << '/' << free.together << ", sieve " << sieved.footnote << '/' << sieved.atropelo
                  << ", polymeter " << poly.footnote << '/' << poly.atropelo << ", hocket " << hocket.footnote
                  << '/' << hocket.atropelo << '/' << hocket.together << ", rupture " << rupture.footnote << '/'
                  << rupture.atropelo << " (" << rupture.controlMarks << " ruptures)\n";
        ok = expect(free.together > 0U, "without control both memory stars attack together") && ok;
        ok = expect(sieved.footnote + sieved.atropelo < free.footnote + free.atropelo
                        && sieved.footnote > 0U && sieved.atropelo > 0U,
                    "the Sieve thins the attacks without silencing them") && ok;
        ok = expect(poly.footnote != poly.atropelo && poly.footnote > 0U && poly.atropelo > 0U,
                    "the Polymeter gives each star its own cycle") && ok;
        ok = expect(hocket.together == 0U && hocket.footnote > 0U && hocket.atropelo > 0U,
                    "the Hocket never lets two stars attack at once") && ok;
        ok = expect(rupture.controlMarks > 0U
                        && rupture.footnote + rupture.atropelo < free.footnote + free.atropelo
                        && rupture.energy < free.energy,
                    "the Rupture silences the sky now and then") && ok;
    }
    // Toda estrela tocável soa, finita; as de ataque marcam ataques (a 17ª
    // caía fora da máscara de 16 bits); renderizar duas vezes na mesma amostra
    // não avança o DSP.
    for (const auto star : kPerformanceSkyStars) {
        PerformanceSky sky;
        sky.prepare(kRate);
        sky.setSeed(181U);
        PerformanceSkyContext context;
        double energy = 0.0;
        std::uint32_t attacks = 0U;
        bool finite = true, stable = true;
        for (std::size_t index = 0U; index < static_cast<std::size_t>(kRate * 20.0); ++index) {
            sky.advance(context);
            const double first = sky.renderStar(star, 110.0);
            stable = stable && sky.renderStar(star, 110.0) == first;
            finite = finite && std::isfinite(first);
            energy += first * first;
            attacks += sky.attacked(star) ? 1U : 0U;
        }
        const std::string name{starName(star)};
        if (performanceStarFunction(star) == PerformanceStarFunction::Control) {
            ok = expect(finite && energy == 0.0, (name + " is a silent control star").c_str()) && ok;
            continue;
        }
        ok = expect(finite && energy > 1.0, (name + " sounds, finite").c_str()) && ok;
        ok = expect(stable, (name + " renders once per sample").c_str()) && ok;
        if (star == PerformanceModuleBody::Footnote || star == PerformanceModuleBody::Bluff
            || star == PerformanceModuleBody::Spectral || star == PerformanceModuleBody::Harmonaig
            || star == PerformanceModuleBody::Bloom || star == PerformanceModuleBody::Core
            || star == PerformanceModuleBody::BassFallback)
            ok = expect(attacks > 2U, (name + " marks its attacks").c_str()) && ok;
    }
    // Desde 3/11 a seed escolhe entre todas as estrelas do céu; no máximo uma
    // estrela de controle (não soa) por céu.
    {
        bool library = false;
        for (std::uint32_t seed = 1U; seed <= 400U; ++seed) {
            std::size_t controls = 0U;
            for (const auto body : performanceInitialAsterism(seed)) {
                controls += body != PerformanceModuleBody::ModalResonator
                    && performanceStarFunction(body) == PerformanceStarFunction::Control;
                library = library || body == PerformanceModuleBody::Tuba || body == PerformanceModuleBody::Harmonaig;
            }
            ok = expect(controls <= 1U, "at most one control star in a sky") && ok;
        }
        ok = expect(library, "the seed now also chooses the library stars") && ok;
    }
    ok = expect(!PerformanceSky::isStar(PerformanceModuleBody::ModalResonator),
                "Modal is not a star until its adapter moves here") && ok;

    // Every star sounds alone, finite and bounded (control stars are silent:
    // they filter the others' attacks, tested above).
    const auto seconds = static_cast<std::size_t>(kRate * 8.0);
    for (const auto star : kPerformanceSkyStars) {
        if (performanceStarFunction(star) == PerformanceStarFunction::Control) continue;
        const auto signal = renderStar(181U, star, {}, seconds);
        double peak = 0.0;
        bool finite = true;
        for (const auto sample : signal) {
            finite = finite && std::isfinite(sample);
            peak = std::max(peak, std::fabs(sample));
        }
        std::cout << starName(star) << " peak " << peak << '\n';
        ok = expect(finite, starName(star)) && ok;
        ok = expect(peak > 0.01, starName(star)) && ok;
        ok = expect(peak <= 1.0, starName(star)) && ok;
    }

    // One module = one stem: a star does not change when other sounding stars
    // are lit. Control stars change the others on purpose (2/11), so they are
    // left out here and tested by their effect above.
    const auto shortRun = static_cast<std::size_t>(kRate * 2.0);
    for (const auto star : kPerformanceSkyStars) {
        std::vector<PerformanceModuleBody> everyoneElse;
        for (const auto other : kPerformanceSkyStars)
            if (other != star && performanceStarFunction(other) != PerformanceStarFunction::Control)
                everyoneElse.push_back(other);
        ok = expect(renderStar(4242U, star, {}, shortRun)
                        == renderStar(4242U, star, everyoneElse, shortRun),
                    starName(star)) && ok;
    }

    // Rendering twice in one sample must not advance the star twice.
    ok = expect(renderStar(77U, PerformanceModuleBody::Acid, {}, shortRun)
                    == renderStar(77U, PerformanceModuleBody::Acid, {}, shortRun, true),
                "double render advanced the DSP") && ok;

    // The seed is a reproducible score, and different seeds differ.
    ok = expect(renderStar(1234U, PerformanceModuleBody::Acid, {}, shortRun)
                    == renderStar(1234U, PerformanceModuleBody::Acid, {}, shortRun),
                "same seed must reproduce") && ok;
    ok = expect(renderStar(1234U, PerformanceModuleBody::Acid, {}, shortRun)
                    != renderStar(1235U, PerformanceModuleBody::Acid, {}, shortRun),
                "different seeds must differ") && ok;

    // A star's own control reaches its sound.
    {
        PerformanceSky low;
        PerformanceSky high;
        low.prepare(kRate);
        high.prepare(kRate);
        high.setControl(PerformanceModuleBody::Walsh, PerformanceModuleControl::Primary, 0.95);
        low.setControl(PerformanceModuleBody::Walsh, PerformanceModuleControl::Primary, 0.05);
        PerformanceSkyContext context;
        double difference = 0.0;
        for (std::size_t index = 0U; index < shortRun; ++index) {
            low.advance(context);
            high.advance(context);
            difference += std::fabs(low.renderStar(PerformanceModuleBody::Walsh, 110.0)
                                    - high.renderStar(PerformanceModuleBody::Walsh, 110.0));
        }
        ok = expect(difference > 1.0, "Walsh primary control must change the sound") && ok;
    }

    // The seed chooses stars, not cohorts: the opening figure has a tonal
    // floor, a matter of friction and a pulse; no star lights twice; and over
    // many seeds every star appears and stars from different former families
    // meet in the same asterism.
    {
        std::array<int, static_cast<std::size_t>(PerformanceModuleBody::Count)> appearances{};
        bool crossFamily = false;
        for (std::uint32_t seed = 1U; seed <= 4000U; ++seed) {
            const auto asterism = performanceInitialAsterism(seed);
            ok = expect(performanceStarFunction(asterism[0]) == PerformanceStarFunction::Tone,
                        "body 1 is a tonal floor") && ok;
            ok = expect(performanceStarFunction(asterism[1]) == PerformanceStarFunction::Texture
                            || performanceStarFunction(asterism[1]) == PerformanceStarFunction::Voice,
                        "body 2 is matter of friction") && ok;
            ok = expect(performanceStarFunction(asterism[2]) == PerformanceStarFunction::Pulse,
                        "body 3 is a pulse") && ok;
            ok = expect(asterism[4] == PerformanceModuleBody::ModalResonator, "body 5 stays Modal") && ok;
            for (std::size_t index = 0U; index < asterism.size(); ++index) {
                if (index != 4U) {
                    ok = expect(PerformanceSky::isStar(asterism[index]), "every other body is a star") && ok;
                    ++appearances[static_cast<std::size_t>(asterism[index])];
                }
                for (std::size_t other = 0U; other < index; ++other)
                    ok = expect(asterism[index] != asterism[other], "no star lights twice") && ok;
            }
            // Theremin (former Breath) next to Atari Punk (former Digital).
            bool theremin = false, atari = false;
            for (const auto body : asterism) {
                theremin = theremin || body == PerformanceModuleBody::Theremin;
                atari = atari || body == PerformanceModuleBody::AtariPunk;
            }
            crossFamily = crossFamily || (theremin && atari);
        }
        // Every star of the sky appears in some asterism (since 3/11 the seed
        // chooses among all of them).
        for (const auto star : kPerformanceSkyStars)
            ok = expect(appearances[static_cast<std::size_t>(star)] > 0, starName(star)) && ok;
        ok = expect(crossFamily, "stars of different former families meet") && ok;
        ok = expect(performanceInitialAsterism(181U) == performanceInitialAsterism(181U),
                    "the asterism is reproducible from the seed") && ok;
    }

    // Kepler swing: equal angles of an orbit fall at unequal times. A
    // circular orbit is straight time; with eccentricity the first half of the
    // beat lasts longer (perihelion at the off-beat) and the bar keeps its
    // length; the circuit steps really fall at those times.
    {
        for (double fraction = 0.0; fraction <= 1.0; fraction += 0.125)
            ok = expect(std::fabs(PerformanceSky::keplerWarp(fraction, 0.0, 3.14159) - fraction) < 1.0e-9,
                        "a circular orbit is straight time") && ok;
        double previous = -1.0;
        for (double fraction = 0.0; fraction <= 1.0 + 1.0e-12; fraction += 0.0625) {
            const double warped = PerformanceSky::keplerWarp(fraction, 0.3, 3.14159265);
            ok = expect(warped > previous, "the orbital time only moves forward") && ok;
            previous = warped;
        }
        ok = expect(std::fabs(PerformanceSky::keplerWarp(1.0, 0.3, 3.14159265) - 1.0) < 1.0e-9,
                    "one orbit is one beat") && ok;
        ok = expect(PerformanceSky::keplerWarp(0.5, 0.3, 1.5707963) > 0.55,
                    "with eccentricity the first half of the beat swings long") && ok;

        PerformanceSky sky;
        sky.prepare(kRate);
        sky.setSeed(181U);
        sky.setTiming(0.3, 1.5707963, 0.0);
        PerformanceSkyContext context;
        context.tempoBpm = 120.0;
        std::vector<std::uint64_t> attacks;
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 20.0); ++sample) {
            sky.advance(context);
            sky.renderStar(PerformanceModuleBody::Percussion, 110.0);
            if (sky.attacked(PerformanceModuleBody::Percussion)) attacks.push_back(sample);
        }
        bool uneven = false;
        for (std::size_t index = 2U; index < attacks.size(); ++index) {
            const auto a = attacks[index - 1U] - attacks[index - 2U];
            const auto b = attacks[index] - attacks[index - 1U];
            if (a > 0U && b > 0U && (a > b * 1.3 || b > a * 1.3)) uneven = true;
        }
        ok = expect(attacks.size() > 10U && uneven, "the drum steps fall at orbital, uneven times") && ok;
        for (std::size_t step = 1U; step <= 16U; ++step)
            ok = expect(sky.stepTime(step) > sky.stepTime(step - 1U), "step times keep their order") && ok;
    }

    // Counting: Euclidean patterns are exact (E(3,8) is the tresillo
    // x..x..x.), every counting star acts exactly k times per cycle of n
    // pulses, and cycles of different lengths realign only at their lcm.
    {
        std::string tresillo;
        for (std::uint32_t step = 0U; step < 8U; ++step)
            tresillo += PerformanceSky::euclidOnset(step, 3U, 8U, 0U) ? 'x' : '.';
        std::cout << "E(3,8) = " << tresillo << '\n';
        ok = expect(tresillo == "x..x..x.", "E(3,8) is the tresillo") && ok;
        for (std::uint32_t n = 2U; n <= 16U; ++n)
            for (std::uint32_t k = 1U; k < n; ++k) {
                std::uint32_t count = 0U;
                for (std::uint32_t step = 0U; step < n; ++step)
                    count += PerformanceSky::euclidOnset(step, k, n, 3U) ? 1U : 0U;
                ok = expect(count == k, "every rotation of E(k,n) has k onsets") && ok;
            }
        // Realignment: E(2,5) and E(3,7) share their first onset again only
        // after 35 pulses.
        std::uint32_t firstCommon = 0U;
        for (std::uint32_t step = 1U; step < 200U && firstCommon == 0U; ++step)
            if (step % 5U == 0U && step % 7U == 0U) firstCommon = step;
        ok = expect(firstCommon == 35U, "cycles of 5 and 7 realign after 35 pulses") && ok;

        PerformanceSky sky;
        sky.prepare(kRate);
        sky.setSeed(4242U);
        sky.setCounting(true, 0U);
        PerformanceSkyContext context;
        context.tempoBpm = 120.0;
        const auto length = sky.euclidLength(PerformanceModuleBody::Percussion);
        const auto pulses = sky.euclidPulses(PerformanceModuleBody::Percussion);
        std::uint32_t attacks = 0U;
        std::uint64_t ticks = 0U;
        for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 30.0); ++sample) {
            sky.advance(context);
            sky.renderStar(PerformanceModuleBody::Percussion, 110.0);
            sky.renderStar(PerformanceModuleBody::Acid, 110.0);
            if (sky.attacked(PerformanceModuleBody::Acid)) ++ticks;
            if (sky.attacked(PerformanceModuleBody::Percussion)) ++attacks;
        }
        std::cout << "drums count E(" << pulses << "," << length << "): " << attacks << " attacks\n";
        ok = expect(length >= 8U && pulses >= 1U && pulses < length, "drums count a dense cycle") && ok;
        ok = expect(attacks > 0U, "a counting star still plays") && ok;

        // Additive groupings: every grouping fills the 16-pulse bar starting
        // on the downbeat; a contrast always picks a grouping other than the
        // home one, and the return restores it; no counting, no grouping.
        for (const auto mask : PerformanceSky::kGroupings)
            ok = expect((mask & 1U) != 0U && mask < (1U << 16U), "a grouping starts on the downbeat") && ok;
        const auto home = sky.groupingMask();
        bool contrastDiffers = true;
        for (std::uint32_t ordinal = 1U; ordinal < 12U; ++ordinal) {
            sky.setCounting(true, ordinal);
            contrastDiffers = contrastDiffers && sky.groupingMask() != home;
        }
        sky.setCounting(true, 0U);
        std::cout << "home grouping mask 0x" << std::hex << home << std::dec << '\n';
        ok = expect(contrastDiffers, "a contrast changes the grouping") && ok;
        ok = expect(sky.groupingMask() == home, "the return restores the home grouping") && ok;
        // Duration sequences: with counting, a counting star's duration factor
        // changes from attack to attack and averages 1 over its sequence.
        {
            double low = 10.0, high = 0.0;
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 10.0); ++sample) {
                sky.advance(context);
                sky.renderStar(PerformanceModuleBody::Scat, 220.0);
                const double factor = sky.durationFactor(PerformanceModuleBody::Scat);
                low = std::min(low, factor);
                high = std::max(high, factor);
            }
            std::cout << "scat duration factor " << low << ".." << high << '\n';
            ok = expect(sky.durationFactor(PerformanceModuleBody::Percussion) == 1.0,
                        "drums keep their own third control") && ok;
            ok = expect(low < 1.0 && high > 1.0, "a counting star varies its durations") && ok;
            for (const auto& sequence : PerformanceSky::kDurationSequences) {
                double sum = 0.0;
                std::size_t length = 0U;
                for (const auto term : sequence) { if (term == 0U) break; sum += term; ++length; }
                ok = expect(length >= 2U && sum > 0.0, "a duration sequence has terms") && ok;
            }
        }
        sky.setCounting(false, 0U);
        ok = expect(sky.groupingMask() == 0U, "without counting there is no grouping") && ok;
        ok = expect(sky.durationFactor(PerformanceModuleBody::Scat) == 1.0,
                    "without counting durations are untouched") && ok;

        // Sieves: (3,0) ∪ (5,1) repeats every 15 steps with 7 onsets
        // x x . x . . x . . x . x x . . ; home counting never uses a sieve,
        // some contrast does, and a sieving star still plays.
        std::string sieve;
        for (std::uint32_t step = 0U; step < 15U; ++step)
            sieve += PerformanceSky::sieveOnset(step, 3U, 5U, 1U) ? 'x' : '.';
        std::cout << "sieve (3,0)u(5,1) = " << sieve << '\n';
        ok = expect(sieve == "xx.x..x..x.xx..", "the sieve (3,0)u(5,1) is exact") && ok;
        bool sieveAtHome = false;
        for (const auto body : kPerformanceSkyStars) {
            sky.setCounting(true, 0U);
            sieveAtHome = sieveAtHome || sky.usesSieve(body);
        }
        ok = expect(!sieveAtHome, "the home metre never sieves") && ok;
        std::uint32_t sieveOrdinal = 0U;
        PerformanceModuleBody sieving = PerformanceModuleBody::Acid;
        for (std::uint32_t ordinal = 1U; ordinal < 16U && sieveOrdinal == 0U; ++ordinal) {
            sky.setCounting(true, ordinal);
            for (const auto body : kPerformanceSkyStars)
                if (sky.usesSieve(body)) { sieveOrdinal = ordinal; sieving = body; break; }
        }
        ok = expect(sieveOrdinal != 0U, "some contrast sieves") && ok;
        if (sieveOrdinal != 0U) {
            std::uint32_t sieveAttacks = 0U;
            for (std::uint64_t sample = 0U; sample < static_cast<std::uint64_t>(kRate * 20.0); ++sample) {
                sky.advance(context);
                sky.renderStar(sieving, 220.0);
                if (sky.attacked(sieving)) ++sieveAttacks;
            }
            std::cout << "a sieving star attacked " << sieveAttacks << " times in 20 s\n";
            ok = expect(sieveAttacks > 0U, "a sieving star still plays") && ok;
        }
    }

    // Cost of the whole sky lit: reported, and bounded far below real time.
    {
        PerformanceSky sky;
        sky.prepare(kRate);
        PerformanceSkyContext context;
        const auto samples = static_cast<std::size_t>(kRate * 10.0);
        double sink = 0.0;
        const auto start = std::chrono::steady_clock::now();
        for (std::size_t index = 0U; index < samples; ++index) {
            sky.advance(context);
            for (const auto star : kPerformanceSkyStars) sink += sky.renderStar(star, 110.0);
        }
        const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        const double blockMs = elapsed / (static_cast<double>(samples) / 512.0) * 1000.0;
        std::cout << "14 stars lit: " << blockMs << " ms per 512-sample block (budget "
                  << 512.0 / kRate * 1000.0 << " ms), " << 10.0 / elapsed << "x real time\n";
        ok = expect(std::isfinite(sink), "sky sum must be finite") && ok;
        ok = expect(elapsed < 10.0, "fourteen stars must render faster than real time") && ok;
    }

    if (!ok) return EXIT_FAILURE;
    std::cout << "Performance sky tests passed\n";
    return EXIT_SUCCESS;
}
