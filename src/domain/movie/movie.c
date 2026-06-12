#include "movie.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------
 * movie_criar
 *   Preenche e retorna um TMovie com terminação de string
 *   garantida em todos os campos de texto.
 * ------------------------------------------------------------ */
TMovie movie_criar(unsigned long id, const char *titulo,
                   int ano_lancamento, const char *tagline) {
    TMovie m;

    m.id_filme       = id;
    m.ano_lancamento = ano_lancamento;

    strncpy(m.titulo, titulo, MAX_TITULO_FILME - 1);
    m.titulo[MAX_TITULO_FILME - 1] = '\0';

    strncpy(m.tagline, tagline, MAX_TAGLINE_FILME - 1);
    m.tagline[MAX_TAGLINE_FILME - 1] = '\0';

    return m;
}

/* ------------------------------------------------------------
 * movie_imprimir
 *   Exibe os dados de um filme de forma legível.
 * ------------------------------------------------------------ */
void movie_imprimir(const TMovie *m) {
    if (!m) return;
    printf("Filme   | ID: %-20lu | Titulo: %-30s | Ano: %d | Tagline: %s\n",
           m->id_filme, m->titulo, m->ano_lancamento, m->tagline);
}