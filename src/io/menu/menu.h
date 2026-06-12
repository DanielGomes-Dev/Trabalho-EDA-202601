#ifndef MENU_H
#define MENU_H

/* ============================================================
 * MÓDULO DE MENU
 *
 * Interface de texto com o usuário.
 * Exibe as opções e despacha para as funções corretas.
 *
 * Estrutura de menus:
 *   menu_principal()        → menu raiz
 *     menu_operacoes()      → inserção, remoção, busca, impressão
 *     menu_consultas()      → consultas (a) a (t)
 * ============================================================ */

#include "../../io/db.h"

/* Exibe o menu principal e gerencia o loop de interação */
void menu_principal(TDB *db);

/* Sub-menu de operações na árvore B */
void menu_operacoes(TDB *db);

/* Sub-menu de consultas (a) a (t) */
void menu_consultas(TDB *db);

#endif /* MENU_H */