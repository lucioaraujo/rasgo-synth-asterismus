# Arquitetura do Rasgo Synth Performance

## Estado operacional vigente — 2026-10-06

O host apresenta até oito módulos internos na Constelação e conserva o teto
de quatro stems de produção. As lanes 0/4, 1/5, 2/6 e 3/7 compartilham,
respectivamente, os buses 0, 1, 2 e 3. `PerformanceStemBusMixer` faz essa
dobra com armazenamento fixo, antes do controle de mix dos stems.
Os módulos 5–8 têm controles locais e variação próprios, não buses adicionais.
Ensaios de cinco/oito stems são alvos offline de calibração separados do host.
Tuba/Breath permanece experimental, sem promoção para rota ativa.

As seções datadas e o modelo acumulativo abaixo registram etapas anteriores;
não substituem este limite operacional. Validação técnica não equivale a
aprovação musical ou estabilidade no dispositivo físico.

## Direção evolutiva: mesa acumulativa

O modelo padrão mantém três corpos e três stems por território. A extensão
acumulativa opt-in adiciona uma matéria complementar como quarto stem em cada
ecologia adaptada: Walsh/Hard Sync no Circuito, Atari Punk no Sopro, Walsh na
Voz, VOSIM no Digital e Hard Sync na Memory. A direção de
produto registrada em 2026-08-30 é evoluir para uma mesa acumulativa: a seed
escolhe o conjunto inicial de corpos; depois, o músico acrescenta, recolhe e
relaciona módulos na mesma mesa, sem usar novas seeds como mecanismo de
composição. A capacidade futura deve tomar como referência os quase 20 stems
simultâneos já possíveis no Studio, mas somente módulos com adapter, contrato e
teste próprios poderão atravessar a fronteira. A expansão deverá preservar as
relações existentes e introduzir novos stems de forma bounded e realtime-safe.
Capacidade efetiva, remoção, mixagem e transição sonora já foram validadas para
quatro corpos; `kPerformanceSafeBodyLimit` fixa esse teto nesta máquina. O
quinto slot permanece reservado para futura auditoria.

Auditoria de referência: o Studio declara 23 categorias em `OrganismStem`
(`Count` não é uma categoria). Esse número descreve a estrutura de catalogação
e auditoria; não equivale automaticamente a 23 geradores pesados simultâneos.
Nesta máquina, a avaliação inicial encontra Intel i5-6500 com 4 núcleos/threads
e 23 GiB de RAM. O limite operacional deverá ser obtido por benchmark gradual,
observando CPU, xruns, memória e tamanho de buffer, com proteção contra
sobrecarga antes de liberar a mesa acumulativa.

### Mesa modular e controles contextuais

Cada corpo inserido na mesa deverá expor, quando selecionado, uma superfície
contextual própria: sliders, botões, knobs, gráficos e leituras adequadas ao
módulo. A mesa também deverá permitir aproximar e afastar corpos; atração,
repulsão e distância poderão interferir em relações, presença, ganho, timbre
ou trajetória, desde que a consequência seja documentada e audível. O desenho é
de um sintetizador modular performável, não de presets independentes.

Um stem interno por corpo pode ser útil para controle, medição e segurança, mas
não define necessariamente a experiência sonora final. A proximidade deverá
poder criar simbiose — mistura, modulação cruzada ou compartilhamento de
energia — enquanto a distância preserva o comportamento próprio de cada corpo.
Essa interação deve ser modelada por uma matriz espacial bounded, sem presumir
que a soma de stems isolados seja o resultado musical.

### Camada lúdica da mesa

As ações de jogo pertencem ao mesmo sistema generativo: reordenamento,
confusão, movimento próprio, blefe, disputa ou colapso podem reorganizar corpos,
alterar distâncias e perturbar relações. Não devem ser um placar ou uma camada
decorativa; cada ação precisa ter consequência espacial, temporal ou sonora
observável e permanecer segura no callback.

Essa camada deverá partir dos conceitos de jogo já trabalhados no Rasgo Synth
Studio — incluindo os referenciais conceituais mobilizados nas pesquisas de
Caillois e afins — como fonte de projeto. O Performance adapta esses conceitos
para gestos, relações, movimento e som; não cria um sistema autônomo de pontos,
vitórias ou competição desligado da experiência musical. Cada adaptação deverá
registrar o conceito de origem, a tradução para a mesa e a evidência sonora
obtida no teste.

**Estado:** arquitetura implementada em evolução; estado operacional auditado
em 2026-08-27

## Objetivo arquitetural

Transformar a pesquisa sonora e performativa do Rasgo Synth em um instrumento
de palco contínuo sem alterar o Rasgo Synth Studio. O Performance não renderiza
primeiro uma peça inteira para depois reproduzi-la: o som nasce e se transforma
bloco a bloco enquanto seed, memória, gesto humano e organismos negociam o
curso da sessão.

## Definição vigente — instrumento modular em composição

O modo principal abre com uma seed já soando. A seed não gera um rack vazio:
ela seleciona, configura e encaminha uma coorte limitada de módulos reais do
Rasgo, e essa rede produz música continuamente. O performer toca e compõe com
os recursos disponíveis — stems, vozes, clocks, sequenciadores, modulação,
processamento, memória, rotas, acontecimentos e acaso — enquanto a seed mantém
agência e propõe desenvolvimento.

A arquitetura conserva três camadas musicais distintas:

```text
SeedScoreBaseline     o curso proposto pela seed, sempre em movimento
PerformanceOverlay    execução humana: gesto, stem, recurso, rota e história
ObservedRealization   o que efetivamente soou, incluindo falha e consequência
```

O overlay não apaga silenciosamente a baseline. O motor combina os dois,
observa a realização e alimenta uma trajetória; assim a seed pode resistir,
absorver, responder ou carregar cicatriz, e o performer continua responsável
por frase, timing, articulação, contraste, continuidade e forma.

