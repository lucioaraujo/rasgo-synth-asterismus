# Gramática do campo performativo

**Estado:** especificação musical e critério de superfície — 2026-09-02
**Função:** definir as possibilidades sonoras que nascem do movimento dos
corpos, das conexões, dos fluxos, das forças espaciais e da variação performada
da seed.

## Decisão central

O núcleo identitário do Rasgo Synth Performance é o **CAMPO**. Ele não é uma
representação de mixer nem uma animação sobre parâmetros existentes. É o lugar
composicional onde corpos sonoros são posicionados, movidos, conectados,
separados, atraídos, repelidos, fusionados, interrompidos e transformados.

`MESA` permanece apenas como nome histórico da interface em desenvolvimento.
`ESPAÇO` designa uma dimensão musical do campo. Na linguagem criativa e na
próxima interface, o nome principal é **CAMPO**.

O Studio fornece genealogia, matérias, mecanismos, memória, jogo e experiência
generativa. O Performance deve ir além dele ao permitir que o músico toque, em
tempo real, não só os módulos produzidos pela seed, mas também as
**possibilidades de variação da própria seed**.

Assim como o Studio, o Performance é um **instrumento que gera músicas
gravadas**. Não se trata de manter indefinidamente um ambiente sonoro ou de
registrar opcionalmente um ensaio: cada realização possui abertura,
desenvolvimento, coda, epílogo e uma tomada de áudio vinculada ao campo e ao
seu trace. No Performance, essa obra nasce da conversa entre a ação corporal
do músico e a continuidade generativa de uma ou mais seeds.

```text
seed de origem abre um campo
  -> escolhe módulos reais, papéis e uma estratégia inicial de jogo
      -> somente os corpos necessários à abertura ocupam o campo e soam
      -> o músico modela posição, profundidade, movimento, forças e fluxos
          -> uma seed viva deriva em cada corpo e relação
              -> o campo lembra, resiste e transforma acontecimentos futuros
```

Módulos e estratégia não são anexos posteriores. Eles são a primeira
materialização musical da seed e o motor que permite à trajetória começar. Uma
seed sem corpos audíveis, papéis, relações e estratégia inicial ainda não abriu
um campo performável.

Esse motor não transforma o músico em acompanhante da automação. No campo, o
músico **conversa e compõe com a seed**: pode segui-la, contrariá-la,
pervertê-la, interrompê-la, rasgá-la, reparar seus vestígios, desviar seus
fluxos ou colocá-la em conflito com outra seed. A seed responde por meio dos
módulos, da estratégia, da memória e das consequências acumuladas. A obra nasce
dessa reciprocidade, inclusive quando uma das partes resiste ou se cala.

## Constelação como wayfinding

O Campo é o meio causal tridimensional; a Constelação é a organização legível
de astros, relações, regiões, encontros e percursos que emerge nele. Ela
funciona como **wayfinding não linear**: permite reconhecer onde a música está,
de onde veio, o que está próximo e quais transformações estão se tornando
possíveis sem prescrever uma rota. A navegação modifica o mapa por memória,
insistência, cicatriz, fadiga e reparo.

Fluxos, mapeamentos, encontros, rastros e profundidade pertencem à própria
caixa `CAMPO`; não formam outro painel gestualmente duplicado. O modelo usa
`x`, `y` e `z`, com projeção bidimensional causal. A vista pode orbitar para
inspeção sem alterar o som; girar a Constelação musical é outra ação e produz
consequência sonora. A especificação está em
[`WAYFINDING_CONSTELACAO.md`](WAYFINDING_CONSTELACAO.md).

## Invariantes de autoria

1. Os corpos já presentes no campo não desaparecem quando a seed viva varia; o
   polígono é a configuração coletiva que eles desenham.
2. As âncoras, posições autorais, conexões e seleção permanecem intactas até um
   gesto espacial explícito do músico.
3. Movimento, atração e repulsão podem transformar o estado vivo; não
   sobrescrevem silenciosamente o desenho-base salvo.
4. A seed de origem é preservada como identidade, memória e ponto de retorno.
5. Uma variação de seed não troca coorte, não recolhe módulos e não instala um
   campo novo.
6. Somente a ação deliberada `NOVO CAMPO` pode substituir o conjunto inicial
   de corpos; ela deverá possuir memória/retorno antes de entrar no palco.
7. Movimento visual e consequência sonora leem o mesmo estado causal. Não há
   animação decorativa.
8. Um controle criativo sem alvo, verbo e resultado reconhecível não fica
   disponível na superfície.
9. Nada é meramente decorativo. Formas coletivas, órbitas, cabos, fluxos,
   halos, colisões e indicadores representam estado ou acontecimento musical
   real; se essa ligação ainda não existe, o elemento permanece oculto.

## Um campo tridimensional, não uma planta

Mesmo que a tela seja plana, o modelo musical é tridimensional. Cada corpo
possui `x`, `y` e **profundidade**; a projeção visual pode usar tamanho, halo,
nitidez, oclusão parcial e perspectiva. Profundidade não é apenas volume:
participa de presença, distância relacional, detalhe, reverberação, latência e
capacidade de atravessar ou ser atravessado por fluxos.

