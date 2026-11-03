# Auditoria da interface — botões e controles (2026-10-29)

Pedido do autor: "fazer uma auditoria dos botões, quanto ao layout (visual,
texto, etc), quanto a função, quanto a pertinência de manter ou melhorar para
o novo modelo de instrumento."

Método: captura da janela inteira (1920×1006, binário de 20:20, parado) lida
por região; cada controle conferido no código (`src/app/PerformanceApp.cpp`
e o motor), sem afirmar função que não foi lida. O "novo modelo" é o céu:
cada módulo uma estrela, a TRAMA (relações escolhidas pelo diretor), a forma
(abertura, desenvolvimento, contraste, retorno), o tempo maleável e, desde
hoje, o movimento físico na Constelação.

Legenda da última coluna: **corrigido** (defeito, feito nesta sessão),
**manter**, **melhorar** (proposta), **decisão do autor** (escolha estética
ou de identidade — não aplicada).

## 1. Barra superior

| Controle | Visual / texto | Função (conferida) | Pertinência no modelo novo | Recomendação |
|---|---|---|---|---|
| SEED (número) + ALEA | legível | ALEA sorteia uma seed e redesenha o território | central: a seed escolhe estrelas, forma, contagens, luas | manter |
| TEMPO / BPM (slider + caixa) | legível | andamento-base | o andamento real agora respira (TEMPO 1,04× no cabeçalho) | melhorar: mostrar ao lado o andamento efetivo (base × fator) |
| MASTER / dB | legível | ganho final | neutro | manter |
| PEAK | barra discreta | pico de saída | neutro | manter |
| TUTORIAL / SOBRE / PT | legível | janelas de ajuda; PT muda o idioma dessas janelas e de parte da interface | o tutorial não fala do céu, da trama, da forma nem do movimento | melhorar: reescrever o tutorial para o modelo novo |
| REC/CODA + PLAY | legível; caixa "TRANSPORTE · REC / CODA" | REC grava; durante a gravação pede a coda; a peça termina sozinha ao fim do percurso da seed (com REC) | central | manter; melhorar: avisar no rodapé quando a peça se aproxima da coda (hoje parar sozinho surpreende — foi perguntado hoje) |
| Linha de estado (STOP · SAÍDA SILENCIOSA · CENA · CORPOS · STEMS · LIMITE · PERFIL · ATRASOS · MODAL · FORMA · TEMPO · SWING · agrupamento · DERIVA · ECLIPSE · CPU) | muito longa; com a trama ligada passa a ter 15+ itens | leitura do estado | útil, mas mistura técnica (STEMS, LIMITE, PERFIL, MODAL v1) e música (FORMA, TEMPO, SWING) | melhorar: separar em duas linhas — música (forma, tempo, swing, agrupamento, deriva, eclipse) e máquina (stems, atrasos, CPU) |

## 2. Cenas e gestos

| Controle | Visual / texto | Função | Pertinência | Recomendação |
|---|---|---|---|---|
| DESFAZER / CAPTURAR | **cortados** ("DESFAZ…", "CAPTUR…") | captura BPM, 7 macros, presenças e relações numa cena; desfaz a última captura | útil para performance | **corrigido** (largura) |
| MORPH 0,8 s | legível, pequeno | tempo de transição entre cenas | útil | manter |
| ABERTURA / ESPIRAL / RASGO / BALANÇO (cenas 1–4) | legível | recallam cenas | **colisão de nome**: "ABERTURA" é também a primeira seção da forma, que aparece no cabeçalho (FORMA · ABERTURA) | **resolvido 03/11/2026**: a cena 1 passou a NASCENTE (decisão do autor); a seção segue ABERTURA |
| HOLD | legível | suspende os relógios (o céu também para) | pertinente | manter |
| ABRIR · ⟨módulo⟩ / RASGAR · ⟨módulo⟩ / REPARAR · ⟨módulo⟩ | **tamanhos de letra diferentes** (RASGAR e REPARAR encolhem por serem longos) | atuam no módulo em foco (variação selvagem, cicatriz, reparo) | pertinentes; o alvo é o nó em foco | **corrigido** (fonte única); a dica do campo dizia "MUTATE, RASGAR E RECALL" — **corrigido** para os nomes dos botões |
| STUTTER | legível | recorte rítmico do material | pertinente | manter |
| TEMPO SHIFT | legível | é um **recorte** (como o stutter, deslocando o material), não muda o andamento | o nome engana agora que existe tempo elástico de verdade | decisão do autor: renomear (ex.: DESLOCAR) |
| PANIC SAFE | vermelho, legível | limita o ganho; com REC fecha a tomada | segurança | manter |

