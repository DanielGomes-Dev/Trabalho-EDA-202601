#include "relation.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------
 * relation_papel_de_string
 *   Mapeia o texto lido do arquivo para o enum TPapel.
 * ------------------------------------------------------------ */
TPapel relation_papel_de_string(const char *papel_str) {
    if (!papel_str) return PAPEL_DESCONHECIDO;
    if (strcmp(papel_str, "ACTED_IN")  == 0) return PAPEL_ACTED_IN;
    if (strcmp(papel_str, "DIRECTED")  == 0) return PAPEL_DIRECTED;
    if (strcmp(papel_str, "PRODUCED")  == 0) return PAPEL_PRODUCED;
    if (strcmp(papel_str, "WROTE")     == 0) return PAPEL_WROTE;
    return PAPEL_DESCONHECIDO;
}

/* ------------------------------------------------------------
 * relation_criar
 *   Preenche e retorna um TRelation.
 * ------------------------------------------------------------ */
TRelation relation_criar(unsigned long id_pessoa, unsigned long id_filme,
                         const char *papel_str, const char *info_adicional) {
    TRelation r;

    r.id_pessoa = id_pessoa;
    r.id_filme  = id_filme;
    r.papel     = relation_papel_de_string(papel_str);

    strncpy(r.papel_str, papel_str, MAX_PAPEL_RELATION - 1);
    r.papel_str[MAX_PAPEL_RELATION - 1] = '\0';

    if (info_adicional && strlen(info_adicional) > 0) {
        strncpy(r.info_adicional, info_adicional, MAX_INFO_RELATION - 1);
        r.info_adicional[MAX_INFO_RELATION - 1] = '\0';
    } else {
        r.info_adicional[0] = '\0';
    }

    return r;
}

/* ------------------------------------------------------------
 * relation_imprimir
 *   Exibe os dados de um relacionamento de forma legível.
 * ------------------------------------------------------------ */
void relation_imprimir(const TRelation *r) {
    if (!r) return;
    printf("Relacao | Pessoa: %-20lu | Filme: %-20lu | Papel: %-12s | Info: %s\n",
           r->id_pessoa, r->id_filme, r->papel_str, r->info_adicional);
}