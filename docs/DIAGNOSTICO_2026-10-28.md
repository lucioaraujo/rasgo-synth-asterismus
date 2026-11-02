# Diagnóstico do Rasgo Synth Asterismus — 2026-10-28

**Origem:** o autor não está contente com o que foi feito e apontou cinco
frentes: os módulos que ainda não entraram, os stems para mixer e
combinação, os problemas técnicos e limitações, os códigos mal resolvidos e
as funções pouco claras ou que deixam a desejar. Pediu também que os
conceitos e a base vinda do Studio fossem lidos antes de qualquer mudança.

**Lido antes deste diagnóstico:** `PROGRAMA_PERFORMATIVO_CANONICO.md`,
`GRAMATICA_DO_CAMPO_PERFORMATIVO.md`, `WAYFINDING_CONSTELACAO.md`,
`ESTRATEGIA_MUSICAL.md`, `MODELO_JOGO_SEED_CAMPO.md`,
`AUDITORIA_STUDIO_MODULOS_E_JOGO_2026-09-02.md`, `PONTE_STUDIO_PERFORMANCE.md`
e, no Studio (somente leitura), `Genealogia_do_Instrumento.md` e as fases
121 (orquestra relacional), 122 (ecologia móvel de audibilidade) e 123
(agência relacional).

Este documento descreve o código como ele está (v0.1.16-dev, 31/31 testes);
nenhuma mudança de som foi feita para escrevê-lo. Não é escuta: as
afirmações musicais dependem da escuta do autor.

## 1. Os módulos que ainda não entraram

Catálogo (`PerformanceModuleLibrary.hpp`): 29 entradas.

| Estado | Módulos |
|---|---|
| tocam (14) | Acid, Gendyn, Drums, FM Flute, Theremin, Scat, Choir, VOSIM, Walsh, Hard Sync, Atari Punk, Déjà Vu, Atropelo, Galo |
| candidato (1) | Modal Resonator (tem adapter próprio, `ModalMatterAdapter`) |
| catalogados sem adapter (14) | Core, Texture, Tuba/Breath, Bass Fallback, Harmonaig, Sieve, Bloom, Hocket, Spectral Object, Polymeter, Bluff Signal, Footnote, Rupture, Thermal Static |

Os 14 sem adapter são organismos do Studio (`OrganismStem::…`). Pela regra
de autonomia, entram por **cópia com proveniência** a partir de um commit
conhecido do Studio, adaptados ao campo — nunca por dependência. O DSP de
vários já está copiado em `vendor/studio-dsp/` (por exemplo `ModalResonator`,
`BreathExciter`, `MoogFilter`); falta a camada que os torna corpos do campo
(adapter, papel, controles, stem).

## 2. Stems: por que não há mixer nem combinação por módulo

- **A formação é sorteada, não escolhida.** `PerformanceModuleCohortAdapter`
  escolhe uma de cinco coortes fixas pelo resto da divisão da seed
  (`seed % 5`): Circuito (Acid, Gendyn, Drums), Sopro (FM Flute, Theremin,
  VOSIM), Voz (Scat, Choir, VOSIM), Digital (Walsh, Hard Sync, Atari Punk) e
  Memória (Déjà Vu, Atropelo, Galo). Módulos de coortes diferentes não
  podem tocar juntos (Acid com Theremin, por exemplo). Isso contraria a
  gramática ("a seed escolhe corpos, papéis, relações e estratégia") e
  impede a combinação livre pedida pelo autor.
- **Um stem não é um módulo.** `PerformanceStemAssignment` guarda até oito
  módulos por stem, mas como *alternativas*: "the rest are seed-compatible
  matters … alternatives, never simultaneous voices". O mixer mistura vagas,
  não módulos. Oito lanes internas caem em até quatro stems.
- **Princípio violado:** o programa diz "cada caixa é um módulo, um ator e um
  stem" (princípio 3).
- **Controles por módulo:** três (`Primary`, `Secondary`, `Motion`).

## 3. Problemas técnicos e limitações

