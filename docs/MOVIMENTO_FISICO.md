# Movimento físico na Constelação (2026-10-29)

Pedido do autor, durante a sessão: "faltam movimentos de conceitos de física
nas constelações: elásticos, billiard mathematics, gravidades, atração,
repulsão, magnetismos; e também movimentos de dança"; "pêndulos, etc"; "os
movimentos estão ainda duros demais ou ainda muito previsíveis"; "podemos
fazer melhor"; "movimentos mais interessantes, inusitados, generativos";
"efeitos de spins (tênis de mesa), curvas, balanços, molas, espumas,
distorções"; "veja alguns conceitos no batruc".

## 1. Diagnóstico

A Constelação desenhava cada estrela por fórmulas paramétricas — seno e
cosseno de uma fase comum (ÓRBITA, PULSO, FIGURA-8 e os fenômenos de
população). Por isso o movimento era rígido e previsível. O diretor do céu
já calculava gravidade entre as estrelas, mas essas posições só serviam ao
cálculo (o "tempo-luz" das respostas); nunca chegavam à tela.

## 2. O que mudou

Com a **TRAMA** ligada, cada estrela é um corpo com massa (a da sua função:
pulso mais pesado, textura mais leve), inércia e forças, integrado pelo
diretor a cada intervalo de controle (64 amostras). A Constelação desenha
esse lugar; a profundidade continua a da órbita. Com a trama desligada, tudo
é como antes. As primitivas estão em
`include/rasgo/performance/PerformanceSkyMotion.hpp`, testadas uma a uma.

| Força / fenômeno | Física | O que se vê |
| --- | --- | --- |
| Gravidade | atração newtoniana suavizada entre estrelas acesas | grupos que se puxam |
| Espuma | bolhas macias (modelo de Durian): pressão pela sobreposição; cada bolha respira em frequências ligadas pela razão áurea | o grupo se arruma e pulsa sem nunca repetir |
| Elásticos | cada relação em foco é uma mola com comprimento próprio (pedal e contágio curtos, resposta média, hocket e atrito longos) | pares que se aproximam e se afastam conforme a trama |
| Molas e balanços | mola ao lugar de repouso; o lugar passeia em **epiciclos de Fibonacci** (velocidades Fₙ·ω₀, raios ∝ 1/Fₙ; razões áureas entre corpos) | a estrela segue um ponto que nunca para, com inércia |
| Pêndulo elástico | no contraste, um corpo pende de uma mola com gravidade, na ressonância 2:1 (k = 4g/L) | troca, sem aviso, entre balançar e quicar (caótico) |
| Cadeia de pêndulos | no desenvolvimento, três corpos pendem um do outro por molas (o primeiro do seu lugar); ação e reação entre os elos | um pêndulo triplo elástico: o movimento passa de um elo ao outro (caótico) |
| Bilhar | choques elásticos entre corpos (conservam momento e energia); bordas do campo refletem | ricochetes |
| Estádio | no contraste, paredes de estádio (Bunimovich) empurram para dentro | bilhar caótico |
| Magnetismo | carga ± da seed; força de Lorentz q·B·(vy, −vx); o campo se inverte devagar (~40 s) | espirais que trocam de sentido |
| Spin (tênis de mesa) | cada ataque é um "saque" com top ou backspin; efeito Magnus k·s·(−vy, vx); nos choques, o atrito tangencial troca giro | curvas, efeitos |
| Pista de dança | arrasto para as correntes de um atrator da família de Clifford, com parâmetros da seed que mudam a cada seção | redemoinhos generativos |
| Impulso do som | cada ataque dá um impulso no corpo | o som move a estrela |
| Distorção | matriz móvel u = c + M(t)(v − c): rotação, cisalhamento (em razão áurea), escala | o espaço inteiro se deforma, mais no contraste |

## 3. Coreografia por seção

Os pesos mudam devagar entre as seções, sem degrau:

