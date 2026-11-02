# Ponte futura entre Rasgo Synth Studio e Performance

**Estado:** direção arquitetural; não implementar antes dos contratos e do
primeiro motor Performance validados.

## Dois instrumentos autônomos, uma ecologia compartilhada

Studio e Performance devem abrir, funcionar, salvar e produzir resultados sem
que um dependa da instalação ou das pastas privadas do outro. Em estágio
avançado, porém, podem formar um ciclo criativo:

```text
STUDIO
  pesquisa, compõe, renderiza, compara e revela um território
      -> Rasgo Territory Manifest
PERFORMANCE
  importa o território, nutre constelações e registra uma trajetória
      -> Rasgo Performance Trace
STUDIO
  estuda, arquiva ou reinterpreta a trajetória em render offline futuro
```

A complementaridade acontece por contratos de dados explícitos, não por
includes cruzados entre apps, leitura da memória interna do outro processo ou
uso do mesmo diretório de preferências.

## O que o Studio poderá oferecer

Um futuro `Rasgo Territory Manifest` poderá registrar:

- seed, versão do gerador e identidade;
- vocabulário tonal/modal, afinação e registro;
- DNA temporal, métrico e de groove;
- módulos/atores potenciais e capacidades verificadas;
- afinidades, resistências e limites relacionais;
- envelopes de risco, memória, densidade e ruptura;
- proveniência suficiente para reproduzir ou diagnosticar o território.

O manifesto não precisa conter áudio nem congelar a peça renderizada. O mesmo
território pode produzir uma obra fechada no Studio e trajetórias distintas no
Performance. Identidade compartilhada não significa PCM idêntico.

## O que o Performance poderá devolver

Um futuro `Rasgo Performance Trace` poderá registrar:

- referência ao território/seed de origem;
- cenas e constelações realmente usadas;
- ações de nutrição do performer e mudanças de foco/papel;
- relações solicitadas e realizações observadas;
- eventos, tempos, falhas, reparos, rasgos e vestígios;
- trajetória dos macrocampos e evidência de audibilidade;
- referência opcional ao take gravado, sem incorporar o áudio ao documento.

O Studio poderá futuramente usar esse trace como partitura de acontecimentos,
material de análise, memória composicional ou entrada para um render offline de
alta qualidade. Essa importação não está autorizada agora: primeiro é preciso
provar que o trace contém valor musical real e não apenas telemetria.

## Etapas de aproximação

1. **Autonomia:** Performance usa contratos e doubles próprios; Studio fica
   intocado.
2. **Linguagem comum:** estabilizar IDs, capacidades, papéis, relações e
   versionamento em testes independentes.
3. **Ponte somente leitura:** Performance importa um manifesto exportado pelo
   Studio, sem escrever em sua pasta.
4. **Retorno criativo:** após validação musical, Studio passa a importar um
   trace como dado externo e opcional.
5. **Sincronização ao vivo:** OSC, rede ou processos acoplados só serão
   avaliados se houver necessidade artística comprovada; não são requisito da
   primeira release.

## Regras de preservação

- nenhum passo altera o Studio preventivamente;
- formatos têm versão, limites, validação e migração documentada;
- caminhos de áudio, recordings, preferências e builds continuam separados;
- uma seed compartilhada garante parentesco de território, não igualdade de
  eventos ou som;
- módulos indisponíveis numa versão geram diagnóstico e degradação explícita,
  nunca substituição silenciosa;
- a ponte é opcional: nenhum instrumento deixa de ser completo sozinho.

## Primeiro experimento de complementaridade

Quando o motor Performance já sustentar uma sessão com três a cinco atores:

1. selecionar no Studio uma seed cuja identidade e módulos estejam auditados;
2. criar manualmente um manifesto mínimo imutável para essa seed;
3. executar duas sessões Performance com nutrições diferentes;
4. verificar parentesco de identidade e diferença de trajetória;
5. comparar os traces com o render Studio sem exigir igualdade de forma;
6. somente então decidir se a exportação deve ser implementada no Studio.
