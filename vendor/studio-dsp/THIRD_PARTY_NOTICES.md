# Third-Party Notices

Revisão: 9 de agosto de 2026.

Este arquivo identifica software de terceiros usado para construir ou executar
o Rasgo Synth. Ele complementa, mas não substitui, os textos integrais das
licenças aplicáveis. Créditos históricos e influências conceituais estão em
[`CREDITS.md`](CREDITS.md).

## 1. Código próprio do Rasgo Synth

O código específico do Rasgo Synth é copyright © 2026 Lucio de Araujo e é
distribuído sob a GNU General Public License, versão 3, conforme
[`LICENSE`](LICENSE).

O motor em `rasgo-synth-core/` usa apenas a biblioteca padrão de C++ e APIs
básicas do sistema operacional. A auditoria do código atual não encontrou
firmware, código-fonte, amostras de áudio, imagens ou outros ativos copiados de
Kosmische, SuperCollider, Mutable Instruments ou dos circuitos e instrumentos
citados em `CREDITS.md`.

Equações matemáticas, comportamentos musicais, topologias descritas
publicamente e referências conceituais não são aqui classificados como
componentes de software incorporados. As implementações correspondentes foram
escritas para o Rasgo Synth.

## 2. JUCE — aplicativo gráfico

O aplicativo em `rasgo-synth-app/` é construído com o
**[JUCE](https://github.com/juce-framework/JUCE)**. Os arquivos locais
examinados declaram **JUCE 9.0.0**; `JUCE-master/` é ignorado pelo Git e não faz
parte do código publicado neste repositório. A referência reproduzível adotada
para novos checkouts é a tag `9.0.0`, commit
`f8f8864172464b9adf9eba6101e1f784838d1597`, registrada em
[`DEPENDENCIES.md`](DEPENDENCIES.md).

O script `rasgo-synth-app/build_juce_minimal.sh` compila os seguintes módulos:

- `juce_core`;
- `juce_events`;
- `juce_data_structures`;
- `juce_graphics`;
- `juce_gui_basics`;
- `juce_audio_basics`;
- `juce_audio_devices`.

Os módulos JUCE 9 são oferecidos sob licença dupla:

- **GNU Affero General Public License v3 (AGPLv3)**; ou
- uma **licença comercial do JUCE**.

Consulte o [aviso oficial do
JUCE](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md) e escolha a
opção aplicável antes de distribuir o aplicativo. Quando o JUCE é utilizado sob
a opção AGPLv3, a distribuição e o uso do aplicativo combinado precisam
observar as obrigações aplicáveis da AGPLv3. Uma licença comercial do JUCE segue
os termos comerciais correspondentes. A licença do motor Rasgo Synth, por si
só, não altera a licença do JUCE.

O nome local `JUCE-master` não fixa uma revisão por si só. Builds de release
devem partir do commit registrado em `DEPENDENCIES.md` e conferir novamente o
arquivo `LICENSE.md` daquela revisão.

## 3. Componentes compilados através do JUCE

O procedimento de build atual também compila componentes mantidos dentro da
árvore do JUCE. As cópias dos textos integrais abaixo se encontram no checkout
do JUCE indicado em cada linha e devem acompanhar qualquer pacote binário na
forma exigida pelas respectivas licenças.

| Componente | Titular/autoria indicada no código examinado | Licença | Texto no checkout do JUCE |
| --- | --- | --- | --- |
| [HarfBuzz](https://harfbuzz.github.io/) | Vários contribuidores e organizações; relação completa no arquivo de licença | Old MIT | `modules/juce_graphics/fonts/harfbuzz/COPYING` |
| [SheenBidi](https://github.com/Tehreer/SheenBidi) 2.9.0 | Copyright © 2014–2025 Muhammad Tayyab Akram | Apache License 2.0 | `modules/juce_graphics/unicode/sheenbidi/LICENSE` |
| [LunaSVG](https://github.com/sammycage/lunasvg) | Copyright © 2020–2025 Samuel Ugochukwu | MIT | `modules/juce_graphics/drawables/lunasvg/LICENSE` |
| [PlutoVG](https://github.com/sammycage/plutovg) | Copyright © 2020–2025 Samuel Ugochukwu | MIT | `modules/juce_graphics/drawables/lunasvg/plutovg/LICENSE` |

Esses componentes não são contribuições de Lucio de Araujo e não são
relicenciados pela licença do Rasgo Synth. Devem ser preservados seus avisos de
copyright, permissões, condições e exclusões de garantia.

O próprio arquivo de licença da revisão escolhida do JUCE é a fonte de verdade
para outros componentes que possam passar a ser compilados por uma atualização
do framework ou do script de build.

## 4. Bibliotecas fornecidas pelo sistema

No GNU/Linux, o script de build referencia bibliotecas fornecidas pelo sistema,
incluindo:

- FreeType e Fontconfig;
- X11, Xext e Xrender;
- libpng, libjpeg e zlib;
- ALSA (`libasound`);
- bibliotecas de execução do compilador e do sistema, como `libstdc++`,
  `libgcc`, `libm`, `libdl`, `pthread` e a biblioteca C.

Essas bibliotecas não são armazenadas neste repositório. Elas são resolvidas no
sistema de build e de execução, e mantêm seus próprios titulares e licenças. A
lista efetiva pode variar por plataforma, distribuição, configuração do linker
e versão do JUCE. O responsável por um pacote binário deve gerar novamente a
lista de dependências do artefato final e incluir os avisos exigidos pelas
versões realmente distribuídas.

## 4.1 Fontes distribuídas com o aplicativo

| Família | Titular indicado no aviso oficial | Licença | Arquivos e texto integral |
|---|---|---|---|
| Big Shoulders | The Big Shoulders Project Authors | SIL Open Font License 1.1 | `rasgo-synth-app/assets/fonts/BigShouldersDisplay.ttf` e `LICENSE-BigShoulders.txt` |
| IBM Plex Mono | Copyright © 2017 IBM Corp.; nome reservado “Plex” | SIL Open Font License 1.1 | `rasgo-synth-app/assets/fonts/IBMPlexMono-Regular.ttf`, `IBMPlexMono-Bold.ttf` e `LICENSE-IBMPlexMono.txt` |

O inventário e a proveniência dos demais assets do aplicativo estão em
`rasgo-synth-app/assets/README.md`. A revisão e os hashes exatos dos binários de
fonte devem ser registrados antes de uma distribuição binária.

## 5. Projetos citados, mas não incorporados

Os itens seguintes são referências conceituais ou históricas, não dependências
embarcadas no código atual:

- Kosmische, de Nicolas Steven Miller — GNU GPL v3;
- Chaos UGens do SuperCollider, de Lance Putnam — GPL v2 ou posterior na fonte
  examinada;
- firmware Eurorack da Mutable Instruments, de Émilie Gillet — licenças que
  variam por módulo e revisão;
- esquemas, módulos e instrumentos identificados em `CREDITS.md`.

A licença de uma referência não deve ser descrita como “herdada” pelo Rasgo
Synth quando nenhum código daquela referência foi incorporado. Do mesmo modo,
a compatibilidade entre licenças não transforma uma influência conceitual em
dependência ou coautoria.

## 6. Checklist para distribuição

Antes de publicar código ou binários do aplicativo:

1. conferir o checkout contra o commit do JUCE fixado em `DEPENDENCIES.md`;
2. escolher e documentar a opção de licença do JUCE usada no build;
3. incluir `LICENSE`, este arquivo e os textos integrais das licenças de todos
   os componentes compilados ou redistribuídos;
4. conferir as dependências do binário final na plataforma de lançamento;
5. preservar os avisos de copyright de terceiros;
6. atualizar este inventário sempre que módulos, bibliotecas, fontes, imagens,
   amostras ou outros ativos externos forem adicionados.

Este inventário descreve o estado do repositório e do procedimento de build na
data indicada no início do arquivo; não é aconselhamento jurídico.
