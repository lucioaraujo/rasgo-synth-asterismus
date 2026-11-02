# Auditoria de traduções — 2026-09-07

## Objetivo e escopo

Revisar a superfície textual do host JUCE do Rasgo Synth Performance depois da
correção de cenas, da abertura do quarto módulo e da revisão do Tutorial. A
auditoria cobre textos visíveis, tooltips, diálogos e retornos de estado em
`src/app/PerformanceApp.cpp`; não trata nomes próprios RASGO, siglas musicais
ou identificadores técnicos que devem permanecer estáveis.

Foram localizadas **199 ocorrências** de produção ou desenho de texto de
interface. A mudança de idioma agora altera **Sobre**, **Tutorial**, a camada
operacional P0 e uma parcela dos retornos dinâmicos; as zonas P1/P2 ainda
precisam de revisão visual e semântica.

## Correções realizadas nesta revisão

- Tutorial em PT/EN/FR/ES revisto para usar `Campo`/`Field`/`Champ`/`Campo`,
  preservar o desenho do performer e explicar que valores iniciais dos módulos
  pertencem à seed.
- Tutorial corrigido: `ABERTURA`, `ESPIRAL`, `RASGO` e `BALANÇO` transformam a
  peça sem interromper um `PLAY` ativo.
- Tutorial corrigido: `REC/CODA` abre a tomada e pede o final composto;
  `STOP` é interrupção técnica, não coda.
- Tutorial atualizado em PT/EN/FR/ES para declarar a orquestração atual: até
  oito módulos internos retornam por quatro stems audíveis; as lanes 5–8 são
  camadas internas e não novos canais de saída.
- Janela de Tutorial agora traduz título, subtítulo e botão de fechamento.
- Criado o gateway único `uiText(PT, EN, FR, ES)` para o host, aplicado ao
  cabeçalho, transporte, REC/CODA, seed, captura/desfazer, cenas, gestos
  focados, títulos de zonas e caixas Sobre/Tutorial.
- Retornos imediatos de cenas, captura, gestos, transporte, gravação, seed,
  tempo, presença, relações, controles locais, stems e macros passaram a
  acompanhar o idioma. Nomes musicais de identidade (`RASGO`, `ALEA`, `CODA`,
  `BPM`, `MUTE`, `SOLO`) permanecem estáveis.
- O gráfico do Campo agora traduz os seus quatro eixos; os comandos visíveis
  da Constelação e suas instruções de navegação também seguem o idioma ativo.
  Tooltips de presença, stem, mute e solo foram migrados.
- Rótulos e explicações de macro, controles locais derivados da coorte e os
  16 pares relacionais agora são recompostos no idioma ativo sem recarregar a
  seed, reiniciar transporte, apagar o desenho do Campo ou perder o foco.
- A biblioteca de módulos e as funções de cada corpo da Constelação foram
  localizadas nas quatro línguas; nomes próprios de módulos continuam iguais.
- Fenômenos relacionais, tipos de movimento, vida/prontidão de stems,
  trajetória, alertas de CPU/XRUN e rodapé de áudio agora traduzem a camada
  humana. `CPU`, `PEAK`, `XRUN`, `dB`, `BPM`, `WAV`, `REC`, `CODA`, `MUTE` e
  `SOLO` permanecem estáveis como leitura técnica ou vocabulário do instrumento.
- Mensagens dinâmicas de macro, a reserva do quarto corpo, o controle por célula
  e o caso de lista sem corpos ativos agora também passam pelo gateway PT/EN/FR/ES.

## Inventário de pendências

| Prioridade | Área | Estado atual | Necessidade |
|---|---|---|---|
| P0 | Cabeçalho e transporte | Localizados | Revisar visualmente os quatro idiomas; preservar siglas universais como `REC`, `CODA`, `BPM`, `MUTE` e `SOLO`. |
| P0 | Erros e última ação | Caminhos críticos e macros localizados | Migrar os poucos retornos de Campo/Constelação ainda compostos diretamente; não compor frases traduzidas no callback. |
| P0 | Cenas e gestos | Explicações, retorno e ações focadas localizados | Manter os nomes performativos portugueses como vocabulário RASGO ou apresentar equivalência no tutorial. |
| P1 | Campo e Constelação | Eixos, comandos, biblioteca, papéis, relações, fenômenos e movimento localizados | Inspeção visual nos quatro idiomas. |
| P1 | Campo de Matérias | Macros, tooltips de presença/mix/mute/solo e controles derivados da coorte localizados | Inspeção visual; manter nomes de módulos e labels de DSP como identidade técnica/musical. |
| P1 | Diálogos e menus | Localizados | Inspeção visual dos itens auxiliares. |
| P2 | Rodapé, telemetria e status | Localizados | Inspeção visual do cabeçalho em largura mínima e monitor de ação. |

## Regra de implementação

Criar uma camada única de vocabulário de interface por idioma, com chaves
semânticas. O host não deve espalhar condicionais `PT/EN/FR/ES` nem traduzir
frases em tempo real. Rótulos estáveis de performance podem permanecer como
nomes RASGO (`ABERTURA`, `ESPIRAL`, `RASGO`, `BALANÇO`, `ALEA`, `CODA`), desde
que o Tutorial ofereça sua função no idioma escolhido.

Toda chave nova deve ter as quatro versões PT, EN, FR e ES. A validação mínima
é abrir cada idioma, conferir ausência de `Â`/mojibake, largura sem sobreposição
e confirmar que nenhuma mensagem de falha retorna para português em EN/FR/ES.

## Próximo passo

Abrir o host nos quatro idiomas e confirmar ausência de mojibake, truncamento
ou sobreposição. A revisão de código está concluída; essa validação é visual e
não deve alterar a semântica musical nem criar novos controles.
