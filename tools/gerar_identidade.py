#!/usr/bin/env python3
"""A identidade do Asterismus a partir do logo escolhido pelo autor (7/11/2026):
packaging/logo/proposta/E-ceu-asterismus.svg — o céu da animação do portal nas
cores da interface. Gera:

- packaging/logo/asterismus-logo.svg      o logo (cópia fiel da proposta E)
- packaging/icon/asterismus.svg           o ícone: o céu na moldura quadrada
                                          verde-escura (a da proposta de ícone
                                          de outubro), para barras e docks
- packaging/icon/asterismus-<n>.png       16 … 1024 px (≤ 48 px: traço e
                                          estrelas mais grossos, para não sumirem)
- packaging/icon/asterismus.ico           Windows (16, 32, 48, 64, 128, 256)
- website/assets/identity/asterismus-symbol.svg, asterismus-favicon.svg,
  favicon-32.png, apple-touch-icon.png
- assets/asterismus-icon.png              o ícone da janela do app (256 px)

Uso, da raiz do projeto: python3 tools/gerar_identidade.py
Precisa de inkscape (SVG → PNG) e do convert do ImageMagick (.ico).
"""
import pathlib
import re
import shutil
import subprocess
import tempfile

RAIZ = pathlib.Path(__file__).resolve().parent.parent
FONTE = RAIZ / "packaging/logo/proposta/E-ceu-asterismus.svg"
LOGO = RAIZ / "packaging/logo/asterismus-logo.svg"
ICONES = RAIZ / "packaging/icon"
SITE = RAIZ / "website/assets/identity"
TAMANHOS = [16, 32, 48, 64, 128, 256, 512, 1024]


def elementos(grosso):
    """Os elementos do céu E; grosso = traços e estrelas reforçados."""
    corpo = FONTE.read_text(encoding="utf-8")
    corpo = corpo[corpo.index(">", corpo.index("<svg")) + 1:corpo.rindex("</svg>")]
    corpo = re.sub(r"<!--.*?-->", "", corpo, flags=re.S)
    if grosso:
        corpo = re.sub(r'stroke-width="([\d.]+)"', lambda m: 'stroke-width="%g"' % (float(m.group(1)) * 2.4), corpo)
        corpo = re.sub(r'r="([\d.]+)"', lambda m: 'r="%g"' % (float(m.group(1)) * 1.7), corpo)
    return corpo.strip()


def icone(grosso):
    # Moldura: a da proposta de ícone (rect 944 px, cantos 190, verde #102d27).
    # O céu (160 × 140) ocupa ~76 % da largura, centrado.
    escala = 4.75 if not grosso else 5.0
    largura, altura = 160 * escala, 140 * escala
    x, y = (1024 - largura) / 2, (1024 - altura) / 2
    return ('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024" width="1024" height="1024">'
            '<rect x="40" y="40" width="944" height="944" rx="190" fill="#102d27"/>'
            '<g transform="translate(%g %g) scale(%g)">%s</g></svg>\n' % (x, y, escala, elementos(grosso)))


def png(svg_texto, destino, lado, pasta):
    origem = pathlib.Path(pasta) / ("i%d.svg" % lado)
    origem.write_text(svg_texto, encoding="utf-8")
    subprocess.run(["inkscape", str(origem), "--export-type=png", "--export-filename=%s" % destino,
                    "-w", str(lado), "-h", str(lado)], check=True, capture_output=True)


def main():
    LOGO.write_text(FONTE.read_text(encoding="utf-8"), encoding="utf-8")
    ICONES.mkdir(parents=True, exist_ok=True)
    (ICONES / "asterismus.svg").write_text(icone(False), encoding="utf-8")
    with tempfile.TemporaryDirectory() as pasta:
        for lado in TAMANHOS:
            png(icone(lado <= 48), ICONES / ("asterismus-%d.png" % lado), lado, pasta)
        subprocess.run(["convert"] + [str(ICONES / ("asterismus-%d.png" % n)) for n in (16, 32, 48, 64, 128, 256)]
                       + [str(ICONES / "asterismus.ico")], check=True)
        SITE.mkdir(parents=True, exist_ok=True)
        shutil.copy(LOGO, SITE / "asterismus-symbol.svg")
        (SITE / "asterismus-favicon.svg").write_text(icone(True), encoding="utf-8")
        png(icone(True), SITE / "favicon-32.png", 32, pasta)
        png(icone(False), SITE / "apple-touch-icon.png", 180, pasta)
    # O ícone da janela (Linux: barra de tarefas), lido de assets/ pelo app.
    shutil.copy(ICONES / "asterismus-256.png", RAIZ / "assets/asterismus-icon.png")
    print("gerar_identidade.py: logo, %d ícones, .ico e os do site" % len(TAMANHOS))


if __name__ == "__main__":
    main()
