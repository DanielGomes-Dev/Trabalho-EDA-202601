#include "query.h"
#include <stdio.h>

/* ============================================================
 * MÓDULO DE CONSULTAS – Implementações
 *
 * NOTA: As consultas dependem de estruturas de dados adicionais
 * que armazenam os dados completos (além das chaves na árvore B).
 * No estado atual do projeto as árvores guardam apenas as
 * chaves inteiras (hashes); para responder as consultas
 * será necessário:
 *
 *   1. Criar um arquivo de dados por entidade (ex: pessoas.bin)
 *      com os registros TMovie / TPerson / TRelation completos.
 *   2. Carregar os dados em memória (ou percorrer o arquivo)
 *      ao executar cada consulta.
 *
 * As funções abaixo estão preparadas com a assinatura correta
 * e um stub que indica o que deve ser implementado.
 * ============================================================ */

/* Helper interno: imprime cabeçalho de consulta */
static void _print_header(const char *letra, const char *descricao) {
    printf("\n");
    printf("============================================================\n");
    printf("  Consulta (%s): %s\n", letra, descricao);
    printf("============================================================\n");
}

/* Helper: indica que a consulta ainda não foi implementada */
static void _todo(const char *letra) {
    printf("  [TODO] Consulta (%s) ainda nao implementada.\n", letra);
    printf("  Para implementar: carregue os registros completos do\n");
    printf("  arquivo de dados e aplique o filtro descrito acima.\n");
}

/* ------------------------------------------------------------ */

void query_a_trabalharam_juntos(TDB *db) {
    (void)db;
    _print_header("a", "Pessoas que trabalharam juntas (qualquer papel)");
    /*
     * COMO IMPLEMENTAR:
     *   1. Percorrer a árvore de relações.
     *   2. Agrupar por id_filme: todos os pares de pessoas
     *      que aparecem no mesmo filme em qualquer papel.
     *   3. Exibir cada par (pessoa_A, pessoa_B, filme).
     */
    _todo("a");
}

void query_b_atores_diretores_juntos(TDB *db) {
    (void)db;
    _print_header("b", "Atores e diretores que trabalharam juntos");
    /*
     * COMO IMPLEMENTAR:
     *   Igual à (a), mas filtrando pares onde
     *   um tem papel ACTED_IN e o outro DIRECTED.
     */
    _todo("b");
}

void query_c_atores_juntos(TDB *db) {
    (void)db;
    _print_header("c", "Atores que atuaram juntos");
    /*
     * COMO IMPLEMENTAR:
     *   Igual à (a), filtrando apenas registros com ACTED_IN.
     *   Agrupa por filme e exibe todos os pares de atores.
     */
    _todo("c");
}

void query_d_atores_juntos_por_decada(TDB *db) {
    (void)db;
    _print_header("d", "Atores que mais atuaram juntos por decada");
    /*
     * COMO IMPLEMENTAR:
     *   - Calcular a décda de cada filme: decada = (ano / 10) * 10.
     *   - Contar quantas vezes cada par (ator_A, ator_B) aparece
     *     na mesma década.
     *   - Ordenar por contagem e exibir o(s) par(es) máximo(s).
     */
    _todo("d");
}

void query_e_atores_diretores_juntos_por_decada(TDB *db) {
    (void)db;
    _print_header("e", "Atores e diretores que trabalharam juntos por decada");
    _todo("e");
}

void query_f_atores_mais_atuaram(TDB *db) {
    (void)db;
    _print_header("f", "Atores que mais atuaram, com lista de filmes");
    /*
     * COMO IMPLEMENTAR:
     *   - Percorrer relações com papel ACTED_IN.
     *   - Contar aparições por ator (id_pessoa).
     *   - Ordenar decrescentemente.
     *   - Para cada ator, listar os títulos dos filmes.
     */
    _todo("f");
}

void query_g_atores_menos_atuaram(TDB *db) {
    (void)db;
    _print_header("g", "Atores que menos atuaram, com lista de filmes");
    /* Igual à (f), mas ordena crescentemente e exibe os menores. */
    _todo("g");
}

