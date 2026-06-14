#include "query.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * MÓDULO DE CONSULTAS
 *
 * Todas as 20 consultas (a–t) do trabalho.
 *
 * ESTRATÉGIA GERAL:
 *   A árvore B armazena as chaves (hashes) para busca eficiente.
 *   Os arrays db->tab_filmes[], db->tab_pessoas[] e
 *   db->tab_relacoes[] guardam os dados completos e são
 *   percorridos para responder as consultas.
 *   Com ~167 nós e ~242 relações isso é perfeitamente eficiente.
 * ============================================================ */

/* ============================================================
 * UTILITÁRIOS INTERNOS
 * ============================================================ */

/* Cabeçalho padrão de cada consulta */
static void _cabecalho(const char *letra, const char *descricao) {
    printf("\n");
    printf("============================================================\n");
    printf("  Consulta (%s): %s\n", letra, descricao);
    printf("============================================================\n");
}

/* Calcula a década de um ano: 1999 → 1990, 2003 → 2000 */
static int _decada(int ano) {
    return (ano / 10) * 10;
}

/* Retorna 1 se a pessoa `id` tem pelo menos uma relação com `papel` */
static int _tem_papel(TDB *db, unsigned long id_pessoa, TPapel papel) {
    for (int i = 0; i < db->n_relacoes; i++)
        if (db->tab_relacoes[i].id_pessoa == id_pessoa &&
            db->tab_relacoes[i].papel == papel)
            return 1;
    return 0;
}

/* Retorna o ano de lançamento de um filme dado seu id (0 se não encontrado) */
static int _ano_filme(TDB *db, unsigned long id_filme) {
    for (int i = 0; i < db->n_filmes; i++)
        if (db->tab_filmes[i].id_filme == id_filme)
            return db->tab_filmes[i].ano_lancamento;
    return 0;
}

/* Retorna o título de um filme dado seu id ("?" se não encontrado) */
static const char *_titulo_filme(TDB *db, unsigned long id_filme) {
    for (int i = 0; i < db->n_filmes; i++)
        if (db->tab_filmes[i].id_filme == id_filme)
            return db->tab_filmes[i].titulo;
    return "?";
}

/* Retorna o nome de uma pessoa dado seu id ("?" se não encontrado) */
static const char *_nome_pessoa(TDB *db, unsigned long id_pessoa) {
    for (int i = 0; i < db->n_pessoas; i++)
        if (db->tab_pessoas[i].id_pessoa == id_pessoa)
            return db->tab_pessoas[i].nome;
    return "?";
}

/* ============================================================
 * (a) Pessoas que trabalharam juntas (qualquer papel)
 *
 * Para cada filme, lista todos os pares de pessoas que
 * aparecem nele em qualquer papel.
 * ============================================================ */
void query_a_trabalharam_juntos(TDB *db) {
    _cabecalho("a", "Pessoas que trabalharam juntas (qualquer papel)");

    /* Para cada filme, coleta quem participou */
    for (int f = 0; f < db->n_filmes; f++) {
        unsigned long id_filme = db->tab_filmes[f].id_filme;

        /* Coleta os índices das relações deste filme */
        int participantes[DB_MAX_RELACOES];
        int n_part = 0;
        for (int r = 0; r < db->n_relacoes; r++)
            if (db->tab_relacoes[r].id_filme == id_filme)
                participantes[n_part++] = r;

        if (n_part < 2) continue;

        printf("\n  Filme: %s\n", db->tab_filmes[f].titulo);

        /* Imprime cada par sem repetição (i < j) */
        for (int i = 0; i < n_part; i++) {
            for (int j = i + 1; j < n_part; j++) {
                TRelation *ra = &db->tab_relacoes[participantes[i]];
                TRelation *rb = &db->tab_relacoes[participantes[j]];
                printf("    %-25s (%-10s)  +  %-25s (%s)\n",
                       _nome_pessoa(db, ra->id_pessoa), ra->papel_str,
                       _nome_pessoa(db, rb->id_pessoa), rb->papel_str);
            }
        }
    }
}

