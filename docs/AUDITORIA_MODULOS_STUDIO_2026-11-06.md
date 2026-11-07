# Auditoria — os módulos do Studio que o Asterismus ainda não tem (2026-11-06)

Pergunta do autor: "no studio há mais de 80 módulos, por que no asterismus há
só uns 30?"; pedido: "faça a verificação dos módulos ainda não implementados
que são do studio".

**Método.** Leitura do código do Rasgo Synth Studio
(`RASGO_SYNTH_STUDIO/rasgo-synth-core/src/`, só lido, nada mudado), comparado
com o céu do Asterismus (`kPerformanceSkyStars`, 28 estrelas + MODAL) e com o
que já foi copiado para `vendor/studio-dsp/` (29 arquivos). O "risco" de cada
arquivo é uma contagem de marcas que não cabem no callback de áudio
(`std::vector`, `std::string`, `std::random`, `sstream`, `new`, `push_back`,
`resize`, `std::map`, `std::function`): **é um indício, não um veredito** — cada
candidata pede leitura, cópia fixada com proveniência, adaptação, teste e
escuta (o caminho de todas as estrelas desde 30/10).

## 1. De onde vêm os "mais de 80"

| No Studio | Quantos |
|---|---|
| Peças de DSP (`dsp/`) | 69 |
| Sequenciador e composição (`sequencer/`) | 15 |
| **Total** | **84** |
| Módulos que o músico liga ou desliga (`StudioModule`) | 34 |
| Organismos com stem próprio (`OrganismStem`) | 23 |

Os 84 não são 84 vozes: muitos são filtros, saturadores, glitches, limitadores
e ferramentas de composição. O Asterismus trouxe **as vozes**: as 28 estrelas
cobrem 24 dos 34 módulos do Studio (bumbo, caixa e chimbal dentro da DRUMS) e
22 dos 23 organismos (o que falta é "outro"). Ficam de fora 10 módulos:
BINAURAL, VECTOR, ENVELOPE PULSE, GRANULAR, NOISE BED, COWBELL, CLAP, TOMS,
CYMBAL e MARACAS.

## 2. Vozes que podem virar estrelas

| Peça do Studio | Linhas | Risco | O que é | Caminho |
|---|---|---|---|---|
| `DrumVoices.hpp` — COWBELL, CLAP, CYMBAL, MARACAS | (já copiado) | — | percussões que já estão no `vendor/` e não tocam (a DRUMS usa só bumbo, caixa e chimbal); os TOMS não têm classe própria nesse arquivo | **o mais barato**: estrelas de pulso novas ou a DRUMS ganhando kit pela seed |
| `AY8910Chip.hpp` | 149 | 0 | o chip de som de 8 bits (3 tons + ruído + envelope) | estrela de tom/pulso |
| `MorphOscillator.hpp` | 80 | 0 | oscilador que passa entre formas de onda | estrela de tom |
| `RosslerMap.hpp`, `ChaosMaps.hpp` | 98, 424 | 0, 1 | atratores caóticos como fonte | estrela de textura (ou fonte de controle) |
| `EnvelopeOscillator.hpp`, `EnvelopeLoopPulse.hpp` | 57, 56 | 0, 0 | o ENVELOPE PULSE do Studio | estrela de pulso |
| `GranularCloud.hpp` | 118 | 2 | a nuvem granular (o GRANULAR do Studio) | estrela de textura; adaptar a memória |
| `GrainyNoise.hpp`, `NoiseGateGenerator.hpp` | 45, 66 | 1, 1 | o NOISE BED | estrela de textura |
| `OchdBank.hpp` | 67 | 1 | banco de osciladores | estrela de tom |
| `KSRandomizerCore.hpp` | 50 | 2 | corda pinçada (Karplus-Strong) com acaso | estrela de tom |
| `SpectralGhost.hpp` | 166 | 12 | eco espectral | precisa de adaptação séria (aloca) |
| `MelodyVoicingBank.hpp`, `BassPhraseTimbre.hpp` | 192, 106 | 3, 1 | vozes de melodia e de baixo | estrelas de voz/tom |
| BINAURAL, VECTOR | — | — | escritos dentro do `PieceRenderer` (10 731 linhas), sem voz separável | reescrever a partir do conceito (como HARMONAIG, BLOOM e CORE em 2/11) |

Com elas, o céu passaria de 28 para cerca de 40 estrelas.

## 3. Processadores (não são estrelas)

