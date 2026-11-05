# Memória, álbum, percurso e registro de faixas (2026-11-02)

Decisões do autor: "o app abre algo novo a cada nova sessão; talvez criar um
dispositivo ou memória para salvar a sessão que o usuário desejar"; "veja nos
outros instrumentos da família RASGO"; "veja no studio como isso funciona — a
criação de faixas, registros também de faixas"; "pode fazer, nessa ordem,
seguindo a forma do céu".

## 1. O que a família já fazia (só lido)

| Instrumento | Mecanismo |
|---|---|
| **Aquorbium** (`src/app/PerformancePresetBank.h`, `PerformanceSceneFile.h`, commit `19140e2`) | 4 cenas locais com nome (até 32 caracteres), GUARDAR / RECUPERAR (sem reiniciar notas, sementes nem memória de DSP) / PADRÃO / RENOMEAR / DESFAZER; exportar e importar `.aquscene` (só parâmetros, nunca áudio; tamanho limitado; cópia de segurança) |
| **Navalha 2** | arquivo de projeto completo (Project v2, JSON) — é um editor de álbum |
| **Rasgo Modular** | `ControlSnapshot`, "memória de estados" interna (Mutable Frames), sem salvar do usuário |
| **Rasgo Synth Studio** (`AlbumPlan.hpp`, `AlbumRenderer.hpp`, commit `cfe1698`) | **Modo Álbum**: tema, número de faixas (até 40) e seed de álbum; seeds de faixa derivadas (peças relacionadas, não uma peça longa cortada); andamento por contorno (equilibrado, contrastante, ascendente, arco) e abertura em %; duração-alvo por faixa; nível igualado entre faixas pela mediana (×0,7 a ×1,4); **registro** `album_plan.json` com seed, BPM, duração planejada e real e arquivo de cada faixa. Dentro da faixa, o arco da peça anda sozinho até a coda. Renderização offline. |

Nada de código foi copiado; os conceitos estão citados nos cabeçalhos.

## 2. MEMÓRIA

`include/rasgo/performance/PerformanceMemory.hpp` (formato) e o botão
**MEMÓRIA** na linha de cenas. Oito lugares com nome; **GUARDAR ESTA SESSÃO
EM**, **LEMBRAR**, **RENOMEAR**, **APAGAR**, **DESFAZER A ÚLTIMA MUDANÇA**,
**SALVAR SESSÃO… / ABRIR SESSÃO…** (desde 4/11; antes EXPORTAR um lugar /
IMPORTAR para um lugar) um arquivo `.asterismus` — salvar grava o que está
soando, abrir já toca, sem ocupar um lugar; o seletor abre na última pasta (texto; só parâmetros, nunca
áudio; até 96 KB; validado campo a campo). Os lugares ficam em
`<dados do app>/RASGO/Rasgo Synth Asterismus/memorias/memoria-N.asterismus`
(no Linux, `~/.config/…`); gravação ao lado e troca, para um arquivo pela
metade nunca substituir o bom. **Nada é carregado ao abrir**: o app sempre
abre algo novo.

A memória guarda: a seed e a cena em que foi guardada (o documento de sessão
existente, com BPM, macros, presenças e relações capturados na hora), as
estrelas trocadas e fixadas, o mix, mute/solo, os controles locais, a
variação da seed, MASTER, TRAMA, VARIAÇÃO, DANÇA (do campo e de cada corpo) e
CÉU. **LEMBRAR** enfileira o território ao vivo (como uma seed nova) e
reaplica o estado do performer 300 ms depois, quando o motor já trocou de
território.

Mudança junto: **uma seed nova desfaz as trocas e fixações de estrela** (uma
seed nova é uma peça nova; a memória reaplica as suas).

## 3. ÁLBUM

`include/rasgo/performance/PerformanceAlbum.hpp` e o botão **ÁLBUM** na caixa
FORMA · PERCURSO / ÁLBUM do cabeçalho (desde 5/11; antes na faixa de título
das cenas). **NOVO ÁLBUM…** pede nome/tema, faixas (1–40, o limite
do Studio), seed do álbum (sorteada, editável), contorno do andamento e
abertura (0–35 %), em torno do TEMPO atual. A seed de cada faixa vem da seed
do álbum (splitmix32 com o número da faixa); a faixa entra ao vivo com o seu
andamento gravado em todas as cenas (para o PERCURSO não o desfazer). O
menu: **PRÓXIMA FAIXA**, **FAIXA ANTERIOR**, **RECOMEÇAR ESTA FAIXA**, **IR
PARA A FAIXA** (cada uma com seed, BPM e quantas tomadas), **ABRIR A PASTA**,
**ENCERRAR**.

Pasta: `~/Music/Rasgo Synth Performance/albuns/<data hora> <nome>/`. Com um
álbum ativo, o REC grava ali como `FAIXA_nn_<data hora>.wav` (com o
manifesto e os stems de sempre).

## 4. PERCURSO (seguindo a forma do céu)

