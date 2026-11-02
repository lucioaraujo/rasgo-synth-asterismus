# A trama de relações do Asterismus — proposta (2026-10-28)

**Origem:** decisões do autor nesta data — "cada módulo como sendo uma
estrela, que podem ser acionados ou não no asterismo"; "não haverão estrelas
sozinhas, todas as estrelas se conectam para compor generativamente"; "aqui
acho que cabe se inspirar no que fizermos no aquorbium, ele está mais
avançado nessa trama de relações"; e, precisando: "não que todas as
estrelas precisem se relacionar ao mesmo tempo, mas criar recursos
inteligentes, sobretudo no quesito composição, para que o resultado musical
seja o mais elaborado e inaudito possível".

**Estado:** proposta em implementação. **Primeira fatia no código
(28/10):** `PerformanceSkyDirector.hpp` — gravitação de N corpos (leapfrog),
percepção, teia, foco rotativo (até 3 relações por compasso, com pressão por
novidade) e cinco atos audíveis: resposta com tempo-luz e altura ouvida
transformada, contágio, sombra (aumentação uma oitava abaixo), hoquetus no vão
entre ataques e pedal de apoio. Cada estrela avisa seus ataques e atende a
chamados do seu jeito (`PerformanceSky::cue`). Influência 0 = som idêntico bit
a bit ao anterior. Testes em `tests/test_performance_sky_director.cpp`
(29 relações diferentes em foco em 2 min; a estrela distante responde depois
da próxima). Custo médio inalterado (~2,6 ms por bloco com 8 acesas).

**Segunda fatia (29/10, madrugada, trabalho autônomo):**
- **Motivos e desenvolvimento (passo 2):** cada estrela guarda o seu motivo
  (as últimas 8 alturas e o ritmo entre elas). Na relação de resposta, quem
  responde espera a frase de quem chama fechar e devolve um fragmento de 2 a
  4 notas do motivo, transformado; cada resposta do par usa a transformação
  seguinte (transposição, inversão, retrógrado, aumentação, diminuição,
  fragmento). A sombra devolve o motivo em aumentação, uma oitava abaixo.
  Teste: em 40 s, 7 respostas, 4 com várias notas, 5 transformações, uma
  desenvolvendo a frase para baixo.
- **Variação do mixer** (seção 2c em código): brilho, troca e reacender das
  estrelas; mistura pelo movimento da Constelação; botão VARIAÇÃO.
- **Botão TRAMA** no Campo de Matérias, para comparar de ouvido com e sem as
  relações.
- **Guarda de capacidade** (parte do passo 4).
- **Harmonia que trabalha (passo 3, primeira parte):** as respostas caem na
  escala da seed; parte dos pares de estrelas de altura vive em **atrito**
  (a resposta vem meio-tom acima, fora da escala, e acumula tensão); com a
  tensão acima de 0,6 a primeira relação entre estrelas de altura vira
  **reparo** — a cadência à fundamental ou à quinta da seed — e a tensão cai;
  ela também se dissipa sozinha a cada compasso. Tudo dentro da TRAMA
  (desligada, som idêntico). Teste: tensão chega a 1, cadência ocorre,
  respostas cromáticas existem, a cadência desfaz a tensão. Falta: dominante
  e modulação.
- **A estrela é ouvida quando age:** com a TRAMA ligada, o evento do próprio
  módulo abre os portões de frase e pulso dele. Medido em 60 s, 8 acesas,
  3 seeds: maior silêncio contínuo 11,5→6,1 s, 6,6→1,9 s, 8,0→4,3 s; tempo
  mudo médio por estrela 72→67%, 67→56%, 71→66% (o resto é a respiração por
  frases da ecologia, para a escuta do autor).
- **Os jogos dão caráter à trama:** a estratégia do `GamePlan` (a intenção da
  peça) orienta metade das relações em foco — seguir: contágio e pedal;
  transformar: resposta com motivos; contrariar: atrito e hoquetus;
  interromper: hoquetus; esconder: sombra; negociar: resposta, com a cadência
  chegando mais cedo. Teste: contrariar dá 340 focos de atrito contra 52 de
  seguir; esconder, 924 de sombra contra 200.
- **A trama no desenho:** setas de quem chama para quem responde na
  Constelação, com o nome do ato (RESPOSTA, CONTÁGIO, SOMBRA, HOQUETUS, PEDAL,
  ATRITO, CADÊNCIA) e a cor esquentando com a tensão. Compilado e com
  telemetria testada; não verificado na tela.
