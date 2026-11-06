// O céu do Asterismus na apresentação do site (6/11/2026, pedido do autor:
// "inserir com as cores corretas a animação (só do asterismus) ao lado da
// primeira parte"). A mesma cena da família no portal RASGO
// (RASGO_WEBSITE/assets/familia.js, RASGO SYNTH ASTERISMUS), sozinha e nas
// cores da interface: oito estrelas nos seus lugares, cada uma no seu
// passeio com um rastro curto, as arestas da constelação, dois pares em foco
// acesos em âmbar a cada 1,3 s e a estrela que lidera em vermelho. Canvas, sem
// biblioteca e sem shadowBlur; para fora da tela ou com a aba oculta; com
// "reduzir movimento", um quadro parado.
(() => {
  const canvas = document.querySelector('[data-ceu]');
  if (!canvas || !canvas.getContext) return;
  const g = canvas.getContext('2d');
  const css = getComputedStyle(document.documentElement);
  const cor = (nome, reserva) => css.getPropertyValue(nome).trim() || reserva;
  const AMBAR = cor('--star', '#d18a45');
  const CREME = cor('--ink', '#ece7d8');
  const LINHA = '#3d6a5d';
  const RASTRO = cor('--muted', '#b3c2ba');
  const VERMELHO = '#df4747';
  const quieto = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
  let W = 0, H = 0;
  let s = (Math.random() * 4294967296) >>> 0;
  const rnd = () => { s = (s + 0x9e3779b9) >>> 0; let z = s; z = Math.imul(z ^ (z >>> 16), 0x85ebca6b) >>> 0; z = Math.imul(z ^ (z >>> 13), 0xc2b2ae35) >>> 0; return ((z ^ (z >>> 16)) >>> 0) / 4294967296; };
  const ceu = [[0.10, 0.28], [0.32, 0.12], [0.52, 0.32], [0.72, 0.16], [0.90, 0.44], [0.70, 0.76], [0.42, 0.84], [0.22, 0.60]]
    .map(([x, y], i) => ({ x, y, a: rnd() * 6.28, v: 0.35 + 0.5 * rnd(), r: 0.022 + 0.024 * rnd(), i }));
  const arestas = [[0, 1], [1, 2], [2, 3], [3, 4], [4, 5], [5, 6], [6, 7], [7, 0], [2, 6]];
  const linha = (larg, c) => { g.lineWidth = larg; g.strokeStyle = c; g.lineCap = 'round'; };
  const pos = (e, t) => [(e.x + Math.cos(e.a + t * e.v) * e.r) * W, (e.y + Math.sin(e.a * 1.7 + t * e.v * 1.3) * e.r * 1.4) * H];

  const quadro = (t) => {
    g.clearRect(0, 0, W, H);
    const m = Math.min(W, H);
    linha(Math.max(1, m * 0.005), LINHA);
    for (const [a, b] of arestas) { const A = pos(ceu[a], t), B = pos(ceu[b], t); g.beginPath(); g.moveTo(A[0], A[1]); g.lineTo(B[0], B[1]); g.stroke(); }
    const compasso = Math.floor(t / 1.3);
    for (let k = 0; k < 2; k++) {
      const [a, b] = arestas[(compasso * 3 + k * 4) % arestas.length];
      const A = pos(ceu[a], t), B = pos(ceu[b], t);
      linha(Math.max(2, m * 0.012), AMBAR); g.beginPath(); g.moveTo(A[0], A[1]); g.lineTo(B[0], B[1]); g.stroke();
    }
    const lider = (compasso * 5) % ceu.length;
    for (const e of ceu) {
      g.save(); g.globalAlpha = 0.35; linha(Math.max(1, m * 0.004), RASTRO); g.beginPath();
      for (let j = 0; j <= 8; j++) { const [x, y] = pos(e, t - 0.5 * j / 8); j === 0 ? g.moveTo(x, y) : g.lineTo(x, y); }
      g.stroke(); g.restore();
      const [x, y] = pos(e, t);
      g.fillStyle = e.i === lider ? VERMELHO : (e.i % 3 === 0 ? AMBAR : CREME);
      g.beginPath(); g.arc(x, y, m * (e.i === lider ? 0.034 : 0.021), 0, 6.28); g.fill();
    }
  };

  const ajusta = () => {
    const r = Math.min(2, window.devicePixelRatio || 1), caixa = canvas.getBoundingClientRect();
    W = Math.max(160, caixa.width); H = Math.max(120, caixa.height);
    canvas.width = Math.round(W * r); canvas.height = Math.round(H * r);
    g.setTransform(r, 0, 0, r, 0, 0);
  };
  ajusta();
  const fixo = new URLSearchParams(location.search).get('quadro');
  if (fixo !== null) { quadro(parseFloat(fixo) || 0); return; }
  window.addEventListener('resize', () => { ajusta(); if (quieto) quadro(2.0); });
  if (quieto) { quadro(2.0); return; }
  let visivel = true; const inicio = performance.now();
  const passo = (agora) => { if (visivel && !document.hidden) { quadro((agora - inicio) / 1000); requestAnimationFrame(passo); } };
  if ('IntersectionObserver' in window) {
    new IntersectionObserver((entradas) => {
      const antes = visivel; visivel = entradas[0].isIntersecting;
      if (visivel && !antes) requestAnimationFrame(passo);
    }).observe(canvas);
  }
  document.addEventListener('visibilitychange', () => { if (!document.hidden && visivel) requestAnimationFrame(passo); });
  requestAnimationFrame(passo);
})();
