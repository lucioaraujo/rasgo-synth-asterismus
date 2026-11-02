# Controlador de sessão — fronteira de host

**Estado:** implementado; concorrência e transação endurecidas em 2026-08-28
**Escopo:** controle próprio do Performance entre documento, cenas, gestos e
motor; não é GUI, app JUCE, gravação ou integração com o Studio.

## Função

`PerformanceSessionController` é a fachada do thread de controle do host
standalone próprio. Ele prepara seu `PerformanceSessionEngine`, recebe um
`PerformanceSessionDocument` já validado, escolhe a primeira cena ou uma cena
indicada, recupera cenas por morph bounded e encaminha gestos ao motor.

O controlador mantém duas cópias fixas do documento: `controlDocument_` só é
lido/escrito pelo produtor de controle e `audioDocument_` só pelo callback.
Captura e território carregam uma cópia bounded na fila SPSC; nenhuma thread
acessa o documento mutável da outra. Não há alocação, I/O ou interface gráfica
nessa camada. `prepare` e carregamento exigem o áudio parado. Um novo território
de seed também só pode ser enfileirado com o motor parado e atravessa uma fila
SPSC fixa de duas mensagens. Depois disso, recall de cena atravessa uma fila
SPSC fixa de 32 comandos:
GUI ou MIDI somente enfileiram; o início do morph acontece no começo de
`processBlock`. Assim, nenhum produtor de controle altera diretamente o estado
do motor durante um bloco. Território é aplicado antes do transporte no mesmo
callback; um PLAY imediato já pertence à seed nova.

## Regras verificadas

- um documento não carrega antes de `prepare`;
- a primeira cena válida torna-se a cena atual quando nenhuma é indicada;
- cenas inexistentes e morphs abaixo do limite seguro são recusados;
- recall válido é enfileirado e o callback inicia o morph; a telemetria só
  troca para a cena alvo quando o morph alcança seu fim;
- gestos só passam depois de documento carregado;
- `Stutter` e `TempoShift` usam a fila fixa de gestos já existente; o
  controlador somente valida/enfileira o evento e não calcula DSP, duração ou
  coordenadas no thread de controle;
- território inválido, cena inicial ausente ou troca com áudio em curso são
  recusados; uma troca válida substitui o documento antes do bloco seguinte e
  reinicia a mesa, preservando MASTER;
- fila cheia não publica território/captura rejeitados na cópia de controle;
- seed 0 permanece zero entre factory, painel, documento, adapter e DSP;
- a ecologia da cena sobrevive ao primeiro bloco de áudio;
- nenhuma classe, header, estado ou caminho do Studio participa.

## Evidência

`tests/test_performance_session_controller.cpp` valida esse fluxo. O factory
tem teste próprio para determinismo, diferença entre seeds e estado de
transporte. Em 2026-08-28, o host completo `v0.1.1-dev` passou 8/8 CTests
não-stress com as três ecologias. Motor/controlador passaram também ASan e
UBSan; o stress de 20 minutos anterior não foi repetido neste incremento.

## Próximo passo

Validar manualmente seed 0 e STOP → NOVO TERRITÓRIO → PLAY. MIDI, medição em
áudio real e coalescência opcional de gestos contínuos permanecem posteriores.
O núcleo continua independente de arquivos do Rasgo Synth Studio.