| | âncora | atrito | correntes | magnetismo | estádio | elásticos | impulso | spin | espuma | epiciclo | distorção |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Abertura (reunir) | 3,2 | 1,3 | 0,35 | 0 | 0 | 0,6 | 0,6 | 0,4 | 0,6 | 0,04 | 0,2 |
| Desenvolvimento (pares, espirais, pêndulo) | 1,6 | 0,7 | 0,7 | 1,0 | 0 | 1,6 | 1,0 | 1,0 | 0,3 | 0,06 | 0,5 |
| Contraste (espalhar, bilhar) | 0,55 | 0,32 | 1,3 | 0,5 | 1,0 | 0,6 | 1,6 | 1,4 | 1,0 | 0,03 | 1,0 |
| Retorno (assentar) | 4,5 | 1,6 | 0,2 | 0 | 0 | 0,4 | 0,5 | 0,3 | 0,4 | 0,03 | 0,1 |

Rapidez limitada (0,9 campo/s): o céu dança, não explode.

**Borda macia da tela.** A distorção pode empurrar o lugar mostrado para
fora do campo; o corte seco em [0, 1] deixava a estrela colada na borda (até
13% do tempo, em 5 seeds de 104 s). Agora, até 0,4 do centro o lugar é o
mesmo; além disso o desvio é comprimido por tanh e nunca passa de 0,48:
nenhuma estrela fica a menos de 1% da borda (conferido nas mesmas seeds),
nenhuma fica parada.

## 4. Efeito no som

**A física também toca:**

- **choques audíveis** — quando duas estrelas acesas se chocam com
  velocidade relativa acima de 0,18 campo/s, as duas atacam juntas, com a
  força do impacto: um ritmo que nasce do bilhar, sobretudo no contraste;
- **a separatriz do pêndulo** — quando o corpo pendurado passa por cima do
  ponto de suspensão (o balanço vira volta completa; conceito estudado no
  Batruc), ele ataca.

Medido (teste, 120 s, com a cadeia de pêndulos): seed 181 — 8 choques
audíveis e 3 voltas completas; seed 9001 — 10 choques e 5 voltas. São
acentos raros, não um novo pulso.

A Constelação mostra **rastros**: cerca de 1,5 s do caminho de cada estrela,
apagando-se — as curvas e os ricochetes ficam legíveis.

As posições alimentam a distância entre estrelas, que dá o atraso das
respostas ("tempo-luz"). Logo o movimento novo muda o tempo das respostas —
só com a trama ligada (com ela desligada, o som é idêntico, conferido por
hash). Desde 3/11/2026 a física também dá o **lugar no estéreo** (pan pela
física, §4.1).

### 4.1 Pan pela física (3/11/2026, aprovado pelo autor)

Antes, todas as estrelas eram somadas em mono e uma fase lenta e global
(ESPAÇO + cabo de órbita) abria esse mono: o céu se movia em bloco, quase
mono (correlação 0,999) e com um viés fixo de 1–2 dB para a direita. Agora,
com a TRAMA ligada, cada estrela soa onde a Constelação a mostra
(`include/rasgo/performance/PerformanceSkyPan.hpp`):

- a posição x mostrada (`shownX`) vira o pan do stem, lei de potência igual,
  normalizada para o ganho de antes no centro (0,94 por canal);
- **ESPAÇO** (+ a pressão do cabo de órbita) é a largura: 0 = mono exato (o
  contrato de antes), 1 = até **70 %** para um lado — nenhuma estrela fica
  só num ouvido;
- o **grave fica no centro**: em cada stem, um filtro de um polo (~150 Hz)
  separa o grave (sempre no centro) do resto (agudo = sinal − grave, soma
  exata). Isso vale para toda estrela, não por lista — o bumbo da DRUMS, a
  Tuba e o Bass Fallback ficam firmes, e o agudo deles ainda anda;
- o lugar é alisado (~30 ms); um stem novo começa no seu lugar, sem deslizar;
- o retorno do Modal fica no centro; o stem gravado por estrela segue mono.

Sem a trama (ou em saída mono) o estéreo é o de antes, **idêntico** (hash de
20 s × 3 seeds igual ao do commit anterior). Medido com a trama, 60 s, seeds
181/9001/42, ESPAÇO 0,7: correlação 0,97, lado/meio 0,12–0,17 (antes
0,07–0,08), equilíbrio em janelas de 0,5 s entre −5 e +5 dB (antes +1 a +2
fixo), picos iguais. Teste: `tests/test_performance_sky_pan.cpp`.

