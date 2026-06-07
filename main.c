#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "src/services/arvorebm/TARVBM_MEMORIASECUNDARIA.h"
#include "src/services/files/readfile.h"

#define ARQ_INDICE "indice.dat"

int main(void) {
    setlocale(LC_ALL, "");

    int t           = 3;   // grau mínimo (pode virar menu depois)
    int total_folhas = 0;
    long raiz        = -1;

    // Abre (ou cria) o arquivo de índice ÚNICO
    FILE *arq_indice = fopen(ARQ_INDICE, "rb+");
    if (!arq_indice) arq_indice = fopen(ARQ_INDICE, "wb+");
    if (!arq_indice) {
        fprintf(stderr, "[ERRO] Nao foi possivel abrir '%s'\n", ARQ_INDICE);
        return 1;
    }

    // Lê os arquivos e popula a árvore
    readfile(arq_indice, &raiz, t, &total_folhas);

    printf("\n=== Arvore B+ (chaves em ordem) ===\n");
    TARVBM_imprime_chaves(arq_indice, raiz, t);

    printf("\n=== Arvore B+ (formato visual) ===\n");
    TARVBM_imprime(arq_indice, raiz, 0, t);

    printf("\nTotal de folhas geradas: %d\n", total_folhas);

    fclose(arq_indice);
    return 0;
}