Cada corpo também possui propriedades ecológicas, não uma simulação física
literal:

- **massa:** persistência e resistência a forças/transformações;
- **energia:** capacidade atual de emitir acontecimentos e influenciar outros;
- **inércia:** tempo necessário para mudar trajetória;
- **permeabilidade:** quanto de um fluxo externo pode entrar;
- **gravidade/afinidade:** tendência de aproximar certos corpos;
- **repulsão/incompatibilidade:** tendência de preservar distância e contraste;
- **memória/fadiga:** alteração dessas respostas pelo que já aconteceu.

Essas grandezas são bounded, suavizadas e reversíveis. Elas não formam um
motor caótico sem controle nem movem as âncoras de autoria.

## A constelação inteira entra em movimento

Depois que o músico configura os módulos, o conjunto não fica congelado. O
desenho passa a ser uma **constelação móvel**. Três referenciais coexistem:

1. **desenho-base:** âncoras e conexões configuradas pelo músico;
2. **movimento coletivo:** transformação da constelação inteira;
3. **movimento local:** trajetória própria de cada astro/corpo.

O desenho-base é sempre recuperável. Movimento coletivo e movimento local não
sobrescrevem suas coordenadas. Se o músico quiser transformar o estado vivo em
novo desenho-base, isso exige uma ação explícita de inscrição.

### Movimentos coletivos

| Movimento do conjunto | Geometria | Consequência musical própria |
|---|---|---|
| **Rotação plana** | formação gira em torno do centro comum, preservando distâncias | circulação de foco, panorama e ordem de ataques entre os corpos |
| **Rotação em eixo 3D** | formação inclina e gira em profundidade | alternância frente/fundo, detalhe/névoa, latência e perspectiva espectral |
| **Órbita do campo** | centro da constelação percorre trajetória em torno de seed ou atrator | desenvolvimento macro de frase, energia e espacialidade |
| **Expansão / contração** | distâncias crescem ou diminuem proporcionalmente | autonomia versus permeabilidade, rarefação versus densidade e fusão potencial |
| **Precessão** | eixo de rotação muda lentamente | deriva de fase, métrica, timbre e narrativa sem repetição circular idêntica |
| **Gravitação elástica** | distâncias se deformam segundo massa, atração e repulsão | alianças, disputas, sync parcial, colisões e separações com memória |
| **Enxame / turbulência** | corpos preservam confinamento, mas perdem formação rígida | polirritmia, fragmentação, glitches e redistribuição de energia bounded |
| **Estilingue** | um encontro acelera e ejeta um corpo | tempo shift, ruptura de frase, mudança de liderança e rastro |

Há dois regimes claramente escolhidos:

- **rígido:** a constelação atua como um **corpo rígido virtual**: todos os
  astros giram com a mesma velocidade angular em torno do centro comum, e as
  distâncias entre pares permanecem constantes. É análogo à rotação aparente
  coletiva da esfera celeste; musicalmente, circula foco e panorama sem mudar
  a topologia;
- **elástico:** atração, repulsão, colisão e fusão deformam temporariamente as
  distâncias, sem destruir o desenho-base.

Expansão/contração proporcional preserva a forma, mas não as distâncias;
portanto é uma respiração da constelação, não rotação rígida. Movimento comum
designa astros que seguem uma direção histórica parecida; não exige que se
comportem como corpo rígido. A interface precisa distinguir essas três leituras
para que o músico escolha exatamente o tipo de estabilidade ou transformação.

O movimento coletivo continua quando a mão para e passa a alimentar a
narrativa. O performer pode alterar eixo, sentido, velocidade, amplitude,
centro gravitacional e elasticidade durante a execução. Parar o movimento
retorna suavemente à figura-base ou congela o estado vivo, conforme a ação
escolhida; nunca salta silenciosamente para outra formação.

## Hierarquia astronômica de movimentos musicais

O Campo não deve usar uma única animação repetida. A referência astronômica
define escalas simultâneas: um astro pode rodar internamente, integrar uma
órbita, sofrer uma perturbação e, ao mesmo tempo, participar da deriva lenta
da constelação. Cada camada precisa ter consequência sonora distinta.

| Movimento astronômico | Escala no Campo | Consequência musical exigida |
|---|---|---|
| **Rotação** | eixo interno de um astro | transformação de timbre, excitação, articulação ou espectro do próprio stem; não muda vizinhança |
| **Translação / órbita** | corpo em torno de outro corpo, atrator ou baricentro | frase cíclica, panorama, espera/resposta e variação da permeabilidade entre parceiros |
| **Baricentro** | centro móvel calculado por massa, presença e relações de um agrupamento | liderança negociada, distribuição de energia e pulso comum incompleto; não impõe figura pré-definida |
| **Precessão** | mudança lenta do eixo ou centro da constelação | transformação macro de fase, métrica, harmonia, matéria e narrativa, sem ciclo curto perceptível |
| **Nutação** | pequenas oscilações sobre a precessão | microvariações rítmicas/tímbricas, atraso, respiração e instabilidade controlada; nunca jitter gratuito |
| **Movimento próprio** | deriva longa de cada astro na história da peça | memória, fadiga, amadurecimento ou deslocamento de papel, preservando o desenho-base recuperável |
| **Movimento radial** | aproximação/afastamento em profundidade | presença, foco, filtragem, reverberação e Doppler musical (cor/altura/tempo bounded) |
| **Órbita galáctica** | arco de longa duração do Campo inteiro | seções, densidade, energia e direção da narrativa entre abertura, desenvolvimento, coda e epílogo |
| **Perturbação orbital** | influência temporária de outro corpo, relação ou seed | atração, repulsão, colisão, stutter, tempo shift, contágio ou cicatriz com retorno explícito |