O catálogo atual contém 142 mecanismos documentados, apoiados por 67 headers
DSP, 46 headers de engine, 15 de sequenciamento e 48 testes no core. Eles não
viram 142 widgets: a auditoria separa módulo tocável, recurso interno, política,
relação, observador e infraestrutura. Cada seed escolhe poucos módulos
complementares; o painel expõe apenas seus recursos performáticos de alta
consequência.

A separação musical central está em
[`ESTRATEGIA_MUSICAL.md`](ESTRATEGIA_MUSICAL.md): a seed gera território e
predisposições; o performer nutre atenção, energia, memória, relações, silêncio
e risco; o instrumento escolhe uma realização limitada por capacidades,
audibilidade e consequência.

## Precedentes internos, com fronteiras

- **Antitotem:** referência autoral de anatomia performática, leitura material,
  16 steps, relações, mixer/log e organização single/dual monitor;
- **Navalha 2:** referência operacional de app JUCE completo, cenas/FORM,
  persistência versionada, telemetria, gravação, projeto portátil e separação
  de modos. Navalha 2 permanece fora de RASGO e conserva sua genealogia própria;
- **Rasgo Synth Studio:** fonte da identidade generativa e de candidatos DSP,
  preservado sem alterações para acomodar o Performance.

Esses precedentes orientam decisões. Nenhum app, painel ou engine é copiado
inteiro para o novo projeto.

Studio e Performance poderão se complementar quando ambos estiverem maduros,
por manifestos versionados e opcionais de território e trajetória. A sequência
e as fronteiras estão em
[`PONTE_STUDIO_PERFORMANCE.md`](PONTE_STUDIO_PERFORMANCE.md); não há integração
cruzada autorizada no primeiro motor.

## Camadas

```text
PAINEL / MIDI / OSC futuro
  -> comandos e gestos com timestamp
  -> PerformanceSessionController (thread de controle)
  -> PerformanceSessionEngine
       -> SceneMorpher
       -> GestureInterpreter
       -> Territory / Constellation / Trajectory
       -> Actor / Role / Relation State
       -> RasgoCoreAdapter (fronteira versionada futura)
            -> vozes e DSP reutilizáveis
       -> master criativo do Performance
  -> saída técnica segura
  -> dispositivo / gravação

METER (somente observação)
  <- níveis, CPU, xruns, atores, cena, memória e risco
```

### 1. Aplicação de plataforma

JUCE abre dispositivo, negocia sample rate/block size, recebe MIDI e mantém a
janela. Não contém decisões musicais no callback.

### 2. Estado da sessão

`PerformanceSessionState` descreve valores contínuos e limitados:

- seed e identidade carregada;
- território gerado e constelação ativa;
- andamento/clock e estado de transporte;
- autonomia, intervenção, densidade, tensão, memória e risco;
- foco e presença relativa das famílias `Tone`, `Hit`, `Texture`, `Pattern`,
  `Rupture` e `Relation`;
- papéis contextuais `Chão`, `Corpo`, `Atrito`, `Contágio`, `Rasgo` e
  `Vestígio`, independentes da família material;
- cena atual, alvo de morph e tempo de transição;
- trim de saída e estado de segurança.

O estado não guarda widgets nem objetos JUCE. GUI e áudio enxergam snapshots
imutáveis ou campos atômicos apropriados.

O formato persistente atual é `RSPERF 1`, com versão explícita e limite de 64
KiB. Ele recupera território e cenas, valida limites antes de preencher as
estruturas e permanece fora do callback. Não reutiliza o schema Project v2 do
Navalha, nem estado, arquivos ou runtime do Studio. Ver
[`SESSION_DOCUMENT_MARCO_1.md`](SESSION_DOCUMENT_MARCO_1.md).

O primeiro conjunto independente de território, constelação, papéis, relações,
sessão, gestos, cenas, fila e telemetria está implementado e testado. Ver
[`CONTRATOS_MARCO_1.md`](CONTRATOS_MARCO_1.md). `PerformanceSessionDocument`
foi implementado depois dessa estabilização e cobre a memória de controle, não
a interface.

### 3. Eventos performáticos

`PerformanceGestureEvent` representa ações discretas, não parâmetros soltos:

- `Hold`: preservar temporariamente uma configuração ou memória;
- `Release`: devolver autonomia ao instrumento;
- `Mutate`: variar a cena mantendo sua identidade reconhecível;
- `Rasgar`: abrir uma ruptura limitada, com consequência e resíduo;
- `Interrupt`: interromper uma relação/ator sem zerar o instrumento inteiro;
- `Stutter`: recortar/reiterar temporariamente um stem ou o conjunto, com
  envelope de retorno e sem substituir o corpo;
- `TempoShift`: deslocar temporariamente fase/pulso locais, sem escrever o
  BPM persistido;
- `Recall`: retornar a uma cena armazenada;
- `Silence`: produzir silêncio musical com fade seguro;
- `PanicSafe`: reduzir risco técnico sem matar abruptamente a sessão.

Eventos carregam timestamp de amostra ou offset de bloco e entram por fila fixa.
Não há alocação, lock, I/O ou descoberta no callback.

`PerformanceSessionController` carrega o documento somente com áudio parado e
faz recall por fila SPSC fixa: GUI/MIDI enfileiram e o callback inicia o morph.
Ele deixa o futuro host JUCE como camada de dispositivo/interface, sem transferir
ao Studio qualquer responsabilidade de runtime. Ver
[`CONTROLADOR_SESSAO_MARCO_2.md`](CONTROLADOR_SESSAO_MARCO_2.md).

Os macros `Autonomy`, `Intervention`, `Density`, `Tension`, `Memory`, `Risk` e
`Space` usam uma segunda fila SPSC fixa. O controle só envia valor bounded; o
motor aplica a alteração no início do bloco e a preserva durante morph de cena.
Isso permite campo XY e controles de palco sem escrita concorrente em estado de
áudio.

