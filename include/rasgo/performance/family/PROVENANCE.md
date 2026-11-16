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

## Saída técnica comum (16/11/2026)

Nivelamento pela versão de referência `saida-ref 1.0`
(`RASGO/REFERENCIAS/SAIDA_TECNICA/` no monorepo do autor; núcleo do Aquorbium
`ab1da6b`, AGPL-3.0-or-later, mesmo autor).

| Arquivo | Origem | SHA-256 da origem | Uso |
|---|---|---|---|
| `saida/Dither.hpp` | `REFERENCIAS/SAIDA_TECNICA/src/Dither.h` (do gravador do Aquorbium ← Navalha 2) + `floatForJuceInt24` (do nivelamento do Antitotem, mesmo dia) | `9e8a2c60d975e99fe620e2200470cab81b2c1e973dfb936faa4a22e57fb1d946` | dither TPDF no domínio inteiro para a mistura e os stems do REC; namespace `rasgo::performance`, sem a linha SPDX |
| `saida/DeliveryMaster`, `OutputStage`, `LoudnessMeter`, `TruePeakDetector`, `TakeAnalysis`, `Numeric` (.h/.cpp) | `REFERENCIAS/SAIDA_TECNICA/src/` (`saida-ref 1.1`, 16/11/2026; núcleo do Aquorbium `ab1da6b` + `DeliveryMaster` novo) | hashes nos cabeçalhos | versão de entrega a −14 LUFS ao lado da mistura gravada (`src/app/StreamingVersion.h`); namespace `rasgo::performance::saida`, sem a linha SPDX |

