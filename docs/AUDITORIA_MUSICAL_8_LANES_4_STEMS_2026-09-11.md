# Auditoria musical dos 8 módulos internos — 4 stems

Data: 2026-09-11

Projeto: Rasgo Synth Performance

Escopo: integração interna já concluída; limite operacional mantido em quatro stems.

## Estado da evidência

Esta rodada auditou os caminhos técnicos no adapter, na ecologia, na
distribuição e no mixer. A leitura musical em host ainda não foi realizada:
o preflight encontrou `DISPLAY=:0` sem servidor X acessível e nenhum `/dev/snd`.
Os itens abaixo descrevem capacidades e rotas verificadas no código, não uma
afirmação de qualidade auditiva.

O mixer mantém oito lanes internas em quatro buses com o mapa fixo:

| Lanes internas | Stem de saída |
|---|---|
| 0 e 4 | 0 |
| 1 e 5 | 1 |
| 2 e 6 | 2 |
| 3 e 7 | 3 |

Não há quinta saída, retorno global ou alocação no callback. As lanes 0–3
representam a camada nativa da coorte; as lanes 4–7 são matérias expandidas,
recolhidas de volta nos mesmos quatro stems.

## Origem ↔ variação das lanes internas

Em 2026-09-12 foi corrigida uma discrepância entre essa topologia e a
interface: os módulos 5–8 eram apresentados como reserva e o controle
`ORIGEM ↔ VARIAÇÃO` ficava indisponível, embora suas lanes já fossem
renderizadas e dobradas. Cada uma dessas quatro lanes agora guarda um valor
de variação próprio e o aplica à sua matéria antes da dobra no stem de destino.
Não cria voz, stem, saída, cabo, thread ou alocação no callback.

O inspector permite selecionar 5–8 para esse único controle; presença,
ganho, mute e solo continuam pertencendo exclusivamente aos quatro stems
audíveis. A regressão foi coberta por teste A/B: variar o módulo 5 altera o
histórico sonoro dobrado e mantém `activeActorCount() == 4`; o controlador
aceita também os IDs 5–8 e rejeita IDs não declarados. A escuta humana dessa
diferença ainda é pendente.

Atualização de 2026-09-12: as lanes 5–8 também receberam seus três controles
locais. Eles passam pela fila realtime e agem na matéria interna antes do
mesmo fold 8→4. A interface as marca como `LANE INTERNA → STEM 1..4 · ATIVA`;
não exibe presença, ganho, mute ou solo inexistentes. A borda amarela fica
reservada ao foco selecionado, nunca como estado permanente de uma lane.
O A/B automatizado foi ampliado para 5, 6, 7 e 8 individualmente: cada uma
altera o histórico audível de sua dobra e preserva quatro corpos ativos.

Correção de interface em 2026-09-12: o inspector local usava indevidamente o
loop dos quatro stems e ocultava os controles das lanes 5–8. Esse percurso foi
separado e agora cobre as oito lanes. A Constelação também volta a desenhar os
oito módulos: Modal, Galo, Atropelo e Deja Vu são nós internos selecionáveis e
posicionáveis. A posição atravessa a fila realtime e modula bounded a camada
dobrada; não cria stem, controle de saída ou voz adicional. O host recompilou
e os testes de engine/controller passaram; falta a confirmação visual humana.

Também foi iniciado o catálogo local Studio → Performance: a biblioteca passou
de 15 para 29 entradas. As 14 matérias já adaptadas e o candidato Modal seguem
distintos de 14 organismos canônicos do Studio, cuja proveniência é preservada
e cujo estado é `NeedsAudit`; eles não aparecem como áudio disponível antes de
receberem adapter e teste próprios.

Tuba/Breath é o primeiro adapter experimental desse próximo ciclo. A adaptação
pertence ao Performance e consulta `ModalResonator` e `BreathExciter` do
Studio/Core apenas como referência. A sua rota direta inicial ao submix foi
retirada após regredir a prioridade de gestos. O ensaio isolado está registrado
no CTest, mas ainda depende da reconfiguração do build; até passar esse ensaio
e a regressão de engine, Tuba não é módulo ativo nem altera os quatro stems.

## Auditoria por módulo