## 3. Eixo de Forças e macros

| Controle | Visual / texto | Função | Pertinência | Recomendação |
|---|---|---|---|---|
| EIXO DE FORÇAS (dois pontos, azul e laranja) | área grande, quase vazia | o ponto azul dá AUTO e INTERV.; o laranja, DENS. e TENS.; MEM., RISCO, ESPAÇO e PROF. saem da combinação dos dois | os macros alimentam o céu (densidade, tensão, autonomia…) | melhorar: o eixo podia mostrar também o estado do céu (energia do movimento, tensão harmônica) |
| 8 macros (AUTO, INTERV., DENS., TENS., MEM., RISCO, ESPAÇO, PROF.) | legíveis | **os mesmos valores do Eixo de Forças** — dois controles para os mesmos parâmetros | redundância | decisão do autor: manter os botões redondos só como leitura, ou fundir |

## 4. Campo / Constelação

| Controle | Visual / texto | Função | Pertinência | Recomendação |
|---|---|---|---|---|
| Nós (estrelas) e cabos | com a trama: setas da relação em foco, brilho, ponteiro de fase, anel de deriva, eclipse, rastros, movimento físico | clicar escolhe o nó em foco; arrastar desenha o lugar | central | manter |
| Caixas laterais (1 FM FLUTE · CHÃO / STEM 1 · ATIVO / SIL) | "SIL" e "EMERGE / SIL" não se explicam | papel do módulo e estado do stem | úteis, mas crípticas | melhorar: "SIL" por extenso (SILÊNCIO?) — decisão do autor |
| DANÇA · ÓRBITA / PULSO / FIGURA-8 | botão pequeno no canto | escolhe a órbita paramétrica | **com a trama ligada a tela mostra a física**, então este botão não muda mais o que se vê | decisão do autor: transformar em escolha de família de movimento (ex.: BILHAR, ESPUMA, PÊNDULOS, MAGNÉTICO, LIVRE) ou deixar só para a trama desligada |
| FENÔMENO (23 tipos nos cabos: ATRAIR, REPELIR, ORBITAR, ESPIRAL, OSCILAR…) | escolhido num menu ao clicar num cabo | deslocamentos **paramétricos** somados ao lugar | agora a atração, a repulsão, a espiral e a oscilação existem **fisicamente** | melhorar: cada fenômeno virar uma força física no par (ATRAIR = mola curta/gravidade forte; REPELIR = carga igual; ESPIRAL = campo magnético local; OSCILAR = mola longa) — decisão do autor |
| BIBLIOTECA · 29 MÓDULOS · … / STUDIO: 14 EM AUDITORIA … | texto de desenvolvimento dentro da área de tocar | inserir um módulo da biblioteca (clique) | a linha "STUDIO: … EM AUDITORIA · CANDIDATO REALTIME" é informação de bastidor | melhorar: manter "clique para inserir", levar a contagem de auditoria para SOBRE |
| 16 relações (MATÉRIA FM FLUTE → SCAT, TEMPO SCAT → ATARI PUNK…; 8 visíveis por vez) | legíveis, valores 0,00 nos cabos desligados | intensidade das relações **fixas** do território (antigas); alimentam a ecologia | **dois vocabulários de relação convivem**: os 16 cabos fixos (MATÉRIA, TEMPO, ESPAÇO, ÓRBITA, MEMÓRIA, PULSO, RISCO) e a trama do céu (RESPOSTA, CONTÁGIO, SOMBRA, HOQUETUS, PEDAL, ATRITO, CADÊNCIA), escolhida pelo diretor | decisão do autor — a mais importante desta auditoria: (a) manter os dois; (b) fazer dos 16 sliders o peso com que cada par pode entrar em foco na trama; (c) aposentar os cabos fixos |
| Linha "última ação · … · seed · memória ≤ · risco ≤ · TRAJETÓRIA" | legível | eco das ações | útil | manter |

