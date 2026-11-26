# Rasgo Synth Asterismus — Installation Guide / Guia de Instalação

- [English](#english)
- [Português](#português)

## English

Asterismus is distributed without a paid code-signing certificate (a RASGO
project decision). Windows and macOS therefore show a warning the first time
you open it; the steps below say what to do. The complete source code comes
with every release (GPLv3; JUCE under its AGPLv3 option).

Packages are on the
[releases page](https://github.com/lucioaraujo/rasgo-synth-asterismus/releases).
The Windows and macOS packages are built by the release workflow; they have
not yet been opened on a real Windows or Mac machine (see the platform matrix
in the README).

To verify one downloaded file, download `SHA256SUMS` from the same release and
run `grep -F '<filename>' SHA256SUMS | sha256sum -c -`, replacing
`<filename>` with the exact package name.

### Linux

- **`.deb` package** (Debian, Ubuntu and derivatives):
  `sudo apt install ./rasgo-synth-asterismus-<version>-linux-x86_64.deb`
  — the shortcut appears in the applications menu (Sound & Video).
- **AppImage**: make it executable, then run it:
  ```sh
  chmod +x rasgo-synth-asterismus-<version>-linux-x86_64.AppImage
  ./rasgo-synth-asterismus-<version>-linux-x86_64.AppImage
  ```
  If the system has no FUSE support, run it with
  `APPIMAGE_EXTRACT_AND_RUN=1 ./rasgo-synth-asterismus-<version>-linux-x86_64.AppImage`.
- **`.tar.gz` archive** (other distributions): extract it, enter the extracted
  directory and run `./install.sh`. This installs for the current user and
  needs Python 3. Remove it with `./install.sh --remove`. The application and
  its resources are installed together under `~/.local/`.

### Windows

1. Run the installer `rasgo-synth-asterismus-<version>-windows-x64.exe` (or
   extract the portable `rasgo-synth-asterismus-<version>-windows-x64.zip`).
2. If **"Windows protected your PC"** (SmartScreen) appears: click
   **More info** → **Run anyway**.
3. The shortcut is in the Start menu and on the desktop. The Visual C++
   runtime is inside the program: nothing else needs installing.

### macOS (Intel and Apple Silicon, 10.15 or newer)

1. Open `rasgo-synth-asterismus-<version>-macos-universal.dmg` and drag
   **Rasgo Synth Asterismus** to Applications.
2. The first time, macOS says it cannot verify the developer: right-click the
   app → **Open** → **Open**. (On recent versions: System Settings → Privacy &
   Security → **Open Anyway**.)
3. If it still refuses, in Terminal:
   `xattr -dr com.apple.quarantine "/Applications/Rasgo Synth Asterismus.app"`

### Where the recordings go

`~/Music/Rasgo Synth Asterismus/recordings/` (the mixed take as a 24-bit WAV
with dither, one WAV per star and a JSON manifest, one folder per take),
albums in `~/Music/Rasgo Synth Asterismus/albuns/`, and the diagnostics in
`~/Music/Rasgo Synth Asterismus/diagnostico/`.

### Building from source

JUCE 9.0.3 is not in the repository (see [DEPENDENCIES.md](DEPENDENCIES.md)).
On Debian/Ubuntu Linux, the system libraries JUCE needs (the same ones the
release workflow installs):

```sh
sudo apt-get install -y build-essential cmake git \
  libasound2-dev libjack-jackd2-dev libcurl4-openssl-dev \
  libfreetype6-dev libfontconfig1-dev \
  libx11-dev libxcomposite-dev libxcursor-dev libxext-dev \
  libxinerama-dev libxrandr-dev libxrender-dev libgtk-3-dev
```

Then, in the repository folder:

```sh
git clone --branch 9.0.3 --depth 1 https://github.com/juce-framework/JUCE.git third_party/JUCE
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRASGO_PERFORMANCE_BUILD_APP=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

The app is in `build/src/app/RasgoSynthAsterismusApp_artefacts/Release/`.
The Windows and macOS packages are assembled by the release workflow
(`.github/workflows/package.yml`); each system's steps are there.

## Português

O Asterismus é distribuído sem assinatura digital paga (decisão do projeto
RASGO). Por isso o Windows e o macOS mostram um aviso na primeira abertura;
os passos abaixo dizem o que fazer. O código-fonte completo acompanha cada
publicação (GPLv3; JUCE pela via AGPLv3).

Os pacotes ficam na
[página de releases](https://github.com/lucioaraujo/rasgo-synth-asterismus/releases).
Os de Windows e macOS são montados pela integração contínua da publicação;
ainda não foram abertos numa máquina Windows ou Mac de verdade (ver a matriz
de plataformas no README).

Para conferir um arquivo baixado, baixe `SHA256SUMS` da mesma publicação e
rode `grep -F '<nome-do-arquivo>' SHA256SUMS | sha256sum -c -`, trocando
`<nome-do-arquivo>` pelo nome exato do pacote.

### Linux

- **Pacote `.deb`** (Debian, Ubuntu e derivados):
  `sudo apt install ./rasgo-synth-asterismus-<versão>-linux-x86_64.deb`
  — o atalho aparece no menu de aplicativos (Som e Vídeo).
- **AppImage**: torne-o executável e rode-o:
  ```sh
  chmod +x rasgo-synth-asterismus-<versão>-linux-x86_64.AppImage
  ./rasgo-synth-asterismus-<versão>-linux-x86_64.AppImage
  ```
  Se o sistema não tiver suporte a FUSE, rode com
  `APPIMAGE_EXTRACT_AND_RUN=1 ./rasgo-synth-asterismus-<versão>-linux-x86_64.AppImage`.
- **Arquivo `.tar.gz`** (outras distribuições): extraia, entre na pasta e rode
  `./install.sh`. A instalação fica no usuário atual e requer Python 3. Para
  remover, rode `./install.sh --remove`. O app e os recursos são instalados
  juntos em `~/.local/`.

### Windows

1. Rode o instalador `rasgo-synth-asterismus-<versão>-windows-x64.exe` (ou
   extraia o arquivo portátil `rasgo-synth-asterismus-<versão>-windows-x64.zip`).
2. Se aparecer "O Windows protegeu o computador" (SmartScreen): clique em
   **Mais informações** → **Executar assim mesmo**.
3. O atalho fica no Menu Iniciar e na Área de Trabalho. O runtime do Visual
   C++ vai dentro do programa: não é preciso instalar nada mais.

### macOS (Intel e Apple Silicon, 10.15 ou mais novo)

1. Abra `rasgo-synth-asterismus-<versão>-macos-universal.dmg` e arraste
   **Rasgo Synth Asterismus** para Aplicativos.
2. Na primeira vez, o macOS diz que não pode verificar o desenvolvedor:
   clique com o botão direito no app → **Abrir** → **Abrir**. (Em versões
   recentes: Ajustes do Sistema → Privacidade e Segurança → **Abrir Mesmo
   Assim**.)
3. Se ainda assim recusar, no Terminal:
   `xattr -dr com.apple.quarantine "/Applications/Rasgo Synth Asterismus.app"`

### Onde ficam as gravações

`~/Music/Rasgo Synth Asterismus/recordings/` (a tomada mixada em WAV de 24 bits
com dither, um WAV por estrela e o manifesto JSON, uma pasta por tomada), os
álbuns em `~/Music/Rasgo Synth Asterismus/albuns/` e o diagnóstico em
`~/Music/Rasgo Synth Asterismus/diagnostico/`.

### Compilar a partir do código

O JUCE 9.0.3 não vem no repositório (ver [DEPENDENCIES.md](DEPENDENCIES.md)).
Num Linux Debian/Ubuntu, as bibliotecas do sistema que o JUCE pede (as
mesmas que a integração contínua instala):

```sh
sudo apt-get install -y build-essential cmake git \
  libasound2-dev libjack-jackd2-dev libcurl4-openssl-dev \
  libfreetype6-dev libfontconfig1-dev \
  libx11-dev libxcomposite-dev libxcursor-dev libxext-dev \
  libxinerama-dev libxrandr-dev libxrender-dev libgtk-3-dev
```

Depois, na pasta do repositório:

```sh
git clone --branch 9.0.3 --depth 1 https://github.com/juce-framework/JUCE.git third_party/JUCE
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRASGO_PERFORMANCE_BUILD_APP=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

O app fica em `build/src/app/RasgoSynthAsterismusApp_artefacts/Release/`.
Os pacotes de Windows e macOS são montados pela integração contínua
(`.github/workflows/package.yml`); os passos de cada sistema estão lá.
