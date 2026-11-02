# Auditoria de expansão da mesa — 2026-08-30

## Resultado

### Estado após a implementação incremental (30-08-2026)

O primeiro degrau da mesa acumulativa foi implementado e validado: a seed
Circuito (`181`) inicia com três corpos e prepara o Walsh como quarto corpo
dormente; ele pode ser inserido ou recolhido durante PLAY, receber controles
contextuais, posição e intensidade da órbita sem reiniciar o transporte. A
ecologia aplica simbiose espacial bounded e o teto operacional atual é de
quatro corpos (`kPerformanceSafeBodyLimit`). O quinto slot continua reservado;
isto não autoriza afirmar suporte aos quase 20 stems simultâneos do Studio.

A mesma fronteira agora possui uma segunda variação auditada: seeds Circuito
alternadas (como `186`) preparam `HardSync` no quarto slot. Não é um aumento do
teto, mas uma confirmação de que o slot acumulativo não está preso a um único
gerador.

Os testes funcionais não-stress passaram (9/9) e o benchmark offline confirmou
3 corpos a 8,36× e 4 corpos a 7,45× tempo real, sem alocações no callback. A
medição de dispositivo de áudio real e a validação Windows/macOS permanecem
pendentes.

Situação da ordem de migração: os itens 1–5 foram executados para o primeiro
caso Walsh (capacidade bounded, contratos, engine/controlador, adapter, uma
relação espacial e painel contextual); o item 6 também foi exercitado com
inserção/recolhimento durante PLAY. A generalização para outras famílias e o
item 7 (medição em hardware de áudio real) continuam sendo o próximo ciclo.

Atualização posterior, 2026-09-01: a generalização do quarto slot para as
ecologias já adaptadas foi concluída dentro do mesmo teto de quatro corpos.
Circuito prepara Walsh ou Hard Sync; Sopro prepara Atari Punk; Voz prepara
Walsh; Digital prepara VOSIM; Memory prepara Hard Sync. Cada possibilidade
possui reserva, identidade, controles, relação, stem e teste próprios. Isso
não equivale a admitir qualquer combinação da biblioteca: a mistura arbitrária
entre coortes continua condicionada a um adapter/mixer multi-coorte e a novo
benchmark.

### Fotografia inicial da auditoria

O Performance atual é coerente como protótipo de quatro corpos audíveis e mesa
acumulativa estrutural em expansão. A capacidade dos contratos já é maior: 8
atores na constelação, 16 relações, 5 stems de telemetria e 96 potenciais de
território. A restrição de áudio aparece principalmente no
adapter sonoro, na fábrica inicial e na interface:

- `PerformanceContracts.hpp` e `PerformanceCumulativeDesk.hpp`: reservam 8
  atores/slots e 16 relações, com identidade, origem da biblioteca, posição,
  presença e recolhimento sem compactação; a ligação desses slots ao áudio ainda
  está pendente;
- `PerformanceTerritoryFactory.hpp`: três atores iniciais e três relações;
- `PerformanceSessionEngine.hpp` / `PerformanceSessionController.hpp`:
  telemetria, solo/mute e comandos ainda precisam aceitar crescimento em tempo
  de execução;
- `PerformanceActorEcology.hpp`: reserva 5 vozes/stems internos, mas recebe do
  adapter uma moldura de apenas 3 corpos sonoros; essa diferença deve ser
  resolvida antes de afirmar que a quinta voz está audível;
- `PerformanceConstellationGeometry.hpp`: projeção triangular de três pontos;
- `PerformanceModuleCohortAdapter.hpp`: frequências, corpos e controles em
  matrizes 3×3;
- `PerformanceApp.cpp`: três cartões, três painéis contextuais, três stems,
  três relações e layout triangular;
- testes: várias asserções de `actorCount == 3`, `stemCount == 3` e relações
  entre os atores 1/2/3.

