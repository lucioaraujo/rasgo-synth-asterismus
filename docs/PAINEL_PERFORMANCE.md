# Painel do Rasgo Synth Performance

**Estado:** primeira superfície de ação e cabeçalho de identidade implementados
no host próprio — 2026-08-25

## Investigação de layout — 2026-08-26

O Atlas foi revisitado para sair da pergunta vaga “painel moderno” e formular
alternativas de palco com função musical. A superfície atual é a **Mesa de
palco**: cenas/gestos à esquerda, campo de ação, constelação e Mesa de Matérias
à direita. Ela continua sendo a base estável de operação.

Dois desdobramentos são relevantes:

- **Constelação central:** o desenho de atores e cabos se torna o principal
  campo de leitura, expondo atividade de stems, pulso, GRID, Hold e Cicatriz.
  Sliders, botões e teclado continuam sendo as rotas precisas/acessíveis;
- **Partitura de trajetória:** células, cenas e eventos quantizados passam a
  formar uma linha temporal curta, útil para revisão e segundo monitor, sem
  transformar o instrumento em sequenciador opaco.

O primeiro protótipo será a Constelação central como modo reversível; não será
imposta como único layout antes de teste humano. A decisão preserva o Rasgo:
cabos representam relações DSP reais, não decoração; estado técnico não finge
ser controle; segurança, cena e master permanecem em posição estável.

Critérios de aceite: alvo mínimo de 24×24 unidades ou espaçamento equivalente,
foco explícito, alternativa a todo arraste e consequência sonora/estado
explicável para cada elemento. Fontes e licenças: W3C WCAG 2.2 como critério;
CoMo BSD-3-Clause para pesquisa de gesto; libmapper LGPL-2.1 como candidato
futuro de mapeamento; iPlug2 zlib-like como referência vetorial. Nenhum código,
asset ou layout de terceiros será importado neste incremento.

### Protótipo `FOCO` — Constelação central

O host agora oferece dois arranjos para comparação humana:

- **Mesa de palco** (padrão): campo/células à esquerda; controles de precisão
  e relações à direita;
- **Constelação central**: campo/células em coluna esquerda, macros/stems na
  direita e o grafo de relações no campo central ampliado.

`FOCO` na faixa de cenas, ou a tecla `F`, alterna os dois sem tocar no motor,
no documento de sessão ou no transporte. Cenas, master, GRID e segurança ficam
no cabeçalho em ambos os modos. É uma comparação reversível, não a substituição
da Mesa de palco. A alternativa por sliders/teclado permanece presente para os
nós e cabos do grafo.

Validação: 7/7 CTests do núcleo, incluindo stress offline de 22,99 s; 3/3
testes relevantes no build do app. Binário recompilado: 2026-08-26 19:58:32
`+02:00`; SHA-256
`12aa45a98c0988c41670bbf6296d405c45f0bdb8b0ddd37d37961cd30b7c6fa9`.

Próximo passo: em 1920×1080, localizar cena, GRID, master, safe e a relação
Órbita em ambos os modos; tocar uma sequência curta e escolher qual arranjo
favorece a leitura sem reduzir a rapidez de intervenção.

### Constelação de módulos reais — 2026-08-27

O protótipo espacial foi promovido de visualização de papéis para controle dos
três primeiros corpos reais: `ACID · CHÃO`, `GENDYN · ATRITO` e
`DRUMS · CONTÁGIO`. Arrastar um círculo em duas dimensões não altera mais sua
presença. A distância até os outros módulos recalcula apenas os cabos de
Matéria, Tempo e Espaço; o tamanho mostra presença e o halo mostra energia que
efetivamente sobreviveu ao mixer.

Esse desacoplamento estabelece a gramática do painel: posição = relação;
tamanho = presença; halo = consequência observada; MIX/MUTE/SOLO = decisão de
saída. As setas movem o nó selecionado, enquanto `C` e `[`/`]` preservam o
ajuste preciso do cabo. `Alt+↑/↓` e os faders da MESA ajustam presença sem
alterar posição. Não foram adicionados botões nem sobreposições.

Build do app e 7/7 testes funcionais passaram; o stress de 20 minutos passou
em 129,79 s. Binário recompilado em 2026-08-27 01:35:12 `+02:00`, SHA-256
`e6f7ae810959f9c7fe78293b6320331305560e9ab7d8b72f9fbfa98bd3fa9828`.
A interface não foi aberta automaticamente; layout e causalidade aguardam a
escuta do performer.

## Princípio

O painel de palco nasce do layout já resolvido do Rasgo Synth, sem recomeçar do
zero. Ele preserva sua identidade visual e sua memória espacial, mas troca as
funções de composição/renderização do Studio por decisões legíveis sob pressão,
com alvos grandes, poucas camadas, feedback imediato e nenhuma grade de 142
mecanismos.

O músico não microedita cada oscilador. Ele conduz território, relações,
memória, risco e presença dos organismos.

## Matriz visual: o layout do Rasgo Synth

A referência obrigatória é o app Studio já implementado e documentado em
`../../docs/assets/app_screenshot.png`, junto de
`../../rasgo-synth-app/src/RasgoLookAndFeel.h`. O Performance deve
parecer imediatamente um segundo instrumento Rasgo Synth, não um produto novo
sem parentesco.

Elementos preservados:

- placa eletrônica verde como substrato e grade de circuito;
- cobre/coral para trilhas, estados vivos e ações principais;
- logo Rasgo autoral e título alto/condensado;
- IBM Plex Mono ou equivalente já licenciado para leituras e parâmetros;
- painéis elevados/rebaixados, bordas discretas e visual de bancada eletrônica;
- meters, scope compacto, log e trajetória como evidência do acontecimento;
- arco/genealogia e módulos ativos como leitura da vida interna do instrumento;
- densidade informativa assumida, organizada em zonas estáveis e legíveis.

O Performance não copia o arquivo de interface nem modifica
`MainComponent.h`. Ele implementa uma nova árvore de componentes no projeto
irmão, guiada pelos mesmos tokens e proporções. O compartilhamento futuro de
logo, fontes ou tokens exige contrato explícito de assets, origem/licença e
empacotamento próprio; não haverá caminho runtime para dentro do Studio.

## Primeiro incremento implementado

`src/app/PerformanceApp.cpp` agora constrói uma superfície própria de host com:

- cabeçalho Rasgo verde/cobre com a logo vetorial e a composição
  `RASGO SYNTH · PERFORMANCE`: Big Shoulders para marca e `Monospace` do
  sistema para o chrome, como no Studio; IBM Plex permanece somente como
  recurso licenciado distribuível; todos os recursos são cópias locais do Performance, com hashes,
  licenças e cópia pós-build registrados em [`../assets/README.md`](../assets/README.md);
- status de áudio, cena, atores, xruns e CPU medida no callback do host;
- rodapé persistente dividido como no Studio: à esquerda, dispositivo de áudio,
  taxa, buffer e saídas efetivamente abertos; à direita, versão, autoria,
  GPLv3 e uso AGPLv3 do JUCE;
- linha de última ação no painel de relações, registrando cena, gesto,
  transporte, MASTER e ResetSafe sem escrever no callback;
- quatro cenas próprias (`ABERTURA`, `ESPIRAL`, `RASGO`, `BALANÇO`) recuperadas
  com morph de 0,8 s pela fila SPSC do controlador e realçadas em cobre quando
  a cena-alvo se torna ativa; seus macros, presenças, relação e campo XY
  sincronizam na superfície ao recall;
- indicador `MOD` no estado de áudio quando macros, presenças ou relação
  alteram a cena em curso; um recall limpa esse indicador;
- transporte `PLAY`/`STOP` na faixa superior, aplicado por fila própria ao
  motor com transição de ganho, nunca por corte de callback;
- `MASTER` em dB na faixa superior (`−24` a `0 dB`), enfileirado até o motor e
  aplicado suavemente ao ganho-alvo, antes da limitação de amostra; é global
  para a apresentação e não retorna ao valor de uma cena durante morph;
- meter `OUT` de pico, alimentado pela telemetria atômica do motor e marcado em
  estado PanicSafe; o pico considera ambos os canais de saída, mas não é ainda
  true peak certificado; a leitura e a barra usam escala −60…0 dB para manter
  informação útil mesmo em volumes baixos; picos a partir de −0,18 dBFS
  mantêm indicação `CLIP` visível por cerca de três segundos;
- ações diretas `HOLD`, `MUTATE`, `RASGAR`, `RECALL` e `PANIC SAFE`; `HOLD`
  alterna explicitamente para `RELEASE` no mesmo botão/célula/atalho; quando a
  segurança está latched o botão vira `RESET SAFE`, que a libera por fila
  bounded e preserva a transição de ganho do motor; PanicSafe também libera
  Hold e zera as energias transitórias de Mutate/Rasgar;
- campo XY que enfileira autonomia↔intervenção e memória↔risco pela fila SPSC
  de macros, sem escrita concorrente no motor;
- sete controles contínuos para autonomia, intervenção, densidade, tensão,
  memória, risco e espaço; os quatro compartilhados pelo XY permanecem
  sincronizados visualmente com ele;
- duas presenças de ator (`CHÃO · 1`, `ATRITO · 2`) e a intensidade da relação
  `1 → 2`, ligadas por comandos SPSC bounded à ecologia nativa;
- grade 4×4 de células para cenas, gestos, macros de nutrição, silêncio e
  segurança, sempre encaminhada pelo controlador;
- operação de teclado para o teste de palco: `1`–`4` recuperam cenas,
  `Q/W/E/R` disparam Hold/Mutate/Rasgar/Recall, `X` silencia, `P` aciona
  PanicSafe/ResetSafe e `Espaço` alterna PLAY/STOP; o painel obtém foco ao clicar fora
  de um controle;
- zonas permanentes de cenas, gestos, constelação e relações, sem waveform ou
  análise visual dominando o centro;
- timer somente de interface para leitura de telemetria atômica; nenhum widget
  atua diretamente no callback de áudio.

O incremento é uma estrutura de palco, não o painel completo: edição/captura
de cenas, controles de presença, relações reais, monitor selecionável, MIDI e
gravação ainda faltam. A compilação do host e 6/6 CTests passaram; XY, células
e controles contínuos aguardam avaliação visual e de áudio em sessão gráfica.
Os controles de ator/relação pertencem apenas à constelação de partida atual;
um editor geral de atores, papéis e relações ainda é um incremento separado.
O host abre maximizado e os botões de cena/morph receberam área vertical maior;
isso ainda requer revisão humana em tela real.
O meter `OUT` e PLAY/STOP usam faixas independentes no cabeçalho, sem
sobreposição horizontal.

### Recortes de colagem na faixa de gestos — 2026-09-02

A faixa de gestos recebeu `STUTTER` e `TEMPO SHIFT`, com atalhos `S` e `D`.
Eles pertencem ao mesmo campo de ação de `HOLD`, `MUTATE`, `RASGAR` e `RECALL`,
não à grade de 16 células: a seleção de um nó define o stem alvo; um clique no
vazio devolve a consequência ao conjunto. O tooltip declara esse alcance e os
limites: recorte curto, BPM e desenho preservados.

O painel apenas enfileira o gesto. Ele não cria módulo, não redesenha a
constelação e não altera BPM, cena ou capacidade. A leitura humana deve agora
verificar se sete botões continuam legíveis na resolução real e se `S`/`D`
produzem um contraste musical reconhecível sem parecer dois comandos arbitrários.

### Faixa `COLAGEM` da MESA — 2026-09-02

