# Protocolo de capacidade do Performance

Objetivo: descobrir quantos corpos simultâneos a máquina sustenta sem travar,
sem xruns persistentes ou sem perda de resposta. O benchmark não substitui
escuta musical e não deve ser executado durante trabalho não salvo.

## Máquina auditada em 2026-08-30

- Intel Core i5-6500, 4 núcleos/threads;
- 23 GiB de RAM total, aproximadamente 14 GiB disponíveis na leitura;
- swap disponível, sem uso na leitura.

### Medição executada — 2026-08-30

Com `48000 Hz`, buffer de 256 samples, sessão simulada de 20 minutos e
relações espaciais/simbiose ativas:

- 3 corpos: 143,512 s de tempo de máquina para 1200 s simulados (8,36× tempo
  real), zero alocações no `processBlock`, zero xruns, saída finita e RSS máximo
  de 4224 KiB;
- 4 corpos (`makeWithWalsh`, Walsh ativo e próximo do Acid): 161,150 s
  (7,45× tempo real), zero alocações, zero xruns, quatro stems estáveis e RSS
  máximo de 4352 KiB na execução combinada.
- 4 corpos (`hard-sync`, seed 186, Hard Sync ativo): 163,108 s (7,36× tempo
  real), zero alocações, zero xruns e quatro stems estáveis.

Essa medição sustenta quatro corpos como limite de desenvolvimento nesta
máquina, não como certificação universal. O limite de produção deve manter
margem e ainda requer repetição em dispositivo de áudio real.

### Smoke offline do quinto corpo — 2026-09-01

O alvo isolado do `ModalResonator`, compilado exclusivamente com teto cinco,
executou 60 segundos simulados em 2,456 s (24,43× tempo real), com zero
alocações no `processBlock`, pico offline de 0,717 ms por bloco de 256 samples
(orçamento de 5,333 ms a 48 kHz) e RSS máximo de 4096 KiB. Durante a sessão o
ator 5 atravessa ganho, mute, solo e retorno ao mixer normal. Ele confirma que
o caminho futuro de stem/mixer permanece bounded sob carga curta. Não mede
dispositivo, latência, xruns reais ou RSS de uma sessão longa, nem autoriza
alterar o limite do host.

Para repetir o smoke:

```bash
./build-app/rasgo_synth_performance_modal_fifth_stability_tests modal-five-smoke
```

O ensaio de 20 minutos continua obrigatório em terminal sem corte de duração:

```bash
./build-app/rasgo_synth_performance_modal_fifth_stability_tests modal-five
```

### Medição longa offline do quinto corpo — 2026-09-01

O ensaio isolado de 20 minutos simulados terminou: 49,88 s de máquina (24,06×
tempo real), pico offline de 0,988 ms por bloco de 256 samples, RSS máximo de
5808 KiB e zero alocações no `processBlock`. A carga inclui a relação Memory e
uma travessia de ganho, mute, solo e retorno normal do mixer do ator 5. A saída
e a telemetria permaneceram finitas, com cinco stems e sem xruns internos.

O resultado autoriza continuar a preparação técnica do `ModalResonator` como
quinta voz, mas **não** altera `kPerformanceSafeBodyLimit == 4`: ainda faltam
driver/dispositivo real, latência observada, escuta musical e margem de CPU do
host gráfico.

## Escada de teste

1. iniciar com 3 corpos, buffer e sample rate de produção;
2. acrescentar um corpo por vez (4, depois 5), mantendo a mesma seed;
3. medir CPU do callback, xruns, memória, pico e estabilidade por sessão;
4. repetir com relações espaciais desligadas e ligadas;
5. repetir com controles contextuais e simbiose ativos;
6. parar no primeiro limite sustentado e adotar margem de segurança.

## Critérios de bloqueio

Qualquer valor não finito, xrun recorrente, crescimento de memória, estalo,
perda de resposta ou sobrecarga sustentada interrompe o teste. O limite de
produção será inferior ao primeiro ponto de falha, nunca igual a ele.

O resultado deverá registrar seed, número de corpos, sample rate, buffer,
duração, CPU, xruns, memória, versão do binário e observação auditiva. Nenhuma
capacidade será liberada por extrapolação teórica dos 23 stems catalogados no
Studio.

## Ensaio de autorização no host real

Este é o gate que ainda falta para considerar o quinto corpo no host. Executar
somente com trabalho salvo, em uma sessão gráfica que possua dispositivo de
áudio real, usando o binário Release atual:

```bash
bash ./check_host_readiness.sh
./run_performance.sh
```

O primeiro comando não abre o instrumento nem mede áudio: confirma binário,
volume gravável, memória, sessão gráfica e exposição do dispositivo. Ele falha
quando algum gate essencial estiver ausente e imprime apenas diagnósticos de
leitura; `PRONTO` significa que o ensaio pode começar, nunca que foi aprovado.

1. abrir uma seed que mostre `STEMS 4 +1 PREP`; confirmar que a mesa continua
   com somente os corpos exigidos pela seed e que não existe um cartão Modal
   arrastável falso;
2. tocar por pelo menos 20 minutos com os quatro corpos atuais, usando a mesa,
   relações e células; observar `XRUNS`, CPU, dispositivo aberto e pico no
   rodapé;
3. deixar o instrumento sem manipulação por alguns minutos e escutar a
   trajetória da seed atuar sem mover nem apagar o desenho espacial;
4. registrar sample rate, buffer, dispositivo, CPU máxima observada, xruns,
   memória, seed, versão do binário e a decisão auditiva; interromper no
   primeiro estalo, atraso persistente, perda de resposta ou pressão de memória;
5. somente depois repetir com um build de ensaio de cinco corpos. A alteração
   de `kPerformanceSafeBodyLimit` e qualquer cartão/gesto do quinto stem exigem
   uma decisão explícita baseada nesses dois ensaios, não a execução deste
   roteiro isoladamente.

Na sessão de desenvolvimento de 2026-09-01, `DISPLAY=:0` estava definido, mas
o servidor recusou conexão e não havia `/dev/snd`; o lançador recusou iniciar o
host. Não houve alegação de latência, xrun ou escuta real a partir dela.