Achado: o "grão" do token do MUTATE (bits 16–23) só acelerava a fase
estéreo antiga; com a trama ele não soa mais (a "inclinação", bits 8–15,
segue mudando as estrelas). O teste dos tokens passou a rodar sem a trama.
Decisão do autor (3/11, R7): com a trama, o grão **empurra a estrela em
foco** (a direção é o grão; ≈ 3 ataques de força) — o gesto se vê e se
ouve no pan. O teste dos tokens confere os dois caminhos.

## 5. Fontes e conceitos

- L. A. Bunimovich, "On the ergodic properties of nowhere dispersing
  billiards", Commun. Math. Phys. 65, 1979.
- P. Lynch, "The swinging spring: a simple model of atmospheric balance",
  2002 (pêndulo elástico, ressonância 2:1).
- C. A. Pickover, *Chaos in Wonderland*, 1994 (atratores de Clifford).
- J. D. Jackson, *Classical Electrodynamics* (força de Lorentz).
- H. Brody, R. Cross e C. Lindsey, *The Physics and Technology of Tennis*,
  2002; R. Cross, "The bounce of a ball", Am. J. Phys. 67, 1999 (Magnus,
  atrito no choque).
- D. Weaire e S. Hutzler, *The Physics of Foams*, 1999; D. J. Durian, "Foam
  mechanics at the bubble scale", Phys. Rev. Lett. 75, 1995.
- Batruc, `MOVIMENTO_GRAFICO_E_SOM.md` (só lido, nada copiado): epiciclos de
  Fourier com receitas de Fibonacci; a matriz móvel u = M(t)v; pêndulos
  acoplados por molas como pesquisa.

## 6. Achado ao ver tocando (29/10, 21h50)

Com o gancho de teste `RASGO_ASTERISMUS_AUTOPLAY=1` (play sozinho 2 s depois
de abrir; só com a variável), uma instância isolada (HOME temporário, saída
`rasgo_mudo`, conferida por `pw-link`) mostrou a tela tocando: FORMA,
TEMPO 1,08×, SWING 63%, 2+3+2+3+3+3, DERIVA, setas RESPOSTA, rastros. Mas as
estrelas se amontoavam embaixo. Causa (medida no motor, seed 2876238397): no
motor, os atores 4–8 começam **todos** em (0,50; 0,78); a tela os desenha
separados — (0,50; 0,78), (0,66; 0,66), (0,33; 0,69), (0,73; 0,38),
(0,42; 0,54). Só o arrasto de um nó envia a posição ao motor; os nós nunca
arrastados ficam empilhados lá. Antes isso não se via (a tela desenhava
órbitas próprias); agora que mostra a física do motor, cinco estrelas nascem
no mesmo ponto.

Correções possíveis (decisão do autor, porque a primeira muda o som também
com a trama desligada):

1. Enviar ao motor, ao abrir, as posições desenhadas dos oito nós (o que o
   arrasto já faz). Corrige a raiz; a ecologia passa a ouvir as distâncias
   que a tela mostra — o som com a trama desligada muda para as estrelas
   5–8 nunca arrastadas.
2. Dar ao diretor do céu as posições desenhadas só como âncoras da física,
   sem tocar na ecologia: o som com a trama desligada fica idêntico.

**Feito em 29/10, 23h30 — opção 2.** Comando novo `SkyAnchor` (controlador →
motor → ecologia): o lugar desenhado de cada nó vira a âncora da física do
céu, sem mudar a posição da ecologia. O app reenvia os oito lugares a cada
~2 s (e logo ao abrir), porque as vozes são refeitas quando a seed muda;
arrastar um nó continua mudando as duas posições. Teste
`testSkyAnchorsSeparateTheStars`: com as âncoras, o par mais próximo entre as
estrelas 4–8 fica a 0,11 (antes, empilhadas). Conferido na tela (instância
isolada tocando): as estrelas ocupam o campo, perto dos seus lugares. Som com
a trama desligada idêntico (hash). A opção 1 continua possível, se o autor
quiser que a ecologia também ouça as distâncias desenhadas.

## 7. A DANÇA vira família de movimento (30/10)

Decisão do autor: "a dança vira família de movimento". Com a trama ligada, o
botão DANÇA da Constelação escolhe a família — sem estrela em foco, para o
campo inteiro (LIVRE → BILHAR → ESPUMA → PÊNDULOS → MAGNÉTICO → GRAVIDADE);
com uma estrela em foco, só para ela ("= LIVRE" quando segue o campo). Sem a
trama, DANÇA continua sendo ÓRBITA / PULSO / FIGURA-8.

