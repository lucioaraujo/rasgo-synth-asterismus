# Propostas de logo do Asterismus (2026-11-06)

Pedido do autor: "crie uma logo baseada na constelação e estrelas do
instrumento". Três símbolos em SVG (120×120, sem fonte embutida), nas cores
da interface (âmbar `#d18a45`, creme `#ece7d8`, verde das linhas `#6f9b8d`):

- **A · asterismo** — a figura da Constelação: seis estrelas, a que lidera em
  âmbar com a órbita pontilhada (a mesma linguagem da proposta de ícone em
  `../icon/proposta/`).
- **B · a trama** — a constelação em linha fina e dois pares em foco acesos
  em âmbar; uma estrela com o rastro pontilhado da física.
- **C · monograma** — um asterismo em forma de A: as estrelas desenham a
  letra, e o topo é a estrela que lidera.

`folha-de-propostas.png`: os três em tamanho grande, em 64/32/16 px sobre o
verde da interface e ao lado da palavra ASTERISMUS. Nada foi aplicado: a
escolha é do autor; depois, o símbolo vai ao cabeçalho do site, ao favicon e,
se o autor quiser, ao ícone do app.

## 2ª rodada (6/11): o céu da animação do portal

O autor não quis A, B nem C: "achei melhor o que fez na animação para o
portal web". D e E são a mesma figura da cena RASGO SYNTH ASTERISMUS de
`RASGO_WEBSITE/assets/familia.js` (as oito estrelas, as arestas, dois pares
em foco acesos e a estrela que lidera em vermelho), parada num quadro de
160×101:

- **D · céu, cores do portal** — amarelo `#f4c842`, vermelho `#df4747`,
  creme, linhas cinza-esverdeadas (como na animação).
- **E · céu, cores do Asterismus** — âmbar `#d18a45` no lugar do amarelo,
  linhas no verde da interface.

`folha-ceu-da-animacao.png`: grande, pequeno sobre o verde e sobre fundo
claro (onde as estrelas creme somem: para fundo claro faria falta uma
variante com estrelas escuras), e ao lado da palavra ASTERISMUS.

## 3ª rodada (6/11): D2 e E2, o céu como carta celeste

Pedido do autor sobre D/E: "traços mais irregulares, melhore as cores";
"não parecem estrelas, traga mais a ideia de constelação"; "a partir da céu
animação". D2 (cores do portal) e E2 (cores do Asterismus) mantêm a figura da
animação (`website/assets/ceu.js`: oito estrelas, nove arestas, dois pares em
foco, a que lidera em vermelho) e mudam o desenho:

- estrelas de quatro pontas com halo e núcleo claro, em magnitudes
  diferentes; a que lidera com uma segunda cintilação em diagonal;
- arestas à mão: tremidas, com arco leve e pressão variável, parando antes
  das estrelas como nas cartas celestes; os pares em foco mais grossos;
- poeira de estrelas de fundo e o rastro curto de cada estrela (a fórmula de
  passeio da animação);
- cores revistas: ouro/âmbar mais quentes, vermelho mais vivo, halos frios
  nas estrelas creme, linhas mais claras.

Gerados por `gerar_ceu.py` (determinístico, só biblioteca padrão; mudar as
paletas ou a magnitude lá e rodar de novo). `folha-ceu-carta-celeste.png`:
os dois grandes e em 64 px sobre o fundo do site. D e E originais ficam como
estavam. Pendente: a escolha do autor; em 32/16 px (favicon) a figura
precisará de uma versão reduzida (menos poeira, sem rastros).
