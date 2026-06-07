#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "src/services/arvorebm/TARVBM_MEMORIASECUNDARIA.h"
#include "src/services/files/readfile.h"

#define ARQ_INDICE "indice.dat"


int main(void) {
    setlocale(LC_ALL, "");

    int t = 3; // grau mínimo — pode virar menu depois

    // -----------------------------------------------------------------------
    // Abre ou cria o arquivo de índice e recupera o cabeçalho.
    // Se o arquivo já existe, raiz e total_folhas são restaurados.
    // Se é novo, raiz = -1 e total_folhas = 0.
    // -----------------------------------------------------------------------
    FILE *arq_indice = NULL;
    TCabecalho cab   = abrir_ou_criar(&arq_indice, ARQ_INDICE, t);

    long raiz        = cab.raiz;
    int total_folhas = cab.total_folhas;

    // -----------------------------------------------------------------------
    // Só popula a árvore se ela estiver vazia (primeira execução).
    // Comente o bloco abaixo se quiser forçar re-inserção.
    // -----------------------------------------------------------------------
    
    if (raiz == -1) {
        printf("[MAIN] Arvore vazia — lendo arquivos de entrada...\n");
        readfile(arq_indice, &raiz, t, &total_folhas);

        // Salva a nova raiz e o contador de folhas no cabeçalho
        cab.raiz         = raiz;
        cab.total_folhas = total_folhas;
        gravar_cabecalho(arq_indice, &cab);
        printf("[MAIN] Cabecalho gravado. Raiz=%ld, Folhas=%d\n", raiz, total_folhas);
    } else {
        printf("[MAIN] Arvore carregada do disco. Raiz=%ld, Folhas=%d\n",
               raiz, total_folhas);
    }

    // -----------------------------------------------------------------------
    // Impressão
    // -----------------------------------------------------------------------
    printf("\n=== Arvore B+ (chaves em ordem) ===\n");
    TARVBM_imprime_chaves(arq_indice, raiz, t);

    printf("\n=== Arvore B+ (formato visual) ===\n");
    TARVBM_imprime(arq_indice, raiz, 0, t);

    printf("\nTotal de folhas: %d\n", total_folhas);

    fclose(arq_indice);
    return 0;
}