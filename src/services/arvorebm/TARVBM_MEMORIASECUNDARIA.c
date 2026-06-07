#include "TARVBM_MEMORIASECUNDARIA.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 1. CABEÇALHO — persiste raiz entre execuções
// ============================================================================

void gravar_cabecalho(FILE *arq_indice, TCabecalho *cab) {
    fseek(arq_indice, 0, SEEK_SET);
    fwrite(cab, sizeof(TCabecalho), 1, arq_indice);
    fflush(arq_indice);
}

// Retorna 1 se leu com sucesso, 0 se arquivo estava vazio/novo
int ler_cabecalho(FILE *arq_indice, TCabecalho *cab) {
    fseek(arq_indice, 0, SEEK_SET);
    return (fread(cab, sizeof(TCabecalho), 1, arq_indice) == 1);
}

// Abre ou cria o arquivo de índice e inicializa/recupera o cabeçalho.
// Retorna o cabeçalho pronto para uso — raiz já tem o valor correto.
TCabecalho abrir_ou_criar(FILE **arq_indice, const char *caminho, int t) {
    TCabecalho cab;

    // Tenta abrir existente
    *arq_indice = fopen(caminho, "rb+");

    if (*arq_indice && ler_cabecalho(*arq_indice, &cab)) {
        // Arquivo existente com cabeçalho válido — restaura estado
        printf("[INDICE] Arquivo existente carregado. Raiz=%ld, Folhas=%d\n",
               cab.raiz, cab.total_folhas);
        return cab;
    }

    // Arquivo novo ou corrompido: cria do zero
    if (*arq_indice) fclose(*arq_indice);
    *arq_indice = fopen(caminho, "wb+");

    cab.raiz         = -1;
    cab.t_grau       = t;
    cab.total_folhas = 0;
    gravar_cabecalho(*arq_indice, &cab);
    printf("[INDICE] Novo arquivo criado.\n");
    return cab;
}

// ============================================================================
// 2. GERENCIAMENTO DE DISCO
// ============================================================================

long TARVBM_cria(FILE *arq_indice, int t, int folha, int *contador_folhas) {
    TARVBM novo;
    memset(&novo, 0, sizeof(TARVBM));

    novo.nchaves    = 0;
    novo.folha      = folha;
    novo.prox_folha = -1;
    for (int i = 0; i < 2 * T_MAX + 1; i++) novo.filho[i] = -1;

    if (folha) {
        (*contador_folhas)++;
        novo.id_folha = *contador_folhas;
    } else {
        novo.id_folha = -1;
    }

    // Grava APÓS o cabeçalho (ou após os nós já existentes)
    fseek(arq_indice, 0, SEEK_END);
    long offset = ftell(arq_indice);
    fwrite(&novo, sizeof(TARVBM), 1, arq_indice);
    fflush(arq_indice);

    if (folha) salvar_no_folha(&novo, t);

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
    if (no->folha) salvar_no_folha(no, 0);
}

void salvar_no_folha(TARVBM *no, int t) {
    (void)t;
    if (!no || no->id_folha <= 0) return;
    char nome[48];
    sprintf(nome, "folha_%03d.bin", no->id_folha);
    FILE *f = fopen(nome, "wb");
    if (f) { fwrite(no, sizeof(TARVBM), 1, f); fflush(f); fclose(f); }
}

TARVBM carregar_no_folha(int id_folha, int t) {
    (void)t;
    TARVBM no;
    memset(&no, 0, sizeof(TARVBM));
    if (id_folha <= 0) return no;
    char nome[48];
    sprintf(nome, "folha_%03d.bin", id_folha);
    FILE *f = fopen(nome, "rb");
    if (f) { fread(&no, sizeof(TARVBM), 1, f); fclose(f); }
    return no;
}

// Helper: lê nó do .dat e, se for folha, recarrega do folha_XXX.bin
static TARVBM carregar(FILE *arq_indice, long offset, int t) {
    TARVBM no = ler_no(arq_indice, offset);
    if (no.folha && no.id_folha > 0)
        no = carregar_no_folha(no.id_folha, t);
    return no;
}

// ============================================================================
// 3. BUSCA
// ============================================================================

long TARVBM_busca(FILE *arq_indice, long offset, unsigned long chave, int t) {
    if (offset == -1) return -1;
    TARVBM no = carregar(arq_indice, offset, t);

    int i = 0;
    while (i < no.nchaves && chave > no.chave[i]) i++;

    if (no.folha)
        return (i < no.nchaves && chave == no.chave[i]) ? offset : -1;

    return TARVBM_busca(arq_indice, no.filho[i], chave, t);
}

// ============================================================================
// 4. INSERÇÃO
// ============================================================================

