# Rasgo Synth Asterismus — registro de desenvolvimento

Notas de desenvolvimento, em português, preservadas como foram escritas
desde 2026-08 (primeiro como *Rasgo Synth Performance*). Alguns caminhos
citados existem só no repositório de trabalho do RASGO. A apresentação
pública do instrumento está no [README](README.md); o que mudou entre
versões, no [CHANGELOG](CHANGELOG.md).

> **Estado atual (2026-11-07):** versão **0.3.0** publicada (36 estrelas,
> cortes e fita do céu, filtros e envios por estrela, groove da DRUMS, LEARN;
> ver CHANGELOG e `docs/PUBLICACAO.md`). As seções de estado abaixo são o
> registro de cada época, não o estado de hoje.

---

## Registro de desenvolvimento

**Nome desde 2026-10-27** (antes *Rasgo Synth Performance*): o autor escolheu
*Asterismus* porque o instrumento se tornou um instrumento da ideia de
constelação. Um asterismo é um desenho de estrelas reconhecido e nomeado sem
ser uma constelação oficial: as figuras que músico e seed desenham entre
corpos na CONSTELAÇÃO do CAMPO (o termo interno continua o mesmo).

Localização atual (2026-10-27): `RASGO/PROJETOS_AUTORAIS/RASGO_SYNTH_ASTERISMUS/`.
Os atalhos com o nome antigo foram removidos em 2026-10-27/28 (o da raiz na
arrumação do RASGO; o de `PROJETOS_AUTORAIS/` depois que o Codex tirou o
atalho interno do Studio). Registros históricos que citam
`RASGO_SYNTH_PERFORMANCE` ficam como proveniência. A pasta de gravações passou a
`~/Music/Rasgo Synth Asterismus` em 2026-11-07 (pedido do autor; as tomadas
antigas foram movidas para ela, `AUDIOS_RASGO/Asterismus`); o formato das
tomadas mantém o nome antigo (`RASGO_PERFORMANCE_TAKE`), para as já gravadas
continuarem legíveis;
os nomes internos do código (`Performance*`) descrevem o ato de tocar e ficam.
Criação e desenvolvimento são a finalidade; autoria, licença e identidade
permanecem próprias. Ler [as instruções RASGO](../../AGENTS.md) e a continuidade
local antes de trabalhar. Organização e validações no
[registro da migração](../../RASGO_DOCUMENTATION/architecture/MIGRACAO_PROJETOS_AUTORAIS.md).
Aplicar o [procedimento de recursos](../../../PROCEDIMENTO_SEGURANCA_RECURSOS.md)
antes de tarefas controladas; publicação continua uma decisão separada.


## Localização e retomada — 2026-10-11

Fonte canônica: `RASGO/RASGO_SYNTH_PERFORMANCE/`, pasta irmã do Studio.
Retomada própria: [CONTINUIDADE.md](CONTINUIDADE.md).
Pacotes DSP locais: `vendor/studio-dsp/`, copiados do Studio com commit,
licença/créditos e36 checksums em
[PROVENANCE.md](vendor/studio-dsp/PROVENANCE.md). Preset e defaults CMake
não buscam mais o Core do Studio. Não há sincronização automática.
Oito módulos internos/quatro stems mantidos; nenhuma mudança de som.
Build novo de cinco alvos e5/5 testes focais passaram; host JUCE e
regressão completa não foram recompilados/repetidos nesta separação.
Builds antigos foram preservados com seus caches históricos, não são
prova de uma nova build independente. Não houve commit/push/release.

Estado: `v0.1.16-dev`, atualização de continuidade em 2026-10-06. O estado
atual separa oito módulos/lanes internos de até quatro stems de produção.
Os módulos 5–8 têm círculos selecionáveis e posicionáveis na Constelação,
três controles locais e `ORIGEM ↔ VARIAÇÃO`; não ganham saída, presença,
ganho, mute ou solo próprios. Esses controles de mix pertencem aos quatro
stems que recebem as camadas. A escuta humana, a validação visual dos quatro
idiomas e a estabilidade prolongada no dispositivo real continuam pendentes.

A biblioteca contém 29 entradas: 14 corpos adaptados, um candidato realtime
e 14 organismos do Studio ainda em auditoria. Catalogar não significa tornar
um módulo inserível. Tuba/Breath permanece protótipo experimental, sem rota
ativa aprovada; seu teste isolado não autoriza promovê-lo ao submix.

Os registros datados abaixo preservam a evolução do instrumento. Ensaios de
cinco/oito stems não ampliam o limite atual de produção. O ponto operacional
mais recente e os resultados de validação estão em [TAREFAS.md](TAREFAS.md).

Instrumento relacionado: [Rasgo Synth Studio](../RASGO_SYNTH_STUDIO/README.md),
em pasta irmã independente.

Rasgo Synth Performance é um instrumento autônomo do Rasgo Synth voltado à
execução ao vivo. Mantém fonte, app, sessão, estado, callback, builds e
arquivos de usuário próprios. É irmão gêmeo do Studio: reutiliza, quando fizer
sentido, código, DSP, módulos e comportamentos já corretos por cópia controlada
dentro da própria árvore, e cria código novo para suas necessidades próprias —
sempre sem alterar o Studio para acomodá-las.
Não é um modo dentro da interface Studio nem uma dependência operacional do
seu núcleo.

## Decisão central — Studio como matéria, Performance como realização

O Performance não é um sintetizador paralelo reduzido nem um catálogo de
módulos decorativos. Ele existe para tornar **performável** a composição viva
que o Rasgo Synth Studio já sabe gerar. A seed deve chegar como território
musical — forma, tempo, harmonia, matéria, memória e stems — e o músico deve
intervir sobre seus encaminhamentos: ativar corpos, redesenhar relações,
subverter trajetórias, ferir, reparar, trocar capítulos e recombinar o que já
está soando.

O Studio é a referência genealógica e composicional; o Performance é uma
realização corporal e temporal independente. Código correto do Studio pode ser
copiado para o Performance e adaptado somente na borda necessária ao palco e
ao realtime; código novo também pode ser criado livremente no Performance.
Ambos os caminhos registram origem quando houver reutilização, licença,
diferenças e testes. Nenhum deles autoriza editar o Studio, depender de seu
runtime, nem misturar diretórios, estados ou builds.