void query_h_diretores_mais_dirigiram(TDB *db) {
    (void)db;
    _print_header("h", "Diretores que mais dirigiram, com lista de filmes");
    _todo("h");
}

void query_i_diretores_menos_dirigiram(TDB *db) {
    (void)db;
    _print_header("i", "Diretores que menos dirigiram, com lista de filmes");
    _todo("i");
}

void query_j_produtores_mais_atuantes(TDB *db) {
    (void)db;
    _print_header("j", "Produtores mais atuantes, com lista de filmes");
    _todo("j");
}

void query_k_produtores_menos_atuantes(TDB *db) {
    (void)db;
    _print_header("k", "Produtores menos atuantes, com lista de filmes");
    _todo("k");
}

void query_l_por_decada(TDB *db) {
    (void)db;
    _print_header("l", "Consultas f-k por decada");
    /*
     * COMO IMPLEMENTAR:
     *   Reutilizar as funções f–k, mas passando como filtro
     *   adicional um intervalo de anos [decada, decada+9].
     */
    _todo("l");
}

void query_m_continuacoes(TDB *db) {
    (void)db;
    _print_header("m", "Filmes que sao continuacoes");
    /*
     * COMO IMPLEMENTAR:
     *   Heurística sugerida: filmes cujo título contém "2", "II",
     *   "Part 2", "Chapter 2", ":" ou que repetem palavras-chave
     *   do título de outro filme.
     */
    _todo("m");
}

void query_n_mesmo_ano_nascimento(TDB *db) {
    (void)db;
    _print_header("n", "Atores que nasceram no mesmo ano");
    /*
     * COMO IMPLEMENTAR:
     *   - Agrupar pessoas por ano_nascimento.
     *   - Filtrar apenas quem tem pelo menos um ACTED_IN.
     *   - Exibir os grupos com 2 ou mais atores.
     */
    _todo("n");
}

void query_o_atores_que_dirigiram(TDB *db) {
    (void)db;
    _print_header("o", "Atores que ja dirigiram");
    /*
     * COMO IMPLEMENTAR:
     *   Encontrar id_pessoa que aparece com ACTED_IN *e* DIRECTED.
     */
    _todo("o");
}

void query_p_atores_que_produziram(TDB *db) {
    (void)db;
    _print_header("p", "Atores que ja produziram");
    _todo("p");
}

void query_q_retirar_participantes_filme(TDB *db, const char *titulo_filme) {
    (void)db;
    _print_header("q", "Retirar todos os participantes de um filme");
    printf("  Filme alvo: %s\n", titulo_filme ? titulo_filme : "(nao informado)");
    /*
     * COMO IMPLEMENTAR:
     *   - Calcular hash do titulo_filme.
     *   - Buscar todas as relações com aquele id_filme.
     *   - Para cada relação encontrada, chamar btree_remover()
     *     na árvore de relações.
     */
    _todo("q");
}

void query_r_escreveu_dirigiu_produziu(TDB *db) {
    (void)db;
    _print_header("r", "Filmes escritos, dirigidos e produzidos pela mesma pessoa");
    /*
     * COMO IMPLEMENTAR:
     *   - Para cada filme, verificar se existe id_pessoa que
     *     aparece com WROTE, DIRECTED e PRODUCED.
     */
    _todo("r");
}

void query_s_dirigiu_e_produziu(TDB *db) {
    (void)db;
    _print_header("s", "Filmes dirigidos e produzidos pela mesma pessoa");
    _todo("s");
}

void query_t_nasceram_no_ano_do_filme(TDB *db) {
    (void)db;
    _print_header("t", "Atores que nasceram no ano de lancamento de um filme");
    /*
     * COMO IMPLEMENTAR:
     *   Para cada filme com ano_lancamento X, listar atores
     *   cujo ano_nascimento == X.
     */
    _todo("t");
}