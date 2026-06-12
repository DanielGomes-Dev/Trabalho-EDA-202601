#include "btree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations das funções estáticas internas */
static long _btree_buscar_rec(BTree *bt, long offset, int mat);
static void _btree_dividir_filho(BTree *bt, long offset_x, int i, long offset_y);
static long _btree_inserir_nao_cheio(BTree *bt, long offset_x, int mat);
static void _btree_imprimir_rec(BTree *bt, long offset, int nivel);

/* ============================================================
 * SEÇÃO 1: GERENCIAMENTO DE DISCO
 *   Funções responsáveis por criar, ler e gravar nós no
 *   arquivo binário de índice e nos arquivos de folha.
 * ============================================================ */

/* ------------------------------------------------------------
 * btree_abrir
 *   Tenta abrir o arquivo de índice em modo "rb+" (leitura e
 *   escrita sem truncar).  Se não existir, cria em "wb+".
 *   Retorna NULL se não conseguir abrir o arquivo.
 * ------------------------------------------------------------ */
BTree *btree_abrir(const char *nome_arquivo, int t) {
    BTree *bt = (BTree *)malloc(sizeof(BTree));
    if (!bt) return NULL;

    /* Tenta abrir existente; se falhar, cria novo */
    bt->arq_indice = fopen(nome_arquivo, "rb+");
    if (!bt->arq_indice) {
        bt->arq_indice = fopen(nome_arquivo, "wb+");
    }

    if (!bt->arq_indice) {
        free(bt);
        return NULL;
    }

    bt->t               = t;
    bt->offset_raiz     = -1;  /* -1 indica árvore vazia */
    bt->contador_folhas = 0;

    return bt;
}

/* ------------------------------------------------------------
 * btree_fechar
 *   Fecha o arquivo e libera a memória do handle.
 * ------------------------------------------------------------ */
void btree_fechar(BTree *bt) {
    if (!bt) return;
    if (bt->arq_indice) {
        fclose(bt->arq_indice);
        bt->arq_indice = NULL;
    }
    free(bt);
}

/* ------------------------------------------------------------
 * btree_no_criar
 *   Aloca um novo nó zerado no FINAL do arquivo de índice.
 *   Se for folha, incrementa o contador e salva o arquivo
 *   individual folha_NNN.bin.
 *   Retorna o offset (em bytes) onde o nó foi gravado.
 * ------------------------------------------------------------ */
long btree_no_criar(BTree *bt, int folha) {
    BTreeNo novo;
    memset(&novo, 0, sizeof(BTreeNo));

    novo.nchaves    = 0;
    novo.folha      = folha;
    novo.prox_folha = -1;

    /* Filhos inicializados com -1 (ausente) */
    for (int i = 0; i < BTREE_MAX_FILHOS; i++) {
        novo.filho[i] = -1;
    }

    if (folha) {
        bt->contador_folhas++;
        novo.id_folha = bt->contador_folhas;
    } else {
        novo.id_folha = -1;
    }

    /* Posiciona no fim do arquivo e grava */
    fseek(bt->arq_indice, 0, SEEK_END);
    long offset = ftell(bt->arq_indice);

    fwrite(&novo, sizeof(BTreeNo), 1, bt->arq_indice);
    fflush(bt->arq_indice);

    /* Cria também o arquivo individual se for folha */
    if (folha) {
        btree_folha_salvar(&novo);
    }

    return offset;
}

/* ------------------------------------------------------------
 * btree_no_ler
 *   Lê o nó armazenado no `offset` do arquivo de índice.
 * ------------------------------------------------------------ */
BTreeNo btree_no_ler(BTree *bt, long offset) {
    BTreeNo no;
    memset(&no, 0, sizeof(BTreeNo));
    if (offset < 0 || !bt->arq_indice) return no;

    fseek(bt->arq_indice, offset, SEEK_SET);
    fread(&no, sizeof(BTreeNo), 1, bt->arq_indice);
    return no;
}

/* ------------------------------------------------------------
 * btree_no_escrever
 *   Grava o nó no `offset` do arquivo de índice.
 *   Se for folha, sincroniza também o arquivo individual.
 * ------------------------------------------------------------ */
