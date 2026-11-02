# Auditoria funcional completa — 2026-08-27

## Escopo e método

Esta auditoria responde a três perguntas diferentes para cada função do Rasgo
Synth Performance:

1. existe uma ação de painel/atalho capaz de acioná-la;
2. o comando chega ao motor e altera estado ou DSP;
3. a consequência corresponde ao rótulo e tem força suficiente para tocar.

Foram rastreados `PerformanceApp` → `PerformanceSessionController` →
`PerformanceSessionEngine` → `PerformanceActorEcology` → adapters DSP. Duas
sondas temporárias em `/tmp` compararam as seeds 181/182/183 e reproduziram
inconsistências de território/estado. Elas não fazem parte do projeto. Os 8/8
CTests não-stress existentes continuaram aprovados. O app não foi aberto; logo,
tipografia, sobreposição, percepção musical e gravação em dispositivo real não
foram declaradas validadas.

## Resultado executivo

Não há botões principais totalmente vazios, mas há quatro classes de problema:

- **falhas reais de estado:** UI e áudio podem representar seeds ou mixer
  diferentes;
- **funções parciais:** Capturar, XY e contratos prometem mais do que guardam ou
  executam;
- **consequências fracas:** Memória, Hold, Recall e controles locais específicos
  alteram sinal, porém muito menos que seus vizinhos;
- **contratos ainda sem realização:** vários campos são validados/serializados,
  mas nunca chegam ao comportamento musical.

## P0 — incorreto ou capaz de divergir UI e áudio

### 1. Seed 0 mostra VOZ e renderiza CIRCUITO

O editor aceita zero. `cohortForSeed(0)` retorna VOZ, usado pelo painel, mas
`PerformanceModuleCohortAdapter::setSeed(0)` normaliza internamente para 1 e
renderiza CIRCUITO. Reprodução da sonda:

```text
seed-0-ui-cohort=2  dsp-cohort=0  normalized-dsp-seed=1
```

Consequência: cartões Scat/Choir/VOSIM controlam corpos Acid/Gendyn/Drums.

### 2. `NOVO TERRITÓRIO` pode ser perdido por PLAY imediato

Depois de STOP, o host aceita e mostra a nova seed antes de o áudio instalar o
território. No próximo bloco, comandos de transporte são processados antes do
comando de território. Se PLAY já estiver na fila, o motor volta a tocar e
descarta o território. Reprodução:

```text
territory-queued=1  immediate-play-queued=1  installed-seed=181  playing=1
```

O painel, entretanto, já pode mostrar 182/SOPRO. O teste atual cobre apenas o
caminho com um bloco de áudio entre território e PLAY.

### 3. Mute/solo antigos atravessam a seed nova enquanto a UI os apaga

`setScene` preserva mixer quando os actor IDs continuam 1/2/3. O host, ao trocar
território, limpa seus arrays visuais de mute/solo sem mandar os resets ao
motor. Reprodução após trocar 181 → 182:

```text
new-territory-stem-gain-3=0.23  muted-2=1  solo-id=1
```

O ganho antigo também atravessa, mas seu slider não é resetado; mute/solo são o
caso perigoso porque tela e áudio discordam.

### 4. Filas podem perder o estado final sem a interface admitir

XY envia quatro macros por evento de mouse; presença, relações e controles
locais também podem produzir eventos rapidamente. Vários callbacks ignoram o
`false` devolvido por filas cheias e atualizam ponto, slider, painel e “última
ação” mesmo assim. O último gesto do mouse pode, portanto, existir somente na
tela. `requestHold` também troca o rótulo antes de saber se o gesto entrou.

### 5. Documento do controlador é compartilhado sem sincronização integral

O áudio altera `document_` ao aplicar captura/território enquanto a GUI o lê em
recall, validação de atores/relações e limites de macros. A fila protege o
comando, mas não protege o documento depois da aplicação. É uma data race C++ e
deve ser resolvida antes de save/load, MIDI ou novas constelações.

## P1 — funciona parcialmente ou não corresponde à promessa

### Seed e sessão

- A seed 181 inicial é criada por `makeStartupDocument`; digitar novamente 181
  usa `PerformanceTerritoryFactory`. A mesma seed não reconstrói o mesmo início.
