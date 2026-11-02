# Contrato das 16 células performativas

## Problema reconhecido

As 16 células atuais misturam quatro cenas, quatro gestos, seis macros e duas
ações de segurança. A mesma superfície alterna recall, evento momentâneo,
setpoint e reset. O resultado é pouco legível: o músico não sabe exatamente o
que será alterado, pode perder uma decisão espacial recém-desenhada e não tem
um desfazer geral para uma ação destrutiva.

Essa semântica fica congelada para revisão. Não serão adicionadas novas ações à
grade atual até a nova matriz ser implementada e testada.

## Conceito substituto: Matriz de Campos de Acontecimento — 2026-09-02

As 16 células continuam como uma estrutura 4×4, mas deixam de ser atalhos
heterogêneos. Elas passam a ser uma **Matriz de Campos de Acontecimento**:
quatro campos generativos agem sobre os quatro corpos que a seed preparou na
mesa. Uma célula não cria uma identidade fora da seed nem reconfigura a mesa;
ela liga, desliga ou intensifica uma força situada, com estado e consequência
sonora visíveis.

| Linha / campo | Pergunta musical | Consequência sobre o corpo da coluna |
|---|---|---|
| **VARIAÇÃO** | como a seed continua sem repetir? | altera frase, densidade, matéria e cadência de forma determinística; dá uma prévia curta ao entrar |
| **RECOMBINAÇÃO** | com quem este fluxo se mistura? | escolhe/fortalece uma travessia com corpo ligado ou próximo; cria simbiose, interferência ou contraste audíveis |
| **DANÇA** | como este corpo habita o espaço? | chaveia movimento próprio — órbita, pulso, figura-8 e futuras rotas — com consequência rítmica/espacial no stem, sem escrever a âncora |
| **RASGO / VESTÍGIO** | que marca de colagem permanece? | introduz recorte, stutter, tempo deslocado, memória ou reparo limitado no corpo, com retorno declarado |

As colunas mostram os nomes dos corpos reais da seed, não `1–4`; um quarto corpo
em reserva aparece como indisponível, não como um botão inerte. Cada célula
precisa exibir `campo · alvo · intensidade/estado`, emitir uma prévia curta no
clique e continuar como uma influência observável na narrativa da seed. O
segundo clique somente reduz/desliga esse mesmo campo; não pode assumir outra
função escondida.

Essa matriz reutiliza filas de gesto, controles de módulo, relações, movimento,
colagem, telemetria e stems já existentes, mas exige uma nova camada explícita
de estado por corpo antes de voltar à superfície.

## Diagnóstico de origem

As células pertencem a uma camada legada do protótipo: foram criadas antes de
existirem módulos adaptados, posições persistentes, relações espaciais,
presença por stem e memória de sessão. Naquele momento, uma célula podia
funcionar como atalho para uma cena ou gesto global. Com a mesa modular atual,
essa premissa deixou de ser válida. O módulo e seu campo são agora a unidade
primária; a célula será apenas uma rota performática explícita para atuar sobre
essa unidade.

Não se deve ampliar, renomear superficialmente ou adicionar mais efeitos à
grade legada. Ela será substituída depois que os contratos dos módulos, do
campo e dos checkpoints estiverem definidos.

## Módulos e generatividade da seed

Os módulos não são peças neutras que aguardam um botão. Cada módulo foi criado
no Studio com uma intenção composicional e participa da realização generativa
da seed. Esse princípio deve continuar no Performance:

`seed → território → coorte → parâmetros do módulo → relações → áudio`

O `PerformanceModuleCohortAdapter` já realiza uma primeira versão desse fluxo:
a seed escolhe a coorte, configura os corpos, separa as fontes determinísticas
e alimenta os stems. A reconstrução das células não pode interromper esse
processo, trocar a identidade do módulo por um preset genérico ou gerar um
evento sem relação com a matéria que está soando.

No novo contrato, uma célula atua sobre um módulo ou relação já existente: não
cria módulo paralelo, não sorteia identidade fora da seed, não apaga posição,
presença ou história e sempre mostra alvo e transformação.

## Geração autônoma durante a pausa do gesto

O Performance é um instrumento generativo espacial. A pausa momentânea do
performer não significa silêncio composicional, mas também não autoriza o
motor a abandonar o campo que o músico desenhou. A seed, os módulos, suas
relações e a memória devem continuar gerando acontecimentos dentro das
posições, rotas, focos e espacialidades vigentes na mesa.

Nesse intervalo, a narrativa emergente assume temporariamente a condução:
varia densidade, tensão e matéria, repete com diferença, acumula memória e
produz consequências. Quando o performer retorna, encontra o mesmo campo
espacial transformado pelos acontecimentos e pode intervir sobre ele. A
narrativa atua no lugar do gesto por um momento; não atua no lugar da autoria
espacial do músico.

Essa relação é uma conversa. A seed deve ler as posições, distâncias, focos e
relações escolhidos pelo performer e responder musicalmente a eles. A resposta
pode aproximar, separar, contaminar, contrariar ou fazer um módulo responder a
outro; depois, o músico escuta essa consequência e decide como remodelar o
campo. Assim, espaço e narrativa se influenciam sem que a geração redesenhe a
mesa ou que a mesa vire apenas um painel decorativo.