void btree_no_escrever(BTree *bt, long offset, BTreeNo *no) {
    if (offset < 0 || !no || !bt->arq_indice) return;

    fseek(bt->arq_indice, offset, SEEK_SET);
    fwrite(no, sizeof(BTreeNo), 1, bt->arq_indice);
    fflush(bt->arq_indice);

    if (no->folha) {
        btree_folha_salvar(no);
    }
}

/* ------------------------------------------------------------
 * btree_folha_salvar
 *   Grava o nó folha no arquivo "folha_NNN.bin".
 *   Cada folha tem seu arquivo próprio, conforme requisito do
 *   trabalho (similar a SGBDs que gravam pages individualmente).
 * ------------------------------------------------------------ */
void btree_folha_salvar(BTreeNo *no) {
    if (!no || no->id_folha <= 0) return;

    char nome[32];
    snprintf(nome, sizeof(nome), "folha_%03d.bin", no->id_folha);

    FILE *f = fopen(nome, "wb");
    if (f) {
        fwrite(no, sizeof(BTreeNo), 1, f);
        fflush(f);
        fclose(f);
    }
}

/* ------------------------------------------------------------
 * btree_folha_carregar
 *   Lê o arquivo "folha_NNN.bin" e retorna o nó.
 * ------------------------------------------------------------ */
BTreeNo btree_folha_carregar(int id_folha) {
    BTreeNo no;
    memset(&no, 0, sizeof(BTreeNo));
    if (id_folha <= 0) return no;

    char nome[32];
    snprintf(nome, sizeof(nome), "folha_%03d.bin", id_folha);

    FILE *f = fopen(nome, "rb");
    if (f) {
        fread(&no, sizeof(BTreeNo), 1, f);
        fclose(f);
    }
    return no;
}

/* ============================================================
 * SEÇÃO 2: BUSCA
 * ============================================================ */

/* ------------------------------------------------------------
 * btree_buscar
 *   Percorre a árvore de cima para baixo buscando `mat`.
 *   Retorna o offset do nó que contém a chave, ou -1.
 * ------------------------------------------------------------ */
long btree_buscar(BTree *bt, int mat) {
    if (!bt || bt->offset_raiz == -1) return -1;
    return _btree_buscar_rec(bt, bt->offset_raiz, mat);
}

/* Auxiliar recursivo de busca (escopo interno) */
static long _btree_buscar_rec(BTree *bt, long offset, int mat) {
    if (offset == -1) return -1;

    BTreeNo no = btree_no_ler(bt, offset);

    /* Folhas têm os dados reais – carrega do arquivo individual */
    if (no.folha) {
        no = btree_folha_carregar(no.id_folha);
    }

    /* Avança i enquanto mat > chave[i] */
    int i = 0;
    while (i < no.nchaves && mat > no.chave[i]) i++;

    /* Chave encontrada em nó folha */
    if (i < no.nchaves && no.folha && mat == no.chave[i]) return offset;

    /* Chegou na folha sem encontrar */
    if (no.folha) return -1;

    /* Desce para o filho correto */
    return _btree_buscar_rec(bt, no.filho[i], mat);
}

/* ============================================================
 * SEÇÃO 3: INSERÇÃO
 *   Implementa a inserção clássica em árvore B com divisão
 *   de nó cheio (split) durante a descida.
 * ============================================================ */

/* ------------------------------------------------------------
 * _btree_dividir_filho
 *   Divide o filho y (cheio, 2t-1 chaves) do nó x.
 *   Cria novo nó z com as t-1 chaves da metade direita de y.
 *   A chave mediana sobe para x na posição i-1.
 *
 *   Param i: índice em x onde o novo filho z será inserido
 *             (os filhos à direita de i são deslocados).
 * ------------------------------------------------------------ */