- Posições dos nós, mixer e controles locais não possuem política única de
  reset/preservação ao trocar território. Cada subsistema hoje faz algo
  diferente.
- A biblioteca de módulos e os descriptors usados pelo runtime são registros
  separados; podem voltar a divergir sem um teste cruzado.

### Capturar / Desfazer

`CAPTURAR` guarda macros, tempo, presenças e intensidades das três relações.
Não guarda:

- controles locais dos módulos;
- posição dos nós;
- ganho/mute/solo dos stems;
- GRID, Hold, morph, master ou estado dos gestos.

Parte disso pode ser global por decisão, mas o tooltip “controles atuais” é
amplo demais e não há indicação visual do que pertence à cena. Controles locais
nem marcam `MOD`. `DESFAZER` restaura somente a última captura em memória e não
é persistente após fechar o app.

### Constelação e relações

- Arraste, presença e intensidade das três relações chegam ao áudio.
- Posição → intensidade funciona; intensidade recuperada por cena → posição não
  funciona. Após recall, espessura e valor do cabo mudam, mas distância entre
  nós pode continuar contradizendo a relação.
- O cabeçalho chama `activeActorCount` de ATORES, mas o valor é a quantidade de
  vozes alocadas na cena, incluindo módulo na reserva com presença zero.
- Descriptors declaram quais domínios cada módulo aceita, mas essa metadata não
  governa o roteamento DSP.

### Campo XY

O gesto envia quatro macros:

```text
x -> autonomia = 1-x, intervenção = x
y -> memória = 1-y, risco = y
```

Entretanto, o ponto desenhado é reconstruído somente com Intervenção e Memória.
Alterar Autonomia ou Risco pelos knobs não move o ponto; limites territoriais
também quebram a complementaridade. XY funciona como emissor, mas não como
representação fiel do estado dos quatro macros.

### Coortes v2 e gestos globais

SOPRO/VOZ renderizam corpos reais, mas `renderAlternative` não recebe Memória,
Intervenção, Mutate ou Rasgar diretamente. Esses estados ainda aparecem na
camada provisória de 16%, retorno modal, ganho ou espacialização, portanto o A/B
global muda, mas o corpo principal não é subvertido por eles como no CIRCUITO.
Isso explica por que um gesto pode medir diferença e ainda parecer desligado do
módulo que o performer está ouvindo.

## Força observada dos controles sonoros

A coluna abaixo é distância estrutural normalizada entre duas renderizações
determinísticas, não porcentagem de volume nem teste psicoacústico. Valores
menores que aproximadamente `0,05` foram classificados como fracos para uma
superfície de palco.

| Função | 181 CIRCUITO | 182 SOPRO | 183 VOZ | Diagnóstico |
|---|---:|---:|---:|---|
| Memória 0,1→0,9 | 0,021 | 0,011 | 0,014 | funciona, mas é o macro mais fraco |
| Hold | 0,072 | 0,089 | 0,041 | audível por teste; fraco em VOZ |
| Recall limpo | 0,032 | 0,048 | 0,055 | contorno pequeno; reparo de cicatriz é mais claro |
| Interrupt | 0,023 | 0,027 | 0,039 | fraco e sem ação no painel |
| Tempo 60→220 | 0,057 | 0,163 | 0,123 | funciona; discreto em CIRCUITO |
| Relação Matéria | 0,190 | 0,189 | 0,092 | funciona; mais fraca em VOZ |
| Relação Tempo | 0,928 | 0,841 | 0,878 | muito forte |
| Relação Espaço | 0,255 | 0,255 | 0,389 | clara |

Controles locais especialmente fracos:

- CIRCUITO/Acid: `CORTE` 0,014; `ARTIC` 0,010;
- CIRCUITO/Drums: `FRAG.` 0,006; `ABERT.` 0,008;
- SOPRO/Theremin: `PRES.` 0,033;
- VOZ/Choir: `CORPO` 0,035; `FRASE` 0,046.

Todos são diferentes de zero e atingem código DSP; o problema é escala,
contexto ou consequência indireta, não callback vazio. Mutate, Rasgar,
Silêncio, PanicSafe, master, presença e morph produziram contrastes fortes.

## Matriz das funções visíveis

