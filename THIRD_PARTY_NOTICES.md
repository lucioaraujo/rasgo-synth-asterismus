# Avisos de terceiros — Rasgo Synth Asterismus (ex-Performance)

## Código autoral

O código do Rasgo Synth Performance é distribuído sob GNU GPLv3, no arquivo
[`LICENSE`](LICENSE), igual ao regime do Rasgo Synth Studio.

## JUCE para o host standalone

O host opcional usa JUCE somente pela via **AGPLv3**. Não há licença comercial
JUCE registrada para este projeto. Desde 15/11/2026 a versão é a JUCE
9.0.3 (a comum da família RASGO; antes 9.0.0), com SHA-256 de `LICENSE.md`
`6318e8f9bfcd5471de3608ea1ed10953279e912b9c76b78d55575b90f6bf1bf2`.
O CMake não possui fallback para `RASGO_SYNTH/JUCE-master` nem para arquivos do
Studio.

JUCE e seus avisos de dependências permanecem propriedade de seus titulares e
devem acompanhar uma distribuição conforme os termos aplicáveis da revisão
efetivamente utilizada.

## Tipografia do cabeçalho

O host inclui Big Shoulders Display e IBM Plex Mono como recursos locais do
Performance. Ambas estão sob SIL Open Font License 1.1; os textos integrais
acompanham respectivamente `assets/fonts/LICENSE-BigShoulders.txt` e
`assets/fonts/LICENSE-IBMPlexMono.txt`. A proveniência, hashes e decisão de
uso estão em [`assets/README.md`](assets/README.md). A logo
`assets/rasgo_logo_2026.svg` é arte do projeto RASGO, de Lucio Araújo (2026),
e não é recurso de terceiros.

## Rasgo Synth Modal Matter v1

O build musical atual pode ligar opcionalmente o target GPLv3
`rasgo::modal_matter_v1`, fornecido pelo core irmão e limitado ao header
autoral `dsp/ModalResonator.hpp`. O package verifica o SHA-256
`5e87d761f3f9b07ee337234b4e402fb6571e2dba44ce09df4f1593649628b996`.
A inspiração conceitual em Rings/Mutable Instruments permanece crédito de
pesquisa; nenhum firmware ou fonte Rings foi incorporado ao adapter.

## Master de saída — cópia adaptada do Antitotem (30/10/2026)

`include/rasgo/performance/PerformanceMasterStage.hpp` é cópia adaptada de
`ANTITOTEM/src/core/OutputStage.h` (o `SignalLeveler.h`, usado na primeira
versão, foi retirado no mesmo dia),
repositório próprio do Antitotem, commit `645e5ad`, mesma autoria (Lúcio de
Araújo / RASGO). O arquivo herda a **GNU AGPL-3.0-or-later** do Antitotem; o
código autoral do Asterismus segue GPLv3, e a combinação é permitida pela
seção 13 da GPLv3 (como já ocorre com o JUCE pela via AGPLv3). Decisão do
autor, se quiser: relicenciar este trecho para GPLv3 (é titular dos dois).
Detalhes em `docs/SAIDA_DE_AUDIO.md`.

## Medidor de saída — cópia adaptada do Navalha 2 (30/10/2026)

`include/rasgo/performance/PerformanceLoudnessMeter.hpp` é cópia adaptada da
ponderação K (ITU-R BS.1770-4) de `NAVALHA2_JUCE/src/core/MasteringAnalysis.cpp`
(commit `b1a9614`) e do detector de true peak 4× de
`NAVALHA2_JUCE/src/core/TruePeakDetector.*` (commit `34f8fd7`), repositório
próprio do Navalha 2, GPLv3, mesma autoria (Lúcio de Araújo / RASGO).

## Receptor do Maestruc — cópia do Maestruc (15/11/2026)

`src/app/MaestrucLink.h` e `src/app/PonteDeSom.h` são cópias do receptor de
referência do Maestruc (`RASGO/PROJETOS_AUTORAIS/MAESTRUC/receptor/`, monorepo
privado `rasgo-instruments`), mesma autoria (Lúcio de Araújo / RASGO); a
origem e o SHA-256 estão no cabeçalho de cada um. Os arquivos levam a
**GNU AGPL-3.0-or-later** do Maestruc; a combinação com o código GPLv3 do
Asterismus é permitida pela seção 13 da GPLv3, como no master de saída
acima. O receptor só liga com `--maestruc`; sem ele, o app não abre porta
nem ponte.

## Conceitos citados (sem código de terceiros)

O movimento físico, o tempo maleável e a trama citam trabalhos acadêmicos
como fontes de conceito, sem código copiado: ver as seções de fontes de
`docs/MOVIMENTO_FISICO.md`, `docs/TEMPO_MALEAVEL.md` e
`docs/TRAMA_DE_RELACOES.md`. Conceitos do Batruc (especificação de ritmos,
movimento gráfico) foram só lidos.
