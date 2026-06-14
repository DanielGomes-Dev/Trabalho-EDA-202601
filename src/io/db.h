#ifndef DB_H
#define DB_H

/* ============================================================
 * CONTEXTO DO BANCO DE DADOS
 *
 * Centraliza as três árvores B e os arrays de dados completos
 * em memória.  As árvores B guardam as chaves (hashes) para
 * busca eficiente; os arrays guardam os registros completos
 * necessários para responder as consultas.
 *
 * Arquivos em disco:
 *   arvore_filmes.dat   – índice de filmes   (árvore B)
 *   arvore_pessoas.dat  – índice de pessoas  (árvore B)
 *   arvore_relacoes.dat – índice de relações (árvore B)
 * ============================================================ */

#include "../core/btree/btree.h"
#include "../domain/movie/movie.h"
#include "../domain/person/person.h"
#include "../domain/relation/relation.h"

#define DB_ARQ_FILMES    "arvore_filmes.dat"
#define DB_ARQ_PESSOAS   "arvore_pessoas.dat"
#define DB_ARQ_RELACOES  "arvore_relacoes.dat"

/* Capacidades máximas dos arrays em memória */
#define DB_MAX_FILMES    500
#define DB_MAX_PESSOAS   500
#define DB_MAX_RELACOES  1000

/* -------------------------------------------------------
 * Handle do banco – árvores B + arrays de dados completos
 * ------------------------------------------------------- */
typedef struct {
    /* Árvores B (índice em disco) */
    BTree *filmes;
    BTree *pessoas;
    BTree *relacoes;
    int    t;              /* Grau mínimo compartilhado */

    /* Arrays de dados completos em memória */
    TMovie    tab_filmes[DB_MAX_FILMES];
    int       n_filmes;

    TPerson   tab_pessoas[DB_MAX_PESSOAS];
    int       n_pessoas;

    TRelation tab_relacoes[DB_MAX_RELACOES];
    int       n_relacoes;
} TDB;

/* Abre/cria as três árvores com grau mínimo `t`       */
TDB *db_abrir(int t);

/* Fecha as três árvores e libera memória               */
void db_fechar(TDB *db);

/* Insere um filme (índice + array)                     */
int db_inserir_filme(TDB *db, TMovie *m);

/* Insere uma pessoa (índice + array)                   */
int db_inserir_pessoa(TDB *db, TPerson *p);

/* Insere um relacionamento (índice + array)            */
int db_inserir_relacao(TDB *db, TRelation *r);

/* Busca filme por ID; retorna ponteiro ou NULL         */
TMovie   *db_buscar_filme_ptr(TDB *db, unsigned long id);

/* Busca pessoa por ID; retorna ponteiro ou NULL        */
TPerson  *db_buscar_pessoa_ptr(TDB *db, unsigned long id);

/* Busca pessoa por nome exato; retorna ponteiro ou NULL */
TPerson  *db_buscar_pessoa_nome(TDB *db, const char *nome);

/* Busca filme por título exato; retorna ponteiro ou NULL */
TMovie   *db_buscar_filme_titulo(TDB *db, const char *titulo);

/* Mantém compatibilidade: retorna offset na árvore ou -1 */
long db_buscar_filme(TDB *db, unsigned long id);
long db_buscar_pessoa(TDB *db, unsigned long id);

/* Remove filme por ID                                  */
int db_remover_filme(TDB *db, unsigned long id);

/* Remove pessoa por ID                                 */
int db_remover_pessoa(TDB *db, unsigned long id);

/* Remove todas as relações de um filme                 */
int db_remover_relacoes_filme(TDB *db, unsigned long id_filme);

/* Imprime as três árvores em formato visual            */
void db_imprimir_arvores(TDB *db);

#endif /* DB_H */
