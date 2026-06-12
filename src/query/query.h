#ifndef QUERY_H
#define QUERY_H

/* ============================================================
 * MÓDULO DE CONSULTAS
 *
 * Centraliza todas as 20 consultas exigidas pelo trabalho (a–t).
 * Cada função recebe o handle TDB e imprime o resultado.
 *
 * As funções marcadas com TODO ainda não foram implementadas.
 * A assinatura já está definida para facilitar a integração
 * com o menu.
 * ============================================================ */

#include "../io/db.h"

/* (a) Pessoas que trabalharam juntas (qualquer papel) */
void query_a_trabalharam_juntos(TDB *db);

/* (b) Atores e diretores que trabalharam juntos */
void query_b_atores_diretores_juntos(TDB *db);

/* (c) Atores que atuaram juntos */
void query_c_atores_juntos(TDB *db);

/* (d) Atores que mais atuaram juntos por década */
void query_d_atores_juntos_por_decada(TDB *db);

/* (e) Atores e diretores que trabalharam juntos por década */
void query_e_atores_diretores_juntos_por_decada(TDB *db);

/* (f) Atores que mais atuaram, com lista de filmes */
void query_f_atores_mais_atuaram(TDB *db);

/* (g) Atores que menos atuaram, com lista de filmes */
void query_g_atores_menos_atuaram(TDB *db);

/* (h) Diretores que mais dirigiram, com lista de filmes */
void query_h_diretores_mais_dirigiram(TDB *db);

/* (i) Diretores que menos dirigiram, com lista de filmes */
void query_i_diretores_menos_dirigiram(TDB *db);

/* (j) Produtores mais atuantes, com lista de filmes */
void query_j_produtores_mais_atuantes(TDB *db);

/* (k) Produtores menos atuantes, com lista de filmes */
void query_k_produtores_menos_atuantes(TDB *db);

/* (l) Consultas f–k por década */
void query_l_por_decada(TDB *db);

/* (m) Filmes que são continuações */
void query_m_continuacoes(TDB *db);

/* (n) Atores que nasceram no mesmo ano */
void query_n_mesmo_ano_nascimento(TDB *db);

/* (o) Atores que já dirigiram */
void query_o_atores_que_dirigiram(TDB *db);

/* (p) Atores que já produziram */
void query_p_atores_que_produziram(TDB *db);

/* (q) Retirar todos os participantes de um filme */
void query_q_retirar_participantes_filme(TDB *db, const char *titulo_filme);

/* (r) Filmes escritos, dirigidos e produzidos pela mesma pessoa */
void query_r_escreveu_dirigiu_produziu(TDB *db);

/* (s) Filmes dirigidos e produzidos pela mesma pessoa */
void query_s_dirigiu_e_produziu(TDB *db);

/* (t) Atores que nasceram no ano de lançamento de um filme */
void query_t_nasceram_no_ano_do_filme(TDB *db);

#endif /* QUERY_H */