A tradução não é literal: um astro não precisa exibir todos os movimentos ao
mesmo tempo. O performer escolhe camadas, intensidade e retorno; a seed pode
propor perturbações durante a pausa. A interface só mostra uma camada quando
ela tiver alvo, escala, consequência audível e estado legível.

**Estado de implementação:** Órbita, Pulso e Figura-8 já realizam uma primeira
camada coletiva/local. Rotação interna, baricentro explícito, precessão,
nutação, movimento radial e perturbações musicais por relação permanecem
próximas extensões, cada uma dependente de prova A/B e escuta humana.

## As cinco camadas do campo

| Camada | O que o músico compõe | O que pode mudar no som |
|---|---|---|
| **Corpo** | presença, identidade, foco e controles próprios | matéria, articulação, registro, energia e silêncio |
| **Âncora e movimento** | posição autoral e dança tridimensional | frase, fase, pulso, timbre, recorte e espacialidade |
| **Topologia e forças** | vizinhanças, profundidade, atração, repulsão e agrupamentos | permeabilidade, resistência, autonomia e hierarquia |
| **Fluxo** | o que atravessa uma conexão, em qual direção e quando | contágio temporal, troca de matéria, memória, contraponto e corte |
| **Seed viva** | deriva, enxerto, fixação e retorno por corpo/relação | desenvolvimento, contraste, densidade, cadência, matéria e vestígio |

Essas camadas não são cinco painéis independentes. O gesto direto no corpo ou
na conexão escolhe o contexto; aparecem somente as ações capazes de atuar nesse
alvo real.

## Movimento como operação musical

Cada corpo conserva sua posição no **desenho-base** do músico. O polígono é a
forma coletiva resultante dessas posições. O movimento coletivo transforma essa
forma em torno do centro comum; o movimento local possui estado próprio por
corpo — forma, amplitude, velocidade, profundidade, fase, sentido e
confinamento — e só a deforma quando for explicitamente escolhido.

Não existe satélite ornamental nem polígono individual em órbita. A posição
viva de cada astro, seja fruto da transformação coletiva ou de uma força local,
é a referência para relações, distâncias, profundidade, encontros, fluxos e
áudio. Interface e motor leem a mesma geometria; uma linha ou halo só pode
existir quando comunicar uma consequência musical real.

| Movimento | Leitura musical própria | Consequência distinta |
|---|---|---|
| **Rotação sobre si** | transformação interna sem trocar vizinhança | timbre, articulação, espectro ou modo de excitação do próprio stem |
| **Pulso radial** | contração/expansão periódica | acentos, densidade, respiração e recuo; nunca simples panorama |
| **Órbita de uma âncora** | recorrência com fase e distância | espera/resposta e modulação espacial/tímbrica cíclica |
| **Órbita de outro corpo** | dependência assimétrica | influência fonte→intérprete, com atraso, resistência e retorno |
| **Figura-8** | passagem alternada por dois polos | hocket, alternância entre parceiros/domínios ou troca em duas fases |
| **Deriva confinada** | exploração lenta sem perder o território | desenvolvimento gradual da seed viva, matéria e frase |
| **Atração / repulsão** | negociação de proximidade | permeabilidade, sincronização parcial, autonomia e tensão |
| **Perseguição / fuga** | relação com atraso móvel | chamada/resposta, antecipação, eco, dívida e liderança |
| **Colisão** | encontro pontual de trajetórias | troca, corte, stutter, saturação ou rasgo bounded com vestígio |
| **Fusão temporária** | dois ou mais corpos formam organismo composto | cross-synthesis, pulso negociado e memória comum sem somar stems cegamente |
| **Separação pós-fusão** | identidades recuperam autonomia | resíduos, desafinação, fadiga, citação ou cicatriz compartilhada |
| **Agrupamento orbital** | formação ecológica de vários corpos | relógio comum incompleto e decisão coletiva sem apagar diferenças |

Fusão nunca destrói nem substitui módulos. Os stems originais continuam
identificáveis e recuperáveis; o organismo composto é uma relação temporária.

Uma forma de movimento só é aprovada quando sua diferença permanece audível
com os demais corpos tocando. Alterar poucos samples ou reutilizar o mesmo
envelope com outra curva não basta.

### Parâmetros com significado

- **amplitude da trajetória:** quanto o corpo explora relações em torno da
  âncora; não é volume;