A identidade própria do Performance está no **CAMPO**: corpos/polígonos reais,
movimentos, forças, conexões e fluxos formam a própria composição. A seed
inicial é ponto de partida, não limite; uma ou mais seeds poderão atuar e variar
por corpo/relação sem remover módulos, mover âncoras ou apagar o desenho feito.
O campo continua sua narrativa quando a mão para e chega a um epílogo — chamado
pelo músico ou iniciado após inatividade prolongada — que conclui também a
gravação. A especificação está em
[`docs/GRAMATICA_DO_CAMPO_PERFORMATIVO.md`](docs/GRAMATICA_DO_CAMPO_PERFORMATIVO.md).

Como o Studio, o Performance é um instrumento que **gera músicas gravadas**.
Cada realização começa quando o músico inicia simultaneamente a abertura, a
gravação e o trace; desenvolve-se na conversa entre gesto e seed; e termina por
coda autoral ou por epílogo generativo após inatividade prolongada. `GRAVANDO`
é um estado obrigatório e legível da performance, não um modo opcional.

No CAMPO, o músico compõe em diálogo com a seed: pode acompanhá-la,
contradizê-la, pervertê-la, interrompê-la, rasgá-la, repará-la e recombiná-la;
a seed responde pelos módulos, pela estratégia de jogo e pela memória. Antes
de escrever qualquer substituto, a implementação correspondente do Studio/core
deve ser localizada e auditada. Se estiver correta, pode ser copiada de modo
controlado para o Performance e adaptada no necessário; se houver uma lacuna
performática, o Performance cria o código próprio de que precisa. Em nenhum
caso se altera a fonte no Studio.

O layout já consolidado do **Rasgo Synth Studio** é o ponto de partida visual
obrigatório. O Performance preservará a identidade da placa eletrônica verde,
trilhas e acentos de cobre, logo autoral, tipografia de laboratório, grade,
painéis rebaixados e instrumentos de visualização do Rasgo Synth. Não haverá
reinício gráfico nem adoção da aparência de outro instrumento.

O painel do Antitotem é uma referência autoral interna de **anatomia
performática**: organização temporal, 16 steps, mixer/relações, memória e log.
Essas funções serão traduzidas para a superfície Rasgo já existente, sem copiar
a cadeia CMOS, as cores, os assets ou o desenho do Antitotem.

## Catálogo realmente performável

O catálogo atual do Performance contém **quatorze corpos com adapter realtime**,
em cinco coortes de três lugares: Circuito (`Acid`, `Gendyn`, `Drums`), Sopro
(`FM Flute`, `Theremin`, `VOSIM`), Voz (`Scat`, `Choir`, `VOSIM`), Digital
(`Walsh`, `Hard Sync`, `Atari Punk`) e Memory (`Deja Vu`, `Atropelo`, `Galo`). VOSIM participa de Sopro e Voz sem ser
duplicado. A seed é um **ponto de partida**: escolhe quantos e quais corpos
começam atuando. Depois que a mesa começa, o músico conduz a acumulação ao
inserir, recolher e relacionar a matéria complementar que a seed preparou, sem
trocar de território: Circuito oferece Walsh ou Hard Sync; Sopro oferece Atari
Punk; Voz oferece Walsh; Digital oferece VOSIM; Memory oferece Hard Sync. A
seed continua como motor composicional depois da abertura: seus módulos,
papéis, estratégia de jogo e memória desenvolvem a trajetória em diálogo com o
músico. Ela também impede um catálogo aleatório: cada stem adicional tem uma
ponte DSP, controles, relação e teste próprios. A capacidade futura deverá
tomar como referência os quase 20 stems
que o Studio já sustenta simultaneamente, sem assumir que todos estarão
automaticamente disponíveis no Performance. A acumulação arbitrária entre
coortes permanece uma etapa posterior, pois exigirá um mixer multi-adapter
dedicado. Há muitos
outros recursos e vozes no core/Studio, mas eles não devem aparecer como
módulos do Performance até terem adapter, contrato bounded, teste e consequência
audível próprios.

O jogo do Performance é conceitual e deriva das pesquisas já realizadas no
Studio. Referenciais como Caillois devem orientar a dramaturgia de atração,
conflito, acaso, vertigem e imitação, sempre traduzidos em comportamento
musical e espacial. Não haverá placar obrigatório nem competição separada do
instrumento.

A especificação prática dessa evolução está em
[`docs/MESA_RELACIONAL_DESIGN.md`](docs/MESA_RELACIONAL_DESIGN.md), derivada do
laboratório intermodular do Studio. Ela define a unidade de módulo, a relação
espacial, o fluxo seguro de comandos e o primeiro protótipo de quatro corpos.
O diagnóstico das estruturas ainda fixas de três está em
[`docs/AUDITORIA_EXPANSAO_MESA_2026-08-30.md`](docs/AUDITORIA_EXPANSAO_MESA_2026-08-30.md).
O estado arquivístico, a classificação de fontes/derivados e o procedimento de
fechamento de marco estão em
[`docs/ARQUIVISTICA_E_PRESERVACAO.md`](docs/ARQUIVISTICA_E_PRESERVACAO.md).
O contrato em revisão das 16 células, com regras de preservação do campo,
previsibilidade e desfazer, está em
[`docs/CONTRATO_CELULAS_PERFORMATIVAS.md`](docs/CONTRATO_CELULAS_PERFORMATIVAS.md).
O programa que consolida a direção performática, os critérios de pronto e a
ordem vinculante de implementação está em
[`docs/PROGRAMA_PERFORMATIVO_CANONICO.md`](docs/PROGRAMA_PERFORMATIVO_CANONICO.md).
A auditoria que localiza no Studio/core os contratos reais de módulos, jogo,
agência, relações, arco e coda — e identifica o que ainda falta na ponte — está
em
[`docs/AUDITORIA_STUDIO_MODULOS_E_JOGO_2026-09-02.md`](docs/AUDITORIA_STUDIO_MODULOS_E_JOGO_2026-09-02.md).
A auditoria do acervo de referências do Studio está em
[`docs/AUDITORIA_REFERENCIAS_STUDIO_2026-08-30.md`](docs/AUDITORIA_REFERENCIAS_STUDIO_2026-08-30.md).
O protocolo para medir a capacidade segura da máquina está em
[`docs/BENCHMARK_CAPACIDADE_HARDWARE.md`](docs/BENCHMARK_CAPACIDADE_HARDWARE.md).
Os requisitos multiplataforma e perfis de degradação estão em
[`docs/REQUISITOS_MULTIPLATAFORMA.md`](docs/REQUISITOS_MULTIPLATAFORMA.md).

