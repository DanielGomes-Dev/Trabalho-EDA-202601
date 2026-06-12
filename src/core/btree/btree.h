#ifndef BTREE_H
#define BTREE_H

/* ============================================================
 * NÚCLEO: Árvore B em Memória Secundária
 *
 * Implementa uma Árvore B+ armazenada em arquivo binário:
 *   - Um único arquivo de índice (.dat) por árvore.
 *   - Cada nó folha também é gravado em arquivo próprio
 *     (folha_NNN.bin), conforme exigido pelo trabalho.
 *   - Nós internos guardam apenas offsets (long) dos filhos.
 *   - Nós folhas são encadeados em lista ligada (prox_folha).
 *
 * Parâmetro t (grau mínimo):
 *   Cada nó (exceto a raiz) tem entre t-1 e 2t-1 chaves.
 *   Cada nó interno tem entre t e 2t filhos.
 *
 * Limitação estática:
 *   Os vetores chave[] e filho[] são alocados com tamanho
 *   fixo (100 e 101).  O grau mínimo t deve satisfazer
 *   2t-1 <= 100, ou seja, t <= 50.
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Tamanho máximo estático dos vetores internos */
#define BTREE_MAX_CHAVES 100
#define BTREE_MAX_FILHOS 101

/* -------------------------------------------------------
 * Estrutura de um nó da Árvore B (armazenado em disco)
 * ------------------------------------------------------- */
typedef struct {
    int  nchaves;                    /* Quantidade atual de chaves no nó      */
    int  folha;                      /* 1 = folha, 0 = interno                */
    int  id_folha;                   /* ID único do arquivo folha_NNN.bin      */
    long prox_folha;                 /* Offset/ID da próxima folha (lista)     */
    int  chave[BTREE_MAX_CHAVES];    /* Vetor estático de chaves inteiras      */
    long filho[BTREE_MAX_FILHOS];    /* Offsets dos filhos no arquivo de índice*/
} BTreeNo;

/* -------------------------------------------------------
 * Handle da árvore – agrupa todas as informações de estado
 * ------------------------------------------------------- */
typedef struct {
    FILE *arq_indice;     /* Arquivo binário de índice (.dat)           */
    long  offset_raiz;    /* Offset (bytes) da raiz no arquivo de índice*/
    int   t;              /* Grau mínimo da árvore                      */
    int   contador_folhas;/* Total de arquivos folha criados             */
} BTree;

/* ============================================================
 * API PÚBLICA
 * ============================================================ */

/* Abre (ou cria) o arquivo de índice e inicializa o handle */
BTree *btree_abrir(const char *nome_arquivo, int t);

/* Fecha o arquivo e libera o handle */
void   btree_fechar(BTree *bt);

/* Insere a chave `mat` na árvore; retorna 0 se já existe */
int    btree_inserir(BTree *bt, int mat);

/* Retorna o offset do nó que contém `mat`, ou -1 se não encontrado */
long   btree_buscar(BTree *bt, int mat);

/* Remove a chave `mat` da árvore (TODO: a ser implementado) */
int    btree_remover(BTree *bt, int mat);

/* Imprime a árvore em formato de árvore (raiz → filhos) */
void   btree_imprimir(BTree *bt);

/* Imprime todas as chaves em ordem crescente (percorre folhas) */
void   btree_imprimir_chaves_em_ordem(BTree *bt);

/* ============================================================
 * API INTERNA (usada apenas por btree.c)
 * Declarada aqui para facilitar testes unitários futuros.
 * ============================================================ */

/* Cria um novo nó vazio no disco; retorna o offset do nó   */
long   btree_no_criar(BTree *bt, int folha);

/* Lê o nó no offset indicado */
BTreeNo btree_no_ler(BTree *bt, long offset);

/* Grava o nó no offset indicado */
void   btree_no_escrever(BTree *bt, long offset, BTreeNo *no);

/* Salva o nó folha em seu arquivo individual folha_NNN.bin */
void   btree_folha_salvar(BTreeNo *no);

/* Carrega o nó folha do arquivo individual folha_NNN.bin */
BTreeNo btree_folha_carregar(int id_folha);

#endif /* BTREE_H */