- **velocidade:** taxa de transformação daquele corpo; não muda
  obrigatoriamente o BPM global;
- **profundidade:** distância perceptiva e relacional no eixo `z`;
- **fase:** momento em que um corpo encontra ou evita outro;
- **sentido:** ordem de leitura/propagação, útil para resposta e reversão;
- **confinamento:** até onde a dança deforma o estado vivo sem alterar a
  posição autoral;
- **rotação interna:** transformação do próprio material, independente do
  deslocamento do corpo.

O mesmo termo não pode esconder escalas diferentes: `GIRAR CORPO` atua no
polígono selecionado; `GIRAR CAMPO` atua no conjunto e preserva sua geometria
relativa. A interface e a telemetria devem sempre distinguir os dois.

## Conexões e fluxos

Distância não deve ser apenas ganho, e um cabo não pode ser apenas uma linha
mais grossa. Toda conexão declara:

```text
fonte -> verbo -> matéria transportada -> destino
      + intensidade + atraso + permeabilidade + resistência + memória
```

| Verbo relacional | Fluxo possível | Resultado sonoro esperado |
|---|---|---|
| **Contaminar** | matéria/timbre | um corpo empresta rugosidade, espectro, excitação ou ruído ao outro |
| **Propagar** | ataque/pulso | eventos atravessam com atraso, divisão, desvio ou perda |
| **Contrapor** | frase/tempo | resposta independente, hocket, síncope ou oposição métrica |
| **Orbitar** | fase/espaço | recorrência assimétrica, aproximação e afastamento audíveis |
| **Sombrear** | memória | o destino cita, deforma ou retém acontecimento anterior da fonte |
| **Resistir** | qualquer domínio | parte do fluxo é recusada; resistência e fadiga mudam com o histórico |
| **Fusionar** | matéria/tempo/memória | cria organismo temporário sem perder as fontes originais |
| **Rasgar** | rota/tempo/matéria | a conexão se rompe, bifurca ou muda de destino e deixa cicatriz |
| **Reparar** | vestígio/topologia | a relação retorna transformada, sem fingir que o rasgo não aconteceu |

A proximidade aumenta a oportunidade de encontro; não garante obediência. A
distância pode produzir autonomia, latência, eco ou recusa em vez de silêncio
automático. A identidade de cada módulo decide como interpreta o mesmo verbo.

Um fluxo deve ser visível somente quando um acontecimento real está
atravessando a conexão. O movimento gráfico do fluxo funciona como telemetria
musical, não como decoração contínua.

## Taxonomia de relações espaciais dinâmicas

Os fenômenos do Campo são **operadores de composição**. A geometria torna sua
ação tocável e legível, mas não é sua finalidade. Cada par fornece três
dimensões contínuas:

- distância `r`: aproxima, afasta, estabiliza ou oscila;
- ângulo `θ`: gira, inverte, mantém direção ou oscila;
- velocidade relativa `v`: acelera, desacelera, sincroniza ou rompe o fluxo.

Essas dimensões alimentam saídas musicais independentes: acoplamento de pulso,
fratura temporal, troca e fusão de matéria, circulação espacial, tensão
dinâmica, memória, desvio da seed e silêncio. Nenhum operador pode ser reduzido
a volume ou panorama.

| Operador de par | Movimento relacional | Operação composicional exigida |
|---|---|---|
| `ATTRACT` | aproxima | aumenta permeabilidade, troca de matéria e possibilidade de sincronização |
| `REPEL` | afasta | preserva autonomia, tensão, contraste, corte e recusa |
| `ORBIT` | gira em torno de corpo/baricentro | produz recorrência, alternância de foco, fase e resposta |
| `ELLIPTICAL ORBIT` | varia distância durante a volta | alterna aproximação/recuo, permeabilidade, acento e tensão dentro do ciclo |
| `BARYCENTRIC DANCE` | ambos giram em torno do centro de massa | negocia liderança, peso, frase e circulação sem centro imóvel |
| `ENCOUNTER` / `COLLIDE` | toca uma região comum | dispara encontro pontual: corte, troca, stutter, silêncio ou rasgo |
| `MERGE` / `CAPTURE` | forma sistema temporário | cria organismo tímbrico e temporal composto sem apagar os stems-fonte |
| `FLYBY` | aproxima, desvia e afasta | produz passagem, Doppler musical, transferência breve e vestígio |
| `SCATTER` | muda direção e segue separado | fragmenta métrica, matéria e liderança |
| `CHASE` | fonte segue destino | chamada/resposta com atraso, dívida, antecipação e liderança móvel |
| `ESCAPE` / `RELEASE` | destino rompe o vínculo | desfaz acoplamento, deixa memória e abre outra trajetória |
| `PARALLEL` / `ALIGN` | mantém distância/direção | cria movimento comum, pulso compartilhado incompleto e contraponto estável |
| `CONVERGE` / `DIVERGE` | vai a uma região comum / separa-se | adensa ou rarefaz a orquestração |
| `OSCILLATE` | aproxima e afasta ciclicamente | alterna permeabilidade, acento, densidade e recuo |
| `SPIRAL IN` / `SPIRAL OUT` | muda `r` e `θ` simultaneamente | combina transformação de frase, matéria e circulação |
| `EQUILIBRIUM` / `LOCK` | estabiliza configuração | sustenta tensão compensada, fase, drone ou memória comum |
| `SYNC` / `RESONATE` | negocia período/fase ou razão | produz relações 1:1, 2:1, 3:2 etc., sem quantização total obrigatória |
| `SWAP` | alterna posições relativas | troca foco, material, papel e janela de escuta |