/* ============================================================
 * (b) Atores e diretores que trabalharam juntos
 * ============================================================ */
void query_b_atores_diretores_juntos(TDB *db) {
    _cabecalho("b", "Atores e diretores que trabalharam juntos");

    for (int f = 0; f < db->n_filmes; f++) {
        unsigned long id_filme = db->tab_filmes[f].id_filme;
        int achou = 0;

        for (int a = 0; a < db->n_relacoes; a++) {
            if (db->tab_relacoes[a].id_filme != id_filme) continue;
            if (db->tab_relacoes[a].papel != PAPEL_ACTED_IN) continue;

            for (int d = 0; d < db->n_relacoes; d++) {
                if (db->tab_relacoes[d].id_filme != id_filme) continue;
                if (db->tab_relacoes[d].papel != PAPEL_DIRECTED) continue;

                if (!achou) {
                    printf("\n  Filme: %s\n", db->tab_filmes[f].titulo);
                    achou = 1;
                }
                printf("    Ator: %-25s  |  Diretor: %s\n",
                       _nome_pessoa(db, db->tab_relacoes[a].id_pessoa),
                       _nome_pessoa(db, db->tab_relacoes[d].id_pessoa));
            }
        }
    }
}

/* ============================================================
 * (c) Atores que atuaram juntos
 * ============================================================ */
void query_c_atores_juntos(TDB *db) {
    _cabecalho("c", "Atores que atuaram juntos");

    for (int f = 0; f < db->n_filmes; f++) {
        unsigned long id_filme = db->tab_filmes[f].id_filme;

        /* Coleta índices dos atores neste filme */
        int idx[DB_MAX_RELACOES];
        int n = 0;
        for (int r = 0; r < db->n_relacoes; r++)
            if (db->tab_relacoes[r].id_filme == id_filme &&
                db->tab_relacoes[r].papel == PAPEL_ACTED_IN)
                idx[n++] = r;

        if (n < 2) continue;

        printf("\n  Filme: %s (%d)\n",
               db->tab_filmes[f].titulo, db->tab_filmes[f].ano_lancamento);

        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                printf("    %-25s  +  %s\n",
                       _nome_pessoa(db, db->tab_relacoes[idx[i]].id_pessoa),
                       _nome_pessoa(db, db->tab_relacoes[idx[j]].id_pessoa));
    }
}

/* ============================================================
 * (d) Atores que mais atuaram juntos por década
 *
 * Para cada década, conta quantas vezes cada par co-atuou
 * e exibe o(s) par(es) com maior contagem.
 * ============================================================ */