Acima do campo XY, a MESA exibe `COLAGEM`, o número da seed secundária, `RODAR`
e `MIX`. O botão não é um segundo `ALEA`: ele sorteia somente a trajetória que
contamina a seed do território. O editor aceita uma fonte reproduzível; o fader
vai de `0` (principal intacta) a `1` (traços secundários). A faixa tem limites
exclusivos antes do campo, para não cobrir seus controles nem a constelação em
layouts compactos.

Trocar o território continua sendo o gesto intencional do cabeçalho. A colagem
é reaplicada depois dessa transação quando o músico a mantém ativa. A interface
não afirma validação visual: falta verificar em tela real se os controles
permanecem legíveis e se a distinção entre `ALEA` e `RODAR` é imediata.

### Incremento de operação de palco — 2026-08-25

Dez ajustes pequenos foram reunidos num único incremento verificável, sem
mudar o contrato de áudio: (1) limite mínimo de janela de 1180×760 para não
colapsar os controles; (2) estado `SAFE` no status; (3) aviso `ATENÇÃO` a
partir de 60% de CPU; (4) aviso `SOBRECARGA` a partir de 85%; (5) prioridade
visual de SAFE no meter; (6) persistência já existente de `CLIP` mantida;
(7) botão Hold e célula correspondente passam juntos para `RELEASE`; (8)
PanicSafe libera esse estado visual quando o motor confirma a segurança; (9)
a célula `SEGURO` passa para `RESET SAFE` enquanto latched; e (10) tooltips
apresentam atalhos e função de transporte, cenas, gestos, XY, macros, atores,
relação e células após 650 ms de repouso.

O trabalho é exclusivamente de interface/timer e filas já existentes: não há
I/O, alocação, lock ou descoberta adicionados ao callback. Build do host e
6/6 CTests passaram em 2026-08-25; ainda falta validação humana em tela e
dispositivo de áudio reais, em especial dos tooltips e dos limiares de CPU.

### Incremento de controle de cenas — 2026-08-25

Mais dez passos de operação foram concluídos: (1) o morph deixou de ser apenas
o valor fixo de 0,8 s; (2) ganhou slider próprio na faixa de cenas; (3) seu
range é limitado a 0,2–3,0 s; (4) o recall usa o valor efetivamente escolhido;
(5) `[` reduz a duração em 0,1 s; (6) `]` a aumenta em 0,1 s; (7) `−` reduz
MASTER em 0,5 dB; (8) `=` o aumenta em 0,5 dB; (9) o status distingue uma cena
em `MORPH` da cena já consolidada pela telemetria; e (10) expõe a profundidade
pendente de eventos como `GESTOS n`, quando houver eventos aguardando o motor.

O histórico de ação agora registra a duração usada no recall. Os novos atalhos
e a duração do morph ainda exigem ensaio humano em áudio real; a suíte C++ não
substitui essa validação.

### Incremento de captura de cenas — 2026-08-25

Dez passos completam a primeira memória editável de palco: (1) há uma fila
SPSC bounded exclusiva para captura; (2) ela contém um snapshot completo de
`PerformanceScene`; (3) o produtor valida a cena e seu id; (4) o consumidor
atualiza apenas o documento de recalls futuros; (5) a cena que está soando não
é alterada pela captura; (6) o botão `CAPTURAR` entra na faixa de cenas; (7)
ele captura a cena-alvo; (8) `Shift+1` até `Shift+4` capturam diretamente cada
cena; (9) macros, presenças e relação visíveis compõem o snapshot; e (10) o
estado `MOD` é limpo somente quando a captura foi aceita na fila.

O histórico declara `captura enfileirada`, sem prometer aplicação síncrona. O
teste do controlador prova a instalação posterior no documento, e build + 6/6
CTests passaram. Ainda falta validar visualmente o botão e realizar um recall
após captura com áudio real.

### Incremento de proteção da captura — 2026-08-25

Dez medidas tornam a captura reversível em palco: (1) a interface preserva um
snapshot da cena imediatamente anterior; (2) somente a última captura aceita
ganha reversão; (3) `DESFAZER` começa desabilitado; (4) é habilitado quando a
captura entra na fila; (5) `Shift+Z` oferece o mesmo gesto; (6) a restauração
usa a fila bounded de captura; (7) o documento local é restaurado junto com o
comando; (8) o botão volta a desabilitar após uma restauração aceita; (9) falha
de enfileiramento restaura o snapshot local e preserva `MOD`; e (10) o teste do
controlador comprova que uma segunda captura pode restaurar a cena anterior.

É um único nível de desfazer deliberadamente: não há ainda histórico ilimitado,
serialização de undo ou escrita em disco. Build e 6/6 CTests passaram; falta
ensaio humano do botão no layout maximizado.

### Eixo de forças preciso — 2026-08-25

O campo também pode ser operado sem mouse: (1) recebe foco ao clicar; (2)
desenha um aro de foco; (3) seta esquerda reduz autonomia; (4) direita aumenta
intervenção; (5) cima aumenta memória; (6) baixo aumenta ruptura; (7) o passo
normal é 0,02; (8) `Shift` usa passo de 0,10; (9) `Home` centraliza o campo;
e (10) cada mudança percorre o callback de macros já usado pelo mouse.

### Terceiro ator na sessão de partida — 2026-08-25

A sessão inicial deixa de ser estritamente dual: (1) o território declara o
ator 3 da família `Pattern`; (2) suas capacidades são `Contágio` e `Rasgo`;
(3) ele entra em estado dormente na abertura; (4) cresce na espiral; (5) vira
`Rasgo` na cena de ruptura; (6) recua no balanço; (7) recebe presença contínua
no painel; (8) a telemetria passa a apresentar três atores ativos; (9) uma
relação `Contagion/Time` liga Atrito 2 a ele; e (10) a relação existente
`Friction/Matter` continua limitada a Chão 1 → Atrito 2.

Esses três corpos continuam doubles nativos e não devem ser confundidos com
flauta, acid, tuba ou outro material do Studio. A próxima potencialização
musical material é integrar uma única família por adaptador versionado, depois
de cumprir seus gates de package, hash, licença e A/B humano.

### Primeiro corpo Rasgo integrado — 2026-08-25

O build atual cumpre os gates técnicos do primeiro adapter: package CMake v1
isolado, SHA-256 verificado na configuração, `CoreModalMatterAdapter` sem heap,
parâmetros bounded, A/B técnico contra o double, inserção na ecologia e stress
offline. O corpo modal assume Chão/Corpo/Vestígio conforme a constelação; recebe
fundamental do ator escolhido, brilho das pressões de matéria/atrito e
excitação periódica da pressão temporal. O status `MODAL v1` identifica o build
inequivocamente.

Isso é matéria real do core Rasgo, mas ainda não é validação musical. A escuta
deve decidir ataque, decaimento, nível, papel e valor relacional antes de
autorizar qualquer segundo adapter.

**Validação humana:** em 2026-08-25, o usuário confirmou que `MODAL v1` aparece
no cabeçalho do binário recompilado. Isso valida binário/package ativo e elimina
a ambiguidade com uma instância anterior; a avaliação musical por escuta segue
pendente.

### Codificação dos textos

O target do app fixa entrada e execução em UTF-8 (`/utf-8` no MSVC e
`-finput-charset=UTF-8 -fexec-charset=UTF-8` no GCC). O arquivo da interface
foi validado como UTF-8; portanto acentos portugueses e os sinais `·` e `→`
não dependem do locale usado para compilar. Essa regra é específica ao host e
não altera o Studio.

Big Shoulders é um TTF variável. O cabeçalho agora seleciona sua instância
nomeada `Bold` diretamente, em vez de aplicar negrito sintético sobre a
instância padrão fina; isso mantém a métrica do wordmark coerente com o Studio.

### Regra obrigatória para criação e revisão de texto no host

`juce::String(const char*)` interpreta a entrada como **ASCII**, mesmo que o
arquivo C++ esteja salvo em UTF-8. Esta é a causa que pode transformar `·` em
`Â·` e acentos em sequências estranhas. A regra permanente do Performance é:

1. todo texto visível não-ASCII passa por `utf8(...)`, que constrói
   explicitamente `juce::CharPointer_UTF8`;
2. é proibido passar literal não-ASCII diretamente a `juce::String`,
   `setText`, `setButtonText`, `drawText`, concatenação ou inicializador de
   componente;
3. ao criar ou alterar texto de código, preferir escapes UTF-8 ASCII, por
   exemplo `utf8("\\xc2\\xb7")`, `utf8("\\xc3\\x81UDIO")` e
   `utf8("\\xe2\\x86\\x92")`; isso torna o resultado independente de editor,
   locale e plataforma;
4. o target fixa `JUCE_STRING_UTF_TYPE=8`, `/utf-8` no MSVC e
   `-finput-charset=UTF-8 -fexec-charset=UTF-8` no GCC;
5. toda alteração de interface deve passar por varredura de literais
   não-ASCII e recompilação antes de ser entregue. A validação humana continua
   necessária, mas não se declara mais a fonte “corrigida” sem ela.

Os componentes de desenho agora recebem `juce::String` já convertido, em vez
de converter um `const char*` internamente. Isso elimina a rota ambígua que
restava nos títulos de zonas, no grafo de relações e no indicador de REC.

**Validação humana:** em 2026-08-25, o usuário confirmou no binário recompilado
que UTF-8 está correto.

### Tradução do layout existente

| Rasgo Synth Studio atual | Rasgo Synth Performance |
|---|---|
| cabeçalho com logo, nível/master, tutorial e idioma | mesmo cabeçalho Rasgo, acrescido de estado de áudio, REC e PanicSafe |
| seed, tempo, pesos de jogo e diretório de saída | seed/sessão, clock, campo autonomia/intervenção e estado de registro |
| genealogia do arco | trajetória da sessão, cena ativa e consequências |
| compor/tocar, timeline e variação | iniciar/parar, Hold/Mutate/Rasgar/Recall e morph de cena |
| painel de módulos manifestados | constelação ativa: atores, papéis e relações audíveis |
| waveform dominante | faixa compacta de história/scope, sem ocupar o centro |
| espectro, granulação, espectrograma e bit-grid simultâneos | um monitor Rasgo selecionável ou visão expandida no segundo monitor |
| log, osciloscópio, retrato de fase e goniômetro | log de gestos/consequências, scope estéreo e segurança de saída |

### Hierarquia de espaço

Em palco, a prioridade do espaço é:

1. segurança, transporte, master e meters;
2. cenas, gestos e células performáticas;
3. atores, papéis, presenças e relações;
4. tempo, memória e trajetória da sessão;
5. log musical/técnico curto;
6. visualizações detalhadas.

Somente meters e um scope compacto permanecem sempre visíveis. Waveform,
espectro, granulação, espectrograma, bit-grid, retrato de fase e goniômetro não
aparecem simultaneamente: ocupam uma baia selecionável, uma sobreposição
temporária ou o segundo monitor. Assim, a linguagem visual do Rasgo continua
presente sem retirar área dos controles que mudam a música.

## Antitotem como referência de anatomia performática

O painel avançado do Antitotem é uma referência autoral interna de funções de
palco. Ele já demonstrou uma organização útil em 1920×1080:

- cabeçalho único com osciloscópio, MASTER, PLAY/STOP/RESET, REC, idioma e
  modos de monitor;
- coluna esquerda para clock, métrica, percurso, feedback, deriva e variações;
- corpo central com fontes/vozes, modulação, espaço e 16 steps;
- coluna direita com mixer, memória, conexão entre objetos e log;
- distinção entre estado global, estado por objeto e relação entre objetos;
- possibilidade de alternar PRINCIPAL/CLONE ou usar dois monitores.

