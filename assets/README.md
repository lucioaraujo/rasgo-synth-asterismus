# Recursos visuais — Rasgo Synth Performance

Este diretório é a cópia de distribuição da identidade do cabeçalho do
Rasgo Synth Performance. Ele evita qualquer dependência de execução com a
árvore do Rasgo Synth Studio: no build, CMake o copia ao lado do executável
em `assets/`.

| Recurso | Origem e proveniência | Licença | SHA-256 |
| --- | --- | --- | --- |
| `rasgo_logo_2026.svg` | Arte vetorizada de Lucio Araújo, 2026; cópia local autorizada de `rasgo-synth-app/assets/` | obra do projeto RASGO | `b600ed690d6475cd202984669c55c7451bfac7aa83b056da2a0391d3e0ab58a6` |
| `asterismus-icon.png` | Ícone do Asterismus (o céu E escolhido pelo autor em 2026-11-07), gerado por `tools/gerar_identidade.py` a partir de `packaging/logo/asterismus-logo.svg`; ícone da janela | obra do projeto RASGO | `d8e46d30bca8b9905f38737b7e5fe62fbbbcb876ef3e9d193b95dcc8846f829a` |
| `fonts/BigShouldersDisplay.ttf` | Big Shoulders Display, Google Fonts | SIL Open Font License 1.1; aviso em `fonts/LICENSE-BigShoulders.txt` | `60e208dc276a1c35fc5b62e94f9fb959c40c11783a9eb7548175c14b1fbeb720` |
| `fonts/IBMPlexMono-Regular.ttf` | IBM Plex Mono | SIL Open Font License 1.1; aviso em `fonts/LICENSE-IBMPlexMono.txt` | `6a3412f058c7d8dfd9170c41e85ade48e5156ecb89356110ca57a0a27734af46` |
| `fonts/IBMPlexMono-Bold.ttf` | IBM Plex Mono | SIL Open Font License 1.1; aviso em `fonts/LICENSE-IBMPlexMono.txt` | `ac27abd6450a64dd94467580a02fe6235156d5b92f2926ebbc8e7489df64e0be` |

Uso atual: a logo e Big Shoulders compõem `RASGO SYNTH · PERFORMANCE`; o host
resolve `Monospace` do sistema para manter a mesma métrica do Studio. IBM Plex
fica preservada como recurso licenciado de distribuição futura. O prefixo de ambiente
`RASGO_SYNTH_PERFORMANCE_ASSETS_DIR` permite uma localização empacotada
alternativa durante distribuição ou teste.
