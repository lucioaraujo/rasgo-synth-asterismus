# Código sem uso e subaproveitado — análise para o modelo do céu (2026-10-29)

Pedido do autor: "verifique o que há no código que não está sendo usado ou
está subaproveitado, em seguida analise para ver como podemos utilizar esse
trabalho de codagem melhor ou reescrevê-lo segundo as novas ideias."

Método: (1) para cada cabeçalho de `include/rasgo/performance/`, quem o inclui
fora dos testes; (2) para cada função definida, quantas vezes o nome aparece
no produto (`include/` + `src/`, sem comentários e sem testes); (3) para cada
DSP copiado em `vendor/studio-dsp/`, se o produto o usa; (4) leitura dirigida
dos caminhos de sinal na ecologia (`PerformanceActorEcology.hpp`) e na
interface, para achar o que é calculado e não chega ao som nem à tela. Nada
foi apagado ou alterado nesta análise.

## 1. Não usado no produto

| O quê | Onde | Estado | Valor | Proposta |
|---|---|---|---|---|
| **TubaBreathVoice** — banco modal grave excitado por sopro | `include/.../TubaBreathVoice.hpp` (48 l.), teste próprio, proveniência em `PROVENIENCIA_TUBA_BREATH.md` | só o teste o inclui | uma voz pronta e auditada | **15ª estrela** (família Tom, registro grave; sopro modulado pela física — velocidade do corpo = pressão do sopro) |
| **StudioTerritoryImporter** — leitor do manifesto RSTM 1/2 do Studio | `include/.../StudioTerritoryImporter.hpp` (142 l.) | só o teste | ponte entre instrumentos sem incluir código do Studio | **abrir um território exportado pelo Studio como asterismo** (estrelas, presenças, relações) — só lê o arquivo; o Studio continua intocado |
| **7 DSP copiados do Studio**: `BreathExciter`, `ColoredNoise`, `LfoBank`, `MoogFilter`, `MultibandFilter`, `PolivoksFilter`, `SallenKeyFilter` | `vendor/studio-dsp/src/dsp/` | nenhum uso | filtros e fontes prontos, com proveniência | **filtros por estrela movidos pela física** (velocidade → corte, spin → ressonância, distância ao centro → brilho; Polivoks no contraste); `ColoredNoise` no estalo dos choques do bilhar; `BreathExciter` para a Tuba |
| **Documento de sessão** — `serializeSessionDocument` / `parseSessionDocument` | `PerformanceSessionDocument.hpp` (297 l.), testado | o app **nunca salva nem abre** uma sessão | cenas capturadas, posições desenhadas, seed, trama: tudo se perde ao fechar | **salvar/abrir sessão** e salvamento automático da última; resolve também o achado de 21h50 (posições 5–8 empilhadas no motor), porque a sessão aberta envia as posições |
| **Gestos sem botão**: `GridCue` (gesto quantizado na grade), `Interrupt` | motor (`PerformanceSessionEngine.hpp`) | o motor trata; a interface nunca envia | GRID existia na interface antiga | **GRID quantizado ao agrupamento do compasso atual** (3+3+2…) ou à contagem da estrela em foco |
| **16 células invisíveis** (SEED → 1–4, LIGA 1–2…, CORPO 1–4, SEED · ABRE/RASGA/REPARA/PAUSA) | `PerformanceApp.cpp` | criadas e atualizadas a cada quadro, sempre ocultas; as funções (`triggerCell`) existem | ações reais escondidas | decisão do autor (já na auditoria da interface): remover, ou reaproveitar "SEED → n" como **acender/apagar uma estrela** |
| Funções nunca chamadas | `removeActor`, `setSessionSnapshot`, `recordingShouldRemainOpen`, `makeWithWalsh`, `assignmentFor`, `primaryModuleId`, `cumulativeDesk()`, `populationPhenomenonLabel` | — | pequenas | revisar junto das propostas acima; várias servem à sessão salva |

## 2. Calculado e jogado fora (custo sem som)

