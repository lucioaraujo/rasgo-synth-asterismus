# Mesa relacional — especificação inicial

Este documento traduz para o Performance os princípios já implementados ou
documentados no Studio/Core, especialmente `INTERMODULAR_LAB.md`,
`IntermodularOrchestra`, `MaterialAgency` e `SpatialExpressionField`.

## Unidade de um módulo

Cada módulo inserido na mesa possui:

- identidade e adapter realtime;
- estado de presença (reserva, emergente, ativo, recolhido, cicatriz);
- posição espacial normalizada;
- canal interno de medição/mixagem, sem obrigar isolamento na saída final;
- controles contextuais próprios;
- memória curta de encontros e fadiga;
- limites de ganho, densidade e custo de CPU.

## Relação entre dois módulos

Uma relação é calculada a partir de distância, direção, energia observada e
histórico. A distância não é um simples pan:

| Distância | Comportamento esperado |
|---|---|
| próxima | maior permeabilidade, mistura/modulação cruzada e simbiose |
| intermediária | resposta parcial, tensão ou negociação |
| distante | menor influência; cada corpo conserva sua trajetória |

O receptor pode aceitar, transformar, resistir, recusar ou responder com atraso.
Nenhuma relação pode impor uníssono, clock comum ou cópia direta de pitch sem
que o conceito e a consequência estejam documentados.

## Fluxo de uma ação do músico

`inserir/recolher → atualizar posição → calcular relações bounded → transformar
controle/áudio → medir consequência → registrar memória/telemetria`.

Inserção e recolhimento são comandos de controle enfileirados; o callback só
consome valores fixos. Não há alocação, mutex, I/O ou busca de catálogo no
processamento por amostra.

## Primeiro protótipo implementável

1. Manter os três corpos iniciais da seed.
2. Permitir inserir um quarto módulo da biblioteca.
3. Dar a cada corpo posição e presença independentes.
4. Implementar uma única relação espacial comprovável (energia do corpo A
   modulando densidade ou drive do corpo B com slew e limite).
5. Renderizar contrafactual com a relação desligada e comparar pós-mix.
6. Exibir o inspetor contextual do corpo selecionado.

O protótipo só será ampliado após teste de finitude, pico, CPU/xrun, estabilidade
e escuta humana. A meta é provar uma relação musical real, não preencher a mesa
com controles.

## Preservação do campo desenhado

O posicionamento dos módulos é estado performático autoral. Depois que o músico
move um nó ou modela uma relação no campo, o arranjo permanece vigente durante
a sessão. Células de gesto, macros, atualização de rótulos e sincronização de
valores de uma cena não podem chamar uma projeção geométrica automática.

Somente estas ações podem alterar a geometria:

- arrastar explicitamente um nó ou um cabo;
- alterar explicitamente um slider de relação que tenha essa função declarada;
- iniciar uma nova sessão/território, quando a reconstrução inicial é
  intencional e documentada.

O recall das células `CENA 1`–`CENA 4` pode trocar valores de sessão, presença e
relações, mas preserva as posições vivas dos nós. Essa distinção evita que o
músico perca uma cartografia que acabou de desenhar ao disparar uma célula.