### 4. Cenas

`PerformanceScene` não é preset de milhares de knobs. É uma constelação musical
compacta: atores, papéis, relações, campos macroscópicos, memória e limites
dentro do território estável da seed. Trocas usam morph bounded; parâmetros
incompatíveis mudam em fronteiras seguras.

O músico pode preparar cenas, mas a seed continua participante: ela define o
território e o instrumento responde aos gestos dentro desse território.

### 5. Motor incremental

O contrato mínimo futuro é:

```cpp
prepare(sampleRate, maxBlockSize, channels)
setSessionSnapshot(snapshot)
pushGesture(event)
processBlock(left, right, numSamples)
readTelemetry()
reset()
```

Esse contrato pertence ao Performance. O adaptador do core não expõe
`PieceRenderer` nem obriga o Studio a mudar seu fluxo offline.

O Marco 2 já contém uma implementação nativa mínima para validar esse ciclo de
vida, gestos agendados, smoothing e telemetria, ainda sem DSP do Studio. Ver
[`MOTOR_MINIMO_MARCO_2.md`](MOTOR_MINIMO_MARCO_2.md).

## Reaproveitamento do núcleo Rasgo Synth

### Pode ser candidato

- osciladores, filtros, ressonadores, corpos de bateria e fontes caóticas que
  já sejam incrementais e realtime-safe;
- `PerformerState`, `NoteMorphology`, `PerformanceActorRegistry`, gestos,
  morfologia material/espacial e memória temporal, depois de auditoria de API;
- `DcBlocker`, true peak/limiter e gravação somente conforme a política da saída
  técnica comum e após testes no segundo consumidor.

### Não atravessa automaticamente

- `PieceRenderer`, Album, escrita de partitura/metadata e render offline;
- componentes JUCE do Studio, layout, idioma, preferências e assets;
- regras de sessão que existam apenas para a narrativa de uma peça fechada;
- caminhos de arquivo, recordings e estado por usuário do Studio.

### Forma de dependência

O objetivo é um pacote CMake versionado do core, com target e headers públicos.
Até isso existir, o Performance usa contratos próprios e doubles de teste. Não
haverá include direto por caminho para o Studio. Se uma matéria do Studio se
tornar necessária antes de um pacote público, ela deverá ser reimplementada ou
duplicada como artefato pertencente ao Performance, com revisão, proveniência,
licença e testes registrados — sem depender do arquivo original em runtime.

### Regra de cópia e reescrita

Reescrever ou copiar uma matéria já comprovada do Studio é permitido quando
evitar uma reinvenção pior e quando a fronteira por pacote ainda não existir.
O resultado deve residir exclusivamente na árvore do Performance e declarar:

- fonte, revisão/hash e licença de origem;
- motivo musical/técnico da adaptação;
- diferenças em relação à origem e responsabilidade de manutenção;
- contrato realtime, limites e testes do Performance.

Isto não cria uma via para editar o Studio, incluí-lo por caminho relativo,
usar seus assets/estado/builds ou manter duas fontes canônicas do mesmo core.
Quando houver segundo consumidor e estabilidade suficiente, a extração de um
pacote público pode ser considerada como decisão posterior e explícita.

## Stems internos do Performance

O primeiro contrato de stems é nativo e realtime-safe. Cada ator ocupa um bus
estável identificado por `actorId`; `Chão`, `Corpo`, `Atrito`, `Contágio`,
`Rasgo` e `Vestígio` são papéis mutáveis da cena, não nomes de canais. A matéria
modal compartilhada ocupa um retorno separado porque é excitada pela ecologia
e não pertence honestamente a um único ator.

`PerformanceStemFrame` carrega até cinco contribuições mono e um retorno modal,
sem alocação. A soma limitada desses campos reconstrói exatamente o ponto que
antes saía diretamente de `PerformanceActorEcology`. O motor publica somente
IDs e picos bounded por mailbox atômica; o host usa esses dados para halos de
energia nos três nós atuais e para o contador `STEMS`.

Este estágio ainda não cria arquivos multitrack. A gravação futura deverá
copiar os buses alinhados para uma fila fixa e escrever em thread própria. Um
eventual pacote de stems do Studio será outra fronteira, versionada e opcional;
o enum e o auditor offline do Studio não foram importados.

### Mesa de Matérias v1

Cada stem de ator possui `MIX` (`0…1`), `MUTE` e `SOLO`. Esses controles são
estado performático independente das cenas: um recall preserva a decisão da
mesa, assim como preserva o MASTER. Todos atravessam uma fila SPSC própria e a
aplicação no motor usa rampas por amostra; mute e solo não param transporte,
relações, clocks ou a sessão.

Há no máximo um solo ativo. Um novo solo substitui o anterior; desfazer o solo
devolve os outros atores com fade. Os picos publicados são pós-mesa, portanto o
halo da constelação confirma o que realmente chega à mixagem, não apenas a
atividade interna de uma voz.

### Primeira coorte real — 2026-08-27

O target opcional `rasgo::performance_cohort_v1`, com hashes fixados no core,
é agora um segundo consumidor real de módulos existentes. Três corpos ocupam
os stems atuais: `AcidBasslineVoice`, `GendynOscillator` e o conjunto
Kick/Snare/HiHat de `DrumVoices`. `PolyrhythmDivider` fornece o relógio de
eventos; `Rungler` e `BuchlaRandomSource` fornecem modulações correlacionadas.
`ModalResonator` entra como camada de memória dentro de uma stem existente no
perfil de quatro stems; somente uma quinta stem explicitamente liberada pode
recebê-lo como corpo independente.