void query_d_atores_juntos_por_decada(TDB *db) {
    _cabecalho("d", "Atores que mais atuaram juntos por decada");

    /* Estrutura local para pares */
    typedef struct {
        unsigned long id_a, id_b;
        int contagem;
        int decada;
    } Par;

    Par pares[5000];
    int n_pares = 0;

    /* Para cada filme, para cada par de atores, acumula por década */
    for (int f = 0; f < db->n_filmes; f++) {
        unsigned long id_filme = db->tab_filmes[f].id_filme;
        int dec = _decada(db->tab_filmes[f].ano_lancamento);
        if (dec == 0) continue;

        /* Coleta atores do filme */
        unsigned long atores[200];
        int n_at = 0;
        for (int r = 0; r < db->n_relacoes; r++)
            if (db->tab_relacoes[r].id_filme == id_filme &&
                db->tab_relacoes[r].papel == PAPEL_ACTED_IN)
                atores[n_at++] = db->tab_relacoes[r].id_pessoa;

        /* Pares */
        for (int i = 0; i < n_at; i++) {
            for (int j = i + 1; j < n_at; j++) {
                unsigned long ia = atores[i] < atores[j] ? atores[i] : atores[j];
                unsigned long ib = atores[i] < atores[j] ? atores[j] : atores[i];

                /* Procura par existente na mesma década */
                int achou = 0;
                for (int k = 0; k < n_pares; k++) {
                    if (pares[k].decada == dec &&
                        pares[k].id_a == ia && pares[k].id_b == ib) {
                        pares[k].contagem++;
                        achou = 1;
                        break;
                    }
                }
                if (!achou && n_pares < 5000) {
                    pares[n_pares].id_a     = ia;
                    pares[n_pares].id_b     = ib;
                    pares[n_pares].decada   = dec;
                    pares[n_pares].contagem = 1;
                    n_pares++;
                }
            }
        }
    }

    /* Imprime por década */
    int decadas[100];
    int n_dec = 0;
    for (int k = 0; k < n_pares; k++) {
        int found = 0;
        for (int d = 0; d < n_dec; d++) if (decadas[d] == pares[k].decada) { found = 1; break; }
        if (!found) decadas[n_dec++] = pares[k].decada;
    }

    /* Ordena décadas crescentemente (bubble simples) */
    for (int i = 0; i < n_dec - 1; i++)
        for (int j = i + 1; j < n_dec; j++)
            if (decadas[j] < decadas[i]) { int tmp = decadas[i]; decadas[i] = decadas[j]; decadas[j] = tmp; }

    for (int d = 0; d < n_dec; d++) {
        int dec = decadas[d];
        int max_cont = 0;
        for (int k = 0; k < n_pares; k++)
            if (pares[k].decada == dec && pares[k].contagem > max_cont)
                max_cont = pares[k].contagem;

        printf("\n  Decada de %d (maximo: %d filme(s) juntos):\n", dec, max_cont);
        for (int k = 0; k < n_pares; k++) {
            if (pares[k].decada == dec && pares[k].contagem == max_cont)
                printf("    %-25s  +  %-25s  (%d vez/vezes)\n",
                       _nome_pessoa(db, pares[k].id_a),
                       _nome_pessoa(db, pares[k].id_b),
                       pares[k].contagem);
        }
    }
}

/* ============================================================
 * (e) Atores e diretores que trabalharam juntos por década
 * ============================================================ */
void query_e_atores_diretores_juntos_por_decada(TDB *db) {
    _cabecalho("e", "Atores e diretores que trabalharam juntos por decada");

    /* Coleta décadas presentes */
    int decadas[100];
    int n_dec = 0;
    for (int f = 0; f < db->n_filmes; f++) {
        int dec = _decada(db->tab_filmes[f].ano_lancamento);
        if (dec == 0) continue;
        int found = 0;
        for (int d = 0; d < n_dec; d++) if (decadas[d] == dec) { found = 1; break; }
        if (!found) decadas[n_dec++] = dec;
    }

    /* Ordena */
    for (int i = 0; i < n_dec - 1; i++)
        for (int j = i + 1; j < n_dec; j++)
            if (decadas[j] < decadas[i]) { int tmp = decadas[i]; decadas[i] = decadas[j]; decadas[j] = tmp; }

    for (int d = 0; d < n_dec; d++) {
        int dec = decadas[d];
        printf("\n  Decada de %d:\n", dec);

        for (int f = 0; f < db->n_filmes; f++) {
            if (_decada(db->tab_filmes[f].ano_lancamento) != dec) continue;
            unsigned long id_filme = db->tab_filmes[f].id_filme;

            for (int a = 0; a < db->n_relacoes; a++) {
                if (db->tab_relacoes[a].id_filme != id_filme) continue;
                if (db->tab_relacoes[a].papel != PAPEL_ACTED_IN) continue;

                for (int dr = 0; dr < db->n_relacoes; dr++) {
                    if (db->tab_relacoes[dr].id_filme != id_filme) continue;
                    if (db->tab_relacoes[dr].papel != PAPEL_DIRECTED) continue;

                    printf("    [%s] Ator: %-25s  |  Diretor: %s\n",
                           db->tab_filmes[f].titulo,
                           _nome_pessoa(db, db->tab_relacoes[a].id_pessoa),
                           _nome_pessoa(db, db->tab_relacoes[dr].id_pessoa));
                }
            }
        }
    }
}

