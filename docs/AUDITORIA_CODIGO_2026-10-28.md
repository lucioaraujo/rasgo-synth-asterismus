# Auditoria de código — 2026-10-28

Pedido do autor: verificar "o que já foi codado e que está inapto, sem
precisão, que destoa, que ficou atrasado para esse modelo que estamos
visualizando", "além claro dos bugs, e de códigos incompletos, ou
inconsistentes". O modelo de referência é o de 28/10: cada módulo uma
estrela, um stem por estrela, a seed escolhe estrelas, a trama do céu
(`docs/TRAMA_DE_RELACOES.md`).

Legenda: **corrigido** (nesta rodada, com teste), **a corrigir** (próxima
rodada), **decidir** (depende do autor).

## 1. Bugs

| # | Onde | Achado | Estado |
|---|---|---|---|
| 1.1 | `PerformanceSkyDirector.hpp` (fatia enviada em `eb537ec`) | dois contágios recíprocos de atraso zero chamavam-se sem fim: no app, retrigger a cada amostra; num teste em que o chamado vira ataque na hora, laço infinito | **corrigido**: profundidade de cadeia ≤ 2 e intervalo mínimo de ⅛ de tempo entre chamados à mesma estrela; teste novo "reciprocal relations stay bounded" |
| 1.2 | `PerformanceActorEcology.hpp`, ramo de fallback | o corpo 5 (Modal) calculava uma senoide de imitação; com o nível do Modal em zero, essa imitação soava com o nome "Modal" | **corrigido**: o corpo do Modal só soa pelo adapter do Modal |
| 1.3 | ecologia, caminho do Modal | sem o corpo 5 no céu, o Modal era dobrado dentro do stem de outro corpo (MUTE desse corpo não calava só ele) | **corrigido**: sem corpo 5, o Modal não soa |
| 1.4 | ecologia com 8 corpos | ~1 atraso por segundo parado: a ecologia rodava nos corpos apagados | **corrigido** em `411cc4a` |
| 1.5 | abertura | Galo, Atropelo e Deja Vu não subiam: eram reservas fora do som | **corrigido** em `04efee6` |

## 2. Código atrasado para o modelo (morto ou enganoso)

| # | Onde | Achado | Estado |
|---|---|---|---|
| 2.1 | `PerformanceModuleCohortAdapter.hpp` (1.124 linhas, 36 `#if`) | não toca mais nada, mas a ecologia ainda o preparava (todo o DSP das cinco famílias), semeava e configurava | instância **removida** da ecologia; o arquivo ainda fornece tipos (`PerformanceModuleBody`, `PerformanceModuleControl`, perfil de composição) — **a corrigir**: mover os tipos para um cabeçalho próprio e apagar a classe e seu teste |
| 2.2 | `PerformanceStemBusMixer.hpp`, `PerformanceStemFrame::busMixer/hasBusMix` | o mixer de 4 barramentos não é mais usado no som | **a corrigir**: retirar com o teste dele |
| 2.3 | `PerformanceStemReservations.hpp`, `isSeedPreparedLibraryModule` | reservas por coorte do "quarto slot"; o controlador já usa o asterismo | **a corrigir**: retirar com os trechos de teste |
| 2.4 | ecologia, `internalModuleControls_/Positions_/SeedVariation_` | controles, posições e variação dos corpos 5–8 quando não eram vozes; guardados e **nunca mais lidos** (os setters respondem "aceito" sem efeito) | **a corrigir**: com 8 corpos no céu, os corpos 5–8 são vozes; retirar o caminho paralelo |
| 2.5 | ecologia, ramo de fallback em senoide | fórmulas "MODAL/GALO/ATROPELO/DEJA VU" de imitação; hoje só alcançável por corpo sem estrela | **a corrigir**: retirar (todo corpo do céu tem estrela; o Modal tem adapter) |
| 2.6 | ecologia, `modalLayerOwnerIndex` e configuração do Modal pelo "dono" | resto da dobra do Modal em outro stem | **a corrigir** junto com o passo "Modal como estrela" |
| 2.7 | `PerformanceCohortControls.hpp` | nomes de controle por coorte; substituído por `performanceStarControlNames` | **a corrigir**: retirar quando nada mais o usar |
| 2.8 | testes `module_cohort`, `stem_bus_mixer`, `internal_stem_layers` | testam o modelo antigo | **a corrigir** junto com 2.1–2.4 |

