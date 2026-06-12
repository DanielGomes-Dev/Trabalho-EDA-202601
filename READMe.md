# Sistema de Gestão de Filmes — Árvore B em Memória Secundária

Trabalho computacional de **Estruturas de Dados e Seus Algoritmos** — UFF 2026.1

---

## Sumário

1. [Visão geral](#visão-geral)
2. [Estrutura de arquivos](#estrutura-de-arquivos)
3. [Como compilar e executar](#como-compilar-e-executar)
4. [Arquitetura da solução](#arquitetura-da-solução)
5. [Módulos em detalhe](#módulos-em-detalhe)
6. [A Árvore B: como funciona](#a-árvore-b-como-funciona)
7. [Como os dados chegam ao disco](#como-os-dados-chegam-ao-disco)
8. [Consultas disponíveis](#consultas-disponíveis)
9. [O que ainda precisa ser implementado](#o-que-ainda-precisa-ser-implementado)
10. [Convenções de código](#convenções-de-código)

---

## Visão geral

O programa lê dois arquivos de texto (`Nodes.txt` e `Relationships.txt`) que
descrevem filmes, pessoas (atores, diretores, produtores, escritores) e os
relacionamentos entre eles.  Essas informações são indexadas em três **Árvores B**
armazenadas em memória secundária (arquivos binários `.dat`).

Cada **nó folha** da árvore é também gravado em um **arquivo binário individual**
(`folha_NNN.bin`), simulando o comportamento de um SGBD real que mantém páginas
de dados separadas das páginas de índice.

---

## Estrutura de arquivos

```
projeto/
│
├── main.c                          ← Ponto de entrada; apenas inicializa e delega
├── Makefile                        ← Compila, limpa e executa o projeto
├── README.md                       ← Esta documentação
│
├── files/
│   ├── Nodes.txt                   ← Dados de filmes e pessoas
│   └── Relationships.txt           ← Relacionamentos pessoa ↔ filme
│
└── src/
    ├── core/
    │   ├── btree/
    │   │   ├── btree.h             ← Interface pública da Árvore B
    │   │   └── btree.c             ← Implementação (disco, busca, inserção, impressão)
    │   └── hash/
    │       ├── hash.h              ← Interface da função de hash
    │       └── hash.c              ← Algoritmo djb2 + truncagem para int
    │
    ├── domain/
    │   ├── movie/
    │   │   ├── movie.h             ← Tipo TMovie
    │   │   └── movie.c             ← Construtor e impressão
    │   ├── person/
    │   │   ├── person.h            ← Tipo TPerson
    │   │   └── person.c            ← Construtor e impressão
    │   └── relation/
    │       ├── relation.h          ← Tipo TRelation + enum TPapel
    │       └── relation.c          ← Construtor, mapeamento de papel e impressão
    │
    ├── io/
    │   ├── db.h                    ← Handle TDB: agrupa as 3 árvores
    │   ├── db.c                    ← Abre/fecha banco, inserir/buscar/remover
    │   ├── file_reader/
    │   │   ├── file_reader.h       ← Interface de leitura de arquivos
    │   │   └── file_reader.c       ← Parsers de Nodes.txt e Relationships.txt
    │   └── menu/
    │       ├── menu.h              ← Interface do menu interativo
    │       └── menu.c              ← Menu principal, sub-menu de operações e consultas
    │
    ├── query/
    │   ├── query.h                 ← Assinaturas das 20 consultas (a–t)
    │   └── query.c                 ← Implementações (stubs prontos para completar)
    │
    └── utils/
        ├── string_utils.h          ← Utilitários de string
        └── string_utils.c          ← Extração de campo, trim, detecção de linha vazia
```

---

## Como compilar e executar

### Pré-requisitos

- GCC (qualquer versão ≥ 7)
- Make

### Compilar

```bash
make
```

### Executar

```bash
./filmes
# ou simplesmente:
make run
```

### Limpar arquivos gerados

```bash
make clean
```

Isso remove os `.o`, o executável e todos os arquivos `.bin` e `.dat` gerados.

---

## Arquitetura da solução

O projeto foi refatorado seguindo o **Princípio da Responsabilidade Única (SRP)**:
cada arquivo tem uma razão para existir e não faz mais do que o seu nome diz.

```
main.c
  └── inicializa o banco (db_abrir)
  └── carrega os dados (file_reader_carregar_tudo)
  └── entrega o controle (menu_principal)
        ├── menu_operacoes   → insere, remove, busca, imprime árvore
        └── menu_consultas   → consultas (a) a (t)
              └── query_X()  → cada consulta em função própria
```

O fluxo de dados é:

```
Nodes.txt / Relationships.txt
    ↓ file_reader (parseia linha por linha)
    ↓ domain (cria TMovie / TPerson / TRelation)
    ↓ hash (gera chave int a partir do ID)
    ↓ db (insere na árvore correta)
    ↓ btree (grava nó no arquivo .dat e folha_NNN.bin)
```

---

## Módulos em detalhe

### `src/core/btree/`

O coração do projeto. Implementa a **Árvore B+** em memória secundária.

| Função | O que faz |
|---|---|
| `btree_abrir()` | Abre (ou cria) o arquivo `.dat` de índice |
| `btree_fechar()` | Fecha o arquivo e libera memória |
| `btree_no_criar()` | Grava um novo nó zerado no final do `.dat` |
| `btree_no_ler()` | Lê um nó pelo seu offset em bytes |
| `btree_no_escrever()` | Grava um nó no seu offset |
| `btree_folha_salvar()` | Grava o nó folha em `folha_NNN.bin` |
| `btree_folha_carregar()` | Lê o nó folha de `folha_NNN.bin` |
| `btree_inserir()` | Inserção com split automático |
| `btree_buscar()` | Busca recursiva pela chave |
| `btree_remover()` | **TODO** — ainda não implementado |
| `btree_imprimir()` | Imprime a árvore rotacionada (raiz no topo) |
| `btree_imprimir_chaves_em_ordem()` | Percorre as folhas encadeadas |

**Estrutura do nó em disco** (`BTreeNo`):

```c
typedef struct {
    int  nchaves;               // Quantidade de chaves no nó
    int  folha;                 // 1 = folha, 0 = interno
    int  id_folha;              // ID do arquivo folha_NNN.bin
    long prox_folha;            // Encadeamento da lista de folhas
    int  chave[100];            // Vetor estático de chaves
    long filho[101];            // Offsets dos filhos no .dat
} BTreeNo;
```

### `src/core/hash/`

Isola completamente a geração de chaves. Usa o algoritmo **djb2** e trunca o
resultado para `int` via módulo com o primo de Mersenne 2³¹−1.

### `src/domain/`

Três tipos de dados puros, sem dependência de nada além da stdlib:

- **`TMovie`** — id (hash do título), título, ano, tagline
- **`TPerson`** — id (hash do nome), nome, ano de nascimento
- **`TRelation`** — id_pessoa, id_filme, papel (enum + string), info adicional

O papel é representado tanto como `enum TPapel` (para comparação eficiente)
quanto como `char papel_str[]` (para exibição).

### `src/io/db.h` + `db.c`

O **handle central** (`TDB`) agrupa as três árvores abertas.  Toda operação de
inserção, busca e remoção passa por aqui, que converte o ID de domínio para chave
`int` antes de chamar a árvore.

### `src/io/file_reader/`

Responsável exclusivamente por ler os arquivos de entrada.  Organizado em camadas:

1. `_parsear_linha_movie()` — interpreta uma linha de filme
2. `_parsear_linha_person()` — interpreta uma linha de pessoa
3. `_parsear_linha_node()` — roteador: lê o tipo e chama o parser correto
4. `_parsear_linha_relacao()` — interpreta uma linha de relacionamento
5. `file_reader_ler_nodes()` — abre o arquivo e itera, chamando (3)
6. `file_reader_ler_relacoes()` — abre o arquivo e itera, chamando (4)
7. `file_reader_carregar_tudo()` — chama (5) e depois (6)

### `src/utils/string_utils/`

Funções reutilizáveis de manipulação de strings:

- `str_extrair_campo()` — extrai um campo delimitado por `|` com trim automático
- `str_trim()` — remove espaços do início e do fim (in-place)
- `str_linha_vazia()` — retorna 1 se a string for só whitespace

### `src/query/`

Contém as 20 funções de consulta, uma por letra (a–t).  Cada função:
- Tem assinatura definitiva (`void query_X(TDB *db)`)
- Já exibe um cabeçalho padronizado com o enunciado
- Contém um comentário **COMO IMPLEMENTAR** explicando o algoritmo sugerido
- Exibe `[TODO]` enquanto não implementada

### `src/io/menu/`

Gerencia toda a interação com o usuário.  Três funções:

- `menu_principal()` — loop principal com 4 opções
- `menu_operacoes()` — inserção, remoção, busca, impressão da árvore
- `menu_consultas()` — despacha para `query_X()` conforme letra digitada

---

## A Árvore B: como funciona

### Grau mínimo `t`

Cada nó (exceto a raiz) tem entre `t-1` e `2t-1` chaves.
Cada nó interno tem entre `t` e `2t` filhos.
O valor padrão é `t = 3` (configurável no menu).

### Inserção

1. Se a árvore estiver vazia, cria a raiz folha e insere.
2. Se a raiz estiver cheia (2t-1 chaves), cria nova raiz e divide a antiga.
3. Desce pela árvore, dividindo (split) qualquer nó cheio encontrado no caminho.
4. Insere na folha correta.

### Divisão de nó (split)

Quando um nó `y` com 2t-1 chaves precisa ser dividido:
- Cria novo nó `z`
- Se `y` for folha: `z` copia as `t` chaves da metade direita de `y`, e as
  folhas são encadeadas: `y.prox → z`
- Se `y` for interno: `z` herda as `t-1` chaves e `t` filhos da metade direita
- A chave mediana sobe para o pai

### Arquivos em disco

| Arquivo | Conteúdo |
|---|---|
| `arvore_filmes.dat` | Nós da árvore de filmes |
| `arvore_pessoas.dat` | Nós da árvore de pessoas |
| `arvore_relacoes.dat` | Nós da árvore de relacionamentos |
| `folha_001.bin` … `folha_NNN.bin` | Cada nó folha em arquivo próprio |

Os arquivos `folha_NNN.bin` são o diferencial exigido pelo trabalho: simulam o
comportamento de SGBDs que separam o arquivo de índice (`*.dat`) dos arquivos de
dados (as "páginas" de folha).

---

## Como os dados chegam ao disco

```
Nodes.txt (linha):  "Movie | The Matrix | 1999 | Welcome to the Real World"
        ↓
str_extrair_campo() → titulo="The Matrix", ano=1999, tagline="..."
        ↓
hash_string("The Matrix") → unsigned long
hash_para_int(hash)       → int (chave da árvore)
        ↓
btree_inserir(db->filmes, chave_int)
        ↓
btree_no_criar() → grava BTreeNo em arvore_filmes.dat (offset X)
btree_folha_salvar() → grava mesma BTreeNo em folha_001.bin
```

---

## Consultas disponíveis

| Letra | Descrição |
|---|---|
| a | Pessoas que trabalharam juntas (qualquer papel) |
| b | Atores e diretores que trabalharam juntos |
| c | Atores que atuaram juntos |
| d | Atores que mais atuaram juntos por década |
| e | Atores e diretores que trabalharam juntos por década |
| f | Atores que mais atuaram, com lista de filmes |
| g | Atores que menos atuaram, com lista de filmes |
| h | Diretores que mais dirigiram, com lista de filmes |
| i | Diretores que menos dirigiram, com lista de filmes |
| j | Produtores mais atuantes, com lista de filmes |
| k | Produtores menos atuantes, com lista de filmes |
| l | Consultas f–k filtradas por década |
| m | Filmes que são continuações |
| n | Atores que nasceram no mesmo ano |
| o | Atores que já dirigiram |
| p | Atores que já produziram |
| q | Retirar todos os participantes de um filme |
| r | Filmes escritos, dirigidos e produzidos pela mesma pessoa |
| s | Filmes dirigidos e produzidos pela mesma pessoa |
| t | Atores que nasceram no ano de lançamento de um filme |

---

## O que ainda precisa ser implementado

### 1. Armazenamento dos dados completos

As árvores atualmente guardam apenas **chaves inteiras** (hash truncado).  Para
responder as consultas, é preciso salvar também os dados completos das entidades.

**Abordagem recomendada:** criar um arquivo de dados paralelo por entidade:

```
pessoas.bin   → registros TPerson em sequência (acesso por offset)
filmes.bin    → registros TMovie em sequência
relacoes.bin  → registros TRelation em sequência
```

A chave da árvore aponta para o **offset** do registro no arquivo de dados,
exatamente como um SGBD real faz.

### 2. Remoção na Árvore B (`btree_remover`)

A função está declarada mas retorna `[TODO]`.  O algoritmo padrão envolve:
- Encontrar e remover a chave
- Redistribuição com irmão adjacente (se irmão tiver chaves sobrando)
- Fusão de nós (se redistribuição não for possível)
- Propagação da correção para cima até a raiz

### 3. Implementar as consultas (a–t)

Cada consulta em `src/query/query.c` tem um comentário **COMO IMPLEMENTAR**
detalhado com o algoritmo sugerido. As assinaturas já estão definitivas.

### 4. Estrutura auxiliar de busca por nome

Para a operação "buscar pelo nome" funcionar com strings (e não só com hash),
é necessário ou (a) percorrer o arquivo de dados comparando nomes, ou (b)
manter um segundo índice com chave baseada em string.

---

## Convenções de código

- **Um arquivo por responsabilidade** — cada `.c` implementa exatamente o que
  seu `.h` promete, nada mais.
- **Funções estáticas são internas** — auxiliares com `static` não aparecem no
  `.h` e não poluem o namespace global.
- **Comentários por seção** — cada `.c` é dividido em seções numeradas com
  banners `/* === */` para facilitar a navegação.
- **Comentários inline** explicam o *porquê*, não o *o quê* (o código já diz
  o que está fazendo).
- **Nomes em português** para tipos de domínio (`TMovie`, `TPerson`) e em
  português/inglês misto para funções utilitárias (`str_trim`, `hash_string`).
- **Sem `malloc` desnecessário** — as structs de domínio são passadas por valor
  ou ponteiro sem alocação dinâmica desnecessária.