| Família | O que muda na física da estrela |
|---|---|
| LIVRE | a coreografia da seção, como antes |
| BILHAR | âncora ×0,25, atrito ×0,25, impulsos ×2,2, paredes de estádio sempre |
| ESPUMA | bolhas ×1,9, atrito ×2 (viscosa), gravidade ×0,5 |
| PÊNDULOS | pende do seu lugar (pêndulo elástico), atrito ×0,4 |
| MAGNÉTICO | campo magnético sempre presente e forte — espirais |
| GRAVIDADE | gravidade ×2, âncora ×0,6 — aglomerados que respiram |

Prioridade: a família da estrela; a do campo; a que vem do campo de órbita
antigo (ÓRBITA → LIVRE, PULSO → ESPUMA, FIGURA-8 → PÊNDULOS), para que esse
comando continue audível. Medido (teste, 40 s): BILHAR percorre 5,35 contra
2,17 da LIVRE; GRAVIDADE junta o grupo (espalhamento 0,085 contra 0,20 — com
×8 virava um bolo de 0,05, por isso ×2).

**Com isso a ecologia passou a ouvir a física** (o passo revertido em 29/10):
com a trama, as distâncias e relações da ecologia usam o lugar que a
Constelação desenha. O jogo espacial (as viradas e pausas da narrativa)
continua lendo a órbita: com a física as estrelas ficam mais quietas e o
jogo lia isso como pausa longa — a peça calava (o Galo inaudível no teste de
um minuto). Dois testes foram corrigidos por serem frágeis, não por mudança
de intenção: o A/B "desenhos diferentes soam diferentes" media o começo de
uma pausa (resíduo de 3,6e-5 contra limiar de 1e-5) e agora mede logo depois
da pausa, onde há som; o teste da fratura contava buracos no áudio de uma
estrela esparsa (loteria) e agora conta os cortes que o rasgo abre (4 contra
0). Som com a trama desligada idêntico (hash).

## 8. A VARIAÇÃO visível no mixer (30/10)

O autor: "ainda não vejo o VARIA atuando no mixer — enquanto a música está
acontecendo, somente quando se clica em algum botão; seria interessante que
houvesse variação mesmo sem clicar em nada". Medido no motor (2 min, sem
cliques): a variação age o tempo todo — o brilho de cada estrela sobe e
desce, o contraste apaga estrelas, o retorno as reacende. Mas o brilho é um
multiplicador por trás dos faders: o som muda e os faders ficam parados.
Agora, com a VARIAÇÃO ligada e tocando, uma barra âmbar ao lado de cada
fader de mix mostra o nível que a variação aplica naquele instante (15 vezes
por segundo). O fader continua sendo o ganho do performer. Alternativa
possível (decisão do autor): faders "motorizados" que se movem com a
variação.

## 9. Giro e viagem da constelação (30/10)

Pedido do autor: "não há o giro de rotação da constelação, poderia ser uma
opção a mais de composição — rotação, translação". Com a trama, o botão
**CÉU** (à esquerda da DANÇA, na Constelação) alterna FIXA → GIRA → VIAJA →
GIRA+VIAJA. GIRA roda os lugares de repouso em torno do centro da
constelação, uma volta a cada 16 compassos (sentido da seed); VIAJA leva o
centro numa órbita larga (figura de Lissajous 1:2) pelo campo, uma volta a
cada 24 compassos. As estrelas seguem os lugares pela física — o giro sai
fluido, com inércia e choques. O giro preserva as distâncias entre as
estrelas; o que muda o som é o caminho (as correntes do campo, as paredes).
FIXA devolve a constelação ao desenho em ~4 s, pelo caminho mais curto.
Teste: GIRA avança 1,51 rad em 16 tempos (¼ de volta = 1,57); FIXA volta a
menos de 0,05 rad.

## 10. Próximo

- Escuta e olhar do autor: pesos da coreografia, rapidez, quanto distorcer.
- Pan do áudio pela física: feito em 3/11 (§4.1); falta a escuta. A altura
  y (mais longe = mais escuro) fica para depois da escuta.
- Mais ou menos choques audíveis (o limiar de impacto) — decisão de escuta.