/* ============================================================
 * AUXILIAR PARA (f),(g),(h),(i),(j),(k)
 *
 * Conta aparições de cada pessoa com um determinado papel
 * e exibe em ordem crescente ou decrescente.
 * `decada_filtro` = 0 → sem filtro de década.
 * ============================================================ */
static void _listar_por_papel(TDB *db, TPapel papel,
                              const char *titulo_papel,
                              int mais,         /* 1=mais, 0=menos */
                              int decada_filtro)
{
    /* Conta filmes por pessoa com este papel */
    typedef struct { unsigned long id; int count; } Cont;
    Cont cont[DB_MAX_PESSOAS];
    int  n = 0;

    for (int r = 0; r < db->n_relacoes; r++) {
        if (db->tab_relacoes[r].papel != papel) continue;

        /* Filtro de década */
        if (decada_filtro != 0) {
            int ano = _ano_filme(db, db->tab_relacoes[r].id_filme);
            if (_decada(ano) != decada_filtro) continue;
        }

        unsigned long id = db->tab_relacoes[r].id_pessoa;
        int found = 0;
        for (int k = 0; k < n; k++) {
            if (cont[k].id == id) { cont[k].count++; found = 1; break; }
        }
        if (!found && n < DB_MAX_PESSOAS) {
            cont[n].id    = id;
            cont[n].count = 1;
            n++;
        }
    }

    if (n == 0) {
        printf("  (nenhum registro encontrado)\n");
        return;
    }

    /* Ordena por contagem: bubble sort */
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++) {
            int troca = mais ? (cont[j].count > cont[i].count)
                             : (cont[j].count < cont[i].count);
            if (troca) { Cont tmp = cont[i]; cont[i] = cont[j]; cont[j] = tmp; }
        }

    /* Exibe – mostra todos que têm a mesma contagem do topo */
    int alvo = cont[0].count;
    for (int k = 0; k < n; k++) {
        if (mais  && cont[k].count < alvo) break;
        if (!mais && cont[k].count > alvo) break;

        printf("  %-30s  (%d %s):\n",
               _nome_pessoa(db, cont[k].id), cont[k].count, titulo_papel);

        /* Lista os filmes */
        for (int r = 0; r < db->n_relacoes; r++) {
            if (db->tab_relacoes[r].id_pessoa != cont[k].id) continue;
            if (db->tab_relacoes[r].papel     != papel)      continue;
            if (decada_filtro != 0) {
                int ano = _ano_filme(db, db->tab_relacoes[r].id_filme);
                if (_decada(ano) != decada_filtro) continue;
            }
            printf("      - %s (%d)\n",
                   _titulo_filme(db, db->tab_relacoes[r].id_filme),
                   _ano_filme   (db, db->tab_relacoes[r].id_filme));
        }
    }
}

/* ============================================================
 * (f) Atores que mais atuaram
 * ============================================================ */
void query_f_atores_mais_atuaram(TDB *db) {
    _cabecalho("f", "Atores que mais atuaram, com lista de filmes");
    _listar_por_papel(db, PAPEL_ACTED_IN, "filme(s)", 1, 0);
}

/* ============================================================
 * (g) Atores que menos atuaram
 * ============================================================ */
void query_g_atores_menos_atuaram(TDB *db) {
    _cabecalho("g", "Atores que menos atuaram, com lista de filmes");
    _listar_por_papel(db, PAPEL_ACTED_IN, "filme(s)", 0, 0);
}

/* ============================================================
 * (h) Diretores que mais dirigiram
 * ============================================================ */