O Performance aprende com essa **anatomia de ação**, mas a acomoda na gramática
espacial e visual do Rasgo Synth já existente. Não copia rótulos CMOS,
semântica do Objeto Sonoro 5, cores, assets, engine dual, cada knob ou o layout
integral. A genealogia deve permanecer visível e documentada, enquanto cada
instrumento conserva sua identidade.

### Tradução de funções

| Antitotem | Rasgo Synth Performance |
|---|---|
| CLOCK / PULSO / MÉTRICA / PERCURSO | clock, groove, arquitetura temporal e direção da sessão |
| 5 OSC / VCF / ADSR | atores ativos por família e seus macros de matéria/articulação |
| PORTAS DE FEEDBACK | relações, infiltrações e permissões de ruptura entre atores |
| DERIVA / VARIAÇÃO | memória, Hold, Mutate, Rasgar, Orbit e comportamento da seed |
| 16 CV com AMP/FX/MUTE | 16 células de gesto/cena com energia, duração, espaço e condição |
| MIXER FILTRO/RING/NOISE/ESPAÇO | presença por família, retorno espacial e master criativo |
| CONEXÃO PRINCIPAL/CLONE | matriz de relações entre atores/cenas, não um clone obrigatório |
| LOG | história performática, consequência e estado técnico |
| PRINCIPAL/CLONE + 1/2 monitores | foco de superfície e visão expandida de atores/relações |

## Navalha 2 como referência operacional

Navalha 2 é uma referência interna de engenharia e workflow, não um instrumento
RASGO nem ancestral estético do Performance. A genealogia permanece explícita:
o Navalha original é de Glerm Soares; Navalha 2 é o upgrade, reescrita e
desenvolvimento de Lúcio Araújo.

Seu app JUCE já demonstra soluções maduras que podem orientar o Performance:

- faixa superior fixa de modos (`PREPARAR`, `FORMA`, `PERFORMAR`,
  `TAKES/MASTER`) e estado de áudio sempre visível;
- layout single/dual monitor responsivo;
- cenas nomeadas, FORM, captura, recall, lock, undo/redo e memória de motivos;
- estado de projeto portátil e compatibilidade versionada;
- fila UI→áudio sem locks, telemetria atômica e gravação em thread própria;
- REC limitado e diagnosticado, meters pós-master e saída live-safe;
- separação entre tocar, preparar material, revisar takes e masterizar;
- tutorial, LEARN contextual e idiomas no chrome, fora da superfície sonora.

### O que traduzir para o Performance

| Navalha 2 | Rasgo Synth Performance |
|---|---|
| PREPARAR / FORMA / PERFORMAR / TAKES | SESSÃO / CENAS / PERFORMANCE / REGISTROS |
| FORM com até 16 cenas | banco de cenas Rasgo com morph e história |
| Motif Memory | memória de gestos, relações e trajetórias |
| Project v2 / Portable Project | Performance Session versionada e portátil |
| TRACE XY | trajetória do campo autonomia/intervenção × continuidade/ruptura |
| Activity Log | história musical + estado técnico da sessão |
| áudio conectado, drops, sample rate | telemetria fixa de dispositivo, CPU e xruns |
| REC e TAKE Timeline | registro de sessão e manifesto de eventos, sem master offline embutido |
| dois monitores | painel principal + atores/relações expandidos |

Não atravessam automaticamente: Source A/B, slices, BLADE, Heritage Pitch,
Library, TRACK/ALBUM MASTER, identidade gráfica e qualquer semântica específica
de recorte/recombinação. O Performance aprende com a arquitetura do produto sem
se transformar em Navalha.

## Adaptação proposta do layout Rasgo

```text
┌────────────────────────────────────────────────────────────────────────────┐
│ LOGO RASGO │ RASGO SYNTH · PERFORMANCE │ SCOPE/METERS │ AUDIO/CPU │ LANG  │
│ SESSÃO/SEED │ CENAS │ MASTER │ PLAY/STOP │ REC │ SCENE │ PANIC SAFE      │
├──────────────┬───────────────────────────────────────────┬─────────────────┤
│ TEMPO        │ CONSTELAÇÃO / ATORES / MATÉRIA            │ MIX / RELAÇÕES  │
│ clock/groove │ Chão · Corpo · Atrito · Contágio          │ presenças       │
│ métrica      │ Rasgo · Vestígio                          │ espaço          │
│ percurso     │ macros, foco e papéis                     │ matriz relacional│
│              ├───────────────────────────────────────────┤ memória/cenas   │
│ GESTOS       │ CAMPO + 16 CÉLULAS PERFORMÁTICAS          │                 │
│ Hold/Mutate  │ autonomia/intervenção · continuidade/risco│ LOG / ESTADO    │
│ Rasgar/Recall├───────────────────────────────────────────┤ consequências   │
│ Orbit        │ MONITOR RASGO SELECIONÁVEL / TRAJETÓRIA                    │
└──────────────┴───────────────────────────────────────────┴─────────────────┘
```

## Zonas

### Sessão, seed e cenas

- carregar nova seed sem iniciar som abruptamente;
- oito cenas acessíveis por teclado, MIDI ou controle;
- captura explícita de cena e proteção contra sobrescrita acidental;
- morph temporal entre cenas, nunca troca seca por padrão;
- indicador inequívoco de cena ativa, cena alvo e estado modificado.

A sessão será versionada e portátil, seguindo a disciplina já provada no
Navalha 2: salvar somente estado necessário, limites explícitos de tamanho e
profundidade, migração documentada e sincronização dos comandos pendentes antes
de gravar o arquivo.

### Atores

Mostra famílias e organismos realmente ativos, não infraestrutura compilada.
Cada linha oferece foco, presença e estado (`dormant`, `emerging`, `active`,
`yielding`, `ruptured`). Solo/mute tradicionais podem existir, mas não são o
centro: o gesto principal é negociar presença e relação.

O primeiro layout mantém faixas centrais na região hoje ocupada pelos módulos
manifestados e monitores do Studio. Cada faixa muda conforme o ator real
selecionado, mas preserva geometria constante para permitir memória muscular.

### Campo de performance

Um campo XY grande concentra dois antagonismos:

- X: autonomia da seed ↔ intervenção humana;
- Y: continuidade/memória ↔ ruptura/risco.

Densidade e tensão ficam em controles separados para não esconder quatro
decisões num único XY. O campo mostra trajetória e retorno, permitindo perceber
história em vez de apenas posição instantânea.

O campo ocupa uma área central, mas não substitui os controles diretos herdados
da clareza do Antitotem. Ele é uma macro de trajetória; atores, relações, cenas
e células continuam acessíveis separadamente.

### Dezesseis células performáticas

As 16 posições preservam a força performática da grade do Antitotem, mas deixam
de representar CVs obrigatórios. Cada célula pode disparar ou sustentar:

- cena ou gesto;
- energia/intensidade;
- duração/gate;
- envio espacial;
- condição ou probabilidade;
- mute/skip sem apagar a programação.

O percurso pode ser linear, reverso, alternado, memória ou outro modelo
documentado. Assim, a grade serve tanto como sequenciador quanto como partitura
de ações de palco.

### Gestos

Botões são ações musicais com ciclo de vida e consequência. `Rasgar` exige
armamento/press-and-hold para evitar disparo acidental; `PanicSafe` permanece
sempre disponível e visualmente separado.

### Tempo

- clock interno, MIDI clock e Ableton Link/OSC ficam como capacidades distintas;
- tap tempo e divisão/multiplicação;
- groove, swing e memória temporal como macros;
- nenhum dispositivo externo muda sample rate ou callback timing como efeito
  musical.

### Espaço e master

Espaço preserva o campo artístico do Rasgo. Master apresenta somente macros
performáticas e a saída segura: nível, pico, true peak, mute/fade e estado do
dispositivo. Controles técnicos perigosos ficam fora da superfície principal.

### Gravação e memória

REC grava a sessão estéreo e um manifesto leve de eventos/cenas. O áudio fica no
diretório próprio do Performance. Nenhum recording usa a pasta do Studio ou é
incluído no GitHub.

`REGISTROS` revisa takes e o manifesto da performance, mas não incorpora TRACK
MASTER ou ALBUM MASTER. Entrega/masterização continuam fluxos separados; o app
de palco não deve crescer até virar uma estação de produção.

## Acessibilidade e palco

- operação completa em 1920×1080 e modo compacto;
- contraste e estado não dependem apenas de cor;
- foco de teclado visível e mapeamento MIDI aprendível;
- áreas críticas com confirmação física/temporal, não diálogos modais;
- sem textos longos, tooltips indispensáveis ou menus profundos durante a
  execução;
- modo escuro de alto contraste e leitura a distância.

## Primeiro teste humano

O primeiro painel deve permitir, sem abrir menus:

1. carregar uma seed e iniciar uma sessão;
2. salvar duas cenas e fazer morph entre elas;
3. focar uma família sem silenciar o restante à força;
4. executar Hold → Mutate → Rasgar → Recall;
5. recuperar segurança com PanicSafe;
6. gravar e encerrar a sessão.
7. programar e percorrer ao menos quatro das 16 células sem abrir menu;
8. alternar foco entre atores preservando a mesma geometria;
9. compreender, a partir da coluna direita, quais relações estão realmente
   ativas.

Se esse percurso exigir explicar o Studio inteiro, o painel ainda não é de
performance.

## Incremento ergonômico — 2026-08-25

A auditoria do Atlas confirmou uma lacuna específica em ergonomia de painéis
performativos. O recorte inicial foi registrado na seção 121 do
`ATLAS_DE_REFERENCIA_RASGO(4).md`, com fontes primárias abertas e licenças
verificadas. As decisões aplicadas aqui são:

- cada controle visível precisa ter consequência sonora testável ou papel de
  monitor explicitamente indicado;
- gráficos arrastáveis conservam uma rota alternativa por slider/teclado;
- foco, seleção e estado não dependem somente de cor;
- cabos representam relações reais, com direção e intensidade legíveis;
- controles críticos de palco devem superar o piso de alvo da WCAG 2.2.

O primeiro experimento implementa uma constelação direta de `Chão`, `Atrito` e
`Contágio`. A altura do nó controla presença e a distância entre atores
adjacentes controla as relações `1 → 2` e `2 → 3`. Os três sliders de presença
e os dois sliders de relação permanecem disponíveis e escrevem no mesmo estado
enfileirado do motor. Falta validação humana do gesto, escala e causalidade
sonora no binário recém-gerado.

Validação técnica do incremento:

- build do app concluído em 2026-08-25 19:32:44 +02:00;
- 6/6 testes aprovados, incluindo stress offline de 43,01 s;
- binário verificado com `MODAL v1` e controles de relação presentes;
- SHA-256: `9406c4256051f039b911b82f0361fb7fe9d92d042cf3de9962c069fd0a8b03f8`;
- tentativa de abertura automatizada encerrou com código 0, mas sem processo ou
  janela confirmados pelo gerenciador; pendência: abrir o mesmo executável no
  terminal gráfico e realizar avaliação humana em áudio real.

## Correção de abertura multi-monitor — 2026-08-25

A janela principal agora consulta explicitamente o display primário do JUCE,
posiciona-se em sua área útil antes de ficar visível e só então solicita o
estado maximizado ao gerenciador de janelas. Isso impede que uma posição de
ponteiro ou janela persistida no segundo monitor escolha o display de abertura.

Validação: app recompilado sem avisos em 2026-08-25 20:10:20 +02:00;
SHA-256 `9f8a5b36d3d58aa7b50741265d842fa843ea22a8b741c89cf44533e40eeacec9`.
Os 6 testes de contratos/motor/controller já haviam passado antes desta mudança
de posicionamento; a confirmação final necessária é visual, no desktop real.

