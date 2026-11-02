# Programa performativo canônico — Rasgo Synth Performance

**Estado:** direção criativa e critério de implementação — 2026-09-02
**Função:** reunir, sem substituir as fontes, o conceito performático disperso
entre estratégia musical, campo relacional, jogo da seed, células, colagem,
painel e continuidade.

## Tese central

O Rasgo Synth Performance é um **instrumento generativo de campo**. A seed
coloca uma ecologia musical em curso; o performer modela um campo de corpos,
distâncias, relações e fluxos; os módulos interpretam esse campo e produzem
acontecimentos; memória, vestígio e jogo alteram o que poderá acontecer depois.

Assim como o Rasgo Synth Studio, ele é um instrumento que **gera músicas
gravadas do começo ao fim**. Sua unidade de realização não é um preset, um
loop ou uma sessão sonora sem término, mas uma faixa com abertura,
desenvolvimento, coda e epílogo. No Performance, forma e duração emergem do
diálogo entre o campo modelado pelo músico e a narrativa generativa de uma ou
mais seeds; áudio e trace registram essa mesma realização.

O instrumento não é um rack silencioso, uma coleção de presets, um disparador
de cenas nem um painel de efeitos. Também não é uma versão reduzida do Studio.
Ele torna corporal, temporal e espacialmente performável a composição viva que
o universo Rasgo já sabe gerar.

```text
seed propõe território e jogo
  -> módulos reais começam a soar
      -> performer desenha campo, relações e movimentos
          -> módulos se contaminam, resistem, respondem e deixam vestígios
              -> a pausa abre espaço para a narrativa da seed
                  -> performer escuta, aceita, contraria e remodela
```

## Princípios que governam o código

1. **O campo é do músico.** Seed, célula, cena, recall e automação não movem,
   apagam ou substituem posições. Só um gesto espacial explícito altera a
   cartografia.
2. **A seed possui origem e vida.** A origem escolhe corpos, predisposições,
   relações, limites e histórias possíveis. Uma ou mais seeds vivas podem ser
   derivadas, enxertadas, fixadas e retornadas pelo performer sem apagar o campo.
3. **Cada caixa é um módulo, um ator e um stem.** Não existem agrupamentos
   invisíveis dentro de um cartão nem módulos de catálogo sem corpo de áudio.
4. **O performer compõe, não apenas intervém.** Ele escolhe formação, foco,
   topologia, proximidade, fluxo, continuidade, corte, silêncio, risco e memória.
5. **Espaço é causalidade musical.** Aproximar, afastar, conectar, orbitar,
   agrupar e separar corpos altera simbiose, ritmo, matéria, densidade e espaço;
   não serve apenas como panorama ou animação.
6. **A pausa não é abandono.** Sem manipulação, a seed continua uma narrativa
   situada no campo vigente, com repetição e diferença, acúmulo, contraste e
   retorno; nunca repete indefinidamente um ciclo curto.
7. **Relação não é LFO genérico.** Apoio, pergunta/resposta, hocket, sombra,
   contágio, atrito, infiltração, reparo e órbita possuem fonte, destino,
   domínio, duração, limite e consequência própria.
8. **Caos tem causa.** Alea, ilinx, conflito, glitches, cortes e colagem podem
   surpreender, mas permanecem determinísticos dentro da seed, bounded e
   legíveis pela escuta.
9. **Toda ação performática responde.** Um controle criativo deve produzir
   consequência sonora reconhecível imediatamente ou declarar claramente que
   prepara uma transformação futura. Mudança mínima de PCM não é aceitação.
10. **Toda transformação pode retornar.** Estados persistentes mostram seu
    alcance e podem ser reduzidos/desligados; gestos temporários mostram ação,
    duração e recolhimento; ações destrutivas exigem checkpoint/undo.
11. **Memória altera futuro.** Rasgo, vestígio, fadiga, repetição e reparo não
    são efeitos descartáveis: influenciam relações e acontecimentos posteriores.
