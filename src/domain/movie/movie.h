#ifndef MOVIE_H
#define MOVIE_H

/* ============================================================
 * DOMÍNIO: Filme
 *
 * Representa um filme no banco de dados.
 * O campo `id_filme` é um hash gerado a partir do título.
 * ============================================================ */

#define MAX_TITULO_FILME  100
#define MAX_TAGLINE_FILME 150

typedef struct {
    unsigned long id_filme;                 /* Hash do título – chave da árvore B */
    char          titulo[MAX_TITULO_FILME]; /* Título do filme                    */
    int           ano_lancamento;           /* Ano de lançamento                  */
    char          tagline[MAX_TAGLINE_FILME]; /* Slogan / tagline do filme         */
} TMovie;

/* Constrói um TMovie preenchido */
TMovie movie_criar(unsigned long id, const char *titulo,
                   int ano_lancamento, const char *tagline);

/* Imprime os campos do filme no stdout */
void movie_imprimir(const TMovie *m);

#endif /* MOVIE_H */