## Reforço de causalidade — Risco, Espaço e Silêncio — 2026-08-25

Os três controles receberam consequências mais explícitas no core:

- `Risco`: aumenta harmônicos de instabilidade, cicatriz e excitação modal;
- `Espaço`: produz abertura estéreo efetiva, com movimento e assimetria entre
  canais, mantendo sinal mono seguro;
- `Silêncio`: faz fade-out perceptível, sustenta o corte por aproximadamente
  0,65 s e retorna gradualmente; `PanicSafe` continua sendo outra função;
- células e tooltips agora explicam `RISCO · RUPTURA`, `ESPAÇO · ABRIR` e
  `SILÊNCIO · CORTE`.

O reset também limpa o estado temporário de silêncio. Build final:
2026-08-25 20:18:17 +02:00, SHA-256
`dd2bb6bdf845ed9a9a1261e083cbfc5fa75ec76503d630642ce85c5e49b45a5c`.
Engine, estabilidade e controller: 3/3 testes direcionados aprovados.

## Correção de STOP — 2026-08-25

O transporte `STOP` zerava o ganho principal, mas a parcela de cicatriz usada
na síntese estéreo ainda era somada diretamente ao canal direito. Isso deixava
uma cauda audível mesmo com o ganho em zero. A parcela agora também é limitada
pela amplitude de transporte.

O `STOP` passa a estabelecer uma fronteira silenciosa no próximo bloco; `PLAY`
continua retornando pela suavização existente. Teste de engine inclui a
verificação de silêncio absoluto nos dois canais: aprovado junto com o teste de
controller. Build final: 2026-08-25 20:25:11 +02:00;
SHA-256 `2639746fe474a18ac8cfdcac36a2d4a757d7525511d6f1b5e2a45ca06f7e21da`.

## Abertura no monitor principal — segunda correção — 2026-08-25

O primeiro ajuste ainda dependia de `setFullScreen(true)`, que no X11 permite
ao gerenciador restaurar o monitor secundário. A janela agora usa diretamente
`primaryDisplay->userBounds` como geometria maximizada, reaplicando essa área
depois do mapeamento da janela; a chamada de fullscreen foi removida.

Build: 2026-08-25 20:34:56 +02:00. SHA-256
`2d0e4102b0b2f50873b006aaaee2e5e3d34c46cd4269974a5fe6c26c2ae2e902`.
Validação final ainda é visual no desktop com dois monitores.

## Transporte sincronizado — 2026-08-25

O snapshot de telemetria agora expõe `playing` por atômico. A interface atualiza
o botão `PLAY/STOP` pelo estado confirmado no motor, além da resposta imediata
ao clique; isso evita divergência visual quando uma cena, atalho ou fila de
transporte muda o estado.

Build: 2026-08-25 20:55:29 +02:00. SHA-256
`caf7ca3ac3ca8b3003f49fbdb3e8d232d273dbfb2743d4179e8cca8bbdd12bf4`.
Cinco suítes não-stress passaram: contratos, engine, adaptador modal,
documento e controller.

## Status de transporte no cabeçalho — 2026-08-25

O cabeçalho agora diferencia explicitamente `PLAY · ÁUDIO ATIVO` de
`STOP · SAÍDA SILENCIOSA`, usando o mesmo snapshot confirmado pelo motor que
atualiza o botão. Build do app: 2026-08-25 21:09:15 +02:00;
SHA-256 `8ffde22fa75dd43b764392b047809d179f18309f87f205d0a03247d33b17dd34`.

## Feedback de ação — 2026-08-25

Macros, presenças e relações agora escrevem nome e valor no campo `última
ação`. O músico consegue confirmar imediatamente qual controle foi acionado e
qual estado foi enfileirado, mesmo quando a consequência sonora é gradual.

Build: 2026-08-25 21:29:35 +02:00. SHA-256
`366eec5e20e2e3db0c8c72ebed96f6582eaadc0e475feb82eccafaee8b0517e3`.

Validação de encerramento do incremento: 6/6 testes passaram, incluindo stress
offline de 48,14 s.

## Semântica de Espaço — 2026-08-25

`ESPAÇO` agora funciona como largura espacial: valor 0 mantém o sinal próximo
do centro/mono; valor 1 abre progressivamente os dois canais e adiciona
movimento/reflexo. O controle deixa de deslocar a imagem inteira para um lado,
tornando a relação entre nome, gesto e resultado mais intuitiva.

Engine e stress: 2/2 testes passaram. Build: 2026-08-25 22:06:05 +02:00;
SHA-256 `cd411a0390b933a283a5e9afcc86f59866896a71d2e85d5e499db769d39dacfa`.

## Décima etapa contínua — causalidade e regressão — 2026-08-25

Este ciclo acrescentou testes explícitos para:

- `Espaço` partindo de centro/mono e abrindo os dois canais;
- `Silêncio` alcançando saída claramente silenciosa;
- telemetria preservando `PLAY/STOP` e `PanicSafe`;
- tooltips distinguindo risco, largura espacial e consequências modais.

Suíte completa: 6/6 testes aprovados, stress offline de 44,85 s. Build final:
2026-08-25 22:16:28 +02:00. SHA-256
`0555effcff14625290b482804650a6cc822ef42fabefe29d25fced620714e2cf`.

## Décimo ciclo de palco — auditoria encerrada — 2026-08-25

Foi revisado o caminho de `PanicSafe`, `Silêncio`, recall de cenas, telemetria
de transporte, atalhos, tooltips e limites `0…1` dos controles. A tarefa local
foi corrigida para refletir o estado real: três atores já estão no host; seed
editável e validação visual de XY/células/constelação continuam pendentes.

Suíte completa novamente aprovada: 6/6, com stress offline de 47,09 s. Como
este ciclo alterou apenas testes e documentação, o binário permanece o mesmo:
SHA-256 `0555effcff14625290b482804650a6cc822ef42fabefe29d25fced620714e2cf`.

## Décimo primeiro ciclo — transporte e retorno audível — 2026-08-25

O teste de motor agora confirma o ciclo completo `PLAY → STOP → PLAY`: o
`STOP` mantém a saída zerada e o `PLAY` retorna por fade-in sem reconstruir a
sessão. No host, falhas de enfileiramento do transporte e do `RESET SAFE` agora
aparecem na última ação, em vez de atualizar a interface como se o comando
tivesse sido aceito.

Também foi feita uma varredura de texto para preservar UTF-8 explícito nos
arquivos do Performance; não há caractere de substituição (`�`) nem sequência
de mojibake nos textos ativos (a única ocorrência de `Ã`/`Â` é a nota histórica
que documenta o defeito já corrigido).

Suíte completa: 6/6 testes aprovados, stress offline de 46,53 s. O binário foi
recompilado e relinkado neste ciclo: 2026-08-25 23:51:13 +02:00; SHA-256
`a51ef50cb733e76c4166a1203fd8f10c48e63f0a5230a0f06a91dc5b55b51651`.

## Primeiro incremento de stems vivos — 2026-08-26

O Performance agora separa internamente até cinco atores e o retorno modal.
O bus acompanha `actorId`, enquanto o papel pode mudar durante cenas e morphs.
`PerformanceStemFrame` é fixo, não aloca e reconstrói exatamente a antiga soma
mono; um teste de 4.096 amostras confirmou erro inferior a `1e-12` e energia
real nos três atores declarados.

O motor publica pico e identidade dos stems por telemetria atômica. No painel,
os nós da constelação ganharam halos e valores de atividade, e o cabeçalho
mostra a contagem `STEMS`. Ainda não há gravação multitrack nem importação de
stems do Studio.

Suíte completa: 6/6 testes aprovados, incluindo stress offline de 53,53 s.
Binário: 2026-08-26 00:06:54 +02:00; SHA-256
`ed54ac96b0240ffc1a1c48ebffcf3b5b6b0b94e7f5f2ca68c049f61f1ac85b1e`.

## Mesa de Matérias v1 — 2026-08-26

Cada uma das três faixas de ator agora reúne `PRES`, `MIX`, `MUTE` e `SOLO`.
`PRES` continua sendo nutrição do ator e pode ser capturada na cena; `MIX`,
`MUTE` e `SOLO` são decisões de palco persistentes através de recall, como o
MASTER. Mute/solo usam fade e não interrompem transporte, relações ou o motor.

O teste de motor confirmou: ganho zero remove gradualmente um stem; mute não
para os demais; solo isola o ator escolhido e a liberação restaura o conjunto.
O painel mostra essa consequência nos halos pós-mesa da constelação.

Suíte completa: 6/6 testes aprovados, incluindo stress offline de 57,59 s.
Binário: 2026-08-26 00:29:00 +02:00; SHA-256
`bb9f79aca5a48b97716795e369f9cf639f4720f9fc4dcaf334bb1a3f271f1445`.

## Territórios determinísticos por seed — 2026-08-26

O objetivo deste incremento foi fazer a `SEED` alterar uma sessão musical
completa, e não apenas aparecer como rótulo. `PerformanceTerritoryFactory`
gera, sem aleatoriedade global, um documento `RSPERF 1` válido a partir de um
inteiro: identidade de território, três atores nativos e quatro cenas com
macros e relações derivadas. A mesma seed sempre serializa o mesmo documento;
seeds diferentes produzem outro território.

O campo numérico e o botão `NOVO TERRITÓRIO` foram incluídos no cabeçalho. O
fluxo é deliberadamente seguro para palco: `STOP → SEED → NOVO TERRITÓRIO →
PLAY`. O controlador recusa a troca enquanto o motor está em execução e a
entrega por uma fila SPSC fixa de duas mensagens; a substituição acontece no
início de `processBlock`, sem I/O ou alocação no callback. Depois da troca, o
host sincroniza a cena inicial e restaura a Mesa de Matérias ao estado neutro.

Arquivos principais: `PerformanceTerritoryFactory.hpp`,
`PerformanceSessionController.hpp`, `PerformanceApp.cpp` e
`test_performance_territory_factory.cpp`. Limitação: ainda falta escuta humana
para avaliar se cada variação de seed cria contraste musical suficiente e se o
campo está legível na resolução de palco.

Validação: 7/7 CTests do núcleo (incluindo stress offline de 18,98 s) e 2/2
testes pertinentes no build do app. Binário recompilado: 2026-08-26 04:12:02
`+02:00`; SHA-256
`2c920cd9844205efb103cf25a81bc8b5d1b981d9d3cddce043aa27d658aa793f`.

Próximo passo recomendado: testar duas ou três seeds no dispositivo de áudio
real e registrar se cena, gestos, stems e resposta de `Risco`/`Espaço` tornam
as diferenças imediatamente percebidas.

## Décimo segundo ciclo — órbita, limites e leitura de território — 2026-08-26

Foram concluídas dez etapas contínuas no mesmo incremento:

1. auditoria da constelação e de sua causalidade de áudio;
2. criação da relação `Órbita 1 → 3` em cada território de seed;
3. incorporação da Órbita à sessão inicial, para que não exista somente após a
   primeira troca de seed;
4. tradução da relação espacial para pressão contínua da ecologia;
5. aplicação dessa pressão ao movimento/fase dos atores;
6. aplicação da mesma pressão ao campo estéreo do motor;
7. extensão do grafo a três cabos, formando o triângulo arrastável;
8. terceiro controle de relação com tooltip e captura/recall de cena;
9. efetivação dos limites de memória e risco da identidade de território no
   factory, motor e captura de cena;
