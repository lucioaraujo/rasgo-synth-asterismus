# Auditoria de saneamento do Performance — 2026-09-04

## Critério

Esta auditoria não considera uma tarefa resolvida apenas porque compila ou
porque produz algum sinal. O controle precisa corresponder ao conceito do
Performance, ter consequência musical identificável e preservar o campo do
músico.

## Problemas encontrados e ação

| Área | Achado | Ação | Estado |
|---|---|---|---|
| Áudio | A ecologia nativa misturava 16% de uma voz senoidal provisória por baixo do módulo real | remover a camada `provisional` quando a coorte real está disponível | corrigido no código; falta escuta |
| Áudio | `ESPAÇO` acrescentava uma senoide pura aos canais | substituir por ponderação do material já produzido pelos stems | corrigido no código; falta escuta |
| Áudio | `NUTRIÇÃO` alimentava permanentemente ganho e corpo nativo | retirar nutrição do ganho-base e reservá-la à ecologia/eventos | corrigido no código; regressão de solo corrigida com ganho-base fixo |
| Áudio | frequência-alvo era fixa durante a pausa | adicionar contorno lento derivado da seed e do movimento | corrigido no código; falta escuta |
| Interface | seleção dos módulos usava hit-test estreito e alvo anterior podia persistir | alvo invisível de 58 px e limpeza explícita de `activeNode_` | corrigido e layout validado |
| Topologia | gráfico visual tinha seis cabos para uma coorte que declara dez relações | ampliar para dez pares canônicos | corrigido e engine validado |
| Governança | documentação afirmava causalidade antes da escuta humana | marcar separadamente validação técnica e escuta pendente | corrigido neste documento |

## Limitações ainda abertas

- A escuta no host ainda é necessária para confirmar que a retirada do drone e
  a variação de eventos são musicalmente adequadas.
- O teste de engine comprova segurança e diferenças numéricas, mas não prova
  que a mudança é agradável ou suficientemente clara para o performer.
- Os quatro novos cabos possuem leitura contextual, mas ainda não têm quatro
  knobs dedicados; não se deve abrir novos controles antes de ouvir a topologia.

## Validação desta sessão

- host Release recompilado;
- teste de layout aprovado;
- `git diff --check` aprovado;
- nenhum arquivo do Studio foi alterado nesta sessão.

Próximo passo: executar uma tomada curta no host, comparar `ESPAÇO`,
`TENSÃO` e `NUTRIÇÃO` em valores baixos/altos e verificar se o campo produz
eventos e variações, não uma frequência sustentada.

## Fechamento técnico da rodada

Depois da remoção das camadas contínuas, foram corrigidos o ganho-base dos
módulos reais e o vazamento de `Recall` para stems não selecionados. O ganho
base agora é independente de nutrição; `AUTONOMIA` chega ao relógio/evento da
coorte real; e a seleção de módulo usa alvo de interação de 58 px.

Validação final: app Release recompilado e 23/23 CTests aprovados. A aprovação
musical continua deliberadamente pendente da escuta humana no host.

## Verificação dos knobs — 2026-09-05

Os sete knobs visíveis seguem `Slider::onValueChange` → `setMacroControl` →
`PerformanceSessionController::setMacro`, com limites de território aplicados
a MEMÓRIA e RISCO. A suíte comprova que cada macro produz diferença renderizada
não trivial e que o controller rejeita valores fora do intervalo. O rótulo
enganoso `NUTRIÇÃO CONTÍNUA` foi substituído por `MACROS DE VARIAÇÃO · EVENTOS`.

Validação: Release recompilado; engine e controller passaram (2/2 testes
selecionados); `git diff --check` aprovado. Ergonomia e clareza sonora ainda
dependem da manipulação no host.

O ajuste de intensidade do satélite agora também dispara um pulso curto quando
ele está inserido. Assim, o diâmetro não é somente decoração visual: sua
alteração chega imediatamente ao stem fonte, sem reintroduzir sustentação
permanente.
