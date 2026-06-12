#ifndef RELATION_H
#define RELATION_H

/* ============================================================
 * DOMÍNIO: Relacionamento Pessoa ↔ Filme
 *
 * Representa o vínculo entre uma pessoa e um filme:
 *   ACTED_IN, DIRECTED, PRODUCED, WROTE, etc.
 *
 * A chave da árvore de relações é composta por (id_pessoa, id_filme)
 * para permitir buscas nos dois sentidos.
 * ============================================================ */

#define MAX_PAPEL_RELATION    50
#define MAX_INFO_RELATION    100

/* Tipos de papel reconhecidos no arquivo de relacionamentos */
typedef enum {
    PAPEL_DESCONHECIDO = 0,
    PAPEL_ACTED_IN,    /* Atuou no filme     */
    PAPEL_DIRECTED,    /* Dirigiu o filme    */
    PAPEL_PRODUCED,    /* Produziu o filme   */
    PAPEL_WROTE        /* Escreveu o filme   */
} TPapel;

typedef struct {
    unsigned long id_pessoa;                  /* Quem realizou a ação          */
    unsigned long id_filme;                   /* Em qual filme                 */
    TPapel        papel;                      /* Tipo do relacionamento        */
    char          papel_str[MAX_PAPEL_RELATION]; /* String original do papel   */
    char          info_adicional[MAX_INFO_RELATION]; /* Ex.: "role: Neo"       */
} TRelation;

/* Constrói um TRelation preenchido */
TRelation relation_criar(unsigned long id_pessoa, unsigned long id_filme,
                         const char *papel_str, const char *info_adicional);

/* Converte string de papel para enum TPapel */
TPapel relation_papel_de_string(const char *papel_str);

/* Imprime os dados de um relacionamento no stdout */
void relation_imprimir(const TRelation *r);

#endif /* RELATION_H */