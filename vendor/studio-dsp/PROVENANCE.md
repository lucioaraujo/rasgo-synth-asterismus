# DSP local do Rasgo Synth Performance

Origem: Rasgo Synth Studio / rasgo-synth-core.
Commit: `8fc69f532f2b7044b2da43a9112188d85ca7a3cc`.
Importação: 2026-10-11, durante a separação física dos instrumentos.
Autor: Lucio de Araujo. Licença herdada: GPLv3, texto integral em LICENSE.
Créditos e referências herdados em CREDITS.md (snapshot do mesmo commit).

28 headers e seis definições CMake, copiados do commit acima. Conferidos
também contra os headers vivos antes da cópia: nenhuma divergência.
Os hashes abaixo descrevem os bytes na origem. A verificação deve conferir
bytes integrais, incluindo newline final; nenhuma transformação de DSP.
Adaptação ao Performance já ocorre nos adapters em include/rasgo/performance;
esta etapa muda somente a custódia/localização dos packages e a resolução
de build. Não é promoção musical de novos módulos.

Não depende do Studio, não segue symlinks para o Studio e não sincroniza
automaticamente. Futuras adaptações pertencem ao Performance e devem ser
registradas aqui. JUCE é dependência separada, com seus próprios avisos;
esta cópia de DSP autoral não redefine o regime de distribuição do host.

## SHA-256 na origem

| Arquivo | SHA-256 |
| --- | --- |
| `cmake/modal-matter/CMakeLists.txt` | `7b8e3ee1d0cdb6700102deb43942d649e86f8754578ec89aec6b28c75ba6e5a3` |
| `cmake/performance-cohort-v1/CMakeLists.txt` | `491b7db307a8c2af7ccffec88c56f7c195deb5bd62fd379aaf75800e780e5af4` |
| `cmake/performance-cohort-v2/CMakeLists.txt` | `eb42b8d7809401ec6dab365327a62e823e24857faf73b5459014f944181d4394` |
| `cmake/performance-cohort-v3/CMakeLists.txt` | `baa90f9682aec1ca18baa1065416f10c4717d7aa6524212868635ec678aa8ad0` |
| `cmake/performance-cohort-v4/CMakeLists.txt` | `68772a04c18d511927ddc7a9fe83f9c38e90b42e1034331d80be40bef366515a` |
| `cmake/performance-fracture-v5/CMakeLists.txt` | `23881b034d1c5f270db85110d50b5e3b979169a7660d338d91ee6cd5eb5cc1c8` |
| `src/dsp/MultibandFilter.hpp` | `af1fd0f7c1cc308a3fc603d8e079b91e5eb3171dbc3b79c55f32a511e6eaff76` |
| `src/dsp/AtropeloVoice.hpp` | `5de4ba9e7b8b89179eecee2e555150790839387fad6177ba2247dc4e9c4d1fc0` |
| `src/dsp/GendynOscillator.hpp` | `228c3de2e707fd40f5a75e93753fb4630e8a099a69fd77db144a39f7c46f1b06` |
| `src/dsp/AtariPunkConsole.hpp` | `fa54ea364e64a425e45409267bf3f6638e1d29738aebf71129972c9d80cc7d48` |
| `src/dsp/BreathExciter.hpp` | `794e9eaf88e4a8848ac0c1b176c2593fc60b488b8f99da6d2d36cc4320dedf48` |
| `src/dsp/ModalResonator.hpp` | `5e87d761f3f9b07ee337234b4e402fb6571e2dba44ce09df4f1593649628b996` |
| `src/dsp/DrumVoices.hpp` | `8d663ca3cdce6ce2e09c9cca2eba83850e14ac812e79bfda86b5bf1f7090271f` |
| `src/dsp/SallenKeyFilter.hpp` | `a5da0138067c47520f3ba6f35113abf501020556e6f926d5c1baa9119a1cd955` |
| `src/dsp/ColoredNoise.hpp` | `c0a9e5335fa5aaa15b2b9caf37cd6d57c34e36bf5adbeed91737487668d5cd00` |
| `src/dsp/Oscillator.hpp` | `28348a5341620fb2c5d54e3dd29cff7c67919437349cdc427a153606bd7b0885` |
| `src/dsp/RasgoFracture.hpp` | `83c92aefe7cccdb302ea498a3db26a6bd63768ab286970b2f0c739b0c92a0a0c` |
| `src/dsp/LfoBank.hpp` | `324b737587c71f1bc9892cf9b92b4db5d1a7bed043b413809ac7ecde456ca5d6` |
| `src/dsp/PolyrhythmDivider.hpp` | `e7b7f9adaaa83fc3e58960361a3a6c1737f4dfb328bb440d4f9391928431adff` |
| `src/dsp/GaloVoice.hpp` | `84aea1857c227b0022c2a53d3f0a607e7071bc95b41b53f79e273f7e770c414a` |
| `src/dsp/DejaVuVoice.hpp` | `e37fca4fe259674806e95aa064a1a8baefe9382a7b0f1a5ac71df5e93ebe5a60` |
| `src/dsp/MoogFilter.hpp` | `c470cba056d14e2922b6ccec5c17b60dd5f099b22a684f8cd83602fd9d599f18` |
| `src/dsp/HardSyncVoice.hpp` | `4e2b6e7b5a0b5f0d6c297a9800618093e2e444f1fbad214d34c216f43e6dddba` |
| `src/dsp/ScatVoice.hpp` | `e8921369409cdb46a7690e9adf239d3ed5d3298c3815f45a5b9e55ce2bd0cf36` |
| `src/dsp/PolivoksFilter.hpp` | `5b7731b44101fb711e54dc845e4eed3fcc65e30815d921e4cb494e6d50dba523` |
| `src/dsp/Envelope.hpp` | `ca1c9300ae0bd8727ee5c18c3a0054d67072cdfa82333513c33b7ea810693014` |
| `src/dsp/WalshOscillator.hpp` | `3161082f18ca18373e7d6cff5f12938035c0552caa2fac9e2c6a9e34948263f6` |
| `src/dsp/FmFluteVoice.hpp` | `9f8247fc5db3b6535fb57cee5c04664d065e21338526da0c24224fdf7123386c` |
| `src/dsp/AcidBasslineVoice.hpp` | `a608924a1b4cfb0dd724e5dabbe968a1b4c7346a3ccc95959986dd4f59eb9229` |
| `src/dsp/BuchlaRandomSource.hpp` | `065f4e761c8e8788edc79a0027d87e0fbe73c1a5052008e33866266a57d05d0f` |
| `src/dsp/ThereminVoice.hpp` | `ca5b5a97d855bc758111db38f625bd836d901e7992145766ba6c4185c998f7fd` |
| `src/dsp/Rungler.hpp` | `2ab0d6f79e6ea24ff70e32364be52d686637894d87f1e185bd59817c814f4dac` |
| `src/dsp/VosimOscillator.hpp` | `16fcd7e0de8402db33e82d588cddb2deb83ad2ecb5800429b0c9870be7735407` |
| `src/dsp/ChoirEnsemble.hpp` | `e6a99c8ecb8fc5bf2b57851a0e77eb764b4a7957ae6a4e24d9e451dad581dba3` |
| `LICENSE` | `8ceb4b9ee5adedde47b31e975c1d90c73ad27b6b165a1dcd80c7c545eb65b903` |
| `CREDITS.md` | `2202e18ff376612dbfcceb4e6e1fb17bf4bea98b649de570bd845b799c9ef3aa` |