void query_h_diretores_mais_dirigiram(TDB *db) {
    _cabecalho("h", "Diretores que mais dirigiram, com lista de filmes");
    _listar_por_papel(db, PAPEL_DIRECTED, "filme(s)", 1, 0);
}

/* ============================================================
 * (i) Diretores que menos dirigiram
 * ============================================================ */
void query_i_diretores_menos_dirigiram(TDB *db) {
    _cabecalho("i", "Diretores que menos dirigiram, com lista de filmes");
    _listar_por_papel(db, PAPEL_DIRECTED, "filme(s)", 0, 0);
}

/* ============================================================
 * (j) Produtores mais atuantes
 * ============================================================ */
void query_j_produtores_mais_atuantes(TDB *db) {
    _cabecalho("j", "Produtores mais atuantes, com lista de filmes");
    _listar_por_papel(db, PAPEL_PRODUCED, "filme(s)", 1, 0);
}

/* ============================================================
 * (k) Produtores menos atuantes
 * ============================================================ */
void query_k_produtores_menos_atuantes(TDB *db) {
    _cabecalho("k", "Produtores menos atuantes, com lista de filmes");
    _listar_por_papel(db, PAPEL_PRODUCED, "filme(s)", 0, 0);
}

/* ============================================================
 * (l) Consultas f–k por década
 *
 * Pede ao usuário a década desejada e exibe todas as 6
 * sub-consultas filtradas por aquela década.
 * ============================================================ */
void query_l_por_decada(TDB *db) {
    _cabecalho("l", "Consultas f-k por decada");

    /* Descobre décadas disponíveis */
    int decadas[100];
    int n_dec = 0;
    for (int f = 0; f < db->n_filmes; f++) {
        int dec = _decada(db->tab_filmes[f].ano_lancamento);
        if (dec == 0) continue;
        int found = 0;
        for (int d = 0; d < n_dec; d++) if (decadas[d] == dec) { found = 1; break; }
        if (!found) decadas[n_dec++] = dec;
    }

    /* Ordena */
    for (int i = 0; i < n_dec - 1; i++)
        for (int j = i + 1; j < n_dec; j++)
            if (decadas[j] < decadas[i]) { int t = decadas[i]; decadas[i] = decadas[j]; decadas[j] = t; }

    printf("  Decadas disponíveis: ");
    for (int d = 0; d < n_dec; d++) printf("%d ", decadas[d]);
    printf("\n  Digite a decada (ex: 1990): ");

    int dec_escolhida = 0;
    if (scanf("%d", &dec_escolhida) != 1) dec_escolhida = 0;
    while (getchar() != '\n');

    if (dec_escolhida == 0) {
        printf("  Decada invalida.\n");
        return;
    }

    printf("\n  -- Decada de %d a %d --\n", dec_escolhida, dec_escolhida + 9);

    printf("\n  [f] Atores que mais atuaram:\n");
    _listar_por_papel(db, PAPEL_ACTED_IN, "filme(s)", 1, dec_escolhida);

    printf("\n  [g] Atores que menos atuaram:\n");
    _listar_por_papel(db, PAPEL_ACTED_IN, "filme(s)", 0, dec_escolhida);

    printf("\n  [h] Diretores que mais dirigiram:\n");
    _listar_por_papel(db, PAPEL_DIRECTED, "filme(s)", 1, dec_escolhida);

    printf("\n  [i] Diretores que menos dirigiram:\n");
    _listar_por_papel(db, PAPEL_DIRECTED, "filme(s)", 0, dec_escolhida);

    printf("\n  [j] Produtores mais atuantes:\n");
    _listar_por_papel(db, PAPEL_PRODUCED, "filme(s)", 1, dec_escolhida);

    printf("\n  [k] Produtores menos atuantes:\n");
    _listar_por_papel(db, PAPEL_PRODUCED, "filme(s)", 0, dec_escolhida);
}

/* ============================================================
 * (m) Filmes que são continuações
 *
 * Heurística: título contém " 2", " II", " 3", " III",
 * "Reloaded", "Revolutions", ": " ou "Part ".
 * Também detecta quando o título de um filme é prefixo de outro.
 * ============================================================ */
