# Explicação Didática do Código — Árvore B+ em Memória Secundária

Vou explicar como se fosse uma aula, do conceito até o código.

---

## 🎓 Conceito Fundamental: Por que Árvore B+?

Imagina que você tem um **catálogo de filmes com 10 milhões de registros**. Eles não cabem na RAM. Precisamos guardar no disco e **buscar rápido** sem carregar tudo.

A Árvore B+ resolve isso:
- Dados ficam **no disco** (arquivo `.dat` + arquivos `folha_XXX.bin`)
- A árvore organiza **índices** (números que apontam onde está cada dado)
- Buscar leva **O(log n)** acessos ao disco — muito eficiente

```
                    [30]              ← nó interno (só guia a busca)
                   /    \
              [10|20]   [40|50]      ← nós internos
             /   |   \
         [5|8] [12|18] [22|28]       ← FOLHAS (têm os dados de verdade)
            ↓      ↓       ↓
         folha_001 folha_002 folha_003  ← arquivos .bin no disco
```

**Diferença B vs B+:** Na B+, **só as folhas têm dados**. Os nós internos têm apenas chaves para guiar o caminho. As folhas ainda são **encadeadas** entre si (lista ligada), o que permite varredura sequencial eficiente.

---

## 🧱 A Estrutura `TARVBM` — O "nó" da árvore

```c
typedef struct arvbm {
    int nchaves;       // quantas chaves válidas esse nó tem agora
    int folha;         // 1 = é folha (tem dados), 0 = é nó interno (só guia)
    int id_folha;      // se for folha: qual é seu número (ex: 3 → "folha_003.bin")
                       // se for interno: -1 (não tem arquivo próprio)
    long prox_folha;   // se for folha: id da PRÓXIMA folha na lista encadeada
                       // se for interno: irrelevante
    int chave[100];    // as chaves (IDs dos filmes/pessoas) guardadas aqui
                       // só as primeiras 'nchaves' posições são válidas
    long filho[101];   // offsets (posição em bytes) dos filhos no arquivo .dat
                       // filho[0] aponta para filhos menores que chave[0]
                       // filho[1] aponta para filhos entre chave[0] e chave[1]
                       // e assim por diante...
} TARVBM;
```

### Por que `long filho[]` e não ponteiro?

Porque é **disco**, não memória RAM. Um ponteiro (`*`) aponta para um endereço na RAM — inútil após fechar o programa. Um `long` guarda a **posição em bytes dentro do arquivo** (offset), que persiste para sempre.

```
Arquivo arvore_filmes.dat:
Byte 0:    [nó raiz — sizeof(TARVBM) bytes]
Byte 1096: [nó filho esquerdo]
Byte 2192: [nó filho direito]

filho[0] = 1096  → "vá para o byte 1096 do arquivo para achar o filho"
```

---

## 📁 Sistema de Dois Arquivos (por que dois?)

O código usa uma estratégia dupla:

| Arquivo | Conteúdo | Acesso |
|---|---|---|
| `arvore_filmes.dat` | **Todos** os nós (internos + folhas) em sequência | via offset (`fseek`) |
| `folha_001.bin`, `folha_002.bin`... | **Cópia** de cada nó-folha individualmente | via `id_folha` |

Por que duplicar as folhas? Para simular o que em sistemas reais seria um **acesso a página de disco separada** — as folhas são a camada de dados, os internos são o índice. No contexto do trabalho, permite carregar uma folha diretamente pelo número sem percorrer o `.dat` inteiro.

---

## 🔧 Funções de Disco — A "camada de I/O"

### `TARVBM_cria` — Fabrica um nó novo no disco

```c
long TARVBM_cria(FILE *arq_indice, int t, int folha, int *contador_folhas) {
    TARVBM novo;
    memset(&novo, 0, sizeof(TARVBM)); // zera tudo — evita lixo de memória

    novo.nchaves = 0;      // nó começa vazio
    novo.folha = folha;    // caller decide se é folha ou nó interno
    novo.prox_folha = -1;  // -1 = "não tem próxima folha" (fim da lista)

    if (folha) {
        (*contador_folhas)++;        // incrementa o contador global
        novo.id_folha = *contador_folhas; // ex: 1ª folha → id=1 → "folha_001.bin"
    } else {
        novo.id_folha = -1;          // nó interno não tem arquivo próprio
    }

    for (int i = 0; i < 101; i++) {
        novo.filho[i] = -1;          // -1 = "esse filho não existe ainda"
    }

    fseek(arq_indice, 0, SEEK_END);  // vai para o FINAL do arquivo
    long offset = ftell(arq_indice); // guarda onde estamos (esse será o offset do novo nó)

    fwrite(&novo, sizeof(TARVBM), 1, arq_indice); // grava o nó no disco
    fflush(arq_indice);              // força escrita imediata (não fica no buffer do SO)

    if (folha) {
        salvar_no_folha(&novo, t);   // também salva em "folha_XXX.bin"
    }

    return offset; // retorna ONDE no arquivo esse nó foi gravado
}
```

