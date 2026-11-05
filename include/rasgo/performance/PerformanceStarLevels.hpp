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
// As estrelas de controle não soam: correção zero. Numa estrela nova,
// medir com a sonda e acrescentar aqui (sem medida, 0 dB).

#include "PerformanceModuleTypes.hpp"

namespace rasgo::performance {

inline constexpr double performanceStarLevelTrimDb(PerformanceModuleBody star) noexcept {
    switch (star) {
    case PerformanceModuleBody::Theremin: return -7.1; // -42.6 dB
    case PerformanceModuleBody::Galo: return -3.9; // -45.8 dB
    case PerformanceModuleBody::BassFallback: return -3.8; // -45.9 dB
    case PerformanceModuleBody::Walsh: return -3.8; // -45.9 dB
    case PerformanceModuleBody::Vosim: return -3.1; // -46.6 dB
    case PerformanceModuleBody::Core: return -2.9; // -46.8 dB
    case PerformanceModuleBody::FmFlute: return -2.8; // -46.9 dB
    case PerformanceModuleBody::AtariPunk: return -2.5; // -47.2 dB
    case PerformanceModuleBody::ThermalStatic: return -2.4; // -47.3 dB
    case PerformanceModuleBody::HardSync: return -1.9; // -47.8 dB
    case PerformanceModuleBody::Choir: return -1.4; // -48.3 dB
    case PerformanceModuleBody::Bloom: return -0.7; // -49.0 dB
    case PerformanceModuleBody::Footnote: return +0.0; // -49.7 dB
    case PerformanceModuleBody::Gendyn: return +0.4; // -50.1 dB
    case PerformanceModuleBody::DejaVu: return +0.7; // -50.4 dB
    case PerformanceModuleBody::Spectral: return +1.6; // -51.3 dB
    case PerformanceModuleBody::Tuba: return +1.9; // -51.6 dB
    case PerformanceModuleBody::Harmonaig: return +2.4; // -52.1 dB
    case PerformanceModuleBody::Scat: return +2.6; // -52.3 dB
    case PerformanceModuleBody::Atropelo: return +4.1; // -53.9 dB
    case PerformanceModuleBody::Texture: return +5.3; // -55.0 dB
    case PerformanceModuleBody::Percussion: return +5.3; // -55.0 dB
    case PerformanceModuleBody::Acid: return +5.5; // -55.2 dB
    case PerformanceModuleBody::Bluff: return +5.5; // -55.2 dB
    case PerformanceModuleBody::ModalResonator: return +9.0; // -60.0 dB
    default: return 0.0;
    }
}

} // namespace rasgo::performance