10. apresentação desses limites no rodapé de ação e no alcance físico dos
   controles, seguida de build e regressão completos.

`ÓRBITA 1 → 3` não é panorâmica decorativa: sua intensidade aumenta a
diferença estéreo mesmo quando o macro `ESPAÇO` começa em zero. A seed também
passa a definir tetos reais de `MEMÓRIA` e `RISCO`; o painel os mostra como
`memória ≤ …` e `risco ≤ …`, e qualquer captura preserva o orçamento em vez de
reintroduzir um valor incompatível.

Validação: 7/7 CTests no build de contratos, incluindo stress offline de
20,12 s; 3/3 testes relevantes no build do app. O teste de motor cobre agora
explicitamente a largura causada pela Órbita. Binário recompilado: 2026-08-26
12:01:50 `+02:00`; SHA-256
`c5413fa9fe4d1c53a18571026c9a965fc47b15d4c9432c363c965400afdeaccb`.

Limitação: a causalidade foi verificada por sinal e por testes, mas ainda falta
escuta humana para decidir se a Órbita é musicalmente diferenciável de
`ESPAÇO` e se os tetos de uma seed são expressivos, não restritivos demais.
Próximo passo: tocar duas seeds com Espaço em zero, subir apenas a Órbita e
anotar abertura, movimento, tensão e utilidade de palco.

## Décimo terceiro ciclo — DNA sonoro de território — 2026-08-26

Dez etapas contínuas converteram a identidade de seed em realização sonora:

1. mapeamento dos campos de identidade ainda inertes;
2. introdução de identidade imutável na ecologia;
3. propagação segura da identidade quando uma cena entra no motor;
4. escala de registro por `tonalVocabulary`;
5. deriva individual por `temporalDna`;
6. peso de pressão material por `matterBias`;
7. peso de pressão temporal por `relationBias`;
8. atualização de frequências de atores já vivos sem recriar a sessão;
9. teste A/B da mesma constelação sob dois DNAs válidos e contrastantes;
10. correção do rótulo do eixo XY para `AUTONOMIA`, documentação e rebuild.

O primeiro motor ainda é deliberadamente pequeno, mas a seed agora atravessa
o caminho inteiro: território → constelação → ecologia → sinal. Ela não é um
nome de preset. Os limites de memória/risco continuam sendo a parte normativa
desse DNA; vocabulário tonal, temporalidade, matéria e relação são a parte
expressiva.

Validação: 7/7 CTests do núcleo, com stress offline de 20,44 s; 3/3 testes
relevantes no build do app. O novo teste confirma diferença de energia acima
do limiar entre duas identidades aplicadas à mesma cena. Binário recompilado:
2026-08-26 12:08:51 `+02:00`; SHA-256
`0927b87e66d69e48b269c7672b9f91cc5d1cba11bb32e735e9b072bad1983e93`.

Próximo passo recomendado: escuta A/B de duas seeds com os mesmos gestos e
mesma cena; somente depois decidir se o próximo ganho vem de mais corpos
materiais, de relações temporais ou de gravação de trajetória.

## Décimo quarto ciclo — pulso nativo e tempo de palco — 2026-08-26

Dez etapas contínuas acrescentaram temporalidade executável ao primeiro motor:

1. auditoria do sinal contínuo e da ausência de pulse;
2. relógio por ator, sem alocação e preservado em morph;
3. leitura do `tempoBpm` já pertencente à sessão;
4. subdivisão distinta por papel (`Chão`, `Corpo`, `Atrito`, `Contágio`,
   `Rasgo`, `Vestígio`);
5. duração de pulso modulada por densidade, memória, risco e tensão;
6. peso de pulso que mantém Chão/Vestígio respirando e faz os demais mais
   articulados;
7. influência de `temporalDna` e da pressão temporal sobre a subdivisão;
8. correção do recálculo de pressão espacial para impedir acúmulo após morphs;
9. fila SPSC de tempo, com faixa segura `20…300 BPM`;
10. editor de BPM, atalhos `,`/`.` e captura de tempo por cena.

O fluxo agora é: seed propõe DNA temporal e tempo inicial; o performer altera
o BPM sem reconstruir a sessão; cenas podem guardar esse tempo. A pulsação não
é um sequenciador fechado nem pretende reproduzir o Studio: é a primeira
articulação contínua dos três corpos nativos.

Validação: 7/7 CTests no núcleo, incluindo stress offline de 21,39 s; 3/3
testes relevantes no build do app. Os testes cobrem BPM válido/inválido no
motor e na fila de controle. Binário recompilado: 2026-08-26 12:45:12
`+02:00`; SHA-256
`c8bf46dec2fd0b1cb0331f0c7a84b278ad1a7d9447e046169edaeb4b5080325f`.

Próximo passo recomendado: testar `90`, `126` e `160 BPM` em uma mesma seed,
variando Densidade e Órbita; registrar se o pulso reforça os papéis sem tornar
o instrumento previsível demais.

## Décimo quinto ciclo — Rasgo, cicatriz e reparo — 2026-08-26

Foram concluídas dez etapas para dar consequência ao gesto `RASGAR`:

1. auditoria do decaimento transitório anterior;
2. criação de uma cicatriz interna limitada `0…1`;
3. acionamento conjunto de transiente e cicatriz por Rasgar;
4. decaimento lento e estável da cicatriz;
5. transmissão da cicatriz para a matéria dos stems;
6. transmissão para a cicatriz espectral/espacial da saída;
7. redução deliberada por Recall;
8. remoção total por PanicSafe;
9. telemetria atômica `scarMicros` e leitura `CICATRIZ` no status;
10. tooltips de gesto/célula, teste de regressão e relink do host.

`RASGAR` deixa agora uma marca audível além do ataque; `RECALL` é um reparo
parcial e `PANIC SAFE` é uma fronteira de recuperação integral. O novo teste
verifica precisamente o pico inicial, a permanência após 128 blocos, a redução
por Recall e o apagamento por PanicSafe. Não há persistência em disco: a
cicatriz pertence à sessão viva e será candidata a um trace somente depois de
uma decisão de formato e gravação assíncrona.

Validação: 7/7 CTests do núcleo, incluindo stress offline de 24,19 s; 3/3
testes relevantes no build do app. Binário recompilado: 2026-08-26 19:38:14
`+02:00`; SHA-256
`bea370664d3fdc094fb2e82aa5fac936147a5f3aa9006dc4fa5d549dbe035d51`.

Próximo passo recomendado: tocar Rasgar uma vez com risco baixo e outra com
risco alto; observar a duração/legibilidade de `CICATRIZ`, recuperar com
Recall e decidir se a cicatriz deve futuramente alterar também papéis ou
relações da constelação.

## Décimo sexto ciclo — Hold temporal da constelação — 2026-08-26

Dez etapas contínuas deram consequência temporal ao gesto `HOLD`:

1. auditoria da antiga trava limitada à frequência;
2. estado de hold próprio na ecologia;
3. propagação do evento Hold ao relógio dos atores;
4. congelamento de fase de pulso sem parar áudio ou transporte;
5. preservação do pulso durante morph;
6. liberação pelo evento Release no mesmo caminho realtime;
7. limpeza explícita do hold por PanicSafe e reset;
8. telemetria atômica do estado;
9. indicador `HOLD` no status do host;
10. teste de evento, regressão completa e relink do binário.

HOLD pode agora suspender a articulação da constelação: o músico congela o
momento rítmico e pode, a partir dele, introduzir Mutate, Rasgar, Recall ou
Release. Ele não silencia, não reinicia a seed e não cria operação no callback.

Validação: 7/7 CTests do núcleo, incluindo stress offline de 25,00 s; 3/3
testes relevantes no build do app. O teste confirma que Hold chega à
telemetria depois de processado e que Release o reverte pelo mesmo caminho.
Binário recompilado: 2026-08-26 19:44:54 `+02:00`; SHA-256
`616436755ed646390a432e438cbd99d4437acb3aafb2bee48abaa66b9a0cd1f0`.

Próximo passo recomendado: com BPM 126, experimentar `HOLD → RASGAR →
RELEASE`, comparando a cicatriz no pulso congelado e no pulso livre. Avaliar
se esse gesto já possui contraste suficiente para palco antes de introduzir
quantização explícita de eventos.

## Décimo sétimo ciclo — grade de gesto musical — 2026-08-26

Dez etapas contínuas criaram a primeira grade temporal de palco:

1. separação entre gestos musicais e gestos de segurança;
2. flag de quantização no motor;
3. cálculo sem alocação do próximo beat a partir de BPM e sample cursor;
4. agendamento de Hold/Release/Mutate/Rasgar/Recall no próximo beat;
5. preservação de Silence e PanicSafe como eventos imediatos;
6. telemetria atômica de GRID;
7. fila SPSC própria para a configuração de grade;
8. teste do controlador e do motor;
9. botão `GRID` no cabeçalho e atalho `G`;
10. indicador de status, regressão completa e relink.

Com `GRID ON`, um gesto musical não interrompe arbitrariamente o pulso: ele
entra no próximo beat. Isso torna possíveis entradas de Rasgar/Recall e
suspensões de Hold mais legíveis em BPM definido. A grade não bloqueia a
saída de risco: `SILÊNCIO` e `PANIC SAFE` permanecem imediatos mesmo ativada.

Validação: 7/7 CTests do núcleo, incluindo stress offline de 25,70 s; 3/3
testes relevantes no build do app. O teste prova que Rasgar aguarda na fila
até o beat, deixa cicatriz ao entrar e que PanicSafe a limpa imediatamente.
Binário recompilado: 2026-08-26 19:49:52 `+02:00`; SHA-256
`8023a6bcdae8b878c095b5c42245483c780551a7d10baf91eab140d95dc09d64`.

Próximo passo recomendado: ativar `GRID`, usar `MUTATE → RASGAR → RECALL` em
três beats sucessivos e comparar a legibilidade com GRID desligada. Se a
grade for musicalmente útil, a próxima evolução pode oferecer divisão de
grade (beat, meio-beat, compasso) sem reescrever os gestos.

## Décimo oitavo ciclo — Constelação tocável por nó e cabo — 2026-08-26

Nota de continuidade: este registro descreve o comportamento histórico daquele
ciclo. Em 2026-08-27, presença foi desacoplada da posição; as setas e o arrasto
agora movem o módulo em 2D e os faders da MESA controlam presença.

Dez etapas contínuas tornam o campo central uma superfície de intervenção
real, em vez de ilustração do mixer:

1. preservação do modo `MESA` e do modo reversível `FOCO` já existentes;
2. seleção explícita de cada nó no desenho;
3. destaque visual persistente do nó selecionado;
4. seleção direta de cada cabo pela proximidade do cursor;
5. destaque textual do cabo selecionado e do seu valor atual;
6. arrasto vertical de nó limitado para controlar sua presença;
7. arrasto horizontal de cabo limitado para controlar sua relação;
8. teclado `1`–`3` para selecionar nó e `↑/↓` para presença;
9. teclado `C` para percorrer cabos e `[`/`]` para intensidade, com `Shift`
   para passo maior;
10. atualização de sliders, fila SPSC, última ação, tooltip e regressão sem
    adicionar I/O, alocação ou lock ao callback.

Os controles gráficos, barras e teclado continuam sendo três caminhos para o
mesmo estado musical: presença do ator e intensidade de relação. Assim, o
arrasto não se torna requisito de operação. A distância entre nós permanece
uma composição gestual: ao mover um nó, as relações adjacentes acompanham a
geometria; ao manipular um cabo, somente a relação escolhida muda. Cada ação
deixa o valor e o nome no campo `última ação`.