- **O teto de quatro corpos e um problema real sem causa conhecida.** Offline
  (sem placa de som), cinco stems rodaram 20 minutos simulados a 24× o tempo
  real, pior bloco 0,99 ms (o orçamento de 512 amostras a 48 kHz é 10,7 ms),
  zero alocações, zero falhas. **Mas com a placa real, em 05/09, uma sessão
  acumulou `ATRASOS 1010` em cerca de 11 minutos** (callbacks que estouraram
  o prazo), sem clipping nem falta de memória; a causa (DSP, dispositivo ou
  agendamento) nunca foi medida, e a execução longa de verificação depois da
  mitigação (48 kHz, gravação em thread separada) ficou pendente
  (TAREFAS.md, "Mitigação do dropout no callback de áudio"). A calibração
  com driver real que decidiria o teto nunca foi feita. Capacidade
  estrutural: 8 atores, 8 stems, 16 relações.
- **Versões de coorte por compilação condicional:** 72 ocorrências de
  `RASGO_PERFORMANCE_WITH_MODULE_COHORT_V1…V4` (36 só no adapter), com
  regras diferentes de sorteio (`seed % 3`, `% 4`, `% 5`) conforme o que foi
  compilado. O instrumento muda de comportamento conforme a flag do build.
- **A ponte com o Studio é parcial:** transporta o `GamePlan` (estratégia,
  regra e recursos), não os módulos concretos, os papéis nem as relações
  iniciais (auditoria de 02/09, prioridade P0 nunca concluída).
- **Pendência de publicação:** o link de notices do snapshot `vendor/studio-dsp`.

## 4. Código mal resolvido

- `src/app/PerformanceApp.cpp`: 4.995 linhas num arquivo só — interface,
  gravação, idiomas, tutorial e controle do motor juntos.
- `PerformanceActorEcology.hpp`: 2.140 linhas; `PerformanceModuleCohortAdapter.hpp`:
  1.124, com a lógica das cinco coortes misturada a `#if` de versão.
- Controle específico de um módulo costurado na interface geral:
  `walshRelationSlider_`/`walshRelationLabel_` (relação só do Walsh).
- Código suspenso mantido no app sem destino: as 16 células
  (`cellButtons_`), os macros (`macroSliders_`), os sliders de relação
  (`relationSliders_`) e o editor de tempo, escondidos por `setVisible(false)`.

## 5. Funções pouco claras ou fracas

A própria auditoria da superfície (`GRAMATICA_DO_CAMPO_PERFORMATIVO.md`)
classificou: o campo XY Autonomia/Intervenção/Memória/Ruptura, os sete
macros, Abertura/Espiral/Rasgo/Balanço + Morph, `GRID` e a matriz de 16
células como **suspender**; o movimento global Órbita/Pulso/Figura-8 como
**substituir**; a colagem global como **reformular**; Hold, Stutter, Tempo
Shift, Rasgar e Reparar como **manter só depois da escuta**. A primeira
prova vertical (13 passos) nunca passou pela escuta do autor. A lista
concreta do que está claro ou não depende dessa escuta.

## 6. O que a lista de tarefas diz

`TAREFAS.md` tem 171 tarefas abertas espalhadas por cerca de 80 seções
(27/08 a 11/10): a lista acumula sem fechar. As abertas confirmam este
diagnóstico, e parte da direção já tinha sido decidida pelo autor:

- **"Nova direção de produto — mesa acumulativa após a seed inicial"**:
  "substituir o modelo de troca de trio por uma mesa acumulativa manipulável
  pelo músico", com inspetor por módulo e a primeira acumulação
  (`Acid + Gendyn + Drums` recebendo um módulo Digital ou Memory) — aberta.
- **"Capacidade audível por hardware"**: dar lanes próprias aos módulos
  complementares, estender o submix a outras coortes, orçamento calibrável
  por hardware, rebaixamento protetor — aberta.
- **"Mitigação do dropout"**: execução longa observando atrasos, CPU, RSS e
  xruns — aberta (ver frente 3).
- **"Próxima sequência — auditoria de módulos candidatos"** e o adapter
  experimental Tuba/Breath (a primeira rota ao submix foi removida por
  regredir a prioridade de gestos) — abertas.
