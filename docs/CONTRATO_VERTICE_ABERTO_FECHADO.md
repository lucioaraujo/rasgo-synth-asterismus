# Contrato de vértice aberto/fechado — 2026-09-07

## Decisão

Cada corpo da Constelação possui um estado performativo explícito:

- **aberto**: pode receber e propor novas conexões quando o músico desenha,
  aproxima corpos ou ativa uma relação;
- **fechado**: conserva todos os cabos já existentes, mas recusa a criação ou
  reativação de novos cabos que incidam sobre ele.

Fechar não apaga posição, presença, módulo, stem, relações existentes, seed,
cena, desenho coletivo ou gravação. Abrir tampouco reconstrói automaticamente
cabos recusados: o músico decide a próxima ligação.

## Consequência musical

O estado não é apenas gráfico. Um vértice fechado mantém os cabos existentes,
mas reduz sua permeabilidade para novas respostas de relação: a matéria do
corpo preserva sua voz e sua posição, enquanto intercâmbio, resposta e
propagação relacional entram em regime de contenção. Assim ele funciona como
um ponto de estabilização do polígono, e não como mute nem desconexão.

## Interação proposta

O cartão e o nó mostram `ABERTO` ou `FECHADO`. Com um nó em foco, um comando
explícito alterna o estado. O comando deve ter tooltip e retorno no monitor de
ação; não pode depender de clique ambíguo que dispute com arraste.

## Contrato realtime e memória

1. O estado entra em `PerformanceActorState` e no morph de cena.
2. O controller o envia por fila bounded ao engine; nenhuma alocação, busca ou
   escrita de documento ocorre no callback.
3. O ecology usa o estado somente como multiplicador bounded da permeabilidade
   relacional, sem remover relação nem alimentar oscilador contínuo.
4. Captura/desfazer e recall preservam o estado por cena.
5. A serialização deve ganhar versão compatível: documentos antigos assumem
   `aberto`.
6. A interface bloqueia apenas novas conexões; cabos existentes continuam
   editáveis e audíveis.

## Critérios de pronto

- fechar um vértice não altera desenho, presença ou cabos existentes;
- tentar criar cabo novo no vértice fechado produz recusa legível;
- abrir não recria cabos automaticamente;
- A/B do mesmo cabo com vértice aberto/fechado apresenta diferença musical
  bounded, sem corte de áudio;
- cenas, seed e documento salvo restauram o estado;
- regressões de engine, controller e documento passam antes da escuta humana.
