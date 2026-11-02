# Requisitos mínimos e perfis multiplataforma

O Performance será distribuído para máquinas diferentes. A capacidade de uma
máquina de desenvolvimento não pode virar requisito universal nem promessa de
que todos os módulos tocarão em qualquer computador.

## Requisitos mínimos provisórios

- sistema 64-bit com suporte ao backend de áudio usado pelo build;
- CPU x86-64 ou arm64 com pelo menos 4 núcleos lógicos;
- 8 GiB de RAM (16 GiB recomendados);
- saída de áudio estéreo funcional;
- espaço livre para o aplicativo, cache, logs e gravações;
- permissões para abrir dispositivo de áudio e gravar na pasta de usuário.

## Tela e escala

- mínimo provisório: 1280×720 px, em orientação horizontal;
- recomendado: 1920×1080 px ou superior;
- escala de sistema/DPI entre 100% e 150% deve manter textos e controles
  acessíveis;
- abaixo da resolução recomendada, o app deve usar layout compacto, ocultar
  painéis secundários e preservar apenas transporte, seed, mixer e módulo em
  foco;
- nenhum controle essencial pode depender de coordenadas exclusivas de
  1920×1080 ou ficar sobreposto em escalas altas.

Esses limites também são provisórios e precisam de validação visual em
Windows, macOS e Linux, incluindo monitores HiDPI e redimensionamento durante
a execução.

Esses valores são uma hipótese de distribuição, não uma certificação. Devem ser
validados por benchmark em Linux, Windows e macOS, com diferentes drivers,
sample rates e tamanhos de buffer.

## Verificação Linux — 2026-08-30

Foi realizado build limpo `Release` em diretório externo (`/tmp`) usando os
packages versionados V1–V4. O host JUCE e os testes compilaram; 9/9 testes
funcionais passaram. ALSA, Freetype, Fontconfig, EGL e GL foram encontrados.
`libcurl`, WebKitGTK e GTK X11 não estão instalados nesta máquina, mas não são
necessários para o host standalone atual. Windows e macOS continuam sem
toolchain disponível nesta sessão e não foram declarados validados.

O teste de dispositivo real também não pôde ser executado nesta sessão:
PulseAudio recusou a conexão, `aplay -l` não encontrou placas e `/dev/snd` não
existe no ambiente. Os benchmarks offline não devem ser confundidos com teste
de driver, latência ou xruns de hardware real.

## Configuração canônica do host — 2026-08-30

O preset `linux-release-host` agora aponta para `modal-matter` e para as coortes
V1–V4 no checkout vizinho `rasgo-synth-core/cmake`. A configuração, compilação
Release e o preset de testes foram executados com esse fluxo; 10/10 testes
funcionais passaram e o teste de estabilidade passou em 43,47 s. A configuração
não altera o Studio nem copia packages para dentro do Performance.

Atualização posterior do mesmo fluxo: o contrato de layout e a verificação de
artefatos foram incluídos; o preset funcional passou 12/12 testes e a
estabilidade passou em 41,28 s. Isso continua sendo validação técnica offline,
não certificação visual, de HiDPI ou de driver de áudio.

## Perfis de operação

- **Compatibilidade:** poucos corpos, relações simples, buffer maior e efeitos
  pesados desativados;
- **Performance:** capacidade intermediária, controles contextuais e relações
  espaciais bounded;
- **Estúdio:** maior número de corpos e análise, somente quando CPU, memória e
  áudio sustentarem a carga.

O app deve detectar capacidade no início, escolher um perfil seguro e permitir
redução de carga sem interromper o áudio. O limite real é definido por CPU,
xruns, memória, valores não finitos e resposta do dispositivo — não por número
teórico de stems.

Implementação atual: o host deriva `PERFORMANCE`/`COMPATIBILIDADE` do número
de CPUs lógicas e, no segundo perfil, bloqueia a ativação do quarto corpo
opcional sem interromper o transporte. Essa heurística é deliberadamente
conservadora e continua sujeita à validação por áudio real.

## Proteções obrigatórias

- nenhuma alocação, mutex ou I/O no callback;
- filas bounded para inserção, remoção e relações;
- limite de corpos e custo por módulo antes de aceitar uma inserção;
- fallback que recolha ou simplifique o módulo excedente, sem travar a máquina;
- indicador visível de perfil, CPU, xruns e sobrecarga;
- teste de restauração e encerramento limpo em cada plataforma.

Os requisitos só serão promovidos de provisórios a mínimos oficiais após uma
matriz de testes multiplataforma e sessões de escuta em hardware real.