void query_m_continuacoes(TDB *db) {
    _cabecalho("m", "Filmes que sao continuacoes");

    /* Palavras/padrões que indicam sequência */
    const char *marcadores[] = {
        " 2", " II", " 3", " III", " IV", " V",
        "Reloaded", "Revolutions", "Part 2", "Part II",
        "Chapter 2", NULL
    };

    int achou_algum = 0;

    for (int f = 0; f < db->n_filmes; f++) {
        const char *titulo = db->tab_filmes[f].titulo;
        int e_continuacao  = 0;

        /* Verifica marcadores explícitos */
        for (int m = 0; marcadores[m] != NULL; m++) {
            if (strstr(titulo, marcadores[m])) {
                e_continuacao = 1;
                break;
            }
        }

        /* Verifica se outro filme tem título que é prefixo deste */
        if (!e_continuacao) {
            for (int g = 0; g < db->n_filmes; g++) {
                if (g == f) continue;
                const char *outro = db->tab_filmes[g].titulo;
                size_t len = strlen(outro);
                /* Ex.: "The Matrix" é prefixo de "The Matrix Reloaded" */
                if (strncmp(titulo, outro, len) == 0 && titulo[len] == ' ') {
                    e_continuacao = 1;
                    break;
                }
            }
        }

        if (e_continuacao) {
            printf("  - %s (%d)\n", titulo, db->tab_filmes[f].ano_lancamento);
            achou_algum = 1;
        }
    }

    if (!achou_algum)
        printf("  (nenhuma continuacao detectada)\n");
}

/* ============================================================
 * (n) Atores que nasceram no mesmo ano
 * ============================================================ */
void query_n_mesmo_ano_nascimento(TDB *db) {
    _cabecalho("n", "Atores que nasceram no mesmo ano");

    /* Coleta anos distintos de atores */
    int anos[500];
    int n_anos = 0;

    for (int p = 0; p < db->n_pessoas; p++) {
        int ano = db->tab_pessoas[p].ano_nascimento;
        if (ano == 0) continue;
        if (!_tem_papel(db, db->tab_pessoas[p].id_pessoa, PAPEL_ACTED_IN)) continue;

        int found = 0;
        for (int a = 0; a < n_anos; a++) if (anos[a] == ano) { found = 1; break; }
        if (!found) anos[n_anos++] = ano;
    }

    /* Ordena anos */
    for (int i = 0; i < n_anos - 1; i++)
        for (int j = i + 1; j < n_anos; j++)
            if (anos[j] < anos[i]) { int t = anos[i]; anos[i] = anos[j]; anos[j] = t; }

    for (int a = 0; a < n_anos; a++) {
        int ano = anos[a];

        /* Coleta atores nascidos neste ano */
        int grupo[DB_MAX_PESSOAS];
        int n_grupo = 0;
        for (int p = 0; p < db->n_pessoas; p++) {
            if (db->tab_pessoas[p].ano_nascimento != ano) continue;
            if (!_tem_papel(db, db->tab_pessoas[p].id_pessoa, PAPEL_ACTED_IN)) continue;
            grupo[n_grupo++] = p;
        }

        if (n_grupo < 2) continue;

        printf("\n  Nascidos em %d:\n", ano);
        for (int k = 0; k < n_grupo; k++)
            printf("    - %s\n", db->tab_pessoas[grupo[k]].nome);
    }
}

/* ============================================================
 * (o) Atores que já dirigiram
 * ============================================================ */
