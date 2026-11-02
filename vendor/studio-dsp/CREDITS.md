# Créditos e linhagens — Rasgo / Rasgo Synth

Revisão: 3 de agosto de 2026.

Este documento registra autoria, influências conceituais e referências
históricas do Rasgo e, especificamente, do software Rasgo Synth. Ele não
transfere autoria de terceiros para o projeto e não afirma que o código de uma
obra citada tenha sido incorporado. Dependências de software e suas licenças
estão separadas em [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).

## O que é Rasgo

**Rasgo** é o projeto geral criado e dirigido por **Lucio de Araujo** para
reunir todos os seus instrumentos e softwares musicais autorais. Cada obra pode
conservar nome e identidade próprios dentro desse conjunto. **Rasgo Synth** é
um desses softwares, não o nome do projeto inteiro.

Essa dimensão de guarda-chuva é a forma atual e ampliada do projeto. Em sua
linhagem física, Rasgo já compreendia módulos concebidos visando ao padrão
Eurorack. Hoje, porém, Rasgo não se limita a essa realização em hardware: passou
a acolher o conjunto mais amplo da produção autoral de instrumentos e softwares
musicais de Lucio.

O hardware continua pertencendo ao horizonte de Rasgo. Lucio pretende retomar,
em outro momento, o desenvolvimento de **consoles embarcados, pedais e módulos**.
Esses caminhos são planos futuros do projeto-guarda-chuva e não devem ser
apresentados como produtos concluídos nem como parte da fase atual do Rasgo
Synth.

Entre as possibilidades previstas para uma etapa posterior está uma **versão
console embarcada do Rasgo Synth**. Ela permanece como direção de pesquisa e
desenvolvimento futuro, sem implementação ou prazo anunciados no estado atual
do projeto.

Essa definição não mistura Rasgo com as outras áreas de criação de Lucio e não
incorpora automaticamente obras de terceiros. Um trabalho de upgrade ou
manutenção também não passa a integrar Rasgo apenas por ter sido realizado por
Lucio quando sua autoria original pertence a outra pessoa.

## Autoria do Rasgo Synth

**Rasgo Synth** foi concebido e é mantido por **Lucio de Araujo**, dentro do
projeto Rasgo.

Copyright © 2026 Lucio de Araujo.

O código específico do Rasgo Synth, sua arquitetura, integração, sistema de
composição por seed, motor de renderização, interface e implementações DSP
atuais foram escritos para este projeto. Os créditos abaixo reconhecem ideias,
obras, instrumentos, circuitos, algoritmos e tradições que ajudaram a formar o
vocabulário do instrumento; eles não indicam cópia literal de código, firmware,
esquemas, amostras ou recursos gráficos.

## Antitotem — precursor e patrono

**Antitotem** é obra de **Lucio de Araujo**. Ele antecede a formalização do
Rasgo e é seu precursor histórico, mas não permaneceu fora do projeto: foi
posteriormente incorporado ao próprio Rasgo.

Dentro dessa linhagem, Antitotem ocupa uma posição semelhante à de um
**patrono**: é a obra fundadora que precede, inspira e simbolicamente ampara o
desenvolvimento posterior do projeto. Portanto, deve ser apresentado ao mesmo
tempo como precursor de Rasgo e como parte integrante de Rasgo — nunca como
obra de terceiro ou mera referência externa.

Neste documento, **Rasgo em hardware** designa o conjunto de módulos físicos
autorais daquela fase, que já eram concebidos visando ao padrão Eurorack. A
expressão não designa um único aparelho chamado “Rasgo Synth de hardware” e não
descreve apenas uma intenção futura.

### Assistência de desenvolvimento

A reescrita integral do código e numerosas implementações foram realizadas com
o **Anthropic Claude** como ferramenta de desenvolvimento, sob direção de Lucio
de Araujo. A ferramenta é registrada como assistência, não como autora dos
projetos, circuitos ou algoritmos citados abaixo e não como biblioteca
incorporada ao executável.

