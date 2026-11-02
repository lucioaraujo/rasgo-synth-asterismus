# O que falta para publicar o Asterismus (levantamento de 2026-10-30)

Pergunta do autor: "o que falta para a publicação?"; "verifique também em
relação ao rasgo synth studio". Referências: o padrão de distribuição RASGO
(Trioio e Maravi publicados em repositório próprio; Navalha 2 e Antitotem no
padrão de saída de áudio) e a lista de publicação do Studio
(`RASGO_SYNTH_STUDIO/PUBLICATION_CHECKLIST.md`, só lida).

Legenda: **feito** · **falta** (trabalho técnico) · **decisão** (do autor).

## 1. Som e música

| Item | Estado |
|---|---|
| Saída na arquitetura comum RASGO (ganho fixo, limitador true peak, teto −1,41 dBTP, dither TPDF, sem normalização ao vivo) | **feito** em 30/10 — −18 a −20 LUFS com MASTER −6 dB (`docs/SAIDA_DE_AUDIO.md`) |
| Masterização de entrega (álbum/streaming), offline | **falta** — fluxo separado, como o TRACK/ALBUM MASTER do Navalha 2 |
| Medidor de loudness/true peak no rodapé | **feito** (30/10, BS.1770, EBU 3341) |
| Detector de true peak 4× no limitador (como o Navalha 2) | **feito** (30/10) — teto de −1,41 dBTP garantido também entre amostras |
| Escuta e aceite musical do autor (trama, tempo, movimento, fratura, cabos, famílias, giro) | **decisão** — nada disso foi ouvido pelo autor ainda |
| Decisões abertas da auditoria da interface (§8) e do código subaproveitado | **decisão** |

## 2. Aplicativo

| Item | Estado |
|---|---|
| Versão | `0.1.16-dev` — **decisão**: número da primeira publicação (0.2.0?) |
| Salvar / abrir sessão (o formato existe, sem botões) | **decisão** (onde, e se reabre a última) |
| Tutorial nos quatro idiomas com o modelo do céu | **feito** (seção 7) |
| Interface: textos cortados, nomes | **feito** (auditoria 29/10) |
| Recuperação de falha / áudio que trava | parcial — a guarda reabre o dispositivo (29/10) |

## 3. Empacotamento e distribuição (padrão RASGO)

| Item | Estado |
|---|---|
| Linux: `.deb` e `.tar.gz` (CPack), instalação FHS conferida (binário instalado abre e acha as fontes) | **feito** (30/10) — AppImage e teste em contêiner/distribuição mais velha: **falta** (CI) |
| Windows: runtime MSVC estático; instalador NSIS com atalho no menu Iniciar (`CPACK_PACKAGE_EXECUTABLES`) e ícone | **feito no CMake** (30/10) — compilar e conferir com `dumpbin` só na CI/Windows: **falta** |
| macOS: Universal 2 (x86_64;arm64), 10.15, antes do `project()`; `.dmg` (DragNDrop) | **feito no CMake** (30/10; o código não usa `std::filesystem` nem `from_chars/to_chars`) — selar ad hoc e conferir na CI/Mac: **falta** |
| Sem assinatura paga: guia de instalação (Gatekeeper, SmartScreen) | **feito** — `INSTALL.md` |
| Ícone do aplicativo | **decisão** — proposta em `packaging/icon/proposta/` (gerada por `tools/gerar_icone_proposta.py`); aprovada, copiar para `packaging/icon/` e o build a embute |
| CI multiplataforma (portão de lançamento) | **feito o arquivo** (2/11): `.github/workflows/package.yml`, só em tags `v*` e pelo botão manual — **nunca rodou**; a primeira execução é decisão do autor (~160 min cobrados). AppImage e testes de arranque em outras distribuições: **falta** |
| Repositório público próprio | **feito** (2/11, "pode criar" do autor): `lucioaraujo/rasgo-synth-asterismus`, aninhado e ignorado pelo monorepo; os diários (`CONTINUIDADE.md`, `TAREFAS.md`) ficam só no monorepo privado |

## 4. Licenças, créditos, proveniência

