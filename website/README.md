# Site do Rasgo Synth Asterismus

**Estado: publicado** — versão 0.3.0 desde 2026-11-07 (com a página LEARN;
antes, a 0.2.0, de 2026-11-06); a 0.4.0 está preparada no branch
`release-0.4.0` (9/11), à espera da tag:
<https://lucioaraujo.github.io/rasgo-synth-asterismus/> (GitHub Pages do
repositório próprio). `PUBLICADO = True` no `gerar_paginas.py`: sem `noindex`,
o selo da versão e os links de download do release. O workflow
`.github/workflows/deploy-site.yml` publica a cada push que mude `website/**`
(e pelo botão manual).

## Numa versão nova

1. Conferir, na release, os nomes reais dos pacotes e ajustar `PACOTES` no
   `gerar_paginas.py` (os de hoje são os nomes padrão do CPack).
2. `PUBLICADO = True` no `gerar_paginas.py`; a versão fica em `VERSAO` no
   `gerar_paginas.py` e no `seo.py` (numa versão nova, mudar as duas).
3. O tutorial é o do app: `python3 tools/extrair_tutorial.py src/app/PerformanceApp.cpp > website/tutorial_textos.json`
   (da raiz do projeto). O site da 0.2.0 usa esse texto sem o que só existe
   na 0.3 (as 36 estrelas, CORTE · RES. · ECO · SALA, os cortes e a fita) —
   na release 0.3, extrair de novo, inteiro.
   O LEARN (as fichas das estrelas, 0.3) fica fora do site até a release:
   prévia com `python3 tools/extrair_learn.py > .local-build/previa-learn/learn_textos.json`
   e `python3 website/gerar_paginas.py --previa-learn .local-build/previa-learn`.
   Na release 0.3: `python3 tools/extrair_learn.py > website/learn_textos.json`
   e `LEARN_PUBLICO = True` no `gerar_paginas.py` (a página entra no menu).
4. Gerar: `python3 gerar_paginas.py && python3 seo.py`.
5. Ativar o GitHub Pages no repositório (Settings → Pages → GitHub Actions) —
   decisão e ação do autor.
6. No `deploy-site.yml`, devolver o gatilho `push` com `paths: website/**`
   (como no Trioio), ou disparar à mão uma vez.

## Como gerar

```sh
python3 gerar_paginas.py && python3 seo.py
```

- `gerar_paginas.py` é a fonte única: um modelo e os textos dos quatro idiomas.
  Gera `index.html` (português, canônica), `en.html`, `fr.html` e `es.html`.
  **Os .html são derivados.** Editar os textos no script.
- `seo.py` injeta os metadados (descrição, Open Graph, hreflang, JSON-LD) e
  escreve o `sitemap.xml`. É o código comum da família; só a configuração do
  topo é do Asterismus.

## Conteúdo e regras (da família RASGO)

- A faixa fixa `rasgo-strip` aponta para o portal; o cabeçalho próprio vem
  logo abaixo.
- As fontes são livres e embutidas, nunca fontes de sistema nomeadas: as
  mesmas do cabeçalho do app (Big Shoulders Display e IBM Plex Mono, SIL OFL
  1.1, licenças em `assets/fonts/`).
- O contato usa o diálogo de copiar o endereço, não `mailto:`.
- A paleta vem das cores da interface (`src/app/PerformanceApp.cpp`); não é a
  do portal RASGO.
- Os textos afirmam só o que foi feito e verificado (a matriz de plataformas
  do README).

## Imagens (derivadas, próprias)