O adaptador é de estado fixo: nenhuma descoberta, I/O, lock ou alocação foi
adicionada ao `processBlock`. A seed configura notas, timbres, fases, padrões,
caminhada estocástica e fontes de modulação. Nesta v1 ela ainda não escolhe
outras combinações de corpos: os mesmos três corpos constituem a fatia audível,
com realizações distintas por seed. A antiga matéria nativa permanece em 16%
da voz como fallback reversível e para preservar os contornos dos gestos já
testados; 84% da identidade antes do ganho/presença vem da coorte real.

### Constelação espacial como controle musical

Os círculos deixaram de confundir coordenada vertical com presença. O tamanho
do nó representa presença, o halo representa pico pós-mixer e a posição 2D
representa somente a distribuição relacional. Aproximar dois módulos aumenta a
relação correspondente; afastá-los a reduz:

- Acid ↔ Gendyn: matéria, filtro, cruzamento tímbrico e fricção;
- Gendyn ↔ Percussões: tempo, velocidade e deslocamento do relógio comum;
- Acid ↔ Percussões: órbita e abertura/movimento estéreo.

Distância não é um VCA genérico. A intensidade atravessa o domínio declarado
por cada cabo. Os faders de presença e MIX, MUTE e SOLO continuam na Mesa de
Matérias como edição precisa e independente da posição.

### Mixer do Studio e Modo Selvagem performativo

O Studio já oferece a lição de stems estáveis, auditoria pós-espaço,
audibilidade e mixagem narrativa limitada. O `OrganismStemAuditor` é uma mesa
de sombra offline e não pode ser importado como mixer realtime. O Performance
reaproveita o princípio, não o objeto: buses próprios por ator, telemetria
pós-mesa e comandos SPSC suavizados.

O Modo Selvagem do Studio usa `std::random_device` e reseed em fronteiras de
compasso. Em `v0.1.10-dev`, o Performance começa por uma adaptação menor e
realtime-safe: o host prepara um `variationToken` somente no lado de controle
quando o performer chama `MUTATE`; o token viaja na `FixedEventQueue` de
gestos, cuja capacidade já é bounded, e o motor o consome ao despachar o evento
na janela de áudio correspondente. Não há `random_device`, lock, alocação ou
I/O no callback.

`wild_` tem decaimento próprio e colore tanto o índice de mutação dos stems
quanto uma inflexão limitada de fase/cicatriz. A decisão não cria outro botão:
MUTATE ganha significado de abertura temporária do percurso da seed. `RECALL`
reduz `wild_` de imediato; `PANIC SAFE` o zera. A mailbox publica `wildMicros`,
portanto a palavra `SELVAGEM` na superfície só aparece enquanto o regime altera
o DSP. Uma futura gramática por fronteiras de compasso continua pendente até o
Performance possuir relógio musical próprio, não um temporizador inventado.

A Constelação não simula a alteração com posições automáticas: ela lê
`wildMicros` e intensifica cabos e halos dos corpos realmente ativos. Assim a
superfície devolve o gesto sem lutar com o arraste/soltura nem sobrepor painéis.

### Campo de movimento: visual e som inseparáveis — 2026-09-02

`PerformanceMotionField` escolhe `Orbit`, `Pulse` ou `FigureEight` pela mailbox
da ecologia. Ele nunca carrega coordenadas: a âncora, os cabos e a cena ficam
fora do comando. No callback, contudo, a escolha é parte da realização do stem
real, não um adorno de interface. `Orbit` produz deriva contínua, `Pulse` uma
acentuação rítmica e `FigureEight` uma alternância cruzada; cada voz suaviza o
alvo de movimento antes de aplicá-lo ao corpo preparado. A prova A/B mede os
três campos e também a diferença Órbita/Pulso nos oito primeiros blocos após a
troca, exigindo coordenadas idênticas.

### Recortes de colagem — `STUTTER` e `TEMPO SHIFT` — 2026-09-02

Os recortes não entram como novos stems, células ou estados de cena. São dois
`PerformanceGestureEvent` explícitos que atravessam a `FixedEventQueue` já
existente e podem ser quantizados pela mesma política de GRID. O motor delega a
ação à ecologia, que conserva estado limitado por amostra: alvo opcional,
intensidade, fase, duração máxima e retorno. Não há alocação, I/O, lock ou
descoberta no callback.

`STUTTER` alterna uma janela curta de matéria em pulsos suavizados. `TEMPO
SHIFT` aplica uma razão local transitória à fase e ao pulso e uma pequena
inflexão de frequência no corpo nativo, sem atualizar `session_.tempoBpm`.
Com alvo válido, somente aquele stem recebe a transformação; `kInvalidActorId`
significa recorte coletivo. A duração é confinada a no máximo 0,8 s. Cena,
capacidade, presença e coordenadas da mesa não são entradas nem saídas desse
caminho.

Há regressão A/B com dois motores equivalentes: um recebe ambos os recortes e
produz saída diferente; as posições de dois atores e o BPM persistido devem
permanecer idênticos. Isso prova o contrato, não a qualidade musical, a ausência
de click perceptível ou a ergonomia final, que dependem de escuta no host físico.

### Colagem como sobreposição de trajetória — 2026-09-02

`setSeedCollage(sourceSeed, amount)` é uma transação de ecologia enfileirada
pelo controlador, distinta de `queueTerritory`. A seed principal continua
definindo território, coorte e mesa; a secundária alimenta somente uma segunda
`PerformanceSeedTrajectory`. A combinação acontece por traços preparados, não
por soma de inteiros de seed. `amount` é limitado a `0…1` e não toca posição,
presença, cena ou BPM.

O host expõe essa transação apenas na MESA, numa faixa reservada acima do campo
XY. A roleta produz a fonte secundária no thread de controle; `MIX` enfileira a
quantidade. Quando há uma troca explícita de território, o controlador aplica
primeiro o documento novo e depois o comando de colagem já escolhido. Portanto,
a ordem no callback preserva a autoria da seed principal e não deixa a UI
anunciar uma mistura que foi apagada pelo reset da ecologia.

### Transporte não é estado de cena — `v0.1.11-dev`

