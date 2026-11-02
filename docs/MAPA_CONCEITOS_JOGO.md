# Mapa conceitual de jogo — Rasgo Synth Performance

Este mapa registra a direção de projeto, não regras fechadas. A fonte conceitual
é a pesquisa já realizada no Rasgo Synth Studio; cada implementação deverá
apontar para o documento de origem correspondente.

| Conceito na mesa | Tradução performática inicial | Evidência exigida |
|---|---|---|
| Reordenamento | trocar a posição/ordem de corpos e suas relações | mudança audível de percurso ou timbre |
| Confusão | embaralhar temporariamente relações ou destinos | leitura visual e sonora de instabilidade |
| Movimento próprio | corpo se deslocar sem arrasto contínuo do músico | movimento bounded, reversível e audível |
| Atração / repulsão | distância modular a mistura ou modulação cruzada | simbiose próxima e autonomia distante |
| Blefe / disputa | tensão relacional entre corpos | consequência musical, sem pontuação obrigatória |
| Colapso / reparo | ruptura e reconstrução de relações | transição sonora e estado recuperável |

Os referenciais teóricos (incluindo Caillois) serão vinculados às fichas de
implementação à medida que os documentos do Studio forem localizados e
comparados. Nenhum conceito será convertido diretamente em widget ou placar.

### Estado da ponte em 2026-08-31

A primeira tradução aplicada ao Performance está em
`PerformanceSpatialGame.hpp`: cada jogada autônoma carrega uma estratégia e
uma regra derivadas deterministicamente da seed e do turno. Essa política
orienta a ação no campo e conversa com a trajetória da seed, mas permanece
bounded e não altera posições, presença, slots ou relações escolhidas pelo
performer. `GamePlan.hpp` continua sendo a fonte conceitual do Studio; a
implementação do Performance é uma fronteira local, sem importar o renderer
nem o estado de sessão do Studio.

Ainda pendem escuta A/B humana e validação em dispositivo de áudio real. A
próxima expansão de módulo só será feita depois de auditar contrato realtime,
proveniência, controles, relação espacial e reversibilidade.

## Referências já confirmadas no código do Studio/Core

- `rasgo-synth-core/src/engine/GamePlan.hpp`: Caillois como campo de quatro
  qualidades (`alea`, `agon`, `ilinx`, `mimicry`), com estratégia, regra e
  recursos composicionais.
- `MaterialAgency.hpp`: agência do material, foreground, reivindicação,
  persistência e reação entre famílias sonoras.
- `IntermodularOrchestra.hpp`: verbos relacionais entre módulos (`follow`,
  `transform`, `counter`, `aggress`, `conceal`, `negotiate`) e disposições de
  harmonia.
- `SpatialExpressionField.hpp`: polaridade, profundidade, deriva, tensão,
  panorama e largura como expressão espacial bounded.
- `StructuralTransformation.hpp`: variação, glitch e ruptura com continuidade,
  magnitude e resíduo mensuráveis.
- `CompositionalTopology.hpp`: regiões, lados e modos de passagem (`preserve`,
  `invert`, `difference`, `accumulate`, `fold`) para reorganização estrutural.
- `GamePlan.hpp` e `PieceRenderer.hpp` (fases 80/118): o jogo como política
  composicional determinística, não gamificação; os pesos de Caillois orientam
  estratégias, regras, memória, risco, liberdade e informação oculta.
- `PieceRenderer.hpp` (fase 85): repetição com diferença, explicitamente
  associada a Deleuze, preservando identidade enquanto cada repetição se afasta
  um pouco da anterior.
- `PieceRenderer.hpp`: phasing inspirado em Steve Reich, caos determinístico,
  Xenakis/sieves, microtonalidade, polimetria, transformação estrutural,
  contágio, confiança e sinais de blefe como mecanismos composicionais reais.
- `Plano_de_Upgrade_Instrumento_Generativo.md` (seção de aprofundamento do
  jogo) e `docs/pt/_catalogo_mecanismos.md`: Huizinga (*Homo Ludens*), Caillois
  e Michel de Certeau (estratégia/tática), além de regras, informação,
  confiança, transgressão e jogo ilegível como matéria composicional — não como
  sistema de pontuação.
- `RASGO_SYNTH/docs/INTERMODULAR_LAB.md`: fonte direta para a mesa relacional
  do Performance. O Studio já define participantes que percebem e são
  percebidos, sensibilidade, resistência, permeabilidade, fadiga, memória,
  aceitação/tensão/recusa e consequência com atraso e limites; a relação não
  precisa ser uníssono, clock comum ou resposta imediata. As experiências
  propostas incluem bateria→matéria da voz, voz→espaço, saída→decisão e
  áudio→waveshape, sempre com contrafactual A/B e telemetria causal.

Essas referências são matéria de projeto para o Performance; não são ainda
prova de que cada comportamento esteja exposto no host de palco.