| Módulo | Ritmo | Melodia/harmonia | Timbre | Eventos | Função composicional |
|---|---|---|---|---|---|
| Acid | passo, gate e acento | trajetória tonal e alvo da seed | síntese ácida, brilho e ressonância | relógio de frase e relações | corpo tonal e trajetória |
| Gendyn | irregularidade estocástica bounded | alvo de altura e deriva | matéria dinâmica e instável | variação, eventos e tensão | matéria estocástica |
| Drums/Percussion | pulso, divisão e contágio | suporte, não linha tonal principal | transiente percussivo | clock, gate e acentos | pulso e contágio |
| FM Flute | articulação por frase | linha soprada e intervalos | FM respirada, ataque e decaimento | frase, coda e intervenção | sopro articulado |
| Theremin | continuidade modulada | linha contínua e alvo móvel | timbre portado e tensão | entrada/saída de frase | linha contínua |
| Scat | sílabas e ataques | contorno vocal sintético | formantes e articulação | sílaba, evento e relação | sílabas sintéticas |
| Choir | entradas por evento | intervalos e campo polifônico | massa vocal e formantes | chord/event gate e coda | campo vocal polifônico |
| VOSIM | pulsação formântica | fundação de registro | formante pulsado | excitação fraseada | fundação formântica |
| Walsh | sequência e passo | base ortogonal e registro | espectro digital | sequência, grid e relações | base ortogonal e sequência |
| Hard Sync | ataques e rupturas | razão e ruptura harmônica | sincronismo duro | evento de ruptura e macro | ruptura harmônica |
| Atari Punk | pulso escalonado | pressão rítmica e registro | textura digital áspera | clock, gate e acento | pulso escalonado |
| ModalResonator | excitação pontual | ressonância de memória | registro ressonante e cauda | evento narrativo, memória e decay | vestígio/ressonância |
| Galo | chamada fraseada | chamada e resposta de registro | ressonância vocal/animalizada | chamada e relação | chamada ressonante |
| Atropelo | sílabas e reentrada | voz quase falada | ruído vocal e articulação | controle local e gate | voz quase falada |
| Deja Vu | recorrência e variação | memória estatística do contorno | repetição transformada e cauda | memória, recall e reentrada | memória estatística |

Na configuração corrente, as oito lanes publicadas são: lanes 0–2 da coorte
selecionada, lane 3 como quarto corpo da seed, lane 4 `ModalResonator`, lane 5
`Galo`, lane 6 `Atropelo` e lane 7 `Deja Vu`. A composição das três primeiras
lanes depende do adapter/coorte; a capacidade não autoriza ativar mais de quatro
stems.

## Resultado por tópico

- Ritmo: cobertura técnica presente por clocks, gates, divisões, acentos,
  pulsos digitais e ataques locais. Falta confirmar se as camadas expandidas
  permanecem perceptíveis sem mascarar o pulso principal.
- Melodia: há alvos de altura, trajetórias, intervalos, contornos e linhas
  específicas. Falta confirmar a inteligibilidade da hierarquia melódica no
  submix.
- Harmonia: o caminho inclui dialeto da seed, razões, registro e intervalos
  do Choir. Falta confirmar se o fold 8→4 preserva contraste, sem adensamento
  indistinto.
- Timbre: cada família usa matéria distinta e recebe controles locais, brilho,
  tensão, risco, fricção e rasgo sob limites bounded. Falta confirmar a
  separação tímbrica em reprodução real.
- Eventos: gates de frase, relações, macros, memória, coda, decay e intervenção
  chegam aos módulos; não há alimentação contínua sem função. Falta testar a
  resposta temporal com gesto humano.
- Função composicional: os módulos têm papéis complementares — pulso, trajetória,
  linha, campo, ruptura, chamada, memória e vestígio — e as camadas 4–7 ampliam
  a narrativa sem criar novos corpos visuais ou saídas.

## Veredito conservador

Cobertura técnica dos seis tópicos: aprovada para a próxima escuta. Topologia
8 lanes → 4 stems: aprovada. Limite seguro de produção: quatro stems.
Escuta musical humana: não verificada. Teste de estabilidade longa: inconclusivo
nesta retomada; a seleção não longa executada passou 25/25.

## Próxima escuta autorizada

Quando houver host gráfico e dispositivo de áudio, comparar as oito lanes em
quatro stems com a mesma seed e duração curta. Fazer solo/mute por stem e,
depois, comparar a camada nativa com as matérias expandidas. Registrar decisões
antes de alterar densidade, ganho ou gramática. Não usar STOP/PLAY como
recuperação entre cenas e não elevar o número de stems.

Arquivos de referência: `PerformanceModuleCohortAdapter.hpp`,
`PerformanceActorEcology.hpp`, `PerformanceStemDistribution.hpp` e
`PerformanceStemBusMixer.hpp`.