- **"Prioridade bloqueante — módulos performando com a seed"**: seeds
  diferentes mudando a coorte de forma audível; evolução sem manipulação;
  validação no host com escuta — abertas desde 01/09.

## 7. Asterismo, constelação e dança das estrelas — o conceito do nome

Com o nome *Asterismus* (27/10), três ideias passam a orientar as frentes
acima:

- **Constelação:** o céu organizado — o CAMPO com os corpos (estrelas) que a
  seed abre. Já é o gráfico interno do CAMPO (nomenclatura de 05/09) e o
  sistema de orientação (`WAYFINDING_CONSTELACAO.md`).
- **Asterismo:** a figura que quem observa reconhece e nomeia ligando
  estrelas, sem ser uma constelação oficial (as Três Marias, o Cruzeiro).
  Liga estrelas que podem estar em distâncias muito diferentes, e cada
  cultura desenha as suas. No instrumento: **o desenho que o músico (e a
  seed) traçam entre corpos** — combinações livres, inclusive entre corpos de
  famílias diferentes. É exatamente o que o `seed % 5` impede hoje: os
  asterismos só podem ser formados dentro de cinco constelações fixas.
- **Dança das estrelas:** os movimentos. A gramática já define uma
  hierarquia astronômica (rotação, translação/órbita, baricentro, precessão,
  nutação, movimento próprio, movimento radial, órbita galáctica,
  perturbação), cada um com consequência musical distinta. Só Órbita, Pulso
  e Figura-8 existem, como primeira camada; o resto é proposta.

Consequências para o plano: (a) cada estrela precisa ser um corpo audível e
mixável por si — um módulo = um stem — para que um asterismo seja uma figura
de corpos reais; (b) qualquer estrela do catálogo deve poder entrar no céu e
compor figura com qualquer outra; (c) a dança deve ser escolhida por
estrela, por figura e pelo céu inteiro, cada uma com consequência própria.
O nome do instrumento passa a ser o critério: um asterismo é uma escolha de
quem olha.

## Ordem proposta (a decidir com o autor)

1. **Um módulo = um corpo = um stem**, com mixer e combinação por módulo, e
   primeiro a medição do problema real de áudio (os 1.010 atrasos de 05/09:
   causa, CPU do callback, xruns) e a calibração de capacidade, para saber
   quantos corpos simultâneos a máquina sustenta.
2. **A seed escolhe corpos, não coortes:** fim do `seed % 5`; a reserva
   oferece todos os módulos adaptados; qualquer combinação é possível, com
   papéis e relações propostos pela seed.
3. **Os 14 módulos do Studio**, um por vez, por cópia com proveniência e
   escuta de cada um.
4. **Limpeza do código:** dividir o `PerformanceApp.cpp`, tirar as versões
   por compilação condicional e o controle exclusivo do Walsh, decidir o
   destino do código suspenso.
5. **Funções:** sessão de escuta guiada com o autor para decidir, controle
   por controle, o que fica, o que muda e o que sai.

## Passo 1 — prioridade de tempo real (28/10)

- **Medição antes:** a thread de áudio do JUCE/ALSA rodava em `SCHED_OTHER`
  (sem prioridade), disputando a CPU com a interface (~38% de um núcleo,
  mesmo parada). Com o app em STOP por 9 h: `ATRASOS 513`.
- **Correção:** `src/app/RealtimeAudioThread.h`, cópia com proveniência do
  helper do Aquorbium (`19140e2`): no primeiro callback a própria thread pede
  `SCHED_RR` com prioridade min(RLIMIT_RTPRIO, 70), abaixo do PipeWire (FF 83).
  O rodapé de áudio mostra `RT 70` ou "sem tempo real".
- **Verificado:** build Release, 31/31 testes; binário de 28/10 10:32;
  `ps -L` confirma a thread de áudio em RR 70; saída só para `rasgo_mudo`
  (pw-link); rodapé `RT 70`; `ATRASOS 0` nos primeiros 30 s em STOP.
- **Correção de leitura:** a CPU de bloco de ~20% em STOP continua com a
  thread em tempo real, então ela não vinha do agendamento: o motor processa
  DSP mesmo parado (25% de um núcleo). Investigar se o STOP deve parar o
  processamento (candidato a economia).
