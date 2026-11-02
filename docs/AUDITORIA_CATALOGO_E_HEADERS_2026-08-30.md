# Auditoria do catálogo e dos headers — 2026-08-30

## Resultado da triagem do catálogo

O catálogo canônico é `../docs/pt/_catalogo_mecanismos.md` e contém 142
fichas numeradas. A classificação abaixo cobre todas as entradas; o número é
o identificador da ficha no catálogo e o título completo permanece na fonte
canônica, evitando duplicação e divergência de nomes.

### A — corpo sonoro já atravessado por adapter Performance

`4, 11, 13, 19, 75, 88, 103, 106, 108, 110, 113, 114, 115, 120`

Correspondem às famílias Atari Punk, Modal, Gendyn, bateria, Walsh, VOSIM,
Theremin, Atropelo, Hard Sync, Acid, Galo, Scat, Choir e FM Flute. A memória
Deja Vu pertence à linhagem documental das fichas 39 e 117, mas sua adaptação
é tratada no Performance como coorte v4 própria; a ficha de origem não é
reclassificada nem editada.

### B — candidato a corpo/efeito realtime, ainda sem autorização de entrada

`1, 2, 3, 5, 6, 7, 8, 9, 10, 12, 14, 15, 16, 18, 20, 31, 32, 33, 34, 35,
36, 37, 38, 55, 56, 57, 58, 69, 70, 72, 73, 74, 82, 83, 86, 89, 90, 91, 98,
109, 111, 112, 119, 125, 127, 129, 132`

São candidatos sonoros ou de processamento. A classificação como candidato
não significa que possam ser instanciados no callback: cada um ainda precisa
de contrato `prepare/reset/process`, estado pré-alocado, limite de custo,
determinismo, teste de energia/estabilidade e escuta humana.

### C — recurso interno de composição, relação, política ou controle

`17, 22, 28, 29, 30, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
54, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 76, 77, 78, 79, 80, 81, 84,
85, 87, 92, 93, 94, 95, 96, 99, 100, 101, 102, 105, 107, 121, 122, 123,
124, 126, 128, 130, 131, 133, 134, 135, 138, 139, 140, 141`

Essas fichas devem aparecer como comportamento do território, da seed, dos
stems, da memória e das relações. Não devem virar um botão por mecanismo.
`GamePlan`, `BluffSignal`, `MaterialAgency`, `CompositionalTopology`, gesto de
rasgar, timing selvagem e orquestração relacional entram primeiro como
políticas bounded do Performance.

### D — offline, metadado, arquivo, publicação ou observador

`21, 23, 24, 25, 26, 27, 39, 40, 71, 97, 104, 116, 117, 118, 136, 137, 142`

Essas entradas podem fornecer manifesto, genealogia, render, análise,
gravação, publicação ou evidência. Não entram diretamente no callback nem
devem ser confundidas com módulos tocáveis. Podem alimentar uma futura ponte
versionada Studio → Performance fora do caminho realtime.

## Auditoria estrutural dos headers

O inventário encontrou 67 headers DSP, 47 de engine, 15 de sequencer e 2 de
I/O. A varredura estática identificou os seguintes gates para a próxima fase:

- **Já atravessados:** os packages de coorte v1–v4 e o modal possuem fronteira
  própria, estado limitado e testes funcionais no Performance.
- **Exigem adaptação antes de tocar:** `GranularCloud`, `SpectralGhost`,
  `ParenthesisGlitch`, `Effects`, `LookaheadLimiter`, `TapeSaturator`,
  `DissonanceTreatment`, `AtropeloPhrasePool` e `MontageEditCut` usam buffers,
  vetores, distribuição aleatória ou estruturas que precisam ser preparadas e
  limitadas fora do callback.
- **Exigem fronteira offline explícita:** `Recorder`, `WavWriter`,
  `AlbumRenderer`, `PieceRenderer`, `ProbabilityMarket` e geradores de arco
  que constroem vetores, strings ou acessam arquivos. Podem preparar dados
  antes do transporte, nunca fazer I/O ou crescer estruturas durante o áudio.
- **Risco transversal:** uso de `std::mt19937` e distribuições dentro de
  processamento não é, por si só, prova de travamento, mas exige medição de
  custo e fonte determinística por módulo. Qualquer alocação, lock, I/O ou
  crescimento de container no caminho de áudio reprova o candidato até ser
  corrigido.

Esta é uma auditoria de triagem estática, não uma certificação. O próximo
passo técnico é abrir cada candidato prioritário, registrar linhas de
`prepare/reset/process`, medir alocações e executar stress isolado antes de
criar outro adapter.

## Ordem de decisão

1. Escutar e calibrar DIGITAL e MEMORY no host real.
2. Selecionar um candidato B ou uma política C com maior consequência
   performática e menor risco de recurso.
3. Preparar estado e aleatoriedade fora do callback; definir limite de CPU,
   memória, concorrência e duração.
4. Criar package/adapter versionado, teste determinístico, teste de energia,
   stress e controle contextual.
5. Só então expor a consequência na mesa e registrar proveniência, licença,
   validação e ponto de retomada.

## Seleção preliminar do próximo ciclo

O candidato prioritário é `RasgoFracture` (ficha 132), como adapter de ruptura
e cicatriz v5. A escolha se deve ao contrato pequeno e verificável: seis fibras
em `std::array`, configuração determinística fora do callback, estado
permanente de ruptura e consequência diretamente compatível com `Rasgar`,
`Recall` e `PanicSafe`. A seleção não autoriza ainda sua integração; falta
teste isolado, comparação A/B e decisão de como o estado de cicatriz atravessa
os stems sem aumentar a superfície da mesa. Essa fronteira foi implementada e
testada como `RasgoFractureAdapter` v5; a escuta humana e a calibração no host
real continuam pendentes.

O candidato secundário é `PolyrhythmDivider` (ficha 17), como recurso interno
de ritmo e não como novo módulo visual. Sua implementação usa contador e matriz
fixos, portanto o risco técnico é baixo; a validação deverá medir se o pulso
cria consequência musical sem competir com o GRID existente.

`MaterialAgency` e `CompositionalTopology` ficam como frente posterior de
política relacional. Eles carregam vetores, strings e histórico de sessão e
devem operar fora do callback, alimentando somente snapshots bounded. DSPs com
buffers dinâmicos ou processamento espectral permanecem depois dessas três
frentes.
