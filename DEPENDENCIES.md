# Dependências — Rasgo Synth Asterismus

## Núcleo (motor, testes)

C++20, CMake ≥ 3.16. Nenhuma biblioteca externa: o DSP das estrelas é cópia
local com proveniência (`vendor/studio-dsp/`, do Rasgo Synth Studio commit
`8fc69f532f2b7044b2da43a9112188d85ca7a3cc`, GPLv3).

## Host (aplicativo)

- **JUCE 9.0.0**, via AGPLv3, checkout próprio em `third_party/JUCE/` (fora
  do Git, ignorado). `LICENSE.md` do checkout:
  SHA-256 `a37eaefe4f160da98bc07d2e8d951e46a9b52abd6e23b233ffcadf003d690f93`.
  O CMake não usa o JUCE do Studio (`RASGO_PERFORMANCE_JUCE_ROOT`).
- **Fontes:** Big Shoulders Display, IBM Plex Mono (OFL 1.1), em
  `assets/fonts/`, com as licenças ao lado.

## Ferramentas locais de verificação (opcionais)

- `ffmpeg` (filtro `ebur128`, ITU-R BS.1770) para medir loudness e true peak
  das tomadas (`docs/SAIDA_DE_AUDIO.md`).
- `python3` com `Xlib` e ImageMagick (`import`, `convert`) para capturas de
  tela de instâncias isoladas (`RASGO_ASTERISMUS_AUTOPLAY=1`).