Filtros (`MoogFilter`, `PolivoksFilter`, `SallenKeyFilter`, `MultibandFilter` —
os quatro **já copiados**), `PolivoksVCA`, saturadores (`TubeSaturator`,
`TapeSaturator`, `DiodeShaper`), `RingModulator`, `FractureFolder`,
`LeibnizRectifier`, `LagProcessor`, `Effects`. Não soam sozinhos: podem virar
**a matéria de uma estrela** (um filtro ou saturador escolhido pela seed para
cada corpo) ou um efeito do céu. Quase todos com risco 0–1.

## 4. Glitch e montagem (podem virar gestos)

`SampleHoldGlitch`, `ParenthesisGlitch`, `MontageEditCut`, `ApocryphalTear`,
`DissonanceTreatment`, `BernoulliGate` (risco 1–5). No Asterismus cabem como
**gestos** — do músico e do céu (o item E da auditoria da seed: hoje o céu só
faz STUTTER e DESLOCAR).

## 5. Composição (o sequenciador do Studio)

`NeoRiemannian` (61 linhas: transformações P, L, R entre acordes),
`HarmonicWanderer`, `MicrotonalTuning`, `Scales` (159, risco 0), `Euclidean`,
`MelodicContourMemory` (75, risco 0), `CoherenceCollapse`, `ArcGenerator`,
`NarrativeArc`, `GenerativeArc`, `ArcTemplates`, `StepSequencer`. São **arte
anterior** direta das regiões harmônicas (item D: o NeoRiemannian daria
progressões em vez de saltos de região), das escalas (o Asterismus tem 6), dos
ritmos euclidianos e das formas (item C). Não são estrelas: melhoram a
composição de todas.

## 6. Infraestrutura

`DcBlocker`, `Envelope`, `Oscillator`, `LookaheadLimiter`, `TruePeakDetector`,
`SignalLeveler`, `LinkedOverloadProtector`, `FixedPointOverflow`,
`DeliveryPeakGuard`, `ModulationBus`, `LfoBank`: o Asterismus já tem o
equivalente (master, medição, limitador) ou não precisa.

## 7. Por que não entraram ainda

O Studio renderiza offline; o Asterismus toca ao vivo, num callback de 10,67 ms
com 8 corpos. Cada estrela nova precisa caber nesse orçamento (hoje ~2 ms em
média, pior 4,8 ms), não alocar, ser determinística pela seed, ter nível de
referência medido (`PerformanceStarLevels.hpp`) e passar pela escuta. Por isso
o critério de 30/10: uma estrela de cada vez.

## 8. Recomendação (decisão do autor)

1. **COWBELL, CLAP, CYMBAL, MARACAS** — já estão no repositório; um kit de
   percussão pela seed é o ganho mais rápido.
2. **AY8910, MORPH, RÖSSLER, ENVELOPE PULSE** — risco 0, curtos: quatro
   estrelas novas.
3. **NeoRiemannian e Scales** para a harmonia (progressões, mais escalas).
4. Glitches como gestos do céu.
5. Depois: GRANULAR, NOISE BED, OCHD, KS; por último SPECTRAL GHOST, BINAURAL e
   VECTOR (reescrita).

Nada disso é necessário para publicar a 0.2.0: pode ser a 0.3.

## 9. Feito na 0.3 (6/11) e correções desta auditoria

- **Passo 1:** o kit da DRUMS pela seed (COWBELL, PALMAS, PRATO, MARACAS).
- **Passo 2:** AY-8910, MORPH, RÖSSLER e ENVELOPE PULSE como estrelas
  (cópias fixadas em `vendor/`, PROVENANCE.md), fora do sorteio das 8 da
  seed: na BIBLIOTECA e entre as convidadas do contraste.
- **Passo 3:** NeoRiemannian (progressões P/L/R) e 16 modos de contraste do
  repertório do `Scales.hpp`.
- **Passo 5:** GRANULAR e TURING, **reescritas a partir do conceito**
  (`include/rasgo/performance/PerformanceStudioVoices.hpp`): as fontes do
  Studio usam distribuições de `<random>`, cujo resultado muda de um
  compilador para outro — a mesma seed soaria diferente no Linux, no Windows
  e no macOS; aqui o acaso é um xorshift próprio.
- As máscaras por estrela passaram a 64 bits (o céu tem 34 estrelas).

**Correções:** na tabela do §2, `OchdBank` não é voz — é um banco de oito
LFOs lentos (fonte de modulação); `KSRandomizerCore` não é corda
Karplus-Strong — é o registrador de bits aleatórios do "Turing Machine"
(virou a estrela TURING); `GrainyNoise`/`NoiseGateGenerator` repetem a
estrela TEXTURE (cama de ruído) e ficaram de fora. Faltam: os glitches como
gestos do céu (passo 4), SPECTRAL GHOST, BINAURAL e VECTOR.