A coorte **Digital** entrou em `v0.1.15-dev`: a seed `184` a seleciona. A
coorte **Memory** entra agora em `v0.1.16-dev`: a seed `185` seleciona Deja Vu,
Atropelo e Galo, com memória estatística, voz quase falada e chamada ressonante.
As duas matérias chegam ao mesmo caminho de stems, presença, foco, relações e
gestos; a validação humana permanece pendente. O adapter `RasgoFracture` v5
também atravessa esse caminho como textura bounded de cicatriz, acionada por
`Rasgar`, reduzida por `Recall` e reinicializada por `PanicSafe`, sem novo
controle visual. Sua aprovação musical ainda depende de escuta A/B no host
real.

## Execução manual do host

A partir desta pasta, execute o binário recompilado:

```bash
./run_asterismus.sh   # acha o build mais recente (.local-build/host-*)
```

Ou use o lançador versionado, que verifica a existência do build:

```bash
./run_asterismus.sh
```

O lançador verifica volume gravável, memória disponível mínima de 4 GiB e uma
sessão gráfica realmente acessível antes de iniciar o host. Se algum gate
falhar, ele encerra sem abrir o processo; corrija o ambiente e tente novamente.

Após testes ou builds, verifique resíduos sem apagar nada:

```bash
./check_runtime_artifacts.sh
```

Para conferir as novas ecologias, abrir **MESA**, inserir a seed `184` ou `185`
e acionar `ALEA` durante a reprodução. `ALEA` lança o dado no lado da interface,
mostra a seed sorteada no campo e desenha o novo território; a digitação manual
continua disponível para repetir uma seed conhecida. A biblioteca compacta no painel deve
identificar os módulos adaptados e os candidatos ainda em auditoria; o Campo
deve distinguir os oito módulos internos dos quatro stems de saída. A
confirmação de som e ergonomia é manual e deve ser
registrada após a sessão, não inferida do build. A troca de seed é enfileirada
como transição ao vivo e não exige parar o áudio.

O quarto cartão de reserva é escolhido pela seed: Circuito usa `WALSH` ou
`HARD SYNC` (por exemplo seed `186`); Sopro usa `ATARI PUNK` (seed `182`);
Voz usa `WALSH` (seed `183`); Digital usa `VOSIM` (seed `184`); Memory usa
`HARD SYNC` (seed `185`). Arraste-o para o campo durante PLAY, selecione o nó
para abrir seus três controles contextuais e regule as relações na MESA.
Recolher o nó de volta à reserva não muda os outros três corpos, a seed, as
cenas ou as posições que o músico desenhou. O limite operacional medido nesta
máquina é de quatro corpos.

Para repetir a medição de quatro corpos (20 minutos simulados):

```bash
./build-app/rasgo_synth_performance_stability_tests four
```

Esse comando exige o build com os packages versionados V1–V4. O benchmark é
offline; teste de driver e latência real exige um dispositivo de áudio
disponível.

Para uma verificação curta do mesmo caminho de quatro corpos, sem confundir o
resultado com o benchmark longo:

```bash
./build-app/rasgo_synth_performance_stability_tests four-smoke
```

O smoke simula 60 segundos, verifica finitude, energia, quatro stems e zero
alocações no `processBlock`. Em 2026-09-07 ele passou em 4,98 s (12,06× tempo
real), após a retirada da portadora contínua e das curvas exponenciais de
envelope no callback. Não mede driver, latência ou XRUN de hardware.

Para a mesma medição usando a variação acumulativa `HARD SYNC` (seed `186`):

```bash
./build-app/rasgo_synth_performance_stability_tests hard-sync
```

O quinto corpo ModalResonator possui um benchmark **offline e isolado**. Ele
não altera o limite do host, que permanece em quatro corpos:

```bash
./build-app/rasgo_synth_performance_modal_fifth_stability_tests modal-five
```

Antes da medição longa, há um smoke de 60 segundos simulados para conferir o
caminho isolado. Em 2026-09-01 ele passou em 2,456 s (24,43× tempo real), sem
alocações no `processBlock`, com pico offline de 0,717 ms por bloco e RSS máximo
de 4096 KiB; isso não mede driver, latência ou estabilidade de 20 minutos:

```bash
./build-app/rasgo_synth_performance_modal_fifth_stability_tests modal-five-smoke
```

A execução longa offline também passou: 20 minutos simulados em 49,88 s
(24,06×), pico de bloco de 0,988 ms, RSS máximo de 5808 KiB, cinco stems
estáveis e zero alocações. Ela não substitui a medição com driver e áudio real,
portanto o host ainda mantém o limite de quatro corpos.

Antes do próximo ensaio de áudio real, executar:

```bash
bash ./check_host_readiness.sh
./run_asterismus.sh
```

O preflight só confirma que binário, volume, memória, janela e dispositivo
estão disponíveis; os critérios de escuta, latência e autorização estão em
[`docs/BENCHMARK_CAPACIDADE_HARDWARE.md`](docs/BENCHMARK_CAPACIDADE_HARDWARE.md).

O cabeçalho do host mostra o limite audível atual. Os oito slots da
Constelação são estruturais; no Release, o teto de produção é **quatro** stems.
O perfil de CPU só escolhe um piso conservador — três em compatibilidade e
quatro em máquinas com quatro ou mais núcleos lógicos — e não autoriza cinco a
oito por si só. A promoção futura dependerá de uma calibração local que mede
driver, buffer, CPU, memória e XRUN durante ensaio prolongado; até ela existir,
corpos acima do teto permanecem reserva, sem corte de áudio.