Atualização incremental: a moldura de saída do adapter agora reserva 5 corpos e
declara explicitamente `bodyCount`; as coortes atuais continuam publicando 3.
O Ecology só consome os corpos declarados, preservando o som existente. Engine e
adapter tests passaram após a mudança; nenhum quarto módulo foi inventado ainda.

Nesta sequência, a fronteira de entrada também passou a aceitar um quadro
bounded de cinco frequências. O overload de compatibilidade reduz esse quadro
aos três slots das coortes auditadas; assim, a mesa pode crescer sem ativar um
quarto gerador por engano. A primeira exceção controlada já existe em
`PerformanceTerritoryFactory::makeWithWalsh`: o Walsh é preparado como quarto
corpo dormente e o adapter publica seu sinal no slot 4 para essa variante. A
A inserção/recolhimento do quarto corpo já é acionável no painel durante PLAY.
O próximo trabalho é generalizar essa mesma fronteira para a biblioteca completa
e fazer o controlador transportar a admissão de cada slot até o adapter, sem
confundir capacidade estrutural de oito entradas com o teto realtime seguro de
quatro corpos.

## Consequência

Não é seguro apenas aumentar um número ou acrescentar um cartão. A mesa precisa
de uma capacidade bounded explícita, identificadores estáveis, armazenamento de
presença/posição/controles por slot, matriz de relações e mixagem simbiótica
dependente de distância. O áudio deve preservar segurança de CPU, xruns,
memória e valores não finitos.

## Ordem de migração proposta

1. Definir a capacidade inicial do Performance (primeiro marco: quatro corpos,
   sem prometer os 23 stems catalogados do Studio).
2. Generalizar contratos e documentos para slots/stems opcionais, mantendo
   compatibilidade com territórios de três corpos.
3. Generalizar engine/controlador e filas sem alocação no callback.
4. Generalizar adapter e relações, com uma única interação espacial A/B.
5. Generalizar geometria e painel, abrindo controles contextuais somente para o
   corpo selecionado.
6. Adicionar inserção, recolhimento e remoção durante PLAY.
7. Medir CPU/xrun/memória em quatro corpos e só então considerar capacidade
   superior.

O Studio/Core continua sendo a fonte de conceitos e mecanismos; o código do
Studio não será alterado.

## Atualização de estado — 2026-08-31

A semântica acumulativa foi implementada em `PerformanceCumulativeDesk` e
ligada ao controlador por `addDeskModule`, `addLibraryModule` e
`removeDeskModule`. A procedência agora usa IDs estáveis da biblioteca e a seed
resolve o corpo opcional correto. A fábrica expõe `makeWithOptionalModule`, com
`makeWithWalsh` preservado como alias.

Walsh, Hard Sync e a primeira expansão Digital possuem caminho completo de
território, controle, posição, relação, adapter e teste. No Digital, VOSIM entra
como quarto corpo real, preparado pela seed e mantido dormente até a inserção;
ele não é uma duplicação de Walsh, Hard Sync ou Atari Punk. Sopro, Voz e Memory
seguem na condição de catálogo e coorte existentes, mas sem admissão
acumulativa de novo corpo. Build Release e suíte funcional passaram em 16/16
testes.

### Lista de tarefas vigente

1. Ouvir e medir a combinação Digital + VOSIM em dispositivo real.
2. Testar a mesma fronteira de identidade e recolhimento em uma sessão humana.
3. Medir CPU, memória, xruns e latência antes de alterar o teto de quatro
   corpos.
4. Repetir o processo para Sopro, Voz e Memory somente após o caso Digital ser
   musicalmente e tecnicamente validado.

### Evidência de estabilidade Digital — 2026-08-31

O teste de estabilidade recebeu o modo `digital` e executou 20 minutos
simulados da seed 184 com VOSIM no quarto slot. Resultado: 42,7092 s de CPU,
28,10× tempo real, quatro stems estáveis, zero alocações em `processBlock` e
zero xruns. Isso confirma segurança estrutural/offline do caminho; não substitui
a medição em dispositivo de áudio real nem a escuta musical humana.
