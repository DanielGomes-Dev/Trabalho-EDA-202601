#include "TARVBM_MEMORIASECUNDARIA.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 1. GERENCIAMENTO DE DISCO
// ============================================================================

long TARVBM_cria(FILE *arq_indice, int t, int folha, int *contador_folhas) {
    TARVBM novo;
    memset(&novo, 0, sizeof(TARVBM));

    novo.nchaves   = 0;
    novo.folha     = folha;
    novo.prox_folha = -1;

    for (int i = 0; i < 2 * T_MAX + 1; i++)
        novo.filho[i] = -1;

    if (folha) {
        (*contador_folhas)++;
        novo.id_folha = *contador_folhas;
    } else {
        novo.id_folha = -1;
    }

    fseek(arq_indice, 0, SEEK_END);
    long offset = ftell(arq_indice);
    fwrite(&novo, sizeof(TARVBM), 1, arq_indice);
    fflush(arq_indice);

    if (folha)
        salvar_no_folha(&novo, t);

    return offset;
}

TARVBM ler_no(FILE *arq_indice, long offset) {
    TARVBM no;
    memset(&no, 0, sizeof(TARVBM));
    if (offset < 0) return no;
    fseek(arq_indice, offset, SEEK_SET);
    fread(&no, sizeof(TARVBM), 1, arq_indice);
    return no;
}

void escrever_no(FILE *arq_indice, long offset, TARVBM *no) {
    if (offset < 0 || !no) return;
    fseek(arq_indice, offset, SEEK_SET);
    fwrite(no, sizeof(TARVBM), 1, arq_indice);
    fflush(arq_indice);

    if (no->folha)
        salvar_no_folha(no, 0);
}

void salvar_no_folha(TARVBM *no, int t) {
    if (!no || no->id_folha <= 0) return;
    char nome[48];
    sprintf(nome, "folha_%03d.bin", no->id_folha);
    FILE *f = fopen(nome, "wb");
    if (f) {
        fwrite(no, sizeof(TARVBM), 1, f);
        fflush(f);
        fclose(f);
    }
}

TARVBM carregar_no_folha(int id_folha, int t) {
    TARVBM no;
    memset(&no, 0, sizeof(TARVBM));
    if (id_folha <= 0) return no;
    char nome[48];
    sprintf(nome, "folha_%03d.bin", id_folha);
    FILE *f = fopen(nome, "rb");
    if (f) {
        fread(&no, sizeof(TARVBM), 1, f);
        fclose(f);
    }
    return no;
}

long TARVBM_inicializa(void) {
    return -1;
}

// ============================================================================
// Helpers internos: carregar nó do disco, respeitando folhas
// ============================================================================

static TARVBM carregar(FILE *arq_indice, long offset, int t) {
    TARVBM no = ler_no(arq_indice, offset);
    if (no.folha && no.id_folha > 0)
        no = carregar_no_folha(no.id_folha, t);
    return no;
}

// ============================================================================
// 2. BUSCA
// Retorna o offset do nó folha que contém a chave, ou -1 se não encontrada.
// ============================================================================

long TARVBM_busca(FILE *arq_indice, long offset, unsigned long chave, int t) {
    if (offset == -1) return -1;

    TARVBM no = carregar(arq_indice, offset, t);

    int i = 0;
    while (i < no.nchaves && chave > no.chave[i]) i++;

    if (no.folha) {
        if (i < no.nchaves && chave == no.chave[i])
            return offset;
        return -1;
    }

    return TARVBM_busca(arq_indice, no.filho[i], chave, t);
}

// ============================================================================
// 3. INSERÇÃO
// ============================================================================

// Divide o filho y (filho[i-1] de x) e sobe a chave mediana para x.
static long divisao(FILE *arq_indice, long offset_x, int i,
                    long offset_y, int t, int *contador_folhas) {

    TARVBM x = carregar(arq_indice, offset_x, t);
    TARVBM y = carregar(arq_indice, offset_y, t);

    long offset_z = TARVBM_cria(arq_indice, t, y.folha, contador_folhas);
    TARVBM z = carregar(arq_indice, offset_z, t);

    z.folha = y.folha;

    if (!y.folha) {
        // Nó interno: z recebe a metade direita das chaves/filhos de y
        // A chave do meio (y.chave[t-1]) sobe para x — não fica em nenhum filho
        z.nchaves = t - 1;
        for (int j = 0; j < t - 1; j++)
            z.chave[j] = y.chave[j + t];
        for (int j = 0; j < t; j++) {
            z.filho[j] = y.filho[j + t];
            y.filho[j + t] = -1;
        }
        // chave que sobe
        unsigned long chave_sobe = y.chave[t - 1];
        y.nchaves = t - 1;

        // Abre espaço em x
        for (int j = x.nchaves; j >= i; j--)
            x.filho[j + 1] = x.filho[j];
        x.filho[i] = offset_z;

        for (int j = x.nchaves - 1; j >= i - 1; j--)
            x.chave[j + 1] = x.chave[j];
        x.chave[i - 1] = chave_sobe;
        x.nchaves++;

    } else {
        // Folha: z recebe a metade direita (inclusive a chave do meio — B+ guarda cópia)
        z.nchaves = t;
        for (int j = 0; j < t; j++) {
            z.chave[j] = y.chave[j + t - 1];
            z.dados[j] = y.dados[j + t - 1];
        }
        // Encadeia as folhas
        z.prox_folha = y.prox_folha;
        y.prox_folha = z.id_folha;
        y.nchaves    = t - 1;

        // Cópia da menor chave de z sobe para o pai
        for (int j = x.nchaves; j >= i; j--)
            x.filho[j + 1] = x.filho[j];
        x.filho[i] = offset_z;

        for (int j = x.nchaves - 1; j >= i - 1; j--)
            x.chave[j + 1] = x.chave[j];
        x.chave[i - 1] = z.chave[0];
        x.nchaves++;
    }

    escrever_no(arq_indice, offset_x, &x);
    escrever_no(arq_indice, offset_y, &y);
    escrever_no(arq_indice, offset_z, &z);

    return offset_x;
}