- **A forma (passo 4):** seções vivas de 8 a 20 compassos (seed) — abertura
  (uma relação em foco, pedal e contágio), depois em ciclo desenvolvimento
  (respostas com motivos, 2–3 relações), contraste (atrito e hoquetus; com a
  variação ligada, troca boa parte das estrelas acesas) e retorno (as
  estrelas da abertura reacendem e os motivos voltam só transpostos,
  reconhecíveis). A escolha de cada relação em foco divide-se em terços: tipo
  natural do par, seção da forma e estratégia do jogo. Cabeçalho mostra
  "FORMA · seção". Teste: 14 seções em 400 s, ordem e comprimentos certos,
  abertura com uma relação, retorno reacende a abertura. A coda continua a
  do motor (REC/CODA e epílogo).

## 1. O que o Aquorbium faz e o Asterismus ainda não

Lido em `PROJETOS_AUTORAIS/AQUORBIUM/` (commit `dc3567d`):
`ECOSYSTEM_DESIGN.md`, `ECOLOGICAL_MUSIC_DESIGN.md` §§1–6,
`SYNTH_DEVICE_DESIGN.md` e `src/core/EcologicalMusicDirector.*`.

| Aquorbium | Asterismus hoje |
|---|---|
| A física das relações roda **num relógio de controle** (200 Hz / semicolcheia); as vozes só leem alvos suavizados | a ecologia calcula tudo **por amostra e por corpo** (~0,3 ms por corpo aceso a cada bloco) |
| **Teia de todos com todos** (matriz 5×5), recalculada a cada passo | relações só nos **cabos declarados** (até 16, entre pares fixos) |
| O **tipo** da relação **emerge** do estado (caça, fuga, rivalidade, aliança, corte, guarda) | o tipo é fixado no desenho da cena |
| **Percepção**: ninguém vê tudo; só reage ao que percebeu (distância, campo, turbidez) | não há percepção; a proximidade só pesa a pressão global |
| Cada relação gera **atos musicais** reconhecíveis (cânone/stretto, diminuição, hoquetus, homorritmia, dueto em terças, sinal) | a relação modula ganho, fase e cor, sem atos |
| **Campo harmônico comum**: centro, modo, progressão; pares compatíveis puxam consonância, incompatíveis acumulam tensão → dominante → cadência | cada estrela tem a sua própria altura, sem harmonia comum |
| **Densidade/nicho** (Krause): capacidade de eventos por compasso repartida por saliência; quem coincide de registro se desloca | não há; daí máscaras e silêncios sem razão |
| **Propagação**: o que um organismo imita é o que ele *ouviu*, com atraso = distância | não há |
| **Matriz de modulação** por organismo: sinais do ecossistema → parâmetros da voz; **PROPOSTA** gera o patch de partida pela semente; o músico edita | três controles fixos por estrela |

## 2. A tradução para o céu

O vocabulário fica do Asterismus; o mecanismo vem do Aquorbium.

- **Percepção → gravitação e brilho.** Cada estrela acesa sente as outras
  pela distância viva no Campo, pela presença (magnitude) e pelos cabos.
  Nenhuma estrela sente o céu inteiro com a mesma força.
- **Teia → constelação viva.** Uma matriz de todos os pares acesos (8×8 hoje,
  N×N depois), recalculada no relógio de controle. O **tipo** de cada par
  emerge da função das duas estrelas (perfil orquestral do Studio: registro,
  ocupação, modo de presença, papel), da distância e da seed, usando os tipos
  que o Asterismus já tem: Support, CallResponse, Hocket, Shadow, Contagion,
  Friction, Infiltration, Repair, Orbit. A intensidade sobe rápido e decai
  devagar (como no Aquorbium). **O cabo desenhado pelo músico ou pela seed
  continua sendo a ligação forte**; a teia é a ligação de todos com todos.
- **Atos por relação** (o que se ouve):

| relação | ato musical |
|---|---|
| CallResponse | a resposta chega com atraso proporcional à distância ("tempo-luz") e usa o que foi ouvido |
| Hocket | ataques alternados entre as duas estrelas |
| Shadow | imitação em aumentação, mais grave (memória) |
| Contagion | o ritmo de uma passa à outra |
| Support | homorritmia ou pedal sob a outra |
| Friction | incompatibilidade: puxa dissonância e tensão |
| Repair | puxa consonância e resolução |
| Orbit | retorno periódico do motivo de uma na outra |
| Infiltration | o timbre de uma move os controles da outra |