Sobre populações, o mesmo vocabulário torna-se `ALIGN`, `CLUSTER`, `SCATTER`,
`COLLECTIVE ROTATE`, `EXPAND`, `CONTRACT`, `PULSE`, `FRAGMENT`, `MERGE GROUPS`
e `DRIFT`. A constelação pode atuar como corpo rígido, agrupamento elástico ou
população fragmentada. O regime escolhido precisa permanecer audível.

### Contrato de controle

1. O músico seleciona um cabo real, escolhe seu fenômeno e toca a intensidade
   no próprio cabo.
2. O estado vivo pode mover-se; as âncoras e o desenho-base não são escritos.
3. Intensidade zero devolve o par ou população ao desenho do músico.
4. Fonte e destino permanecem identificáveis em fusão, captura e troca.
5. A seed pode propor ou modular fenômenos durante a pausa, mas não apagar a
   escolha do performer nem substituir silenciosamente a topologia.
6. A interface mostra somente fenômenos que já atravessam o áudio.

**Estado de implementação em 2026-09-03:** o núcleo realtime possui 23
operadores de par, mede `r`, `θ`, velocidade radial/angular e gera forças
musicais bounded. Selecionar um cabo revela `FENÔMENO`; o controle percorre os
operadores, o arraste do cabo regula intensidade, áudio e deslocamento vivo
respondem sem alterar âncoras. Os dez fenômenos de população estão ligados ao
áudio coletivo e ao foco vazio do `CAMPO`: clicar em `CAMPO · FENÔMENO` ativa e
percorre o vocabulário; a roda regula sua intensidade. A cena capturada guarda
fenômeno, razão de ressonância, ativação e intensidade de cada cabo, assim
como o fenômeno coletivo e sua intensidade, no `RSPERF 4`; documentos
`RSPERF 1` a `3` continuam legíveis com retorno compatível.
As trajetórias-base são lentas: a 120 BPM, Órbita completa uma volta em cerca
de 9 s, Figura-8 em 6 s e Pulso em 4 s. Isso deixa o movimento audível como
transformação e não como agitação visual; BPM continua escalando a duração.

## Seed de origem e seed viva

A seed deixa de ser apenas número de abertura. O Performance conserva dois
níveis simultâneos:

- **seed de origem:** escolhe o campo inicial, corpos, predisposições, limites
  e histórias potenciais;
- **seed viva:** estado performável, derivado e reversível que age sobre um
  corpo, uma relação ou o conjunto sem apagar o campo.

### Abertura compilada pela seed

Ao iniciar um campo, a seed de origem deve definir em conjunto:

1. os módulos necessários à abertura e seus stems efetivamente audíveis;
2. os módulos complementares preparados que o músico poderá admitir depois;
3. papéis iniciais de cada corpo — chão, corpo, atrito, contágio, rasgo ou
   vestígio — sem apagar a identidade do módulo;
4. estados iniciais de matéria, frase, pulso, registro, densidade e silêncio;
5. afinidades, resistências, relações possíveis e quais começam abertas;
6. uma **estratégia composicional de jogo** — seguir, transformar, contestar,
   interromper, ocultar ou negociar — e as regras bounded que a realizam;
7. tendências narrativas, relógios de acontecimento, memória e possibilidades
   de coda.

Somente os módulos considerados necessários pela seed aparecem inicialmente no
campo; todos precisam possuir corpo de áudio e começar musicalmente. A reserva
externa mostra apenas módulos já adaptados e pertinentes àquele campo. O músico
pode arrastá-los para dentro quando desejar ampliar a formação, sem trocar a
seed, apagar os corpos presentes ou perder o desenho-base.

A estratégia não é um rótulo de telemetria. Ela distribui decisões entre os
módulos: quem sustenta, pergunta, responde, resiste, interrompe, espera,
contamina ou deixa espaço. A trajetória nasce da execução e transformação
dessas decisões pelo diálogo entre músico, campo, seed e histórico; não de uma
sequência fixa de eventos preparada fora dos módulos.

Não se interpola aritmeticamente o número da seed. Variações são feitas sobre
traços preparados — frase, cadência, densidade, tensão, matéria, memória,
estratégia e pausa — mantendo reprodução e proveniência.

### Quatro verbos da seed viva

| Verbo | Ação | Retorno |
|---|---|---|
| **DERIVAR** | cria descendência determinística da origem para o alvo selecionado | volta à origem preservando, quando escolhido, pequeno vestígio |
| **ENXERTAR** | cruza traços de outra seed com corpo ou fluxo | reduz o enxerto até zero ou o fixa como memória |
| **FIXAR** | congela uma variação encontrada | libera a trajetória a partir do estado fixado |
| **RETORNAR** | aproxima progressivamente a variação da origem | não apaga cicatrizes explicitamente preservadas |

