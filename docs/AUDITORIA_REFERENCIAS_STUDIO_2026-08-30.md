# Auditoria das referências do Studio — 2026-08-30

## Escopo verificado

Foi inventariado o conjunto local de referências do Rasgo Synth Studio/Core:
62 arquivos Markdown/TXT fora de diretórios de build, incluindo documentação
em português, inglês, francês e espanhol, o plano de upgrade, laboratório
intermodular, manifestos, manuais, genealogia, auditorias de fases, catálogo de
mecanismos, pacotes CMake e registros de render.

As traduções foram tratadas como versões de acesso; a documentação portuguesa e
o código do Core permanecem as fontes prioritárias de decisão técnica.

## Núcleos de referência encontrados

- **Conceito e genealogia:** `docs/pt/Manifesto.md`,
  `Genealogia_do_Instrumento.md`, `Documento_de_Referencia.md` e
  `Plano_de_Upgrade_Instrumento_Generativo.md`.
- **Método e relações:** `docs/pt/Metodo_RASGO.md`,
  `INTERMODULAR_LAB.md`, `Reavaliacao_Modular_Pos_Fase_120.md` e
  `Orquestra_Relacional_Fase_121.md`.
- **Auditorias:** fases 118, 119, 120, 122 e 123, com evidências de diversidade,
  agência tímbrica, poesia dos módulos, pós-espaço e agência relacional.
- **Mecanismos e arquitetura:** `docs/pt/_catalogo_mecanismos.md`,
  `docs/pt/Arquitetura_Fluxograma.md`, `rasgo-synth-core/src/engine/` e
  `rasgo-synth-core/src/dsp/`.
- **Jogo e composição:** `GamePlan.hpp`, `BluffSignal.hpp`, `MaterialAgency.hpp`
  e a seção do plano que registra Huizinga, Caillois, de Certeau, blefe,
  confiança, transgressão e informação incompleta.
- **Orquestração relacional:** `IntermodularOrchestra.hpp`,
  `SpatialExpressionField.hpp`, `StructuralTransformation.hpp`,
  `CompositionalTopology.hpp`, memória de seção e auditoria de organismos.

## Estado para o Performance

O levantamento confirma que o Studio/Core já contém uma linguagem de
instrumento generativo e relacional muito mais ampla que a coorte atual do
Performance. O Performance deve adaptar essa linguagem por contratos bounded e
testes, preservando a fonte e sem editar o Studio. A lista de fontes e o mapa de
tradução estão em `MAPA_CONCEITOS_JOGO.md` e `MESA_RELACIONAL_DESIGN.md`.

## Limite da auditoria

O inventário e a leitura dirigida cobriram todos os arquivos de referência
textual e os pontos de código conceitualmente relevantes. Não foi feita uma
leitura manual linha a linha de cada arquivo-fonte extenso (especialmente
`PieceRenderer.hpp`); decisões futuras devem voltar ao trecho de origem antes
de adaptar qualquer mecanismo.

## Decisão operacional para o Performance

Esta auditoria dirigida está concluída como base de arquitetura, mas não deve
ser confundida com uma certificação exaustiva de todo o catálogo. O inventário
de 62 documentos localizou as fontes de conceito, método, genealogia, jogo,
orquestração e mecanismos; a classificação individual das 142 fichas do
catálogo e a auditoria técnica dos 67 headers DSP/engine continuam pendentes.

O aproveitamento no Performance seguirá quatro classes, sempre preservando a
fonte e sua proveniência:

1. **Adaptável agora:** mecanismo com comportamento comprovado, estado
   bounded, `prepare/reset/process`, determinismo e consequência audível. Entra
   por package versionado, adapter próprio, testes e poucos controles
   contextuais.
2. **Candidato posterior:** mecanismo ou voz promissor que precisa de escuta,
   medição, revisão de estado ou contrato realtime antes de entrar na mesa.
3. **Recurso interno:** relação, política, memória, transformação ou regra de
   composição que será traduzida em comportamento do território, dos stems e
   das relações — não em um widget isolado.
4. **Offline/observador/infraestrutura:** material útil para renderização,
   análise, documentação ou suporte, mas inadequado para execução contínua no
   callback do Performance.

Já foram aproveitados pela primeira classificação operacional os 14 corpos das
coortes Circuito, Sopro, Voz, Digital e Memory. O próximo aproveitamento não
será uma importação em massa: primeiro escuta e calibração humana de Digital e
Memory; depois classificação das 142 fichas e auditoria dos 67 headers; em
seguida seleção de um novo adapter/coorte bounded. A ponte Studio → Performance
só será ativada quando houver manifesto versionado, contrato de recursos,
teste determinístico, teste de estabilidade e consequência musical verificável.

Assim, o Studio permanece fonte de linguagem e mecanismos, enquanto o
Performance mantém sua própria superfície, estado, engine, builds e arquivos
de usuário. Nenhum módulo deve ser copiado diretamente para o callback ou
exposto na mesa apenas por existir no catálogo.
