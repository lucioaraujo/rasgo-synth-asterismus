# Changelog

What changed between the published versions of Rasgo Synth Asterismus. The
development notes, in Portuguese, are in [`DESENVOLVIMENTO.md`](DESENVOLVIMENTO.md)
and [`docs/`](docs/).

## 0.2.0 — first public version (preview, not yet released)

The first public version of the Asterismus, a live instrument in which every
synthesis module is a star. The internal development versions went up to
0.1.16-dev; 0.2.0 is the first one published.

- **A sky of 28 stars.** Tone, texture, voice and pulse stars, plus four
  *control* stars that do not sound but filter how the others play (sieve,
  polymeter, hocket, rupture). The **seed** draws the asterism: eight bodies
  chosen among all the stars, a key and a mode, a form, the counts and the
  moons. Every seed is another piece.
- **The web of the sky (WEB).** Each bar, the sky chooses pairs in focus and
  an act for each pair (answer, contagion, shadow, hocket, pedal,
  friction, cadence); the cables the musician draws pull their pair into
  focus. The piece travels a form — opening, development, contrast, return —
  and ends with a coda.
- **Time that breathes:** an elastic tempo, a swing that comes from an orbit,
  each star counting its own cycle, bars grouped as 3+3+2 and the like, a
  star that drifts against the pulse, eclipses brought by computed moons.
- **Physical motion.** In the Constellation the stars move by physics —
  gravity, springs, pendulums, billiards, magnetism, spin, foam — and leave
  trails; strong collisions sound. **Each star sounds where it is** in the
  stereo field (the bass stays centred); ESPAÇO opens that field.
- **Playing.** Four scenes (NASCENTE, ESPIRAL, RASGO, BALANÇO) and the PATH
  that follows the form; gestures on the star in focus (OPEN, TEAR, REPAIR,
  STUTTER, DISPLACE, HOLD, PANIC SAFE); a mixer per star with mute, solo and
  a fader that moves with VARIATION (the sky lighting and dimming stars);
  the library to swap or pin a star.
- **Recording.** REC records a 24-bit stereo WAV, one WAV per body and a
  JSON manifest, in one folder per take; TRACK · DURATION (1, 2, 3 or 5 min)
  fits the form into the chosen length; ▶ LISTEN opens the last take.
- **Memory and sessions.** Eight named places; SAVE SESSION / OPEN SESSION as
  `.asterismus` files (parameters only, never audio).
- **Output** in the common RASGO standard: fixed gain, limiter with 4×
  true-peak detection, −1.41 dBTP ceiling, BS.1770 loudness metering,
  dither on recording. Each star has a fixed reference level (measured over
  48 seeds), so no star sits 15 dB above the others; a typical piece comes
  out near −19 LUFS with MASTER at −6 dB.
- **Robustness:** if the sound system stalls or the device disappears, the
  app reopens it and the piece goes on from where it was; a take survives a
  crash of the app (its header is updated every second).
- **Tutorial** in English, Portuguese, French and Spanish.

The development history up to 2026-11-02 is kept in the author's private
repository (`rasgo-instruments`, last Asterismus commit `e369945`).