A intensidade pode ser manipulada como knob/mixer `ORIGEM ↔ VARIAÇÃO`, mas
sempre com domínio e alvo visíveis. Um mixer global opaco não basta. Em etapa
seguinte, movimentos modulam essa intensidade: deriva espacial aprofunda a
descendência, colisão troca traços e órbita faz a variação retornar
ciclicamente.

### Três escalas de atuação da seed

1. **Por corpo:** altera a trajetória de um stem sem mudar os demais.
2. **Por conexão:** transforma o que atravessa um par — timing, matéria,
   memória ou articulação.
3. **Por campo:** muda uma regra compartilhada, somente por gesto explícito e
   com alcance claramente indicado.

O código atual já possui trajetória de 32 eventos e primeira colagem estrutural
entre duas seeds. Isso é matéria útil, não conclusão: hoje a colagem é global,
afeta poucos traços, não possui descendência por corpo nem prova perceptiva no
host.

## Polifonia de seeds

Uma performance pode conter uma ou várias seeds atuando simultaneamente. A
seed de origem permanece como proveniência do campo; seeds adicionais entram
como **núcleos de influência**, não como troca oculta de preset.

No modelo espacial, uma seed pode aparecer como um centro de força. Sua área
de influência alcança corpos e conexões conforme distância, afinidade,
permeabilidade e gesto do músico. Um módulo pode permanecer ligado à origem,
orbitar outra seed, receber traços de duas ou atravessar uma zona em que as
histórias se sobrepõem.

Cada núcleo de seed declara:

- número/origem e descendência atual;
- corpos, relações ou região alcançados;
- domínios transportados: frase, tempo, matéria, memória, jogo ou espaço;
- intensidade, relógio, fase, duração e modo de retorno;
- relação com as demais seeds: cooperação, disputa, contaminação, recusa ou
  ruptura.

### Modos de encontro entre seeds

| Modo | Organização temporal | Resultado esperado |
|---|---|---|
| **Sobreposição** | narrativas paralelas conservam relógios próprios | camadas independentes que se afetam sem virar uníssono |
| **Mashup** | traços escolhidos de duas histórias coexistem em estrutura comum | frase de uma seed, tempo de outra, matéria de uma terceira, com fontes legíveis |
| **Colagem** | fragmentos entram, saem e reaparecem por cortes | montagem, contraste, memória e retorno não linear |
| **Alternância** | autoridade passa entre seeds por fase, gesto ou condição | chamada/resposta, blocos, hocket ou disputa de foco |
| **Contaminação** | uma seed modifica gradualmente a leitura de outra | híbrido que preserva genealogia e pode ser desfeito |
| **Glitch** | colisão breve rompe relógio, frase ou matéria | stutter, salto, silêncio, repetição quebrada ou erro intencional bounded |
| **Fusão** | duas descendências formam regra temporária comum | terceira narrativa emergente, sem apagar as duas fontes |

Não se somam geradores inteiros indiscriminadamente. A arbitragem ocorre por
domínio, alvo e tempo. Toda influência é bounded e toda seed mantém
proveniência. A arquitetura admite vários núcleos; a implementação inicial usa
apenas dois para provar legibilidade, estabilidade e valor musical antes de
ampliar a polifonia.

Uma seed adicional também não instala automaticamente seus módulos. Ela atua
primeiro sobre os corpos presentes. Um novo corpo só entra quando estiver
adaptado, houver capacidade realtime e o músico o aceitar explicitamente; os
polígonos anteriores continuam no campo.

## Continuidade quando a mão para

A música deve estar sempre em transformação, inclusive quando o performer
interrompe momentaneamente a manipulação. Isso não significa preencher tudo ou
proibir silêncio e repetição. Significa que o **estado continua vivendo** e que
uma repetição retorna com diferença, memória ou nova função.

Há três narrativas simultâneas:

1. **micro — corpo:** cada módulo desenvolve frase, matéria, fadiga, memória e
   possibilidade de silêncio;
2. **meso — relação:** cada par desenvolve aproximação, disputa, contágio,
   recusa, fusão, separação e vestígio;
3. **macro — campo:** a seed viva organiza tensões, contrastes, passagens,
   mudanças de foco, rasgos e retornos.

Com várias seeds, pode haver mais de uma narrativa em cada escala. O campo não
as reduz obrigatoriamente a uma linha mestra: elas podem se alternar, divergir,
colidir ou formar uma narrativa temporária comum.

O regime temporal é:

```text
mão ativa -> consequência direta
soltura -> curta passagem de autoridade
pausa -> narrativas continuam a partir do estado vigente
retorno da mão -> diálogo/interrupção, nunca reset automático
```

As narrativas podem divergir: um corpo sustenta, outro deriva e uma relação
entra em fadiga. Elas não devem receber um mesmo relógio ou arco compulsório.
O motor impede apenas a estagnação mecânica em loop curto; não força mudança
superficial a cada instante.