Uma cena contém a composição recuperável, mas não tem autoridade para desligar
uma performance já iniciada. `recallScene` preserva o PLAY/STOP do motor e
`beginSceneMorph` fixa o mesmo estado tanto na origem quanto no alvo antes de
morfá-los. `setTerritoryScene` permanece distinto: ao instalar um território
novo, ele pode estabelecer o estado inicial deliberado. A regressão cobre uma
cena capturada com STOP recuperada sob PLAY e exige que o áudio permaneça ativo.

### Controles locais com uma só gramática — `v0.1.11-dev`

Os controles próprios dos três módulos deixaram de usar barras estreitas
heterogêneas. São nove valores contínuos e agora compartilham knob rotativo,
arco limitado, rótulo central e valor abaixo. No PALCO, cada painel de módulo
ativo os dispõe em três colunas; gestos continuam botões porque são eventos,
não parâmetros. A mudança é somente de superfície e mantém o mesmo comando
bounded de módulo no controlador.

Validação técnica desta integração: 7/7 testes funcionais e a simulação stress
de 20 minutos passaram. Com os módulos reais, o stress offline levou 129,79 s,
contra cerca de 23 s na ecologia provisória documentada anteriormente. Ainda é
mais rápido que tempo real, mas o aumento exige medir CPU no dispositivo antes
de ampliar a coorte; ativação musical e orçamento computacional passam a ser
avaliados juntos.

## Threads e realtime

- callback de áudio: somente DSP bounded e leitura de estado lock-free;
- GUI/MIDI: produz snapshots e eventos em fila fixa;
- gravação: thread/fila própria, nunca escrita direta no callback;
- análise pesada e descoberta: fora do callback;
- telemetria: escalares atômicos ou snapshot de baixa frequência;
- mudanças de cena: preparadas fora do callback e aplicadas por morph.

Comandos pendentes devem ser drenados/sincronizados antes de salvar uma sessão,
seguindo o problema de consistência já resolvido no Navalha 2, mas com formato
próprio do Performance.

### Invariantes operacionais exigidos pela auditoria de 2026-08-27

A arquitetura declarada acima ainda não está integralmente garantida pela
implementação. A auditoria funcional encontrou cinco violações P0; elas passam
a ser gates antes de novas coortes, ilhas ou protocolos de controle:

1. a seed canônica deve ser idêntica no painel, documento, adapter e DSP,
   inclusive para zero;
2. trocar território é uma transação indivisível: instalar documento, coorte,
   cena, mixer e representação visual deve ocorrer em uma ordem única antes de
   aceitar PLAY;
3. cada estado preservado ou reiniciado na troca de território deve seguir uma
   política explícita e comum ao host e ao motor;
4. a interface só pode confirmar o estado final de um gesto depois que a fila
   o aceitar; saturação deve ser observável e recuperável;
5. o callback não pode escrever no mesmo `PerformanceSessionDocument` que a
   GUI lê. Documento de controle e snapshot de áudio precisam de posse ou
   publicação bem definida.

Também ficou demonstrado que contratos ricos não equivalem a função musical:
campos de vida, capacidades, saliência, latência, duração, alvo e clock externo
ainda podem ser validados ou serializados sem governar o DSP. O inventário de
funções deve distinguir `declarado`, `conectado`, `audível` e `performável`.
Medições e matriz completas estão em
[`AUDITORIA_FUNCIONAL_2026-08-27.md`](AUDITORIA_FUNCIONAL_2026-08-27.md).

O incremento `v0.1.1-dev` implementou esses cinco gates: documentos separados
por thread, território antes do transporte, seed canônica, reset explícito da
mesa no limite da sessão e confirmação visual condicionada à aceitação da fila.
Os gates permanecem como invariantes e possuem regressões; a validação manual
do host ainda é obrigatória.

## Saída e segurança

O master criativo pertence ao Performance. Depois dele vem uma saída técnica
previsível: não-finitos, DC/subgrave, pico, trim, mute/fade, medição, recorder e
estado do dispositivo. `PanicSafe` atua nessa fronteira e nos budgets do motor;
não é um botão que encerra o processo ou substitui decisão musical por mute.

## Critério do primeiro protótipo sonoro

Um protótipo só será chamado Performance quando:

1. produzir áudio contínuo por pelo menos 20 minutos sem xrun atribuído ao app;
2. aceitar gesto com latência perceptiva adequada e sem click;
3. trocar entre três cenas mantendo continuidade;
4. executar `Hold`, `Mutate`, `Rasgar`, `Recall` e `PanicSafe` com telemetria;
5. gravar a sessão sem I/O no callback;
6. demonstrar identidade musical distinta do fluxo Studio.

## Reserva, pertencimento e painéis locais — 2026-08-27

Cada corpo da coorte é preparado uma única vez na inicialização. O estado de
pertencimento à constelação não cria nem destrói DSP: altera presença com
rampa, habilita relações cujo par também está presente e mostra/esconde seu
painel local no host. Isso preserva continuidade de fase, memória e contrato
realtime quando o músico arrasta um módulo da reserva para o campo ou o devolve
à reserva.

O caminho de controle é fixo e limitado:

```text
reserva/constelação no host
  -> presença e controle local em filas SPSC
  -> PerformanceSessionController
  -> PerformanceSessionEngine
  -> PerformanceActorEcology
  -> PerformanceModuleCohortAdapter
  -> corpo DSP já preparado
```

`PerformanceModuleControl` declara três controles locais por corpo. Na coorte
v1: Acid recebe `CORTE`, `RESO`, `ARTIC`; Gendyn recebe `PASSO`, `TEMPO`,
`FLUXO`; Percussões recebem `ATIV.`, `FRAG.`, `ABERT.`. Eles não são aliases
dos macros globais: percorrem fila própria e alteram mapeamentos reais dos três
corpos DSP. Os testes cobrem limites inválidos e diferença estrutural no DSP.
A auditoria posterior mostrou que `CORTE`, `ARTIC`, `FRAG.` e `ABERT.` ainda
têm força pequena para uma superfície de palco; conexão técnica não constitui
aceitação musical.