void query_o_atores_que_dirigiram(TDB *db) {
    _cabecalho("o", "Atores que ja dirigiram");

    int achou = 0;
    for (int p = 0; p < db->n_pessoas; p++) {
        unsigned long id = db->tab_pessoas[p].id_pessoa;
        if (_tem_papel(db, id, PAPEL_ACTED_IN) && _tem_papel(db, id, PAPEL_DIRECTED)) {
            printf("  - %s\n", db->tab_pessoas[p].nome);
            /* Lista filmes como ator e como diretor */
            printf("    Como ator:\n");
            for (int r = 0; r < db->n_relacoes; r++)
                if (db->tab_relacoes[r].id_pessoa == id &&
                    db->tab_relacoes[r].papel == PAPEL_ACTED_IN)
                    printf("      * %s\n", _titulo_filme(db, db->tab_relacoes[r].id_filme));
            printf("    Como diretor:\n");
            for (int r = 0; r < db->n_relacoes; r++)
                if (db->tab_relacoes[r].id_pessoa == id &&
                    db->tab_relacoes[r].papel == PAPEL_DIRECTED)
                    printf("      * %s\n", _titulo_filme(db, db->tab_relacoes[r].id_filme));
            achou = 1;
        }
    }
    if (!achou) printf("  (nenhum encontrado)\n");
}

/* ============================================================
 * (p) Atores que já produziram
 * ============================================================ */
void query_p_atores_que_produziram(TDB *db) {
    _cabecalho("p", "Atores que ja produziram");

    int achou = 0;
    for (int p = 0; p < db->n_pessoas; p++) {
        unsigned long id = db->tab_pessoas[p].id_pessoa;
        if (_tem_papel(db, id, PAPEL_ACTED_IN) && _tem_papel(db, id, PAPEL_PRODUCED)) {
            printf("  - %s\n", db->tab_pessoas[p].nome);
            printf("    Como ator:\n");
            for (int r = 0; r < db->n_relacoes; r++)
                if (db->tab_relacoes[r].id_pessoa == id &&
                    db->tab_relacoes[r].papel == PAPEL_ACTED_IN)
                    printf("      * %s\n", _titulo_filme(db, db->tab_relacoes[r].id_filme));
            printf("    Como produtor:\n");
            for (int r = 0; r < db->n_relacoes; r++)
                if (db->tab_relacoes[r].id_pessoa == id &&
                    db->tab_relacoes[r].papel == PAPEL_PRODUCED)
                    printf("      * %s\n", _titulo_filme(db, db->tab_relacoes[r].id_filme));
            achou = 1;
        }
    }
    if (!achou) printf("  (nenhum encontrado)\n");
}

/* ============================================================
 * (q) Retirar todos os participantes de um filme
 *
 * Remove do array de relações todas as relações do filme.
 * A árvore B de relações não suporta remoção ainda, mas o
 * array (que alimenta todas as consultas) é atualizado.
 * ============================================================ */
void query_q_retirar_participantes_filme(TDB *db, const char *titulo_filme) {
    _cabecalho("q", "Retirar participantes de um filme");

    if (!titulo_filme || strlen(titulo_filme) == 0) {
        printf("  Titulo nao informado.\n");
        return;
    }

    /* Encontra o filme */
    TMovie *m = db_buscar_filme_titulo(db, titulo_filme);
    if (!m) {
        printf("  Filme '%s' nao encontrado.\n", titulo_filme);
        return;
    }

    unsigned long id_filme = m->id_filme;
    printf("  Filme: %s\n", m->titulo);

    /* Lista quem será removido antes de remover */
    printf("  Participantes a remover:\n");
    for (int r = 0; r < db->n_relacoes; r++)
        if (db->tab_relacoes[r].id_filme == id_filme)
            printf("    - %-25s  (%s)\n",
                   _nome_pessoa(db, db->tab_relacoes[r].id_pessoa),
                   db->tab_relacoes[r].papel_str);

    int n = db_remover_relacoes_filme(db, id_filme);
    printf("  %d relacao(oes) removida(s).\n", n);
}

/* ============================================================
 * (r) Filmes em que a mesma pessoa escreveu, dirigiu e produziu
 * ============================================================ */