Validação técnica: 7/7 CTests do núcleo, incluindo stress offline de 21,37 s;
3/3 testes relevantes do build do app. Binário recompilado: 2026-08-26
20:16:43 `+02:00`; SHA-256
`449dda397f96c8950b10e12374c3b132d58afecf055475c04f646e3500389423`.

Limite atual: esta é validação de compilação e contrato, não avaliação humana
de ergonomia ou de som. Próximo ensaio: em `FOCO`, tocar `1 → ↑/↓ → C → [/]`
durante `GRID ON`, comparar com os sliders e observar se Órbita é compreendida
como espaço antes de acrescentar qualquer novo módulo DSP.

## Décimo nono ciclo — GRID com divisões musicais — 2026-08-26

Dez etapas contínuas aprofundam a GRID sem transformar o host em sequenciador:

1. enum tipado e validado para as divisões `BEAT`, `1/2` e `1/4`;
2. preservação do modo desligado;
3. cálculo de entrada pelo cursor de samples já realtime-safe;
4. divisão derivada de BPM, sem relógio externo ou alocação;
5. fila SPSC passa estado e divisão juntos;
6. telemetria atômica declara a divisão realmente ativa;
7. status mostra `GRID BEAT`, `GRID 1/2` ou `GRID 1/4`;
8. botão e tecla `G` percorrem `OFF → BEAT → 1/2 → 1/4 → OFF`;
9. Silence e PanicSafe seguem fora da quantização;
10. testes cobrem divisão de quarto, transporte pela fila e rejeição de
    divisão inválida.

Na prática, `BEAT` dá entradas estruturais, `1/2` permite resposta mais ágil e
`1/4` aproxima gesto e pulso sem delegar a decisão ao sequenciador. A divisão
é uma grade de chegada para Hold/Release, Mutate, Rasgar e Recall; não altera
a forma da matéria, a seed ou a velocidade do transporte.

Validação técnica: 7/7 CTests do núcleo, incluindo stress offline de 23,60 s;
3/3 testes relevantes do build do app. Binário recompilado: 2026-08-26
20:24:00 `+02:00`; SHA-256
`25fa3a2f6cd21646cd9d036cc40c979f06196067e9606cf4cd4f5d58e3467999`.

Limite atual: não houve escuta humana deste comportamento. Próximo ensaio:
mesma seed e BPM 126, repetir `MUTATE → RASGAR → RECALL` em `BEAT`, `1/2` e
`1/4`; reter somente as divisões que ajudam a frase musical em vez de induzir
um automatismo rítmico.

## Correção de legibilidade — fontes e alvos — 2026-08-26

O feedback de teste humano apontou retorno de glifos incorretos no rodapé de
relações/território e controles pequenos. A causa encontrada foi objetiva: os
arquivos IBM Plex Mono já eram empacotados ao lado do binário, mas o chrome
ainda solicitava a família genérica `Monospace` do sistema. O host agora usa
IBM Plex local como fonte primária e reserva fallbacks somente para glifos
excepcionais. Isso fixa métrica e cobertura de português/símbolos ao binário,
em vez de depender da sessão gráfica da máquina.

Também foram ampliados `FOCO`, `CAPTURAR`, `DESFAZER`, morph e os botões de
stem; os sete knobs de Nutrição passam a duas linhas (quatro + três), e `FOCO`
lhes dá 60% da coluna direita. A mudança é somente de host: não altera o motor,
as filas nem o áudio. Recompilado e validado com 7/7 CTests do núcleo (stress
21,63 s) e 3/3 testes relevantes do app. Binário: 2026-08-26 22:18:30
`+02:00`; SHA-256
`a24f794649a02718175472bea1240fcc6067f185fd9d290957252614e4c7cfca`.

Ainda falta a confirmação humana de que o rodapé, `novo território` e os knobs
estão legíveis na tela real; não havia instância antiga do app aberta ao fim do
incremento.

### Ajuste após teste — cenas compactas

O teste apontou que os botões `ABERTURA`, `ESPIRAL`, `RASGO` e `BALANÇO` tinham
ganhado peso visual excessivo. A faixa de cenas foi reduzida de modo a manter
alvos de 42 px, devolvendo altura ao campo e aos knobs. Não há mudança de
função, áudio ou atalhos. Build/testes: 7/7 núcleo (stress 21,62 s) e 3/3 app;
binário 2026-08-26 22:20:44 `+02:00`, SHA-256
`cd5d4972b481da50f5891ed5ba33c0d35c14e56dba8b175c69ea97585345e3d7`.

## Vigésimo ciclo — consequência audível dos macros — 2026-08-26

O teste humano identificou uma lacuna decisiva: alguns controles possuíam
caminho técnico, mas não contraste suficiente para palco. Dez etapas foram
aplicadas no motor nativo:

1. auditoria de cada macro contra o sinal estéreo renderizado;
2. aumento limitado da deriva tonal/temporal conduzida por `AUTONOMIA`;
3. manutenção da individualidade por ator nessa deriva;
4. criação de cor cruzada proporcional a `INTERVENÇÃO`;
5. ligação da cor ao pulso de cada ator, sem ser ruído decorativo;
6. ampliação do resíduo de fase de `MEMÓRIA`;
7. prolongamento suave da energia de `MUTATE`;
8. reforço da interação Mutate ↔ relações temporais;
9. aumento do limiar de teste: todo macro precisa agora causar diferença
   estéreo não trivial, não apenas diferença numérica microscópica;
10. revisão dos tooltips para declarar a consequência musical real.

Resultado esperado: Autonomia altera deriva, Intervenção acrescenta cor e
movimento, Memória deixa rastro, e Mutate sustenta uma mudança de timbre por
tempo suficiente para ser tocado. Densidade, Tensão, Risco, Espaço, cenas,
stems, Rasgar, Recall, Silêncio e segurança preservam seus papéis anteriores.

Validação técnica: 7/7 CTests do núcleo, incluindo stress offline de 24,31 s;
3/3 testes relevantes no app. Binário recompilado: 2026-08-26 23:08:33
`+02:00`; SHA-256
`0130d35c141114399f557e458b5ac533bf6d9d5a61424aa0295d832c94f41605`.

Limite: o contraste foi provado por sinal e testes, não ainda por escuta
humana. Próximo ensaio: mover cada macro de `0.10` para `0.90` isoladamente,
com a mesma cena/seed; depois tocar `W → E → R` e avaliar se Mutate, cicatriz e
reparo se distinguem sem precisar olhar para a tela.

## Vigésimo primeiro ciclo — cabos com consequência musical — 2026-08-26

Dez etapas reforçam a Constelação como partitura de relações ativas:

1. auditoria de pressões por domínio de relação;
2. confirmação de que Atrito já chegava à matéria, mas discretamente;
3. reforço harmônico limitado de matéria;
4. uso explícito da pressão de fricção na cor dos stems;
5. reforço temporal do Contágio na deriva de cada ator;
6. reforço do Contágio no ritmo de pulso;
7. ampliação limitada da contribuição espacial da Órbita;
8. preservação do macro Espaço como caminho independente;
9. novos testes A/B para matéria/fricção e tempo/contágio;
10. otimização para reutilizar harmônicos já calculados, preservando CPU.

Consequências esperadas: `ATRITO 1 → 2` torna a matéria mais áspera e rica;
`CONTÁGIO 2 → 3` muda pulso e deriva; `ÓRBITA 1 → 3` abre/movimenta o estéreo.
Os três cabos seguem editáveis pelo slider, pelo grafo e pelo teclado, sem
criar uma rota paralela de estado.

Validação técnica: testes não-stress 6/6, simulação de 20 min em 23,213 s
(51,70× realtime), zero alocações em `processBlock`; build do app 3/3. Binário
recompilado: 2026-08-26 23:29:05 `+02:00`; SHA-256
`2f3e0c7f472e2114e1a399333e193a2e10e3a15e62a5778e4845f8e5c765ee53`.

Limite: a prova é técnica, não substitui escuta estéreo humana. Próximo ensaio:
solo de `CHÃO` e `CONTÁGIO`, mover Órbita de 0 a 1; depois comparar Atrito e
Contágio separadamente de 0 a 1, mantendo a mesma cena e BPM.

## Vigésimo segundo ciclo — células de macro realmente performáveis — 2026-08-26

O comportamento anterior das células `AUTON`, `DENS`, `TENS`, `MEM`, `RISCO` e
`ESPAÇO` era inadequado: todas escreviam uma vez `0,78`; o segundo clique não
produzia nova consequência. Dez etapas corrigem isso:

1. identificação do valor fixo repetido;
2. substituição por acento alto/baixo alternável;
3. alvo baixo explícito `0,18`;
4. alvo alto explícito `0,86`;
5. preservação dos limites de território para Memória e Risco;
6. sincronização com os knobs correspondentes;
7. sincronização com o campo XY quando aplicável;
8. sincronização ao recuperar uma cena;
9. rótulo `+`/`−` que declara o próximo clique;
10. tooltip e última ação coerentes com o novo gesto.

Essas células não são presets escondidos: são acentos alternáveis para palco.
Se o valor está baixo, o botão mostra `+` e eleva; se está alto, mostra `−` e
reduz. O motor, as filas e o documento de cenas continuam sendo os mesmos.

Validação: 6/6 testes funcionais do núcleo, stress de sessão simulada de 20 min
em 24,190 s (49,61× realtime), sem alocação em `processBlock`; 3/3 testes do
app. Binário recompilado: 2026-08-26 23:31:12 `+02:00`; SHA-256
`2a179cb184530a4f2f15b6f8b80ab96a5acd1fa4c78e0243fdba2bbb8107961e`.

Próximo ensaio: deixar uma cena em reprodução, clicar a mesma célula três
vezes e confirmar `alto → baixo → alto`; observar especialmente Densidade,
Tensão e Espaço em estéreo.

## Vigésimo terceiro ciclo — Recall como reparo audível — 2026-08-26

Dez etapas fecham a lacuna de Recall em estado limpo:

1. diagnóstico: Recall só era evidente após Rasgar ou Mutate;
2. criação de envelope interno de reparo, limitado e transitório;
3. disparo do envelope por Recall;
4. preservação da redução de Mutate, tear e cicatriz já existente;
5. passagem do envelope ao renderer de stems;
6. suavização temporária dos harmônicos da matéria;
7. manutenção de presença, relação, cena e transporte;
8. limpeza do envelope em reset e PanicSafe;
9. teste A/B de Recall sem cicatriz anterior;
10. tooltip atualizado para não prometer apenas uma ação invisível.

Recall agora é uma inflexão de reparo: depois de Rasgar, reduz a marca; em
estado limpo, ainda abre uma breve janela de matéria suavizada. Não é mute,
não reinicia o pulso e não substitui PanicSafe.

Validação: 6/6 testes funcionais do núcleo, incluindo novo teste A/B de Recall;
stress de 20 min em 24,641 s (48,70× realtime), zero alocações no callback;
3/3 testes do app. Binário: 2026-08-26 23:43:29 `+02:00`; SHA-256
`aab4c061b583435b3712f5b932d94121341314d461215fcaaaf5a1b70502e315`.

Próximo ensaio: `RECALL` sozinho; depois `RASGAR → RECALL`; comparar a janela
de suavização com a redução da cicatriz e decidir se a duração é musical.

## Vigésimo quarto ciclo — Hold com consequência imediata — 2026-08-27

Dez etapas tornam o gesto temporal mais legível pela escuta:

1. diagnóstico de Hold: congelava pulso, mas podia parecer inerte no primeiro
   instante;
