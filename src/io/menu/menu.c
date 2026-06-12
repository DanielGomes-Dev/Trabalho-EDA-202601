#include "menu.h"
#include "../../query/query.h"
#include "../../core/hash/hash.h"
#include "../../io/file_reader/file_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * SEÇÃO 1: UTILITÁRIOS DE I/O DO MENU
 * ============================================================ */

/* Limpa o buffer de entrada até '\n' ou EOF */
static void _limpar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Lê uma linha do stdin com tamanho máximo `tam` */
static void _ler_linha(char *buf, int tam) {
    if (fgets(buf, tam, stdin)) {
        buf[strcspn(buf, "\r\n")] = '\0';
    }
}

/* ============================================================
 * SEÇÃO 2: SUB-MENU DE OPERAÇÕES
 * ============================================================ */

void menu_operacoes(TDB *db) {
    int opcao;

    do {
        printf("\n--- OPERACOES NA ARVORE B ---\n");
        printf("  1. Inserir ator/diretor/produtor\n");
        printf("  2. Inserir filme\n");
        printf("  3. Remover pessoa\n");
        printf("  4. Remover filme\n");
        printf("  5. Buscar pelo nome\n");
        printf("  6. Imprimir arvore B (formato visual)\n");
        printf("  7. Imprimir chaves em ordem\n");
        printf("  0. Voltar\n");
        printf("Opcao: ");

        if (scanf("%d", &opcao) != 1) { _limpar_buffer(); opcao = -1; }
        _limpar_buffer();

        char nome[100];
        unsigned long id;

        switch (opcao) {
            case 1: {
                /* Insere uma pessoa informada pelo usuário */
                printf("Nome: ");
                _ler_linha(nome, sizeof(nome));
                int ano;
                printf("Ano de nascimento (0 se desconhecido): ");
                scanf("%d", &ano); _limpar_buffer();

                id = hash_string(nome);
                TPerson p = person_criar(id, nome, ano);
                int ret = db_inserir_pessoa(db, &p);
                if (ret == 0)
                    printf("  [AVISO] Pessoa ja existia no indice.\n");
                else
                    printf("  [OK] Pessoa inserida.\n");
                break;
            }
            case 2: {
                /* Insere um filme informado pelo usuário */
                char titulo[100], tagline[150];
                int ano;
                printf("Titulo: ");
                _ler_linha(titulo, sizeof(titulo));
                printf("Ano de lancamento: ");
                scanf("%d", &ano); _limpar_buffer();
                printf("Tagline (pode deixar vazio): ");
                _ler_linha(tagline, sizeof(tagline));

                id = hash_string(titulo);
                TMovie m = movie_criar(id, titulo, ano, tagline);
                int ret = db_inserir_filme(db, &m);
                if (ret == 0)
                    printf("  [AVISO] Filme ja existia no indice.\n");
                else
                    printf("  [OK] Filme inserido.\n");
                break;
            }
            case 3: {
                printf("Nome da pessoa a remover: ");
                _ler_linha(nome, sizeof(nome));
                id = hash_string(nome);
                db_remover_pessoa(db, id);
                break;
            }
            case 4: {
                printf("Titulo do filme a remover: ");
                _ler_linha(nome, sizeof(nome));
                id = hash_string(nome);
                db_remover_filme(db, id);
                break;
            }
            case 5: {
                /* Busca por nome (pessoa ou filme) */
                printf("Nome/titulo para buscar: ");
                _ler_linha(nome, sizeof(nome));
                id = hash_string(nome);

                long off_p = db_buscar_pessoa(db, id);
                long off_f = db_buscar_filme(db, id);

                if (off_p != -1)
                    printf("  Pessoa encontrada (offset=%ld)\n", off_p);
                else if (off_f != -1)
                    printf("  Filme encontrado  (offset=%ld)\n", off_f);
                else
                    printf("  Nao encontrado.\n");
                break;
            }
            case 6:
                db_imprimir_arvores(db);
                break;
            case 7:
                printf("\n=== CHAVES EM ORDEM - FILMES ===\n");
                btree_imprimir_chaves_em_ordem(db->filmes);
                printf("\n=== CHAVES EM ORDEM - PESSOAS ===\n");
                btree_imprimir_chaves_em_ordem(db->pessoas);
                printf("\n=== CHAVES EM ORDEM - RELACOES ===\n");
                btree_imprimir_chaves_em_ordem(db->relacoes);
                break;
            case 0:
                break;
            default:
                printf("  Opcao invalida.\n");
        }
    } while (opcao != 0);
}

/* ============================================================
 * SEÇÃO 3: SUB-MENU DE CONSULTAS (a–t)
 * ============================================================ */

