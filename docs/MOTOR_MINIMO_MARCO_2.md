# Motor mínimo — Marco 2

**Estado:** primeira implementação técnica validada; ainda não é motor musical
nem adaptador do Studio.

`PerformanceSessionEngine` implementa o contrato incremental previsto:

```cpp
prepare(sampleRate, maxBlockSize, channels)
setSessionSnapshot(snapshot)
pushGesture(event)
processBlock(left, right, numSamples)
readTelemetry()
reset()
```

Ele agora usa uma pequena ecologia nativa de três a cinco atores — `Chão`,
`Corpo`, `Atrito`, `Contágio` e `Vestígio` — e relações de tempo, matéria e
memória. As vozes e suas formas são doubles de áudio próprios: provam
continuidade, papéis, relações, morph, agendamento e segurança sem antecipar a
adaptação de flauta, tuba, acid, bateria ou qualquer outro corpo do Studio.

## O que o motor já demonstra

- saída mono ou estéreo limitada e livre de não-finitos;
- frequência e ganho suavizados para evitar troca descontínua de cena/gesto;
- snapshots de cena e gestos discretos: Hold, Mutate, Rasgar, Interrupt,
  Stutter, TempoShift, Recall e Silence; PanicSafe também é testado;
- até cinco atores próprios com corpos distintos por papel e um morph entre duas
  constelações, incluindo mudança explícita de `Corpo` para `Contágio` e de
  `Atrito` para `Rasgo`;
- relações `Support/Time` e `Friction/Matter` participam da realização dos
  atores, sem uma malha irrestrita de modulação;
- fila fixa de 128 gestos, sem alocação em `processBlock()`;
- evento agendado por timestamp absoluto + offset de bloco;
- telemetria de pico, fila, cena, ator de teste, xruns e PanicSafe;
- recusa de buffer, canal ou snapshot inválido.

### Recortes temporários — 2026-09-02

`Stutter` e `TempoShift` são gestos bounded sobre corpos já preparados. O
primeiro aplica uma porta rítmica suavizada; o segundo desloca transitoriamente
fase/pulso local e não escreve o andamento da sessão. Ambos podem apontar para
um ator ou para o coletivo, duram no máximo 0,8 s e não alteram geometria,
capacidade ou cenas. A regressão A/B confirma alteração de PCM e preservação de
posição/BPM; a escuta humana continua necessária para avaliar click e resultado
musical.

## Limites conhecidos

- ainda não há clock musical, MIDI, OSC, gravação, GUI/host ou medição de
  CPU/latência em dispositivo real; cenas persistidas e seu controlador próprio
  agora existem fora deste motor;
- a simulação offline de 20 minutos em 48 kHz/256 samples completou em 14,25 s
  (84,19× tempo real), sem xruns e com zero alocações dentro de `processBlock()`;
- `PerformanceSessionDocument` e `PerformanceSessionController` são camadas de
  controle próprias e ficam fora do callback deste Marco 2;
- nenhuma dependência do Studio foi introduzida.

## Validação atual

- Release: 6/6 CTests aprovados;
- AddressSanitizer: testes funcionais aprovados com `detect_leaks=0`, porque o LeakSanitizer
  não funciona sob o rastreamento desta sessão;
- UndefinedBehaviorSanitizer: testes funcionais aprovados.

Próximo incremento: medir CPU/latência em dispositivo real quando existir o
host próprio e, antes do primeiro adaptador Rasgo, definir quais capacidades
incrementais de uma única matéria serão auditadas.