### `ler_no` e `escrever_no` — O "fread/fwrite com endereço"

```c
TARVBM ler_no(FILE *arq_indice, long offset) {
    TARVBM no;
    memset(&no, 0, sizeof(TARVBM));
    if (offset < 0) return no;       // offset -1 = nó inválido, retorna vazio

    fseek(arq_indice, offset, SEEK_SET); // "vai para o byte X do arquivo"
    fread(&no, sizeof(TARVBM), 1, arq_indice); // lê exatamente 1 nó
    return no;
}

void escrever_no(FILE *arq_indice, long offset, TARVBM *no) {
    if (offset < 0 || !no) return;

    fseek(arq_indice, offset, SEEK_SET); // posiciona no lugar certo
    fwrite(no, sizeof(TARVBM), 1, arq_indice); // sobrescreve o nó antigo
    fflush(arq_indice);

    if (no->folha) {
        salvar_no_folha(no, 0);      // atualiza também o arquivo folha_XXX.bin
    }
}
```

> 💡 **Analogia:** `ler_no` é como abrir um livro na página X e copiar o capítulo. `escrever_no` é abrir na página X e reescrever o capítulo.

---

## 🔍 Busca — `TARVBM_busca`

```c
long TARVBM_busca(FILE *arq_indice, long offset, int mat, int t) {
    if (offset == -1) return -1;     // árvore vazia ou chegou em filho inexistente

    TARVBM no = ler_no(arq_indice, offset); // carrega o nó do disco
    if (no.folha) {
        no = carregar_no_folha(no.id_folha, t); // folha → lê do arquivo próprio
    }

    int i = 0;
    while ((i < no.nchaves) && (mat > no.chave[i])) i++;
    // Percorre as chaves até achar uma >= mat
    // Ex: chaves=[10,30,50], busco 25 → paro em i=1 (chave[1]=30 > 25)

    // Caso 1: achei a chave exata em uma folha → sucesso
    if ((i < no.nchaves) && (no.folha) && (mat == no.chave[i])) return offset;

    // Caso 2: chegou em folha mas não achou → não existe
    if (no.folha) return -1;

    // Caso 3: nó interno → desce para o filho correto (recursão)
    return TARVBM_busca(arq_indice, no.filho[i], mat, t);
}
```

### Visualizando a busca de "25":

```
Raiz: [10 | 30 | 50]
       ↓    ↓    ↓    ↓
      f1   f2   f3   f4

Busco 25:
  i=0: 25 > 10 → avança
  i=1: 25 < 30 → para em i=1
  Desce para filho[1] → f2

f2 (folha): [12 | 18 | 25]
  Acha 25 na posição i=2 → retorna offset de f2
```

---

## ➕ Inserção — O coração do código

A inserção tem **3 funções em camadas**:

### Camada 1: `TARVBM_insere` — Decide a estratégia

```c
long TARVBM_insere(FILE *arq_indice, long offset_raiz, int mat, int t, int *contador_folhas) {

    // Regra 1: Não insere duplicatas
    if (TARVBM_busca(arq_indice, offset_raiz, mat, t) != -1) return offset_raiz;

    // Regra 2: Árvore vazia → cria a primeira folha e já insere a chave
    if (offset_raiz == -1) {
        long offset = TARVBM_cria(arq_indice, t, 1, contador_folhas);
        TARVBM no = ler_no(arq_indice, offset);
        if (no.folha) no = carregar_no_folha(no.id_folha, t);
        no.chave[0] = mat;  // insere a chave na posição 0
        no.nchaves = 1;     // agora tem 1 chave
        escrever_no(arq_indice, offset, &no);
        return offset;      // esse offset é a nova raiz
    }

    TARVBM T = ler_no(arq_indice, offset_raiz);
    if (T.folha) T = carregar_no_folha(T.id_folha, t);

    // Regra 3: Raiz está CHEIA (2t-1 chaves) → precisa crescer para cima
    if (T.nchaves == (2 * t) - 1) {
        long offset_S = TARVBM_cria(arq_indice, t, 0, contador_folhas); // nova raiz (interno)
        TARVBM S = ler_no(arq_indice, offset_S);
        S.nchaves = 0;
        S.folha = 0;
        S.filho[0] = offset_raiz;  // a raiz antiga vira filho[0] da nova raiz

        escrever_no(arq_indice, offset_S, &S);
        offset_S = divisao(arq_indice, offset_S, 1, offset_raiz, t, contador_folhas); // divide a antiga raiz
        offset_S = insere_nao_completo(arq_indice, offset_S, mat, t, contador_folhas);
        return offset_S; // nova raiz
    }

    // Regra 4: Raiz tem espaço → insere normalmente descendo
    return insere_nao_completo(arq_indice, offset_raiz, mat, t, contador_folhas);
}
```

