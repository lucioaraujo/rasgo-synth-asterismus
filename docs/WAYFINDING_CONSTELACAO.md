# Wayfinding e profundidade da Constelação

**Estado:** direção conceitual e contrato de implementação — 2026-09-05
**Escopo:** Rasgo Synth Performance, sem alteração do Rasgo Synth Studio
**Lugar na interface:** interior da caixa `CAMPO`

## Tese

A Constelação é um sistema de **orientação performativa não linear**. Ela não é
apenas o desenho dos módulos nem um grafo explicativo: oferece referências para
que músico e seed reconheçam onde a música está, por onde passou, quais
encontros estão próximos e quais transformações estão se tornando possíveis.

```text
sequencer       organiza o tempo
preset browser  seleciona estados prontos
patch           organiza conexões
constelação     organiza possibilidades de percurso
wayfinding      torna esses percursos legíveis e tocáveis
```

O mapa não é imóvel. Insistência, retorno, rasgo, memória, fadiga e reparo
alteram as regiões atravessadas e, portanto, os acontecimentos futuros.

Cada seed também inaugura uma **cosmologia composicional**: além de escolher
matéria e vocabulário, determina a afinidade espacial e coletiva da peça. Essa
afinidade não redesenha o Campo; ela regula a energia com que movimentos,
encontros, fluxos e conjuntos orquestrais se tornam disponíveis.

## Campo, Constelação e Performance

O **Campo** é o meio causal tridimensional onde forças, movimentos, encontros,
fluxos, memória e seed atuam. A **Constelação** é a organização musical legível
que emerge dos astros, relações, regiões e percursos nesse Campo.
**Performance** é a prática de navegar, transformar e escutar essa organização.

```text
CAMPO         condições, forças, profundidade e transformação
CONSTELAÇÃO   astros, relações, orientação e percursos possíveis
PERFORMANCE   navegação e inscrição do músico
```

Wayfinding, fluxos, mapeamentos, encontros e rastros devem existir **dentro da
caixa CAMPO**, não em outro painel que duplique o mesmo gesto.

## Espaço tridimensional

Cada astro possui posição `x`, `y`, `z` e uma âncora autoral recuperável:

- `x`: lateralidade, panorama, oposição e circulação horizontal;
- `y`: elevação no Campo, registro, densidade, energia ou função vertical;
- `z`: profundidade, frente/fundo, distância perceptiva e permeabilidade.

O eixo `z` não é perspectiva ornamental. Ele participa de presença, detalhe,
filtragem, difusão, reverberação, latência, largura estéreo, relação
direto/refletido, capacidade de atravessar fluxos e prioridade dos eventos.
Mover um astro em profundidade deve produzir resposta sonora imediata e
reconhecível, preservando volume seguro e identidade tímbrica.

A tela continua bidimensional, mas projeta `z` por uma combinação bounded de
tamanho, nitidez, halo, contraste, espessura das conexões e paralaxe. A
projeção visual e o áudio leem o mesmo estado causal.

### Girar para compreender

O músico pode girar a visualização da Constelação para compreender melhor
polígonos, profundidade, cruzamentos e conexões ocultas. Há duas operações
distintas e sempre identificadas:

1. **ÓRBITA DA VISTA:** desloca a câmera virtual em torno do polígono nos eixos
   `x`, `y` e `z`; não altera posições, relações nem som. É wayfinding: muda o
   ponto de observação, não a composição.
2. **GIRAR CONSTELAÇÃO:** transforma o polígono musical em torno de seu centro,
   baricentro ou astro escolhido; altera relações e produz consequência sonora
   conforme a gramática do Campo.

A órbita da vista oferece arraste livre, limites de inclinação, zoom bounded e
um comando `VISTA BASE`. O desenho-base do músico permanece intacto. A interface
deve mostrar claramente quando o gesto atua na câmera e quando atua na música,
impedindo que uma inspeção visual modifique a performance por acidente.