static long divisao(FILE *arq_indice, long offset_x, int i,
                    long offset_y, int t, int *contador_folhas) {
    TARVBM x = carregar(arq_indice, offset_x, t);
    TARVBM y = carregar(arq_indice, offset_y, t);

    long offset_z = TARVBM_cria(arq_indice, t, y.folha, contador_folhas);
    TARVBM z = carregar(arq_indice, offset_z, t);
    z.folha = y.folha;

    if (!y.folha) {
        z.nchaves = t - 1;
        for (int j = 0; j < t - 1; j++) z.chave[j] = y.chave[j + t];
        for (int j = 0; j < t;     j++) { z.filho[j] = y.filho[j + t]; y.filho[j + t] = -1; }
        unsigned long chave_sobe = y.chave[t - 1];
        y.nchaves = t - 1;

        for (int j = x.nchaves; j >= i;     j--) x.filho[j + 1] = x.filho[j];
        x.filho[i] = offset_z;
        for (int j = x.nchaves - 1; j >= i - 1; j--) x.chave[j + 1] = x.chave[j];
        x.chave[i - 1] = chave_sobe;
        x.nchaves++;
    } else {
        z.nchaves = t;
        for (int j = 0; j < t; j++) {
            z.chave[j] = y.chave[j + t - 1];
            z.dados[j] = y.dados[j + t - 1];
        }
        z.prox_folha = y.prox_folha;
        y.prox_folha = z.id_folha;
        y.nchaves    = t - 1;

        for (int j = x.nchaves; j >= i;     j--) x.filho[j + 1] = x.filho[j];
        x.filho[i] = offset_z;
        for (int j = x.nchaves - 1; j >= i - 1; j--) x.chave[j + 1] = x.chave[j];
        x.chave[i - 1] = z.chave[0];
        x.nchaves++;
    }

    escrever_no(arq_indice, offset_x, &x);
    escrever_no(arq_indice, offset_y, &y);
    escrever_no(arq_indice, offset_z, &z);
    return offset_x;
}

static long insere_nao_completo(FILE *arq_indice, long offset_x,
                                TRegister reg, unsigned long chave,
                                int t, int *contador_folhas) {
    TARVBM x = carregar(arq_indice, offset_x, t);
    int i = x.nchaves - 1;

    if (x.folha) {
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

    while (i >= 0 && chave < x.chave[i]) i--;
    i++;

    TARVBM filho = carregar(arq_indice, x.filho[i], t);
    if (filho.nchaves == 2 * t - 1) {
        offset_x = divisao(arq_indice, offset_x, i + 1, x.filho[i], t, contador_folhas);
        x = carregar(arq_indice, offset_x, t);
        if (chave > x.chave[i]) i++;
    }

    long novo = insere_nao_completo(arq_indice, x.filho[i], reg, chave, t, contador_folhas);
    x = carregar(arq_indice, offset_x, t);
    x.filho[i] = novo;
    escrever_no(arq_indice, offset_x, &x);
    return offset_x;
}

long TARVBM_insere(FILE *arq_indice, long offset_raiz,
                   TRegister reg, unsigned long chave,
                   int t, int *contador_folhas) {

    if (TARVBM_busca(arq_indice, offset_raiz, chave, t) != -1)
        return offset_raiz;

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
        long offset_s = TARVBM_cria(arq_indice, t, 0, contador_folhas);
        TARVBM s;
        memset(&s, 0, sizeof(TARVBM));
        s.folha    = 0;
        s.id_folha = -1;
        s.prox_folha = -1;
        for (int i = 0; i < 2 * T_MAX + 1; i++) s.filho[i] = -1;
        s.filho[0] = offset_raiz;
        escrever_no(arq_indice, offset_s, &s);

        offset_s = divisao(arq_indice, offset_s, 1, offset_raiz, t, contador_folhas);
        return insere_nao_completo(arq_indice, offset_s, reg, chave, t, contador_folhas);
    }

    return insere_nao_completo(arq_indice, offset_raiz, reg, chave, t, contador_folhas);
}

// ============================================================================
// 5. REMOÇÃO (stub)
// ============================================================================

long TARVBM_retira(FILE *arq_indice, long offset_raiz, unsigned long chave, int t) {
    (void)arq_indice; (void)t;
    printf("[AVISO] TARVBM_retira: nao implementado (chave %lu)\n", chave);
    return offset_raiz;
}

// ============================================================================
// 6. IMPRESSÃO
// ============================================================================

void TARVBM_imprime_chaves(FILE *arq_indice, long offset_raiz, int t) {
    if (offset_raiz == -1) { printf("(arvore vazia)\n"); return; }
    TARVBM no = carregar(arq_indice, offset_raiz, t);
    while (!no.folha) no = carregar(arq_indice, no.filho[0], t);
    while (1) {
        for (int i = 0; i < no.nchaves; i++) printf("%lu ", no.chave[i]);
        if (no.prox_folha != -1) no = carregar_no_folha(no.prox_folha, t);
        else break;
    }
    printf("\n");
}

void TARVBM_imprime(FILE *arq_indice, long offset_no, int andar, int t) {
    if (offset_no == -1) return;
    TARVBM no = carregar(arq_indice, offset_no, t);
    if (!no.folha) TARVBM_imprime(arq_indice, no.filho[no.nchaves], andar + 1, t);
    for (int i = no.nchaves - 1; i >= 0; i--) {
        for (int j = 0; j <= andar; j++) printf("\t");
        printf("%lu\n", no.chave[i]);
        if (!no.folha) TARVBM_imprime(arq_indice, no.filho[i], andar + 1, t);
    }
}