| Superfície | Estado | Observação |
|---|---|---|
| PLAY/STOP | funcional | STOP alcança silêncio duro e PLAY retoma por fade |
| MASTER | funcional | forte, global, preservado nas cenas |
| REC | implementado, sem validação E2E nesta auditoria | mix WAV 24-bit; sem stems/metadata/teste de arquivo real |
| SEED / NOVO TERRITÓRIO | quebrado em bordas | bugs de zero, PLAY imediato e estado herdado |
| TEMPO | funcional | resposta menor no CIRCUITO |
| GRID | funcional | muda agendamento futuro; mover o botão sozinho não deve soar |
| quatro cenas | funcional | morph forte; nomes são hardcoded e ignoram nome do documento |
| MORPH | funcional indireto | só afeta o próximo recall de cena |
| CAPTURAR / DESFAZER | parcial | captura apenas subconjunto e somente em memória |
| PALCO / MESA | funcional visual | não deve mudar áudio |
| HOLD/RELEASE | funcional fraco | relógio congela; concentração pouco clara em VOZ |
| MUTATE | funcional forte | em v2 atua sobretudo fora do corpo principal |
| RASGAR | funcional forte | cicatriz/estéreo/modal; em v2 não entra diretamente nos corpos |
| RECALL | funcional fraco quando limpo | reparo de cicatriz funciona melhor que gesto isolado |
| PANIC SAFE / RESET | funcional | redução/limpeza e telemetria; cena com `panicSafe=true` não tem fluxo completo de saída |
| sete macros | todos chegam ao áudio | Memória requer recalibração; XY representa só metade do estado |
| presença/arraste | funcional forte | retorno de fila ignorado pode desalinhar UI |
| nove controles locais | todos conectados | sete combinações acima têm escala fraca |
| três relações/cabos | funcionais | Tempo domina; geometria não é restaurada por cena |
| MIX/MUTE/SOLO | funcionais | política de território e UI estão inconsistentes |
| 16 células | funcionais como duplicatas | cenas, quatro gestos, seis macros, silêncio e safe; não adicionam nova lógica |
| atalhos | funcionais como duplicatas | não existe MIDI learn |
| meter OUT | pico de amostra funcional | `truePeak` é apenas cópia; não é true peak certificado |
| CPU | medição do callback no host | não vem da telemetria do motor |
| XRUNS | parcial/enganoso | conta argumentos inválidos do motor, não xruns reais do dispositivo |
| ATORES | rótulo enganoso | conta vozes da cena, não módulos audivelmente ativos |

## Contratos existentes sem função musical completa

| Contrato | Situação atual |
|---|---|
| `PerformanceFamily`, `capabilities`, `salience` | validados/serializados; não escolhem nem governam DSP |
| `ActorLifeState` | morfado como dado; Dormant/Emerging/Yielding/Ruptured/Recovering não têm comportamento próprio |
| `observedAudibility` | serializado/morfado; não recebe observação do áudio |
| `gestureBudget` | serializado/morfado; nunca limita gestos |
| `minimumPresence`/`maximumPresence` | validam cenas; controles ao vivo aceitam 0…1 e ignoram os limites |
| tipos de relação | apenas `Friction` tem semântica específica; os demais colapsam no domínio |
| domínios Pitch/Dynamics/Memory | válidos no contrato; não geram pressão no motor |
| latency/duration/limit da relação | serializados/morfados; ignorados pelo DSP |
| `ClockSource` MIDI/Link/OSC | valores válidos; motor sempre usa relógio interno |
| gesto `targetActorId` | ignorado; todos os gestos são globais |
| gesto `duration` e `condition` | validados; ignorados na aplicação |
| `Interrupt` | existe e funciona fracamente no motor; não tem botão/célula/atalho |
| nome da cena | campo serializado; host mostra quatro nomes hardcoded |
| `targetSceneId` e `session.morphSeconds` | preservados no contrato; fluxo real usa ID do comando e slider separado |
| `PerformanceModuleLibrary` | auditada por teste; runtime escolhe por descriptors paralelos |
| `truePeakMicros` | cópia do sample peak, sem oversampling |

## Funções ainda ausentes, não confundir com controles quebrados