> **Feito em 29/10 (item 1 da ordem):** a fratura rasga o stem da estrela
> atingida. Correção do que está escrito abaixo: `process()` não devolve
> áudio, devolve um **nível** 0–1 (cicatriz + impulso de ruptura) — a camada
> provisória o somava como deslocamento à onda. Agora, com a trama ligada,
> esse nível (a) dobra a onda do stem atingido e (b) abre cortes curtos e
> irregulares (até ~25/s, de 3 a 37 ms, rampa de ~0,25 ms); cada fibra nova
> que se rompe faz a estrela atacar (estalo). Alvo: a estrela em foco no
> RASGAR, enquanto houver cicatriz; sem alvo, papéis RASGO (forte) e ATRITO
> (leve). REPARAR baixa a cicatriz. Medido (seed 3570930745): nível 0,58 logo
> após o gesto, 0,09 em 2 s; no stem rasgado, 2 buracos curtos dentro do som
> contra 0 no gêmeo intacto (o stem é 76% silêncio — estrela esparsa). Trama
> desligada: som idêntico (hash). Teste: `testFractureTearsTheStruckStem`.

### 2.1 A fratura (RasgoFracture) não soa — o achado principal

`fracture_.process()` roda **a cada amostra** e responde ao RASGAR
(`applyStress`), à pressão de risco/tensão/atrito e ao reparo. Mas o sinal
(`fractureSignal`) só entra na **camada provisória de senoides**
(`provisionalBody`), que desde 28/10 só soa para corpos sem estrela — isto
é, nunca, num build com as quatro coortes. A ruptura que dá nome ao
instrumento é calculada e descartada; só a cicatriz (um número) chega ao
cabeçalho (CICATRIZ).

Proposta (reescrita segundo o céu): a fratura vira um **acontecimento do
céu**, não um zumbido de fundo — (a) a estrela atingida pelo RASGAR recebe o
sinal da fratura no seu próprio stem, articulado pelos seus ataques; (b) a
cicatriz vira força física: a estrela rasgada ganha massa/atrito, ou um
"vazio" no campo que repele as outras; (c) REPARAR recompõe. Alternativa
mínima: se o autor preferir que a fratura não soe, parar de processá-la
(economia de CPU por amostra).

### 2.2 A camada provisória e o fallback por posição

~160 linhas em `renderStems` (senoides por papel, "colisão", "órbita",
imitações de MODAL/GALO/ATROPELO/DEJA VU por índice) só soam sem céu. Hoje
são código morto no build real. Propostas: manter como modo de segurança
documentado, ou retirar e deixar o build sem coortes falhar cedo. As forças
que só ela usava diretamente — `relationalSeedDeviation_` (desvio de seed) e
`relationalSilence_` (silêncio relacional) — podem ser **reencaminhadas ao
céu**: silêncio → pausas na trama (estrelas que se calam juntas); desvio →
qual transformação o motivo recebe.

## 3. Subaproveitado

### 3.1 As 16 relações fixas e os 23 fenômenos — informação de par perdida

> **Feito em 29/10 (item 2 da ordem), com a trama ligada:** a cada intervalo
> de controle a ecologia entrega ao diretor cada cabo (par, intensidade,
> fenômeno ligado). (a) A intensidade multiplica a chance do par entrar em
> foco por até 2,5× (a direção desenhada pesa o dobro da inversa); (b) o
> fenômeno escolhe o ato do par — ATRAIR, PARALELO, CONVERGIR, EQUILÍBRIO,
> RESSOAR → pedal; REPELIR → atrito; ORBITAR, ÓRBITA ELÍPTICA, OSCILAR, TROCAR
> → resposta; DANÇA BARICÊNTRICA, ENCONTRO, FUSIONAR, TRAVAR, SINCRONIZAR →
> contágio; SOBREVOAR, PERSEGUIR, ESPIRAIS → sombra; ESPALHAR, FUGIR,
> DIVERGIR → hocket; LIBERAR → o ato natural; (c) o fenômeno vira força
> física entre os dois corpos (elástico curto, repulsão, giro tangencial,
> elástico longo, mola rígida, perseguição/fuga, alinhamento de
> velocidades). Medido: com um cabo forte (ESPALHAR) entre dois corpos, o
> par fica em foco em 24 de 221 focos (11%), contra ~1,8% por acaso. As cinco
> pressões globais continuam como antes (o som com a trama desligada é
> idêntico — hash). Testes no diretor: todo fenômeno pede um ato tocável;
> ATRAIR aproxima, REPELIR afasta, ORBITAR gira o par em torno do meio.

Cada relação fixa (MATÉRIA ACID → GENDYN, TEMPO GENDYN → DRUMS…) com o seu
fenômeno (ATRAIR, REPELIR, ORBITAR, ESPIRAL, OSCILAR, SINCRONIZAR, TROCAR…)
gera forças **por par** (`PerformanceCompositionalForces`: acoplamento de
pulso, fratura temporal, troca e fusão de matéria, circulação espacial,
tensão, memória, desvio, silêncio). A ecologia **soma tudo em cinco números
globais** (pressões de tempo, matéria, atrito, espaço, memória) que entram no
céu. Quem se relaciona com quem — exatamente o que a trama precisa — se perde
no funil. E agora existem dois vocabulários de relação na tela.

