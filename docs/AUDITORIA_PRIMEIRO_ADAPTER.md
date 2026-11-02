# Auditoria do primeiro adapter Rasgo

**Estado:** fronteira v1 integrada; `ModalResonator` promovido a quinto corpo
real dentro da coorte de oito; A/B humano ainda pendente — 2026-09-04.

## Decisão

O primeiro candidato auditado é `ModalResonator`, do core Rasgo Synth. A
escolha definiu a primeira matéria a atravessar a fronteira do Performance;
package, compatibilidade e licença foram verificados. Isso não autoriza
reutilização indiscriminada de outros headers: cada novo corpo exige sua
própria auditoria, adapter, stem, controles, teste bounded e escuta.

| Candidato | Dependências locais | Força musical | Decisão atual |
|---|---:|---|---|
| `ModalResonator` | nenhuma além da biblioteca padrão | corpo ressonante pode atuar como Chão, Corpo, Atrito ou Vestígio; aceita excitação e matéria relacional | selecionado para fronteira v1 |
| `FmFluteVoice` | `Envelope.hpp` | voz Tone de grande interesse, mas exige política de nota/gate e morfologia já conectada | posterior |
| `AcidBasslineVoice` | `Oscillator.hpp`, `PolivoksFilter.hpp` | forte potencial de Atrito/Contágio e baixo performativo | posterior, após contrato de baixo/slide/accent |
| `ThereminVoice` | `Oscillator.hpp`, `LfoBank.hpp` | relação contínua e espacial, sem gate | posterior, após alvo relacional contínuo |

## Evidência de origem e versão auditada

- revisão do workspace: `ae33cba5a1104bccf9427b9d7cf229250cc4dcd5`;
- arquivo observado: `RASGO_SYNTH/rasgo-synth-core/src/dsp/ModalResonator.hpp`;
- SHA-256 observado: `5e87d761f3f9b07ee337234b4e402fb6571e2dba44ce09df4f1593649628b996`;
- API observada: `setSampleRate`, `setFundamental`, `setModes`,
  `setBrightness`, `excite`, `process`;
- implementação observada: arrays fixos, sem heap, e saneamento interno de
  não-finitos antes de contaminar a saída;
- proveniência: `CREDITS.md` registra banco ressonante original do Rasgo,
  inspirado conceitualmente em Rings/Émilie Gillet. `THIRD_PARTY_NOTICES.md`
  registra que não há firmware ou código-fonte de terceiros copiado.

## Fronteira v1 implementada

`ModalMatterAdapter.hpp` define:

- `ModalMatterParameters`: fundamental, brilho, excitação, pressão relacional e
  papel, todos bounded;
- `ModalMatterAdapter`: `prepare`, `reset`, `setParameters`, `excite`,
  `process` e descritor de latência/proveniência;
- `ModalMatterDescriptor`: versão de contrato e hashes observados, sem incluir
  arquivo externo em runtime;
- `NativeModalMatterDouble`: realização provisória independente, de quatro
  parciais, usada apenas para testar a API. Não é `ModalResonator` e não deve
  ser apresentada como o corpo do Studio.

O incremento de 2026-08-25 acrescenta `CoreModalMatterAdapter`, que contém
diretamente um `rasgo::ModalResonator` e traduz fundamental, brilho, pressão
relacional e papel em parâmetros bounded. O pacote público
`rasgo::modal_matter_v1` fica em
`../rasgo-synth-core/cmake/modal-matter/`, não configura o app Studio nem os
demos e falha no CMake se o SHA-256 do header divergir do valor auditado.
O build do Performance continua possível sem o pacote; quando ele está ativo,
o host declara `MODAL v1` no status.

Os testes confirmam rejeição de parâmetros inseguros, saída finita/bounded,
ataque audível, proveniência fixada e diferença mensurável entre o double e o
corpo real. O adapter não contém `JUCE` nem `rasgo-synth-app`.

O caminho deixou de ser uma reserva silenciosa: o ator 5 atravessa presença,
ganho, mute, solo, controles, relações e telemetria junto aos demais corpos.
O ensaio expandido de oito corpos passou offline sem xruns internos,
alocações no callback ou saída não finita. A validação humana no host —
clareza musical, densidade, ergonomia e velocidade dos movimentos — permanece
deliberadamente aberta.

## Gates para integração real

1. [x] criar no core um package/target CMake público de versão fixada, sem
   alterar a aplicação Studio;
2. [x] repetir hash e auditoria de licença/proveniência na revisão usada;
3. [x] testar sample rate, excitação, finitude, limite, reset e descritor;
4. [x] integrar por target versionado, sem include relativo do Performance;
5. [x] executar A/B técnico contra o double, provando corpos distintos;
6. [ ] fazer A/B humano da função musical em dispositivo real;
7. [ ] decidir pela escuta se o corpo modal permanece, muda de papel ou precisa
   de nova calibração antes de qualquer segundo adapter.

A ausência do pacote continua sem impedir build e testes do projeto irmão; ela
apenas remove o corpo modal real da ecologia. O pacote é uma dependência de
build explícita, não uma leitura runtime do checkout do Studio.
