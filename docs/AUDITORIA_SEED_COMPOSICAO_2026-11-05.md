# Auditoria — o que a seed faz e o que poderia fazer (2026-11-05)

Pedido do autor: "faça uma auditoria para saber o que o seed faz e ou que ele
poderia fazer que ainda não faz, para melhorar a parte composicional das
faixas" — "com relação a todas possibilidades codadas do instrumento".

**Método.** Leitura do código (`include/rasgo/performance/`), não da
especificação: cada espaço de possibilidades codado (enums, tabelas, faixas de
parâmetro) foi seguido até quem o escolhe — a seed, o músico ou uma regra fixa.
A especificação de 2026-09-08 (`ALGORITMO_SEED_MODULOS.md`) serve de
comparação: ela pede regiões harmônicas, progressões, pivôs e modulação, que o
código não tem. Nada foi mudado nesta auditoria.

## 1. O que a seed decide hoje

| Domínio | O que a seed escolhe | Onde |
|---|---|---|
| Céu | 8 corpos entre 28 estrelas (≤ 1 de controle); corpo 5 sempre MODAL | `performanceInitialAsterism` (Sky.hpp) |
| Perfil | estilo (5), dialeto (6), extensão melódica, impulso rítmico, brilho, afinidade a eventos, espacial e coletiva | `compositionProfileForSeed` (ModuleTypes.hpp) |
| Harmonia | raiz (12), escala (6: dórico, bebop dominante, bebop maior, alterada, diminuta, tons inteiros), transposição global contínua (×0,78–1,26) | Ecology.hpp `seedScale`, `seedRootMidi`, `seedTonalScale` |
| Andamento | 96–148 BPM; ±6 % e swing por parte (caráter) | TerritoryFactory `between(seed,14)`; SkyDirector |
| Jogo | estratégia (6) × regra (6), energia, memória, risco, liberdade, informação oculta | `nativeGamePlan` |
| Forma | duração de cada parte (8–20 compassos); o caráter de cada parte (9 dimensões: estrelas, atividade, andamento, swing, peso das 5 famílias) | SkyDirector `sectionLength`, `stepCharacter` |
| Elenco | quem acende em cada parte (peso por família), liderança que gira, plano de mixagem | SkyDirector |
| Trama | pares em foco e o ato de cada par (um terço pela natureza do par, um terço pela parte da forma, um terço pela estratégia); transformação de motivo (transpor, inverter, retrógrado, aumentar) | SkyDirector `strategicKind`, `MotifTransform` |
| Tempo | contagens por estrela, agrupamentos (3+3+2…), estrela que deriva, luas/eclipses | Sky.hpp (ver `TEMPO_MALEAVEL.md`) |
| Cenas | presenças, intensidades das relações, intensidade do fenômeno de população (não o fenômeno) | TerritoryFactory |
| Estrelas | os 3 controles iniciais de cada estrela (0,24–0,80) | `initialModuleControls` |
| Coda | o verbo (4: reunir, revezar, memória, dispersar) e o líder | Ecology `beginCoda` |
| Álbum | a seed de cada faixa (da seed do álbum) | Album.hpp |

## 2. O que está codado e a seed não usa