Uma stem liberada é uma **faixa de orquestração**, não a autorização para
instanciar todo o catálogo de DSP. A seed distribui os módulos adaptados por
função, família, afinidade de coorte e dramaturgia: o primeiro módulo abre a
faixa e os demais formam alternativas para eventos, cenas, memória e
recombinação. Assim, um catálogo de vinte módulos numa máquina segura para
quatro stems resulta em cerca de cinco candidatos por stem, mas somente quatro
vozes processadas simultaneamente. Módulos sem adapter e benchmark continuam
reservas — visíveis para o planejamento, mas jamais caixas ou áudio fantasma.

O primeiro submix interno já está realizado nas coortes nativas: uma matéria
relacionada pode entrar como camada breve numa stem por acontecimento da seed e
pelos cabos vivos. A camada usa sinal de módulo já renderizado, respeita frase,
estado de vida, ganho, mute e solo, e não muda posições, polígonos ou conexões.
É um primeiro gesto de mistura — não a alegação de que todo o catálogo já está
misturado em tempo real. A expansão para outros adapters seguirá o mesmo
contrato e a mesma validação de carga.

No Campo, a mistura ativa é indicada por uma órbita interna cobre no astro da
stem. Ao selecioná-lo, a linha de estado mostra `MISTURA`, além de pico e
profundidade: a visualização confirma uma consequência que está ocorrendo no
áudio, não cria outro controle nem outro corpo.

Navalha 2 é uma segunda referência, desta vez de **operação e engenharia**:
modo dual monitor, faixas de workflow, cenas/FORM, memória, projeto portátil,
telemetria, REC e saída segura. Ele não pertence ao núcleo RASGO e mantém
genealogia própria; o Performance não reutiliza seu layout, identidade, fontes
A/B, recorte ou masterização.

Ao final existirão dois instrumentos:

- **Rasgo Synth Studio:** composição generativa, renderização, escuta,
  documentação e produção de peças completas;
- **Rasgo Synth Performance:** áudio contínuo de baixa latência, gesto humano,
  cenas, relações entre organismos e decisões em tempo real.

Quando ambos estiverem maduros, poderão se complementar por uma ponte
versionada e opcional: o Studio oferece um território/seed auditado; o
Performance registra a trajetória vivida; o Studio poderá futuramente estudar
ou reinterpretar esse trace. A autonomia operacional não é isolamento de
pesquisa: a coorte inicial prova a fronteira, mas o objetivo é tornar
progressivamente performável o amplo ecossistema já avançado no Studio, sem
duplicar seu código ou forçar todos os módulos a aparecerem como widgets.

## Fronteira inviolável do Studio

O desenvolvimento do Performance segue estas regras:

1. não mover, renomear ou editar código, assets, configuração ou documentação
   operacional do Studio para acomodar o novo app;
2. não editar a interface do Studio; reaproveitar sua linguagem visual por
   especificação e implementação independentes no Performance;
3. não compartilhar diretório de preferências, recordings, builds ou binários;
4. não depender de caminhos runtime ocultos até `../rasgo-synth-app/` ou
   `../rasgo-synth-core/` numa release;
5. toda matéria reutilizada do core entra no Performance como cópia controlada,
   com versão de origem, licença, adaptação e testes documentados;
6. nenhuma evolução do core/Studio é autorizada por uma necessidade do
   Performance; alteração naquele projeto exige autorização explícita em tarefa
   separada.

As alterações editoriais que já estavam pendentes em `RASGO_SYNTH/` antes da
criação deste diretório continuam pertencendo à frente anterior. A partir deste
marco, o trabalho do Performance fica neste projeto.

## Estratégia de reaproveitamento

O `PieceRenderer` do Studio foi desenhado para materializar uma peça completa.
Ele não deve virar à força um motor de palco interrompível. O Performance terá
um motor incremental próprio, com `prepare()`, `processBlock()` e `reset()`, e
reutilizará somente DSP/contratos que façam sentido em áudio contínuo.

A integração será feita em três passos:

1. definir no Performance os contratos de sessão, gesto, cena e telemetria sem
   incluir headers do Studio;
2. validar esses contratos com um motor mínimo e testes realtime;
3. copiar para o Performance a matéria comprovada de que ele precisar, ou criar
   sua própria matéria quando a proposta performática exigir, sempre sem alterar
   a aplicação ou core do Studio.

Quando um comportamento, módulo ou DSP do Studio for necessário, o Performance
deve preferir uma **cópia controlada** em sua própria árvore quando ela atender
ao problema; quando não atender, pode criar uma implementação própria no
Performance. A cópia passa a ser um artefato
do Performance: deve registrar arquivo/revisão de origem, finalidade, licença,
alterações introduzidas e testes próprios. Não autoriza editar o Studio, fazer
include relativo de seus fontes, ler seus diretórios em runtime nem transformar
uma cópia provisória em núcleo compartilhado sem contrato e segundo consumidor.

Não se cria uma segunda fonte canônica do Studio. Uma cópia controlada no
Performance é sua fonte operacional própria e mantém o vínculo de proveniência
com a origem. Cinco fronteiras CMake opcionais e hash-pinned estão ativas:
`rasgo::modal_matter_v1` para o retorno modal,
`rasgo::performance_cohort_v1` para Acid/Gendyn/percussões e seus recursos, e
`rasgo::performance_cohort_v2` para FM Flute, Theremin, Scat, Choir e VOSIM,
além de `rasgo::performance_cohort_v3` para Walsh, Hard Sync e Atari Punk e
`rasgo::performance_cohort_v4` para Deja Vu, Atropelo e Galo.
O Performance permanece compilável sem elas; qualquer nova matéria exige
auditoria, versão, hashes e testes equivalentes.

## Estrutura do projeto

```text
rasgo-synth-performance/
├── README.md
├── LICENSE
├── THIRD_PARTY_NOTICES.md
├── TAREFAS.md
├── .gitignore
├── docs/
│   ├── ARQUITETURA.md
│   ├── AUDITORIA_FUNCIONAL_2026-08-27.md
│   ├── AUDITORIA_PRIMEIRO_ADAPTER.md
│   ├── MAPA_CORPOS_CONTROLES_2026-09-04.md
│   ├── CONTRATOS_MARCO_1.md
│   ├── CONTROLADOR_SESSAO_MARCO_2.md
│   ├── ESTRATEGIA_MUSICAL.md
│   ├── HOST_STANDALONE_MARCO_4.md
│   ├── MOTOR_MINIMO_MARCO_2.md
│   ├── PAINEL_PERFORMANCE.md
│   ├── SESSION_DOCUMENT_MARCO_1.md
│   └── PONTE_STUDIO_PERFORMANCE.md
├── src/                    # contratos, motor, adapters e host próprios
├── tests/                  # testes puros e realtime do novo instrumento
├── assets/                 # identidade Rasgo adaptada, com proveniência própria
├── validation/             # evidências locais; áudio nunca vai ao GitHub
└── packaging/              # Linux/Windows/macOS em etapa posterior
```