## 5. Mixer (Campo de matérias)

| Controle | Visual / texto | Função | Pertinência | Recomendação |
|---|---|---|---|---|
| Título "CAMPO DE MATÉRIAS · ⟨módulos⟩" | **encoberto** pelos botões TRAMA e VARIAÇÃO ("CAMPO DE M…") | — | — | **corrigido** (os botões descem para baixo do título) |
| TRAMA · LIGADA / VARIAÇÃO · LIGADA | legíveis | ligam a trama do céu e a variação do mixer | centrais | manter |
| PRES (8) + ganho (8) | os valores do ganho aparecem cortados ("1.00 …") | presença no Campo e ganho do stem | pertinentes; a VARIAÇÃO move o brilho de cada estrela por cima deles, sem mexer nos faders | **corrigido** (caixa de valor); melhorar: mostrar o brilho atual da estrela no fader |
| MUTE / SOLO (8+8) | legíveis | silenciar / solar stem | manter |
| ORIGEM ⇄ VARIAÇÃO + "0.00 SEED" | o trilho do slider quase não aparece | quanto a seed varia o módulo em foco | pertinente | melhorar o trilho (decisão visual do autor) |
| CORPO / BRILHO / FRASE | legíveis | 3 controles do módulo em foco | pertinentes | manter |

## 6. Código sem uso

- 16 **células** (`cellButtons_`): criadas, atualizadas a cada quadro (HOLD/
  RELEASE, SEGURO), mas **sempre invisíveis** (`setVisible(false)`). Já
  estavam na auditoria de 28/10 (item 3.5). Decisão do autor: remover ou
  reaproveitar.

## 7. O que foi corrigido nesta sessão

1. DESFAZER e CAPTURAR inteiros.
2. Fonte única nos gestos ABRIR / RASGAR / REPARAR.
3. A dica do nó em foco e o tutorial usam os nomes dos botões (ABRIR,
   RASGAR, REPARAR), não os antigos (MUTATE, RECALL).
4. O título do mixer não é mais coberto por TRAMA / VARIAÇÃO.
5. As caixas de valor do ganho mostram o número inteiro (o sufixo " MIX" não
   cabia; a dica do fader continua dizendo o que ele é).
6. Conferido numa instância isolada (HOME temporário, saída muda), por
   captura de tela: todos os itens acima aparecem corrigidos.

7. **CODA EM BREVE** no cabeçalho quando se grava e a prontidão da coda passa
   de 85%. A coda automática não depende só da seed: pesa sobretudo a pausa
   do performer (56% — cerca de 100 a 190 tempos sem tocar em nada), depois a
   idade da peça (24%) e as viradas da seed (20%) — por isso a peça das
   18:27 terminou sozinha enquanto o autor só escutava.
8. TUTORIAL com uma seção nova, "7. O CÉU · TRAMA, FORMA, TEMPO E
   MOVIMENTO", nos quatro idiomas, e a regra da coda automática explicada.

## 8. Decisões pedidas ao autor (em ordem de impacto)

1. Os dois vocabulários de relação (16 cabos fixos × trama do céu).
2. FENÔMENOS paramétricos × forças físicas.
3. DANÇA · ÓRBITA/PULSO/FIGURA-8 com a física ligada.
4. Nome das cenas (ABERTURA colide com a seção da forma) e de TEMPO SHIFT.
5. Eixo de Forças × 8 macros (redundância).
6. As 16 células invisíveis.
7. Linha de estado em duas linhas (música / máquina).

## 9. Estado em 3/11/2026 e recomendações para o que resta

Conferido no código: **1, 2 e 3 já estavam feitos** e a lista de tarefas não
tinha sido atualizada. Os cabos desenhados entram na trama par a par desde
29/10 (`bada31b`, `docs/CODIGO_SUBAPROVEITADO_2026-10-29.md` §3.1): a
intensidade pesa a chance do par entrar em foco (até 2,5×), o fenômeno
escolhe o ato musical do par e vira uma força física entre os dois. A DANÇA
virou família de movimento em 30/10. O 4 (nome da cena) foi resolvido hoje
(NASCENTE).