- **Medição depois (28/10):** parado, `ATRASOS 0` em ~57 min (10:33–11:29;
  antes, mesmo build sem RT: 513 em 9 h, ~57/h). Tocando (PLAY com gravação,
  saída muda, 48 kHz, 512 amostras, 4 corpos), `ATRASOS 0` em 12 min
  (11:33–11:45), CPU de bloco entre 15 e 35%, load 0,9–1,9, swap 12 MB.
  Em 05/09: 1010 em ~11 min.
- **Limite da comparação tocando:** o build de 05/09 era anterior à mitigação
  (48 kHz, gravação fora do callback), então a queda de 1010 para 0 mistura as
  duas correções; o A/B limpo existe só para o estado parado. Para a
  calibração do teto de corpos, medir de novo com mais corpos quando o passo 2
  (um módulo = um stem) existir.
- **Passo 1 concluído** quanto aos atrasos. Ficam como candidatos: o motor
  processando em STOP e a interface com ~40% de um núcleo.

## Passo 2 — um módulo = uma estrela = um stem (28/10)

**Achados no código antes da mudança:**

- Os corpos 5 a 8 do Campo (MODAL, GALO, ATROPELO, DEJA VU) **não tocavam os
  módulos reais**: com o limite de 4, eles não eram vozes; o som que levava
  esses nomes eram lanes de outras famílias (Sopro/Voz, Digital, Memória)
  dobradas como "camada interna" dentro dos stems 1–4, e as etiquetas não
  correspondiam (a lane rotulada Modal era FM Flute ou Scat). Quando havia
  mais de 4 vozes, os corpos 5–8 eram senoides de imitação escritas na
  ecologia.
- O adaptador de coortes renderizava as cinco famílias inteiras a cada
  amostra (inclusive em STOP), e nas seeds Sopro/Voz o código de Sopro rodava
  duas vezes por amostra, adiantando o relógio de frase em dobro.
- `applyInternalStemLayers` misturava o som de um stem dentro de outro, de
  modo que MUTE num corpo não o silenciava de fato.

**Mudança:**

- `include/rasgo/performance/PerformanceSky.hpp`: o céu. As 14 estrelas
  (Acid, Gendyn, Drums, FM Flute, Theremin, Scat, Choir, VOSIM, Walsh, Hard
  Sync, Atari Punk, Deja Vu, Atropelo, Galo), cada uma com DSP próprio, só
  renderizada quando acesa; o que as estrelas compartilham é o tempo do céu
  (três relógios, Rungler e Buchla). Código copiado da família de origem de
  cada uma, com os controles da própria estrela. Diferenças intencionais
  listadas no cabeçalho do arquivo (termos cruzados Acid×Gendyn retirados,
  relógios de frase/memória sem o controle emprestado de uma vaga; o Galo
  chama já na primeira batida e CHAMADA decide uma, duas ou quatro batidas).
- `performanceInitialAsterism(seed)`: o primeiro asterismo da seed para os 8
  corpos (por ora a abertura de antes: trio da família, quarta matéria,
  Modal no 5, Galo/Atropelo/Deja Vu no 6–8, sem repetir estrela).
- Ecologia: cada corpo toca a sua estrela no seu stem; saíram o
  `renderAllModules`, a dobra das lanes 4–7 e a camada interna entre stems;
  o mixer não dobra mais 8 corpos em 4 barramentos.
- `kPerformanceSafeBodyLimit` 4 → 8; o distribuidor de stems usa o mesmo
  asterismo (um módulo por stem, nunca a mesma estrela duas vezes) e perdeu
  o teto auditado de 5.
- Interface: os 8 canais do Campo de Matérias visíveis, com o nome e os
  controles da estrela que toca em cada um; sai o rótulo "lane interna".