O screenshot de referência atual permanece em
`../docs/assets/app_screenshot.png`. Ele é evidência do layout e
não será duplicado como asset do Performance. Logo, fontes e outros elementos
compartilháveis só entram neste projeto com origem, licença e versão
registradas, sem criar dependência runtime oculta no diretório do Studio.

## Tecnologia e plataformas

- C++17 ou superior para núcleo e contratos puros;
- JUCE para app standalone e dispositivos de áudio;
- GNU/Linux como primeira plataforma de desenvolvimento;
- Windows e macOS como gates obrigatórios antes de declarar uma release
  multiplataforma;
- plugin, console embarcado e hardware ficam fora do primeiro marco.

O código autoral usa GPLv3 e o host usa a via AGPLv3 do JUCE. Cada release ainda
deve levar os avisos e textos exigidos pela revisão efetivamente empacotada;
ver [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).

## Estado auditado e próximo passo

O núcleo de contratos, motor incremental, documento `RSPERF 1`, host e painel
estão implementados sem incluir arquivos do Studio. As fronteiras opcionais
`rasgo::modal_matter_v1`, `rasgo::performance_cohort_v1` e
`rasgo::performance_cohort_v2` e `rasgo::performance_cohort_v3` alimentam as
ecologias CIRCUITO, SOPRO, VOZ e DIGITAL, selecionadas pelas seeds 181–184.
Os módulos atravessam stems, mixer,
relações e controles locais bounded até o DSP.

A auditoria de 2026-08-27 substituiu a orientação de começar pela escuta livre.
O incremento `v0.1.1-dev` corrigiu seed 0, PLAY imediato após território,
reset coerente da mesa, confirmação das filas e posse separada dos documentos
de controle/áudio. Em `v0.1.2-dev`, relações recuperadas por cena também
reconstroem a posição dos nós; `CAPTURAR` foi delimitado a BPM, sete macros,
presenças e três relações. Controles locais, mixer, MASTER, GRID, Hold e morph
são estado da mesa e não acendem `MOD`. A seed inicial 181 vem da fábrica
canônica. Em `v0.1.3-dev`, Memória, Intervenção, Mutate, Rasgar, Recall e Hold
passam também por relógios, articulações, formantes e energia dos corpos reais
de SOPRO e VOZ. Em `v0.1.4-dev`, CORTE/ARTIC/FRAG./ABERT., PRES. do Theremin e
CORPO/FRASE do Choir receberam escalas diretas nos corpos, com regressões de
contraste por endpoint. Regressões passaram no host com as três ecologias; o
app não foi aberto pelo workflow. Em `v0.1.5-dev`, o cabeçalho passou a dizer
CORPOS, ATRASOS (orçamento de callback) e PEAK (sample peak), sem prometer
medição de driver ou true peak. Antes de ampliar catálogo, ilhas ou
constelações, falta a confirmação manual dessas bordas. Em `v0.1.6-dev`, a
constelação informa o próximo gesto possível conforme reserva, nó ou cabo em
foco; não acrescenta comandos. Em `v0.1.7-dev`, Rasgar e Recall deixam de ter
somente um rótulo de ação: a telemetria publica cicatriz e reparo que já modulam
o DSP, e a constelação e o rodapé mostram esse estado como trajetória. Não é
score nem trace persistido. Em `v0.1.8-dev`, selecionar nó ou cabo mostra o
pico real do stem e a consequência concreta da relação (matéria, tempo ou
espaço), distinguindo ligação ativa de potencial. Ver
Em `v0.1.9-dev`, a vida declarada pela seed também atua no som: corpos podem
emergir, permanecer ativos ou ficar em reserva; arrastar presença acorda ou
recolhe o stem real e o campo mostra esse estado.
Em `v0.1.10-dev`, `MUTATE` deixou de ser apenas uma alteração abstrata de
timbre: o host prepara um token de variação no lado de controle, o evento o
leva pela fila bounded já existente e o motor abre por alguns segundos um
percurso selvagem sobre os stems. `RECALL` reduz essa abertura de imediato e
`PANIC SAFE` a zera; cabeçalho e trajetória só dizem `SELVAGEM` enquanto o DSP
está de fato sob esse regime. A Constelação usa a mesma telemetria: os cabos e
halos dos módulos ativos se intensificam sem deslocar a geometria do performer.
Em `v0.1.11-dev`, cena e transporte foram separados: recuperar ou morfar cenas
durante PLAY nunca pode parar a seed; somente PLAY/STOP ou a entrada de um novo
território definem transporte. A regra UTF-8 do workspace também exige conversão
explícita antes de todo texto visível chegar ao JUCE.
Em `v0.1.12-dev`, selecionar um módulo na Constelação antes de `MUTATE` faz a
abertura selvagem privilegiar seu stem; sem seleção, o gesto continua coletivo.
Em `v0.1.13-dev`, a mesma escolha de nó orienta `RASGAR`: a cicatriz se torna
principalmente um acontecimento daquele corpo, preservando uma continuidade
menor nos demais stems.
Em `v0.1.14-dev`, `RECALL` completa a tríade: com um nó em foco, o reparo
privilegia aquele stem; sem seleção, continua sendo uma recomposição coletiva.
Em `v0.1.15-dev`, a superfície torna o foco explícito: os três botões passam a
dizer qual corpo receberá Abrir/Rasgar/Reparar, o foco inicial desenhado é o
mesmo alvo do motor e um clique no vazio retorna à consequência coletiva.
Um corpo na reserva não é anunciado como alvo: ele precisa primeiro entrar na
Constelação por arraste, evitando um gesto aparentemente sem consequência.
[`docs/AUDITORIA_FUNCIONAL_2026-08-27.md`](docs/AUDITORIA_FUNCIONAL_2026-08-27.md).