| Possibilidade codada | Quantas | Quem escolhe hoje |
|---|---|---|
| Fenômeno relacional de cada cabo (atrair, repelir, orbitar, perseguir, fundir, ressoar, trocar…) | 23 | só o músico; toda relação nasce ATRAIR (todos os manifestos de 5/11 dizem ATRAIR) |
| Fenômeno de população por cena (alinhar, aglomerar, espalhar, girar, pulsar, fragmentar…) | 10 | fixo: ESPIRAL = girar, RASGO = fragmentar, BALANÇO = alinhar; a seed só dá a intensidade |
| Família de movimento (livre, bilhar, espuma, pêndulo, magnético, gravidade) | 6 | só o músico (DANÇA) |
| Movimento da constelação (parada, gira, viaja, gira e viaja) | 4 | só o músico |
| Coreografia por parte (âncora, atrito, correntes, magnetismo, estádio, elásticos, impulso, Magnus, espuma…) | 11 pesos × 4 partes | tabela fixa: toda seed tem o mesmo contraste "bilhar no estádio" |
| Cena por parte no PERCURSO | 4 | fixo: abertura → NASCENTE, desenvolvimento → ESPIRAL, contraste → RASGO, retorno → BALANÇO |
| Gestos (HOLD, MUTATE, RASGAR, INTERRUPT, STUTTER, DESLOCAR, RECALL, SILENCE…) | 11 | só o músico; o céu nunca faz um gesto |
| Gramática da forma | 1 | fixa: abertura uma vez, depois desenvolvimento → contraste → retorno em ciclo |
| Ato da trama por parte | — | fixo por parte (abertura = apoio/contágio, desenvolvimento = chamada e resposta, contraste = atrito/hocket, retorno = chamada e resposta) |
| Estrelas fora do céu | 20 por seed | só o músico (BIBLIOTECA); o céu nunca troca uma estrela durante a peça |
| Harmonia no tempo | — | não existe: uma raiz e uma escala para a peça inteira, sem região, progressão nem modulação (a especificação pede) |
| Coda | duração | fixa: 4 s de coda + 1,2 s de cauda, numa faixa de 1 ou de 5 min |

## 3. Defeitos encontrados (técnicos)

1. **As seeds pequenas perdem o perfil.** `compositionProfileForSeed` lê fatias
   de bits do número da seed (`fold >> 8`, `>> 12`, `>> 16`, `>> 20`, `>> 24`) sem
   misturar. Para qualquer seed abaixo de 256, extensão melódica, impulso
   rítmico, brilho, afinidade a eventos e coletividade ficam **todos no mínimo
   (0,12)**. As cinco seeds da escuta guiada (1, 26, 84, 628, 1086) têm esse
   perfil achatado; uma seed do ALEA (ex.: 2326326530) varia de 0,53 a 0,87.
   As seeds digitadas à mão — as mais tocadas — são as mais pobres.
2. **Genes correlacionados.** As fatias se sobrepõem (bits 8–15 e 12–19, 4–11 e
   8–15…): extensão melódica e impulso rítmico, por exemplo, dividem 4 bits. O
   modelo de seed do Studio pede o contrário: fluxos separados por domínio.
3. **Raiz e escala por fatia de bits:** `36 + (seed >> 3) % 12` e
   `(seed >> 7) % 6`. As seeds 0–7 têm a mesma raiz; 26 e 1086 caem na mesma
   raiz (39); abaixo de 128 a escala depende só do dialeto.
4. **Somas lineares em vez de mistura:** o verbo da coda é
   `(seed + virada·3 + relações·5) % 4` e o líder `(seed + …) % corpos` — seeds
   vizinhas dão escolhas em sequência, não independentes.
5. **Desafinação em Hz absolutos:** `(actorId + seed) % 9 × 0,7 Hz` (até 5,6 Hz)
   é somado à frequência. No grave (65 Hz) isso é mais de um semitom; no agudo,
   quase nada.
6. **A coda não acompanha a duração** (4 s fixos).

Os defeitos 1–4 mudam o som de todas as seeds quando corrigidos (cada seed
vira outra peça). Por isso ficam para a decisão do autor, junto com as
propostas.

## 4. Propostas — o que a seed poderia fazer

Em ordem de impacto composicional. Todas passam pela escuta e pelo OK do
autor (são mecânica). Princípio comum, já do projeto: **poucos genes com
sentido, fluxo separado por domínio, orçamento em vez de sorteio a cada
passo** (modelo de seed do Studio), e nada de padrão igual entre faixas.

**A. Consertar a seed (pré-requisito).** Perfil, raiz, escala e coda pela
mesma mistura `mix(seed, domínio)` que a fábrica e o diretor já usam, um
fluxo por domínio. Toda seed — 1 ou 4 bilhões — passa a ter um perfil inteiro.

