#ifndef QUERY_H
#define QUERY_H

/* ============================================================
 * MÓDULO DE CONSULTAS (a–t)
 *
 * Cada função recebe o handle TDB e imprime o resultado.
 * Os dados completos são lidos de db->tab_filmes[],
 * db->tab_pessoas[] e db->tab_relacoes[].
 * ============================================================ */

#include "../io/db.h"

void query_a_trabalharam_juntos(TDB *db);
void query_b_atores_diretores_juntos(TDB *db);
void query_c_atores_juntos(TDB *db);
void query_d_atores_juntos_por_decada(TDB *db);
void query_e_atores_diretores_juntos_por_decada(TDB *db);
void query_f_atores_mais_atuaram(TDB *db);
void query_g_atores_menos_atuaram(TDB *db);
void query_h_diretores_mais_dirigiram(TDB *db);
void query_i_diretores_menos_dirigiram(TDB *db);
void query_j_produtores_mais_atuantes(TDB *db);
void query_k_produtores_menos_atuantes(TDB *db);
void query_l_por_decada(TDB *db);
void query_m_continuacoes(TDB *db);
void query_n_mesmo_ano_nascimento(TDB *db);
void query_o_atores_que_dirigiram(TDB *db);
void query_p_atores_que_produziram(TDB *db);
void query_q_retirar_participantes_filme(TDB *db, const char *titulo_filme);
void query_r_escreveu_dirigiu_produziu(TDB *db);
void query_s_dirigiu_e_produziu(TDB *db);
void query_t_nasceram_no_ano_do_filme(TDB *db);

#endif /* QUERY_H */
