# Modelo implementável — jogo da seed e campo espacial

**Estado:** contrato de direção musical e técnica v0 — 2026-08-30

## Tese

O Rasgo Synth Performance é um instrumento generativo espacial. O performer
modela o campo; a seed propõe um jogo; os módulos jogam e produzem
acontecimentos; o músico escuta as consequências e remodela o campo.

Durante uma pausa momentânea da manipulação, os acontecimentos continuam. A
narrativa da seed conduz temporariamente a ação, mas sempre dentro das
espacialidades, relações e focos construídos pelo performer.

## Separação de responsabilidades

| Camada | Responsabilidade | Não pode fazer |
|---|---|---|
| Campo espacial | posições, distâncias, focos, rotas e relações visíveis | ser apagado por célula, recall ou geração implícita |
| Jogo da seed | regras, papéis, fluxos, afinidades, resistências, riscos e memória | virar sorteio sem causa ou roteiro fixo |
| Módulos | interpretar o jogo e gerar matéria, eventos e transformações | perder sua identidade por preset genérico |
| Performer | desenhar, nutrir, interromper, aceitar, contrariar e redirecionar | perder o controle sem ação explícita |
| Memória/narrativa | registrar consequências e propor o próximo acontecimento | substituir a autoria espacial |

## Ciclo causal

```text
seed cria o jogo
  -> performer configura o campo
      -> relações espaciais condicionam as jogadas
          -> módulos geram acontecimentos situados
              -> memória atualiza tensões e possibilidades
                  -> performer escuta e remodela o campo
```

Uma jogada situada precisa ter: origem na seed ou no estado acumulado, alvo
musical identificável, influência do campo atual, consequência bounded e estado
observável. A geração pode surpreender, mas não pode ser arbitrária.

## Abertura: módulos e estratégia são o motor da seed

A seed não começa como número abstrato para depois receber som. Como no Studio,
ela compila uma definição inicial que contém módulos audíveis, papéis, estados
de matéria/tempo, relações possíveis e uma estratégia composicional de jogo.
Essa estratégia pode orientar os corpos a seguir, transformar, contestar,
interromper, ocultar ou negociar. É desse conjunto em execução que nasce a
trajetória.

Somente os módulos necessários à abertura entram automaticamente no campo; os
complementares já adaptados e pertinentes ficam em reserva externa para
admissão explícita pelo músico. Não há cartão sem stem nem estratégia que seja
apenas texto: ambos precisam participar causalmente dos acontecimentos.

O performer não apenas corrige ou acompanha esse plano. Ele conversa com a
seed: aceita, contradiz, perverte, rompe, repara, desvia e recombina suas
proposições no campo. A seed responde através dos módulos e da memória do jogo.
A trajetória é o histórico dessa composição recíproca.

## Pausa de manipulação

“Sem manipulação” significa ausência temporária de gesto, não ausência de
autoria nem autorização para trocar de cena. O motor deve:

- continuar os módulos e as relações presentes;
- desenvolver acontecimentos com repetição e diferença;
- ler distância, foco, rota e intensidade das relações;
- acumular memória e alterar tensões de forma gradual;
- preservar posições e organização espacial;
- devolver ao performer um campo audivelmente transformado.

Ao retomar o gesto, o músico pode responder ao que ouviu. A próxima ação deve
operar sobre o estado resultante, e não reiniciar uma narrativa escondida.

Na implementação inicial, a pausa é reconhecida por um marcador de atividade
do performer. Gestos, macros, posição, relações, presença, controles locais,
mixagem e transporte reiniciam esse marcador. A jogada autônoma só é avaliada
depois de dois segundos sem atividade, e apenas quando `autonomy` não está
abaixo de `intervention`. Esse intervalo é uma primeira política de segurança
musical, não uma duração definitiva.

## Critérios para implementação

1. A mesma seed em dois campos espaciais distintos produz respostas
   perceptivelmente distintas, mantendo a identidade da seed.
2. O mesmo campo sem manipulação não repete indefinidamente um ciclo curto:
   há desenvolvimento, contraste, acúmulo e retorno transformado.
3. Uma pausa não move, remove ou substitui módulos e relações.
4. Uma intervenção espacial altera o jogo de modo causal e audível.
5. Uma ação de célula, gesto ou recall não apaga o campo; é bounded,
   determinística e desfazível.
6. O estado narrativo retornado ao performer é legível sem transformar a mesa
   em um painel de logs.

## Ordem de implementação

1. Auditar no Studio os mecanismos canônicos de fluxo, memória, jogo e
   acontecimentos que serão traduzidos; não usar `PerformanceSeedTrajectory`
   como roteiro substituto.
2. Definir um contrato puro de `SpatialGameState` que leia somente um snapshot
   bounded do campo e produza uma intenção de jogada determinística.
3. Fazer uma única coorte de módulos interpretar essa jogada, sem mover a
   geometria.
4. Testar A/B da mesma seed em dois campos, continuidade sem gesto e retorno
   após pausa.
5. Só depois substituir a grade de 16 células por ações que intervenham nesse
   jogo, preservando checkpoint e undo.

O documento é uma direção de implementação. A execução só será considerada
musicalmente válida após teste técnico e escuta humana.