## 10. Os últimos (6/11, noite): SPECTRAL GHOST e VECTOR; BINAURAL em aberto

O passo 4 (os cortes) já entrou em `4252cdc`. Agora, reescritas a partir do
conceito em `PerformanceStudioVoices.hpp` (acaso xorshift, nada alocado no
callback), fora do sorteio das 8 da seed, na BIBLIOTECA e entre as
convidadas — o céu passa a 36 estrelas:

- **SPECTRAL GHOST** — o `SpectralGhost.hpp` do Studio (FFT 1024, Hann
  periódica, 50 % de sobreposição) refeito sem `std::vector`/`deque` no
  callback. **O fantasma ouve o céu:** a fonte é a soma das estrelas acesas
  na amostra anterior (`skyHeard_`, no `advance`), mais uma frase própria
  que recua quando há céu (para soar sozinho). Diferenças em relação ao
  Studio: as fases avançam com a faixa (DERIVA 0 = fantasma coerente, 1 =
  borrão), a magnitude pode demorar (MEMÓRIA, até ~0,4 s) e escurecer
  (BRILHO), e o DC sai. Teste novo: com ACID e DRUMS acesos ele soa outra
  coisa do que sozinho, deterministicamente.
- **VECTOR** — a fase 76 do `PieceRenderer` (Prophet VS): quatro cantos
  (onda estocástica de 8 pontos — o GENDYN de Xenakis em versão
  determinística —, Walsh, Atari Punk, ruído rosa de Kellet) misturados
  bilinearmente por um ponto X/Y. **Desvio obrigatório:** no Studio o ponto
  vinha de dois sinais caóticos da voz principal; aqui X e Y são controles
  e ÓRBITA o faz girar numa figura de Lissajous lenta (razão das voltas
  pela seed). Bloqueador de DC (o Atari Punk é unipolar).

Níveis (sonda `novas`, corpo 2, 12 seeds × 60 s, levados a −49,1 dB):
GHOST −5,2 dB, VECTOR +3,9 dB. Custo (céu sozinho com o ACID, bloco de
512): GRANULAR 225 µs, GHOST 269 µs, VECTOR 176 µs.

**BINAURAL fica para o autor decidir:** o batimento binaural é uma
diferença de frequência *entre L e R*; as estrelas do Asterismus são mono
e soam onde estão no campo (o pan vem da posição). Fazê-lo pede uma estrela
com saída estéreo própria — uma mudança de arquitetura do caminho das
estrelas, não só uma voz nova.

**Decisão do autor (7/11): opção B, na 0.4.** O BINAURAL entra como
**camada do céu**, não como estrela — no padrão da fita e dos cortes
(`PerformanceSkyCuts`, a camada de fita): uma sombra binaural sobre a frase
do céu (dois senos, um por canal, com alguns Hz de diferença que andam com
o caos), nas partes que a seed escolher, em nível baixo. Mantém a regra da
Constelação (cada estrela soa onde está, mono) e é fiel ao Studio, onde ele
também era camada aditiva. Só funciona de fone; em caixas vira batimento
comum — documentar assim, sem promessas de "ondas cerebrais". Fica fora da
0.3 para não somar mais uma camada a uma versão ainda não ouvida.
Alternativas recusadas: A (estrela estéreo — mudaria o caminho de todas as
estrelas, e o stem mono do corpo não guardaria o efeito) e C (ficar de fora).

**Feito (7/11, para a 0.4):** `PerformanceSkyBinaural.hpp` — dois senos, um
por canal; portadora = a raiz harmônica do céu (centro tonal + região),
dobrada para 160–320 Hz ou uma oitava acima (seed), deslizando ~0,6 s;
diferença de 3–10 Hz (seed) que anda ±50 % em alvos novos a cada 2–5 s,
com acaso xorshift próprio. No diretor, 30 % de chance por tipo de parte
(intensidade 0,5–1; medido: 30,5 % em 300 seeds), fluxos novos da seed (os
sorteios antigos não mudam). No motor, aditivo antes dos cortes e do master,
entrando e saindo em 2 s, multiplicado pelo ganho que cala SILÊNCIO e STOP;
só em saída estéreo e pela largura do ESPAÇO (fechado, os dois canais recebem o mesmo — um batimento comum no centro; aberto, a pulsação binaural), como os envios. Nível 0,0034 (~20 dB abaixo da peça; a 0,0012 ficava
~29 dB abaixo). Teste `test_performance_sky_binaural.cpp` (L em 220 Hz, R
5,95 Hz acima). **Não está nos stems** (é do céu, como a fita e os cortes).