A estratégia musical que governa esses contratos está em:
[`docs/ESTRATEGIA_MUSICAL.md`](docs/ESTRATEGIA_MUSICAL.md),
[`docs/HOST_STANDALONE_MARCO_4.md`](docs/HOST_STANDALONE_MARCO_4.md),
[`docs/PONTE_STUDIO_PERFORMANCE.md`](docs/PONTE_STUDIO_PERFORMANCE.md),
[`docs/ARQUITETURA.md`](docs/ARQUITETURA.md),
[`docs/AUDITORIA_FUNCIONAL_2026-08-27.md`](docs/AUDITORIA_FUNCIONAL_2026-08-27.md),
[`docs/AUDITORIA_PRIMEIRO_ADAPTER.md`](docs/AUDITORIA_PRIMEIRO_ADAPTER.md),
[`docs/CONTRATOS_MARCO_1.md`](docs/CONTRATOS_MARCO_1.md),
[`docs/CONTROLADOR_SESSAO_MARCO_2.md`](docs/CONTROLADOR_SESSAO_MARCO_2.md),
[`docs/MOTOR_MINIMO_MARCO_2.md`](docs/MOTOR_MINIMO_MARCO_2.md),
[`docs/SESSION_DOCUMENT_MARCO_1.md`](docs/SESSION_DOCUMENT_MARCO_1.md),
[`docs/PAINEL_PERFORMANCE.md`](docs/PAINEL_PERFORMANCE.md) e
[`TAREFAS.md`](TAREFAS.md).

## Estado técnico consolidado — 2026-08-30

O host Release está configurado pelo preset `linux-release-host`. A MESA
atualiza o inspetor conforme a seed/coorte e o layout possui testes de não
interseção para cabeçalho, cenas e `ConstellationGraph`. O CTest funcional
passa 12/12; a estabilidade passou em 41,28 s. A validação visual real e a
escuta em dispositivo físico ainda são obrigatórias antes de release.

Ao encerrar uma sessão, executar `./check_runtime_artifacts.sh`. A ferramenta é
somente diagnóstica: não remove temporários nem altera recordings.

## Última validação integrada — 2026-09-01

O controlador foi ensaiado de ponta a ponta com cinco seeds representando as
ecologias Circuito, Sopro, Voz, Digital e Memória. Em cada uma, o quarto
módulo foi inserido com posição, controle local e relação, permaneceu audível
durante PLAY e recebeu um acontecimento autônomo depois da pausa. A posição do
performer permaneceu idêntica; a seed modulou o acontecimento e as pressões,
sem redesenhar a mesa. O alvo passou 18/18 CTests não-longos.

Esse ensaio é uma validação de contrato e DSP offline. A confirmação visual,
ergonômica e de áudio real continua pendente no host físico.

## Movimento do Campo e dança local — 2026-09-03

O **polígono** é a configuração coletiva desenhada pelo músico: a constelação
que emerge do conjunto de corpos. Cada círculo é um astro, ator e stem audível
no Campo. Ele abre com um módulo concreto escolhido pela seed e carrega um
pequeno repertório de módulos afins para transformação ao longo da peça; não
representa vozes simultâneas escondidas nem objetos decorativos.

O desenho-base permanece recuperável. Com nenhum corpo selecionado, o controle
do alto à direita passa a se chamar `CAMPO` e a tecla `O` alterna a
transformação coletiva entre `ÓRBITA`, `PULSO` e `FIGURA-8`. A constelação
inteira recebe uma transformação a partir do seu centro comum. Em `ÓRBITA`,
ela gira como um corpo rígido virtual: todos os astros têm a mesma velocidade
angular e preservam as distâncias relativas. `PULSO` e `FIGURA-8` podem
adicionar respiração/deformação proporcional, sempre sem apagar o
desenho-base.

Com um astro selecionado, o mesmo controle se chama `DANÇA` e atua somente
nele. `Shift`+clique ou `Shift`+`O` inverte seu sentido. Essa diferença local
é uma força musical explícita: pode deformar temporariamente a constelação e
alterar as relações, mas não apaga âncoras, conexões, cenas ou o desenho do
performer.

As posições vivas compartilhadas alimentam interface, relações e áudio.
`ÓRBITA` conduz deriva contínua, `PULSO` acentua a matéria ritmicamente e
`FIGURA-8` alterna o cruzamento; cada transformação chega aos stems reais com
transição suavizada. Em 2026-09-03, o relógio foi desacelerado para escala de
trajetória legível — Pulso segue mais ativo, mas não opera mais como LFO
rápido. App Release recompilado; os CTests de contrato e engine
da rota passaram. Ainda é obrigatória a escuta humana com oito corpos para
calibrar contraste, velocidade, direção e utilidade musical.

## Quinto corpo real — 2026-09-03

`Modal Resonator` deixou a reserva: cada território inicial passa a abrir cinco
corpos reais no Campo. O Modal recebe presença, posição, controles, stem,
telemetria, dança e uma relação de memória com o primeiro corpo; não é cartão
falso nem retorno escondido. O teto atual de realização foi elevado de quatro
para cinco após o smoke offline repetido (zero alocações e pico de bloco dentro
do orçamento). O app Release e sete CTests funcionais foram recompilados e
aprovados. A ampliação seguinte deve chegar como corpos 6–8 completos, com
auditoria de carga por etapa, e não como uma lista visual desligada do som.

## Narrativa quando o performer pausa — 2026-09-02

Depois de dois segundos sem gesto manual, a seed realiza um turno situado sobre
as pressões musicais e uma relação existente. A mesa do músico permanece sua:
o turno não move módulos, não altera cabos, não faz recall de cena e não troca
a seed. `Intervenção` alta não bloqueia mais esse processo; somente
`AUTONOMIA` em zero é uma recusa explícita da condução narrativa. O motor possui
regressão para a combinação de autonomia baixa/intervenção alta e para dois
turnos consecutivos. A mesa mostra o último acontecimento como `SEED n ·
MOVIMENTO · ESTRATÉGIA`, por exemplo `SEED 4 · DISPUTA · CONTRAPONTO`; é
telemetria somente leitura, não uma nova superfície de comando. A validação
técnica atual passou 19/19 CTests não-longos. Falta calibrar a sensação musical
da cadência e a utilidade dessa leitura em escuta humana.