### Camada 2: `divisao` — Quebra um nó cheio em dois

Quando um nó tem `2t-1` chaves (cheio), ele é **partido ao meio**:

```
Antes (t=3, nó cheio com 5 chaves):
y = [10 | 20 | 30 | 40 | 50]

Depois da divisão:
y = [10 | 20]    chave promovida: [30]    z = [40 | 50]
                        ↓
              sobe para o pai (nó x)
```

Para **folhas** (B+), a chave do meio é **copiada** (não removida) para o pai, e as folhas permanecem encadeadas:

```
Antes:
y_folha = [10 | 20 | 30 | 40 | 50]

Depois:
y_folha = [10 | 20 | 30]  →  z_folha = [30 | 40 | 50]
                               ↑ 30 foi copiado, não removido
Pai recebe cópia do 30 para saber que "à direita começa em 30"
```

```c
static long divisao(FILE *arq_indice, long offset_x, int i, long offset_y, int t, int *contador_folhas) {
    TARVBM x = ler_no(arq_indice, offset_x); // pai
    TARVBM y = ler_no(arq_indice, offset_y); // filho cheio que será dividido
    if (y.folha) y = carregar_no_folha(y.id_folha, t);

    long offset_z = TARVBM_cria(arq_indice, t, y.folha, contador_folhas); // novo irmão
    TARVBM z = ler_no(arq_indice, offset_z);
    if (z.folha) z = carregar_no_folha(z.id_folha, t);
    z.folha = y.folha;

    if (!y.folha) {
        // NÓ INTERNO: move metade das chaves e filhos para z
        z.nchaves = t - 1;
        for (j = 0; j < t - 1; j++) z.chave[j] = y.chave[j + t];   // metade direita
        for (j = 0; j < t; j++) {
            z.filho[j] = y.filho[j + t];  // filhos da metade direita
            y.filho[j + t] = -1;          // limpa y
        }
    } else {
        // FOLHA: copia (não move) as chaves para z, mantém encadeamento
        z.nchaves = t;
        for (j = 0; j < t; j++) z.chave[j] = y.chave[j + t - 1];
        z.prox_folha = y.prox_folha; // z aponta para quem y apontava
        y.prox_folha = z.id_folha;   // y passa a apontar para z (lista ligada)
    }

    y.nchaves = t - 1; // y perde metade

    // Abre espaço no pai (x) e insere a chave do meio + ponteiro para z
    for (j = x.nchaves; j >= i; j--) x.filho[j + 1] = x.filho[j];
    x.filho[i] = offset_z;
    for (j = x.nchaves; j >= i; j--) x.chave[j] = x.chave[j - 1];
    x.chave[i - 1] = y.chave[t - 1]; // chave do meio sobe para o pai
    x.nchaves++;

    // Salva tudo no disco
    escrever_no(arq_indice, offset_x, &x);
    escrever_no(arq_indice, offset_y, &y);
    escrever_no(arq_indice, offset_z, &z);

    return offset_x;
}
```

### Camada 3: `insere_nao_completo` — Desce e insere