Uma futura malha de até quatro constelações será estado de pertencimento e
relações de capacidade fixa. Ela não poderá instanciar módulos durante áudio,
nem permitir uma rede irrestrita: cada ilha terá orçamento de corpos, cabos e
energia. A UI pode desenhar regiões/ilhas, mas o motor receberá apenas comandos
bounded de entrada, saída ou transferência entre elas.

### Biblioteca performável e coortes por seed

O catálogo tem 142 mecanismos documentados, mas não são todos vozes autônomas:
há 67 headers DSP, além de engine, sequenciamento, relações, observadores e
infraestrutura. A reserva não deve expor um cartão arrastável que ainda não
produz resposta real. A segunda fatia integra `FmFluteVoice`, `ThereminVoice`,
`ScatVoice`, `ChoirEnsemble` e `VosimOscillator` por uma fronteira CMake
hash-pinned. A terceira fatia traz `WalshOscillator`, `HardSyncVoice` e
`AtariPunkConsole` pela fronteira v3; seus estados são preparados antes do
áudio e o adaptador não faz alocação, I/O ou descoberta no callback.

`PerformanceModuleLibrary.hpp` agora registra esta diferença no próprio
Performance: os quatorze corpos integrados são `ActiveCohort`; VOSIM pertence às
ecologias Sopro, Voz e Digital; Walsh é base Circuito e complemento da Voz;
Hard Sync é complemento de Circuito e Memory; Atari Punk é complemento de Sopro
e corpo Digital. O teste da biblioteca impede que um item sem auditoria seja
apresentado como corpo disponível.

A seed agora escolhe uma ecologia contrastante de três corpos sem alterar o
limite visual: `seed % 5 == 1` produz CIRCUITO (Acid/Gendyn/Drums), resto 2
produz SOPRO (FM Flute/Theremin/VOSIM), resto 3 produz VOZ (Scat/Choir/VOSIM),
resto 4 produz DIGITAL (Walsh/Hard Sync/Atari Punk) e resto 0 produz MEMORY
(Deja Vu/Atropelo/Galo). Os corpos das cinco ecologias são preparados antes do áudio;
somente a formação escolhida é mostrada e renderizada. Os testes cobrem energia,
determinismo, os nove controles locais de cada ecologia e a passagem por stems,
mix e saída estéreo do motor completo. Esses testes confirmam caminho e
determinismo, mas sete controles entre as três ecologias foram medidos como
fracos e os corpos principais de SOPRO/VOZ ainda não recebem diretamente
Memória, Intervenção, Mutate ou Rasgar. A expansão da biblioteca fica
condicionada à correção dos P0 e à causalidade perceptiva dos módulos já
integrados.

### Posse de cena e projeção relacional — `v0.1.2-dev`

A cena conserva estado de território reconstruível (BPM e macros) e pode ser
capturada explicitamente. A mesa, porém, pertence ao performer: presenças,
posição dos nós, cabos vivos, controles locais, MIX/MUTE/SOLO, MASTER, GRID,
Hold e morph não são substituídos por recall. Novo território é a ação explícita
que reinicia esse campo e instala a cena inicial.

A posição dos nós é um desenho espacial vivo. Em um trio, a projeção
`PerformanceConstellationGeometry.hpp` ainda oferece uma leitura determinística
das três relações; depois de qualquer arrasto, porém, a posição desenhada é a
autoridade. Com quarto corpo preparado, a mesa declara os seis pares possíveis:
a seed abre somente o primeiro cabo e `2 → 4` / `3 → 4` começam fechados. O
músico os abre ou modula diretamente no campo, formando quadrado, cadeia ou
estrela sem trocar seed, cena ou desenho. O DSP recebe as intensidades originais
dos cabos, sem projetar a geometria de volta sobre a mesa.

### Estados globais atravessam ecologias — `v0.1.3-dev`

O adapter de coorte deixa de tratar SOPRO/VOZ como exceção passiva. Memória,
Intervenção, Mutate, Rasgar e `repair` de Recall entram em
`renderAlternative()` e modulam diretamente o relógio, os eventos e as vozes
FM Flute/Theremin/Scat/Choir/VOSIM. `repair` continua sendo produzido pelo
engine em resposta a Recall; apenas deixou de ficar restrito à camada
provisória quando a coorte alternativa está ativa.

O contrato continua fixed-state e sem I/O, locks ou alocação no callback. Uma
regressão A/B por seed garante que cada estado deixa contraste estrutural nos
corpos reais, mas não substitui a audição humana de escala e ergonomia.

### Escala dos controles locais — `v0.1.4-dev`

Os controles locais continuam bounded em `[0,1]` e atravessam a fila própria;
o ajuste ocorreu somente no adapter fixed-state. A escala agora entra antes do
ganho final: cutoff/envelope do Acid, componentes de snare/hat/rungler dos
Drums, ganho físico do Theremin e registro/frase/energia do Choir. Isso evita
o falso conserto de multiplicar a saída inteira e conserva a identidade de
cada corpo.

O teste de CIRCUITO percorre os nove controles em seus endpoints e exige
contraste de palco no stem correspondente. SOPRO/VOZ mantêm seus testes por
corpo. Não há alocação, descoberta ou I/O no caminho de áudio.

### Telemetria e origem da medida — `v0.1.5-dev`

`activeActorCount` é a quantidade de corpos presentes na ecologia e aparece
como `CORPOS`. `xrunCount` é incrementado pelo host somente quando a duração
medida de um callback excede `numSamples/sampleRate`; por isso o painel o chama
`ATRASOS`, não underrun de driver. Esse incremento segue no mesmo thread de
áudio e não usa fila, lock ou alocação.