| Item | Estado |
|---|---|
| `LICENSE` (GPL-3.0-or-later; o JUCE pela via AGPLv3) | **feito** |
| `THIRD_PARTY_NOTICES.md` | **feito** (30/10) — master do Antitotem (AGPL, combinação pela GPLv3 §13) e conceitos citados |
| `CREDITS.md` e `DEPENDENCIES.md` | **feito** (30/10) |
| DSP copiado do Studio (`vendor/studio-dsp`) com proveniência | **feito** |
| Fontes do cabeçalho (OFL, com licenças) | **feito** |
| `third_party/JUCE` fora do Git | **feito** (`.gitignore`) |
| Nenhum áudio, caminho de máquina ou `0_BRAINSTORM` no repositório | **feito** (verificado) |

## 5. Portal e página

| Item | Estado |
|---|---|
| Cartão no portal RASGO e página do instrumento (com faixa fixa do portal) | **falta** |
| Captura 1920×1006 para o cartão | **decisão** — o autor faz a captura (padrão) |
| Manual / página pública nos quatro idiomas | **falta** |

## 6. Comparação com o Rasgo Synth Studio (só leitura)

O Studio está mais adiantado em **distribuição** e **documentação pública**:
tem `INSTALL.md`, `CREDITS.md`, `DEPENDENCIES.md`, `AUDIO_OUTPUT_POLICY.md`,
`packaging/` (Linux, AppImage, portabilidade), runtime MSVC estático no
CMake, quatro apresentações públicas e tutorial gerado dos manuais, pacote
Linux verificado (11/10). Mas a sua própria lista registra que **não passou
na escuta do autor** (vozes desiguais, baixo e bateria fracos, timbres
repetidos), mediu −26 LUFS nas tomadas, e a recuperação de falhas, o pacote
final, os avisos e a aprovação de publicação seguem abertos; nada foi
publicado.

O Asterismus está à frente em **saída de áudio** (−16 LUFS com o master
novo) e no **modelo musical**; atrás em **empacotamento e documentação
pública**. O que se pode reaproveitar do Studio, como modelo de documento
(sem copiar código): a estrutura de `INSTALL.md`, `CREDITS.md`,
`DEPENDENCIES.md`, `AUDIO_OUTPUT_POLICY.md` e a lista de publicação.

### 6.1 Saída de áudio do Studio (medida em 30/10, só leitura)

Tomadas recentes em `~/Music/Rasgo Synth` (ffmpeg `ebur128`, BS.1770):

| Tomada | Loudness | LRA | True peak |
|---|---|---|---|
| 17/10 01:46 | −23,5 LUFS | 17,5 LU | −2,0 dBTP |
| 16/10 20:24 | −20,4 LUFS | 8,4 LU | −2,1 dBTP |
| 16/10 01:26 | −17,9 LUFS | 8,5 LU | −2,4 dBTP |

O Studio sai bem mais alto do que o Asterismus saía (−44 a −50 LUFS) e com
teto respeitado (≈ −2 dBTP), mas com loudness irregular entre peças (6 dB de
diferença) — a própria lista dele registra −26 LUFS em outras tomadas. O
`AUDIO_OUTPUT_POLICY.md` do Studio é, apesar do nome, uma política de
arquivos gerados; o padrão técnico de saída comum é
`RASGO_DOCUMENTATION/architecture/SAIDA_AUDIO_COMUM.md`, que lista Aquorbium,
Navalha 2 e o Performance (Asterismus), **não o Studio**. Adotar a saída
comum no Studio fica para quem trabalha nele (o Studio segue com o autor e o
Codex).

O Asterismus agora segue a saída comum: ganho fixo, sem normalização ao vivo,
limitador com true peak 4×, teto −1,41 dBTP, medição BS.1770, dither TPDF.
Pendências do padrão para ele: peak hold e clip latch no rodapé, redução de
ganho e RMS na telemetria, correlação/mono, taps nomeados de gravação
(pré-master, pré-safety, pós-safety), PCM16 e float na exportação, teste
prolongado (soak).

## 7. Ordem proposta

1. Técnico, sem decisão — **feito em 30/10**: `.gitignore` do JUCE;
   `CREDITS.md`, `DEPENDENCIES.md`, avisos; empacotamento no CMake (Linux
   conferido; Windows e macOS só na CI); `INSTALL.md`; licença e avisos
   dentro do pacote; medidor de loudness e true peak.
2. Decisões do autor: versão, repositório, ícone, CI, salvar sessão.
3. Escuta e aceite musical.
4. CI verde → publicação; teste do autor em Windows e macOS reais depois
   (portão de lançamento é a CI).
