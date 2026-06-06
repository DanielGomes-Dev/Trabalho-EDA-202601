#include "TARVBM_MEMORIASECUNDARIA.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 1. GERENCIAMENTO DE DISCO OTIMIZADO
// ============================================================================

long TARVBM_cria(FILE *arq_indice, int t, int folha, int *contador_folhas) {
    TARVBM novo;
    memset(&novo, 0, sizeof(TARVBM));
    novo.nchaves = 0;
    novo.folha = folha;
    novo.prox_folha = -1;

    if (folha) {
        (*contador_folhas)++;
        novo.id_folha = *contador_folhas;
    } else {
        novo.id_folha = -1;
    }

    // Limpa exatamente até a capacidade máxima real do vetor filho (101 elementos)
    for (int i = 0; i < 101; i++) {
        novo.filho[i] = -1;
    }

    // Garante posicionamento absoluto e gravação síncrona
    fseek(arq_indice, 0, SEEK_END);
    long offset = ftell(arq_indice);
    
    fwrite(&novo, sizeof(TARVBM), 1, arq_indice);
    fflush(arq_indice); 
    
    if (folha) {
        salvar_no_folha(&novo, t);
    }
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
    
    if (no->folha) {
        salvar_no_folha(no, 0);
    }
}

void salvar_no_folha(TARVBM *no, int t) {
    if (!no || no->id_folha <= 0) return;
    
    char nome_arquivo[32];
    sprintf(nome_arquivo, "folha_%03d.bin", no->id_folha);
    FILE *f = fopen(nome_arquivo, "wb");
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

    char nome_arquivo[32];
    sprintf(nome_arquivo, "folha_%03d.bin", id_folha);
    FILE *f = fopen(nome_arquivo, "rb");
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
// 2. ALGORITMOS DE BUSCA E INSERÇÃO
// ============================================================================

long TARVBM_busca(FILE *arq_indice, long offset, int mat, int t) {
    if (offset == -1) return -1;
    
    TARVBM no = ler_no(arq_indice, offset);
    if (no.folha) {
        no = carregar_no_folha(no.id_folha, t);
    }

    int i = 0;
    while ((i < no.nchaves) && (mat > no.chave[i])) i++;
    
    if ((i < no.nchaves) && (no.folha) && (mat == no.chave[i])) return offset;
    if (no.folha) return -1;
    
    return TARVBM_busca(arq_indice, no.filho[i], mat, t);
}

static long divisao(FILE *arq_indice, long offset_x, int i, long offset_y, int t, int *contador_folhas) {
    TARVBM x = ler_no(arq_indice, offset_x);
    TARVBM y = ler_no(arq_indice, offset_y);
    
    if (y.folha) {
        y = carregar_no_folha(y.id_folha, t);
    }
    
    long offset_z = TARVBM_cria(arq_indice, t, y.folha, contador_folhas);
    TARVBM z = ler_no(arq_indice, offset_z);
    if (z.folha) {
        z = carregar_no_folha(z.id_folha, t);
    }
    
    z.folha = y.folha;
    int j;
    if (!y.folha) {
        z.nchaves = t - 1;
        for (j = 0; j < t - 1; j++) z.chave[j] = y.chave[j + t];
        for (j = 0; j < t; j++) {
            z.filho[j] = y.filho[j + t];
            y.filho[j + t] = -1;
        }
    } else {
        z.nchaves = t;
        for (j = 0; j < t; j++) z.chave[j] = y.chave[j + t - 1];
        z.prox_folha = y.prox_folha;
        y.prox_folha = z.id_folha; 
    }
    
    y.nchaves = t - 1;
    for (j = x.nchaves; j >= i; j--) x.filho[j + 1] = x.filho[j];
    x.filho[i] = offset_z;
    for (j = x.nchaves; j >= i; j--) x.chave[j] = x.chave[j - 1];
    x.chave[i - 1] = y.chave[t - 1];
    x.nchaves++;
    
    escrever_no(arq_indice, offset_x, &x);
    escrever_no(arq_indice, offset_y, &y);
    escrever_no(arq_indice, offset_z, &z);
    
    return offset_x;
}

static long insere_nao_completo(FILE *arq_indice, long offset_x, int mat, int t, int *contador_folhas) {
    TARVBM x = ler_no(arq_indice, offset_x);
    if (x.folha) {
        x = carregar_no_folha(x.id_folha, t);
    }
    int i = x.nchaves - 1;
    
    if (x.folha) {
        while ((i >= 0) && (mat < x.chave[i])) {
            x.chave[i + 1] = x.chave[i];
            i--;
        }
        x.chave[i + 1] = mat;
        x.nchaves++;
        escrever_no(arq_indice, offset_x, &x);
        return offset_x;
    }
    
    while ((i >= 0) && (mat < x.chave[i])) i--;
    i++;
    
    TARVBM filho_i = ler_no(arq_indice, x.filho[i]);
    if (filho_i.folha) {
        filho_i = carregar_no_folha(filho_i.id_folha, t);
    }

    if (filho_i.nchaves == ((2 * t) - 1)) {
        offset_x = divisao(arq_indice, offset_x, (i + 1), x.filho[i], t, contador_folhas);
        x = ler_no(arq_indice, offset_x);
        if (mat > x.chave[i]) i++;
    }
    
    x.filho[i] = insere_nao_completo(arq_indice, x.filho[i], mat, t, contador_folhas);
    escrever_no(arq_indice, offset_x, &x);
    return offset_x;
}

long TARVBM_insere(FILE *arq_indice, long offset_raiz, int mat, int t, int *contador_folhas) {
    if (TARVBM_busca(arq_indice, offset_raiz, mat, t) != -1) return offset_raiz;
    
    if (offset_raiz == -1) {
        long offset = TARVBM_cria(arq_indice, t, 1, contador_folhas);
        TARVBM no = ler_no(arq_indice, offset);
        if (no.folha) no = carregar_no_folha(no.id_folha, t);
        no.chave[0] = mat;   // <-- insere a chave
        no.nchaves = 1;      // <-- registra que tem 1 chave
        escrever_no(arq_indice, offset, &no);
        return offset;
    }
    
    TARVBM T = ler_no(arq_indice, offset_raiz);
    if (T.folha) {
        T = carregar_no_folha(T.id_folha, t);
    }

    if (T.nchaves == (2 * t) - 1) {
        long offset_S = TARVBM_cria(arq_indice, t, 0, contador_folhas);
        TARVBM S = ler_no(arq_indice, offset_S);
        S.nchaves = 0;
        S.folha = 0;
        S.filho[0] = offset_raiz;
        
        escrever_no(arq_indice, offset_S, &S);
        offset_S = divisao(arq_indice, offset_S, 1, offset_raiz, t, contador_folhas);
        offset_S = insere_nao_completo(arq_indice, offset_S, mat, t, contador_folhas);
        return offset_S;
    }
    
    return insere_nao_completo(arq_indice, offset_raiz, mat, t, contador_folhas);
}

// ============================================================================
// 3. FUNÇÕES DE IMPRESSÃO
// ============================================================================

void TARVBM_imprime_chaves(FILE *arq_indice, long offset_raiz, int t) {
    if (offset_raiz == -1) return;
    long curr_offset = offset_raiz;
    TARVBM no = ler_no(arq_indice, curr_offset);
    
    while (!no.folha) {
        curr_offset = no.filho[0];
        no = ler_no(arq_indice, curr_offset);
    }
    
    if (no.folha) {
        no = carregar_no_folha(no.id_folha, t);
    }
    
    while (1) {
        for (int i = 0; i < no.nchaves; i++) printf("%d ", no.chave[i]);
        if (no.prox_folha != -1) {
            no = carregar_no_folha(no.prox_folha, t);
        } else {
            break;
        }
    }
    printf("\n");
}

void TARVBM_imprime(FILE *arq_indice, long offset_no, int andar, int t) {
    if (offset_no == -1) return;
    TARVBM no = ler_no(arq_indice, offset_no);
    if (no.folha) {
        no = carregar_no_folha(no.id_folha, t);
    }
    int i, j;
    
    if (!no.folha) {
        TARVBM_imprime(arq_indice, no.filho[no.nchaves], andar + 1, t);
    }
    for (i = no.nchaves - 1; i >= 0; i--) {
        for (j = 0; j <= andar; j++) printf("\t");
        printf("%d\n", no.chave[i]);
        if (!no.folha) {
            TARVBM_imprime(arq_indice, no.filho[i], andar + 1, t);
        }
    }
}