`outputPeakMicros` é peak por amostra pós-saída e alimenta o medidor `PEAK`.
`truePeakMicros` permanece proxy compatível até uma futura implementação de
oversampling que seja comprovadamente segura para o callback. Nenhuma decisão
de segurança ou clipping deve tratar o proxy como true peak.

### Orientação sem nova rota de controle — `v0.1.6-dev`

`ConstellationGraph` deriva uma frase de próximo gesto somente de seu estado
visual existente: reserva, nó ativo ou cabo selecionado. Ela não envia comando,
não cria estado no motor e não é serializada. Esse limite preserva a separação:
o painel sugere uma entrada legível, mas não automatiza a composição nem
introduz um segundo protocolo de performance.

### Trajetória observável sem score — `v0.1.7-dev`

O estado interno `scar_`, criado por Rasgar e lentamente dissipado, já modulava
os corpos da ecologia. O estado `repair_`, criado por Recall, também já atuava
na síntese, mas não atravessava a fronteira de telemetria. `repairMicros` passa
a acompanhar `scarMicros` na mailbox atômica bounded; ambos são normalizados em
0…1 e não exigem lock, alocação ou I/O no callback.

O host deriva deles somente uma leitura efêmera de trajetória: seed em curso,
cicatriz, reparo, suspensão, parada ou safe. Essa leitura não é pontuação, não
escolhe o próximo gesto, não entra no documento de sessão e não constitui ainda
o `Rasgo Performance Trace`. A persistência continua pendente de evidência
musical de sessões sob a mesma seed.

### Relação observável por efeito — `v0.1.8-dev`

As três relações já eram aplicadas no motor como pressões de matéria, tempo e
espaço, dependentes de intensidade e da presença dos dois extremos. O campo
agora expõe essa causalidade sem criar telemetria inventada: nó selecionado lê
o pico de seu stem publicado; cabo selecionado diz sua consequência concreta e
se os dois corpos permitem que ela atue. Um cabo que toca a reserva continua
potencial e não é apresentado como alteração audível já em curso.

### Vida do ator como ganho musical — `v0.1.9-dev`

`ActorLifeState` deixou de ser campo apenas serializado. Dormant zera o ganho
do stem; Emerging, Active, Yielding, Ruptured e Recovering aplicam escalas
musicais distintas. O comando de presença do performer acorda um corpo acima
do limiar ou o devolve à reserva; a vida efetiva atravessa o stem frame e a
mailbox de telemetria para o campo. Nenhum DSP é construído, destruído ou
alocado no callback.

### Abertura selvagem delimitada — `v0.1.10-dev`

O primeiro regime de jogo da seed não é uma aleatoriedade global: ele é uma
abertura breve e reversível. O mesmo evento que o performer produz é o que
limita a entrada e carrega sua variação; em ausência de token externo, o motor
deriva uma alternativa determinística a partir da seed e do cursor. Os testes
confirmam que o regime se torna observável, Recall o reduz, dois tokens produzem
trajetórias estéreo distintas sob a mesma seed, e o caminho permanece finito
sob estabilidade prolongada.

### Gesto dirigido por módulo — `v0.1.12-dev`

`targetActorId` finalmente ganhou consumidor musical. Ao selecionar um nó e
chamar `MUTATE`, a interface leva o id daquele ator no próprio evento bounded;
o motor entrega a abertura selvagem à ecologia e o stem escolhido recebe a
maior parcela de mutação, desvio e corpo. Os demais conservam uma parcela de
continuidade. Sem foco, o id é inválido e o gesto permanece coletivo. Nenhuma
consulta de UI, alocação ou lock ocorre no callback.

Em `v0.1.13-dev`, Mutate e Rasgar mantêm alvos independentes: uma abertura e
uma cicatriz podem atravessar tempos diferentes sem trocar de corpo por efeito
colateral. A ecologia reduz somente a parcela de fundo nos vizinhos e conserva
o regime coletivo quando o alvo é inválido.

Em `v0.1.14-dev`, Recall leva um terceiro alvo independente. A ecologia aplica
o reparo inteiro ao corpo em foco e uma parcela de continuidade aos vizinhos;
sem alvo válido, preserva o reparo coletivo. Os três ids percorrem a fila
bounded e são consumidos no callback sem UI, alocação ou lock.

Em `v0.1.15-dev`, a seleção visual passa a ser contrato performável: o nó que
abre selecionado na Constelação define o alvo inicial do motor, os nomes dos
três gestos mostram esse corpo e clicar no vazio limpa o alvo. Não existe foco
residual invisível nem um seletor adicional fora do campo.

O foco dirigido é limitado a atores ativos. Um cartão ainda na reserva informa
que deve ser arrastado ao campo; ele não transforma Abrir/Rasgar/Reparar em um
comando aparentemente mudo. Ao entrar, o corpo passa a ser o foco natural;
ao sair, a superfície escolhe outro corpo ativo ou retorna ao coletivo.

### Limite honesto do catálogo performável — 2026-08-29

`PerformanceModuleLibrary` registra quatorze módulos adaptados: Circuito, Sopro,
Voz, Digital e Memory, em coortes de três corpos por seed. A factory escolhe uma
coorte e a ecologia prepara a formação inicial; `makeWithOptionalModule`
acrescenta um quarto corpo dormente e o ativa pela mesa durante PLAY, escolhendo
uma matéria complementar específica da ecologia. O inventário
maior do Rasgo core não é uma lista de módulos performáveis: cada candidato
precisa de adapter sem alocação, descritor, controles, teste e causalidade
musical antes de poder entrar na reserva da Constelação.

A admissão não depende apenas do ID do módulo: a reserva também declara uma
máscara de coortes e o adapter compara essa máscara com a ecologia calculada a
partir da seed. Assim, um módulo ativo, porém pertencente à ecologia errada,
não pode ocupar o quarto slot por engano.

