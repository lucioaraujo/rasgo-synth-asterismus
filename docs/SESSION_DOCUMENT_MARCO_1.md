# Documento de sessão — Marco 1

**Estado:** implementado e testado — 2026-08-24
**Escopo:** `PerformanceSessionDocument` próprio; sem JUCE, sem callback de
áudio e sem dependência de arquivo, código ou estado do Rasgo Synth Studio.

## Decisão

`PerformanceSessionDocument` persiste o território imutável de uma sessão e
até oito cenas ordenadas. Cada cena contém seu estado macroscópico, constelação
de até oito atores e até dezesseis relações. O formato é deliberadamente
pequeno: ele salva condições musicais recuperáveis, não uma cópia da GUI nem um
render, recording ou preset do Studio.

O primeiro formato canônico é texto `RSPERF 1`. A versão faz parte do cabeçalho
e uma versão desconhecida falha de modo explícito. A leitura aceita no máximo
64 KiB e valida contagens antes de preencher estruturas. O parser não é
realtime-safe e só pode ser chamado em controle/I/O antes de entregar snapshots
ao motor.

## Conteúdo e limites

- `PerformanceTerritory`: seed, seis valores de identidade e até 96 potenciais
  de ator;
- até 8 `PerformanceScene` com a mesma seed do território;
- por cena: até 8 estados de ator e 16 relações;
- enums, IDs, ranges e números não-finitos são verificados pelos contratos já
  existentes;
- nomes de cena e metadados de interface ficam fora de `RSPERF 1`: são
  informação editorial/UI e só entram quando houver requisito de recuperação
  definido, com uma migração de formato nova.

## Fronteira

O header está em
`include/rasgo/performance/PerformanceSessionDocument.hpp`. Ele não inclui
headers do Studio, não conhece `Project v2` do Navalha e não cria dependência
de caminho para projetos irmãos. Qualquer matéria proveniente do Studio que
venha a ser necessária no futuro deverá existir como artefato próprio do
Performance, com proveniência, revisão e licença registradas; o runtime não
consumirá arquivos do Studio.

## Evidência

`tests/test_performance_session_document.cpp` cobre round-trip canônico,
integridade de território/cenas/relações, versão desconhecida, contagem acima do
limite, cena de seed estrangeira, input acima de 64 KiB, dados posteriores ao
marcador final e cada prefixo semanticamente truncado do documento canônico (a
quebra de linha final é opcional). Em qualquer falha, o destino já carregado
permanece intacto.

Validação Release em 2026-08-24: 5/5 CTests passaram, incluindo a simulação
offline de 20 minutos do motor. ASan e UBSan também aprovaram os quatro testes
não-stress. O host JUCE posterior elevou a suíte a 6/6; a persistência ainda não
foi medida nele nem em dispositivo real.

## Próximo passo

Manter o documento como memória de controle no host standalone já criado e
validar seu uso em dispositivo real. A integração de uma matéria do Studio
permanece bloqueada: exige cópia/artefato próprio no Performance, proveniência,
licença e testes de compatibilidade — sem editar ou incluir o Studio.
