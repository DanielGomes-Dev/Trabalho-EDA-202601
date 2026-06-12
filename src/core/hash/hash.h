#ifndef HASH_H
#define HASH_H

/* ============================================================
 * UTILITÁRIO: Funções de Hash
 *
 * Centraliza a geração de hashes usados como chaves nas
 * árvores B.  O algoritmo djb2 é simples, rápido e distribui
 * bem chaves de texto.
 * ============================================================ */

/* Gera um hash unsigned long a partir de uma string (djb2) */
unsigned long hash_string(const char *str);

/* Converte um hash unsigned long para int truncado (para a
   árvore B que usa int como chave internamente).
   ATENÇÃO: colisões são possíveis; suficiente para o trabalho. */
int hash_para_int(unsigned long hash);

#endif /* HASH_H */