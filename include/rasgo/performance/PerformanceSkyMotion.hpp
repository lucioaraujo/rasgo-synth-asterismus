#pragma once

// Movimento físico das estrelas na Constelação (29/10). Pedido do autor:
// "faltam movimentos de conceitos de física nas constelações: elásticos,
// billiard mathematics, gravidades, atração, repulsão, magnetismos; e também
// movimentos de dança", "pêndulos", "os movimentos estão ainda duros demais
// ou ainda muito previsíveis", "movimentos mais interessantes, inusitados,
// generativos".
//
// Até aqui a Constelação desenhava órbitas paramétricas (senos de uma fase
// comum): rígidas e previsíveis. Agora cada estrela é um corpo com massa,
// inércia e forças, integrado pelo diretor do céu; as primitivas ficam aqui,
// testáveis uma a uma:
//
// - choque elástico entre dois corpos (bilhar): conserva momento e energia;
// - parede de estádio (o bilhar de Bunimovich, caótico): reflexão especular;
// - força de Lorentz (magnetismo): a ∝ q·B·(vy, −vx), curva o caminho;
// - campo de correntes de um atrator (família de Clifford): a pista de dança
//   generativa, que arrasta os corpos em redemoinhos;
// - pêndulo elástico (mola com gravidade, ressonância 2:1): troca sem aviso
//   entre balançar e quicar;
// - spin e efeito Magnus (tênis de mesa): o giro curva a trajetória; nos
//   choques o atrito tangencial troca giro e velocidade;
// - espuma: cada corpo é uma bolha que respira; bolhas que se tocam se
//   empurram pela sobreposição (pressão), e o conjunto se arruma;
// - epiciclos de Fibonacci: o lugar de repouso passeia numa soma de círculos
//   com velocidades Fₙ·ω₀ e raios ∝ 1/Fₙ — sem período comum, não repete;
// - distorção: uma matriz móvel u = M(t)·v (rotação, cisalhamento, escala)
//   deforma o espaço inteiro em torno do centro.
//
// Pedido seguinte do autor: "efeitos de spins (tenis de table), curvas,
// balanços, molas, espumas, distorções"; "veja alguns conceitos no batruc".
// Do Batruc (MOVIMENTO_GRAFICO_E_SOM.md, só lido): os epiciclos de Fourier
// com receitas de Fibonacci e a matriz móvel u = M(t)v como conceitos.
//
// Fontes: L. A. Bunimovich, "On the ergodic properties of nowhere dispersing
// billiards", Commun. Math. Phys. 65, 1979; P. Lynch, "The swinging spring: a
// simple model of atmospheric balance", 2002; C. A. Pickover, *Chaos in
// Wonderland*, 1994 (os atratores de Clifford); J. D. Jackson, *Classical
// Electrodynamics* (Lorentz); H. M. Brody et al., *The Physics and
// Technology of Tennis*, 2002, e R. Cross, "The bounce of a ball", Am. J.
// Phys. 67, 1999 (Magnus, atrito no choque); D. Weaire e S. Hutzler, *The
// Physics of Foams*, 1999 (o modelo de bolhas macias de Durian, Phys. Rev.
// Lett. 75, 1995). Nenhum código copiado.

#include <algorithm>
#include <cmath>

