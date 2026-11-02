# Host standalone — Marco 4 inicial

**Estado:** host próprio compilado localmente; validação visual/dispositivo
pendente de sessão gráfica.

## Decisão de licença

- código autoral: GNU GPLv3, igual ao Rasgo Synth Studio;
- framework do host: via AGPLv3 do JUCE, sem licença comercial registrada;
- checkout JUCE: cópia própria em `third_party/JUCE/`, JUCE 9.0.0; a licença
  local tem SHA-256 `a37eaefe4f160da98bc07d2e8d951e46a9b52abd6e23b233ffcadf003d690f93`.

## O que existe

O target opcional `RasgoSynthPerformanceApp` abre um dispositivo de áudio,
prepara `PerformanceSessionController`, carrega um território/cena próprios de
partida e chama `processBlock`. A janela já contém uma primeira superfície de
ação com quatro cenas e cinco gestos, preservando a linguagem verde/cobre; ela
ainda não pretende ser o painel de performance final.

O app é desativado por padrão. O núcleo puro permanece compilável/testável sem
JUCE:

```bash
cmake -S . -B build-contracts
cmake --build build-contracts
ctest --test-dir build-contracts --output-on-failure
```

Com o checkout próprio registrado:

```bash
cmake -S . -B build-app -DRASGO_PERFORMANCE_BUILD_APP=ON \
  -DRASGO_PERFORMANCE_JUCE_ROOT="$PWD/third_party/JUCE" \
  -DRASGO_PERFORMANCE_MODAL_MATTER_PACKAGE="../rasgo-synth-core/cmake/modal-matter" \
  -DRASGO_PERFORMANCE_MODULE_COHORT_PACKAGE="../rasgo-synth-core/cmake/performance-cohort-v1"
cmake --build build-app
```

Os dois packages são opcionais e explícitos. Quando presentes, configuram
somente o retorno modal v1 e a coorte Performance v1 com hashes fixados; não
configuram nem compilam o app Studio.

## Limitações honestas

- o host foi compilado em 2026-08-24 no binário
  `build-app/src/app/RasgoSynthPerformanceApp_artefacts/Rasgo Synth Performance`
  (28.814.016 bytes); a suíte CTest do build passou 6/6;
- uma execução curta foi tentada com esse binário, mas esta sessão não dispõe
  de display gráfico para confirmar janela nem dispositivo de áudio. Não há
  validação visual, de CPU/latência ou de escuta humana registrada;
- campo XY, 16 células, MIDI, monitor, telemetria de CPU/xrun e gravação
  seguem pendentes; cenas e gestos básicos estão documentados em
  `PAINEL_PERFORMANCE.md`;
- no primeiro host GNU/Linux, XInput é desativado para não exigir `libxinput-dev`;
  isso limita recursos avançados de touch/apontamento, não o caminho de áudio;
- não há dependência de código, assets, configuração ou runtime do Studio.

## Atualização de operação — 2026-08-30

O fluxo canônico do host é o preset `linux-release-host`, que aponta para o
checkout próprio do JUCE e para `modal-matter`/coortes V1–V4 em
`../rasgo-synth-core/cmake`:

```bash
cmake --preset linux-release-host
cmake --build --preset linux-release-host -j1
ctest --preset linux-release-functional
```

O host Release compilou; 12/12 testes funcionais passaram e a estabilidade
passou em 41,28 s. O lançador verifica volume gravável, memória mínima e sessão
gráfica acessível. `check_runtime_artifacts.sh` verifica resíduos sem apagar
arquivos. A janela, HiDPI, áudio físico, Windows e macOS continuam pendentes de
validação própria.