12. **Escuta humana é gate.** Build, estabilidade e A/B numérico provam
    segurança e causalidade técnica; não provam valor performático.
13. **Nada é decorativo.** Toda forma coletiva, órbita, cabo, fluxo, halo e
    controle representa estado ou acontecimento musical real; caso contrário,
    permanece oculto.
14. **Toda realização gera uma tomada.** `INICIAR` abre simultaneamente música,
    áudio e trace; a coda conclui musicalmente e a cauda segura fecha o arquivo.
    Gravar não é modo opcional ou operação posterior.
15. **A seed começa nos módulos e no jogo.** Ela só abre um campo quando escolhe
    corpos realmente audíveis, papéis, relações e uma estratégia composicional
    inicial. Esses elementos são o motor de sua trajetória, não decoração de
    interface ou dados acrescentados depois.
16. **Músico e seed são interlocutores.** O performer pode acompanhar,
    contradizer, perverter, interromper, rasgar, reparar e recombinar as
    proposições da seed. Ela responde pelos módulos, pelo jogo e pela memória;
    nenhuma das duas agências é simples controle remoto da outra.
17. **Irmãos gêmeos, fronteiras íntegras.** Antes de criar no Performance
    qualquer módulo, estratégia, trajetória, relação, memória ou coda,
    localizar e auditar o mecanismo correspondente no Studio/core. Se ele já
    estiver correto, ele pode ser copiado de modo controlado para a árvore do
    Performance e adaptado na borda necessária; quando houver lacuna própria do
    campo, o Performance cria código novo. Reutilização registra proveniência,
    licença e testes. Studio/core são sempre somente leitura: uma necessidade
    do Performance nunca autoriza alterá-los, depender de seu runtime ou fundir
    os dois instrumentos.
18. **A Constelação orienta.** Ela torna legíveis estado atual, percurso,
    proximidades, encontros, tendências e limiares. O músico encontra caminhos
    no Campo sem receber uma sequência obrigatória, e a navegação transforma o
    mapa por memória e cicatriz.
19. **Vista e composição são distintas.** Orbitar a vista ajuda a compreender
    o polígono tridimensional sem mudar o som. Girar a Constelação musical
    transforma relações e precisa produzir consequência audível.

## Três escalas da atuação

| Escala | O músico faz | O instrumento responde |
|---|---|---|
| **Conduzir** | nutre energia, densidade, continuidade, risco, memória, silêncio e espaço | reorganiza o comportamento da ecologia sem apagar sua identidade |
| **Negociar** | aproxima, afasta, conecta, separa, foca, isola e troca papéis | produz permeabilidade, resistência, contágio, disputa, resposta ou recuo |
| **Inscrever** | corta, repete, desloca, rasga, repara, recorda e deixa vestígios | altera o presente e as possibilidades narrativas seguintes |

Sliders e knobs de precisão podem existir como controles contextuais, mas a
linguagem principal do CAMPO é feita de corpos, verbos, relações e
consequências, não nomes técnicos de parâmetros.

## Movimento e dança dos corpos

A posição autoral integra o desenho-base do músico. Sobre ele, a constelação e
cada corpo podem receber movimentos combináveis:

- órbita elíptica, circular ou irregular;
- pulso radial e respiração;
- figura-8 e cruzamentos;
- rotação em torno de si;
- deriva e deslocamento confinado;
- atração, repulsão, perseguição e troca de centro;
- fases, sentidos e divisões rítmicas distintas por corpo.

O polígono é a configuração coletiva que emerge do conjunto de astros/módulos;
não é um pequeno elemento que orbita cada círculo. O movimento gráfico e o
movimento sonoro são duas leituras do mesmo estado. Uma transformação de Campo
gira, escala ou desloca a formação relativa a partir de seu centro; uma dança
local ou força explicitamente escolhida pode deformá-la temporariamente. Cada
dança atua no stem correspondente — articulação, pulso, modulação, panorama,
acoplamento ou recorte — sem apagar o desenho-base. O performer escolhe
movimentos e topologias; o sistema não impõe triângulo, quadrado ou qualquer
formação como única regra. A gramática causal completa está em
`GRAMATICA_DO_CAMPO_PERFORMATIVO.md`.

