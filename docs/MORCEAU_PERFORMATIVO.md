# Morceau performativo

**Estado:** contrato de produto e implementação — 2026-09-06
**Escopo:** Rasgo Synth Performance; sem alteração do Studio.

## Definição

Cada gravação aceita do Performance é uma música completa, não apenas um trecho
de áudio capturado. O *morceau performativo* nasce de uma tomada real: a seed
abre uma cosmologia, o músico coreografa a Constelação, o sistema desenvolve a
narrativa nas pausas e uma coda encerra a obra e a própria gravação.

O Studio produz uma faixa por realização generativa. O Performance produz uma
faixa por realização performada. São instrumentos irmãos, com formas próprias.

## Ciclo da obra

| Estado | Função musical | Gravação |
|---|---|---|
| preparado | seed, módulos, papéis e Campo prontos | arquivo ainda fechado |
| abertura | primeira entrada e apresentação do território | abre WAV e trace antes do primeiro som |
| desenvolvimento | diálogo músico–seed, jogo, dança e memória | registra áudio e acontecimentos |
| suspensão | pausa do músico; narrativa continua | grava normalmente |
| coda solicitada | músico pede o fim ou inatividade prolongada o propõe | permanece gravando |
| coda comprometida | retorno já não cancela; trajetória vivida conduz o final | registra fechamento e caudas |
| cauda segura | silêncio musical alcançado; efeitos e ressonâncias terminam | aguarda limite bounded |
| finalizado | áudio e trace são fechados atomicamente | publica o morceau completo |
| interrompido | STOP/Panic/dispositivo encerrou de emergência | preserva take com estado explícito |

## Operação no instrumento

`INICIAR` deve iniciar música e gravação juntas. Ele abre o arquivo antes da
primeira amostra, coloca o transporte em PLAY e dispara uma abertura derivada
da seed. A operação não redesenha o Campo.

`ENCERRAR / CODA` pede um final musical. A coda lê seed, duração, regiões
visitadas, funções dos módulos, relações, cicatrizes, densidade e último gesto.
Ela pode escolher retorno, dispersão, reunião, rarefação, implosão, última
colisão, corte ou silêncio plural. O gravador fecha somente depois da cauda.

`STOP` continua existindo como segurança. Ele não finge uma coda: encerra o
arquivo como `interrompido`, preservando o áudio válido já escrito. `PANIC
SAFE` e a perda do dispositivo fazem o mesmo: o callback apenas levanta um
sinal atômico, e a interface fecha WAV e manifesto fora da rota realtime.

## Inatividade

Uma pausa curta libera o jogo autônomo da seed; não inicia o fim. A decisão de
CODA agora combina: seed, batidas sem intervenção, idade da tomada, turnos
narrativos já realizados, memória, risco, viés relacional do território e massa
dos cabos ativos. Cada seed escolhe outro limiar; uma nova ação do músico zera
a elegibilidade sem redesenhar o Campo. Quando a coda entra na fase
comprometida, ela conclui a peça com duração bounded e não volta silenciosamente
ao desenvolvimento.

Essa regra não substitui a escuta: ela é determinística e limitada, mas os
limites musicais por família e a escolha humana de uma tomada continuam sendo
validados no host real.

## Conteúdo do arquivo final

O primeiro produto é um WAV estéreo de 24 bits, já existente no gravador. Ao
finalizar, ele deve receber um manifesto lateral com:

- identificador, versão, data, duração e estado `completo` ou `interrompido`;
- seed principal e sementes de influência;
- módulos e funções orquestrais usados;
- BPM inicial e regiões harmônicas percorridas;
- estratégia, regra, acontecimentos, relações e fenômenos principais;
- trajetória do Campo, memória, rasgos, reparos e tipo de coda;
- taxa de amostragem, canais, blocos descartados e diagnóstico técnico.

O manifesto não substitui o áudio e não incorpora o WAV. Stems podem ser uma
expansão posterior; não bloqueiam o primeiro morceau completo.

## Integridade

- Escrita de áudio permanece em thread própria e fila bounded.
- O caminho realtime nunca cria arquivos, strings ou estruturas crescentes.
- Fechamento ocorre fora do callback e preserva take parcial em STOP, PANIC
  SAFE ou falha/perda de dispositivo. A taxa de amostragem é guardada na
  abertura da tomada, portanto seu manifesto mantém duração e taxa corretas
  mesmo quando o dispositivo já saiu.
- STOP não é transformado em coda: ele recebe somente uma borda técnica de
  8 ms em meia-cosseno no limite de saída. O host espera esse bloco antes de
  fechar o writer, evitando que o WAV termine numa descontinuidade. PANIC SAFE
  mantém prioridade de silenciamento imediato.
- O nome provisório é criado no início; o estado completo só é declarado após
  fechamento e validação do WAV.