**Testes:** novo `test_performance_sky.cpp` (cada estrela soa sozinha; a
saída de uma estrela é idêntica com as outras 13 acesas; render duplo não
avança o DSP; seed reprodutível; controle próprio muda o som; **14 estrelas
acesas: 0,65 ms por bloco de 512 amostras, 6% do orçamento**). Quatro testes
antigos afirmavam a política anterior e foram reescritos: limite de 4 → 8;
distribuição de alternativas → uma estrela distinta por stem; corpo fora do
céu "dobrado" → corpo fora do céu nunca vaza; controles dos corpos 5–8
testados em SOLO (são estrelas por evento — o Atropelo no papel Rasgo só
entra aos 9 s numa cena da seed 196).

**Observado e não mudado (para a escuta):** com os módulos reais, os
portões de frase e pulso da ecologia deixam vários stems em silêncio por
longos trechos (na seed 196, o Gendyn some depois de 0,4 s por 10 s). Era
assim também nos corpos 1–4 antes; com 8 corpos reais fica mais evidente.
É matéria de escuta, não de teste.

**Pendente:** escuta do autor; medição no app com 8 corpos (ATRASOS, CPU);
o Modal como estrela (passo 4); a seed escolhendo estrelas (passo 3).

## Escuta do autor e passo 3 — a seed escolhe estrelas (28/10)

**Relato do autor (primeira escuta do binário de 12:03):** "galo, atropelo e
deja vu não está subindo o volume". Causa: a abertura criava só 5 corpos
(`makeForReleasedStems` parava no Modal); os corpos 6–8 eram reservas fora do
som, e o fader de presença não tinha voz em que agir. Correção: com 8 stems
liberados a abertura usa a cena expandida, e os corpos 6–8 entram **no céu
com presença 0** — apagados até o músico subir o fader. Medido pelo
controlador, sem SOLO: apagados, silêncio; acesos, o Galo soa na hora, o
Atropelo em 0,6 s e o Deja Vu em 4,3 s (estrelas por evento). Teste novo em
`test_performance_session_controller.cpp`.

**Passo 3:** `performanceInitialAsterism(seed)` não usa mais coortes. A seed
embaralha as 14 estrelas; a figura de abertura pede um chão tonal (corpo 1),
uma matéria de atrito — textura ou voz — (corpo 2) e um pulso (corpo 3); os
corpos 4 e 6–8 recebem as estrelas seguintes da ordem da seed; o 5 continua
Modal. A função de cada estrela vem do catálogo (`PerformanceModuleLibrary`).
O controlador admite no corpo 4 a estrela do asterismo (fim da reserva por
coorte); o cabeçalho deixa de mostrar "COORTE v4 · …"; a interface nomeia os
corpos 1–4 pelo asterismo. Testes: 4.000 seeds — papéis de abertura
respeitados, nenhuma estrela repetida, todas as 14 aparecem, Theremin
encontra Atari Punk. Os testes do controlador que fixavam "seed 186 → Hard
Sync" passaram a procurar a seed que acende cada estrela.

**Ainda com o modelo antigo:** o `PerformanceModuleCohortAdapter` continua no
código (descritores, controles iniciais por coorte do `TerritoryFactory`,
`#if` de versão) e sai na limpeza (passo 5).

## Regressão de atrasos com 8 corpos e correção (28/10, noite)

- **Observado no app** (binário 19:12, seed 3570930745, parado): ATRASOS
  subindo ~1 por segundo (6 → 100 em 3 min), CPU de bloco 22–31%, thread de
  áudio 33% de um núcleo, governador `powersave` em 4 núcleos.
- **Medido fora do app:** com 8 corpos, 2,6 ms por bloco de 512 em média e
  pior 4–6 ms (sem blocos acima do prazo offline); proteção de desnormais
  (FTZ/DAZ) não muda nada — hipótese descartada. As 14 estrelas custam só
  0,65 ms: o custo é a ecologia por corpo (~0,3 ms por corpo por bloco),
  que rodava também nos corpos apagados.
- **Correção:** corpo com presença 0 e fade terminado não roda (nem estrela
  nem ecologia) — "a estrela apagada continua no céu sem gastar CPU". Parado:
  2,62 → 1,52 ms por bloco. No app (binário 19:25, saída muda): ATRASOS 0 em
  60 s parado, CPU 19%.