namespace rasgo::performance {

struct SkyBodyState {
    double x = 0.5;
    double y = 0.5;
    double vx = 0.0;
    double vy = 0.0;
};

namespace sky_motion {

// Choque elástico entre dois discos de massas ma e mb que se tocam
// (distância < contato) e se aproximam: troca a componente normal das
// velocidades como manda a conservação de momento e energia, e os separa.
inline bool collide(SkyBodyState& a, SkyBodyState& b, double ma, double mb, double contact) noexcept {
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    const double distance = std::sqrt(dx * dx + dy * dy);
    if (distance >= contact || distance < 1.0e-9 || ma <= 0.0 || mb <= 0.0) return false;
    const double nx = dx / distance;
    const double ny = dy / distance;
    const double approach = (a.vx - b.vx) * nx + (a.vy - b.vy) * ny;
    // Afasta os dois até o contato, na razão inversa das massas.
    const double overlap = contact - distance;
    a.x -= nx * overlap * mb / (ma + mb);
    a.y -= ny * overlap * mb / (ma + mb);
    b.x += nx * overlap * ma / (ma + mb);
    b.y += ny * overlap * ma / (ma + mb);
    if (approach <= 0.0) return false;
    const double impulse = 2.0 * approach / (ma + mb);
    a.vx -= impulse * mb * nx;
    a.vy -= impulse * mb * ny;
    b.vx += impulse * ma * nx;
    b.vy += impulse * ma * ny;
    return true;
}

// O estádio: os pontos a no máximo `radius` do segmento horizontal de
// (cx − half, cy) a (cx + half, cy). Fora dele, o corpo volta à borda e a
// velocidade é refletida na normal (reflexão especular, sem perda).
inline bool reflectStadium(SkyBodyState& body, double cx, double cy, double half, double radius) noexcept {
    const double closestX = std::clamp(body.x, cx - half, cx + half);
    const double dx = body.x - closestX;
    const double dy = body.y - cy;
    const double distance = std::sqrt(dx * dx + dy * dy);
    if (distance <= radius || distance < 1.0e-9) return false;
    const double nx = dx / distance;
    const double ny = dy / distance;
    body.x = closestX + nx * radius;
    body.y = cy + ny * radius;
    const double outward = body.vx * nx + body.vy * ny;
    if (outward > 0.0) {
        body.vx -= 2.0 * outward * nx;
        body.vy -= 2.0 * outward * ny;
    }
    return true;
}

// As bordas do campo [0, 1]² como mesa de bilhar retangular.
inline void reflectBox(SkyBodyState& body) noexcept {
    if (body.x < 0.0) { body.x = -body.x; body.vx = std::fabs(body.vx); }
    if (body.x > 1.0) { body.x = 2.0 - body.x; body.vx = -std::fabs(body.vx); }
    if (body.y < 0.0) { body.y = -body.y; body.vy = std::fabs(body.vy); }
    if (body.y > 1.0) { body.y = 2.0 - body.y; body.vy = -std::fabs(body.vy); }
    body.x = std::clamp(body.x, 0.0, 1.0);
    body.y = std::clamp(body.y, 0.0, 1.0);
}

// Força de Lorentz por unidade de massa num campo magnético perpendicular
// ao plano: a = (q/m)·B·(vy, −vx). Gira a velocidade sem mudar a rapidez;
// sozinha, faz um círculo de raio |v|·m/(|q|·|B|).
inline void lorentz(const SkyBodyState& body, double chargeOverMass, double field,
                    double& ax, double& ay) noexcept {
    ax += chargeOverMass * field * body.vy;
    ay -= chargeOverMass * field * body.vx;
}

// A pista de dança: o campo de correntes de um atrator da família de
// Clifford, (sen(a·Y) + c·cos(a·X), sen(b·X) + d·cos(b·Y)), com X e Y o
// lugar no campo levado a [−π, π] e uma fase que anda devagar — o desenho
// dos redemoinhos se transforma sem repetir.
struct FlowShape {
    double a = 1.4;
    double b = -2.3;
    double c = 2.4;
    double d = -2.1;
};
inline void flow(const FlowShape& shape, double x, double y, double phase, double& u, double& v) noexcept {
    constexpr double pi = 3.141592653589793;
    const double X = (x - 0.5) * 2.0 * pi;
    const double Y = (y - 0.5) * 2.0 * pi;
    u = std::sin(shape.a * Y + phase) + shape.c * std::cos(shape.a * X - phase * 0.7);
    v = std::sin(shape.b * X - phase * 0.5) + shape.d * std::cos(shape.b * Y + phase * 0.3);
    const double norm = 1.0 + std::fabs(shape.c) + std::fabs(shape.d);
    u /= norm;
    v /= norm;
}

// Pêndulo elástico: mola de comprimento de repouso `rest` e rigidez
// `stiffness` presa ao ponto (px, py), mais a gravidade `gravity` para baixo
// (y cresce para baixo na tela). Devolve a aceleração da mola + gravidade.
inline void swingingSpring(const SkyBodyState& body, double px, double py, double rest, double stiffness,
                           double gravity, double& ax, double& ay) noexcept {
    const double dx = body.x - px;
    const double dy = body.y - py;
    const double length = std::max(std::sqrt(dx * dx + dy * dy), 1.0e-6);
    const double stretch = length - rest;
    ax -= stiffness * stretch * dx / length;
    ay -= stiffness * stretch * dy / length;
    ay += gravity;
}

// Efeito Magnus: um corpo que gira (spin s, positivo anti-horário) sente
// uma força perpendicular à velocidade, a = k·s·(−vy, vx): a bola de tênis
// de mesa com efeito faz curva.
inline void magnus(const SkyBodyState& body, double spin, double coefficient, double& ax, double& ay) noexcept {
    ax -= coefficient * spin * body.vy;
    ay += coefficient * spin * body.vx;
}

// O atrito tangencial de um choque: a velocidade relativa tangencial de
// contato (incluindo o giro, raio `radius`) vira giro nos dois corpos, com
// sinais opostos; uma parte `grip` da diferença é trocada.
inline void spinExchange(const SkyBodyState& a, const SkyBodyState& b, double& spinA, double& spinB,
                         double radius, double grip) noexcept {
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    const double distance = std::sqrt(dx * dx + dy * dy);
    if (distance < 1.0e-9) return;
    const double tx = -dy / distance;
    const double ty = dx / distance;
    const double slide = (a.vx - b.vx) * tx + (a.vy - b.vy) * ty + radius * (spinA + spinB);
    spinA -= grip * slide / radius * 0.5;
    spinB -= grip * slide / radius * 0.5;
}

// Espuma (bolhas macias de Durian): dois corpos de raios ra e rb que se
// sobrepõem são empurrados na proporção da sobreposição.
inline void bubblePressure(const SkyBodyState& a, const SkyBodyState& b, double ra, double rb,
                           double stiffness, double& ax, double& ay) noexcept {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    const double distance = std::sqrt(dx * dx + dy * dy);
    const double overlap = ra + rb - distance;
    if (overlap <= 0.0 || distance < 1.0e-9) return;
    ax += stiffness * overlap * dx / distance;
    ay += stiffness * overlap * dy / distance;
}

// Epiciclos de Fibonacci: z(t) = Σ rₙ·e^{i(Fₙ·ω₀·t + φₙ)}, rₙ = r / Fₙ,
// n = 1…4 (F = 1, 2, 3, 5); com ω₀ irracional em relação ao resto do céu.
inline void fibonacciEpicycle(double t, double omega, double radius, double phase, double& x, double& y) noexcept {
    constexpr double fib[4] = {1.0, 2.0, 3.0, 5.0};
    x = 0.0;
    y = 0.0;
    for (int n = 0; n < 4; ++n) {
        const double angle = fib[n] * omega * t + phase * (n + 1);
        x += radius / fib[n] * std::cos(angle);
        y += radius / fib[n] * std::sin(angle);
    }
}

// A matriz móvel M(t) = escala · rotação · cisalhamento, aplicada em torno
// do centro do campo: u = c + M(v − c). `amount` 0 é a identidade.
inline void distort(double t, double amount, double phase, double x, double y, double& ux, double& uy) noexcept {
    const double angle = amount * 0.22 * std::sin(t * 0.093 + phase);
    const double shear = amount * 0.16 * std::sin(t * 0.061 * 1.618033988749895 + phase * 2.0);
    const double scale = 1.0 + amount * 0.07 * std::sin(t * 0.047 + phase * 3.0);
    const double vx = x - 0.5;
    const double vy = y - 0.5;
    const double sx = vx + shear * vy;
    const double sy = vy;
    ux = 0.5 + scale * (std::cos(angle) * sx - std::sin(angle) * sy);
    uy = 0.5 + scale * (std::sin(angle) * sx + std::cos(angle) * sy);
}

// A borda macia da tela: até 0,4 do centro, identidade; além, o desvio é
// comprimido por tanh e nunca passa de 0,48 — a estrela se aproxima da borda
// em curva, sem grudar nela (o corte seco em [0, 1] a deixava parada na
// borda quando a distorção a empurrava para fora).
inline double softEdge(double value) noexcept {
    const double offset = value - 0.5;
    const double magnitude = std::fabs(offset);
    if (magnitude <= 0.4) return value;
    const double squeezed = 0.4 + 0.08 * std::tanh((magnitude - 0.4) / 0.08);
    return 0.5 + (offset < 0.0 ? -squeezed : squeezed);
}

} // namespace sky_motion
} // namespace rasgo::performance
