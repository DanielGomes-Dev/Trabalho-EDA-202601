#include "db.h"
#include "../core/hash/hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * ABERTURA / FECHAMENTO
 * ============================================================ */

TDB *db_abrir(int t) {
    TDB *db = (TDB *)calloc(1, sizeof(TDB));
    if (!db) return NULL;

    db->t        = t;
    db->n_filmes   = 0;
    db->n_pessoas  = 0;
    db->n_relacoes = 0;

    db->filmes   = btree_abrir(DB_ARQ_FILMES,   t);
    db->pessoas  = btree_abrir(DB_ARQ_PESSOAS,  t);
    db->relacoes = btree_abrir(DB_ARQ_RELACOES, t);

    if (!db->filmes || !db->pessoas || !db->relacoes) {
        db_fechar(db);
        return NULL;
    }

    return db;
}

void db_fechar(TDB *db) {
    if (!db) return;
    if (db->filmes)   btree_fechar(db->filmes);
    if (db->pessoas)  btree_fechar(db->pessoas);
    if (db->relacoes) btree_fechar(db->relacoes);
    free(db);
}

/* ============================================================
 * INSERÇÃO – índice (árvore B) + array em memória
 * ============================================================ */

int db_inserir_filme(TDB *db, TMovie *m) {
    if (!db || !m) return -1;

    /* Evita duplicata no array */
    for (int i = 0; i < db->n_filmes; i++)
        if (db->tab_filmes[i].id_filme == m->id_filme) return 0;

    if (db->n_filmes >= DB_MAX_FILMES) {
        fprintf(stderr, "[DB] Limite de filmes atingido.\n");
        return -1;
    }

    /* Grava no array */
    db->tab_filmes[db->n_filmes++] = *m;

    /* Indexa na árvore B */
    int chave = hash_para_int(m->id_filme);
    return btree_inserir(db->filmes, chave);
}

int db_inserir_pessoa(TDB *db, TPerson *p) {
    if (!db || !p) return -1;

    for (int i = 0; i < db->n_pessoas; i++)
        if (db->tab_pessoas[i].id_pessoa == p->id_pessoa) return 0;

    if (db->n_pessoas >= DB_MAX_PESSOAS) {
        fprintf(stderr, "[DB] Limite de pessoas atingido.\n");
        return -1;
    }

    db->tab_pessoas[db->n_pessoas++] = *p;

    int chave = hash_para_int(p->id_pessoa);
    return btree_inserir(db->pessoas, chave);
}

int db_inserir_relacao(TDB *db, TRelation *r) {
    if (!db || !r) return -1;

    if (db->n_relacoes >= DB_MAX_RELACOES) {
        fprintf(stderr, "[DB] Limite de relacoes atingido.\n");
        return -1;
    }

    db->tab_relacoes[db->n_relacoes++] = *r;

    int chave = hash_para_int(r->id_pessoa ^ r->id_filme ^ (unsigned long)r->papel);
    btree_inserir(db->relacoes, chave);
    return 1;
}

/* ============================================================
 * BUSCA – percorre o array em O(n)
 * ============================================================ */

TMovie *db_buscar_filme_ptr(TDB *db, unsigned long id) {
    if (!db) return NULL;
    for (int i = 0; i < db->n_filmes; i++)
        if (db->tab_filmes[i].id_filme == id) return &db->tab_filmes[i];
    return NULL;
}

TPerson *db_buscar_pessoa_ptr(TDB *db, unsigned long id) {
    if (!db) return NULL;
    for (int i = 0; i < db->n_pessoas; i++)
        if (db->tab_pessoas[i].id_pessoa == id) return &db->tab_pessoas[i];
    return NULL;
}

TPerson *db_buscar_pessoa_nome(TDB *db, const char *nome) {
    if (!db || !nome) return NULL;
    for (int i = 0; i < db->n_pessoas; i++)
        if (strcmp(db->tab_pessoas[i].nome, nome) == 0) return &db->tab_pessoas[i];
    return NULL;
}

TMovie *db_buscar_filme_titulo(TDB *db, const char *titulo) {
    if (!db || !titulo) return NULL;
    for (int i = 0; i < db->n_filmes; i++)
        if (strcmp(db->tab_filmes[i].titulo, titulo) == 0) return &db->tab_filmes[i];
    return NULL;
}

/* Compatibilidade com chamadas anteriores */
long db_buscar_filme(TDB *db, unsigned long id) {
    if (!db) return -1;
    return btree_buscar(db->filmes, hash_para_int(id));
}

long db_buscar_pessoa(TDB *db, unsigned long id) {
    if (!db) return -1;
    return btree_buscar(db->pessoas, hash_para_int(id));
}

/* ============================================================
 * REMOÇÃO
 * ============================================================ */

int db_remover_filme(TDB *db, unsigned long id) {
    if (!db) return -1;

    /* Remove do array deslocando elementos */
    for (int i = 0; i < db->n_filmes; i++) {
        if (db->tab_filmes[i].id_filme == id) {
            for (int j = i; j < db->n_filmes - 1; j++)
                db->tab_filmes[j] = db->tab_filmes[j + 1];
            db->n_filmes--;
            printf("[DB] Filme removido do array.\n");
            break;
        }
    }

    return btree_remover(db->filmes, hash_para_int(id));
}

int db_remover_pessoa(TDB *db, unsigned long id) {
    if (!db) return -1;

    for (int i = 0; i < db->n_pessoas; i++) {
        if (db->tab_pessoas[i].id_pessoa == id) {
            for (int j = i; j < db->n_pessoas - 1; j++)
                db->tab_pessoas[j] = db->tab_pessoas[j + 1];
            db->n_pessoas--;
            printf("[DB] Pessoa removida do array.\n");
            break;
        }
    }

    return btree_remover(db->pessoas, hash_para_int(id));
}

/* Remove todas as relações de um filme (consulta q) */
int db_remover_relacoes_filme(TDB *db, unsigned long id_filme) {
    if (!db) return 0;
    int removidos = 0;
    int i = 0;

    while (i < db->n_relacoes) {
        if (db->tab_relacoes[i].id_filme == id_filme) {
            /* Desloca para cobrir o buraco */
            for (int j = i; j < db->n_relacoes - 1; j++)
                db->tab_relacoes[j] = db->tab_relacoes[j + 1];
            db->n_relacoes--;
            removidos++;
            /* NÃO incrementa i: o elemento que veio para [i] ainda precisa ser verificado */
        } else {
            i++;
        }
    }

    return removidos;
}

/* ============================================================
 * IMPRESSÃO
 * ============================================================ */

void db_imprimir_arvores(TDB *db) {
    if (!db) return;

    printf("\n=== ARVORE B DE FILMES (chaves) ===\n");
    btree_imprimir(db->filmes);

    printf("\n=== ARVORE B DE PESSOAS (chaves) ===\n");
    btree_imprimir(db->pessoas);

    printf("\n=== ARVORE B DE RELACOES (chaves) ===\n");
    btree_imprimir(db->relacoes);
}