static void _btree_dividir_filho(BTree *bt, long offset_x, int i, long offset_y) {
    int t = bt->t;
    BTreeNo x = btree_no_ler(bt, offset_x);
    BTreeNo y = btree_no_ler(bt, offset_y);

    if (y.folha) y = btree_folha_carregar(y.id_folha);

    /* Cria nó irmão z */
    long offset_z = btree_no_criar(bt, y.folha);
    BTreeNo z = btree_no_ler(bt, offset_z);
    if (z.folha) z = btree_folha_carregar(z.id_folha);

    z.folha = y.folha;

    if (!y.folha) {
        /* Nó interno: z herda a metade direita de y (t-1 chaves) */
        z.nchaves = t - 1;
        for (int j = 0; j < t - 1; j++)
            z.chave[j] = y.chave[j + t];
        for (int j = 0; j < t; j++) {
            z.filho[j] = y.filho[j + t];
            y.filho[j + t] = -1;
        }
    } else {
        /* Folha: z copia t chaves (mantém duplicata na fronteira) */
        z.nchaves = t;
        for (int j = 0; j < t; j++)
            z.chave[j] = y.chave[j + t - 1];
        /* Encadeia as folhas em lista ligada */
        z.prox_folha  = y.prox_folha;
        y.prox_folha  = z.id_folha;
    }

    y.nchaves = t - 1;

    /* Abre espaço em x para o novo filho e chave mediana */
    for (int j = x.nchaves; j >= i; j--)
        x.filho[j + 1] = x.filho[j];
    x.filho[i] = offset_z;

    for (int j = x.nchaves; j >= i; j--)
        x.chave[j] = x.chave[j - 1];
    x.chave[i - 1] = y.chave[t - 1];
    x.nchaves++;

    /* Grava os três nós modificados */
    btree_no_escrever(bt, offset_x, &x);
    btree_no_escrever(bt, offset_y, &y);
    btree_no_escrever(bt, offset_z, &z);
}

/* ------------------------------------------------------------
 * _btree_inserir_nao_cheio
 *   Insere `mat` em um nó x garantidamente não-cheio.
 *   Se x for folha, insere diretamente.
 *   Se for interno, desce para o filho correto, dividindo-o
 *   previamente se estiver cheio.
 * ------------------------------------------------------------ */
static long _btree_inserir_nao_cheio(BTree *bt, long offset_x, int mat) {
    int t = bt->t;
    BTreeNo x = btree_no_ler(bt, offset_x);
    if (x.folha) x = btree_folha_carregar(x.id_folha);

    int i = x.nchaves - 1;

    if (x.folha) {
        /* Desloca chaves maiores para abrir espaço */
        while (i >= 0 && mat < x.chave[i]) {
            x.chave[i + 1] = x.chave[i];
            i--;
        }
        x.chave[i + 1] = mat;
        x.nchaves++;
        btree_no_escrever(bt, offset_x, &x);
        return offset_x;
    }

    /* Nó interno: encontra o filho correto */
    while (i >= 0 && mat < x.chave[i]) i--;
    i++;

    BTreeNo filho_i = btree_no_ler(bt, x.filho[i]);
    if (filho_i.folha) filho_i = btree_folha_carregar(filho_i.id_folha);

    /* Divide o filho se estiver cheio */
    if (filho_i.nchaves == (2 * t) - 1) {
        _btree_dividir_filho(bt, offset_x, i + 1, x.filho[i]);
        /* Após a divisão, relê x (foi modificado) e decide qual filho */
        x = btree_no_ler(bt, offset_x);
        if (mat > x.chave[i]) i++;
    }

    x.filho[i] = _btree_inserir_nao_cheio(bt, x.filho[i], mat);
    btree_no_escrever(bt, offset_x, &x);
    return offset_x;
}

/* ------------------------------------------------------------
 * btree_inserir
 *   Ponto de entrada para inserção.
 *   Trata dois casos especiais:
 *     1. Árvore vazia: cria a raiz.
 *     2. Raiz cheia: cria nova raiz e divide a antiga.
 *   Retorna 0 se a chave já existe (sem duplicatas).
 * ------------------------------------------------------------ */