// Insere em nó que já sabemos não estar cheio
static long insere_nao_completo(FILE *arq_indice, long offset_x,
                                TRegister reg, unsigned long chave,
                                int t, int *contador_folhas) {

    TARVBM x = carregar(arq_indice, offset_x, t);
    int i = x.nchaves - 1;

    if (x.folha) {
        // Desloca chaves/dados para abrir espaço
        while (i >= 0 && chave < x.chave[i]) {
            x.chave[i + 1] = x.chave[i];
            x.dados[i + 1] = x.dados[i];
            i--;
        }
        x.chave[i + 1] = chave;
        x.dados[i + 1] = reg;
        x.nchaves++;
        escrever_no(arq_indice, offset_x, &x);
        return offset_x;
    }

    // Nó interno: desce para o filho correto
    while (i >= 0 && chave < x.chave[i]) i--;
    i++;

    TARVBM filho = carregar(arq_indice, x.filho[i], t);

    if (filho.nchaves == 2 * t - 1) {
        offset_x = divisao(arq_indice, offset_x, i + 1, x.filho[i], t, contador_folhas);
        x = carregar(arq_indice, offset_x, t);
        if (chave > x.chave[i]) i++;
    }

    long novo_offset = insere_nao_completo(arq_indice, x.filho[i], reg, chave, t, contador_folhas);
    x = carregar(arq_indice, offset_x, t);
    x.filho[i] = novo_offset;
    escrever_no(arq_indice, offset_x, &x);
    return offset_x;
}

long TARVBM_insere(FILE *arq_indice, long offset_raiz,
                   TRegister reg, unsigned long chave,
                   int t, int *contador_folhas) {

    // Chave duplicada? — não insere
    if (TARVBM_busca(arq_indice, offset_raiz, chave, t) != -1)
        return offset_raiz;

    // Árvore vazia
    if (offset_raiz == -1) {
        long offset = TARVBM_cria(arq_indice, t, 1, contador_folhas);
        TARVBM no   = carregar(arq_indice, offset, t);
        no.chave[0] = chave;
        no.dados[0] = reg;
        no.nchaves  = 1;
        escrever_no(arq_indice, offset, &no);
        return offset;
    }

    TARVBM raiz = carregar(arq_indice, offset_raiz, t);

    if (raiz.nchaves == 2 * t - 1) {
        // Raiz cheia: cria nova raiz e divide a antiga
        long offset_s = TARVBM_cria(arq_indice, t, 0, contador_folhas);
        TARVBM s;
        memset(&s, 0, sizeof(TARVBM));
        s.nchaves  = 0;
        s.folha    = 0;
        s.id_folha = -1;
        for (int i = 0; i < 2 * T_MAX + 1; i++) s.filho[i] = -1;
        s.filho[0] = offset_raiz;
        escrever_no(arq_indice, offset_s, &s);

        offset_s = divisao(arq_indice, offset_s, 1, offset_raiz, t, contador_folhas);
        offset_s = insere_nao_completo(arq_indice, offset_s, reg, chave, t, contador_folhas);
        return offset_s;
    }

    return insere_nao_completo(arq_indice, offset_raiz, reg, chave, t, contador_folhas);
}

// ============================================================================
// 4. REMOÇÃO  (esqueleto — necessária para o requisito (3) do enunciado)
// ============================================================================

long TARVBM_retira(FILE *arq_indice, long offset_raiz, unsigned long chave, int t) {
    // TODO: implementar remoção completa
    // Por enquanto avisa e devolve a raiz intacta
    printf("[AVISO] TARVBM_retira: ainda nao implementado (chave %lu)\n", chave);
    return offset_raiz;
}

// ============================================================================
// 5. IMPRESSÃO
// ============================================================================

// Imprime todas as chaves em ordem (percorre a lista encadeada de folhas)
void TARVBM_imprime_chaves(FILE *arq_indice, long offset_raiz, int t) {
    if (offset_raiz == -1) return;

    // Desce até a folha mais à esquerda
    TARVBM no = carregar(arq_indice, offset_raiz, t);
    while (!no.folha)
        no = carregar(arq_indice, no.filho[0], t);

    // Percorre lista encadeada de folhas
    while (1) {
        for (int i = 0; i < no.nchaves; i++)
            printf("%lu ", no.chave[i]);
        if (no.prox_folha != -1)
            no = carregar_no_folha(no.prox_folha, t);
        else
            break;
    }
    printf("\n");
}

// Imprime a árvore no formato visual rotacionado (raiz no topo)
void TARVBM_imprime(FILE *arq_indice, long offset_no, int andar, int t) {
    if (offset_no == -1) return;

    TARVBM no = carregar(arq_indice, offset_no, t);

    if (!no.folha)
        TARVBM_imprime(arq_indice, no.filho[no.nchaves], andar + 1, t);

    for (int i = no.nchaves - 1; i >= 0; i--) {
        for (int j = 0; j <= andar; j++) printf("\t");
        printf("%lu\n", no.chave[i]);
        if (!no.folha)
            TARVBM_imprime(arq_indice, no.filho[i], andar + 1, t);
    }
}