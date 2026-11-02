# Tempo maleável no Asterismus (2026-10-29)

Pedido do autor: "desenvolver a ideia de tempo maleável — tempo elástico —
variação de contagens de tempo — cálculo diferencial — Fourier — ou outros
cálculos interessantes"; "swing, groove, gesto"; "já temos coisas codadas em
outros instrumentos que podem nos ajudar"; "podemos inovar com cálculos que
são aplicados para astronomia".

Tudo abaixo vive dentro da **TRAMA**: com o botão desligado o tempo é reto e
o som é idêntico ao anterior (conferido por hash).

## 1. Feito

### Tempo elástico (`PerformanceSkyDirector::stepElasticTime`)

O fator de andamento do céu (0,7–1,35 do andamento-base) é a soma de:

- **uma mola amortecida de segunda ordem** — cálculo diferencial:
  `x'' = −ω²(x − alvo) − 2ζω·x'` (ω = 1,1 rad/s, ζ = 0,62, levemente
  subamortecida). Acelera e freia como um regente, com pequena ultrapassagem.
  **Alvo:** abertura 0,95; desenvolvimento 1,05; contraste e retorno 1,0;
  multiplicado por `1 − 0,07·tensão` (a tensão harmônica puxa um ritardando,
  a cadência o desfaz);
- **rubato em série de Fourier**: 4 harmônicos sobre o período da frase
  (2 compassos), amplitudes da seed caindo com 1/k²; largura 1,8% (4,5% no
  contraste);
- **gesto — o arco de frase**: dentro de cada frase o tempo avança até o meio
  e relaxa nas pontas (o modelo de N. P. Todd, "The dynamics of dynamics: a
  model of musical expression", JASA 91(6), 1992, que o Trioio já usa no seu
  rubato);
- **a equação do tempo** (astronomia): o relógio de sol se adianta e se
  atrasa pela órbita elíptica (1 ciclo por ano) e pela inclinação do eixo
  (2 ciclos): `E(B) ≈ 9,87·sen 2B − 7,53·cos B − 1,5·sen B` minutos. O
  andamento usa a **derivada** `dE/dB` (a velocidade do adiantamento),
  normalizada a ±2,5%, com o "ano" valendo 16 compassos.

Medido (seed 31415, 180 s): fator entre 0,91 e 1,09, passo máximo 0,0016 por
intervalo de controle (sem saltos).

### Swing kepleriano (`PerformanceSky::keplerWarp`, relógio orbital)

Os 4 passos de cada tempo são **ângulos iguais de uma órbita**; o instante de
cada passo sai da **equação de Kepler** (`M = E − e·sen E`, segunda lei: áreas
iguais em tempos iguais). Órbita circular = tempo reto. O tempo começa a π/2
do periélio (começando num ápside, cada metade da órbita dura exatamente meio
período, por simetria, e não há swing); uma seed em quatro começa a 3π/2 —
swing invertido. A primeira metade do tempo vale ≈ `0,5 + 0,63·e`:

| excentricidade | primeira colcheia |
|---|---|
| 0,1 | 56% |
| 0,2 | 63% (perto do ternário) |
| 0,36 | 72% |

O swing por seção: a seed dá a base (50–60%); o desenvolvimento balança 4%
mais; o contraste inverte (reto ↔ ~62%). A excentricidade é escolhida para dar
o swing pedido. Proveniência do conceito de swing como **razão de duração**:
Trioio (`src/core/Groove.h`, commit `c73ede4`), F. Benadon, "Slicing the beat:
jazz eighth-notes as expressive microrhythm", Ethnomusicology 50(1), 2006.

### Groove

Cada seed gera um molde de microtempo dos 16 passos: os tempos (0, 4, 8, 12)
ficam no lugar; as subdivisões se adiantam ou atrasam até 1% do compasso
(~20 ms a 120 bpm); metade da profundidade na abertura. Conceito do
microtempo sistemático por posição medido no Trioio (mesma fonte acima; A.
Danielsen, *Musical Rhythm in the Age of Digital Reproduction*, 2010; M.
Wright e E. Berdahl, ICMC 2006).

### Contagens variáveis e polimetria (`PerformanceSky::setCounting`)