Os slots acumulativos (`PerformanceCumulativeDesk`) suportam oito entradas
estruturais, com identidade de biblioteca, posição, presença e recolhimento
independentes. Essa capacidade não equivale a oito stems audíveis: o host mantém
o limite seguro medido de quatro corpos e conserva slots realtime fixos durante
PLAY para não reconfigurar o áudio no callback. A próxima expansão deve criar
uma combinação real de nova família no adapter e na factory, incluindo
identidade, controles, relações e benchmark. A primeira combinação adicional
foi Digital + VOSIM: a seed prepara o quarto corpo, que permanece dormente até
ser inserido explicitamente pela mesa. A escuta humana e a medição em hardware
real ainda são necessárias antes de elevar o teto de quatro corpos.

### Direção arquitetural — Studio → Performance — 2026-08-29

O próximo eixo não é multiplicar sintetizadores locais. O Studio já contém a
composição generativa, suas vozes, estratégias, memória, rupturas e stems; o
Performance deve receber uma realização limitada e versionada disso como
território inicial. A Constelação torna os stems corpos manipuláveis e cenas
guardam capítulos de composição, não somente valores de controles. A ponte
precisa preservar a separação de processos: manifesto e dados no controle,
áudio já preparado no callback, nenhuma inclusão do app Studio ou I/O no
caminho realtime.

### Contrato musical prioritário — seed e módulos — 2026-09-01

O Performance não é uma coleção de módulos estáticos com uma seed decorativa.
Sua realização deve seguir o fluxo bounded:

`seed → trajetória de acontecimentos → parâmetros dos módulos → stems → mistura`

A seed define uma história temporal e uma ecologia de módulos. O performer
desenha e manipula o campo; quando deixa de manipular, a trajetória da seed
assume provisoriamente a agência e transforma densidade, tensão, memória,
relações, articulação e mistura. A ação autônoma pode alterar o som, nunca a
geometria desenhada.

O contrato de aceitação exige escuta real: duas seeds devem produzir diferenças
audíveis, os módulos inseridos devem ter stems próprios e controles causais, e
PLAY sem intervenção deve evoluir em vez de repetir um estado fixo. Telemetria
e testes são evidências auxiliares, não substitutos da prova musical no host.

Memórias de posição são uma possibilidade futura e devem permanecer separadas:
guardar/recuperar será sempre explícito e reversível, nunca efeito colateral de
seed, cena ou célula.

### Ergonomia da mesa — inserção e remoção — 2026-09-01

A mesa usa um gesto espacial bidirecional: cartões de módulos são reservas;
arrastar um cartão para o campo ativa o corpo no ponto escolhido, e arrastar o
nó de volta ao cartão o recolhe. A biblioteca oferece o mesmo gesto para o
módulo opcional preparado pela seed, mantendo o clique como atalho de inserção.
O callback de ativação e o de posição são enviados na mesma operação lógica,
evitando que a interface mostre um módulo num ponto enquanto o engine o recebe
em outro.

Esses gestos são locais e reversíveis: não podem alterar seed, cena, célula ou
geometria de outros módulos. A validação final é manual no host, observando
seleção, arraste, retorno à reserva e reintrodução durante PLAY.

### Chave de confinamento orbital — 2026-09-02

O movimento dos corpos na mesa é uma camada temporal sobre o campo autoral.
Cada módulo mantém um centro fixo para posição, hit-test, relações e recall;
um satélite visual percorre uma região limitada em torno desse centro. A fase
é rítmica, seguindo BPM, PLAY e autonomia, com deriva inicial determinada pela
seed.

O seletor clicável `MOVIMENTO`, no alto à direita do campo, e a tecla `O`
alternam três confinamentos: elipse, pulso radial e figura-8. A escolha é
reversível e não grava uma nova posição. Assim o universo gráfico ganha
variação e órbita sem repetir o erro original de substituir o desenho do
performer quando um acontecimento narrativo acontece.

Cada cartão corresponde estritamente a um módulo/ator/stem; não é uma caixa
com vários módulos internos. O círculo numerado continua sendo a âncora de
gesto e de persistência. O corpo laranja e sua trilha são a realização visual
em movimento dessa mesma matéria: têm amplitude suficiente para leitura no
campo, derivam lentamente em repouso e seguem o BPM em PLAY. A camada visual
não é usada para hit-test nem serializada como posição.

### Pausa do performer e turno narrativo da seed — 2026-09-02

O relógio de inatividade do motor é reiniciado por qualquer gesto manual. Após
dois segundos em PLAY, a ecologia executa um turno do jogo espacial informado
pela trajetória da seed: altera pressões musicais e uma relação já declarada,
sem escrever posição, presença, cabo ou cena. `Intervenção` é intensidade/cor
da mão e não deve competir como veto contra a narrativa; por isso o turno
acontece com qualquer `Autonomia > 0.02`. Zero permanece uma escolha explícita
de suprimir a condução autônoma. Uma regressão cobre autonomia 0.24,
intervenção 0.86 e confirma evento válido com a posição desenhada intacta.

O último turno percorre telemetria atômica, sem caminho de controle de volta ao
motor: ordinal, movimento, estratégia e regra chegam ao painel em forma de
texto (`SEED n · MOVIMENTO · ESTRATÉGIA`). A indicação dá legibilidade à
história que está agindo, mas não oferece recall, comando ou alteração de
geometria. A regressão também confirma que, sem nova mão do performer, o turno
seguinte da trajetória chega ao fim do próximo intervalo ocioso.

A própria trajetória carrega o tempo entre acontecimentos. O evento inicial
usa quatro batidas para responder prontamente ao silêncio do performer; os
eventos posteriores alternam deterministicamente entre quatro, seis e oito
batidas conforme a seed. O engine converte a duração pelo BPM vigente e mantém
um piso de segurança de 0,75 s. Logo, a história tem ritmo próprio sem timer
aleatório, alocação ou modificação da mesa.
