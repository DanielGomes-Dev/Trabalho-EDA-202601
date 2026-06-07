#ifndef TARVBM_H
#define TARVBM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINHA 512
#define T_MAX 50

// ============================================================================
// TIPOS DE REGISTRO
// ============================================================================

typedef struct {
    unsigned long id;
    char titulo[100];
    int ano;
    char tagline[150];
} TMovie;

typedef struct {
    unsigned long id;
    char nome[100];
    int ano_nascimento;
} TPerson;

typedef struct {
    unsigned long id_pessoa;
    unsigned long id_filme;
    char papel[50];
    char info_adicional[100];
} TMoviePerson;

typedef enum {
    TIPO_INVALIDO = 0,
    REG_FILME,
    REG_PESSOA,
    REG_RELACIONAMENTO
} TipoRegistro;

typedef struct {
    TipoRegistro tipo;
    union {
        TMovie       filme;
        TPerson      pessoa;
        TMoviePerson rel;
    } conteudo;
} TRegister;

// ============================================================================
// CABEÇALHO — primeiros bytes do indice.dat
// Guarda a raiz e o estado da árvore entre execuções.
// ============================================================================

typedef struct {
    long raiz;          // offset da raiz (-1 se árvore vazia)
    int  t_grau;        // grau mínimo da árvore
    int  total_folhas;  // contador global de folhas criadas
} TCabecalho;

// ============================================================================
// NÓ DA ÁRVORE B+
// ============================================================================

typedef struct arvbm {
    int           nchaves;
    int           folha;
    int           id_folha;
    int           prox_folha;

    unsigned long chave[2 * T_MAX];
    TRegister     dados [2 * T_MAX];
    long          filho[2 * T_MAX + 1];
} TARVBM;

// ============================================================================
// PROTÓTIPOS
// ============================================================================

// Cabeçalho
void    gravar_cabecalho     (FILE *arq_indice, TCabecalho *cab);
int     ler_cabecalho        (FILE *arq_indice, TCabecalho *cab);
TCabecalho abrir_ou_criar    (FILE **arq_indice, const char *caminho, int t);

// Disco
long    TARVBM_cria          (FILE *arq_indice, int t, int folha, int *contador_folhas);
TARVBM  ler_no               (FILE *arq_indice, long offset);
void    escrever_no          (FILE *arq_indice, long offset, TARVBM *no);

// Folhas individuais
void    salvar_no_folha      (TARVBM *no, int t);
TARVBM  carregar_no_folha    (int id_folha, int t);

// Árvore
long    TARVBM_busca         (FILE *arq_indice, long offset_raiz, unsigned long chave, int t);
long    TARVBM_insere        (FILE *arq_indice, long offset_raiz, TRegister reg, unsigned long chave, int t, int *contador_folhas);
long    TARVBM_retira        (FILE *arq_indice, long offset_raiz, unsigned long chave, int t);

// Impressão
void    TARVBM_imprime_chaves(FILE *arq_indice, long offset_raiz, int t);
void    TARVBM_imprime       (FILE *arq_indice, long offset_no,   int andar, int t);

#endif