- **Campo harmônico do céu.** Centro, modo e progressão comuns (a escala pode
  vir da peneira de Xenakis do Studio, `SieveScale`). A tensão acumulada pela
  fricção leva à dominante; o reparo leva à cadência.
- **Nicho e densidade.** A capacidade de acontecimentos por compasso é
  repartida por saliência; duas estrelas no mesmo registro e ocupação não se
  mascaram — uma cede. **A mesma capacidade é também o orçamento de CPU**:
  o céu nunca acende mais do que a máquina mede como seguro.
- **Matriz de modulação por estrela** (como o dispositivo de síntese do
  Aquorbium): sinais do céu — energia, proximidade, relação mais forte,
  tensão harmônica, ataque recebido — ligados aos três controles de cada
  estrela; a seed propõe as ligações iniciais; o músico edita.

## 2b. A inteligência composicional (o centro da proposta)

A teia é **potencial**, não um emaranhado permanente: qualquer estrela *pode*
se relacionar com qualquer outra, mas a cada momento poucas relações estão
em primeiro plano. Quem decide é um conjunto de recursos composicionais,
para que o resultado seja elaborado e inaudito — nem sino de vento (o
diagnóstico do Aquorbium em 20/10: muitas notas, nenhuma forma), nem tudo
soando ao mesmo tempo.

- **Foco relacional.** A cada frase o céu escolhe uma, duas ou três relações
  ativas (por saliência, forma e estratégia da seed); as outras ficam
  latentes, à espera. Mudar de foco já é um acontecimento formal.
- **Motivo e desenvolvimento.** Cada estrela tem um motivo próprio (sua
  "assinatura", como o canto de cada espécie em Messiaen) e o céu guarda uma
  memória dos motivos. Uma relação não repete: transforma — transposição,
  inversão, retrogradação, aumentação, diminuição, fragmentação, troca de
  timbre (Klangfarbenmelodie entre estrelas), stretto.
- **Pressão por novidade.** O diretor mede o que acabou de soar (classes de
  altura, registros, ritmos, densidade — como o diagnóstico de entropia do
  Aquorbium) e, quando o material se repete, força contraste: outra relação
  em foco, outro registro, silêncio, ruptura. Repetição só como escolha
  formal (retorno, refrão, soundmark), nunca por inércia.
- **Forma.** Um arco vivo de seções — abertura, desenvolvimento, contraste,
  retorno, coda — com memória formal (a cadeia de Markov da forma do
  Aquorbium; o `GenerativeArc` e o `GamePlan` do Studio: estratégias Follow,
  Transform, Counter, Interrupt, Conceal, Negotiate). A seção decide quantas
  estrelas e relações cabem e que tipo de ato predomina.
- **Harmonia que trabalha.** Tensão acumulada pela fricção, dominante,
  cadência, modulação só depois de estabilidade; a escala da seed pode ser
  não catalogada (peneira de Xenakis), o que já afasta o resultado do
  conhecido.
- **Raridade.** Atos raros e marcantes (o "footnote" do Studio, a revolução
  de canto e a rede de bolhas do Aquorbium têm equivalentes possíveis no
  céu: um motivo que contamina todos os que o ouvem; um gesto fora da escala
  uma vez por peça) têm orçamento próprio, para que soem como acontecimento.
- **O músico manda.** Todo foco, motivo e seção pode ser contrariado pelo
  gesto, pelo cabo e pelo fader; o céu responde ao que o músico fez, não
  apaga.

Custo: estes recursos são decisões no relógio de controle (por frase,
compasso ou semicolcheia), não trabalho por amostra.

## 2c. A troca de estrelas — o brilho como ciclo de vida

Decisão do autor (28/10): "pode haver um sistema de troca de estrelas,
estrelas que perdem o brilho e são substituídas por outras, ou que retornam
em momentos diferentes. Essa dança de estrelas depende dos algoritmos
criados, dos jogos a serem jogados, de intenções diferentes".

- **Brilho como estado de vida.** Cada estrela no Campo tem um brilho que
  nasce, cresce, se mantém, perde força e se apaga — não um volume fixo. É a
  forma em grande escala: quem entra, quem sai, quem volta.
- **Troca.** Uma estrela que se apaga libera o seu lugar no Campo para outra
  do céu inteiro (o catálogo todo, não só as oito da abertura). O Campo tem
  oito lugares; o céu tem todas as estrelas. Isso também responde à questão
  em aberto do Campo diante de mais de 20 estrelas.
