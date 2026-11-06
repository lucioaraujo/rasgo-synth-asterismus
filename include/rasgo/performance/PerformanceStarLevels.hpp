#pragma once

// Nível de referência de cada estrela (2026-11-05).
//
// Pedido do autor, na escuta guiada: "verifique as saídas, se não está muito
// baixo o volume" — e, com a proposta, "sim, faça as duas" (igualar as
// estrelas; depois recuperar o nível de saída).
//
// Medido offline (sonda `niveis`, como o app: 8 corpos, blocos de 512,
// trama e VARIAÇÃO ligadas, MASTER −6 dB), 48 seeds (2000–2047) × 90 s: o
// nível de cada estrela *quando soa* (média de potência dos blocos acima de
// −70 dBFS no stem). Ia de −42,6 dB (THEREMIN) a −60,0 dB (MODAL): 17 dB
// entre a mais alta e a mais baixa. A correção leva cada uma à mediana
// (−49,7 dB), limitada a ±9 dB, e é fixa — não é nivelador: a dinâmica de
// cada estrela, da forma e da VARIAÇÃO continua inteira. Entra no fim da
// cadeia do stem (depois de tanh e profundidade), então não muda timbre nem
// saturação; os stems gravados saem já corrigidos.
//
// 2ª medida (5/11, depois do conserto da seed — auditoria da seed: o timbre
// de cada estrela passou a variar de verdade com a seed): 48 seeds (5000–5047)
// × 90 s com a tabela anterior; cada correção somou a distância à nova
// mediana. Comentário de cada linha: 1ª medida · 2ª medida com a correção antiga.
//
// As estrelas de controle não soam: correção zero. Numa estrela nova,
// medir com a sonda e acrescentar aqui (sem medida, 0 dB).

#include "PerformanceModuleTypes.hpp"

namespace rasgo::performance {

inline constexpr double performanceStarLevelTrimDb(PerformanceModuleBody star) noexcept {
    switch (star) {
    case PerformanceModuleBody::Theremin: return -6.3; // -42.6 dB (5/11 2ª medida: -49.9 com -7.1)
    case PerformanceModuleBody::Galo: return -5.6; // -45.8 dB (5/11 2ª medida: -47.3 com -3.9)
    case PerformanceModuleBody::BassFallback: return -5.1; // -45.9 dB (5/11 2ª medida: -47.8 com -3.8)
    case PerformanceModuleBody::Walsh: return -3.9; // -45.9 dB (5/11 2ª medida: -48.9 com -3.8)
    case PerformanceModuleBody::Vosim: return -3.6; // -46.6 dB (5/11 2ª medida: -48.6 com -3.1)
    case PerformanceModuleBody::Core: return -3.9; // -46.8 dB (5/11 2ª medida: -48.0 com -2.9)
    case PerformanceModuleBody::FmFlute: return -7.4; // -46.9 dB (5/11 2ª medida: -44.5 com -2.8)
    case PerformanceModuleBody::AtariPunk: return -2.4; // -47.2 dB (5/11 2ª medida: -49.1 com -2.5)
    case PerformanceModuleBody::ThermalStatic: return -0.8; // -47.3 dB (5/11 2ª medida: -50.6 com -2.4)
    case PerformanceModuleBody::HardSync: return -2.8; // -47.8 dB (5/11 2ª medida: -48.2 com -1.9)
    case PerformanceModuleBody::Choir: return -1.3; // -48.3 dB (5/11 2ª medida: -49.2 com -1.4)
    case PerformanceModuleBody::Bloom: return +0.9; // -49.0 dB (5/11 2ª medida: -50.7 com -0.7)
    case PerformanceModuleBody::Footnote: return +0.0; // -49.7 dB (5/11 2ª medida: -49.1 com +0.0)
    case PerformanceModuleBody::Gendyn: return +0.5; // -50.1 dB (5/11 2ª medida: -49.1 com +0.4)
    case PerformanceModuleBody::DejaVu: return +0.4; // -50.4 dB (5/11 2ª medida: -48.8 com +0.7)
    case PerformanceModuleBody::Spectral: return +1.2; // -51.3 dB (5/11 2ª medida: -48.7 com +1.6)
    case PerformanceModuleBody::Tuba: return +0.7; // -51.6 dB (5/11 2ª medida: -47.9 com +1.9)
    case PerformanceModuleBody::Harmonaig: return -0.7; // -52.1 dB (5/11 2ª medida: -46.0 com +2.4)
    case PerformanceModuleBody::Scat: return +2.5; // -52.3 dB (5/11 2ª medida: -49.0 com +2.6)
    case PerformanceModuleBody::Atropelo: return +5.3; // -53.9 dB (5/11 2ª medida: -50.2 com +4.1)
    case PerformanceModuleBody::Texture: return +5.3; // -55.0 dB (5/11 2ª medida: -49.1 com +5.3)
    case PerformanceModuleBody::Percussion: return +5.8; // -55.0 dB (5/11 2ª medida: -49.6 com +5.3)
    case PerformanceModuleBody::Acid: return +8.4; // -55.2 dB (5/11 2ª medida: -52.0 com +5.5)
    case PerformanceModuleBody::Bluff: return +7.5; // -55.2 dB (5/11 2ª medida: -51.1 com +5.5)
    case PerformanceModuleBody::ModalResonator: return +9.0; // -60.0 dB (5/11 2ª medida: -53.1 com +9.0)
    // As vozes do Studio de 6/11, medidas no corpo 2 de 12 seeds × 60 s
    // (sonda `novas`), levadas à mediana da 2ª medida (−49,1 dB).
    case PerformanceModuleBody::AyChip: return +0.2; // -49.3 dB
    case PerformanceModuleBody::Morph: return -1.0; // -48.1 dB
    case PerformanceModuleBody::Rossler: return -4.2; // -44.9 dB
    case PerformanceModuleBody::EnvelopePulse: return -1.7; // -47.4 dB
    default: return 0.0;
    }
}

} // namespace rasgo::performance