## Abertura, coda e epílogo: a performance gera uma faixa completa

O Rasgo Synth Performance não é um ambiente obrigado a tocar indefinidamente.
Assim como o Studio gera uma composição do começo ao fim, o Performance gera
uma música gravada completa: abertura do campo, vida, transformações, coda e
epílogo.

Toda performance é gravada. O músico define o instante autoral de início e o
instante em que chama a coda; não existe uma performance oficial tocando sem
registro.

```text
preparar campo (ainda fora da faixa)
  -> INICIAR: abertura musical + início simultâneo de áudio e trace
      -> desenvolvimento performado e narrativas autônomas
          -> CHAMAR CODA pelo músico ou por inatividade prolongada
              -> coda musical -> cauda segura -> silêncio
                  -> epílogo técnico: fechar gravação e preservar trace
```

Neste contrato, **coda** é a passagem musical final; **epílogo** inclui a coda
e o encerramento completo da performance/arquivo.

O epílogo não é `STOP`, fade-out genérico nem cena fixa. Ele é uma fase
generativa que lê a história efetivamente vivida:

- corpos presentes, ausentes, fusionados e separados;
- seeds de origem, descendências e núcleos ainda ativos;
- relações de apoio, disputa, contaminação, recusa e reparo;
- movimentos, colisões, cicatrizes, memórias e fadigas acumuladas;
- gesto recente do performer e estado atual do campo.

### Como o epílogo começa

Há duas portas principais, sempre legíveis:

1. **FIM PELO MÚSICO:** o performer chama a coda a qualquer momento. Ela
   responde musicalmente dentro do próximo pulso/frase indicado;
2. **FIM POR INATIVIDADE:** quando a mão para, o instrumento continua primeiro
   sua narrativa. Depois de um intervalo mais longo, definido de forma
   determinística pela seed e pelo estado do sistema, inicia o epílogo.

Pausa narrativa e espera para terminar são relógios diferentes:

```text
último gesto
  -> passagem curta: seed assume e continua a música
      -> narrativa autônoma em curso
          -> limite prolongado de inatividade
              -> epílogo automático
                  -> cauda sonora -> silêncio -> fim da gravação
```

O prazo de epílogo automático deve aparecer como estado real, com tempo
restante. Qualquer gesto novo antes do ponto final reinicia a inatividade e
pode cancelar/adiar o epílogo automático. O músico conserva autoridade para
chamar o fim, prolongar a performance ou usar `STOP` por segurança; `STOP`
continua operação técnica e não substitui o epílogo musical.

### Transporte e gravação

- `INICIAR` arma e inicia a gravação junto da abertura; se o arquivo não puder
  ser criado, a performance não começa sem informar o erro;
- `GRAVANDO` é indicador, não decisão opcional nem botão concorrente;
- `CHAMAR CODA` inicia a conclusão musical e mantém a gravação aberta;
- a gravação fecha automaticamente depois da cauda segura;
- `STOP DE EMERGÊNCIA` interrompe o áudio, mas preserva e fecha o take como
  `interrompido`, nunca o descarta;
- cada take inclui vínculo com seed(s), campo, trace e razão do encerramento:
  coda do músico, coda por inatividade ou emergência.

### Famílias de conclusão

| Epílogo | Comportamento do campo |
|---|---|
| **Dispersão** | corpos se afastam, relações rareiam e narrativas terminam em tempos distintos |
| **Retorno** | a constelação retorna ao desenho-base e reapresenta traços transformados da origem |
| **Implosão** | atração cresce até fusão/corte final bounded |
| **Cicatriz** | quase tudo recua e um vestígio material ou memorial permanece até o silêncio |
| **Última colisão** | histórias/seed entram num último mashup, glitch ou rasgo e cessam abruptamente |
| **Corte seco** | silêncio súbito preparado pela trajetória, não simples interrupção acidental |
| **Coda plural** | seeds concluem separadamente; uma pode citar ou responder à conclusão da outra |

A seed propõe tendências, mas o histórico decide quais conclusões ainda fazem
sentido. A mesma seed não precisa terminar sempre do mesmo modo.

### Encerramento técnico e memória

Quando todos os corpos concluem, o sinal alcança silêncio técnico por uma
janela segura ou o limite máximo de cauda é atingido, a gravação da faixa é
fechada automaticamente e o writer termina seu flush fora do callback. O
instrumento preserva:

- seed(s), descendências e modos de encontro;
- desenho-base e estado final do campo;
- trajetória de movimentos, relações, rasgos e reparos;
- decisão de epílogo e duração das caudas;
- vínculo entre o arquivo de áudio e o trace da performance.

O fim da faixa não apaga o campo. O músico pode revisar o trace, guardar uma
memória ou iniciar deliberadamente outro campo.

## Auditoria da superfície atual