- **Retorno.** Uma estrela apagada guarda memória (o seu motivo, os seus
  controles, as relações que viveu) e pode voltar em outro momento —
  transformada pelo que se passou enquanto estava fora, ou como retorno
  formal reconhecível.
- **Reacender** (o autor, 28/10: "estrelas que reacendem"). Além de ser
  substituída ou voltar mais tarde, uma estrela que perdeu o brilho pode
  **reacender no próprio lugar**. A astronomia oferece maneiras distintas,
  cada uma com consequência musical própria:
  - **estrela variável** (as cefeidas): o brilho pulsa num período próprio
    — presença periódica, um retorno que se pode prever e esperar;
  - **nova**: reacende de repente, muito acima do brilho anterior, e decai
    — um acontecimento, com o material transformado pelo que viveu;
  - **nova recorrente**: reacende em intervalos irregulares — retornos
    imprevisíveis de uma mesma identidade;
  - **eclipse** (binária eclipsante): perde o brilho porque outra estrela
    passa à sua frente e o recupera quando ela sai — o silêncio de uma é
    causado pela relação com outra.
  O tipo de reacendimento de cada estrela pode vir da seed, do jogo em curso
  e da intenção da seção; o músico pode provocar ou impedir.
- **Quem decide a dança:** os algoritmos do céu (forma, pressão por
  novidade, nicho), os **jogos** em curso (as estratégias e regras do
  `GamePlan` do Studio — seguir, transformar, contrariar, interromper,
  esconder, negociar; os pesos Alea/Agon/Ilinx/Mimicry de Caillois) e
  **intenções** diferentes por seed e por seção. A mesma seed com outro jogo
  dá outra dança.
- **Referências de mecanismo já existentes:** nascimento, morte, partida e
  revolução de canto no Aquorbium (`EcologicalMusicDirector`: `birth`,
  `death`, `departure`, `revolution`); o registro de atores do Studio, que só
  admite organismos realmente manifestados (`PerformanceActorRegistry`).
- **O músico participa:** pode segurar uma estrela (impedir que se apague),
  apagar uma à mão, pedir um retorno, ou deixar o céu dançar sozinho.
- **Custo:** a troca respeita a guarda de capacidade — nunca há mais
  estrelas acesas do que a máquina sustenta. Entrar e sair são eventos
  suavizados (sem clique) e preparados fora do áudio (sem alocação).

## 2d. As leis do universo como cálculo da dança

Pedido do autor (28/10): "cálculos das leis físicas, quânticas, do
universo, etc". As leis entram como **motores de cálculo** — equações
integradas no relógio de controle, determinísticas pela seed e limitadas —,
não como nomes dados a um sorteio. Cada uma alimenta a forma, as relações,
o brilho ou os parâmetros das estrelas.

| Lei ou fenômeno | O que se calcula | Consequência musical |
|---|---|---|
| Gravitação de Newton, problema de N corpos (integrador simplético, ex. leapfrog) | posições e velocidades das estrelas no Campo sob atração mútua; a âncora desenhada pelo músico é o poço de potencial | a dança real: aproximação, captura, fuga; a distância calculada alimenta percepção e relações |
| Leis de Kepler (órbita elíptica, áreas iguais, T² ∝ a³) | período e velocidade orbital de cada par ligado | andamentos relacionados por razões físicas; acelera no periélio, abranda no afélio |
| Três corpos e caos (sensibilidade às condições iniciais) | trajetórias imprevisíveis mas determinísticas pela seed | material inaudito que a mesma seed reproduz |
| Pontos de Lagrange | posições de equilíbrio entre dois corpos | lugares estáveis onde uma terceira estrela "descansa" (pedal, acompanhamento) |
| Precessão e nutação | rotação lenta do eixo e pequena oscilação | já previstas na gramática do Campo: deriva lenta de registro/timbre e microvariação |
| Efeito de maré e travamento de maré | dois corpos próximos sincronizam rotação | duas estrelas próximas acabam em fase (uníssono rítmico) |
| Relação massa–luminosidade (L ∝ M^~3,5) e tempo de vida (t ∝ M^−2,5) | brilho e duração da vida de cada estrela | estrela "massiva": forte e breve; "leve": discreta e longa — o ciclo de brilho da seção 2c |
| Lei de Wien e corpo negro (Planck; Stefan–Boltzmann L ∝ R²T⁴) | temperatura → pico espectral e potência | temperatura da estrela vira brilho tímbrico (corte do filtro, parciais) e nível |
| Efeito Doppler / desvio para o vermelho | velocidade relativa ao ouvinte | microafinação ao aproximar/afastar |
| Linhas espectrais do hidrogênio (Rydberg; séries de Lyman, Balmer, Paschen: 1/λ = R(1/n₁² − 1/n₂²)) | razões de frequência das transições | **escalas e acordes não catalogados** a partir das transições atômicas — afinação vinda da física, não de um sistema musical |
| Níveis quânticos e saltos (estados discretos, transição com probabilidade) | cada estrela tem estados de energia; salta entre eles por probabilidade | mudanças de registro ou de motivo em degraus, não contínuas |
| Tunelamento quântico | probabilidade de atravessar uma barreira proibida | o raro: uma estrela que "atravessa" para um estado que as regras impediam (o ato raro da seção 2b) |
| Superposição e medição (colapso) | estado indefinido entre alternativas até ser "observado" | a escolha só se decide quando o músico foca ou um cabo liga: a observação compõe |
| Emaranhamento (correlação não local) | pares correlacionados independentemente da distância | duas estrelas distantes que mudam juntas — uma relação que não depende do espaço |
| Expansão do universo (lei de Hubble, v = H₀·d) | o espaço do Campo se dilata ou contrai | rarefação ou adensamento global; a coda como expansão |
| Matéria escura | massa invisível que curva as órbitas | uma influência inaudível que organiza a dança (centro de gravidade sem som) |
| Supernova e nebulosa | fim violento de uma estrela massiva; a matéria espalhada forma novas | ruptura formal; o material da estrela morta semeia as que nascem (a memória da seção 2c) |

