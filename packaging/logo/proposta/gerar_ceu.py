#!/usr/bin/env python3
"""Logo do Asterismus, 3ª rodada (6/11/2026): o céu da animação.

Pedido do autor sobre D e E: "traços mais irregulares, melhore as cores";
"não parecem estrelas, traga mais a ideia de constelação"; "a partir da céu
animação". A figura é a de RASGO_WEBSITE/assets/familia.js e
website/assets/ceu.js (as oito estrelas nos seus lugares, as nove arestas,
dois pares em foco, a estrela que lidera em vermelho), agora como carta do
céu: estrelas de quatro pontas com halo, magnitudes diferentes, poeira de
estrelas de fundo, arestas desenhadas à mão (tremidas, de largura variável,
parando antes das estrelas como nas cartas celestes) e o rastro curto de cada
estrela, tirado da mesma fórmula de passeio da animação.

Determinístico (acaso próprio); só biblioteca padrão. Uso:
    python3 gerar_ceu.py   -> D2-ceu-portal.svg, E2-ceu-asterismus.svg
"""
import math
from pathlib import Path

AQUI = Path(__file__).resolve().parent
W, H = 160.0, 140.0

# as posições da animação do site (ceu.js), num quadro 160x140 com margem
CEU = [(0.10, 0.28), (0.32, 0.12), (0.52, 0.32), (0.72, 0.16),
       (0.90, 0.44), (0.70, 0.76), (0.42, 0.84), (0.22, 0.60)]
ARESTAS = [(0, 1), (1, 2), (2, 3), (3, 4), (4, 5), (5, 6), (6, 7), (7, 0), (2, 6)]
FOCO = {(3, 4), (7, 0)}       # os dois pares acesos
LIDER = 5                     # a estrela que lidera
ACESAS = {0, 3, 6}            # e.i % 3 == 0 na animação
# magnitude (raio da estrela) — nem todas iguais, como num céu
MAG = [4.6, 3.4, 4.0, 4.8, 3.6, 6.4, 4.2, 3.2]

PALETAS = {
    "D2-ceu-portal": {
        "nota": "cores do portal RASGO: ouro, vermelho, creme",
        "ouro": "#f6cd4c", "ouro_claro": "#fff1b8",
        "verm": "#e8513f", "verm_claro": "#ffd2c4",
        "creme": "#f4efe0", "creme_halo": "#cfe0ff",
        "linha": "#8a9a8f", "linha_foco": "#f6cd4c", "poeira": "#c9cfd6",
    },
    "E2-ceu-asterismus": {
        "nota": "cores da interface do Asterismus: âmbar, vermelho, creme, verde",
        "ouro": "#e39a50", "ouro_claro": "#ffe0b8",
        "verm": "#e2574a", "verm_claro": "#ffd0c4",
        "creme": "#efe9d8", "creme_halo": "#bfe3d6",
        "linha": "#6fa595", "linha_foco": "#e39a50", "poeira": "#a9c9be",
    },
}


class Acaso:
    def __init__(self, s):
        self.s = s & 0xFFFFFFFF or 0x9E3779B9

    def unit(self):
        s = self.s
        s ^= (s << 13) & 0xFFFFFFFF
        s ^= s >> 17
        s ^= (s << 5) & 0xFFFFFFFF
        self.s = s
        return (s >> 8) / 16777216.0

    def entre(self, a, b):
        return a + (b - a) * self.unit()


def pontos_estrela():
    margem_x, margem_y = 14.0, 14.0
    return [(margem_x + x * (W - 2 * margem_x), margem_y + y * (H - 2 * margem_y)) for x, y in CEU]


def estrela4(cx, cy, r, giro, cintura=0.30):
    """Estrela de quatro pontas côncava (a cintilação), com pontas desiguais."""
    pts = []
    for k in range(8):
        ang = giro + k * math.pi / 4
        rr = r * (1.0 if k % 4 == 0 else 0.78 if k % 2 == 0 else cintura)
        pts.append((cx + math.cos(ang) * rr, cy + math.sin(ang) * rr))
    d = f"M{pts[0][0]:.2f},{pts[0][1]:.2f}"
    for i in range(1, 9):
        a, b = pts[i - 1], pts[i % 8]
        # curva para dentro: controle puxado ao centro
        mx, my = (a[0] + b[0]) / 2, (a[1] + b[1]) / 2
        qx, qy = cx + (mx - cx) * 0.55, cy + (my - cy) * 0.55
        d += f" Q{qx:.2f},{qy:.2f} {b[0]:.2f},{b[1]:.2f}"
    return d + "Z"


def traco_mao(a, b, acaso, folga_a, folga_b, largura):
    """Uma aresta à mão: começa e termina antes das estrelas, treme no meio,
    e a largura varia ao longo (um polígono, não um stroke uniforme)."""
    ax, ay = a
    bx, by = b
    dx, dy = bx - ax, by - ay
    comp = math.hypot(dx, dy)
    ux, uy = dx / comp, dy / comp
    nx, ny = -uy, ux
    t0, t1 = folga_a / comp, 1.0 - folga_b / comp
    n = 24
    curva = acaso.entre(-1.6, 1.6)          # um arco leve, de mão
    fase = acaso.entre(0, math.tau)
    fase2 = acaso.entre(0, math.tau)
    pressao = acaso.entre(0, math.tau)
    esq, dir_ = [], []
    for i in range(n + 1):
        u = t0 + (t1 - t0) * i / n
        tremor = math.sin(fase + u * 9.0) * 0.42 + math.sin(fase2 + u * 23.0) * 0.18
        desvio = curva * math.sin(math.pi * u) + tremor
        x, y = ax + dx * u + nx * desvio, ay + dy * u + ny * desvio
        # largura: afina nas pontas, engrossa e afina no meio (pressão)
        p = math.sin(math.pi * (i / n)) ** 0.6
        w = largura * (0.35 + 0.65 * p) * (1.0 + 0.22 * math.sin(pressao + u * 7.0))
        esq.append((x + nx * w / 2, y + ny * w / 2))
        dir_.append((x - nx * w / 2, y - ny * w / 2))
    poly = esq + dir_[::-1]
    return "M" + " L".join(f"{x:.2f},{y:.2f}" for x, y in poly) + "Z"