## Seed de origem, seed viva e polifonia de seeds

A seed inicial abre o campo, mas não encerra a autoria do performer. Sua origem
permanece como identidade e ponto de retorno; descendências podem atuar por
corpo, conexão ou campo. Seeds adicionais podem coexistir como núcleos de
influência em sobreposição, mashup, colagem, alternância, contaminação, glitch
ou fusão.

Na abertura, a seed compila um patch performável: módulos necessários e stems,
papéis, estados materiais e temporais, afinidades/resistências, relações
possíveis e uma estratégia de jogo — seguir, transformar, contestar,
interromper, ocultar ou negociar. A estratégia distribui condutas entre corpos
e estabelece o impulso inicial da composição. O campo, o músico e o histórico
passam a transformá-la assim que a música começa.

Somente os módulos necessários à abertura entram automaticamente no campo. Os
complementares pertinentes aparecem em reserva externa e só entram por gesto
do músico. Em ambos os casos, um módulo só é oferecido quando possui adapter,
stem, controles contextuais, papel no jogo e consequência sonora real.

Variar seeds vivas nunca troca coorte, remove polígonos, move âncoras ou apaga
cabos. Os números de seed não são interpolados: cruzam-se traços preparados de
frase, cadência, densidade, tensão, matéria, memória, estratégia e pausa. Um
novo conjunto de corpos só entra por `NOVO CAMPO`, com intenção e memória.

## Simbiose, impregnação e recombinação

Cada par de corpos pode abrir uma relação bounded. Distância, direção, energia,
histórico e papel definem permeabilidade e resistência.

O vocabulário relacional canônico passa a incluir atração, repulsão, órbita,
encontro/colisão, fusão/captura, sobrevoo, espalhamento, perseguição/fuga,
paralelismo, convergência/divergência, oscilação, espirais, equilíbrio,
travamento, sincronização, ressonância, troca e liberação. Esses nomes não são
presets gráficos: são verbos composicionais que distribuem pulso, fratura,
matéria, espaço, tensão, memória, silêncio e variação de seed entre corpos.

| Situação | Possibilidade musical |
|---|---|
| próximos | impregnação, modulação cruzada, mistura, sincronização parcial |
| intermediários | tensão, resposta atrasada, negociação, alternância |
| distantes | autonomia, contraste, eco, memória ou recusa |
| colisão/agrupamento | saturação, hocket, contágio, colapso limitado |
| separação posterior | rastro, fadiga, reparo ou retorno transformado |

Recombinar fluxos não significa somar sinais indiscriminadamente. Pode cruzar
timing, densidade, contorno, matéria, espaço, articulação ou memória. Cada
módulo interpreta a intenção segundo sua identidade; não existe uma modulação
genérica aplicada igualmente a todos.

## Matriz de Campos de Acontecimento 4×4

As 16 células poderão ser reaproveitadas como uma matriz entre **quatro campos
generativos** e **quatro corpos reais do campo**. Não são atalhos para cenas,
macros ou segurança.

| Campo / linha | Função contínua sobre o corpo da coluna |
|---|---|
| **VARIAÇÃO** | a seed desenvolve frase, cadência, densidade e matéria sem repetição mecânica |
| **RECOMBINAÇÃO** | o corpo cruza seu fluxo com um vizinho, relação ou parceiro escolhido pelo campo |
| **DANÇA** | o corpo recebe/chaveia movimento espacial e sua consequência sonora |
| **RASGO / VESTÍGIO** | recorte, stutter, deslocamento temporal, memória, cicatriz ou reparo entram com duração e retorno |

