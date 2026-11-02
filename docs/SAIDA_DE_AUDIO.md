# Saída de áudio — padrão de álbum (2026-10-30)

Pedido do autor: "verifique a excelência de saída de áudio, parece que está
baixo o som"; "seguir o padrão dos outros instrumentos, como o Navalha 2 ou o
Antitotem — padrão de qualidade de saída de som, já pensando nas gravações
prontas, padrão álbum".

## 1. O que estava errado (medido)

`ffmpeg -af ebur128=peak=true` (ITU-R BS.1770) nas tomadas do autor:

| Tomada | Loudness integrada | Pico |
|---|---|---|
| 30/10 02:10 | −46,6 LUFS | −28,8 dBFS |
| 30/10 01:00 | −49,7 LUFS | −30,9 dBFS |
| 29/10 22:20 | −44,0 LUFS | −28,3 dBFS |

Cerca de 30 dB abaixo de um álbum (−16 a −14 LUFS, true peak ≤ −1 dBTP).
Causas: o ganho de saída do motor (`0,06 + densidade·0,19 + intervenção·0,05`)
vem de quando o instrumento era uma camada de senoides; o MASTER começa em
−6 dB; não havia nivelador nem limitador (só um teto suave em 0,98) nem
dither na gravação de 24 bits.

## 2. O padrão dos outros instrumentos RASGO

Navalha 2 (`NAVALHA2_JUCE/docs/AUDITORIA_ENGENHARIA_SAIDA_AUDIO.md`) e
Antitotem (`ANTITOTEM/src/core/OutputStage.h`, `SignalLeveler.h`): proteção
(valores finitos, DC), nivelamento automático (gain staging), limitador com
antecipação e teto (true peak), medição (dBFS, true peak, RMS, LUFS
BS.1770), WAV de 24 bits com dither TPDF. Master do Navalha 2: −13,6 LUFS;
Antitotem: true peak entre −1,7 e −1,1 dBTP.

## 3. O que mudou

`include/rasgo/performance/PerformanceMasterStage.hpp`, segundo a
**arquitetura comum de saída RASGO**
(`RASGO_DOCUMENTATION/architecture/SAIDA_AUDIO_COMUM.md`), que já previa que o
Performance (hoje Asterismus) a adotasse. Cadeia, depois da soma estéreo:

1. DC fora;
2. **orçamento de ganho fixo** (+26 dB);
3. **MASTER** do performer (saiu de dentro do ganho do motor);
4. **limitador** com antecipação de 2 ms e detecção de true peak 4×;
5. joelho suave até o teto 0,85 (≈ −1,41 dBTP);
6. parado, depois do fade do STOP: zero exato (o estado do master é zerado —
   sem cauda do filtro anti-DC).

Na gravação (`PerformanceMixRecorder`): **dither TPDF** de ±1 LSB só na
redução para 24 bits (nunca no barramento float). Os **stems** continuam crus
(antes do master): material de mixagem.

**Correção no mesmo dia.** A primeira versão tinha um nivelador automático
(cópia adaptada do `SignalLeveler` do Antitotem) que "andava no fader" até
−16 LUFS. A arquitetura comum diz que a loudness informa a decisão, mas **não
deve normalizar uma performance ao vivo automaticamente**. O nivelador saiu;
ficou o ganho fixo. Normalizar para álbum ou streaming é um passo separado,
offline, na entrega (como o TRACK/ALBUM MASTER do Navalha 2).

Proveniência: técnica do `OutputStage` do Antitotem (commit `645e5ad`,
AGPL-3.0-or-later — ver `THIRD_PARTY_NOTICES.md`).

## 4. Medido (render de 90 s, MASTER −6 dB, trama e variação ligadas)

| Seed | Antes | Com o master (ganho fixo) | LRA | True peak |
|---|---|---|---|---|
| 2876238397 | −43,6 LUFS | −17,6 LUFS | 10,1 LU | −7,2 dBTP |
| 181 | — | −18,5 LUFS | 13,0 LU | −3,4 dBTP |
| 31415 | — | −19,9 LUFS | 13,9 LU | −3,4 dBTP |

A dinâmica da performance fica inteira. Com o MASTER em 0 dB, ~6 dB a mais
(−12 a −14 LUFS), com o limitador segurando o teto. Renders descartáveis em
`AUDIOS_RASGO/Asterismus/testes-master/`.

Testes: o master aplica exatamente o ganho fixo; nada passa do teto (nem entre
amostras); o STOP termina em silêncio exato. Referência A/B da trama desligada
(o nível mudou): `4230363d713a0000`.

## 5. Medição (30/10)

`include/rasgo/performance/PerformanceLoudnessMeter.hpp` — cópia adaptada do
Navalha 2 (GPLv3): ponderação K da BS.1770-4 (commit `b1a9614`) e detector de
true peak 4× (commit `34f8fd7`). Na saída, depois do master: loudness
momentânea (400 ms) e de curto prazo (3 s), true peak com retenção (decai
~8,7 dB/s). O rodapé mostra **LUFS** (curto prazo) e **TP** (dBTP) enquanto
toca; o campo `truePeakMicros` da telemetria, que era o pico por amostra,
agora é true peak de verdade. Teste contra a EBU Tech 3341, caso 1: o tom de
1 kHz a −23 dBFS nos dois canais mede −23,04 LUFS e −23,00 dBTP. Só mede: o
som não muda (hash idêntico).

## 6. Limitador com true peak (30/10)

(Os números de loudness desta seção são da versão com nivelador, antes da
correção; os atuais estão no §4.)

O detector do limitador passou a ser o true peak 4× (a mesma classe do
medidor, `PerformanceTruePeakDetector`), com envelope de pico instantâneo e
liberação lenta. Teste: um seno em fs/4 a 45° (amostras a 0,707 do pico),
muito acima do teto, sai com true peak de −1,41 dBTP — exatamente o teto. Nas
três peças de referência, loudness e picos não mudaram (o nivelador já deixa
folga; o limitador quase nunca age). Referência A/B da trama desligada:
`778aee1d5c2f9cbb` (o detector muda arredondamentos mínimos).

## 7. Próximo
- Escuta do autor: alvo de loudness e quanto o nivelador pode subir.
