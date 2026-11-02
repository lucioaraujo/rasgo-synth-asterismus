# Contrato — orquestra interna de cada stem

**Estado:** primeira realização bounded integrada — 2026-09-07

## Princípio

Uma stem é uma unidade de presença no Campo e uma unidade de orquestração.
Ela pode reunir módulos em alternância, sobreposição, resposta, infiltração e
memória. Não equivale a um único timbre, nem a um conjunto de processos
permanentemente ligados.

O músico continua vendo e manipulando um único astro/stem: posição, vértice,
cabos, presença, mute, solo e ganho pertencem à stem. A mistura interna jamais
pode mover esse astro, apagar o polígono, reabrir/fechar um vértice, trocar
cenas ou alterar relações sem gesto/intenção explícita.

## Repertório por stem

Para uma seed e um número de stems liberadas, cada stem recebe:

1. **módulo de abertura** — sua voz inicial, escolhida pela coorte da seed;
2. **candidatos afins** — módulos triados por função, família, coorte, papel
   orquestral e contraste necessário;
3. **regras de entrada** — acontecimento de seed, cena, gesto, memória,
   relação ou coda que pode convidar cada candidato;
4. **regra de saída** — duração, envelope, cauda e retorno ao estado anterior.

Uma biblioteca com vinte módulos e quatro stems liberadas deve poder formar,
em média, cinco candidatos por stem. A distribuição não obriga homogeneidade:
uma stem pode ter mais matéria rítmica, outra mais vozes/memórias, desde que a
seed preserve equilíbrio de função e haja justificativa musical para a
diferença.

## Estados de camada

Cada módulo dentro de uma stem assume exatamente um estado:

| Estado | Consequência |
|---|---|
| `reserva` | integra o repertório, não entra no submix ainda |
| `entrada` | inicia por envelope/crossfade, nunca por salto de ganho |
| `ativo` | participa do submix e responde ao Campo |
| `cauda` | permanece apenas pelo tempo musical/decay definido |
| `memória` | pode reaparecer transformado por vestígio, relação ou cena |
| `recolhido` | não produz áudio, mas não perde seu vínculo com a seed |

Nenhuma camada pode ficar ativa por omissão. Sustentação exige função musical,
evento que a renove e uma rota de saída. Isto preserva a correção já feita
contra drones contínuos e nutrição permanente.

## Mistura e continuidade

- A mudança de módulo ocorre por crossfade ou sobreposição de cauda bounded;
  nunca por desligamento seco do áudio.
- O submix respeita ganho, mute, solo, presença e coda da stem que o contém.
- Relações do Campo incidem na escolha, no peso e no timbre das camadas, mas
  não recriam conexões nem reconfiguram a geometria do performer.
- `MEMÓRIA` favorece retorno transformado; `RUPTURA` pode trocar/fragmentar;
  `AUTONOMIA` dá continuidade narrativa quando o músico para; `INTERVENÇÃO`
  prevalece sempre que o músico atua.
- `STUTTER` e `TEMPO SHIFT` tratam a camada ou a stem em foco como acontecimento
  temporal bounded, não como estado global que paralisa a peça.

## Orçamento técnico interno

O número de módulos audíveis simultaneamente é uma decisão do motor, não uma
limitação exposta ao músico. Cada perfil de hardware precisa declarar:

- número de stems liberadas;
- máximo de camadas simultâneas por stem;
- caudas/transições simultâneas permitidas;
- sample rate e buffer testados;
- CPU de callback, XRUNs, pico, memória e duração observados.

O perfil atual de produção continua em quatro stems até ensaio real aprovado.
A camada de mistura interna será liberada por degraus de teste e só entra no
Release quando o host permanecer estável em sessão longa. O contrato não
autoriza inferir capacidade a partir do número de núcleos da CPU.

## Contratos de código exigidos

A implementação deverá introduzir, no Performance, estruturas bounded e sem
alocação no callback para:

1. plano de repertório por stem (módulo de abertura + candidatos);
2. seleção determinística por causa: seed, cena, gesto, memória, relação ou
   coda;
3. estado/envelope de cada camada;
4. submix por stem, antes do mixer já existente de ganho/mute/solo;
5. telemetria: módulo em foco, camadas ativas, caudas e recusas por orçamento;
6. teste de que uma troca não move posição, não muda cabo e não interrompe
   áudio; e teste de que o orçamento impede crescimento de CPU/memória.

Nenhum adapter do Studio será alterado. Quando for necessário reutilizar DSP ou
comportamento, a cópia/adaptação será feita exclusivamente dentro de
`rasgo-synth-performance`, com proveniência, contrato e teste próprios.

## Realização inicial: Modal como memória proprietária

No orçamento de quatro stems, a matéria `MODAL` já realiza o estado `memória`
sem criar uma quinta voz: pertence primeiro ao destino do cabo de `MEMÓRIA`,
depois a `VESTÍGIO` e, por fim, a um corpo vivo. Não existe `return` global
fora da constelação. A entrada respeita mute, solo, ganho, vida e coda da stem
proprietária; o Campo preserva as mesmas posições, vértices e cabos.

O turno narrativo pode excitá-la uma vez e alterar a cor local do módulo
atingido. O relógio autônomo é proibido nessa realização interna e permanece
reservado à quinta stem Modal explicitamente liberada. A curta abertura durante
o repouso de uma frase serve apenas para que o evento seja audível; o decay da
matéria encerra a cauda. A regressão garante ausência de retorno global e
consequência sonora entre duas intensidades da mesma relação de memória.

## Critério de pronto

O primeiro marco estará pronto quando quatro stems puderem alternar e combinar
módulos de seus repertórios durante uma peça, com consequência musical clara,
sem drones involuntários, XRUN recorrente, clipping, corte de áudio, perda de
posição ou destruição do polígono desenhado pelo performer.

## Estado da primeira integração 8→4 — 2026-09-08

O adaptador publica oito lanes internas e o `PerformanceStemBusMixer` agrupa-as
em quatro buses. As camadas adicionais são controladas pelo ganho e pelo
estado de mute/solo da stem proprietária, mantendo o Campo independente do
roteamento de saída.

Na abertura, Circuito ocupa até quatro lanes; quando a seed oferece somente
três corpos Circuito, a quarta lane recebe o terceiro corpo complementar. As
quatro lanes seguintes recebem matérias complementares de Sopro/Voz, Digital e
Memória, mantendo oito sinais internos finitos e audíveis antes da soma 8→4.

Validação técnica: motor, camadas internas, adapter de oito lanes e mixer 8→4
passaram; o smoke de 60 s não alocou no `processBlock` e atingiu 7,53× realtime
offline. A integração ainda precisa dar lanes próprias a todos os candidatos
do catálogo e passar por escuta humana antes de aumentar a densidade simultânea.