Sobre o pulso comum (elástico, swing orbital, groove), cada estrela de
evento conta o seu próprio ciclo — um **padrão euclidiano `E(k, n, r)`** — e
só age nos ataques dele (`((passo + r)·k) mod n < k`). Ciclos por estrela:
Acid e bateria densos (n = 8, 12, 16; 40–75% de ataques); Gendyn e Scat
ímpares curtos (5, 7, 9); FM Flute e Atropelo ímpares médios (7–13); Choir,
Deja Vu e Galo longos e raros (11–16; 8–20%). Ciclos de comprimentos
diferentes se reencontram no mínimo múltiplo comum (polimetria: 5 e 7 só
coincidem a cada 35 pulsos). **A forma muda as contagens:** abertura,
desenvolvimento e retorno usam a métrica de casa; cada contraste sorteia
novos ciclos (modulação métrica) e o retorno restaura os da casa. Estrelas
de som contínuo (Theremin, VOSIM, Walsh, Hard Sync, Atari Punk) não contam.
Testes: E(3,8) = `x..x..x.` (tresillo); toda rotação de `E(k,n)` tem k
ataques; com a trama desligada, som idêntico (hash). Fontes: G. Toussaint,
"The Euclidean algorithm generates traditional musical rhythms", BRIDGES
2005; conceitos da especificação de ritmos do Batruc (só lida); polimetria
do `PolymeterEnsemble` do Studio.

### Agrupamentos aditivos (`PerformanceSky::kGroupings`)

O compasso de 16 pulsos é dividido em grupos desiguais — 4+4+4+4,
3+3+2+3+3+2, 3+3+3+3+4, 2+3+2+3+3+3, 5+5+6, 3+3+4+3+3, 2+2+3+2+2+3+2,
3+2+3+3+2+3 (máscaras conferidas uma a uma). Quando o ataque de uma estrela
de evento cai no início de um grupo, ela ganha um acento métrico (+18% de
amplitude; +31% no tempo forte), que decai em ~70 ms. A seed escolhe o
agrupamento de casa; cada contraste sorteia outro, sempre diferente; o
retorno restaura o de casa. Assim a contagem euclidiana de cada estrela (o
*quando*) se cruza com a métrica do compasso (o *peso*). Testes: todo
agrupamento começa no tempo forte; o contraste troca; o retorno restaura;
sem contagem, sem agrupamento; trama desligada com som idêntico (hash).
Fontes: C. Brăiloiu, "Le rythme aksak", Revue de Musicologie 33, 1951;
conceito de agrupamentos da especificação de ritmos do Batruc (só lida).

### Coro de fases (`PerformanceSkyDirector::stepChorus`, Kuramoto)

Cada corpo tem um oscilador de fase dentro do pulso (uma fração do passo).
A cada intervalo de controle, as fases seguem o modelo de Kuramoto com
defasagens:

`dθa/dt = ωa − K0·sen(θa − θ0) + K·sen(θb − θa − φ)` (e o simétrico em b).

- **ω** é a frequência própria, da seed (±0,25 rad/s): cada estrela tem o
  seu jeito de estar um pouco à frente ou atrás (até ~5% do passo).
- **K0 = 0,35** prende de leve ao pulso comum, que repousa em θ0 = 0,1 passo
  (assim inclinar-se à frente vira um atraso menor, não quase um passo).
- **K = 2,5 × influência** acopla só os pares em foco na trama, puxando para
  a defasagem que a relação pede: apoio, contágio e reparo em uníssono;
  sombra e atrito a ¼ de passo (o eco, o desencontro); chamada-resposta e
  hocket a ½ passo (a alternância).
- O céu atrasa o ataque de cada estrela que conta por essa fase; se o
  próximo ataque chega antes, o que esperava sai já. O acento do agrupamento
  acompanha o ataque atrasado.

Medido (teste): um par acoplado trava na defasagem pedida com erro ≤ 0,03
passo, apesar das frequências diferentes; um corpo sem par fica a ≤ 0,05
passo do pulso. Como o foco muda a cada compasso, as fases migram
continuamente — sincronia e alternância surgem e se desfazem com a trama.
Fontes: Y. Kuramoto, "Self-entrainment of a population of coupled
non-linear oscillators", International Symposium on Mathematical Problems
in Theoretical Physics, 1975; I. Aihara et al., "Spatio-temporal dynamics
in collective frog choruses examined by mathematical modeling and field
observations", Sci Rep 4:3891, 2014 (via o `ChorusPhase` do Aquorbium,
conceito; nenhum código copiado).