A pausa não é um metrônomo rígido: o primeiro turno responde após quatro
batidas; os seguintes usam quatro, seis ou oito batidas, escolhidas
deterministicamente pela trajetória da seed e convertidas pelo BPM atual. Assim
o instrumento não fica repetindo uma intervenção idêntica em períodos fixos.

## Recortes performáveis — 2026-09-02

`STUTTER` (`S`) e `TEMPO SHIFT` (`D`) são agora gestos de recorte na mesma
faixa de ações de `HOLD`, `MUTATE`, `RASGAR` e `RECALL`. Com um corpo em foco,
atuam somente sobre seu stem; sem foco, atuam coletivamente. `STUTTER` cria uma
reiteração rítmica curta com envelope suavizado. `TEMPO SHIFT` desloca
temporariamente fase e pulso locais, com uma pequena alteração de cor, sem
alterar o BPM salvo. Ambos duram no máximo 0,8 s, passam pela fila realtime e
não criam módulos, não mudam a capacidade, não chamam cenas nem movem o desenho
da mesa.

O host Release foi recompilado e a suíte curta passou 19/19. A regressão A/B
confirma diferença de áudio e preservação de posição/BPM; a escuta no desktop
físico ainda decide se suas durações e intensidades são musicalmente úteis.

## Origem e seed viva no CAMPO — 2026-09-03

A faixa `SEED VIVA`, acima do campo XY, oferece uma seed de variação, o botão
`ALEA` e o fader `VAR`. Ela é uma segunda história, não um novo território:
`ALEA` não troca a coorte nem move o Campo. `VAR 0` mantém a trajetória de
origem; `VAR 1` transfere os traços estruturais da variação; os valores entre
ambos fazem a contaminação gradual definida no motor. Ao trocar
intencionalmente de território, essa escolha é reaplicada depois da transação,
sem alterar a nova seed principal.

Com um astro selecionado, o inspector também oferece `ORIGEM ↔ VARIAÇÃO`.
Esse fader aplica a mesma seed viva apenas ao stem em foco: `0` mantém sua
origem e `1` faz sua variação plena. A mudança é audível no corpo escolhido,
preserva todos os módulos, conexões e posições e não substitui a variação
coletiva.

O host Release recompilou e 19/19 testes passaram. Falta avaliar a ergonomia e
a qualidade musical da faixa em tela e áudio reais.

## Coorte expandida — 2026-09-03

Registro histórico de calibração, superado em produção pela integração 8→4
descrita abaixo. Não usar este ensaio para liberar oito stems no host atual.

O Campo passa a abrir oito corpos: `ACID`, `GENDYN`, `DRUMS`, o quarto módulo
definido pela seed, `MODAL`, `GALO`, `ATROPELO` e `DEJA VU`. Todos possuem
papel, presença, posição, relação inicial, stem e dança; podem ser selecionados
e movidos no Campo. O ensaio offline de 60 segundos a 48 kHz/256 samples
confirmou oito stems, saída finita e audível, zero xruns internos e zero
alocações no callback. A escuta humana no host físico continua obrigatória.

Cada astro da coorte agora abre, ao ser selecionado, o mesmo inspector
contextual: presença, ganho do stem, mute, solo e três controles locais. O
inspector concentrado no astro em foco substitui oito painéis simultâneos e
evita sobreposição. A validação visual/ergonômica humana desta ampliação ainda
é pendente.

## Eixo de Forças — recombinação sem pares forçados — 2026-09-06

O `EIXO DE FORÇAS` conserva os dois corpos manipuláveis, mas deixou de tratar
os macros como pares complementares. O círculo laranja controla
`AUTONOMIA` e `INTERVENÇÃO` por eixos independentes — portanto ambos podem
chegar a zero. O azul controla `DENSIDADE`, `TENSÃO` e `ESPAÇO` e também
participa de `RISCO`; os dois corpos se encontram em `MEMÓRIA`, `RISCO` e
`ESPAÇO`. O texto interno redundante foi removido e o raio de percurso foi
corrigido para que os dois círculos alcancem a borda da grade sem corte.

O próximo passo não é animá-los gratuitamente: trajetórias automáticas só
serão aceitas se a seed, as relações entre módulos e os acontecimentos
produzirem uma consequência imediatamente reconhecível na escuta, preservando
como âncora o desenho espacial do performer. O host Release recompilou; os
testes `performance_layout` e `performance_relational_phenomena` passaram. A
escuta humana desta nova recombinação permanece necessária.

## Ajustes de legibilidade do cabeçalho — 2026-09-06

`TEMPO / BPM` e `MASTER / dB` passaram a ter a mesma geometria: alvos de
210 px, leitura numérica de 64 px e a mesma extensão útil de trilha. A faixa
de `CENAS · MORPH` e `GESTOS` foi compactada de 100 para 72 px, preservando
botões de 28 px e devolvendo espaço ao instrumento. `MORPH` adotou o padrão
horizontal do Antitotem: 112 px de trilha limpa, sem caixa numérica interna, e
valor no rótulo (`MORPH 0.8 s`).

O contorno e o título do `EIXO DE FORÇAS` foram restaurados como margem da
zona, fora da área de interação dos círculos. O menu de idiomas agora usa
hífen ASCII (`EN - English`) para impedir o artefato de codificação `Â` no
PopupMenu. Release recompilado, teste de layout e `git diff --check` aprovados.

## Campo vivo: narrativa, profundidade e rastro — 2026-09-06

O Campo agora separa de modo explícito a âncora espacial desenhada pelo
performer da posição viva. A cada turno narrativo da seed, se `AUTONOMIA` não
estiver em zero, os corpos laranja e azul do `EIXO DE FORÇAS` fazem um desvio
bounded de 3,6 segundos e retornam à âncora. O alvo é derivado da seed e do
número do turno; os sete macros recebem mudanças quantizadas, para que o
acontecimento seja ouvido como transformação e não como nutrição contínua.

A Constelação usa `x/y/z` no mesmo cálculo que alimenta relações e pressão
musical. `z` passa a ser legível por perspectiva e escala; uma linha entre a
âncora e o corpo vivo mostra o rastro. Órbita coletiva gira em torno do
baricentro, enquanto órbita, pulso e figura-8 podem operar localmente. As
relações convertem distância, velocidade radial e angular em pulso, fratura,
matéria, memória, espacialidade, tensão, semente e silêncio. Build Release e
os testes de layout, engine, jogo espacial e fenômenos relacionais passaram.
Falta somente a validação A/B humana no host físico.

