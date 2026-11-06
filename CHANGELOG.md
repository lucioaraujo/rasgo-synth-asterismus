# Changelog

What changed between the published versions of Rasgo Synth Asterismus. The
development notes, in Portuguese, are in [`DESENVOLVIMENTO.md`](DESENVOLVIMENTO.md)
and [`docs/`](docs/).

## 0.3.0 — in development (not released)

- **More stars:** AY-8910, MORPH, RÖSSLER, ENVELOPE PULSE (Studio voices),
  GRANULAR, TURING, SPECTRAL GHOST and VECTOR (rewritten from Studio
  concepts — the ghost keeps the spectrum of what the sky is playing and
  lets its phase drift; the vector blends four timbres by a point that
  orbits a square); they play from the
  LIBRARY and as guests in the contrast — the eight stars a seed draws are
  the same as in 0.2.0. The DRUMS kit (cowbell, clap, cymbal, maracas) comes
  from the seed.
- **Harmony:** neo-Riemannian progressions (P/L/R) and 16 contrast modes.
- **The sky's cuts and tape:** PARENTHESIS, MONTAGE CUT, APOCRYPHAL,
  DECIMATION, TAPE STOP, SPIN BACK and VARISPEED as sky gestures; a TAPE or
  VINYL layer (saturation, wow, hiss, crackle) in the parts the seed picks.
- **Filters and sends:** a filter on every star (SVF from Rasgo Modular, CMOS
  and MATERIAL from Antitotem; type, cutoff, resonance and mix from the seed)
  and two sends, ECHO (in tempo) and HALL; the focused star has CUT, RES.,
  ECHO and HALL controls (centre = the seed).
- **Groove dialogue on the DRUMS:** in about six seeds out of ten the DRUMS
  plays groove families (long-short sixteenths, loose backbeat, interlocking
  offbeats, 3+3+2, displaced accents, phrase and answer — the Studio's
  GrooveDialogue concept) instead of the Circuit cells: the family changes
  with each part of the form and comes back with it; each drum has its own
  timing (swing, a late snare, a late hat), ghost snares and accents, and
  every four bars ends with an answer (a ghost snare and a 2–4 hat ratchet).

## 0.2.0 — first public version (2026-11-06)

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
  the LIBRARY button (▾) to swap or pin a star.
- **Recording.** REC records a 24-bit stereo WAV, one WAV per body and a
  JSON manifest, in one folder per take; TRACK · DURATION (1, 2, 3 or 5 min)
  fits the form into the chosen length; when the coda ends only the recording
  closes — the sound goes on and a new piece opens; ▶ LISTEN opens the last take.
- **The seed composes more.** Every seed — small numbers too — has its own
  profile; the seed chooses the cables' phenomena (23), the scenes' population
  phenomena (10), the physics of each part, one of six form grammars (cycle,
  arc, rondo, waves, kishōtenketsu, shock), a harmonic region for the
  contrast, a few sky gestures (STUTTER, DISPLACE), guest stars from outside
  the sky in the contrast, the order of the PATH scenes and a coda in
  proportion to the track.
- **POLYPHONY.** The seed's density (how many stars light, how much they play,
  how often the sky calls them) is a set of genes; the POLYPHONY slider moves
  it from soloist to choral, and its label shows the seed's index (1–10).
- **Album.** Tracks with seeds derived from the album seed; the next track
  comes in by itself; RENDER THE WHOLE ALBUM renders every track faster than real
  time; MASTER THE ALBUM brings the album to −16 LUFS with a
  look-ahead true-peak limiter at −1 dBTP (copies only, the takes never change).
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
