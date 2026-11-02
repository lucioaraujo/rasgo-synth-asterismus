# Módulos restantes, gestão e repositório próprio (2026-10-30)

Decisões do autor: "repositório próprio, inserção dos módulos restantes e
como gerenciá-los". Levantamento feito no código (o do Studio só lido).

## 1. O que existe hoje

A biblioteca (`PerformanceModuleLibrary.hpp`) tem 29 módulos: **14 estrelas
ativas** no céu, o **Modal Resonator** (soa como corpo 5 por adaptador
próprio, mas não é estrela) e **14 organismos do Studio** catalogados sem voz
no Asterismus. O campo tem **8 lugares** (um stem por estrela); a seed escolhe
o asterismo entre as 14 estrelas; a capacidade baixa se a máquina pedir.

## 2. Os 15 que faltam — três grupos

| Grupo | Módulos | Situação no Studio (lido) | Caminho no Asterismus |
|---|---|---|---|
| **A. Vozes prontas ou quase** | Modal Resonator; Tuba / Breath; Footnote; Bluff Signal; Spectral Object | Modal já adaptado; Tuba já copiada (`TubaBreathVoice.hpp`); Footnote é um pluck de 57 linhas (`dsp/FootnotePluck.hpp`); Bluff, 110 linhas (`sequencer/BluffSignal.hpp`); Spectral, 238 linhas (`dsp/SpectralObject.hpp`, usa `std::random` e `sstream` — precisa adaptar para tempo real) | cópia fixada com proveniência + adaptação, uma estrela de cada vez, com teste e escuta |
| **B. Comportamentos dentro do renderizador do Studio** | Harmonaig (pad de acordes), Bloom (melodia fractal), Thermal Static (vida térmica: esmaecer / morte súbita), Core, Texture, Bass Fallback | escritos dentro do `PieceRenderer.hpp` (10 731 linhas), sem voz separável | **reescrever** como estrelas a partir do conceito (regra RASGO: reaproveitar, adaptar, reinventar) |
| **C. Já existem no céu como conceito** | Sieve, Polymeter, Hocket, Rupture | `RhythmSieve`, `PolymeterEnsemble`; o hocket e a ruptura são lógica | viram **estrelas de controle**: não soam sozinhas, mudam como as vizinhas tocam (Sieve = peneira de Xenakis nas vizinhas; Polymeter = contagens; Hocket = alternância; Rupture = a fratura) |

## 3. Inserir sem mudar as seeds existentes

Hoje a seed escolhe o asterismo entre as 14 estrelas. Se as novas entrarem
nessa lista, **toda seed muda de asterismo** (a sua peça de ontem viraria
outra). Proposta: as estrelas novas entram como **estrelas da biblioteca** —
o performer as insere (arrastar da BIBLIOTECA para o campo, que já existe
para uma estrela); a escolha da seed só passa a considerá-las numa versão
nova, por decisão do autor.

Ordem proposta (uma por vez, com teste, medição de custo e escuta):

1. **Tuba** (voz pronta, local) e **Modal Resonator como estrela** (feitos);
2. **Footnote** e **Bluff Signal** (feitos em 30/10);
3. **Spectral Object** (adaptação para tempo real — feito em 30/10);
4. as estrelas de controle (grupo C — feitas em 2/11);
5. as reescritas do grupo B, começando por **Harmonaig** e **Bloom** (feitos
   em 2/11; Thermal Static, Core, Texture e Bass Fallback também, no mesmo dia).

## 3.1 Feito em 30/10

- **Tuba** toca no céu como estrela da biblioteca (15 tocáveis; a seed segue
  escolhendo entre as 14 — nenhum asterismo muda, hash idêntico). O sopro
  entra em escala linear (a voz, como copiada, saturava: RMS 0,81, quase onda
  quadrada; agora pico 0,70, fator de crista ~3).
- **Trocar a estrela de um corpo:** com uma estrela em foco na Constelação,
  clicar na faixa da BIBLIOTECA abre o menu das estrelas tocáveis por função
  (Tom, Textura, Voz, Pulso), com as da biblioteca marcadas, e "voltar à
  estrela da seed". A troca vale na hora, segue o ator quando as vozes são
  refeitas, e os rótulos, controles e relações mostram a estrela nova. Teste
  `testPerformerSwapsAStar`.
- **Footnote** toca como estrela da biblioteca (16 tocáveis): o
  `FootnotePluck` do Studio copiado byte a byte para `vendor/studio-dsp`
  (commit `cfe1698`, SHA-256 no `PROVENANCE.md`). Duas parciais senoidais em
  razão inarmônica (1,18–1,94, pulando a quinta), raro (Euclides esparso).
  Controles ALTURA, INARMONIA, DECAIM.
- **Bluff Signal** toca como estrela da biblioteca (17 tocáveis): a lógica do
  `BluffSignal` do Studio adaptada em `PerformanceBluffSignal.hpp` (sem
  `<random>`; janela em batidas; **reorganização** oito sinais depois de uma
  traição, porque a performance continua). O sinal soa sempre igual; só o
  genuíno é confirmado, uma quinta acima, com o ganho da confiança; depois de
  uma traição a resposta se cala. Controles VERDADE, JANELA, ALTURA. Teste
  `test_performance_bluff_signal.cpp`.
