#ifndef TARVBM_H
#define TARVBM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHAVES ((t * 2) - 1)
#define MAX_FILHOS (t * 2)

// Estrutura estática para gravação direta via fwrite/fread
typedef struct arvbm {
    int nchaves;
    int folha;
    int id_folha;      // Identificador único para gerar o arquivo "folha_X.bin"
    long prox_folha;   // ID ou offset da próxima folha (lista encadeada)
    int chave[100];    // Alocação estática (ajustada dinamicamente via parâmetro 't')
    long filho[101];   // Offsets (long) para os nós filhos no arquivo de índices
} TARVBM;

// Funções de gerenciamento de disco
long TARVBM_cria(FILE *arq_indice, int t, int folha, int *contador_folhas);
TARVBM ler_no(FILE *arq_indice, long offset);
void escrever_no(FILE *arq_indice, long offset, TARVBM *no);

// Lógica de manipulação de folhas individuais
void salvar_no_folha(TARVBM *no, int t);
TARVBM carregar_no_folha(int id_folha, int t);

// Funções da árvore adaptadas para disco
long TARVBM_inicializa(void);
long TARVBM_busca(FILE *arq_indice, long offset_raiz, int mat, int t);
long TARVBM_insere(FILE *arq_indice, long offset_raiz, int mat, int t, int *contador_folhas);
long TARVBM_retira(FILE *arq_indice, long offset_raiz, int k, int t);

// Utilitários de Impressão adaptados
void TARVBM_imprime_chaves(FILE *arq_indice, long offset_raiz, int t);
void TARVBM_imprime(FILE *arq_indice, long offset_no, int andar, int t);

#endif