2. preservação do congelamento de relógio existente;
3. passagem do estado Hold ao renderer de stems;
4. concentração limitada dos harmônicos enquanto Hold está ativo;
5. preservação de fase, presença, cenas e transporte;
6. Release retorna o estado ao pulso e matéria livres;
7. PanicSafe continua liberando Hold;
8. ausência de envelope extra, I/O ou alocação no callback;
9. teste A/B de sinal e telemetria `holding`;
10. tooltip esclarece a dupla consequência do gesto.

`HOLD` agora congela o pulso e concentra a matéria; `RELEASE` restitui ambas as
dimensões. Não é Silence, não reduz cicatriz como Recall e não é uma trava de
transporte.

Validação: 6/6 testes funcionais do núcleo, incluindo Hold/Release A/B; stress
de 20 min em 23,882 s (50,25× realtime), zero alocações no callback; 3/3
testes do app. Binário: 2026-08-27 00:35:28 `+02:00`; SHA-256
`ba1b9279197f25e5fbd84415dea2359fbc2d665e05b5bbe6704e0c657f2561e6`.

Próximo ensaio: deixar a cena correr, `Q` para Hold, fazer `W` ou `E` enquanto
o pulso está congelado e usar `Q` para Release; comparar a volta com GRID
desligada e em `1/2`.

## Vigésimo quinto ciclo — tomada de mix assíncrona — 2026-08-27

O primeiro passo de gravação do Performance é deliberadamente a **mix estéreo**,
não uma exportação fingida de stems. Dez etapas delimitam a operação:

1. inclusão do módulo JUCE de formatos de áudio no host próprio;
2. criação de `PerformanceMixRecorder`, isolado da ecologia e do Studio;
3. abertura do WAV somente pelo gesto de interface, fora do callback;
4. WAV estéreo de 24 bits, na taxa do dispositivo efetivamente aberto;
5. cópia de blocos para o FIFO bounded de `AudioFormatWriter::ThreadedWriter`;
6. escrita e flush executados no `TimeSliceThread` de baixa prioridade;
7. parada protege o ponteiro do writer antes de sua destruição e nunca roda no
   callback de áudio;
8. botão `REC` e atalho `V`, com estado visível no cabeçalho e na última ação;
9. destino exclusivo, criado apenas ao armar: `Música/Rasgo Synth
   Performance/recordings`;
10. telemetria local de blocos descartados para tornar uma fila saturada
    observável ao encerrar a tomada.

O botão inicia uma tomada e muda para `REC ●`; o segundo acionamento fecha a
tomada e relata sua duração aproximada, nome e eventual saturação. `STOP` só
silencia a saída do instrumento: ele não encerra uma tomada que o performer
tenha escolhido manter aberta. Nenhum arquivo foi criado nesta implementação;
a primeira escrita depende do acionamento humano de `REC`.

Limite: ainda não há stems de ator nem retorno modal escritos em arquivos
alinhados. Essa expansão só deve ocorrer depois de uma tomada humana curta
confirmar caminho, abertura do WAV e ausência de impacto perceptível na sessão.

Validação técnica: build do app concluído; CTests do app 3/3 e contratos/core
6/6 aprovados; stress offline de 20 min em 24,1475 s (49,6946× realtime), com
zero alocações em `processBlock`. Binário recompilado: 2026-08-27 00:42:36
`+02:00`, 31.330.992 bytes, SHA-256
`10ca6756e9f574c45478e0c4926752d31fb94b3d93d1940cdb73b5558b703335`.

Próximo ensaio: abrir esse binário, fazer `REC → uma cena e dois gestos → REC`,
localizar o WAV em `Música/Rasgo Synth Performance/recordings`, ouvir começo e
fim e informar qualquer dropout ou caractere estranho. Só então separar as
vias de stem.

## Redistribuição da superfície — decisão de arquitetura — 2026-08-27

O Performance não é um Studio reduzido nem uma mesa que exponha todos os
parâmetros: é uma prática de **performar a partir de um seed**. O seed abre um
território (matéria, limites, relações e predisposições); a performance é a
trajetória singular de cenas, gestos e escuta dentro dele. A superfície deve
tornar essa narrativa evidente e não apresentar caminhos redundantes.

```text
TERRITÓRIO          CENAS             GESTO              CAMPO / CONSTELAÇÃO
seed + tempo   →    passagens     →   verbos         →    consequência audível
                                      Hold / Mutate          XY, nós e cabos
                                      Rasgar / Recall

MESA (somente preparação): captura, ganho/mute/solo de stems, relações
numéricas, campo/colagem e controles de precisão. A grade legada de 16 células
foi retirada; ela não volta sem contrato performático explícito.
```

### Contrato de redistribuição

| Camada | Finalidade | O que permanece visível | O que não aparece aqui |
|---|---|---|---|
| **PALCO** (padrão) | tocar e escutar sem ler manual | seed/tempo compactos, quatro cenas, cinco gestos, campo XY, sete knobs e constelação arrastável | células duplicadas, captura, mute/solo, ganhos por stem e sliders numéricos de relação |
| **MESA** | preparar, auditar e capturar uma cena | tudo do PALCO mais campo/colagem, captura/desfazer, stems e relações precisas | grade legada de ações ambíguas ou nova rota musical paralela |
| **Cabeçalho persistente** | segurança e estado | áudio, master, REC, PLAY/STOP, SAFE, GRID | controles criativos que disputem atenção com o território |

As 16 células não serão removidas: tornam-se uma grade de preparação e teclado
alternativo, não uma segunda fachada permanente para as mesmas cenas, gestos e
macros. `PALCO` já começa sem elas; o próximo redesenho deve aumentar a
hierarquia espacial de `CAMPO` e `CONSTELAÇÃO`, sem criar novos botões.

### Regra de não sobreposição

Antes de introduzir ou mover qualquer controle, declarar a faixa a que ele
pertence, revisar os limites em largura mínima e máxima e verificar que nenhum
controle visível intercepta outro. O cabeçalho usa as faixas contíguas `GRID |
MASTER | REC | TRANSPORTE`; o novo `REC` foi reposicionado para não cruzar o
MASTER. Sobreposição só é aceitável para elementos puramente gráficos (halo,
meter ou linha), nunca para alvos de interação.

Limite: a redistribuição foi implementada estruturalmente, mas ainda precisa de
avaliação visual humana em monitor real. Não se deve acrescentar controles ao
PALCO até essa avaliação; o próximo incremento é escala, hierarquia e leitura
dos quatro blocos existentes.

## Reserva e constelação tocável — 2026-08-27

`PALCO` passa a declarar uma única ação central: **arrastar um módulo da
reserva para dentro da Constelação 1**. A seed abre com dois corpos audíveis e
um corpo disponível na reserva; a paridade da seed escolhe Gendyn ou Drums como
o segundo companheiro, para que a entrada inicial já tenha identidade e ainda
reste uma decisão real para o músico.

Os cartões dos três corpos circundam o campo em vez de formar uma barra de
botões: Acid à esquerda, Gendyn acima e Drums à direita. Mesmo quando já está
ativo, o cartão mostra seu estado e continua sendo seu destino de retorno. A
geometria deixa o centro livre para relações e torna visível, numa só leitura,
o que ainda pode entrar e de onde cada corpo pode sair.

Ao soltar um corpo no campo, ele entra por presença suavizada, seus cabos só se
tornam pressionados se o outro extremo também estiver presente e um pequeno
painel se abre na coluna direita. O painel local contém três barras homogêneas
mais MIX, MUTE e SOLO. Ao devolver o nó à reserva, o painel desaparece e a
presença vai a zero; não há botão escondido, duplicação de controle nem
instanciação de DSP.

O estado ativo é único: arrastar, usar a presença precisa na MESA ou a
alternativa de teclado atualiza ao mesmo tempo áudio, nó, reserva e painel. Um
corpo não pode permanecer desenhado como disponível enquanto continua audível,
nem conservar painel quando sua presença chega a zero.

| Corpo | Painel local | Consequência no DSP real |
|---|---|---|
| Acid | `CORTE`, `RESO`, `ARTIC` | cutoff, ressonância, acento/slide e energia tonal |
| Gendyn | `PASSO`, `TEMPO`, `FLUXO` | passo, duração e energia estocástica |
| Percussões | `ATIV.`, `FRAG.`, `ABERT.` | atividade, fragmentação e abertura de hats/eventos |

Esta versão realiza uma constelação por vez. Várias constelações serão futuras
ilhas dentro do mesmo campo — com borda, cor e orçamento próprios — e não
vários painéis empilhados. O requisito de entrada será sempre o mesmo: reserva
visível, destino legível, resposta audível e painel proporcional ao módulo.

Validação pendente: teste humano do gesto completo no binário novo. Confirmar
que arrastar o corpo de reserva para o campo cria uma diferença imediata, que
seus três controles são distinguíveis e que devolvê-lo à reserva retira somente
aquele corpo sem interromper a seed.

## Painel ligado às ecologias da seed — 2026-08-27

O painel não chama mais todos os atores de Acid/Gendyn/Drums quando o motor
renderiza outra formação. `PerformanceModuleCohortAdapter::descriptorForSeed`
é a fonte única para os três cartões, labels de presença, painéis locais,
relações e indicação de coorte no cabeçalho:

| Seed de ensaio | Cabeçalho | Cartões e painéis locais |
|---|---|---|
| 181 | `COORTE v2 · CIRCUITO` | Acid, Gendyn, Drums |
| 182 | `COORTE v2 · SOPRO` | FM Flute, Theremin, VOSIM |
| 183 | `COORTE v2 · VOZ` | Scat, Choir, VOSIM |

Os controles também mudam de vocabulário. Sopro expõe Corpo/Brilho/Frase,
Altura/Presença/Deriva e Pulso/Formante/Decay. Voz expõe
Corpo/Vogal/Frase, Corpo/Abertura/Frase e Pulso/Formante/Decay. Todos foram
verificados por A/B de sinal; o teste falhou inicialmente no Brilho da flauta,
revelou uma rota decorativa e levou à conexão real com o índice FM. Deriva do
Theremin e vogal/abertura das vozes também atingem agora a configuração ou o
próximo evento dos DSPs.

Nenhum botão foi acrescentado e a geometria permanece a mesma: a seed troca o
elenco dentro dos três lugares já compreendidos. A validação humana continua
necessária para tipografia, ausência de sobreposição e intensidade perceptiva;
o workflow não abre o app automaticamente.

Validação técnica: build completo aprovado e 8/8 CTests não-stress passaram.
O teste de coortes renderiza 181/182/183 por dois segundos, confirma energia,
determinismo e A/B dos nove controles locais; o teste do motor confirma três
stems e ecologias distintas depois do mix/estéreo. `git diff --check` passou.
O stress longo não foi repetido para SOPRO/VOZ e CPU real continua pendente.
Binário verificado em
`build-app/src/app/RasgoSynthPerformanceApp_artefacts/Rasgo Synth Performance`,
recompilado em 2026-08-27 21:40:57 `+02:00`, SHA-256
`1573975bd4fb27fed90be199b0134aebbb9990b28ea91d363bdf3e56bd5c9305`.
As strings `COORTE v2`, `FM FLUTE`, `THEREMIN`, `VOSIM`, `SCAT` e `CHOIR`
foram confirmadas no executável recém-gerado; o app não foi aberto.

## Auditoria de funções — 2026-08-27