Polígonos podem ser observados de frente, de perfil, por cima ou em perspectiva.
Linhas parcialmente ocultas continuam selecionáveis por realce, foco ou
isolamento temporário, sem alterar sua intensidade musical.

## Vocabulário de orientação

| Elemento | Função musical |
|---|---|
| **astro** | módulo sonoro, comportamento e núcleo de agência |
| **linha** | relação, fluxo, afinidade ou transição possível |
| **distância 3D** | proximidade musical e custo/tensão de passagem |
| **região** | combinação reconhecível de matéria, energia, memória e estratégia |
| **trajeto** | percurso efetivamente realizado por músico e seed |
| **rastro** | memória audível e visual deixada pelo percurso |
| **horizonte** | possibilidades próximas cujas condições já estão presentes |
| **limiar** | condição cuja travessia produz acontecimento ou mudança de regime |
| **encontro** | proximidade temporal e espacial que permite troca, colisão ou recusa |
| **fluxo** | matéria, pulso, articulação, memória ou seed viva atravessando relação |

Regiões como `SUSPENSÃO`, `ATRITO`, `ACÚMULO`, `RASGO`, `RESÍDUO` e `DERIVA`
não são presets. A realização pode estar entre regiões e tender a outra sem
saltar para um estado pronto.

## Três leituras simultâneas

1. **Estrutural:** astros, relações, regiões e caminhos existentes no
   território aberto pela seed.
2. **Performativa:** posição atual, forças ativas, tendências, encontros e
   limiares alcançáveis.
3. **Histórica:** percurso, retornos, insistências, cicatrizes e regiões
   transformadas pela própria performance.

Essas camadas respondem, dentro do Campo:

- onde estou;
- de onde vim;
- o que está próximo em `x`, `y` e `z`;
- quais encontros e fluxos estão ativos;
- o que está se tornando possível;
- o que exige ruptura;
- o que mudou porque passei aqui.

## Mapeamentos e encontros

Distância, direção e velocidade relativas não controlam apenas panorama.
Mapeiam dimensões musicais independentes:

| Relação espacial | Possibilidades musicais |
|---|---|
| aproximação em `x/y` | troca de foco, hocket, sincronização, contágio |
| aproximação em `z` | presença, detalhe, permeabilidade, ataque, prioridade |
| afastamento em `z` | difusão, memória, latência, filtragem, recusa |
| cruzamento | troca de papel, corte, stutter, mutação ou silêncio breve |
| colisão | evento bounded de rasgo, fusão, fragmentação ou repique |
| órbita 3D | alternância frente/fundo, fase, timbre, fluxo e perspectiva |
| caminho repetido | expectativa, fadiga, massa, cicatriz ou transformação |

Cada encontro tem fonte, destino, janela temporal, intensidade, consequência,
memória e possibilidade de retorno. Proximidade cria oportunidade; não obriga
os módulos a obedecerem nem reduz tudo a mistura de volume.

## Linguagem gestual

`APROXIMAR` · `AFASTAR` · `APROFUNDAR` · `TRAZER À FRENTE` · `INSISTIR` ·
`DESVIAR` · `ATRAVESSAR` · `CRUZAR` · `COLIDIR` · `ROMPER` · `PERMANECER` ·
`RETORNAR`

Cada verbo declara alvo, eixos envolvidos, duração, reversibilidade,
consequência sonora e efeito sobre a memória.

## Linhas e fluxos não equivalentes

- contínua: relação disponível e estável;
- pulsante: fluxo condicionado por fase, pulso ou janela;
- descontínua: passagem instável ou intermitente;
- espessa: maior afinidade, fluxo ou probabilidade;
- cicatrizada: passagem anterior transformada pela memória;
- latente: possibilidade dependente de condição ainda não satisfeita;
- em profundidade: fluxo cuja projeção e resposta dependem de `z`.

Uma linha aparece quando existe relação real. Um fluxo se anima quando matéria
ou informação musical está atravessando essa relação. Nenhum dos dois é
decoração permanente.