```c
static long insere_nao_completo(FILE *arq_indice, long offset_x, int mat, int t, int *contador_folhas) {
    TARVBM x = ler_no(arq_indice, offset_x);
    if (x.folha) x = carregar_no_folha(x.id_folha, t);
    int i = x.nchaves - 1;

    if (x.folha) {
        // CHEGOU NA FOLHA: abre espaço e insere ordenado
        while ((i >= 0) && (mat < x.chave[i])) {
            x.chave[i + 1] = x.chave[i]; // empurra para direita
            i--;
        }
        x.chave[i + 1] = mat; // insere no lugar certo
        x.nchaves++;
        escrever_no(arq_indice, offset_x, &x);
        return offset_x;
    }

    // NÓ INTERNO: acha qual filho seguir
    while ((i >= 0) && (mat < x.chave[i])) i--;
    i++; // filho correto é filho[i]

    TARVBM filho_i = ler_no(arq_indice, x.filho[i]);
    if (filho_i.folha) filho_i = carregar_no_folha(filho_i.id_folha, t);

    // Se o filho está cheio, divide ANTES de descer (estratégia top-down)
    if (filho_i.nchaves == ((2 * t) - 1)) {
        offset_x = divisao(arq_indice, offset_x, (i + 1), x.filho[i], t, contador_folhas);
        x = ler_no(arq_indice, offset_x);
        if (mat > x.chave[i]) i++; // após divisão, decide qual dos dois filhos seguir
    }

    // Desce recursivamente
    x.filho[i] = insere_nao_completo(arq_indice, x.filho[i], mat, t, contador_folhas);
    escrever_no(arq_indice, offset_x, &x);
    return offset_x;
}
```

---

## 📖 A camada de aplicação — `readfile.c`

Essa parte lê arquivos `.txt` e alimenta as árvores:

```
Nodes.txt:
Movie | The Matrix | 1999 | Welcome to the Real World
Person | Keanu Reeves | 1964

Relationships.txt:
START Person | Keanu Reeves | ACTED_IN | END Movie | The Matrix | Neo
```

O fluxo é:

```
fgets(linha) 
    → processar_nodes() / processar_relationships()
        → extrair_campo() [separa por '|']
            → gerar_hash(nome) [transforma string em número único]
                → criar_movie() / criar_person() / criar_movie_person()
                    → inserir_arvore_b()
                        → TARVBM_insere() [grava no disco]
```

### `gerar_hash` — Por que transformar string em número?

A Árvore B+ ordena por **chaves numéricas**. "The Matrix" não é comparável diretamente. O hash transforma a string num `unsigned long` de forma determinística:

```c
unsigned long gerar_hash(const char *str) {
    unsigned long hash = 5381; // valor inicial clássico (djb2)
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
        // equivale a: hash = hash * 33 + c
        // cada caractere muda o hash de forma única
    }
    return hash;
}
// "The Matrix" → sempre produz 1806568347
// "Matrix, The" → produziria um número diferente
```

---

## ⚠️ Bugs corrigidos e por quê importavam

### Bug 1 — Raízes iniciadas com `0` em vez de `-1`

```c
// ERRADO — 0 é um offset válido! Faz ler lixo do arquivo
long raiz_filmes = 0;

// CORRETO — -1 significa "árvore vazia"
long raiz_filmes = -1;
```

`0` é o offset do **primeiro byte do arquivo** — se o arquivo já existir de uma execução anterior, `ler_no` lê dados velhos. Se não existir, `fread` retorna lixo e `nchaves` vira um número aleatório, fazendo o `while` iterar para sempre → **travamento**.

### Bug 2 — Primeiro nó criado mas sem chave inserida

```c
// ANTES (bugado): criava o nó vazio e retornava
if (offset_raiz == -1) {
    return TARVBM_cria(arq_indice, t, 1, contador_folhas); // nó com nchaves=0!
}

// DEPOIS (corrigido): cria E insere a chave
if (offset_raiz == -1) {
    long offset = TARVBM_cria(arq_indice, t, 1, contador_folhas);
    TARVBM no = ler_no(arq_indice, offset);
    if (no.folha) no = carregar_no_folha(no.id_folha, t);
    no.chave[0] = mat;  // ← insere a chave
    no.nchaves = 1;     // ← marca que tem 1 chave
    escrever_no(arq_indice, offset, &no);
    return offset;
}
```

Sem essa correção, a árvore ficava com um nó raiz vazio. Na próxima inserção, `TARVBM_busca` encontrava esse nó, `nchaves=0`, não achava a chave, mas também não desceria (era folha) — retornava `-1`, e `TARVBM_insere` tentava criar outra raiz do zero, corrompendo a estrutura.

---

## 🗺️ Diagrama geral do sistema

```
main.c
  └─ readfile()
       ├─ Nodes.txt → processar_nodes()
       │    ├─ Movie → criar_movie() → inserir_arvore_b() → arvore_filmes.dat
       │    └─ Person → criar_person() → inserir_arvore_b() → arvore_pessoas.dat
       └─ Relationships.txt → processar_relationships()
            └─ ACTED_IN/DIRECTED → criar_movie_person() → arvore_relacoes.dat

Cada inserção em arvore_X.dat também gera/atualiza folha_NNN.bin
```

O sistema ainda não implementa `TARVBM_retira` (remoção) nem uma interface de consulta — essas seriam as próximas etapas naturais do trabalho.