As colunas exibem nomes reais — `Acid`, `Theremin`, `Galo` etc. — e acompanham
a coorte escolhida pela seed. Um corpo em reserva aparece como reserva e não
aceita clique. Cada célula mostra campo, alvo, nível/estado e consequência
prevista. Ao entrar, produz uma confirmação sonora curta; depois continua como
força disponível para a narrativa. O segundo clique reduz ou desliga a mesma
força, sem trocar secretamente de função.

A matriz permanece suspensa enquanto as mesmas funções não estiverem provadas
por gesto direto no campo. Depois poderá oferecer acesso secundário. Ela exige
um estado bounded próprio por corpo e reutiliza adapters, stems, relações,
movimento, colagem, filas e telemetria; não cria 16 engines paralelas.

## Colagem, mashup e recorte

A estética transversal do Performance inclui recombinação de histórias,
stutter, tempo shifts, paradas, silêncios, glitch, fita, ruído, contraste de
matérias e transições abruptas. Essa gramática deve permitir choque entre
percussão, baixos, vozes, tuba/metais, theremin, piano, synths, guitarras e
matérias futuras, mas não obriga todos os elementos a tocar simultaneamente.

A seed secundária pode enxertar traços na principal sem trocar território,
coorte ou campo.
Recortes são acontecimentos sobre stems existentes, com alvo e retorno. A
colagem é composição por fragmentação, contaminação e reaparição; não é um
efeito global opaco.

## Abertura, continuidade, coda e epílogo

Quando o músico interrompe a manipulação, a música continua por narrativas de
corpo, relação e campo; não reinicia nem cai em loop curto. Se a inatividade se
prolonga até o prazo definido pela seed/sistema, começa um epílogo generativo.
O músico também pode chamar o fim antes ou adiar o fim automático ao retornar.

Toda performance iniciada é gravada desde a abertura escolhida pelo músico. A
coda lê a trajetória vivida e conduz a faixa ao silêncio por dispersão,
retorno, implosão, cicatriz, última colisão, corte ou coda plural. A gravação só
termina depois da cauda segura; o epílogo técnico fecha áudio e trace. O campo
não é apagado. `STOP` é emergência e salva o take como interrompido.

## Estado atual comparado ao programa

| Eixo | O que existe | O que falta para ser considerado performático |
|---|---|---|
| Seed | coortes, identidade, trajetória, seed viva coletiva e por corpo, turnos na pausa | seed viva por relação, polifonia, descendência, fixação, retorno e desenvolvimento reconhecível |
| Módulos | 14 adapters catalogados; oito corpos/stems iniciais audíveis, cada um com inspector contextual | validar ergonomia/escuta do inspector e ampliar escolha/acumulação livre por escuta |
| Campo | posições preservadas, inserção/recolhimento, relações e transformação coletiva | profundidade, forças, topologias livres e causalidade espacial mais rica |
| Movimento | `CAMPO` transforma coletivamente Órbita/Pulso/Figura-8; `DANÇA` atua no corpo selecionado com sentido reversível; áudio, relações e interface usam posição viva | combinações, fases próprias, forças intencionais e contraste humano comprovado |
| Relações | pressões de tempo, matéria, espaço, memória e primeiro cross-colour | simbiose por par, resistência, atraso, impregnação e vestígio |
| Narrativa | eventos determinísticos após pausa | evitar ciclo curto e usar memória/ações anteriores para mudar o futuro |
| Colagem | segunda seed, MIX, Stutter e Tempo Shift | resposta imediata clara, mais fluxos e integração à matriz |
| 16 células | estrutura visual/código legado suspenso | contrato de campos por corpo e uma linha completa aceita antes de reexibir |
| Controles | filas realtime e muitas rotas DSP | eliminar fantasmas, calibrar contraste e separar gesto de preparação |

## Critério de pronto de um controle

Um controle criativo só entra na superfície quando:

