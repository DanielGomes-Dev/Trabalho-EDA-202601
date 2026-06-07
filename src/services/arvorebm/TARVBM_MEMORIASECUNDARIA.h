#ifndef TARVBM_H
#define TARVBM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINHA 512
#define T_MAX 50   // grau mínimo máximo permitido (t pode ser 2..50 no menu)

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
        TMovie      filme;
        TPerson     pessoa;
        TMoviePerson rel;
    } conteudo;
} TRegister;

// ============================================================================
// NÓ DA ÁRVORE B+
//
// Capacidade máxima estática dimensionada para t = T_MAX:
//   - até 2*T_MAX - 1  chaves  por nó
//   - até 2*T_MAX      filhos  por nó (nós internos)
//   - até 2*T_MAX - 1  dados   por posição de folha
//
// Arrays fixos em tempo de compilação → tamanho de struct constante
// → fwrite/fread funcionam perfeitamente em disco.
// ============================================================================

typedef struct arvbm {
    int           nchaves;            // quantas chaves válidas neste nó
    int           folha;              // 1 = folha, 0 = nó interno
    int           id_folha;           // ID único para gerar "folha_XXX.bin"
    int           prox_folha;         // id_folha da próxima folha (lista encadeada); -1 se não houver

    unsigned long chave [2 * T_MAX];  // chaves (hash unsigned long)
    TRegister     dados [2 * T_MAX];  // dados reais — usados APENAS nas folhas
    long          filho [2 * T_MAX + 1]; // offsets dos filhos no arquivo de índice
} TARVBM;

// ============================================================================
// PROTÓTIPOS
// ============================================================================

// Disco
long    TARVBM_cria          (FILE *arq_indice, int t, int folha, int *contador_folhas);
TARVBM  ler_no               (FILE *arq_indice, long offset);
void    escrever_no          (FILE *arq_indice, long offset, TARVBM *no);

// Folhas individuais
void    salvar_no_folha      (TARVBM *no, int t);
TARVBM  carregar_no_folha    (int id_folha, int t);

// Árvore
long    TARVBM_inicializa    (void);
long    TARVBM_busca         (FILE *arq_indice, long offset_raiz, unsigned long chave, int t);
long    TARVBM_insere        (FILE *arq_indice, long offset_raiz, TRegister reg, unsigned long chave, int t, int *contador_folhas);
long    TARVBM_retira        (FILE *arq_indice, long offset_raiz, unsigned long chave, int t);

// Impressão
void    TARVBM_imprime_chaves(FILE *arq_indice, long offset_raiz, int t);
void    TARVBM_imprime       (FILE *arq_indice, long offset_no,   int andar, int t);

#endif