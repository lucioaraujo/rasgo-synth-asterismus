# Auditoria Studio → Performance: módulos, jogo e trajetória

**Data:** 2026-09-02
**Escopo:** fontes canônicas já existentes no Studio/core que devem alimentar a
próxima abertura do CAMPO.
**Regra:** Studio/core são fontes de estudo somente leitura. Código e conceitos
corretos podem ser copiados de modo controlado para o Performance e adaptados
somente no necessário; lacunas performáticas podem receber código novo no
Performance. Não incluir `PieceRenderer` no callback nem alterar o Studio.

## Conclusão

O Studio já contém a base conceitual e executável necessária para a primeira
prova: seleção seed-dependente de organismos realmente manifestados,
estratégias e regras de jogo, agência material, relações com resistência e
permeabilidade, arco generativo e coda. O Performance ainda não recebe todo
esse estado. A ponte agora transporta o `GamePlan` completo, mas ainda não
transporta relações iniciais nem a identidade concreta de cada módulo sonoro.

Portanto, a prioridade não é inventar outra narrativa. É ampliar de forma
versionada o contrato Studio/core → Performance e ligar os adapters já
existentes a esse conteúdo.

## Fontes auditadas

| Fonte do Studio/core | O que já resolve | Como aproveitar no Performance |
|---|---|---|
| `rasgo-synth-core/src/engine/GamePlan.hpp` | estratégias `Follow`, `Transform`, `Counter`, `Interrupt`, `Conceal`, `Negotiate`; regras `Preserve`, `Mutate`, `Oppose`, `Withhold`, `Exchange`, `Break`; energia, memória, risco, liberdade e informação oculta; densidade e probabilidade de mudança causal | extrair/consumir como contrato puro versionado; não reduzir estratégia a uma string no log |
| `rasgo-synth-core/src/engine/PerformanceActorRegistry.hpp` | admite somente organismos ativos, audíveis e não infraestruturais; declara família, agência, capacidades, saliência e orçamento de gesto; faz seleção determinística | usar como filtro de ingresso e disponibilidade; manter ausentes realmente ausentes e impedir cartões sem áudio |
| `rasgo-synth-core/src/engine/RasgoTerritoryManifest.hpp` | hand-off estreito, somente leitura, de seed, BPM e atores manifestados | evoluir para um contrato versionado capaz de declarar estratégia/regra, recursos do jogo, identidade concreta e relações iniciais sem transportar DSP, PCM ou estado interno do Studio |
| `rasgo-synth-core/src/engine/IntermodularOrchestra.hpp` | traduz estratégia em verbo relacional e realiza sensibilidade, resistência, permeabilidade, memória, fadiga, presença e resposta assimétrica | adaptar relações entre corpos reais do CAMPO; distância/profundidade entram como contexto, sem substituir a agência do organismo |
| `rasgo-synth-core/src/engine/MaterialAgency.hpp` | organismo interpreta o plano segundo família, estado, energia e histórico | reutilizar como base para a seed responder ao músico, em vez de aplicar o mesmo macro a todos os stems |
| `rasgo-synth-core/src/engine/OpenModuleArchive.hpp` e a topologia criada em `PieceRenderer.hpp` | regiões, passagens, domínios, resistência e transformação; rasgo altera rotas | traduzir passagens para conexões/fluxos visíveis do CAMPO e preservar vestígios |
| `rasgo-synth-core/src/engine/RasgarGesture.hpp` | rasgo situado pela seed, identidade, jogo e topologia | reaproveitar como operação causal sobre rota/corpo, não efeito global solto |
| `rasgo-synth-core/src/sequencer/GenerativeArc.hpp` e `ArcTemplates.hpp` | arco vivo chega à coda por estado, não por lista fixa; papéis e famílias de coda já existem | adaptar abertura, continuidade e conclusão da tomada performada, permitindo chamada autoral e epílogo por inatividade |
| `rasgo-synth-core/src/engine/PieceRenderer.hpp` | ponto de integração comprovado: cria `GamePlan`, topologia, relações, sequencer, organismos, arco e coda; registra metadados | usar como referência de integração e fonte para extração de contratos; não importar o renderer monolítico para o Performance |

## O que o Studio já demonstra

1. `GamePlan` é política composicional, não coleção de efeitos temáticos.
2. Estratégia e regra são diferentes: uma oposição pode preservar-se, trocar
   papéis ou romper; o jogo não cabe num único nome.
3. A seed distribui energia, memória, risco, liberdade e informação oculta por
   fluxos determinísticos independentes.