## 3. Interface (textos e controles que destoam)

| # | Onde | Achado | Estado |
|---|---|---|---|
| 3.1 | tutorial, seções 1 e 6, nas quatro línguas | ensinava "quatro stems audíveis; os módulos 5–8 são camadas internas" e "coorte" | **corrigido**: estrela com stem próprio, apagada sem custo, trama do céu |
| 3.2 | rodapé da biblioteca no Campo | "quatro stems / oito lanes" | **corrigido**: "um stem por estrela" |
| 3.3 | rótulo da seed | "SEED / COORTE" | **corrigido**: "SEED / ASTERISMO" |
| 3.4 | `walshRelationSlider_` | controle exclusivo de uma relação ("ÓRBITA ACID → …"), escondido, com o nome do Acid fixo mesmo quando o corpo 1 não é Acid | **a corrigir**: retirar |
| 3.5 | controles escondidos com `setVisible(false)`: 16 células, macros, sliders de relação, editor de tempo | código suspenso desde setembro, sem destino | **decidir**: retirar ou redesenhar dentro da trama (lista D da triagem) |
| 3.6 | valores iniciais de `moduleNames_`, `actorLabels`, `relationDisplayNames_` | nomes fixos "ACID, GENDYN, DRUMS, WALSH…" usados até a primeira atualização | **a corrigir**: iniciar já pelo asterismo da seed |
| 3.7 | a trama do céu não tem controle visível | influência fixa em 1 | **a corrigir**: controle de influência e leitura das relações em foco no Campo |

## 4. Incompleto

| # | Achado | Estado |
|---|---|---|
| 4.1 | a gravação grava só a mistura; não grava **um arquivo por stem** (L860 da lista), justamente o que o modelo de um stem por estrela pede para mixar e combinar | **a corrigir** (prioridade alta) |
| 4.2 | o motor continua processando em STOP (~20% de CPU de bloco) | **a corrigir** |
| 4.3 | a interface gasta ~40% de um núcleo mesmo parada | **a corrigir** |
| 4.4 | guarda de capacidade (limite de estrelas acesas medido por máquina) | **a corrigir** (passo 4 da trama) |
| 4.5 | Modal fora do céu; Tuba/Breath com adapter sem rota | **a corrigir** (passo 6) |
| 4.6 | `PerformanceApp.cpp` com ~5.000 linhas num arquivo | **a corrigir**: dividir por responsabilidade (interface, gravação, idiomas, tutorial) |

## 5. Validação desta rodada

- Suíte completa: 33/33 (o teste `internal_stem_layers`, que afirmava a dobra
  do Modal e do quarto corpo dentro de outro stem, foi reescrito para a regra
  nova: nenhum módulo dobrado; o quarto corpo soa no próprio stem).
- O bug 1.1 foi reproduzido na versão enviada (o teste de reciprocidade
  ficou preso no laço por mais de 10 minutos e foi interrompido) e não
  ocorre na versão corrigida (no máximo 80 chamados em 20 s no pior caso).
- App recompilado; aberto com o binário corrigido.

## 6. Rodada de 29/10 (madrugada)

- **Gravação por stem (4.1) — feito:** um WAV mono 24-bit por corpo junto
  com a tomada (`PerformanceStemRecorder`, escrita em thread própria), antes
  da espacialização global; o manifesto lista os arquivos (`stems`).
- **Variação do mixer (pedido do autor) — feito:** botão VARIAÇÃO no Campo de
  Matérias (ligada ao abrir o app). A seed escolhe quais estrelas abrem acesas
  (corpos 1–3 sempre; de 1 a 4 entre os corpos 4–8); a dança do brilho apaga e
  reacende estrelas (vida por massa–luminosidade, 3 a 6 acesas); a mistura
  segue o movimento da Constelação (30–100% do fader pela proximidade ao
  centro de massa da figura); subir um fader acende e segura a estrela. O
  brilho multiplica o fader, nunca o substitui. Testado: 3..6 acesas,
  8 reacendimentos em 4 min, nível 0,34..1,0; desligada, brilho inteiro.
- **Corpos 6–8 deixaram de abrir sempre zerados** (pedido do autor): todos
  abrem com a presença da seed; a variação decide quem acende.