- **Spectral Object** toca como estrela da biblioteca (18 tocáveis), adaptado
  para tempo real em `PerformanceSpectralObject.hpp` (sem `<random>`,
  `<sstream>`, `std::string`, `std::vector`). Uma proporção, três leituras: o
  parcial p soa em f0·p·√(1 + B·p²) (Fletcher e Rossing) e pulsa p vezes por
  período; a peneira de Xenakis abre buracos no espectro a cada oito
  períodos. Conjunto de parciais pela seed (série, ímpar, primos).
  Controles ALTURA, INARMONIA, PERÍODO (1, 2 ou 4 batidas). Função: Textura
  (o Bluff entra em Pulso; o Footnote, em Tom).
- **Correção:** as máscaras por estrela (renderizada, atacou) eram de 16 bits;
  a 17ª estrela caía fora (sem ataque e com o DSP avançando duas vezes na
  mesma amostra). Agora 32 bits, com `static_assert`, e um teste que toca
  cada estrela tocável 20 s (soa, finita, uma renderização por amostra).
- **Modal como estrela trocável** (2/11): o dono do Modal passa a ser o corpo
  cuja estrela é o Modal (antes, o ator 5 fixo — e o som do Modal
  sobrescrevia o stem do corpo 5 qualquer que fosse a estrela, deixando muda
  a estrela trocada para lá). O menu da troca oferece o MODAL em TOM; um
  ressonador, um corpo (se outro corpo já o toca, o item diz qual). A faixa
  da BIBLIOTECA agora diz o que o clique faz. Teste: Tuba no corpo 5 e Modal
  no corpo 1, os dois soam.
- **Estrelas de controle** (2/11, grupo C): Sieve, Polymeter, Hocket e
  Rupture entram no céu (22 tocáveis) numa função nova, **CONTROLE**: não
  soam (o corpo fica com o stem mudo) e, acesas, filtram os **ataques** das
  outras estrelas — todo ataque passa por `PerformanceSky::controlGate`. As
  de som contínuo seguem soando, menos na Ruptura.
  - **SIEVE** — peneira de Xenakis (A, 0) ∪ (B, R) sobre a contagem de
    ataques de cada estrela. MÓDULO A 2–7, MÓDULO B 3–11, RESÍDUO.
  - **POLYMETER** — cada estrela num Euclides próprio de k ataques em n
    passos, n diferente por estrela. CICLO 3–9, PULSOS, ESPALHA.
  - **HOCKET** — um ataque por instante, e quem atacou há pouco espera a vez
    (GRUPO 2–4); PACIÊNCIA = quantas vezes cede antes de atacar mesmo assim;
    ALCANCE = a parte dos ataques sob a regra.
  - **RUPTURE** — de PERÍODO em PERÍODO (4–16 batidas), com CHANCE, o céu se
    cala por DURAÇÃO (1–4 batidas): corte em ~3 ms, volta em ~20 ms. A estrela
    marca o ataque no corte.
  Teste: Footnote e Atropelo no mesmo relógio — sem controle atacam juntos;
  a Peneira afina sem calar; o Polímetro dá contagens diferentes; o Hoquetus
  nunca deixa os dois atacarem no mesmo instante; a Ruptura corta.
  Escolha minha, para a escuta do autor: o alcance é **o céu todo** (não só os
  vizinhos na Constelação) e o parâmetro vale com uma amostra de atraso.
- **Harmonaig e Bloom** (2/11, grupo B) reescritos a partir do conceito em
  `PerformanceHarmonyStars.hpp` (24 tocáveis). No Studio eram comportamentos
  do `PieceRenderer` (fases 77 e 79, só lido); nada copiado. Conceitos
  citados: o quantizador de acordes harmonàig da Instruō (Jason Lim) e o
  sequenciador Bloom da Qu-Bit Electronix (Andrew Ikenberry).
  - **HARMONAIG** (Tom): pad de quatro vozes raiz/3ª/5ª/7ª empilhadas em
    terças no modo do céu (dórico; a qualidade de cada acorde vem do modo),
    que anda em quartas a cada 1, 2 ou 4 batidas, com portamento de ~90 ms.
    Controles INVERSÃO, ABERTURA, MOVIMENTO.
  - **BLOOM** (Tom): tronco de 5 graus (−4 a +4) e galho de 25 (o tronco
    aplicado a si mesmo); ao fim de cada volta, CAMINHO é a chance de trocar
    de nível. Serra com polyBLEP e passa-baixas suave. Controles CAMINHO,
    REGISTRO, DURAÇÃO.
  Teste próprio: `test_performance_harmony_stars.cpp`.