Essa conversa acontece pelos fluxos e acontecimentos propostos pelo jogo da
seed. A seed oferece regras de transformação, papéis, afinidades, resistências,
riscos e memórias; o campo espacial define as condições em que esses elementos
se encontram. Uma geração válida não é acaso sem leitura: é uma jogada situada
no campo atual, com consequência observável e possibilidade de resposta do
performer.

Qualquer trajetória de acontecimentos deve ser derivada dos mecanismos
generativos canônicos do Studio e dos estados reais dos módulos. O componente
experimental `PerformanceSeedTrajectory` não é, por si só, a arquitetura da
geração: não deve virar uma lista fixa de presets nem conduzir o Performance
fora do campo espacial.

Regra de continuidade: ao deixar o app tocando sem manipulação, deve ser
possível ouvir desenvolvimento, contraste e transformação reconhecíveis sem
perda da organização espacial desenhada. Ao comparar dois campos espaciais
distintos para a mesma seed, a narrativa deve responder de modo distinto e
audível. Se a saída repetir um ciclo curto sem evolução, ignorar o campo ou
redesenhar/substituir a mesa, a implementação está incompleta, mesmo que o
áudio permaneça estável.

## Princípios invioláveis

1. **O campo é do músico.** Nenhuma célula, gesto, macro ou recall pode alterar
   posição, rota ou organização espacial. Só uma ação espacial explícita pode
   fazê-lo.
2. **Seed não é sorteio oculto.** A seed cria o território inicial. Uma célula
   não pode gerar uma variação imprevisível nem consumir aleatoriedade global.
   Toda variação deve ser determinística, nomeada e repetível.
3. **Toda célula anuncia seu efeito.** O rótulo e o estado visual devem mostrar
   ação, alvo e magnitude antes e depois do clique. Tooltip não é a única
   explicação.
4. **Toda alteração é uma transação.** Antes de aplicar uma célula, o
   Performance guarda um checkpoint bounded do estado musical. O músico pode
   desfazer a última ação sem depender de reconstruir a cena.
5. **Ação momentânea não se disfarça de estado.** Um evento como Mutate ou
   Rasgar mostra duração/intensidade e seu estado (`PREPARA`, `ATUA`, `RECOLHE`),
   em vez de parecer um preset permanente.
6. **Recall é explícito e limitado.** Uma cena pode morphar macros, presença e
   relações, mas preserva o campo espacial vivo. O botão deve indicar origem,
   destino e tempo de morph.
7. **Segurança não é composição.** Silêncio, Panic Safe e Undo têm estado e
   retorno próprios; não compartilham a mesma aparência das ações musicais.

## Matriz 4×4 superada

O esboço anterior de estados/gestos/campo/saída foi substituído pela Matriz de
Campos de Acontecimento acima. Ele fica como histórico de transição, não como
modelo de interface.

| Linha | Função | Regra das quatro células |
|---|---|---|
| 1 | **ESTADOS** | quatro snapshots explícitos; preservam o campo e mostram `A → B` |
| 2 | **GESTOS** | Mutate, Rasgar, Recall e Hold com intensidade/duração declaradas e desfazível |
| 3 | **CAMPO** | quatro variações de relação/macros, sempre como setpoints visíveis, sem sorteio |
| 4 | **SAÍDA** | Silêncio, Panic Safe, Undo e uma ação de retorno/continuidade claramente nomeada |

A composição exata da linha CAMPO será definida a partir dos controles já
existentes, sem duplicar sliders. A célula deve chamar o mesmo contrato do
slider, não criar um segundo estado escondido.

## Comportamento de uma célula

Cada célula terá um descritor bounded com: `id`, família, nome, alvo, valor
previsto, unidade visual, duração/morph, reversibilidade e origem da seed. O
fluxo será:

`previsão → clique → checkpoint → comando enfileirado → consequência → estado
visível → desfazer`

O segundo clique não pode ter significado diferente sem indicação visual. Para
ações alternáveis, o rótulo muda (`HOLD`/`RELEASE`, `ATIVA`/`RECOLHE`). Para
ações momentâneas, o segundo clique repete a mesma ação determinística e a
ação `UNDO` desfaz a última transação; não haverá “sorteio novo” implícito.

## Ordem de implementação

1. Criar contrato puro e teste dos 16 descritores.
2. Generalizar o checkpoint hoje usado apenas para captura de cena para a
   última transação de célula/gesto/recall.
3. Remover a aleatoriedade de sistema do `MUTATE`; derivar seu token de
   `territorySeed + cellId + actionSerial`.
4. Fazer o recall preservar campo, presença espacial e rotas, alterando apenas
   o que o descritor declarar.
5. Implementar estados visuais e texto de consequência antes de reativar a
   grade.
6. Testar sequência `desenhar campo → célula → gesto → recall → undo`, incluindo
   determinismo, preservação de posição, reversão e Panic Safe.
7. Fazer escuta humana da nova matriz antes de considerar a superfície pronta.

Até a conclusão, a grade atual é protótipo técnico e não deve ser tratada como
interface performática final.