Botão **PERCURSO** na caixa FORMA · PERCURSO / ÁLBUM do cabeçalho, à esquerda de FAIXA · DURAÇÃO (desde 5/11; antes na faixa de título das cenas). As cenas seguem a forma do
céu: ABERTURA → cena 1 (NASCENTE), DESENVOLVIMENTO → ESPIRAL, CONTRASTE →
RASGO, RETORNO → BALANÇO, com o MORPH escolhido. A forma: abertura uma vez,
depois desenvolvimento → contraste → retorno em ciclo, cada seção com 8 a 20
compassos. Ela só anda com a TRAMA ou a VARIAÇÃO; se as duas estiverem
desligadas, o PERCURSO liga a TRAMA. **Gravando, quando o retorno acaba, a
tomada pede a coda**: uma faixa é uma volta da forma. O performer intervém a
qualquer momento; o PERCURSO só chama a cena quando a seção muda.

## 5. REGISTRO DE FAIXAS

`album.json` na pasta do álbum, reescrito a cada tomada fechada: o plano
(todas as faixas: seed, BPM, quantas tomadas) e cada tomada na ordem (faixa,
seed, BPM, arquivo, memória lembrada, início, duração, loudness integrada,
true peak máximo, se foi com PERCURSO).

## 6. Gravação (2/11, decisões do autor)

- "A gravação começa quando o usuário define fazer a gravação": **PLAY só
  toca**; **REC** abre a tomada (e começa a tocar, se parado); gravando, o
  botão vira **CODA**. Antes, todo PLAY abria uma tomada. Verificado no app
  mudo e isolado: com autoplay, nenhuma tomada criada.
- Prefixo das tomadas: `RASGO_ASTERISMUS_` (antes `RASGO_PERFORMANCE_`; a
  pasta e o identificador do formato no manifesto mantêm o nome antigo).
- **Destaque**: gravando, moldura e caixa de transporte vermelhas, ponto
  pulsando, tempo da tomada; coda pedida ou próxima, âmbar, quadrado
  pulsando mais rápido, "CODA · FINALIZANDO" / "CODA EM BREVE". O caminho do
  arquivo na linha de status e na dica do REC.
- **FAIXA · DURAÇÃO** (1 / 2 / 3 / 5 MIN; pedido: "um dispositivo como no
  Antitotem, de no máximo 5 minutos; como no Studio, um ciclo, um percurso
  até o fim da gravação"). Do Antitotem (`src/app/Main.cpp`, commit
  `645e5ad`, só lido): quatro botões explícitos, um clique arma e grava, com
  contagem regressiva, só o escolhido aceso, a duração é um alvo e não um
  corte. Do Studio: o arco da peça se ajusta à duração. Aqui: a forma do céu
  recebe um **plano de compassos** (`PerformanceSkyDirector::setFormPlan`) —
  abertura 15 %, desenvolvimento 35 %, contraste 25 %, retorno 25 % dos
  compassos até a coda, no TEMPO da faixa — e o PERCURSO troca as cenas com
  ela; a coda é pedida 5,2 s antes do alvo (coda 4 s + cauda 1,2 s), e o
  arquivo fecha em torno do tempo escolhido. Ao fechar, o plano se desfaz.
  Teste no diretor: com 2/4/3/3 compassos a 120 BPM, desenvolvimento em
  4,1 s, contraste em 11,9 s, retorno em 17,7 s (o tempo elástico adianta um
  pouco). O manifesto da tomada traz `targetSeconds`.

## 7. Pastas e correções (3/11)

- **Uma pasta por tomada**: `recordings/AAAA-MM-DD/HH-MM-SS seed N/` com a
  mistura, os 8 stems e o manifesto; num álbum, `<álbum>/FAIXA_nn HH-MM-SS/`
  (o registro guarda o caminho dentro do álbum). As 355 gravações de teste
  antigas foram removidas (autorizado; da lixeira só os itens daquela pasta;
  +17 GB livres).
- **Chamar uma cena não reinicia a forma do céu** (com a mesma seed); só um
  território novo recomeça. Antes o PERCURSO ficava preso entre abertura e
  desenvolvimento e o clique numa cena voltava à abertura.
- Numa faixa com duração, quando o retorno acaba antes do alvo (o tempo
  elástico adianta ~4 %), o PERCURSO fica no BALANÇO até a coda.
- Gancho de teste `RASGO_ASTERISMUS_AUTOREC=1` (aperta 1 MIN sozinho quando o
  áudio fica pronto). **Verificado no app mudo e isolado**: pasta
  `2026-11-03/01-…-… seed N/`, tomada de 60,1 s finalizada, rastro
  `percurso 0, 1, 2, 3` e `coda:duracao`.

## 8. Falta

- Masterização de álbum offline (igualar o nível entre as faixas, como o
  Studio) — a parte 4 do pedido.
- Ouvir uma tomada de dentro do app (hoje: ABRIR A PASTA).
- Validar no app (compilado só quando o autor fechar o app) e a escuta.
