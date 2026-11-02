# Direção sonora — colagem punk aleatória — 2026-09-02

## Função deste documento

Este é o briefing musical para a próxima camada do Rasgo Synth Performance.
Ele condensa referências fornecidas pelo músico: punk sujo e instável,
percussão saturada, baixo fuzz, guitarras rasgadas, tuba e metais em conflito,
theremin, piano vertical detonado, synth lo-fi/8-bit, glitch, fita, ruídos,
chamados animais/galo, vozes scat gritadas e coro SATB. Não é uma lista de
assets obrigatórios nem autorização para colocar todos os elementos ao mesmo
tempo.

## Gramática performática

| Vetor | Realização desejada | Limite de implementação |
|---|---|---|
| Corpo | bateria angular, baixo fuzz, guitarras/tuba/metais em atrito | stems existentes e adapters auditados; não criar massa sonora falsa |
| Recorte | glitch, fita, synth curto, piano, theremin, voz ou ruído | entra como acontecimento bounded sobre um stem, com saída e retorno explícitos |
| Tempo | stutter, deslocamento de pulso, métrica solta, paradas | nunca mudar a posição da mesa; duração quantizada/limitada e recuperação sem click |
| Colagem | duas histórias de seed se contaminam | misturar traços estruturais, não números de seed; seed principal continua dona da coorte e da mesa |
| Orquestra | big band caótica, klezmer, samba/baião, surf-western, gospel quebrado | contraste e disputa entre matérias, não uma etiqueta de gênero aplicada ao mix |
| Voz | scat, grito, gagueira rítmica, coro | matéria performativa futura; não simular linguagem ou patologizar uma pessoa real |
| Forma | explosão, corte seco, vazio, reaparição e coda disforme | o silêncio é gesto composicional reversível, não falha de áudio |

## Prioridade de implementação

1. `STUTTER`: implementado como janela rítmica curta, limitada e suavizada
   no stem em foco ou no coletivo; falta escuta humana de duração e intensidade.
2. `TEMPO SHIFT`: implementado como deslocamento temporário de fase/pulso e
   cor do stem, sem alterar o BPM armazenado nem reconfigurar a mesa; falta
   escuta humana de contraste e retorno.
3. `COLAGEM`: roleta de seed secundária + mixer contínuo estão expostos na
   MESA; os recortes da seed secundária colorem acontecimentos, não substituem
   corpos. Falta escuta humana da sua faixa de mistura.
4. Expandir matérias (tuba, theremin, brass, piano, voz, ruído) somente após
   adapter, stem, controle causal, limite realtime e escuta humana.

## Critério musical

O resultado deve soar vivo, impreciso e perigosamente mutável, mas legível:
cada entrada tem causa, cada corte deixa espaço, cada recorte pode ser retirado
e o performer continua autor do campo espacial. "Caos" não justifica
repetição mecânica, acúmulo contínuo ou perda de controle.

## Realização inicial dos recortes — 2026-09-02

`STUTTER` e `TEMPO SHIFT` entram pela mesma fila realtime bounded dos gestos
da Performance. A interface os mostra como dois verbos próprios na faixa de
gestos (`S` e `D`), e não como células aleatórias: o nó selecionado é o alvo;
sem nó, o gesto atua no conjunto. Ambos têm duração máxima de 0,8 s e retorno
suavizado. `STUTTER` porta/reitera a matéria; `TEMPO SHIFT` muda fase, pulso e
uma pequena cor de frequência do corpo durante esse intervalo. Nenhum cria
stem, altera o teto de quatro corpos, move coordenadas, chama cena ou escreve o
BPM da sessão.

A prova offline A/B confirma mudança de áudio, preservação exata de duas
posições autorais e BPM persistido. Ela não substitui a audição humana nem fixa
uma escolha estética de duração, intensidade ou alvo.

## Superfície de colagem — 2026-09-02

A MESA agora possui uma faixa exclusiva `COLAGEM` acima do campo XY: editor da
seed secundária, `RODAR` e fader `MIX`. `RODAR` escolhe somente a fonte da
trajetória secundária; não chama `ALEA`, não reinicia território, não troca a
coorte e não redesenha a mesa. `MIX 0` preserva integralmente a história da
seed principal; `MIX 1` deixa a secundária fornecer seus traços estruturais.
Valores intermediários interpolam densidade, tensão, matéria e memória, com
regra discreta delimitada na trajetória.

Ao abrir novo território, o host instala a seed principal e reaplica a escolha
de colagem depois da transação, se `MIX` estiver acima de zero. Assim, a
colagem é uma decisão contínua do músico, não um recall escondido. A sequência
compilou no host e a suíte técnica passou; ergonomia e resultado musical ainda
dependem de teste humano.