**Cuidados:** cada lei é um modelo simplificado e declarado (escalas
normalizadas, constantes ajustadas ao Campo), sem pretensão de simulação
científica; o que conta é a coerência do cálculo e a consequência audível.
N corpos com 8 estrelas são 28 pares por passo de controle — custo
desprezível. As fontes ficam citadas; nenhum código de terceiros entra no
repositório: as equações são escritas a partir da teoria.

**Fontes:** Newton, *Philosophiae Naturalis Principia Mathematica* (1687);
Kepler, *Astronomia Nova* (1609) e *Harmonices Mundi* (1619); Poincaré, *Les
méthodes nouvelles de la mécanique céleste* (1892–99); Lagrange, *Essai sur
le problème des trois corps* (1772); Planck (1901), lei da radiação do corpo
negro; Rydberg (1890) e Balmer (1885), séries espectrais do hidrogênio;
Hubble (1929), relação velocidade–distância; Hertzsprung–Russell, relação
massa–luminosidade (Eddington, 1924); Einstein, Podolsky e Rosen (1935) e
Bell (1964), correlações quânticas; Hairer, Lubich e Wanner, *Geometric
Numerical Integration* (2006), integradores simpléticos. Na música:
Xenakis, *Formalized Music* (1971) — leis físicas e estocásticas como
geradoras de forma.

## 3. Arquitetura e custo

Um diretor do céu num relógio de controle (por exemplo a cada 64 amostras ou
por semicolcheia) calcula percepção, teia, atos, harmonia e nicho e entrega a
cada estrela alvos limitados e eventos. As estrelas continuam por amostra,
lendo esses alvos suavizados. Isso implementa as conexões **e** tira da
amostra o trabalho que hoje custa ~0,3 ms por corpo — o mesmo desenho que
mantém o Aquorbium leve. Meta medida: ≤ 0,1 ms por estrela acesa.

Reuso: o Aquorbium é do mesmo autor. Conceitos e estruturas serão
reescritos para o céu; trechos de código copiados levam proveniência
(origem, commit, licença). Nenhuma dependência de build entre os dois.

## 4. Ordem proposta

1. Diretor do céu no relógio de controle, com a gravitação de N corpos
   como base da dança: percepção e teia 8×8 com os tipos existentes —
   primeiro só telemetria e testes, sem mudar o som.
2. Foco relacional e atos por relação chegando às estrelas como eventos;
   motivo por estrela e suas transformações.
3. Campo harmônico comum, compatibilidade e tensão; pressão por novidade
   medida.
4. Forma (seções, memória formal, estratégia da seed); nicho e densidade,
   ligados ao orçamento de CPU (guarda de capacidade); a troca de estrelas
   (brilho, substituição, retorno) guiada por jogos e intenções.
5. Matriz de modulação por estrela e proposta pela seed.
6. Só então: Modal como estrela e os módulos do Studio, um por vez, cada um
   com seu perfil orquestral e sua medição de custo.

Cada passo: teste, medição de custo, escuta do autor.