1. possui verbo e alvo visíveis;
2. declara se é gesto, estado ou preparação;
3. alcança um stem, relação ou campo realmente existente;
4. produz resposta reconhecível em contexto musical, não só diferença numérica;
5. preserva a cartografia salvo gesto espacial explícito;
6. mostra entrada, atuação e retorno;
7. possui limite realtime e falha visível;
8. passa escuta A/B humana com o restante da música tocando.

Controles operacionais — `INICIAR`, indicador `GRAVANDO`, captura, modo de tela
e segurança — não precisam alterar o som como gesto criativo, mas precisam
declarar claramente seu estado e não disputar identidade visual com controles
musicais.

## Ordem de implementação vinculante

1. Auditar no Studio/core os módulos e estratégias canônicas necessários à
   primeira seed de prova, registrando fonte, comportamento, controles e forma
   de adaptação ao CAMPO.
2. Fazer a seed abrir um campo musical completo com módulos necessários
   audíveis, stems, papéis, relações e estratégia de jogo causal; expor em
   reserva os complementares já adaptados e pertinentes.
3. Suspender da superfície criativa os controles sem alvo e consequência
   reconhecíveis; preservar o código como matéria a readaptar.
4. **Concluído em 2026-09-03:** unificar geometria visual e musical: a
   constelação coletiva e seus desvios locais determinam distâncias, relações e
   consequências sonoras; o desenho-base guarda a configuração autoral.
5. Criar o contrato puro por corpo: âncora, profundidade, dança, seed de origem,
   descendência viva, intensidade, fase, retorno e memória.
6. Implementar uma prova vertical com **ÓRBITA**, **PULSO** e **DERIVA** por
   corpo, mais rotação do conjunto em eixo 2D/3D, sempre audíveis.
7. Implementar relação por par com verbo, fluxo, permeabilidade, resistência e
   A/B próximo/intermediário/distante.
8. Implementar `ORIGEM ↔ VARIAÇÃO` no corpo selecionado e uma segunda seed
   como núcleo de influência, sem trocar módulos ou campo.
9. Provar continuidade narrativa sem a mão e um epílogo que encerre a faixa a
   partir da história vivida, preservando campo e trace.
10. Só depois reavaliar a matriz 4×4 como acesso secundário a funções aprovadas;
   nenhuma célula inerte ou corpo em reserva volta à superfície.

## Fontes preservadas

- `ESTRATEGIA_MUSICAL.md`: tese, seed como patch vivo, tríade de autoria,
  nutrição, papéis, relações e escalas de intervenção;
- `MESA_RELACIONAL_DESIGN.md`: unidade de módulo, distância, simbiose e
  preservação do campo;
- `MODELO_JOGO_SEED_CAMPO.md`: ciclo causal e narrativa durante a pausa;
- `MAPA_CONCEITOS_JOGO.md`: movimento próprio, atração/repulsão, blefe,
  disputa, colapso e referências do Studio/Core;
- `CONTRATO_CELULAS_PERFORMATIVAS.md`: diagnóstico da grade e invariantes;
- `DIRECAO_SONORA_COLAGEM_2026-09-02.md`: recorte, mashup, caos legível e
  materiais sonoros desejados;
- `PAINEL_PERFORMANCE.md`: genealogia visual, ergonomia e operação de palco;
- `GRAMATICA_DO_CAMPO_PERFORMATIVO.md`: tridimensionalidade, movimentos do
  corpo e do conjunto, conexões, fluxos, polifonia de seeds, narrativa,
  epílogo, auditoria dos controles e primeira prova vertical;
- `AUDITORIA_STUDIO_MODULOS_E_JOGO_2026-09-02.md`: fontes executáveis do
  Studio/core para módulos, `GamePlan`, agência, relações, arco e coda; limite
  do manifesto atual e sequência P0 de reaproveitamento;
- `CONTINUIDADE.md`: decisões, correções, testes e limites acumulados.

Quando houver divergência, este programa define a prioridade criativa atual;
as fontes continuam canônicas para detalhes, genealogia e proveniência.
