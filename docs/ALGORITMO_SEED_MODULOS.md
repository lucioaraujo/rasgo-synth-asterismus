# Algoritmo composicional: seed, módulos e Constelação

**Estado:** especificação de implementação atualizada — 2026-09-08

## Princípio

Uma seed não escolhe um preset nem uma sequência fixa. Ela gera uma partitura
de mundo determinística e finita em memória: identidade harmônica, política de
jogo, perfil de dança, funções orquestrais, vocabulários por módulo, eventos,
memória e tendência de epílogo. O Campo do performer condiciona esta partitura
sem ser reescrito por ela.

## Originalidade como requisito técnico

Originalidade não será tratada como rótulo nem como aleatoriedade máxima. Ela
resulta da combinação específica do Rasgo Performance: uma mesma cosmologia de
seed encontra um Campo autoral diferente, corpos com agência e funções
orquestrais, fenômenos de Constelação, jogo e uma memória que transforma os
retornos. Referências do Studio são genealogia e rigor de implementação; não
autorizam copiar sua forma nem reduzir o Performance a uma interface derivada.

O algoritmo deve procurar diferença por contraste e transformação: mudanças de
papel, percurso, harmonia, ritmo, matéria, proximidade, silêncio e coda. Não
deve procurar diferença por saturar todos os módulos, acrescentar ruído
contínuo ou esconder uma escolha aleatória sem consequência reconhecível.

## Estruturas que a seed deve gerar

| Camada | Conteúdo |
|---|---|
| cosmologia | estilo, matéria, temporalidade, espacialidade e coletividade |
| harmonia | raiz, dialeto, regiões, progressões, pivôs e tensão de modulação |
| jogo | pesos de alea, agon, ilinx e mimicry; estratégia, regra e recursos |
| dança | tempo, peso, fluxo, espaço e relação |
| módulos | função, registro, vocabulário, agência, resistência e permeabilidade |
| constelação | centros, rotas, afinidades, horizontes, limiares e memória |
| narrativa | abertura, desenvolvimento, contraste, retorno, coda e silêncio |

## Ciclo causal por acontecimento

1. A seed produz uma intenção com estratégia, regra, qualidade de jogo e gesto.
2. O Campo avalia corpos presentes, cabos, distância 3D, profundidade, rastro,
   memória e intervenção do músico.
3. Cada módulo interpreta a intenção segundo sua função e agência: aceita,
   resiste, transforma, esconde, responde, troca, rompe ou repara.
4. A Constelação decide pares, pequenos conjuntos, rota e regime de dança.
5. A realização altera parâmetros musicais discretos: ritmo, contorno,
   harmonia, timbre, espaço, orquestração ou silêncio.
6. O resultado grava memória e atualiza horizonte, limiar e possibilidade de
   coda; nunca move âncoras, remove módulos ou troca cabos implicitamente.

## Funções dos módulos

Cada módulo deve receber, a cada seed, uma função principal e uma possível
função transitória. Nenhum corpo toca constantemente só porque está presente.

| Função | Ação musical |
|---|---|
| chão | sustenta pulso, baixo, gravidade ou região harmônica |
| chamada | abre seção, pede resposta ou anuncia limiar |
| resposta | completa, contradiz, imita ou desloca a chamada |
| contracanto | cria percurso independente sem competir continuamente |
| matéria | colore, contamina, filtra, granula ou espessa |
| ruptura | interrompe, fragmenta, desloca tempo ou abre silêncio |
| vestígio | guarda, reapresenta e transforma memória |
| coda | rarefaz, reúne ou encerra sem corte técnico |

## Políticas de segurança musical

- Toda transformação tem duração bounded, envelope e condição de retorno.
- Nenhuma seed pode manter oscilador, nutrição, tensão ou efeito contínuo sem
  acontecimento, função e saída musical.
- Sorteio acontece fora do callback ou por função determinística sem alocação.
- A mesma seed, Campo e histórico reproduzem a mesma trajetória; outro Campo
  altera a realização, não apaga a identidade da seed.
- O músico pode interromper qualquer tendência ao mover, conectar, silenciar,
  reconfigurar ou iniciar a coda.

## Camadas internas e memória Modal

Uma stem liberada é um pequeno submix orquestral, não uma caixa limitada a um
único módulo. Quando a seed convoca matéria Modal sem liberar uma quinta stem,
o algoritmo escolhe seu proprietário nesta ordem: destino do cabo de
`MEMÓRIA`, `VESTÍGIO` disponível, primeiro corpo vivo. A matéria entra nesse
submix com ganho bounded e a frase do proprietário, obedecendo mute, solo,
ganho, estado de vida e coda. O desenho espacial, o polígono e os cabos não
são alterados.

Um turno narrativo pode excitar a camada e modificar imediatamente a cor
musical do módulo-alvo. A camada interna não possui relógio autônomo nem
portadora contínua: sua cauda é encerrada pelo decay da matéria. Somente uma
quinta stem expressamente liberada e calibrada pode usar o relógio Modal
esparso próprio. Assim, a memória tem lugar, causa e saída audível dentro da
Constelação.

## Implementação em quatro blocos

1. **Score da seed:** gerar perfil de composição, pesos de jogo, perfil de
   dança, mapa harmônico e 32–64 intenções estruturais reproduzíveis.
2. **Agência dos módulos:** dar a cada corpo uma função, vocabulário, margem de
   resistência/permeabilidade e resposta por estratégia.
3. **Orquestra da Constelação:** selecionar pares e grupos por cabos, presença,
   posição viva, profundidade, memória e qualidade de jogo.
4. **Narrativa e gravação:** conduzir abertura, desenvolvimento, pausa, retorno
   e epílogo; registrar no áudio e no trace a mesma história.

## Critério de pronto

O algoritmo estará pronto quando uma mesma seed puder produzir diferentes
performances em Campos distintos, preservando sua identidade; quando cada
módulo tiver função mutável mas reconhecível; quando a pausa produzir narrativa
em vez de repetição; e quando o performer ouvir relações, escolhas e
consequências antes de qualquer explicação visual.
