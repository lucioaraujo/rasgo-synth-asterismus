# Contratos puros — Marco 1

**Estado:** implementado e testado em 2026-08-24
**Código:** `include/rasgo/performance/PerformanceContracts.hpp`
**Limite:** sem JUCE, sem DSP, sem inclusão de `rasgo-synth-app` ou
`rasgo-synth-core`.

## O que está codificado

| Contrato | Responsabilidade | Invariantes principais |
|---|---|---|
| `PerformanceTerritory` | o que a seed torna possível | identidade bounded, até 96 atores, IDs únicos e capacidades declaradas |
| `PerformanceActorPotential` | capacidade e papéis possíveis de um ator | família material, capabilities, presença mínima/máxima e papéis permitidos |
| `PerformanceConstellation` | recorte vivo do território numa cena | até 8 atores, papéis permitidos e relações somente entre atores presentes |
| `PerformanceRelation` | apoio, atrito, contágio, reparo, órbita etc. | fonte/destino distintos, domínio, intensidade, latência, duração e limite bounded |
| `PerformanceSessionState` | pressão global da sessão | tempo 20–300 BPM, macros e trim bounded, clock/segurança explícitos |
| `PerformanceGestureEvent` | gesto musical agendado | tipo, timestamp, offset estritamente dentro do bloco, intensidade/duração/condição bounded; `variationToken` opcional só é preparado fora do callback e só colore `MUTATE` |
| `PerformanceScene` | sessão + constelação dentro de um território | a seed da sessão coincide com o território e todos os vínculos são válidos |
| `PerformanceSceneMorpher` | transição determinística | macros interpolados; presença/relações entram e saem gradualmente; mudança de papel permanece explícita; territórios distintos são rejeitados |
| `FixedEventQueue` | passagem SPSC de eventos sem alocação | capacidade fixa, FIFO e uma posição reservada contra sobrescrita |
| `PerformanceTelemetryMailbox` | observação leve da sessão | snapshot atômico quantizado de CPU, picos, xruns, cena, atores, fila e PanicSafe |

## Relação com a estratégia musical

O código separa família de papel. `Tone`, `Hit`, `Texture`, `Pattern`,
`Rupture` e `Relation` descrevem capacidade material. `Chão`, `Corpo`,
`Atrito`, `Contágio`, `Rasgo` e `Vestígio` descrevem função musical contextual.
Assim, um mesmo ator pode mudar de papel sem fingir que todos os módulos têm a
mesma matéria.

O teste de cena usa a mesma seed `181` para dois percursos de nutrição. Ele não
produz áudio — isso pertence ao Marco 2 —, mas verifica o contrato necessário:
território e identidade permanecem comuns, enquanto risco, presença e nutrição
podem gerar trajetórias distintas de forma determinística.

## Validação executada

- CMake 3.28.3 e GNU C++ 13.3.0;
- build Release do projeto isolado;
- `ctest --output-on-failure`: 1/1 aprovado;
- execução direta do binário de teste: aprovada;
- verificações de duplicação, papéis incompatíveis, relação sem destino,
  valores fora de faixa, offset de bloco inválido, FIFO, telemetria e morph;
- nenhuma inclusão de JUCE, `rasgo-synth-app` ou `rasgo-synth-core` no target.

## O que não foi implementado

- formato persistente `PerformanceSessionDocument`;
- motor `prepare/processBlock/reset`;
- geração DSP, áudio contínuo, MIDI, OSC, GUI, recorder ou adaptador do core;
- bridge Studio↔Performance.

O próximo marco é um motor mínimo com fonte sonora própria/double de teste. Ele
deve consumir estes contratos e demonstrar três cenas e cinco gestos sem click,
antes de qualquer adaptação de módulos do Studio.