Recomendações para o que resta — **aprovadas pelo autor ("ok", 3/11) e aplicadas no mesmo dia** (detalhe na tabela seguinte):

| # | Item | Recomendação |
|---|---|---|
| R1 | Os dois vocabulários na tela | com a trama, o rótulo do cabo mostra também o ato que ele pede ("MATÉRIA ACID → GENDYN · ATRITO"); os nomes dos cabos ficam |
| R2 | TEMPO SHIFT (é um recorte) | DESLOCAR / DISPLACE / DÉCALER / DESPLAZAR |
| R3 | "SIL", "AUD", "PREP", "REJ" nas caixas laterais | MUDO, SOA, PRONTO, RECUSADO (e nas outras três línguas) |
| R4 | Eixo de Forças × 8 macros | manter os dois (o eixo é o gesto, os botões o ajuste fino); o tutorial diz que são os mesmos valores |
| R5 | As 16 células invisíveis | remover (código morto, atualizado a cada quadro) |
| R6 | Linha de estado | duas linhas: música (forma, tempo, swing, agrupamento, deriva, eclipse) em cima; máquina (stems, atrasos, CPU, limite) embaixo; nenhum dado novo |
| R7 | O "grão" do token do MUTATE (não soa mais com a trama) | vira o empurrão físico que ABRIR dá na estrela em foco: o gesto se vê na Constelação e se ouve no pan |
| R8 | Andamento | o andamento efetivo ao lado do BPM ("120 → 124") |

Como ficou (3/11): R1 — `PerformanceSkyDirector::naturalKind` ficou público
(mesmo cálculo; som idêntico) e o rótulo do cabo ganha " · ATO" com a trama;
R2 — botão, dica, tutorial e eco da ação nos 4 idiomas (o nome interno do
gesto no trace da tomada segue `TEMPO SHIFT`, para não quebrar a leitura de
tomadas antigas); R3 — PRONTO / MUDO / SOA / RECUSADO (4 idiomas); R4 — uma
frase na seção 4 do tutorial (4 idiomas); R5 — as 16 células, a zona "16
CÉLULAS" e as funções que só elas chamavam removidas; R6 — música na
primeira linha, máquina (corpos, stems, limite, perfil, atrasos, Modal, CPU)
na segunda, menor; R7 — o grão empurra a estrela em foco (ou todas, cada
uma num ângulo áureo) com ≈ 3 ataques de força; teste novo no motor; R8 — o
rótulo TEMPO / BPM mostra "→ 124" quando a trama faz o tempo respirar.
**Não conferido na tela** (sem tela virtual na máquina; abrir na tela do
autor arriscaria roubar o foco do teclado): conferência do autor.

### 9.1 O que restava (3/11, noite)

- **Feito:** a faixa da BIBLIOTECA mostra só o que se faz ali ("28
  ESTRELAS · ESCOLHA UMA ESTRELA E CLIQUE AQUI…" / "OU ARRASTE DAQUI E SOLTE
  SOBRE UMA ESTRELA"); as contagens de bastidor (módulos, em auditoria,
  candidato) foram para a janela SOBRE.
- **Proposta A — trilho ORIGEM ⇄ VARIAÇÃO** (hoje verde-escuro sobre
  verde-escuro, quase invisível): (A1, recomendada) o trecho de 0 até o valor
  em âmbar (a cor da VARIAÇÃO no mixer), o resto num verde médio com contorno
  fino, trilho de ~6 px; (A2, opcional) três marcas finas sob o trilho em
  0 · 0,5 · 1.
- **Proposta B — o Eixo de Forças mostra o céu** (hoje só os dois pontos):
  (B1, recomendada) um anel fino em volta de cada ponto — o raio respira com
  a energia do movimento das estrelas, a cor vai do azul ao âmbar com a
  tensão harmônica do céu; nenhum texto novo, só com a trama; (B2) um rastro
  de ~2 s quando o próprio céu move os pontos; (B3) uma faixa discreta no
  fundo com a seção da forma.

Ambas aguardam o autor.