- Modo Selvagem performativo;
- MIDI learn e controle MIDI;
- Link/OSC e relógio externo;
- save/load da sessão no host, apesar do serializer existir;
- Performance Trace;
- gravação separada de stems e retorno modal;
- seletor de dispositivo de áudio;
- múltiplas constelações/ilhas;
- navegador do catálogo completo e módulos ainda `NeedsAudit` como Walsh;
- validação assistiva/teclado completa e testes gráficos automatizados.

## Ordem recomendada de correção

1. Unificar seed/território como transação confirmada pelo áudio: corrigir seed
   0, startup 181, PLAY imediato e reset/preservação explícita do mixer.
2. Eliminar data race do documento e fazer toda rejeição de fila voltar à UI;
   coalescer XY/drag para garantir que o último valor chegue.
3. Definir o que uma cena captura e alinhar tooltip, `MOD`, posições, controles
   locais e mixer a essa decisão.
4. Ligar Memória, Hold, Recall, Mutate e Rasgar aos corpos principais de
   CIRCUITO/SOPRO/VOZ e recalibrar os sete controles locais fracos.
5. Tornar XY e geometria de relações bidirecionais ou parar de apresentá-los
   como retrato integral do estado.
6. Somente depois ativar contratos de jogo/percepção, Modo Selvagem, MIDI,
   Trace e novas ilhas.

Nenhuma correção de código foi aplicada nesta auditoria.

## Correção P0 — `v0.1.1-dev` — 2026-08-28

Os cinco P0 foram tratados no incremento seguinte:

- seed 0 deixou de ser normalizada para 1 no adapter; seleção e DSP usam zero;
- a seed inicial 181 passou a ser produzida por `PerformanceTerritoryFactory`;
- território é aplicado antes das filas de transporte, portanto PLAY imediato
  entra na seed nova;
- nova sessão reinicia MIX, MUTE, SOLO e controles locais em host e DSP,
  preservando MASTER; recalls de cena continuam preservando a mesa;
- `controlDocument_` pertence à thread de controle e `audioDocument_` à thread
  de áudio; comandos bounded carregam as cópias entre as duas;
- callbacks só publicam o novo valor visual quando a fila aceita o comando;
  rejeições restauram o último valor aceito, inclusive em XY, nós, cabos,
  controles locais, mixer, MASTER e Hold.

Foram adicionadas regressões para seed zero, território seguido de PLAY no
mesmo intervalo, reset da mesa com preservação do MASTER e rejeição de comandos
sem publicação falsa. Build completo com as três ecologias e 8/8 CTests
não-stress passaram. ASan/UBSan passaram nos caminhos alterados; LeakSanitizer
foi desativado no ASan porque não funciona sob o supervisor atual. O binário
`v0.1.1-dev` foi recompilado, mas o app não foi aberto. Geometria bidirecional,
conteúdo de captura, causalidade de SOPRO/VOZ, controles fracos e telemetria
continuam P1.

## Correção P1 — cena, mesa e geometria — `v0.1.2-dev` — 2026-08-28

O conteúdo de cena foi fechado sem ampliar o schema `RSPERF 1`: `CAPTURAR`
guarda BPM, sete macros, presença dos três atores e intensidade das três
relações. Controles locais, MIX/MUTE/SOLO, MASTER, GRID, Hold, morph e gestos
são estado efêmero da mesa/performance. Por isso, somente mudanças do primeiro
grupo acendem `MOD`; o tooltip enumera inclusões e exclusões.

Posições 2D deixaram de ser um estado paralelo sem contrato. Relação é a fonte
de verdade persistente e a geometria é sua projeção visual determinística.
Recall e novo território reconstroem os três nós em lote. Arrastar um nó ainda
calcula as relações adjacentes; editar um cabo ou slider agora reposiciona os
nós. Quando três intensidades independentes violam a desigualdade triangular,
a visualização usa a projeção de menor erro na fronteira possível, enquanto os
valores/espessuras dos cabos continuam sendo a autoridade sonora.

O helper puro `PerformanceConstellationGeometry.hpp` tem regressão de
determinismo, limites, round-trip exato para um trio compatível e finitude para
um trio impossível. Build completo, 8/8 CTests não-stress e UBSan do teste de
contratos passaram. Binário `v0.1.2-dev` recompilado; o app não foi aberto.
Permanecem P1: recalibração de controles fracos, causalidade de SOPRO/VOZ,
telemetria e avaliação visual/sonora humana da projeção.