- **Atrasos no app (1.109 numa sessão do autor com a trama):** causa ainda
  não encontrada. Descartado: custo dos chamados (< 15 µs). Uma medição com
  prioridade de tempo real foi um erro (travou a máquina por ocupar um núcleo
  e mediu o corte de tempo real do Linux, não o app) — não repetir.
  Instrumentado no app: rodapé "PIOR BLOCO" e CSV por segundo e por atraso em
  `~/Music/Rasgo Synth Performance/diagnostico/` (tempo do motor separado do
  tempo das gravações, stems soando, virada da seed, gravando).
- Suíte: 33/33 (rodada em `nice 19`, um teste por vez).

## 7. Causa dos atrasos com a trama (29/10)

Sessão do autor com o diagnóstico no app (seed 772637541, variação ligada,
gravando): **202 atrasos em 341 s tocando**; todos com o tempo no motor (a
gravação não pesa); blocos de 10,7–13,2 ms com média ~3 ms; o pior bloco
sobe aos poucos ao longo de segundos, independe de quantos stems soam.

Fora do app, a mesma seed e configuração, rodando sem pausa: pior bloco
4,5 ms, nenhum atraso. **Imitando o ritmo do app** (processa um bloco, dorme
até o próximo): 99% em 9,3 ms, pior 17,6 ms, 17 blocos acima do prazo em
1 min. Causa: entre os blocos o processador baixa a frequência
(`intel_pstate`, preferência `balance_performance`, mínimo 0,8 GHz) e os
caches esfriam; um bloco mais pesado nesse instante custa 3–4×. A trama
aumentou o número de blocos pesados (mais gestos disparados).

Correções: (1) teste reversível do autor — preferência de energia
`performance`; (2) **baixar o custo dos blocos no motor** (escolhido pelo
autor); (3) opção de buffer de 1.024 amostras.

## 8. Custo do motor reduzido (29/10, madrugada) — sem mudar o som

Perfil por amostragem (sinal de temporizador + `addr2line`, sem `perf`):
~45% do tempo em senos, `tanh` e potências da libm. Mudanças, todas
conferidas por hash FNV de 3 seeds × variação ligada/desligada, 20 s cada —
**hash idêntico ao anterior em todas**:

1. A matéria provisória em senoides (harmônico, cicatriz, movimento, resíduo,
   textura de fratura com duas potências, portão relacional) só é calculada
   para corpos sem estrela (o fallback); antes rodava para toda estrela a
   cada amostra e era jogada fora.
2. O contorno de altura da seed só é calculado quando o alvo é renovado.
3. Os índices dos corpos de cada relação ficam guardados na cena (antes,
   procura linear por amostra, duas vezes por relação).
4. O seno do movimento é calculado uma vez por amostra (o sentido inverso
   só troca o sinal), não por corpo.
5. RASGAR e REPARAR com peso zero pulam as suas contas (seno e `tanh`).

| medição (8 acesos, trama ligada) | antes | depois |
|---|---|---|
| contínuo, média por bloco | 1,90 ms | 1,18 ms (−38%) |
| contínuo, pior bloco | 4,15 ms | 3,20 ms |
| **no ritmo do app**, blocos acima do prazo em 2 min | **76** | **8** |
| no ritmo do app, 99% | 9,86 ms | 6,42 ms |

O que sobra no topo do perfil é a `tanh` que dá forma a cada estrela e o
DSP dos próprios módulos (Scat, filtros, Modal, Galo): mexer nisso muda o
timbre e fica para a escuta do autor. Suíte 33/33.

## 9. Limpeza e proteções (29/10, madrugada)

- **2.1–2.3, 2.7, 2.8 corrigidos:** retirados `PerformanceModuleCohortAdapter`
  (1.124 linhas, 36 `#if`), `PerformanceCohortControls`,
  `PerformanceStemReservations`, `PerformanceStemBusMixer` e os três testes
  deles; os tipos que valem foram para `PerformanceModuleTypes.hpp`; o céu
  inclui o seu DSP diretamente; o teste "seeds diferentes chegam ao som" foi
  reescrito sobre o céu. Som tocando idêntico (mesmo hash).
- **3.3/3.4 corrigido:** o slider escondido exclusivo do Walsh saiu (o valor
  da órbita 1→4, ainda usado por um gesto, ficou).
