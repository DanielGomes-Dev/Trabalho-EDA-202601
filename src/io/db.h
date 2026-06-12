#ifndef DB_H
#define DB_H

/* ============================================================
 * CONTEXTO DO BANCO DE DADOS
 *
 * Centraliza as três árvores B e os metadados globais em um
 * único handle (TDB).  Todas as operações de I/O, busca e
 * inserção passam por este handle.
 *
 * Arquivos em disco:
 *   arvore_filmes.dat   – índice de filmes
 *   arvore_pessoas.dat  – índice de pessoas
 *   arvore_relacoes.dat – índice de relacionamentos
 * ============================================================ */

#include "../core/btree/btree.h"
#include "../domain/movie/movie.h"
#include "../domain/person/person.h"
#include "../domain/relation/relation.h"

#define DB_ARQ_FILMES   "arvore_filmes.dat"
#define DB_ARQ_PESSOAS  "arvore_pessoas.dat"
#define DB_ARQ_RELACOES "arvore_relacoes.dat"

/* -------------------------------------------------------
 * Handle do banco – agrupa as três árvores B
 * ------------------------------------------------------- */
typedef struct {
    BTree *filmes;   /* Árvore de índice de filmes   */
    BTree *pessoas;  /* Árvore de índice de pessoas  */
    BTree *relacoes; /* Árvore de índice de relações */
    int    t;        /* Grau mínimo compartilhado    */
} TDB;

/* Abre/cria as três árvores com grau mínimo `t`       */
TDB *db_abrir(int t);

/* Fecha as três árvores e libera memória               */
void db_fechar(TDB *db);

/* Insere um filme no índice                            */
int db_inserir_filme(TDB *db, TMovie *m);

/* Insere uma pessoa no índice                          */
int db_inserir_pessoa(TDB *db, TPerson *p);

/* Insere um relacionamento no índice                   */
int db_inserir_relacao(TDB *db, TRelation *r);

/* Busca filme por ID (hash do título)                  */
long db_buscar_filme(TDB *db, unsigned long id);

/* Busca pessoa por ID (hash do nome)                   */
long db_buscar_pessoa(TDB *db, unsigned long id);

/* Remove filme por ID                                  */
int db_remover_filme(TDB *db, unsigned long id);

/* Remove pessoa por ID                                 */
int db_remover_pessoa(TDB *db, unsigned long id);

/* Imprime as três árvores em formato visual            */
void db_imprimir_arvores(TDB *db);

#endif /* DB_H */