- **Thermal Static, Core, Texture, Bass Fallback** (2/11, os últimos do
  grupo B) reescritos a partir do conceito em
  `PerformanceStudioConceptStars.hpp` (28 tocáveis; **nenhum organismo do
  Studio ficou só catalogado**). No Studio eram comportamentos do
  `PieceRenderer` (só lido; nada copiado).
  - **THERMAL** (Textura): o fim térmico do Studio — "sufocando e perdendo
    harmônicos, como uma máquina ficando sem energia", às vezes uma "morte
    abrupta" — vira uma vida que se repete: um zumbido com estática esquenta
    (CALOR), morre devagar (~3 s) ou de repente (~0,25 s, com a chance de
    MORTE), esfria 1,5 s e volta. ESTÁTICA: os estalos.
  - **CORE** (Tom): a voz melódica principal do Studio (o par em FM cruzado);
    aqui FM cruzado com realimentação, na frase do céu. FM, RAZÃO (razões
    harmônicas em meios), DECAIM.
  - **TEXTURE** (Textura): o "clima de ruído" do Studio; ruído branco → rosa
    (Kellet) → marrom por uma banda ressonante (filtro de variáveis de
    estado) que anda com a densidade. COR, FOCO, MOVIMENTO.
  - **BASS** (Tom): o grave "core-sub" do Studio; seno com saturação suave,
    fraseado na memória do céu, com portamento. REGISTRO (sub, grave,
    médio-grave), ATAQUE, DURAÇÃO.
  Teste próprio: `test_performance_concept_stars.cpp`.
- Os controles das estrelas da biblioteca começavam em zero fora de um corpo
  (a tabela de padrões tinha 14 entradas); agora seguem o mesmo ciclo.
- Em todas: a seed segue escolhendo entre as 14; nenhum asterismo muda (hash
  `4230363d713a0000`).

## 4. Como gerenciar muitos módulos com 8 lugares

**Feito em 2/11:** o menu da troca (estrela em foco → faixa da BIBLIOTECA)
mostra no alto o corpo e a estrela atual, o item **FIXAR ESTA ESTRELA** e as
cinco famílias em **submenus** com a contagem de cada uma (TOM 14 com o Modal, TEXTURA 5,
VOZ 3, PULSO 3, CONTROLE 4 — a família da estrela atual vem marcada); a volta
à estrela da seed no fim. **Fixar**: a VARIAÇÃO e os eclipses não apagam a
estrela (o diretor força o brilho 1 no corpo); segue o ator quando as vozes
são refeitas; um losango âmbar acima da estrela na Constelação. Teste no
diretor: fixada, nunca abaixo de 0,99 em 4 min, enquanto as outras apagam.
Falta: arrastar com 8 lugares cheios trocando a que está embaixo; a troca e
a fixação entrarem no documento de sessão (salvar sessão).

Plano original:

- **BIBLIOTECA por família** (Tom, Textura, Voz, Pulso, Recurso) com o estado
  de cada módulo (ativo, candidato, em auditoria), em vez da linha de texto
  de hoje;
- **inserir**: arrastar para o campo; se os 8 lugares estiverem ocupados,
  soltar sobre uma estrela **troca** uma pela outra (a que sai vai para a
  reserva, com a sua posição guardada);
- **fixar**: uma estrela fixada não é apagada pela VARIAÇÃO nem trocada pelo
  contraste;
- **a seed sugere, o performer decide**: cada seed propõe o seu asterismo;
  as trocas do performer ficam na sessão (salvar sessão);
- **capacidade**: a guarda já reduz as estrelas acesas quando a máquina pede;
  cada estrela nova entra com o custo medido no teste.

## 5. Repositório próprio — como fazer (padrão Trioio / Maravi)

1. Extrair o histórico da pasta (`git subtree split`) a partir do nome atual;
   o histórico anterior a 27/10 (pasta `RASGO_SYNTH_PERFORMANCE`) fica
   referenciado no `README` com o commit do monorepo.
2. Criar o repositório público no GitHub (`lucioaraujo/rasgo-synth-asterismus`)
   — **ação pública: só com o "pode criar" do autor**.
3. O monorepo passa a ignorar a pasta (como faz com o Trioio); o Asterismus
   vira um repositório aninhado, com a sua CI.
4. CI multiplataforma (Linux, Windows, macOS) como **portão de lançamento**,
   não a cada push: só em tags de versão e por disparo manual (custo:
   ~160 min cobrados por execução completa; regra de economia de push e CI).
5. Primeira publicação: **v0.2.0 "prévia"** depois da escuta do autor e da CI
   verde; o autor testa em Windows e macOS reais depois.

### 5.1 Feito em 2/11 ("pode criar" do autor)

Repositório público `lucioaraujo/rasgo-synth-asterismus`, criado a partir de
um commit novo (o padrão de Trioio, Aquorbium e Maravi, em vez do
`subtree split`: o histórico fica no monorepo, até `e369945`, e as mensagens
dele — que citam outros projetos — não vão a público). A pasta é um
repositório aninhado, ignorada pelo monorepo, que guarda só os diários
`CONTINUIDADE.md` e `TAREFAS.md`. CI em `.github/workflows/package.yml`, só em
tags e pelo botão; ainda não rodou.

## 6. Pendências de decisão

- a primeira execução da CI (custo) e a versão da prévia;
- ícone (proposta em `packaging/icon/proposta/`);
- se as estrelas novas entram na escolha da seed já na v0.2.0 ou depois.