As conexões também passaram a reger a orquestração. Em cada foco narrativo, o
astro de primeiro plano chama; um cabo ativo que o alcança faz o astro ligado
responder, e os demais cabos ativos sustentam um pequeno conjunto. A força
depende de atividade, intensidade, presença e proximidade reais da relação. A
ordem interna dos módulos só serve como recuo quando nenhum cabo audível está
disponível: desenhar e conectar a constelação decide, portanto, uma consequência
musical concreta. A confirmação final continua sendo a escuta humana de
chamada, resposta e reorquestração no instrumento aberto.

## Seed como gramática combinatória — 2026-09-06

O seed não é uma lista curta de presets. Ele é uma partitura determinística:
para cada módulo, passo e acontecimento, produz decisões reproduzíveis de
contorno, articulação, registro, aproximação, pausa e ritmo. Assim uma mesma
seed pode sempre ser retomada, mas duas seeds da mesma família de módulos já
desenvolvem peças diferentes. A primeira aplicação cobre Acid, Gendyn e
Percussões; as famílias Breath, Voice, Digital e Memory deverão adotar a mesma
estrutura segundo suas identidades, sem transformar a variedade em ruído ou em
uma voz contínua. A regressão prova simultaneamente reprodução idêntica da
mesma seed e contraste entre as seeds Circuit `181` e `186`.

## Matriz composicional — 2026-09-06

Além das decisões locais, cada seed possui um perfil de composição comum a
toda a constelação: **estilo**, **dialeto harmônico**, alcance melódico,
impulso rítmico, brilho timbral, afinidade de acontecimentos, espacialidade e
coletividade. Ele impede que módulos sejam ilhas: o baixo pode sustentar, a
matéria responder, a percussão articular, a voz interromper e o vestígio
permanecer, de acordo com uma mesma lógica de peça. Nesta primeira integração,
o perfil já orienta escala e registro, velocidade do fluxo, cor do Circuit e
o peso da chamada–resposta entre corpos conectados. As outras famílias devem
traduzir a matriz com suas próprias matérias, não por cópia de parâmetros.

Cada realização deverá concluir-se como uma faixa pronta. O contrato de
abertura, desenvolvimento, coda, cauda e fechamento está em
[`docs/MORCEAU_PERFORMATIVO.md`](docs/MORCEAU_PERFORMATIVO.md).
O algoritmo que articula seed, módulos, jogo, dança e Constelação está em
[`docs/ALGORITMO_SEED_MODULOS.md`](docs/ALGORITMO_SEED_MODULOS.md).

## Correções de continuidade sonora — 2026-09-07

`NASCENTE` (até 03/11/2026 `ABERTURA`), `ESPIRAL`, `RASGO` e `BALANÇO` são transformações composicionais
do mesmo morceau: trocar de cena durante `PLAY` preserva o transporte e o
envelope da peça. O estado `playing` armazenado numa cena não pode interromper
uma performance que já está em curso. A regressão do motor percorre
`PLAY → ESPIRAL → RASGO → BALANÇO` com cenas documentadas como paradas e exige
áudio contínuo, sem a recuperação artificial por `STOP/PLAY`.

O quarto corpo preparado pela seed — Walsh, Hard Sync, Atari Punk, VOSIM ou
outro corpo admitido pela coorte — integra a abertura com presença, nutrição e
posição derivadas da seed. Ele não nasce mais como reserva `Dormant` de volume
zero. O músico pode silencá-lo, recolhê-lo ou reposicioná-lo intencionalmente,
mas a ausência de som não é mais o estado inicial involuntário.

Os controles locais também fazem parte da identidade da seed: os três valores
iniciais de cada módulo, inclusive o quarto, são derivados deterministicamente
da seed e aplicados tanto ao Campo de Matérias quanto à ecologia de áudio. O
MODAL recebe a frequência viva de sua frase e só rearticula em acontecimentos;
isso evita a fundamental fixa e não introduz alimentação contínua.

## Modal como memória interna da stem — 2026-09-07

No perfil de produção com quatro stems, `MODAL` não retorna mais como uma fonte
global sem astro proprietário. Ele se torna uma camada de memória dentro de
uma stem audível: prioriza o destino da relação de `MEMÓRIA`; sem esse cabo,
pertence ao `VESTÍGIO` disponível ou ao primeiro corpo vivo. Mute, solo, ganho,
estado de vida, frase e coda do astro proprietário continuam valendo.

Cada turno narrativo do Campo pode rearticular essa matéria uma única vez e
colore diretamente o módulo escolhido pelo acontecimento. Não existe relógio
autônomo para a camada interna: o relógio esparso permanece exclusivo da quinta
stem Modal, quando ela for expressamente liberada e calibrada. A cauda ainda
pode atravessar uma pausa curta de frase para tornar a decisão espacial
imediatamente audível, mas termina pelo decay da própria matéria — não como
drone. Posição, polígono e cabos nunca são alterados por esse submix.

## Oito lanes em quatro stems — primeira integração — 2026-09-08

O caminho de áudio agora separa a quantidade de módulos da quantidade de
saídas: o adaptador publica oito lanes internas e o `PerformanceStemBusMixer`
as agrupa em quatro buses. As camadas obedecem ao ganho, mute e solo do bus
proprietário, sem vazamento quando uma stem é silenciada.

O arranjo atual reserva quatro lanes ao Circuito quando disponíveis e completa
as lanes de abertura com matérias complementares da coorte; as quatro lanes
restantes são alimentadas por Sopro/Voz, Digital e Memória. Assim, mesmo uma
seed Circuito com apenas três corpos próprios não deixa uma lane muda.

Esta é a primeira integração funcional do arranjo 8→4, não a conclusão do
catálogo inteiro. No smoke de 60 s, o motor permaneceu sem alocações no
callback e em tempo real offline (7,53×); o custo foi maior que o arranjo de
quatro lanes (11,53×), portanto a próxima etapa é medir e otimizar antes de
liberar mais camadas simultâneas.