| Controle atual | Decisão | Razão / transformação necessária |
|---|---|---|
| `ALEA` / seed principal | **manter como NOVO CAMPO** | mudança estrutural deliberada; precisa de memória/retorno antes do uso irreversível em palco |
| arrastar corpos e cabos | **núcleo** | linguagem direta do campo; precisa de causalidade sonora mais profunda |
| mix, mute, solo e master | **manter como operação** | resultado e alvo claros; não fingem ser composição generativa |
| controles locais dos módulos | **auditar um a um** | só aparecem quando o módulo está selecionado e a escuta confirma contraste útil |
| movimento global Órbita/Pulso/Figura-8 | **substituir** | deve ser escolha por corpo, combinável e com consequência própria |
| `COLAGEM · RODAR · MIX` global | **reformular** | torna-se seed viva por corpo/relação, com origem, alvo, domínio, descendência e retorno |
| campo XY Autonomia/Intervenção/Memória/Ruptura | **suspender da superfície** | abstrai o campo real e mistura quatro causas sem alvo corporal explícito |
| sete macros genéricos | **suspender da superfície** | podem permanecer internamente como predisposições, não como gestos principais |
| Abertura/Espiral/Rasgo/Balanço + Morph | **suspender e reler** | poderão voltar como memórias voluntárias do campo, nunca substituição oculta |
| `GRID` | **suspender até prova** | só retorna ligado a fluxo/corpo e com diferença temporal reconhecível |
| Hold, Stutter, Tempo Shift, Rasgar, Reparar | **manter após escuta** | precisam agir no alvo selecionado, mostrar duração e retornar claramente |
| matriz de 16 células | **manter suspensa** | só reaparece como acesso secundário a funções já provadas no campo |
| `INICIAR`, `GRAVANDO`, coda e Panic Safe | **reformular como ciclo da obra** | iniciar abre áudio/trace; `GRAVANDO` informa; coda conclui; emergência preserva take interrompido |

## Primeira prova vertical

A próxima implementação não abre módulos novos nem reexibe a matriz. Ela deve
provar um circuito musical completo:

1. fazer uma seed abrir o campo com seus módulos necessários, stems, papéis,
   relações e estratégia composicional inicial, todos causalmente ativos;
2. selecionar um desses corpos já audíveis no campo;
3. aplicar `ÓRBITA`, `PULSO` e `DERIVA`, uma por vez, com identidade sonora
   distinta e âncora intacta;
4. girar a constelação rigidamente em torno do centro e num eixo 3D, mantendo
   o desenho-base e produzindo uma transformação sonora coletiva distinta;
5. conectar esse corpo a um segundo com verbo explícito e ouvir a diferença
   entre próximo, intermediário, distante e profundidades distintas;
6. criar descendência da seed somente para o primeiro corpo e manipular
   `ORIGEM ↔ VARIAÇÃO` sem trocar coorte, módulos, âncoras ou cabos;
7. introduzir uma segunda seed como núcleo de influência e comparar
   sobreposição, colagem e glitch sem trocar os corpos presentes;
8. permitir atração/repulsão e uma colisão bounded entre os dois corpos;
9. mostrar no campo qual corpo, seed, estratégia, relação, força e traço estão
   atuando;
10. deixar a mão parar e comprovar que corpos, relações e seeds continuam suas
   narrativas sem loop curto ou alteração das âncoras;
11. chamar um epílogo e comprovar que ele deriva da trajetória, chega ao
    silêncio, fecha uma gravação válida e preserva o campo/trace;
12. retirar cada ação e comprovar retorno, memória e ausência de click;
13. comparar em escuta humana com a música completa, não em solo e não somente
   por diferença numérica.

Somente depois dessa prova o contrato se expande aos demais corpos, relações,
movimentos e campos de acontecimento.

## Genealogia e diferença

Esta especificação herda do Studio:

- campo musical antes do mecanismo;
- seed como história e não preset;
- módulos como organismos com resistência, permeabilidade, fadiga e memória;
- espaço como topologia, regiões e passagens;
- relações causais assimétricas;
- silêncio, latência e recusa como possibilidades;
- rasgo capaz de alterar rotas e deixar vestígio;
- contrafactual e escuta humana como critérios de promoção.

Sua invenção específica no Performance é tornar tudo isso corporalmente
tocável dentro do campo e permitir ao músico variar a própria descendência da
seed em diálogo com movimentos, forças, conexões, fluxos e narrativas.

Essa diferença não autoriza recomeçar do zero. Para cada módulo, estratégia,
trajetória, relação, memória, rasgo ou forma de conclusão, o Studio e o core
devem ser consultados primeiro. O Performance reutiliza o comportamento
musical comprovado por contrato/adapter e acrescenta agência espacial e
performática. Uma alternativa nova só nasce depois de demonstrar que o
mecanismo canônico não atende à função pretendida no CAMPO.

Fontes principais: `PROGRAMA_PERFORMATIVO_CANONICO.md`,
`../../docs/pt/Genealogia_do_Instrumento.md`,
`../../docs/pt/Auditoria_Poetica_dos_Modulos_Fase_119.md`,
`../../docs/pt/Orquestra_Relacional_Fase_121.md`,
`../../docs/pt/Auditoria_Pos_Espaco_e_Ecologia_Movel_Fase_122.md` e
`../../docs/pt/Auditoria_de_Agencia_Relacional_Fase_123.md`.