## Memória da navegação

- repetição aumenta familiaridade, expectativa ou desgaste;
- insistência pode aumentar massa e atração de uma região;
- rasgo cria cicatriz e modifica retornos;
- reparo transforma a história, não a apaga;
- pausa permite à seed continuar a partir da posição e memória atuais;
- retorno reapresenta material transformado, não uma cópia mecânica.

O mapa usado para navegar é modificado pela própria navegação.

## Contrato de implementação

1. Wayfinding e profundidade vivem na caixa `CAMPO`.
2. Movimento em `x/y/z` nunca apaga âncoras nem conexões do músico.
3. Estado atual, rastro, horizonte e limiar são distinguíveis sem poluição.
4. Toda indicação visual deriva de estado causal e possui consequência audível.
5. A seed sugere e percorre possibilidades, mas não toma a decisão espacial do
   músico.
6. O músico pode aceitar, contrariar, bloquear ou perverter uma tendência.
7. Cenas memorizam pontos de passagem; não substituem navegação.
8. Áudio e trace registram a mesma trajetória até o epílogo.
9. Rastro, profundidade, encontros e fluxos permanecem bounded em CPU e memória.

## Etapas

1. criar estado bounded de `x/y/z`, região atual, rastro, horizonte e limiar;
2. definir projeção 2D coerente do eixo `z`;
3. implementar órbita da vista e `VISTA BASE`, sem alterar o estado musical;
4. implementar rotação musical do polígono com centro explicitamente escolhido;
5. mapear profundidade para consequências sonoras por módulo;
6. implementar encontros e fluxos com início, duração e memória;
7. mostrar `AGORA`, `RASTRO` e `POSSÍVEL` dentro do Campo;
8. implementar verbos de navegação com resposta imediata;
9. validar cada mapeamento por A/B humano e remover sinais fantasmas.

## Fenômenos da Constelação e tradução musical

Os fenômenos não entram como decoração ou coleção de botões. São regimes que a
seed pode propor e o músico pode aceitar, contrariar ou interromper pelo Campo:

| Fenômeno | Wayfinding | Consequência musical esperada |
|---|---|---|
| órbita / baricentro | reconhece centro e pertença | circulação de foco, frente/fundo e resposta |
| constelação rígida | reconhece um desenho coletivo | pulso e orquestração compartilhados |
| aproximação / encontro | mostra afinidade iminente | hocket, troca, sincronização ou contágio |
| repulsão / fuga | mostra recusa e distância | rarefação, filtragem, silêncio ou afastamento de papel |
| cluster / fusão | mostra conjunto temporário | dobramento, massa, ressonância ou coro |
| dispersão / fragmentação | mostra perda de coesão | polirritmia, contraponto, corte ou distribuição espacial |
| ressonância / travamento | mostra periodicidade comum | acentos partilhados e motivo reconhecível |
| deriva / passagem | mostra rumo sem destino fixo | transformação lenta de timbre, registro e fluxo |
| rastro / cicatriz | mostra percurso e insistência | memória transformada, expectativa ou reparo |
| limiar / rasgo | mostra passagem irreversível de regime | acontecimento bounded, coda parcial ou reorquestração |

O motor já lê posição viva, cabos, velocidade relativa e profundidade; a
primeira ligação da nova matriz faz a afinidade espacial da seed modular a taxa
bounded da dança e a pressão musical que ela expõe. As próximas implementações
devem tornar `AGORA`, `RASTRO`, `POSSÍVEL` e `LIMIAR` legíveis no próprio Campo,
com teste A/B antes de qualquer novo sinal visual.

## Identidade do instrumento

**Constelação** descreve a identidade emergente do instrumento; **Performance**
descreve seu modo de atuação. Essa formulação não autoriza ainda renomear
diretórios, formatos ou executáveis. Um renome exige decisão explícita,
inventário de impactos e preservação de proveniência.
