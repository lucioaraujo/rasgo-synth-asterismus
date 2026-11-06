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
The Windows and macOS packages are built by the release workflow; on this
first release they had not yet been opened on a real Windows or Mac machine
(see the platform matrix in the README).

### Linux

- **`.deb` package** (Debian, Ubuntu and derivatives):
  `sudo apt install ./rasgo-synth-asterismus-<version>-Linux.deb`
  — the shortcut appears in the applications menu (Sound & Video).
- **`.tar.gz` archive**: extract it and run `usr/bin/rasgo-synth-asterismus`;
  the resources live in `usr/share/rasgo-synth-asterismus/` (the program finds
  them by itself).

### Windows

1. Run the installer `rasgo-synth-asterismus-<version>-win64.exe`.
2. If **"Windows protected your PC"** (SmartScreen) appears: click
   **More info** → **Run anyway**.
3. The shortcut is in the Start menu and on the desktop. The Visual C++
   runtime is inside the program: nothing else needs installing.

### macOS (Intel and Apple Silicon, 10.15 or newer)

1. Open the `.dmg` and drag **Rasgo Synth Asterismus** to Applications.
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

JUCE 9.0.0 is not in the repository (see [DEPENDENCIES.md](DEPENDENCIES.md)).
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
git clone --branch 9.0.0 --depth 1 https://github.com/juce-framework/JUCE.git third_party/JUCE
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
nesta primeira versão ainda não foram abertos numa máquina Windows ou Mac de
verdade (ver a matriz de plataformas no README).

### Linux

- **Pacote `.deb`** (Debian, Ubuntu e derivados):
  `sudo apt install ./rasgo-synth-asterismus-<versão>-Linux.deb`
  — o atalho aparece no menu de aplicativos (Som e Vídeo).
- **Arquivo `.tar.gz`**: extraia e rode `usr/bin/rasgo-synth-asterismus`;
  os recursos ficam em `usr/share/rasgo-synth-asterismus/` (o programa os
  encontra sozinho).

### Windows

1. Rode o instalador `rasgo-synth-asterismus-<versão>-win64.exe`.
2. Se aparecer "O Windows protegeu o computador" (SmartScreen): clique em
   **Mais informações** → **Executar assim mesmo**.
3. O atalho fica no Menu Iniciar e na Área de Trabalho. O runtime do Visual
   C++ vai dentro do programa: não é preciso instalar nada mais.

### macOS (Intel e Apple Silicon, 10.15 ou mais novo)

1. Abra o `.dmg` e arraste **Rasgo Synth Asterismus** para Aplicativos.
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

O JUCE 9.0.0 não vem no repositório (ver [DEPENDENCIES.md](DEPENDENCIES.md)).
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
git clone --branch 9.0.0 --depth 1 https://github.com/juce-framework/JUCE.git third_party/JUCE
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRASGO_PERFORMANCE_BUILD_APP=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

O app fica em `build/src/app/RasgoSynthAsterismusApp_artefacts/Release/`.
Os pacotes de Windows e macOS são montados pela integração contínua
(`.github/workflows/package.yml`); os passos de cada sistema estão lá.