void menu_consultas(TDB *db) {
    int opcao;

    do {
        printf("\n--- CONSULTAS ---\n");
        printf("  a. Pessoas que trabalharam juntas\n");
        printf("  b. Atores e diretores juntos\n");
        printf("  c. Atores que atuaram juntos\n");
        printf("  d. Atores juntos por decada\n");
        printf("  e. Atores e diretores juntos por decada\n");
        printf("  f. Atores que mais atuaram\n");
        printf("  g. Atores que menos atuaram\n");
        printf("  h. Diretores que mais dirigiram\n");
        printf("  i. Diretores que menos dirigiram\n");
        printf("  j. Produtores mais atuantes\n");
        printf("  k. Produtores menos atuantes\n");
        printf("  l. Consultas f-k por decada\n");
        printf("  m. Filmes que sao continuacoes\n");
        printf("  n. Atores nascidos no mesmo ano\n");
        printf("  o. Atores que dirigiram\n");
        printf("  p. Atores que produziram\n");
        printf("  q. Retirar participantes de um filme\n");
        printf("  r. Filmes escritos, dirigidos e produzidos pela mesma pessoa\n");
        printf("  s. Filmes dirigidos e produzidos pela mesma pessoa\n");
        printf("  t. Atores nascidos no ano de lancamento de um filme\n");
        printf("  0. Voltar\n");
        printf("Opcao: ");

        char letra[4];
        _ler_linha(letra, sizeof(letra));

        if (strcmp(letra, "0") == 0) { opcao = 0; break; }

        opcao = 1; /* mantém o loop */
        switch (letra[0]) {
            case 'a': query_a_trabalharam_juntos(db); break;
            case 'b': query_b_atores_diretores_juntos(db); break;
            case 'c': query_c_atores_juntos(db); break;
            case 'd': query_d_atores_juntos_por_decada(db); break;
            case 'e': query_e_atores_diretores_juntos_por_decada(db); break;
            case 'f': query_f_atores_mais_atuaram(db); break;
            case 'g': query_g_atores_menos_atuaram(db); break;
            case 'h': query_h_diretores_mais_dirigiram(db); break;
            case 'i': query_i_diretores_menos_dirigiram(db); break;
            case 'j': query_j_produtores_mais_atuantes(db); break;
            case 'k': query_k_produtores_menos_atuantes(db); break;
            case 'l': query_l_por_decada(db); break;
            case 'm': query_m_continuacoes(db); break;
            case 'n': query_n_mesmo_ano_nascimento(db); break;
            case 'o': query_o_atores_que_dirigiram(db); break;
            case 'p': query_p_atores_que_produziram(db); break;
            case 'q': {
                char titulo[100];
                printf("Titulo do filme: ");
                _ler_linha(titulo, sizeof(titulo));
                query_q_retirar_participantes_filme(db, titulo);
                break;
            }
            case 'r': query_r_escreveu_dirigiu_produziu(db); break;
            case 's': query_s_dirigiu_e_produziu(db); break;
            case 't': query_t_nasceram_no_ano_do_filme(db); break;
            default:  printf("  Opcao invalida.\n");
        }
    } while (opcao != 0);
}

/* ============================================================
 * SEÇÃO 4: MENU PRINCIPAL
 * ============================================================ */

void menu_principal(TDB *db) {
    int opcao;

    printf("\n");
    printf("==========================================================\n");
    printf("  Sistema de Gestao de Filmes - Hollywood Talent Agency\n");
    printf("  Estruturas de Dados e Seus Algoritmos - 2026.1\n");
    printf("==========================================================\n");

    do {
        printf("\n--- MENU PRINCIPAL ---\n");
        printf("  1. Carregar dados dos arquivos\n");
        printf("  2. Operacoes na arvore B\n");
        printf("  3. Consultas (a–t)\n");
        printf("  4. Imprimir arvores\n");
        printf("  0. Sair\n");
        printf("Opcao: ");

        if (scanf("%d", &opcao) != 1) { _limpar_buffer(); opcao = -1; }
        _limpar_buffer();

        switch (opcao) {
            case 1: {
                /* Permite ao usuário escolher o valor de t */
                printf("Grau minimo t da arvore B [atual: %d, 0=manter]: ", db->t);
                int novo_t;
                if (scanf("%d", &novo_t) == 1 && novo_t >= 2) {
                    /* Reabre as árvores com o novo t (descarta índices anteriores) */
                    printf("[AVISO] Recriar arvores com t=%d?\n", novo_t);
                }
                _limpar_buffer();

                file_reader_carregar_tudo(db);
                break;
            }
            case 2:
                menu_operacoes(db);
                break;
            case 3:
                menu_consultas(db);
                break;
            case 4:
                db_imprimir_arvores(db);
                break;
            case 0:
                printf("Encerrando...\n");
                break;
            default:
                printf("  Opcao invalida.\n");
        }
    } while (opcao != 0);
}