# Arquivística e preservação — Rasgo Synth Performance

Data da atualização de localização: 2026-10-11; fotografia técnica abaixo
preservada de2026-08-30, não revalidada integralmente.
**Estado:** arquivo de trabalho ativo; ainda não é pacote de preservação nem
release congelada.

## Identidade e fonte canônica

Este diretório é a fonte canônica do projeto `Rasgo Synth Performance`:

- código, headers, testes, CMake, preset e documentação pertencem ao projeto;
- o Studio/Core são fontes de origem; os packages consumidos pelo build
  estão copiados localmente em `vendor/studio-dsp/`, com proveniência;
- `README.md`, `TAREFAS.md`, `docs/` e `../CONTINUIDADE.md` (dentro deste projeto)
  formam o registro de
  estado e retomada;
- `LICENSE`, `THIRD_PARTY_NOTICES.md` e `assets/README.md` registram licença,
  créditos e proveniência conhecidos.

Não há autorização para tratar o binário, o build, o SQLite, uma publicação ou
um backup como substituto dos fontes deste diretório.

## Classificação dos materiais

| Material | Classe | Tratamento atual |
|---|---|---|
| código, headers, CMake, testes e documentação | B — fonte de projeto | preservar com histórico Git e continuidade |
| `LICENSE`, avisos, fontes e logo | B/C — fonte e release | preservar com créditos, licenças e hashes quando congelado |
| binário Release em `build-app/` | E — derivado regenerável | fora do Git; não é fonte canônica |
| builds, caches e dependências baixáveis | E — regenerável | fora do Git conforme `.gitignore` |
| WAVs em `Música/Rasgo Synth Performance/recordings` | A/D conforme conteúdo | fora deste diretório; classificar e copiar separadamente |
| testes, benchmarks e logs de validação | D — pesquisa/evidência | preservar quando sustentarem uma decisão ou release |

Os recordings do usuário não devem ser apagados pelo projeto. Um take de teste
não deve receber a mesma retenção de um master; a classificação precisa ser
registrada no manifesto correspondente.

## Situação verificada

- fonte de trabalho presente e legível;
- `LICENSE`, README, avisos de terceiros, preset CMake e continuidade presentes;
- packages locais referenciados pelo preset em `vendor/studio-dsp/cmake`;
- `.gitignore` exclui builds, caches, áudios e dumps regeneráveis;
- `check_runtime_artifacts.sh` verifica resíduos sem remover arquivos;
- build Release pelo preset canônico concluído;
- 12/12 testes funcionais e estabilidade em 41,28 s passaram na fotografia
  técnica mais recente;
- o estado Git do projeto permanece modificado: 39 entradas no diretório do
  Performance, incluindo 15 arquivos ainda não rastreados; o último commit
  visível é anterior a esta fotografia. Portanto, o estado atual não está
  congelado nem deve ser chamado de release preservada.

## Lacunas arquivísticas

Ainda faltam, para um marco de preservação:

1. recorte explícito da versão/marco e lista de arquivos incluídos;
2. manifesto legível com `archive_id`, origem, versão, direitos e exclusões;
3. checksum do pacote de fontes e dos assets/release selecionados;
4. cópia de preservação fora da árvore de trabalho, além do snapshot geral do
   container;
5. restauração de teste em diretório separado, com reconstrução do host;
6. classificação dos recordings e evidências humanas de escuta/visual;
7. decisão explícita sobre quais arquivos não rastreados pertencem ao marco.

Essas lacunas não impedem o desenvolvimento. Impedem apenas declarar este
estado como pacote arquivístico fechado.

## Procedimento de fechamento de marco

Antes de declarar uma versão preservada:

1. parar processos do projeto e confirmar volume gravável;
2. executar `./check_runtime_artifacts.sh`;
3. revisar `git status`, fontes, assets, licenças e arquivos não rastreados;
4. separar fonte canônica, derivado, evidência e material de usuário;
5. gerar manifesto e checksums sem alterar os originais;
6. copiar o recorte para a camada de preservação definida pelo container;
7. restaurar em local temporário e executar build/testes;
8. registrar data, ferramenta, resultado, limitações e responsável em
   `CONTINUIDADE.md` na raiz do projeto.

Nenhuma limpeza ou exclusão é autorizada apenas porque um arquivo está fora do
Git. Expurgo exige alvo explícito, cópia íntegra verificada e registro.

## Próximo passo arquivístico

Não congelar ainda. Primeiro concluir a validação visual/áudio do host e
decidir quais mudanças atuais compõem o próximo marco. Depois criar um pacote
de preservação do projeto, com manifesto, checksums e restauração demonstrada.