## Acréscimos posteriores

| Arquivo | Origem | Commit | SHA-256 | Data |
|---|---|---|---|---|
| `src/dsp/FootnotePluck.hpp` | `RASGO_SYNTH_STUDIO/rasgo-synth-core/src/dsp/FootnotePluck.hpp` (monorepo `rasgo-instruments`) | `cfe1698` | `c5f61313f5a122a437a4560a5e793ed4904f108dd5b3c572d14efc455976148b` | 2026-10-30 — Footnote como estrela da biblioteca; cópia byte a byte, sem transformação (depende do `Oscillator.hpp` já vendorado, idêntico ao do Studio) |
| (lógica, adaptada fora do vendor) `include/rasgo/performance/PerformanceBluffSignal.hpp` | `RASGO_SYNTH_STUDIO/rasgo-synth-core/src/sequencer/BluffSignal.hpp` | `cfe1698` | `892832f4743a87f96bc7df990805b7e4daae99f4599adbc0316640b087682d9a` (fonte) | 2026-10-30 — Bluff Signal como estrela; adaptado: xorshift no lugar de `std::mt19937`, janela em batidas, reorganização depois da traição (ver o cabeçalho do arquivo) |
| (adaptado fora do vendor) `include/rasgo/performance/PerformanceSpectralObject.hpp` | `RASGO_SYNTH_STUDIO/rasgo-synth-core/src/dsp/SpectralObject.hpp` | `cfe1698` | `29b0f542cdbc232d4b942d87a47aaea254cabebaa4c20c5a418eb373eccfd233` (fonte) | 2026-10-30 — Spectral Object como estrela; adaptado para tempo real: sem `<random>`, `<sstream>`, `std::string`, `std::vector`; reafinação só quando muda; período-base medido nas batidas da estrela; parciais acima de 0,45·fs calados. Usa `Envelope.hpp` e `Oscillator.hpp` já vendorados (idênticos ao Studio) |

## Avisos de terceiros do snapshot (2026-11-04)

`CREDITS.md` (desta pasta) aponta para `THIRD_PARTY_NOTICES.md`, que não
tinha vindo na cópia de 11/10 — link quebrado, pendência de publicação
anotada desde então. Trazido agora do mesmo commit, sem nenhuma alteração:
`RASGO_SYNTH/THIRD_PARTY_NOTICES.md` em `8fc69f5` (repositório RASGO),
sha256 `d5a6d23ad1d73dc948802b46fc9d6e3af41ad64b8d07ffd677d1d95848e85e46`.
Conferido também que `CREDITS.md` e `LICENSE` desta pasta são idênticos aos
do commit. O arquivo cita um `DEPENDENCIES.md` que é o do Studio na origem
(não copiado): as dependências do Asterismus estão em `DEPENDENCIES.md` e
`THIRD_PARTY_NOTICES.md` na raiz do projeto.