## Origem conceitual do Rasgo Synth

O desenvolvimento inicial do Rasgo Synth, como uma obra dentro de Rasgo,
reuniu duas referências principais de naturezas diferentes:

- **Antitotem e os módulos físicos do Rasgo**, de Lucio de Araujo: o primeiro é
  precursor, patrono e parte incorporada à própria linhagem; os módulos já eram
  concebidos visando ao padrão Eurorack e constituem sua origem material,
  estética e autoral;
- **[Kosmische](https://github.com/zrnsm/kosmische)**, de **Nicolas Steven
  Miller** (`zrnsm`), distribuído sob GNU GPL v3: projeto externo usado como
  referência conceitual para o instrumento generativo portátil, os mapas
  caóticos e caminhos de síntese que motivaram a investigação inicial.

Kosmische é um projeto independente, de outro autor. O Rasgo Synth não é uma
continuação autoral de Kosmische, e Kosmische não integra Rasgo. O código do
Rasgo Synth foi reescrito e depois ampliado com numerosas implementações sem
precedente naquele projeto.

## Instrumentos, circuitos e módulos de referência

As implementações do Rasgo Synth reproduzem ou reinterpretam comportamentos
musicais por código original. Salvo indicação expressa em
`THIRD_PARTY_NOTICES.md`, nenhum firmware ou código dos projetos desta tabela é
incluído no repositório.

| Área ou módulo do Rasgo Synth | Crédito e referência | Natureza da relação |
| --- | --- | --- |
| `AtariPunkConsole` | **Forrest M. Mims III** — *Stepped Tone Generator*, posteriormente popularizado como Atari Punk Console | Reimplementação comportamental original do oscilador astável/monoestável |
| Circuitos CMOS/Lunetta | **Stanley Lunetta** e a comunidade DIY de instrumentos CMOS | Linhagem técnica e estética |
| Percussão eletrônica | **Roland Corporation**, esquemas de serviço da TR-808; análises públicas de **Eric Archer** | Vozes sintetizadas originalmente e inspiradas em comportamentos da TR-808; não são amostras nem réplicas completas dos circuitos |
| `BuchlaRandomSource` | **Don Buchla** — Music Easel/Model 208 Random Voltage Source; redesenho de referência de **Scott Stites** | Reimplementação conceitual original |
| `SignalLeveler` e `LagProcessor` | **Buchla Associates / Don Buchla**, documentos de Dual Signal Leveler e Dual Control Voltage Integrator | Interpretação funcional original de documentos de circuito |
| `KSRandomizerCore` | **Scott Stites / KS Synthesis** — 4006 Randomizer Core | Reimplementação conceitual original |
| `Rungler` | **Rob Hordijk** — Rungler/Benjolin | Reimplementação conceitual original |
| Sequenciamento com repetição probabilística | **Tom Whitwell / Music Thing Modular** — [Turing Machine](https://github.com/TomWhitwell/TuringMachine) | Lógica original inspirada no equilíbrio entre sequência travada e mutação |
| `PolyrhythmDivider` | **Ken Stone / Cat Girl Synth (CGS)** — CGS22 Master Divider | Reimplementação conceitual original |
| `NoiseGateGenerator` e `GrainyNoise` | **Ray Wilson / Music From Outer Space** — Noise Cornucopia | Reimplementações conceituais originais |
| `BernoulliGate` | **Émilie Gillet / Mutable Instruments** — Branches | Reimplementação comportamental original |
| `ModalResonator` | **Émilie Gillet / Mutable Instruments** — Rings | Banco ressonante original inspirado no conceito publicado |
| `GranularCloud` | **Émilie Gillet / Mutable Instruments** — Clouds | Motor granular original inspirado no conceito publicado |
| `BreathExciter` | **Émilie Gillet / Mutable Instruments** — Elements | Excitador original inspirado na distinção entre sopro e impacto |
| Saltos melódicos probabilísticos | **Émilie Gillet / Mutable Instruments** — Marbles | Referência comparativa para distribuição de alturas; implementação original |
| `OchdBank` | **Instruō**, em colaboração com **Ben “DivKid” Wilson** — [øchd](https://www.instruomodular.com/product/ochd/) | Banco original de LFOs relacionado ao conceito de velocidades organicamente acopladas; øchd não é um produto da Mutable Instruments |
| `ColoredNoise` | **Frap Tools** — [Sapèl](https://frap.tools/products/sapel/); aproximação de ruído rosa de **Paul Kellet** | Banco original de ruídos branco, rosa, vermelho e azul |
| `MoogFilter` | **Robert Moog**, filtro ladder; **Tim Stilson** e **Julius O. Smith**, aproximação digital Stilson/Smith | Implementação independente da técnica digital documentada |
| `SallenKeyFilter` e núcleo de `PolivoksFilter` | **Hal Chamberlin** — filtro de estado variável, descrito em *Musical Applications of Microprocessors* | O algoritmo implementado é Chamberlin; não deve ser apresentado como implementação da topologia eletrônica Sallen–Key |
| Nome histórico Sallen–Key | **R. P. Sallen** e **E. L. Key** | Referência à topologia eletrônica histórica, distinta do algoritmo usado atualmente |
| `PolivoksFilter` | **Vladimir Kuzmin**, projetista do sintetizador Polivoks | Filtro digital original inspirado no caráter instável do Polivoks; não é modelo do circuito original |
| `FractureFolder` | **Don Buchla**, Buchla 259; **Serge Tcherepnin**, Serge Wave Multiplier; **Émilie Gillet / Mutable Instruments**, Plaits e Tides | Linhagem de wavefolding; implementação e extensão “fracture” originais |
| `LeibnizRectifier` | **Xaoc Devices** — [Leibniz Binary Subsystem](https://xaocdevices.com/main/drezno/), especialmente Drezno/Lipsk | Corrupção digital de bits reinterpretada em código original |
| Referência de retificação dinâmica | **Nonlinearcircuits / Andrew Fitch** — [Difference Rectifier / Neuron](https://www.nonlinearcircuits.com/blog/2013/01/difference-rectifier-neuron.html) | Referência contextual; o nome correto não é “Dynamic Rectifier” e o módulo Rasgo não deve ser descrito como port desse circuito |
| `EnvelopeOscillator` | **CBS Musical Instruments**, documento *Dual Attack Generator No. 180*; tradição Buchla/Serge de envelopes em frequência de áudio | Reimplementação conceitual original |
| `RingModulator` | **CBS Musical Instruments**, documento *Dual Ring Modulator No. 111* | Referência documental; implementação DSP original |
| `MorphOscillator` e `DiodeShaper` | Documentação DIY de **fluxmonkey** e **Beavis Audio Research** | Referências de forma e mistura por diodos; implementação DSP original |
| `Effects` em modo lo-fi | **Princeton Technology Corporation**, circuito integrado PT2399; **Pantala Labs**, Echo 2399 | Emulação comportamental original, sem código ou modelo de silício incorporado |
| `TapeSaturator` e `TubeSaturator` | Instrumentos e módulos da **Pantala Labs** como referências comparativas | Curvas DSP genéricas e originais; não são emulações oficiais dos produtos |
| `AY8910Chip` | **General Instrument** — AY-3-8910; família compatível Yamaha YM2149 | Modelo simplificado e original da arquitetura publicamente documentada |
| Autorrealimentação em frequência de áudio | **Make Noise** — Maths; tradição de módulos Serge | Referência de patch e comportamento; implementação original |
| Modulação cruzada bidirecional | **Vlad Kreimer / SOMA Laboratory** — Lyra-8 | Referência de comportamento; implementação original |
| Cascata polifônica/hocketing | **Intellijel Designs** — Shifty | Referência de registrador de deslocamento melódico; implementação original |
| Leitura endereçável de passos | **Bastl Instruments** — Popcorn | Referência de sequenciamento endereçável; implementação original |
| Síntese vetorial | **Chris Meyer** e a equipe da **Sequential Circuits** — Prophet VS; o nome “Vector Synthesis” foi proposto pelo gerente do projeto **Josh Jeffe**, segundo o [relato de Meyer](https://learningmodular.com/the-story-of-the-prophet-vs/) | Crossfade bidimensional original entre quatro fontes já existentes no Rasgo Synth |
| Pad de acorde de 4 vozes | **Jason Lim**, fundador da **Instruō** (Glasgow, Escócia) — [harmonàig](https://www.instruomodular.com/product/harmonaig/), quantizador de tensão de 4 vozes com saída raiz/3ª/5ª/7ª | Banco original de 4 osciladores/envelopes dedicado, soando o acorde real da peça como pad sustentado; não é modelagem do circuito nem porte de firmware |

Marcas e nomes de produtos são usados apenas para identificar referências
históricas ou comparativas. Rasgo Synth não é afiliado, patrocinado nem
endossado por essas pessoas ou organizações.

## Síntese, DSP, matemática e teoria musical

| Técnica usada ou estudada | Crédito |
| --- | --- |
| Chaos UGens do SuperCollider | **Lance Putnam** (2004). Serviram como referência histórica junto à análise de Kosmische; não há código desses UGens incorporado às implementações atuais |
| *The Chaos Cookbook* | **Joe Pritchard** (1992). O livro não é de Lance Putnam |
| Mapa de Hénon | **Michel Hénon** |
| Atrator de Lorenz | **Edward N. Lorenz** |
| Mapas Latoocarfian/Clifford | **Clifford A. Pickover** |
| Standard map/Chirikov map | **Boris Chirikov** |
| Atrator de Rössler | **Otto E. Rössler** |
| GENDYN e teoria de crivos (*sieves*) | **Iannis Xenakis** |
| VOSIM | Trabalho desenvolvido no Instituto de Sonologia de Utrecht, associado a **Werner Kaegi**; análise espectral por **Stan Tempelaars**. A atribuição deve separar essas contribuições em vez de tratá-las como uma coautoria genérica “Kaegi & Tempelaars, 1978” |
| PolyBLEP/antialiasing de osciladores | **Vesa Välimäki** e **Antti Huovilainen**, [“Antialiasing Oscillators in Subtractive Synthesis”](https://doi.org/10.1109/MSP.2007.323276) |
| Funções de Walsh | **Joseph L. Walsh** |
| Matrizes de Hadamard | **Jacques Hadamard** |
| Código Gray | **Frank Gray** |
| Reverberação por combs/allpasses | **Manfred R. Schroeder**. A implementação atual é do tipo Schroeder e não deve ser chamada de Freeverb |
| Ritmos euclidianos | **E. Bjorklund**, algoritmo; **Godfried Toussaint**, aplicação e análise musical |
| Transformações neo-riemannianas P/L/R | **Hugo Riemann**, **David Lewin** e **Richard Cohn** |
| Cadeias de Markov | **Andrey Markov** |
| Arco dramático/pirâmide | **Gustav Freytag** |
| Kishōtenketsu | Estrutura narrativa de origem chinesa, desenvolvida em tradições do Leste Asiático; não é atribuída a um único autor |
| As quatro categorias de jogo (Alea/Agon/Ilinx/Mimicry) | **Roger Caillois**, *Les Jeux et les Hommes* (1958) |
| Ritmo aditivo/aksak balcânico | **Constantin Brăiloiu**, pesquisa etnomusicológica sobre ritmo aksak; **Stoyan Dzhudzhev**, teoria e nomenclatura do ritmo aksak búlgaro |

## Agradecimento às comunidades

O projeto reconhece a contribuição duradoura das comunidades de síntese DIY,
Lunetta/CMOS, música eletroacústica, software livre e modular Eurorack, cujos
documentos, discussões e experimentos mantêm essas técnicas acessíveis para
estudo e novas criações.

Correções documentais de nomes, autoria ou proveniência são bem-vindas quando
acompanhadas de uma fonte primária ou bibliográfica verificável.
