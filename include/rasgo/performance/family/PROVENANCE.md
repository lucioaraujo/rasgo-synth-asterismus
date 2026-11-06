# Cópias da família RASGO (6/11/2026)

Pedido do autor: "faça os filtros e os envios, com os botões"; "no antitotem
há outros filtros interessantes"; "temos muitos efeitos já codados em outros
instrumentos rasgo". Mesmo autor (Lúcio de Araújo); as origens são
AGPL-3.0-or-later, combináveis com o GPLv3 do Asterismus (GPLv3 §13, como o
master já vindo do Antitotem — THIRD_PARTY_NOTICES.md).

| Arquivo | Origem | Commit | SHA-256 | Uso |
|---|---|---|---|---|
| `antitotem/CmosVcf.h` | `lucioaraujo/antitotem`, `src/core/CmosVcf.h` | `6c88ab4` | `5b0e6485c85abab2fba8fae93a64d83bef1bbbc8c9a07c1a582ec4027e494187` | cópia de bytes integrais; filtro CMOS de cada stem |
| `antitotem/MaterialFilter.h` | `lucioaraujo/antitotem`, `src/core/MaterialFilter.h` | `6c88ab4` | `b1716411a0a85d4ed8bce41b0ff671939aff79b26cc024bf8bb950e55c72d107` | cópia de bytes integrais; filtro MATÉRIA de cada stem |
| (adaptado) `PerformanceStemEffects.hpp` | `lucioaraujo/rasgo-modular`, `src/dsp/Filter.hpp` (o núcleo SVF TPT com saturação no laço), `Space.hpp` (tomadas, realimentação com tom, all-pass de difusão) e `Hall.hpp` (FDN de 8 linhas, Householder) | `07dcb33` | Filter `74b15052…`, Space `242ae0a1…`, Hall `2c01cf2e…` (16 primeiros) | só os núcleos de DSP, tirados do grafo de sinais do Modular para amostra-a-amostra; o Hall com o ganho de decaimento calculado por bloco (não por amostra) |