### Deriva — o planeta contra as estrelas fixas (`chooseDrift`, equação de Adler)

Em cada desenvolvimento um corpo aceso vira o "planeta": a sua frequência
própria passa um pouco do que a âncora do coro segura (|ω| = 1,01–1,06 × K0,
sentido da seed). Abaixo desse limiar a fase trava; acima, escorrega — é a
equação de Adler (`dθ/dt = ω − K0·sen θ`), cujo período de escorregamento é
`2π/√(ω² − K0²)`: uma volta inteira em ~50 a ~180 s. O escorregamento não é
uniforme: demora no gargalo (o fantasma do equilíbrio perdido) e atravessa
o resto depressa. O diretor gira a âncora do planeta um quarto de volta para
que a demora caia **em cima do pulso**: a estrela fica alinhada, desliza um
passo inteiro e se realinha um passo adiante — o *phasing* de Steve Reich,
calculado. Imagem astronômica: o dia sideral contra o solar (366,25 :
365,25) — um ciclo a mais a cada tantos.

Medido (teste): 8,04 voltas em 600 s contra 8,25 previstas por Adler (a
diferença é o transitório inicial); 92% do tempo a menos de ¼ de passo do
pulso. Só há deriva no desenvolvimento. O cabeçalho mostra **DERIVA** e o
nome do módulo, junto do agrupamento do compasso (por exemplo
`3+3+2+3+3+2`). Fonte: R. Adler, "A study of locking phenomena in
oscillators", Proc. IRE 34(6), 1946; S. Reich, *Piano Phase* (1967) como
referência musical.

### Fourier do ritmo (`PerformanceSkyDirector::dominantPeriod`)

O diretor guarda, para cada corpo, os ataques das últimas 48 semicolcheias
(3 compassos; 48 é o mínimo múltiplo comum dos períodos 2, 3, 4, 6, 8, 12 e
16, para que a volta do anel não quebre a fase de nenhum). A cada compasso
calcula a transformada discreta desse trem de ataques, o espectro de
potência |X(k)|² e, pela transformada inversa, a autocorrelação circular
(teorema de Wiener–Khinchin). A periodicidade de P é R(P)/R(0): 1 quando o
padrão se repete exatamente a cada P. O **período dominante** é o menor
período musical com periodicidade a 90% da melhor (os múltiplos também se
repetem); a **clareza** é essa periodicidade.

Uso na trama, quando o ritmo de quem chama é claro (clareza ≥ 0,5):

- **hocket** entra a meio período dominante de quem chama (antes: meio vão
  entre os dois últimos ataques — instável);
- **atrito** entra a 1,5 período — a hemíola, três contra dois, um atrito
  métrico além do meio-tom.

Medido (teste): semínimas → 4; tresillo 3+3+2 → 8 (clareza 1); colcheias
pontuadas → 3; um padrão espalhado tem clareza 0,43. Primeira tentativa,
registrada: medir só |X| no período (concentração numa fase) confundia o
tresillo com período 4 — a periodicidade pede a autocorrelação. Fontes:
N. Wiener, "Generalized harmonic analysis", Acta Math. 55, 1930; A.
Khinchin, 1934; o uso da autocorrelação para achar o período de ritmos
segue J. C. Brown, "Determination of the meter of musical scores by
autocorrelation", JASA 94(4), 1993.

### Peneiras no contraste (`PerformanceSky::sieveOnset`, Xenakis)

