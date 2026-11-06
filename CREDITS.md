# Créditos — Rasgo Synth Asterismus

Instrumento da família RASGO. Ex-Rasgo Synth Performance; nome atual desde
2026-10-27.

## Autoria

- **Concepção, direção artística, decisões musicais e de identidade:** Lúcio
  de Araújo (RASGO).
- **Desenvolvimento assistido:** Claude (Anthropic), sob direção do autor,
  desde 2026-10-27; versões anteriores do Performance tiveram assistência do
  Codex (OpenAI). Toda decisão estética é do autor.

## Linhagem dentro do RASGO

- **Rasgo Synth Studio** — origem do DSP das estrelas, copiado com
  proveniência em `vendor/studio-dsp/` (commit `8fc69f5`, GPLv3; créditos
  herdados em `vendor/studio-dsp/CREDITS.md`); acréscimos de 30/10 (o
  `FootnotePluck` e a lógica do `BluffSignal`, commit `cfe1698`) registrados
  no fim do `vendor/studio-dsp/PROVENANCE.md`. Quatro estrelas da 0.3
  (6/11) são reescritas a partir do conceito do Studio, sem código copiado
  (`include/rasgo/performance/PerformanceStudioVoices.hpp`): GRANULAR
  (Clouds, Émilie Gillet / Mutable Instruments), TURING (Turing Machine,
  Tom Whitwell / Music Thing Modular), SPECTRAL GHOST (randomização de fase
  — a ideia do modo espectral de Clouds/Beads, citada no `SpectralGhost`
  do Studio) e VECTOR (síntese vetorial do Prophet VS, Chris Meyer /
  Sequential Circuits; o canto estocástico segue o GENDYN de Iannis
  Xenakis; o ruído rosa, o filtro de Paul Kellet).
- **Antitotem** — o master de saída (`PerformanceMasterStage.hpp`) adapta a
  técnica do `OutputStage` (commit `645e5ad`, AGPL-3.0-or-later); o
  `SignalLeveler`, usado na primeira versão, saiu no mesmo dia (a
  arquitetura comum de saída proíbe normalizar a performance ao vivo).
- **Navalha 2** — o padrão de saída de áudio (nível, true peak, dither); o
  medidor (`PerformanceLoudnessMeter.hpp`) é cópia adaptada da ponderação K
  BS.1770-4 (commit `b1a9614`) e do detector de true peak 4× (commit
  `34f8fd7`), GPLv3.
- **Aquorbium** — conceitos da teia de relações e do coro de fases
  (`ChorusPhase`), reescritos; nenhum código copiado.
- **Trioio** — conceito de swing como razão de duração e de groove por
  posição; nenhum código copiado.
- **Batruc** — conceitos de ritmos, contagens e movimento gráfico (só lidos).

## Ciência e matemática citadas

As fontes de cada conceito estão nos documentos onde ele é usado:
`docs/TEMPO_MALEAVEL.md` (Todd, Benadon, Danielsen, Toussaint, Brăiloiu,
Kuramoto, Aihara, Adler, Reich, Wiener–Khinchin, Brown, Xenakis, Espenak e
Meeus), `docs/MOVIMENTO_FISICO.md` (Bunimovich, Lynch, Pickover, Jackson,
Brody–Cross–Lindsey, Cross, Weaire–Hutzler, Durian) e
`docs/TRAMA_DE_RELACOES.md`.

## Recursos de terceiros

JUCE 9.0.0 (via AGPLv3) para o host; fontes Big Shoulders Display e IBM Plex
Mono (SIL OFL 1.1). Detalhes em `THIRD_PARTY_NOTICES.md` e `DEPENDENCIES.md`.