A auditoria integral de painel, controlador, motor, ecologia, adapters e
contratos está em
[`AUDITORIA_FUNCIONAL_2026-08-27.md`](AUDITORIA_FUNCIONAL_2026-08-27.md).
Ela reproduziu três divergências concretas de seed/território/mixer, identificou
filas cujo erro não retorna à UI, uma disputa de acesso ao documento, funções de
cena/XY parciais e contratos serializados ainda sem realização musical. A sonda
A/B confirmou todos os controles sonoros como diferentes de zero, mas marcou
Memória, Hold, Recall e sete controles locais abaixo da escala desejável para
palco. Nenhuma correção foi aplicada neste passo; as falhas P0 precedem a próxima
audição humana.

## Correção de coerência do painel — `v0.1.1-dev` — 2026-08-28

O painel não antecipa mais um estado que a fila recusou. Macros/XY, presença,
arraste, cabos, controles locais, MASTER, MIX e Hold conservam o último valor
aceito quando a fila está cheia. NOVO TERRITÓRIO atualiza a mesma seed que o DSP
renderiza, reinicia a mesa e os controles locais, preserva MASTER e pode ser
seguido imediatamente por PLAY sem perder a troca. Abertura e redigitação da
seed 181 agora usam a mesma fábrica determinística.

O host `v0.1.1-dev` compilou com as três ecologias e 8/8 testes não-stress
passaram. O app não foi aberto; fonte, sobreposição e comportamento perceptivo
continuam dependendo do teste do músico.

## Cena honesta e constelação bidirecional — `v0.1.2-dev` — 2026-08-28

`CAPTURAR` agora declara seu perímetro no próprio tooltip: BPM, sete macros,
presenças e três relações pertencem à cena. Controles locais, mixer, MASTER,
GRID, Hold, morph e gestos pertencem à mesa/performance e não acendem `MOD`.
Desfazer e recall operam exatamente sobre o mesmo perímetro; nenhum controle
novo ou nova faixa foi inserido, portanto a redistribuição não cria
sobreposição.

As duas direções do gesto relacional usam uma fonte única. Arrastar um nó
converte distância em relações adjacentes; editar cabo/slider, trocar cena ou
território projeta as três relações de volta na posição dos nós. A projeção é
determinística e mantém todos os círculos dentro do campo. Três intensidades
podem pedir distâncias matematicamente incompatíveis; nesse caso a posição é o
melhor ajuste triangular e o valor/espessura do cabo continua explícito como
autoridade sonora.

Validação: helper puro com round-trip e caso impossível, build completo com
CIRCUITO/SOPRO/VOZ, 8/8 CTests não-stress e UBSan do contrato. Binário
`v0.1.2-dev` recompilado; o workflow não abriu o app. Teste humano pendente:
editar relação → capturar → editar novamente → recall, observando o movimento
dos nós e a preservação da mesa.

## Gestos atingem os corpos alternativos — `v0.1.3-dev` — 2026-08-28

Nenhum controle novo foi acrescentado ao painel. A mudança é de consequência:
em SOPRO e VOZ, Memória, Intervenção, Mutate, Rasgar, Recall e Hold não ficam
mais perceptíveis apenas no retorno/modal/provisório. Eles alteram diretamente
relógio, articulação, duração, pitch, formante e energia dos corpos visíveis.
Assim o performer pode ouvir a subversão no stem que colocou na constelação.

Build e 8/8 testes não-stress passaram, incluindo A/B que cobre os cinco novos
encaminhamentos em 182/SOPRO e 183/VOZ. Falta teste humano de intensidade: o
gesto precisa ser reconhecível imediatamente, não só diferente em PCM.

## Controles locais com escala de palco — `v0.1.4-dev` — 2026-08-28

O painel não mudou de densidade nem ganhou knobs. A diferença está na resposta:
CORTE e ARTIC mudam de modo evidente o Acid; FRAG. e ABERT. tornam a percussão
mais quebrada ou aberta; PRES. torna a linha Theremin realmente emergente;
CORPO e FRASE redesenham registro, duração e energia do Choir. Cada gesto usa
o mesmo slider já presente no pequeno painel do módulo.

O teste técnico comprova contraste de endpoint, mas a aceitação final é do
músico: experimentar baixo/alto no mesmo stem, com os demais estáveis, e
confirmar que cada nome corresponde ao que se ouve.

## Cabeçalho com medidas nomeadas corretamente — `v0.1.5-dev` — 2026-08-28

`CORPOS` informa quantas matérias a ecologia mantém; `STEMS` informa as rotas
publicadas. `ATRASOS` conta callbacks que ultrapassaram o tempo do bloco e não
finge ser contador do driver. O medidor diz `PEAK`, pois mede pico de amostra,
não true peak. Nenhum dado técnico novo foi inventado para preencher a tela.

O teste do músico pode simplesmente observar se esses números acompanham a
sessão sem contradizer o que está ativo. CPU e ATRASOS servem como alerta de
operação, não como certificação de desempenho do sistema inteiro.

## Próximo gesto no próprio campo — `v0.1.6-dev` — 2026-08-28

O PALCO não recebeu tutorial modal nem botão novo. A Constelação passou a
enunciar uma pista curta, dependente do estado: enquanto houver corpo na
reserva, pede para arrastá-lo ao campo; com todos os corpos presentes, convida
a redesenhar relações; ao focar um cabo, explica o arrasto horizontal. O texto
é gráfico, não intercepta mouse ou teclado, e deixa a consequência musical
continuar no centro.

## Consequência legível: cicatriz e reparo — `v0.1.7-dev` — 2026-08-28

A segunda linha da Constelação deixou de repetir instruções quando nenhum cabo
está selecionado. Ela mostra o estado musical que já existe no motor:
`TRAJETÓRIA · SEED EM CURSO`, `CICATRIZ`, `REPARO`, `SUSPENSA`, `PARADA` ou
`SAFE`. O rodapé repete essa leitura curta no lugar da promessa vaga
“performance trace em preparação”; cabo selecionado conserva prioridade e seu
valor continua visível.

O objetivo é fechar o ciclo gesto → consequência: Rasgar deixa uma cicatriz
durável e Recall inicia reparo. Não foi acrescentado score, contador de acertos,
modal, botão ou painel. A validação humana deve verificar se o músico ouve e vê
essa mudança sem precisar conhecer a implementação.

## Cabos com causa, não apenas número — `v0.1.8-dev` — 2026-08-28

Ao selecionar um nó ativo, a segunda linha do campo mostra o `STEM · PICO`
publicado pelo motor. Ao selecionar um cabo, ela informa seu nome, valor,
efeito e condição: matéria filtra/espessa, tempo desloca o relógio e espaço
move o estéreo; `ATUA AGORA` exige os dois extremos ativos, caso contrário o
cabo é explicitamente potencial. O gesto continua sendo o mesmo arrasto
horizontal, sem widget, modal ou comando adicional.

## Corpo em reserva, emergência e atividade — `v0.1.9-dev` — 2026-08-28

O cartão ao redor do campo agora mostra a vida que o motor realmente usa:
`RESERVA`, `EMERGE`, `ATIVO`, `CEDE`, `RASGADO` ou `REPARA`. Não é um estado
gráfico separado: o mesmo valor acompanha o stem pela telemetria. Arrastar um
corpo da reserva o acorda como ativo; devolvê-lo ao cartão o recolhe. A seed
continua tocando com seus corpos já ativos e deixa espaço para a entrada humana.

Build completo e 8/8 CTests não-stress passaram. A verificação humana deve
responder apenas isto: ao abrir a seed, fica claro em poucos segundos qual é o
primeiro gesto possível? Se não, simplificar a mensagem em vez de adicionar
outro controle.

## Coortes DIGITAL e MEMORY visíveis no painel — `v0.1.16-dev` — 2026-08-30

O painel agora usa os descritores da seed também para as duas coortes mais
recentes. A seed `184` apresenta `WALSH`, `HARD SYNC` e `ATARI PUNK`; a seed
`185` apresenta `DEJA VU`, `ATROPELO` e `GALO`. Cada trio mantém os três lugares
da Constelação, com presença, relações, gestos e controles locais próprios.
O cabeçalho informa `COORTE v4` e o nome da ecologia, em vez de continuar
reportando v2 por precedência de macro de compilação.

Validação técnica: o host JUCE foi recompilado e as strings dos novos nomes,
`MEMORY` e `v0.1.16-dev` foram verificadas no executável. Os nove controles da
coorte MEMORY passaram A/B técnico no próprio corpo; a audição humana de
DIGITAL e MEMORY ainda é necessária. O app não foi aberto automaticamente.

Após essa validação, o título da área de módulos passou a exibir sempre a
coorte e o trio corrente (`COORTE · MÓDULO / MÓDULO / MÓDULO`), tanto em PALCO
quanto em MESA; os nomes não dependem mais da leitura dos cartões ou dos nós.

O campo também passou a exibir uma biblioteca compacta, sem criar quatorze
controles concorrentes: Circuito, Sopro, Voz, Digital e Memória aparecem com
seus módulos adaptados. O trio efetivamente sonoro continua sendo escolhido
pela seed (por exemplo, `184` para Digital e `185` para Memória); a biblioteca
é um índice visual de disponibilidade, não uma segunda interface de seleção.

Validação técnica desta atualização: `RasgoSynthPerformanceApp` recompilado
com sucesso. A abertura visual e a audição permanecem como verificação manual
na máquina com áudio disponível.

O campo numérico ao lado de `NOVO TERRITÓRIO` agora é rotulado `SEED /
COORTE`; seu tooltip identifica `184` como Digital e `185` como Memória. A
ação continua disponível somente na MESA, mas pode ser enfileirada durante a
reprodução como transição ao vivo.

O campo numérico seguinte foi rotulado `TEMPO / BPM`: ele controla a
velocidade da sessão, não é outro identificador de módulo ou território.

## Retomada da mesa acumulativa — validação da base — 2026-08-30

A suíte funcional foi executada novamente após a troca de seed durante a
reprodução: 9/9 testes passaram. Os contratos continuam preparados para até 8
atores; a formação padrão permanece em três corpos, enquanto a seed Circuito
oferece o quarto cartão `WALSH` na reserva. O músico pode ativá-lo durante PLAY,
abrir seus três knobs contextuais e regular `ÓRBITA ACID → WALSH` na MESA. O
teto operacional medido é de quatro corpos; o quinto slot continua reservado.

## Atualização operacional — layout, coortes e encerramento seguro — 2026-08-30

O layout passou a ter contratos executáveis em
`include/rasgo/performance/PerformanceLayout.hpp`. O cabeçalho usa faixas
exclusivas para `SEED`, `ALEA`, `TEMPO`, `GRID`, `MASTER`, `REC` e transporte;
a faixa de cenas reserva seus utilitários à direita. Na `ConstellationGraph`, o
quarto cartão não compartilha espaço com `BIBLIOTECA`, indicação de gesto ou
log. O teste cobre larguras de 1280–2560 px e alturas de 180–1080 px.

A MESA mostra o inspetor contextual do módulo selecionado. A seed atualiza
nomes, rótulos e tooltips dos controles conforme a coorte; o quarto slot mantém
seu trio próprio quando ativo. No perfil compacto (`<1600×860`), células,
macros e relações numéricas são ocultadas para preservar campo, mixer, ganho,
MUTE/SOLO e módulo em foco.

O host é aberto por `run_performance.sh`, que verifica volume `rw`, memória
disponível mínima de 4 GiB e sessão gráfica acessível. A verificação
`check_runtime_artifacts.sh` é não destrutiva e está integrada ao CTest; ela
denuncia temporários inesperados sem tocar em recordings do usuário.

Validação técnica: 12/12 testes funcionais e estabilidade em 41,28 s. A
inspeção visual em monitor real, HiDPI, Windows/macOS e áudio físico continua
pendente e não deve ser inferida desses testes.