No contraste, cerca de metade das estrelas que contam troca o padrão
euclidiano por uma **peneira**: a união de duas classes de resíduos,
(m1, 0) ∪ (m2, r2) — ataca no passo n se n mod m1 = 0 ou n mod m2 = r2. O
par de módulos é o do catálogo (2·3, 2·5, 3·4, 3·5, 4·5, 3·7, 4·7, 5·7, 5·8,
7·8, 7·9, 9·11, 11·13, 13·16) cuja densidade `1/m1 + 1/m2 − 1/mmc` fica mais
perto da densidade da estrela; a seed escolhe a ordem e o resíduo. O ciclo
dura o mínimo múltiplo comum (3 e 5: 15 passos; 5 e 7: 35) e é irregular
por dentro — o euclidiano é o máximo de regularidade, a peneira é uma
regularidade de outra ordem. A métrica de casa (abertura, desenvolvimento,
retorno) nunca usa peneira. Teste: (3,0) ∪ (5,1) = `xx.x..x..x.xx..`; a casa
não peneira; algum contraste peneira; a estrela que peneira toca. Fonte: I.
Xenakis, "Sieves", Perspectives of New Music 28(1), 1990 (e *Formalized
Music*, 1992); conceito também na especificação de ritmos do Batruc (só
lida).

### Eclipses e o Saros (`PerformanceSkyDirector::watchTheMoon`)

Duas luas medidas em tempos: o mês sinódico (de lua nova a lua nova,
29,530589 tempos) e o dracônico (de nodo a nodo, 27,212221) — os mesmos
números dos meses reais, em dias. A seed põe as duas fases. Em cada lua nova
há eclipse se a lua está perto de um nodo: fase dracônica a menos de 0,051
do ciclo de 0 ou de ½ (os ±18,5° do limite dos eclipses solares). Como os
dois meses diferem, os eclipses vêm em **estações**, a cada ~173 tempos
(~1,5 min a 120 bpm), às vezes dois seguidos; e como 223 meses sinódicos
quase igualam 242 dracônicos (o **Saros**, 18 anos e 11 dias), a sequência
inteira volta.

Na música, uma ocultação: um corpo aceso — o "sol" — é encoberto por dois
tempos (o brilho cai a 15%; só com a VARIAÇÃO ligada) e outro — a "lua" —
responde com o motivo do sol, refletindo a sua luz. O cabeçalho mostra
**ECLIPSE** e o módulo encoberto.

Medido (teste): 45–46 eclipses por Saros (o céu real dá ~41–43 solares por
série); 98% deles voltam um Saros depois; num céu vivo, dois eclipses em
menos de cinco minutos, com o corpo encoberto escurecendo a 0,46 (a
transição de brilho leva cerca de dois tempos). Fontes: F. Espenak e J.
Meeus, *Five Millennium Canon of Solar Eclipses*, NASA TP-2006-214141
(limites e Saros); J. Meeus, *Astronomical Algorithms*, 2ª ed., 1998 (os
meses).

### Recapitulação (`rememberOpening` / `recapitulate`)

Como no ciclo metônico — 19 anos trazem a mesma fase da Lua na mesma data —,
cada retorno traz de volta o motivo da abertura, **literal** (sem
transformação), pela mesma estrela (ou pela primeira acesa, se ela se
apagou). No fim da abertura o céu guarda até 6 notas do motivo da estrela
mais ativa. Teste: em 150 s (seed 181), duas recapitulações.

### Sequências de durações (`PerformanceSky::durationFactor`)

Nas vozes em que o terceiro controle é a duração da nota — conferido no
código de cada uma: portão da flauta, sílaba do scat e do atropelo
(`durationSeconds`), frase do coro (`baseDurationSeconds`), galo
(`totalDurationSeconds`), deja vu (`durationSeconds`) —, cada ataque de uma
estrela que conta multiplica essa duração pelo próximo termo de uma
sequência de proporções normalizada pela média: [1,2,3,5,8] (Fibonacci),
[3,1,2], [2,1,1,2,3], [1,1,2], [5,3,2,1,1], [2,3]. A seed escolhe a
sequência de cada estrela; o contraste sorteia outras. Bateria e Acid
guardam o seu terceiro controle. Teste: o fator do scat vai de 0,42 a 2,08;
sem contagens, 1. Conceito das sequências de durações do Batruc (só lido).

## 2. Próximo (não feito)

- A Constelação já desenha a fase de cada estrela (ponteiro), o planeta que
  deriva (anel) e o eclipse (disco escuro) — feito em 29/10.
- Escuta do autor de todas as camadas acima, uma de cada vez se preciso
  (contagens, agrupamentos, coro, deriva, Fourier, peneiras, eclipses,
  recapitulação, durações) — a trama desligada é a referência A/B.
