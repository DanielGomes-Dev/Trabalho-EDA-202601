#include "db.h"
#include "../core/hash/hash.h"
#include <stdio.h>
#include <stdlib.h>

/* ------------------------------------------------------------
 * db_abrir
 *   Cria o handle e abre as três árvores B.
 *   Retorna NULL se qualquer arquivo não puder ser aberto.
 * ------------------------------------------------------------ */
TDB *db_abrir(int t) {
    TDB *db = (TDB *)malloc(sizeof(TDB));
    if (!db) return NULL;

    db->t       = t;
    db->filmes  = btree_abrir(DB_ARQ_FILMES,   t);
    db->pessoas = btree_abrir(DB_ARQ_PESSOAS,  t);
    db->relacoes= btree_abrir(DB_ARQ_RELACOES, t);

    if (!db->filmes || !db->pessoas || !db->relacoes) {
        db_fechar(db);
        return NULL;
    }

    return db;
}

/* ------------------------------------------------------------
 * db_fechar
 *   Fecha as árvores abertas e libera memória.
 * ------------------------------------------------------------ */
void db_fechar(TDB *db) {
    if (!db) return;
    if (db->filmes)   btree_fechar(db->filmes);
    if (db->pessoas)  btree_fechar(db->pessoas);
    if (db->relacoes) btree_fechar(db->relacoes);
    free(db);
}

/* ------------------------------------------------------------
 * db_inserir_filme
 *   Converte o id_filme (unsigned long) para int via
 *   hash_para_int e insere na árvore de filmes.
 * ------------------------------------------------------------ */
int db_inserir_filme(TDB *db, TMovie *m) {
    if (!db || !m) return -1;
    int chave = hash_para_int(m->id_filme);
    printf("   [DB] Indexando Filme '%s' (chave=%d)\n", m->titulo, chave);
    return btree_inserir(db->filmes, chave);
}

/* ------------------------------------------------------------
 * db_inserir_pessoa
 *   Converte o id_pessoa para int e insere na árvore de pessoas.
 * ------------------------------------------------------------ */
int db_inserir_pessoa(TDB *db, TPerson *p) {
    if (!db || !p) return -1;
    int chave = hash_para_int(p->id_pessoa);
    printf("   [DB] Indexando Pessoa '%s' (chave=%d)\n", p->nome, chave);
    return btree_inserir(db->pessoas, chave);
}

/* ------------------------------------------------------------
 * db_inserir_relacao
 *   A chave de relação é derivada de (id_pessoa XOR id_filme)
 *   para tentar distribuir melhor no espaço de chaves.
 * ------------------------------------------------------------ */
int db_inserir_relacao(TDB *db, TRelation *r) {
    if (!db || !r) return -1;
    int chave = hash_para_int(r->id_pessoa ^ r->id_filme);
    printf("   [DB] Indexando Relacao Pessoa %lu -> %s -> Filme %lu (chave=%d)\n",
           r->id_pessoa, r->papel_str, r->id_filme, chave);
    return btree_inserir(db->relacoes, chave);
}

/* ------------------------------------------------------------
 * db_buscar_filme / db_buscar_pessoa
 * ------------------------------------------------------------ */
long db_buscar_filme(TDB *db, unsigned long id) {
    if (!db) return -1;
    return btree_buscar(db->filmes, hash_para_int(id));
}

long db_buscar_pessoa(TDB *db, unsigned long id) {
    if (!db) return -1;
    return btree_buscar(db->pessoas, hash_para_int(id));
}

/* ------------------------------------------------------------
 * db_remover_filme / db_remover_pessoa
 * ------------------------------------------------------------ */
int db_remover_filme(TDB *db, unsigned long id) {
    if (!db) return -1;
    return btree_remover(db->filmes, hash_para_int(id));
}

int db_remover_pessoa(TDB *db, unsigned long id) {
    if (!db) return -1;
    return btree_remover(db->pessoas, hash_para_int(id));
}

/* ------------------------------------------------------------
 * db_imprimir_arvores
 *   Imprime visualmente as três árvores no terminal.
 * ------------------------------------------------------------ */
void db_imprimir_arvores(TDB *db) {
    if (!db) return;

    printf("\n=== ARVORE DE FILMES ===\n");
    btree_imprimir(db->filmes);

    printf("\n=== ARVORE DE PESSOAS ===\n");
    btree_imprimir(db->pessoas);

    printf("\n=== ARVORE DE RELACOES ===\n");
    btree_imprimir(db->relacoes);
}