## Correção P1 — estados globais nos corpos SOPRO/VOZ — `v0.1.3-dev` — 2026-08-28

O adapter alternativo agora recebe diretamente Memória, Intervenção, Mutate,
Rasgar e o reparo de Recall, além de Hold já existente. Eles afetam relógio,
pitch de eventos, duração de flauta/sílaba/frase, frequência e ganho de
Theremin, formante/decay/energia VOSIM, articulação Scat e corpo/recuperação do
Choir. Não há novo corpo, callback, alocação ou botão: o gesto existente passa
a subverter o corpo que o músico efetivamente escuta.

Nova regressão A/B percorre seeds 182 e 183 e exige contraste mensurável nos
três corpos para cada um dos cinco estados. Build completo e 8/8 CTests
não-stress passaram. Ainda não é validação psicoacústica: o próximo ensaio deve
confirmar se a escala é clara em palco, especialmente nos controles locais
ainda classificados como fracos.

## Correção P1 — escala dos controles locais — `v0.1.4-dev` — 2026-08-28

Os sete controles antes fracos receberam consequência explícita no próprio
corpo: CORTE amplia o intervalo de cutoff e ARTIC entra no envelope/energia do
Acid; FRAG. passa pela energia de snare/rungler e ABERT. pela energia de hat do
Drums. PRES. passa a controlar o ganho real do Theremin. CORPO e FRASE do Choir
mudam registro, desenho de frase, duração e energia, em vez de só um ganho
final discreto.

Uma nova regressão varre os três endpoints de todos os corpos CIRCUITO e exige
contraste acumulado superior a `0,50` no corpo correspondente. Os testes
alternativos existentes continuam cobrindo cada controle de SOPRO/VOZ. Build
completo e 8/8 CTests não-stress passaram. A decisão de pronto ainda depende
da audição: diferença PCM forte não basta se o gesto não for compreendido pelo
performer.

## Correção P1 — telemetria honesta — `v0.1.5-dev` — 2026-08-28

O cabeçalho deixa de chamar de `ATORES` uma contagem de corpos da ecologia: o
rótulo agora é `CORPOS`. `XRUNS` deixou de ser um contador que só crescia para
blocos inválidos e passa a registrar `ATRASOS`: callbacks cuja duração medida
no host ultrapassa seu orçamento de amostras. A medida não é apresentada como
underrun do driver, pois JUCE não o fornece por este caminho.

O medidor passa a dizer `PEAK`. O campo legado `truePeakMicros` continua sendo
preenchido com o mesmo peak por amostra para compatibilidade do contrato, mas
está documentado como proxy até existir detector com oversampling. Uma regressão
confirma que uma perda de prazo reportada pelo host chega à telemetria. Build
completo e 8/8 CTests não-stress passaram.

## Correção P1 — consequência de jogo observável — `v0.1.7-dev` — 2026-08-28

Rasgar já criava `scar_` durável e Recall já criava `repair_` audível, porém só
a cicatriz atravessava a telemetria e o painel terminava numa promessa abstrata
de trace. `repairMicros` foi acrescentado ao snapshot/mailbox e publicado pelo
motor. O host deriva um estado curto de trajetória e o apresenta na Constelação,
no cabeçalho durante cicatriz/reparo e no rodapé.

Regressões confirmam preservação dos dois campos pela mailbox, reparo acima de
0,75 imediatamente após Recall e limpeza de ambos por PanicSafe. Build completo
e 9/9 CTests, incluindo estabilidade, passaram. Isso não conclui o contrato de
Trace nem ativa os contratos ainda sem consumidor; apenas torna legível uma
consequência já real no áudio. O app não foi aberto e a correspondência
perceptiva gesto → som → leitura permanece pendente do músico.

## Correção P1 — explicação de relação no campo — `v0.1.8-dev` — 2026-08-28

O cabo selecionado mostrava somente índice e intensidade, embora o motor já
aplicasse pressão de matéria, tempo ou espaço condicionada à presença de ambos
os corpos. A Constelação passou a mostrar a consequência concreta e a condição
ativa/potencial. Nó selecionado expõe o pico do stem real, não uma estimativa
de audibilidade. Não há mudança no DSP, no protocolo de controle ou no número
de controles.

