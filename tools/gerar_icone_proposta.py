"""Proposta de ícone do Rasgo Synth Asterismus (30/10/2026), para aprovação.

Um asterismo: cinco estrelas ligadas (a trama), a estrela em foco em âmbar,
e o arco da trajetória de uma delas (o movimento físico). Cores da interface
(verde #102d27, âmbar #e1aa59, creme #f2e7d5). Sai em
packaging/icon/proposta/; o build só usa packaging/icon/asterismus-*.png
quando o autor aprovar e copiar.
"""
import math, pathlib, subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUT = ROOT / "packaging" / "icon" / "proposta"
GREEN, AMBER, CREAM, LINE = "#102d27", "#e1aa59", "#f2e7d5", "#6f9b8d"

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    stars = [(300, 330), (520, 250), (700, 420), (610, 690), (380, 640)]
    focus = 1
    links = [(0, 1), (1, 2), (2, 3), (3, 4), (1, 4)]
    body = f'<rect x="40" y="40" width="944" height="944" rx="190" fill="{GREEN}"/>'
    # o arco de trajetória (uma elipse inclinada em torno da estrela em foco)
    fx, fy = stars[focus]
    body += (f'<ellipse cx="{fx}" cy="{fy + 40}" rx="250" ry="120" transform="rotate(-24 {fx} {fy + 40})" '
             f'fill="none" stroke="{AMBER}" stroke-width="10" stroke-opacity="0.55" stroke-dasharray="2 26" stroke-linecap="round"/>')
    for a, b in links:
        (x0, y0), (x1, y1) = stars[a], stars[b]
        body += f'<line x1="{x0}" y1="{y0}" x2="{x1}" y2="{y1}" stroke="{LINE}" stroke-width="16" stroke-linecap="round"/>'
    for index, (x, y) in enumerate(stars):
        r = 62 if index == focus else 40
        body += f'<circle cx="{x}" cy="{y}" r="{r}" fill="{AMBER if index == focus else CREAM}"/>'
    svg = f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024" width="1024" height="1024">{body}</svg>\n'
    (OUT / "asterismus-proposta.svg").write_text(svg)
    for size in (16, 32, 48, 64, 128, 256, 512, 1024):
        subprocess.run(["convert", "-background", "none", "-density", "300", str(OUT / "asterismus-proposta.svg"),
                        "-resize", f"{size}x{size}", str(OUT / f"asterismus-{size}.png")], check=True)
    subprocess.run(["convert"] + [str(OUT / f"asterismus-{s}.png") for s in (16, 32, 48, 64, 128, 256)]
                   + [str(OUT / "asterismus.ico")], check=True)

if __name__ == "__main__":
    main()