| Arquivo | Origem |
|---|---|
| `assets/interface.jpg` | instantâneo da janela do app 0.2.0 (1920×1006), em 2026-11-04: instância isolada (HOME temporário), saída muda (`rasgo_mudo`), ganchos `RASGO_ASTERISMUS_AUTOPLAY=1` e `RASGO_ASTERISMUS_SNAPSHOT=1` (sem dicas), seed sorteada 4289420027, ~40 s tocando; JPEG q84. **Provisória**: o autor pode trocá-la por uma captura sua |
| `assets/og-asterismus.jpg` | o mesmo instantâneo, recorte central 1200×630, JPEG q85 |
| `assets/identity/rasgo_logo_2026.svg` | a logo RASGO do cabeçalho do app (`../assets/rasgo_logo_2026.svg`), arte de Lúcio de Araújo; na cópia do site o preto vira o âmbar `#d18a45`, a mesma troca que o app faz ao desenhá-la (6/11: em preto ela sumia no fundo verde) |

## Páginas, menu e ícones (6/11/2026)

Pedidos do autor: menu, tutoriais, a ligação com o Studio, favicon e ícones,
fundo que valorize a captura — "como fizemos nas páginas de outros
instrumentos da família rasgo". Seguindo o site do Aquorbium: menu
(Sobre · Tutorial · Instalação · Baixar · Studio) em todas as páginas;
`tutorial*.html` com o tutorial do app (texto extraído de
`src/app/PerformanceApp.cpp` para `tutorial_textos.json`, nos 4 idiomas —
ao mudar o tutorial do app, extrair de novo); `instalacao*.html` (resumo do
`INSTALL.md`, com FR e ES traduzidos aqui); a seção **Irmão do Rasgo Synth
Studio** (`#studio`) e o link no rodapé; fundo `#0b0e0d` (a captura verde
ganha moldura âmbar). Ícones: `asterismus-favicon.svg`, `favicon-32.png` e
`apple-touch-icon.png` (180, reduzido do 512) vêm de
`../packaging/icon/proposta/` — **a proposta de ícone do app, ainda à espera
da decisão do autor**; se ela mudar, trocar aqui também.

**O céu na apresentação (6/11):** `assets/ceu.js`, a cena do Asterismus
da família no portal RASGO (`RASGO_WEBSITE/assets/familia.js`), sozinha e
nas cores da interface, ao lado do texto de abertura (`.hero` em duas
colunas; no celular, abaixo). Para conferir um instante:
`index.html?quadro=3.3`. O script vai com `?v=20261106` no endereço — ao
mudá-lo, trocar a data.

## Proveniência das cópias

| Arquivo | Origem | sha256 | Licença |
|---|---|---|---|
| `seo.py` | `lucioaraujo/trioio` @ `cfbd744`, `website/seo.py` (ele mesmo do Antitotem `b285505`); só a configuração do topo mudou | `3772e6175490779a40939add16454c9aebb9b9aea4c91c06faaf7af8899a0990` na origem | AGPLv3+ (RASGO, mesmo autor) |
| `assets/contact.js` | idem, `website/assets/contact.js`, sem mudança | `55f650ea3bd79a2ccf0af306489bf1fdc90131d6005626416e001f6f92783d58` | AGPLv3+ (RASGO, mesmo autor) |
| `gerar_paginas.py`, `styles.css` | estrutura do mesmo site do Trioio @ `cfbd744`; modelo, textos, paleta e fontes do Asterismus | — | AGPLv3+ (RASGO, mesmo autor) |
| `assets/fonts/BigShouldersDisplay.ttf` | `../assets/fonts/` (Google Fonts) | `60e208dc276a1c35fc5b62e94f9fb959c40c11783a9eb7548175c14b1fbeb720` | SIL OFL 1.1 |
| `assets/fonts/IBMPlexMono-Regular.ttf` | `../assets/fonts/` (IBM Plex) | `6a3412f058c7d8dfd9170c41e85ade48e5156ecb89356110ca57a0a27734af46` | SIL OFL 1.1 |
| `assets/fonts/IBMPlexMono-Bold.ttf` | `../assets/fonts/` (IBM Plex) | `ac27abd6450a64dd94467580a02fe6235156d5b92f2926ebbc8e7489df64e0be` | SIL OFL 1.1 |

O site é um conjunto à parte do app: os arquivos copiados do Trioio mantêm a
licença deles (AGPLv3+, mesmo autor); o app segue GPLv3+.