- **Título do Campo de Matérias:** o texto termina em "…" antes do botão
  VARIAÇÃO, que não o cobre mais.
- **4.2 corrigido:** em STOP silencioso o céu não calcula estrelas e retoma no
  PLAY.
- **4.4 corrigido — guarda de capacidade:** o app mede o pior bloco; dois
  segundos acima de 75% do prazo tiram uma estrela da capacidade (mínimo 3);
  30 s tocando abaixo de 45% devolvem uma. A variação do mixer respeita a
  capacidade (apaga primeiro quem tem menos vida); os faders não são
  bloqueados. Rodapé mostra CAPACIDADE; o CSV ganhou a coluna. Teste: com
  capacidade 3, nunca mais de 3 acesas.
- **Manifesto da tomada:** agora com o asterismo (a estrela de cada corpo),
  a variação e a capacidade.
- Pendentes desta auditoria: 2.4–2.6 (caminho paralelo dos corpos 5–8,
  fallback em senoide, resto do Modal — junto do "Modal como estrela"), 3.5
  (controles escondidos: decisão do autor), 3.6, 3.7, 4.3 (CPU da
  interface), 4.5, 4.6.
- Suíte: 30/30 (3 testes do código retirado saíram com ele).

## 10. Registro de um deslize (29/10)

O commit `587d009` (economia em STOP e redesenho) foi feito no mesmo comando
que leu a suíte, e ela tinha **uma falha**: parado, a telemetria deixava de
informar os corpos (o teste do quinto corpo Modal acusou). Corrigido no
commit seguinte — parado, o motor não calcula som mas continua publicando a
identidade e a vida de cada corpo. Prática corrigida: ler o resultado da
suíte antes de qualquer commit.

## 11. Travamento sentido pelo autor (29/10, tarde)

Sintoma: "acho que tava meio travado". O CSV de diagnóstico da sessão
09:00–18:02 mostra áudio normal até as 14:30; dali em diante, cerca de metade
dos segundos **não teve nenhum bloco de áudio** (pior bloco 0,00, valores de
CPU repetidos) e um take de 14 s gerou só 101 KB. O custo do motor ficou em
~1 ms por bloco: quem parou foi o fluxo do sistema (ponte ALSA/PipeWire),
não o motor. O journal do sistema não registrou nada.

Proteção: o app guarda a hora do último bloco (`lastCallbackMs_`) e o timer
da interface (`watchAudioFlow`) reabre o dispositivo quando ele está aberto e
pronto mas passa mais de 400 ms sem bloco (no máximo uma vez a cada 10 s).
Cada reabertura vira uma linha `reinicio_audio` no CSV e o rodapé mostra
"ÁUDIO REABERTO n×". Conferido numa instância muda (40 s, saída
`rasgo_mudo`): nenhuma reabertura, nenhum segundo sem bloco. A eficácia contra
o travamento real só se confirma quando ele voltar a ocorrer — o CSV dirá.

## 12. Blocos lentos de 18:32 (29/10) — a máquina, não a seed

Na peça de 18:27 (seed 4233069840, 7 min 05 s, terminou sozinha na coda
"gather"), oito blocos passaram do prazo entre 18:32 e 18:33 (motor medido
dentro do app: 10,8–11,8 ms) e a guarda baixou a capacidade de 8 para 3.
Reprodução fora do app, mesma seed, trama e variação ligadas, 430 s, duas
vezes: média 1,15 ms por bloco; os picos **não se repetem** no mesmo ponto
(20,6 ms aos 111 s na primeira, 6,5 ms aos 376 s na segunda) — o motor é
determinístico, logo os picos vêm da máquina. No momento da medição o
Firefox decodificava vídeo (~60% de CPU somando o processo de mídia), Xorg
e Cinnamon somavam ~58%, num i5-6500 de 4 núcleos em `powersave`. Dentro do
app o pior bloco por segundo fica em 4–7 ms contra 2–3 ms fora dele: a
diferença é disputa de CPU/cache com a interface e o resto do sistema.
Recomendação ao autor (decisão dele, não aplicada): fechar vídeo no
navegador enquanto toca; opcionalmente o governador `performance` durante a
sessão. A guarda de capacidade fez o seu papel (nenhum bloco perdido na
gravação: `droppedBlocks 0`).