Proposta (a mais rica desta análise — unifica os dois mundos):

- a intensidade de cada cabo fixo vira o **peso com que o par entra em foco**
  na trama do diretor;
- cada fenômeno vira uma **força física no par**: ATRAIR = elástico curto;
  REPELIR = cargas iguais / bolhas maiores; ORBITAR e ESPIRAL = campo
  magnético local; OSCILAR = elástico longo com pouco atrito; SINCRONIZAR =
  acoplamento forte no coro de fases (Kuramoto); DIVERGIR = antifase;
  PERSEGUIR/FUGIR = forças assimétricas; TRAVAR = mola rígida;
- cada fenômeno também escolhe o **ato musical** preferido do par (SINCRONIZAR
  → contágio; DIVERGIR → hocket; TROCAR → resposta…).

Assim, o cabo desenhado pelo performer comanda, ao mesmo tempo, o movimento
e a música daquele par.

### 3.2 Órbitas paramétricas e fenômenos de população

`PerformanceMotionGeometry` (ÓRBITA/PULSO/FIGURA-8) e os fenômenos de
população (ALINHAR, AGRUPAR, ESPALHAR, GIRAR…) ainda movem a posição viva da
ecologia — usada para distâncias e relações de áudio — enquanto a tela, com
a trama, mostra a física. Dois movimentos para o mesmo corpo. Proposta:
com a trama ligada, a ecologia usa as posições físicas do diretor (o corpo na
tela é o corpo no som, como pede o comentário original); ÓRBITA/PULSO/
FIGURA-8 e os fenômenos de população viram **famílias de força** na
coreografia (ex.: AGRUPAR = gravidade forte; ESPALHAR = bolhas grandes;
GIRAR = campo magnético global).

> **Tentado em 29/10, 23h35, e revertido.** Com a trama, a ecologia passou a
> usar as posições físicas do céu (`baseLivePosition`). Três testes falharam:
> (a) "campos diferentes produzem uma trajetória A/B audível" — ÓRBITA,
> PULSO e FIGURA-8 (o botão DANÇA) deixam de mudar o som quando a física
> assume as posições; (b) o Galo ficou inaudível no teste de estabilidade de
> um minuto — as distâncias novas mudam a ecologia (pressões, relações) de
> modos não medidos. Conclusão: este passo depende da decisão sobre o DANÇA
> (auditoria da interface, §8.3) — transformar ÓRBITA/PULSO/FIGURA-8 em
> famílias de força **antes** de entregar as posições à ecologia, e medir o
> Galo (e as demais estrelas) com as distâncias novas. Nada disso ficou no
> código.

### 3.3 O espaço do som não usa as posições

O estéreo é um campo global (ESPAÇO), não por estrela. Com stems mono por
estrela e posições físicas por estrela, falta pouco para **pan e profundidade
por estrela** (x → pan; distância ao centro → reverberação/filtro). Decisão
do autor (muda o som).

### 3.4 Biblioteca: 29 módulos, 14 tocam

15 entradas em auditoria (Core, Texture, Tuba/Breath, Bass Fallback,
Harmonaig, Sieve, Bloom, Hocket, Spectral Object, Polymeter, Bluff Signal,
Footnote, Rupture, Thermal Static, mais um candidato realtime). Duas já têm
caminho curto aqui: **Tuba/Breath** (voz pronta, §1) e **Sieve/Polymeter/
Hocket** — conceitos que o céu já implementa (peneiras, contagens, hocket) e
podem virar estrelas de **controle** em vez de vozes.

## 4. Ordem proposta

1. **Fratura no stem da estrela rasgada** (§2.1) — recupera a identidade
   RASGO e tira um custo inútil.
2. **Relações fixas e fenômenos → pesos e forças do céu** (§3.1).
3. **Salvar/abrir sessão** (§1) — e com ela as posições 5–8 corretas.
4. **Ecologia com as posições físicas** (§3.2) e **pan por estrela** (§3.3).
5. **Tuba como 15ª estrela** e **filtros vendorados movidos pela física**
   (§1).
6. **Importar território do Studio** (§1).
7. Limpeza: camada provisória (§2.2), células (§1), funções sem chamada.

Itens 1, 2, 4 e 5 mudam o som (com a trama ligada, ou também desligada) —
cada um com A/B e escuta do autor, um de cada vez.