void query_r_escreveu_dirigiu_produziu(TDB *db) {
    _cabecalho("r", "Filmes escritos, dirigidos e produzidos pela mesma pessoa");

    int achou = 0;
    for (int f = 0; f < db->n_filmes; f++) {
        unsigned long id_filme = db->tab_filmes[f].id_filme;

        /* Para cada pessoa com pelo menos um papel neste filme */
        for (int r = 0; r < db->n_relacoes; r++) {
            if (db->tab_relacoes[r].id_filme != id_filme) continue;
            unsigned long id_p = db->tab_relacoes[r].id_pessoa;

            /* Verifica se tem os três papéis */
            int escreveu = 0, dirigiu = 0, produziu = 0;
            for (int s = 0; s < db->n_relacoes; s++) {
                if (db->tab_relacoes[s].id_filme   != id_filme) continue;
                if (db->tab_relacoes[s].id_pessoa  != id_p)     continue;
                if (db->tab_relacoes[s].papel == PAPEL_WROTE)    escreveu = 1;
                if (db->tab_relacoes[s].papel == PAPEL_DIRECTED) dirigiu  = 1;
                if (db->tab_relacoes[s].papel == PAPEL_PRODUCED) produziu = 1;
            }

            if (escreveu && dirigiu && produziu) {
                printf("  Filme: %-30s  |  Pessoa: %s\n",
                       db->tab_filmes[f].titulo, _nome_pessoa(db, id_p));
                achou = 1;
            }
        }
    }
    if (!achou) printf("  (nenhum encontrado)\n");
}

/* ============================================================
 * (s) Filmes em que a mesma pessoa dirigiu e produziu
 * ============================================================ */
void query_s_dirigiu_e_produziu(TDB *db) {
    _cabecalho("s", "Filmes dirigidos e produzidos pela mesma pessoa");

    int achou = 0;
    for (int f = 0; f < db->n_filmes; f++) {
        unsigned long id_filme = db->tab_filmes[f].id_filme;

        for (int r = 0; r < db->n_relacoes; r++) {
            if (db->tab_relacoes[r].id_filme != id_filme) continue;
            unsigned long id_p = db->tab_relacoes[r].id_pessoa;

            int dirigiu = 0, produziu = 0;
            for (int s = 0; s < db->n_relacoes; s++) {
                if (db->tab_relacoes[s].id_filme  != id_filme) continue;
                if (db->tab_relacoes[s].id_pessoa != id_p)     continue;
                if (db->tab_relacoes[s].papel == PAPEL_DIRECTED) dirigiu  = 1;
                if (db->tab_relacoes[s].papel == PAPEL_PRODUCED) produziu = 1;
            }

            if (dirigiu && produziu) {
                printf("  Filme: %-30s  |  Pessoa: %s\n",
                       db->tab_filmes[f].titulo, _nome_pessoa(db, id_p));
                achou = 1;
            }
        }
    }
    if (!achou) printf("  (nenhum encontrado)\n");
}

/* ============================================================
 * (t) Atores que nasceram no ano de lançamento de um filme
 * ============================================================ */
void query_t_nasceram_no_ano_do_filme(TDB *db) {
    _cabecalho("t", "Atores que nasceram no ano de lancamento de um filme");

    int achou = 0;
    for (int f = 0; f < db->n_filmes; f++) {
        int ano = db->tab_filmes[f].ano_lancamento;
        if (ano == 0) continue;

        /* Atores nascidos naquele ano */
        for (int p = 0; p < db->n_pessoas; p++) {
            if (db->tab_pessoas[p].ano_nascimento != ano) continue;
            if (!_tem_papel(db, db->tab_pessoas[p].id_pessoa, PAPEL_ACTED_IN)) continue;

            printf("  Filme: %-30s (%d)  |  Ator: %s\n",
                   db->tab_filmes[f].titulo, ano, db->tab_pessoas[p].nome);
            achou = 1;
        }
    }
    if (!achou) printf("  (nenhum encontrado)\n");
}