**B. A seed escolhe os fenômenos.** Para cada cabo, um fenômeno relacional
entre os 23 compatível com o par (ex.: tom × tom → orbitar/ressoar/sincronizar;
pulso × textura → perseguir/espalhar); por cena, o fenômeno de população entre
os 10; por parte, uma família de movimento entre as 6 e uma coreografia que
varia em torno da tabela atual (a tabela vira centro, não regra). É o maior
espaço codado e hoje parado.

**C. Gramática da forma por seed.** Além do ciclo atual: arco (subida → clímax
→ queda), Freytag, kishōtenketsu (a virada no terceiro tempo), rondó (retornos
que voltam transformados), forma em ondas. A seed escolhe a gramática, onde
fica o clímax e quantos ciclos; accelerando e ritardando reais nas viradas.
Arte anterior: o "morceau/arc" do rasgo-synth (cadeia de Markov sobre
intro/subida/clímax/queda/coda e as gramáticas Freytag/kishōtenketsu) —
conceito, não código (RASGO_SYNTH é só leitura).

**D. Harmonia no tempo.** Regiões por parte: o contraste muda de região
(relativa, dominante, mediante, trítono — a seed escolhe a relação e a
gramática harmônica), o retorno volta à tônica; pivôs nas viradas. É o que a
especificação de setembro pede e o código ainda não tem.

**E. O céu também faz gestos.** Um orçamento de gestos por parte (STUTTER,
DESLOCAR, HOLD, RASGAR, SILENCE), escolhidos pela seed e pela energia da
parte — poucos, com sentido, nunca em cima do gesto do músico.

**F. Troca de estrelas durante a peça.** No contraste, 1 ou 2 estrelas de fora
do céu entram no lugar de outras (escolhidas pela seed, pela família que o
caráter pede), e voltam no retorno — o contraste ganha timbres novos de
verdade, não só outro elenco das mesmas oito.

**G. Coda e cena na proporção da faixa.** Coda de 4 a 12 s conforme a duração;
o PERCURSO escolhe a cena de cada parte pelo caráter (não sempre a mesma).

**H. Álbum gerado inteiro** (pergunta do autor, 5/11). Hoje cada faixa é
gravada com REC (a próxima entra sozinha). O Studio renderiza o álbum offline.
No Asterismus seria **GERAR O ÁLBUM**: as faixas renderizadas fora do tempo
real, numa thread de trabalho, cada uma com a sua seed, andamento e duração
(o contorno do álbum também pode escolher durações e gramáticas), depois a
masterização. Medido nas sondas: 8 corpos rendem ~7× o tempo real numa
thread (8 min de álbum ≈ 70 s). Alternativa sem renderizador: gravar em
sequência, ao vivo, faixa após faixa, sem tocar em nada.

## 5. Próximo passo

O autor escolhe por onde começar. Recomendação: **A** (sem ela, B–G
continuam pobres nas seeds pequenas), depois **B** e **C**, que mais mudam a
composição das faixas, e **H** quando o álbum for o foco.

## 6. Feito

**A (5/11, "pode começar").** `performanceSeedStream(seed, domínio)`
(ModuleTypes.hpp): splitmix64 com o domínio, um fluxo por gene. Perfil (8
genes), raiz, escala, grau de cada corpo, timbre das estrelas do céu (bumbo,
caixa, chimbal, rungler, gendyn, vosim, spectral, harmonaig, passos iniciais) e
o verbo e o líder da coda saem de fluxos próprios; as fatias em tempo de
execução da Sky usam `seedMix_`. Desafinação em cents (0–16), não em Hz. As
seeds da escuta passaram de perfil no mínimo para, por exemplo, seed 1: ritmo
0,59, brilho 0,74; seed 84: melodia 0,70, coletivo 0,84. Achado no caminho: a
SPECTRAL com parciais ímpares passava de 1 (o ganho da biblioteca entrava
depois do tanh) — corrigido. A tabela de nível das estrelas foi remedida
(48 seeds 5000–5047 × 90 s): de 8,6 dB de diferença para a nova mediana;
loudness média −18,1 LUFS com MASTER −6. Suíte 37/37. **Toda seed soa
diferente do que soava antes de 5/11** (é outra peça com o mesmo número).