- O Campo, a sessão e a seed permanecem disponíveis depois do fim para nova
  tomada, sem apagar a obra anterior.

## Abertura, fechamento e acabamento do áudio

O *morceau* precisa de duas escalas de envelope, com funções diferentes:

1. **Envelope musical:** abertura e coda derivadas da seed e da trajetória. Sua duração pode variar de centenas de milissegundos a vários segundos e pode atuar por entrada de módulos, rarefação, retorno, dispersão ou silêncio.
2. **Envelope técnico de borda:** rampa curta e invariável de segurança aplicada à primeira e à última amostra válida. Ele evita descontinuidade e clique, mesmo quando a abertura ou a coda musical forem abruptas.

A cadeia final prevista é: soma das stems, espacialização, envelope musical, MASTER, proteção de pico transparente, envelope técnico de borda, meter e gravador. Deve haver margem real antes de 0 dBFS; limitar não pode esconder excesso permanente de ganho nem destruir a dinâmica.

O estado atual contém fade-in curto de PLAY, MASTER inicialmente em −6 dB, clamp de segurança em ±0,98 e gravação do sinal pós-engine. Isso ainda não equivale ao acabamento final: `STOP` é uma borda dura, não há true-peak certificado, alvo de loudness ou masterização offline. O ciclo do *morceau* deve acrescentar essas camadas sem transformar o instrumento ao vivo num compressor contínuo e cansativo.

Em 2026-09-06 entrou a primeira fundação executável:
`PerformanceMorceauLifecycle`. Ela é uma máquina de estados sem alocação nem
I/O, segura para avançar no callback: `preparado → abertura → desenvolvimento
→ coda → cauda → finalizado`, com `interrompido` separado como fronteira de
emergência. Abertura e coda usam envelopes de meia-cosseno bounded; a cauda
mantém a gravação aberta sem criar energia nova. O teste unitário prova os
limites, as transições e a diferença entre finalização e interrupção. A
integração seguinte é fazê-la comandar o ciclo do gravador, enquanto a ecologia
escolhe o conteúdo musical da coda.

A integração com o engine foi concluída na mesma data: o envelope é aplicado
no limite final da saída, depois da composição das stems e antes da escrita de
áudio. `PLAY` entra em abertura, `beginMorceauCoda()` inicia a queda musical e
`STOP` marca interrupção. Recall de cena durante uma realização não reinicia o
envelope. O host agora abre REC antes de enfileirar PLAY; REC durante uma
tomada pede CODA; o timer de controle fecha o WAV somente quando o áudio
publica `Finalized`, depois da cauda. STOP, PANIC SAFE e perda do dispositivo
preservam o WAV como take interrompido. Ainda faltam o epílogo por inatividade
e uma escolha composicional mais detalhada da coda pela ecologia.

Ao receber CODA, a ecologia já deriva da seed, do turno narrativo e dos cabos
presentes um verbo de encerramento: **reunião** (um corpo conduz e os demais
convergem), **retirada em cadeia** (a proximidade ao foco define a saída),
**memória relacional** (cabos retêm vestígios proporcionais) ou **dispersão**
(cada corpo perde peso em outra velocidade). O verbo muda hierarquia e registro
das stems, jamais as posições desenhadas pelo músico; o envelope global apenas
conclui o processo com segurança.

Cada WAV fechado pelo host recebe agora um JSON vizinho de mesmo nome. Ele
registra formato, versão, data de início, seed, taxa, canais, amostras aceitas,
duração, blocos descartados, estado `finalizado` ou `interrompido` e um trace
inicial (seed, gestos enfileirados, turnos narrativos, pedido de coda e
interrupções). Ele registra também os módulos com papel e presença, cabos
ativos com intensidade/fenômeno e, quando houve final intencional, o verbo da
CODA (`gather`, `relay`, `memory` ou `disperse`). É um manifesto técnico inicial
e uma cópia de acesso: não substitui o WAV nem o traçado temporal detalhado do
Campo, que continua como próxima camada arquivística.

Após o `ThreadedWriter` drenar, o host lê o WAV já fechado fora da rota
realtime e acrescenta ao manifesto `samplePeakDbfs`,
`estimatedInterSamplePeakDbfs` e `integratedRmsDbfs`. A estimativa
interamostral é uma interpolação simples; RMS não é LUFS; nenhum dos dois é
vendido como true-peak certificado, normalização ou masterização. No motor, o
teto final é transparente abaixo de -0,18 dBFS e só curva o excedente para o
headroom restante.

## Critério de pronto

Uma execução deve começar antes do primeiro som, desenvolver-se com e sem a
mão do performer, receber uma coda musical, terminar após cauda segura e gerar
um WAV estéreo reproduzível acompanhado de manifesto coerente. Um fechamento
de emergência precisa continuar recuperável e claramente identificado como
interrompido.