def rastro(cx, cy, acaso, comprimento):
    """O rastro da animação: onde a estrela esteve no último meio segundo
    (a mesma fórmula de passeio de ceu.js, em coordenadas do quadro)."""
    a = acaso.entre(0, math.tau)
    v = acaso.entre(0.35, 0.85)
    r = comprimento
    pts = []
    t = 4.0
    x0 = math.cos(a + t * v) * r
    y0 = math.sin(a * 1.7 + t * v * 1.3) * r * 1.4
    for j in range(13):
        tt = t - 1.4 * j / 12
        x = math.cos(a + tt * v) * r - x0
        y = math.sin(a * 1.7 + tt * v * 1.3) * r * 1.4 - y0
        pts.append((cx + x, cy + y))
    return "M" + " L".join(f"{x:.2f},{y:.2f}" for x, y in pts)


def gerar(nome, cor):
    acaso = Acaso(0xA57E2153)
    est = pontos_estrela()
    out = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W:.0f} {H:.0f}" width="{W:.0f}" height="{H:.0f}">',
           f'  <!-- {nome} · o céu da animação como carta celeste ({cor["nota"]}); gerado por gerar_ceu.py -->',
           "  <defs>"]
    for chave, centro in (("ouro", "ouro_claro"), ("verm", "verm_claro"), ("creme", "creme_halo")):
        out.append(f'    <radialGradient id="halo-{chave}"><stop offset="0" stop-color="{cor[chave]}" stop-opacity="0.55"/>'
                   f'<stop offset="0.45" stop-color="{cor[centro]}" stop-opacity="0.16"/>'
                   f'<stop offset="1" stop-color="{cor[centro]}" stop-opacity="0"/></radialGradient>')
    out.append("  </defs>")

    # poeira: estrelas de fundo, pequenas e de brilho desigual, fora das arestas
    out.append(f'  <g fill="{cor["poeira"]}">')
    for _ in range(26):
        x, y = acaso.entre(4, W - 4), acaso.entre(4, H - 4)
        if min(math.hypot(x - sx, y - sy) for sx, sy in est) < 9:
            continue
        r = acaso.entre(0.35, 1.05)
        o = acaso.entre(0.18, 0.6)
        out.append(f'    <circle cx="{x:.2f}" cy="{y:.2f}" r="{r:.2f}" opacity="{o:.2f}"/>')
    out.append("  </g>")

    # rastros (a física de cada estrela), muito leves
    out.append(f'  <g fill="none" stroke="{cor["poeira"]}" stroke-linecap="round" stroke-width="0.8" opacity="0.26">')
    for (x, y), m in zip(est, MAG):
        out.append(f'    <path d="{rastro(x, y, acaso, m * 1.1)}"/>')
    out.append("  </g>")

    # arestas à mão, paradas antes das estrelas
    for a, b in ARESTAS:
        foco = (a, b) in FOCO
        d = traco_mao(est[a], est[b], acaso, MAG[a] * 1.15 + 1.6, MAG[b] * 1.15 + 1.6, 3.6 if foco else 1.7)
        c = cor["linha_foco"] if foco else cor["linha"]
        op = 0.95 if foco else 0.72
        out.append(f'  <path d="{d}" fill="{c}" opacity="{op}"/>')

    # as estrelas: halo, cintilação de quatro pontas, núcleo claro
    for i, ((x, y), m) in enumerate(zip(est, MAG)):
        if i == LIDER:
            chave, nucleo = "verm", cor["verm_claro"]
        elif i in ACESAS:
            chave, nucleo = "ouro", cor["ouro_claro"]
        else:
            chave, nucleo = "creme", "#ffffff"
        giro = acaso.entre(-0.22, 0.22)
        out.append(f'  <circle cx="{x:.2f}" cy="{y:.2f}" r="{m * 2.6:.2f}" fill="url(#halo-{chave})"/>')
        if i == LIDER:  # a que lidera: uma segunda cintilação, em diagonal
            out.append(f'  <path d="{estrela4(x, y, m * 1.15, giro + math.pi / 4, 0.22)}" fill="{cor[chave]}" opacity="0.55"/>')
        out.append(f'  <path d="{estrela4(x, y, m * 1.7, giro)}" fill="{cor[chave]}"/>')
        out.append(f'  <circle cx="{x:.2f}" cy="{y:.2f}" r="{m * 0.32:.2f}" fill="{nucleo}"/>')
    out.append("</svg>")
    (AQUI / f"{nome}.svg").write_text("\n".join(out) + "\n", encoding="utf-8")


if __name__ == "__main__":
    for nome, cor in PALETAS.items():
        gerar(nome, cor)
