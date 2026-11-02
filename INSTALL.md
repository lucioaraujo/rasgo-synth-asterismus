# Instalar o Rasgo Synth Asterismus

O Asterismus é distribuído sem assinatura digital paga (decisão do projeto
RASGO). Por isso o Windows e o macOS mostram um aviso na primeira abertura;
os passos abaixo dizem o que fazer. O código-fonte completo acompanha cada
publicação (GPLv3; JUCE pela via AGPLv3).

## Linux

- **Pacote `.deb`** (Debian, Ubuntu e derivados):
  `sudo apt install ./rasgo-synth-asterismus-<versão>-Linux.deb`
  — o atalho aparece no menu de aplicativos (Som e Vídeo).
- **Arquivo `.tar.gz`**: extraia e rode `usr/bin/rasgo-synth-asterismus`;
  os recursos ficam em `usr/share/rasgo-synth-asterismus/` (o programa os
  encontra sozinho).
- O pacote atual foi construído no Ubuntu 24.04 (glibc 2.38); distribuições
  mais antigas precisarão do pacote da CI, construído num sistema mais velho.

## Windows

1. Rode o instalador `rasgo-synth-asterismus-<versão>-win64.exe`.
2. Se aparecer "O Windows protegeu o computador" (SmartScreen): clique em
   **Mais informações** → **Executar assim mesmo**.
3. O atalho fica no Menu Iniciar e na Área de Trabalho. O runtime do Visual
   C++ vai dentro do programa: não é preciso instalar nada mais.

## macOS (Intel e Apple Silicon, 10.15 ou mais novo)

1. Abra o `.dmg` e arraste **Rasgo Synth Asterismus** para Aplicativos.
2. Na primeira vez, o macOS diz que não pode verificar o desenvolvedor:
   clique com o botão direito no app → **Abrir** → **Abrir**. (Em versões
   recentes: Ajustes do Sistema → Privacidade e Segurança → **Abrir Mesmo
   Assim**.)
3. Se ainda assim recusar, no Terminal:
   `xattr -dr com.apple.quarantine "/Applications/Rasgo Synth Asterismus.app"`

## Onde ficam as gravações

`~/Music/Rasgo Synth Performance/recordings/` (tomada mixada em WAV 24 bits
com dither, e os stems de cada estrela) e o diagnóstico em
`~/Music/Rasgo Synth Performance/diagnostico/`.