- **Também:** perfil de composição e colagem de seed guardados em cache
  (recalculados só quando mudam); saída idêntica bit a bit à anterior
  (hash FNV de 20 s com 8 corpos acesos), mas o ganho foi desprezível.
- **Falta medir:** 8 corpos acesos tocando no app. Com 2,6 ms em média, a
  margem é ~4× (antes ~6× com 4–5 corpos); se houver atrasos tocando, o
  próximo alvo é o custo da ecologia por corpo (senos por amostra).

## Custo por corpo aceso — instalar módulos inviabiliza o hardware? (28/10)

Pergunta do autor antes do passo 4. Medido nesta máquina (Intel i5-6500,
4 núcleos, governador `powersave`), bloco de 512 a 48 kHz (prazo 10,67 ms),
30 s tocando, seed 3570930745:

| corpos acesos | média | 99% | pior |
|---|---|---|---|
| 0 | 0,28 ms | 0,40 | 0,47 |
| 2 | 0,76 ms | 1,21 | 1,48 |
| 4 | 1,38 ms | 2,02 | 2,27 |
| 6 | 2,03 ms | 2,85 | 3,18 |
| 8 | 2,58 ms | 3,30 | 4,33 |

- O **tamanho do catálogo não custa**: estrela apagada não roda.
- O que custa é cada **corpo aceso: ~0,3 ms por bloco**, quase todo da
  ecologia por corpo (as 14 estrelas juntas custam só 0,65 ms de DSP).
- Offline a margem com 8 acesos é ~2,5× no pior bloco; **no app houve
  28 atrasos** numa sessão do autor (STOP no fim): interface ~40% de CPU e
  frequência variável do processador comem essa margem.
- Módulos do Studio mais pesados (Spectral Object, Harmonaig com 4
  osciladores, Polymeter com 4 vozes) precisam de medição própria antes de
  entrar.

**Consequência — antes de instalar módulos novos:**
1. Baratear a ecologia por corpo (meta ≤ 0,1 ms), com saída idêntica bit a
   bit verificada.
2. Guarda de capacidade medida: a CPU de bloco real decide quantas estrelas
   podem estar acesas ao mesmo tempo; acender além do limite é recusado com
   aviso (o "rebaixamento protetor" que a lista de tarefas já pedia),
   valendo também para máquinas mais fracas.
3. Reduzir a CPU da interface e parar o processamento em STOP.
4. Cada módulo novo entra com medição de custo isolado no teste do céu.

## Decisão do autor: nenhuma estrela sozinha (28/10, noite)

"Não haverão estrelas sozinhas, todas as estrelas se conectam para compor
generativamente." Leitura e consequências:

- **Stem próprio ≠ estrela isolada.** O passo 2 separou os canais (mixer,
  mute, solo) e retirou acoplamentos escondidos que eram *mistura de áudio*
  entre lanes ou termos cruzados fixos de uma coorte (Acid×Gendyn). O teste
  de independência do céu verifica só isso: o áudio de um canal não vaza
  para outro. Não é um princípio musical de isolamento.
- **O que falta:** hoje a conexão entre estrelas é parcial — o tempo comum do
  céu (relógios, Rungler, Buchla) e as relações da ecologia (cabos com
  fenômenos, resposta ao líder, ensemble), limitadas a 16 relações
  declaradas entre pares fixos. Com 8 corpos há 28 pares; com o catálogo
  inteiro, muito mais.
- **Desenho proposto:** um **campo comum do céu** em que cada estrela acesa
  publica, por bloco, o que fez (altura, ataque, energia, brilho) e lê o que
  as outras fizeram, ponderado pela distância e pelos cabos — custo linear
  no número de estrelas, não quadrático. Cada estrela traduz o campo nos
  seus próprios parâmetros (a melodia do Acid pode responder ao ataque do
  Atari Punk; o Choir pode tomar a altura do Theremin; o Deja Vu lembra o
  que qualquer estrela tocou). Os cabos desenhados continuam sendo as
  ligações fortes; o campo é a ligação de todas com todas.
- **Custo:** entra na mesma medição por corpo aceso; meta somada com a
  ecologia barateada.