Build do host passou, o executável foi marcado `v0.1.8-dev` e a suíte integral
anterior 9/9 permanece válida para o core inalterado. A confirmação de que os
textos melhoram a compreensão e de que os cabos têm escala musical perceptível
continua humana; o app não foi aberto pelo workflow.

## Correção P1 — vida declarada chega ao som — `v0.1.9-dev` — 2026-08-28

`life state` era válido, serializado e morfado, mas não tinha consumidor no
motor. Ele agora determina o ganho do stem e é publicado com cada stem. Uma
regressão compara Active a Dormant e confirma que presença desperta um corpo
dormente sem construir DSP. A regressão de coortes foi corrigida para afirmar
o princípio real: toda seed inicia com ao menos dois stems audíveis e pode
deixar um terceiro em reserva para o performer.

Build completo e 9/9 CTests, incluindo estabilidade, passaram. A escala dos
estados e o gesto de despertar ainda requerem escuta humana; o app não foi
aberto pelo workflow.

## Correção P1 — `MUTATE` abre um percurso, não só um botão — `v0.1.10-dev` — 2026-08-28

O painel já anunciava `MUTATE`, mas a leitura do gesto ainda não explicitava
uma consequência jogável nem seu retorno. O evento agora transporta um token
de variação criado no controle pela fila SPSC já existente. O motor o usa para
uma abertura limitada de mutação, fase e matéria dos stems; publica `wildMicros`
na mailbox e a interface só exibe `SELVAGEM` durante esse efeito. `RECALL`
reduz a abertura e `PANIC SAFE` a elimina. O teste do motor cobre entrada,
retorno e a divergência audível entre dois tokens sob a mesma seed; a
estabilidade longa cobre o caminho sem abrir o host.

## Correção P0 — cena não interrompe transporte — `v0.1.11-dev` — 2026-08-28

Após um novo território, as cenas preparadas carregam STOP como estado inicial.
O recall reaplicava esse campo durante PLAY e silenciosamente encerrava a
performance. O motor passa a preservar transporte no recall e no morph; a
instalação de território mantém sua semântica própria. A regressão cria uma
cena capturada como STOP, entra explicitamente em PLAY e confirma que o recall
conclui sem desligar a saída.

## Correção P1 — `MUTATE` pode atingir o corpo escolhido — `v0.1.12-dev` — 2026-08-29

O painel já permitia selecionar módulos, porém `targetActorId` permanecia sem
consumidor e Mutate alterava a ecologia inteira. A seleção agora acompanha o
evento de gesto; a ecologia concentra mutação, desvio e matéria no stem visado,
mantendo uma contribuição menor nos vizinhos. A regressão compara os três
stems e exige diferença maior no ator selecionado. Sem nó em foco o gesto não
perde sua função coletiva.

## Correção P1 — `RASGAR` também é um gesto de módulo — `v0.1.13-dev` — 2026-08-29

O alvo de Mutate já era audível, mas Rasgar ainda depositava sua ruptura por
igual na ecologia. O evento agora porta o mesmo ator escolhido para o caminho
de cicatriz; o corpo selecionado recebe a maior diferença e os vizinhos mantêm
uma parcela de continuidade. A regressão verifica essa prioridade para Mutate
e Rasgar sem desativar a leitura coletiva quando não há nó em foco.

## Correção P1 — `RECALL` repara o corpo escolhido — `v0.1.14-dev` — 2026-08-29

Recall já reduzia abertura, ruptura e cicatriz globais, mas o reparo não tinha
destino performável. Com um nó em foco, o evento bounded carrega agora um alvo
de reparo separado dos alvos de Mutate e Rasgar. O stem escolhido recebe a
maior recomposição; os vizinhos conservam parcela menor e, sem seleção, a ação
permanece coletiva. A regressão de ecologia compara os três stems e confirma a
prioridade do módulo selecionado.

## Correção P1 — foco visível é foco sonoro — `v0.1.15-dev` — 2026-08-29

A Constelação desenhava o primeiro nó selecionado, porém o alvo do motor só era
atualizado depois de um clique. Isso tornava a primeira ação ambígua e permitia
um foco residual não declarado. O host passa a inicializar ambos juntos, expõe
o alvo nos botões Abrir/Rasgar/Reparar e limpa-o quando o performer clica no
vazio. A mudança é de superfície e não introduz caminho novo no callback.
