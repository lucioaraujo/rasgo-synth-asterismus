# Tuba/Breath — proveniência de adaptação

Fonte somente leitura: `../rasgo-synth-core/src/dsp/ModalResonator.hpp` e
`../rasgo-synth-core/src/dsp/BreathExciter.hpp`, além da configuração de Tuba
em `../rasgo-synth-core/src/engine/PieceRenderer.hpp`.

Destino atual: `TubaBreathVoice.hpp`, protótipo próprio do Performance com
estado fixo e teste offline dedicado. A tentativa de rota direta ao submix
foi retirada por regredir a prioridade dos gestos. Não há fold ativo aprovado
para Tuba/Breath nem promoção da entrada `NeedsAudit` na biblioteca.
Nenhum arquivo do Studio/Core é alterado por esta adaptação.

Uma futura integração deverá respeitar os oito módulos internos, o teto de
quatro stems e a prioridade dos gestos; passar no teste isolado não conclui
esse contrato nem substitui a escuta humana.

Revisões consultadas em 2026-10-06, SHA-256 dos arquivos-fonte somente lidos:

- `ModalResonator.hpp`: `5e87d761f3f9b07ee337234b4e402fb6571e2dba44ce09df4f1593649628b996`.
- `BreathExciter.hpp`: `794e9eaf88e4a8848ac0c1b176c2593fc60b488b8f99da6d2d36cc4320dedf48`.

O protótipo usa um banco modal de seis estados e excitação pseudoaleatória
limitada próprios, sem incluir esses headers do Studio. Não é cópia fiel de
suas APIs nem emulação certificada de uma tuba acústica. A licença do
Performance permanece GPLv3; autoria/referências das fontes são preservadas.