int btree_inserir(BTree *bt, int mat) {
    if (!bt) return -1;

    /* Chave duplicada: não insere */
    if (btree_buscar(bt, mat) != -1) return 0;

    int t = bt->t;

    /* Caso 1: árvore vazia – cria raiz folha */
    if (bt->offset_raiz == -1) {
        long offset = btree_no_criar(bt, 1);  /* 1 = folha */
        BTreeNo no  = btree_no_ler(bt, offset);
        if (no.folha) no = btree_folha_carregar(no.id_folha);
        no.chave[0] = mat;
        no.nchaves  = 1;
        btree_no_escrever(bt, offset, &no);
        bt->offset_raiz = offset;
        return 1;
    }

    /* Caso 2: raiz cheia – cria nova raiz e divide a antiga */
    BTreeNo T = btree_no_ler(bt, bt->offset_raiz);
    if (T.folha) T = btree_folha_carregar(T.id_folha);

    if (T.nchaves == (2 * t) - 1) {
        long offset_s = btree_no_criar(bt, 0);  /* 0 = interno */
        BTreeNo s = btree_no_ler(bt, offset_s);
        s.nchaves  = 0;
        s.folha    = 0;
        s.filho[0] = bt->offset_raiz;
        btree_no_escrever(bt, offset_s, &s);

        _btree_dividir_filho(bt, offset_s, 1, bt->offset_raiz);
        bt->offset_raiz = _btree_inserir_nao_cheio(bt, offset_s, mat);
    } else {
        bt->offset_raiz = _btree_inserir_nao_cheio(bt, bt->offset_raiz, mat);
    }

    return 1;
}

/* ============================================================
 * SEÇÃO 4: REMOÇÃO
 *   TODO: implementar conforme o algoritmo padrão de remoção
 *   em árvore B (redistribuição / fusão de nós).
 * ============================================================ */
int btree_remover(BTree *bt, int mat) {
    (void)bt; (void)mat;
    printf("[btree_remover] TODO: ainda nao implementado.\n");
    return -1;
}

/* ============================================================
 * SEÇÃO 5: IMPRESSÃO
 * ============================================================ */

/* ------------------------------------------------------------
 * btree_imprimir_chaves_em_ordem
 *   Percorre a lista encadeada de folhas da esquerda para a
 *   direita, imprimindo as chaves em ordem crescente.
 * ------------------------------------------------------------ */
void btree_imprimir_chaves_em_ordem(BTree *bt) {
    if (!bt || bt->offset_raiz == -1) {
        printf("(arvore vazia)\n");
        return;
    }

    /* Desce até a folha mais à esquerda */
    long curr = bt->offset_raiz;
    BTreeNo no = btree_no_ler(bt, curr);
    while (!no.folha) {
        curr = no.filho[0];
        no   = btree_no_ler(bt, curr);
    }

    /* Percorre as folhas encadeadas */
    no = btree_folha_carregar(no.id_folha);
    while (1) {
        for (int i = 0; i < no.nchaves; i++)
            printf("%d ", no.chave[i]);
        if (no.prox_folha == -1) break;
        no = btree_folha_carregar((int)no.prox_folha);
    }
    printf("\n");
}

/* ------------------------------------------------------------
 * _btree_imprimir_rec
 *   Impressão recursiva em formato de árvore (rotacionada
 *   90° anti-horário: filho direito em cima, esquerdo embaixo).
 * ------------------------------------------------------------ */
static void _btree_imprimir_rec(BTree *bt, long offset, int nivel) {
    if (offset == -1) return;

    BTreeNo no = btree_no_ler(bt, offset);
    if (no.folha) no = btree_folha_carregar(no.id_folha);

    /* Imprime o sub-árvore direito primeiro */
    if (!no.folha) {
        _btree_imprimir_rec(bt, no.filho[no.nchaves], nivel + 1);
    }

    /* Imprime as chaves deste nó (de maior para menor para a rotação) */
    for (int i = no.nchaves - 1; i >= 0; i--) {
        for (int j = 0; j <= nivel; j++) printf("\t");
        printf("%d\n", no.chave[i]);
        if (!no.folha) {
            _btree_imprimir_rec(bt, no.filho[i], nivel + 1);
        }
    }
}

void btree_imprimir(BTree *bt) {
    if (!bt || bt->offset_raiz == -1) {
        printf("(arvore vazia)\n");
        return;
    }
    _btree_imprimir_rec(bt, bt->offset_raiz, 0);
    printf("\n");
}