4. Estratégias alteram densidade, relações, arranjo, presença e decisões dos
   organismos ao longo do tempo.
5. A orquestra relacional não força obediência: usa resistência,
   permeabilidade, memória, fadiga e presença.
6. O registro de atores exclui infraestrutura e qualquer organismo que a seed
   não tenha realmente manifestado.
7. O arco generativo decide quando alcança a coda; a conclusão possui gesto e
   cauda, não apenas corte técnico.

## Fronteira reafirmada — 2026-09-03

O Performance já é autônomo; não se tornará autônomo no futuro. Studio e
Performance são irmãos gêmeos do Rasgo Synth: compartilham genealogia,
pesquisa, conceitos e código reutilizável, mas têm fontes, builds, estados e
responsabilidades próprios. A direção correta é copiar para o Performance a
matéria já correta no Studio quando ela servir, ou criar nele a matéria nova que
o campo performático exigir; nunca alterar o Studio para completar uma ponte.

A experiência de `RSTM 2` abaixo alterou indevidamente o core do Studio e não
estabelece precedente arquitetural. Ela fica registrada como estado factual até
que o autor autorize explicitamente sua reversão; o desenvolvimento do
Performance não pode depender dela.

## Registro técnico transitório: RSTM 2

Em 2026-09-02, o `RSTM 2` passou a acrescentar, preservando leitura de
`RSTM 1`, a estratégia, a regra e os cinco recursos do `GamePlan` que abriram
a peça no Studio. O `PieceRenderer` emite esse snapshot real; o importador do
Performance o valida, a sessão o persiste em `RSPERF 2` e o
`PerformanceSpatialGame` o usa nos turnos autônomos. Assim, `Interrupt` e
`Break`, por exemplo, não chegam como texto: orientam a escolha e a força do
acontecimento aplicado à ecologia, sem mover âncoras ou reconfigurar o campo.

O campo criado diretamente no Performance recebe uma política determinística
equivalente como fallback. Ao importar `RSTM 2`, o snapshot do Studio substitui
esse fallback; uma nova configuração só por seed reinicia a política para não
vazar o jogo de um território anterior.

Os três atores necessários à abertura da coorte nativa agora entram presentes
e audíveis. O quarto permanece reserva explícita: não se finge que a seed
escolheu um corpo adicional quando ele ainda depende de gesto do performer.

## Limite remanescente da ponte

O formato `RSTM 1` contém:

```text
seed + BPM + [id de ator, família gestual, capacidades,
              saliência, audibilidade observada, orçamento]
```

Ele não contém:

- `GameStrategy`, `GameRule`, energia, memória, risco, liberdade e informação
  oculta;
- identidade nominal/concreta do módulo dentro de agregados como tom, textura
  ou ruptura;
- papéis performáticos do CAMPO;
- afinidades, resistências, relações e passagens iniciais;
- relógios, tendências narrativas e possibilidades de coda.

Além disso, o registro construído hoje no `PieceRenderer` agrupa organismos
manifestados em seis atores de capacidade — hit, textura, tom, pattern,
ruptura e relação. Isso é honesto para auditoria, mas ainda não equivale ao
catálogo de módulos concretos que o músico deve ver e arrastar no CAMPO.

## Próxima implementação P0

1. Escolher uma seed-fixture já conhecida no Studio e registrar seu arco,
   organismos concretos e relações realmente manifestadas.
2. Estender o contrato puro de abertura, que já preserva `RSTM 1` e transporta
   em `RSTM 2` estratégia, regra e recursos, com módulos concretos, papéis e
   relações iniciais — sempre sem estado DSP.
3. Mapear cada módulo escolhido para um adapter/stem já executável no
   Performance; qualquer lacuna permanece visível e não ganha cartão falso.
4. Fazer somente os módulos necessários à abertura aparecerem e soarem no
   CAMPO; complementares pertinentes ficam na reserva externa.
5. Fazer a estratégia atuar nos stems e relações desde a primeira frase.
6. Provar o diálogo: o músico segue, contradiz, perverte, rompe e repara; a seed
   responde por módulos, memória e jogo sem mover âncoras.
7. Registrar a performance do início ao epílogo e comparar o áudio/trace com a
   mesma seed no Studio, sem exigir que as duas obras sejam idênticas.

## Critério de pronto

A etapa só termina quando a interface mostra módulos concretos que estão
audíveis, a estratégia muda acontecimentos reais, o performer consegue
contradizê-la sem perder o campo e a seed responde de modo reconhecível. Teste
de contrato e diferença numérica é necessário, mas a promoção depende de
escuta humana com a composição completa.
