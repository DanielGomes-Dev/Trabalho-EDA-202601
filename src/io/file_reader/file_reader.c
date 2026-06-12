#include "file_reader.h"
#include "../../utils/string_utils.h"
#include "../../core/hash/hash.h"
#include "../../domain/movie/movie.h"
#include "../../domain/person/person.h"
#include "../../domain/relation/relation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * SEÇÃO 1: PARSERS DE LINHA INDIVIDUAIS
 *   Cada função recebe UMA linha já sem '\n' e o handle do DB.
 *   Ela é a única responsável por interpretar aquele formato.
 * ============================================================ */

/* ------------------------------------------------------------
 * _parsear_linha_movie
 *   Formato esperado:
 *     Movie | <titulo> | <ano> | <tagline>
 *
 *   Se o campo titulo for vazio, a linha é ignorada.
 * ------------------------------------------------------------ */
static void _parsear_linha_movie(const char *linha, TDB *db) {
    char titulo[100]  = {0};
    char ano_str[10]  = {0};
    char tagline[150] = {0};

    /* Campo 1 = título, campo 2 = ano, campo 3 = tagline */
    str_extrair_campo(linha, 1, titulo,  sizeof(titulo));
    str_extrair_campo(linha, 2, ano_str, sizeof(ano_str));
    str_extrair_campo(linha, 3, tagline, sizeof(tagline));

    if (strlen(titulo) == 0) return;  /* Linha inválida */

    int ano = atoi(ano_str);
    printf("[PARSER] Movie  | %-30s | %d\n", titulo, ano);

    unsigned long id = hash_string(titulo);
    TMovie m = movie_criar(id, titulo, ano, tagline);
    db_inserir_filme(db, &m);
}

/* ------------------------------------------------------------
 * _parsear_linha_person
 *   Formato esperado:
 *     Person | <nome> | <ano_nascimento>
 * ------------------------------------------------------------ */
static void _parsear_linha_person(const char *linha, TDB *db) {
    char nome[100]   = {0};
    char nasc_str[10]= {0};

    str_extrair_campo(linha, 1, nome,     sizeof(nome));
    str_extrair_campo(linha, 2, nasc_str, sizeof(nasc_str));

    if (strlen(nome) == 0) return;  /* Linha inválida */

    int ano_nasc = atoi(nasc_str);
    printf("[PARSER] Person | %-30s | %d\n", nome, ano_nasc);

    unsigned long id = hash_string(nome);
    TPerson p = person_criar(id, nome, ano_nasc);
    db_inserir_pessoa(db, &p);
}

/* ------------------------------------------------------------
 * _parsear_linha_node
 *   Roteador: lê o campo 0 (tipo) e chama o parser correto.
 *   Tipos conhecidos: "Movie", "Person".
 * ------------------------------------------------------------ */
static void _parsear_linha_node(const char *linha, TDB *db) {
    char tipo[30] = {0};
    str_extrair_campo(linha, 0, tipo, sizeof(tipo));

    if      (strcmp(tipo, "Movie")  == 0) _parsear_linha_movie(linha, db);
    else if (strcmp(tipo, "Person") == 0) _parsear_linha_person(linha, db);
    /* Outros tipos desconhecidos são silenciosamente ignorados */
}

/* ------------------------------------------------------------
 * _parsear_linha_relacao
 *   Formato esperado:
 *     START Person | <nome> | <papel> | END Movie | <titulo> | <info>
 *
 *   Campos:
 *     0 = "START Person"
 *     1 = nome da pessoa
 *     2 = papel (ACTED_IN, DIRECTED, PRODUCED, WROTE)
 *     3 = "END Movie"   (ignorado)
 *     4 = título do filme
 *     5 = informação adicional (opcional)
 * ------------------------------------------------------------ */
static void _parsear_linha_relacao(const char *linha, TDB *db) {
    char start_token[30] = {0};
    str_extrair_campo(linha, 0, start_token, sizeof(start_token));

    /* Garante que a linha seja do tipo correto */
    if (strcmp(start_token, "START Person") != 0) return;

    char nome[100]  = {0};
    char papel[50]  = {0};
    char titulo[100]= {0};
    char info[100]  = {0};

    str_extrair_campo(linha, 1, nome,   sizeof(nome));
    str_extrair_campo(linha, 2, papel,  sizeof(papel));
    str_extrair_campo(linha, 4, titulo, sizeof(titulo));
    str_extrair_campo(linha, 5, info,   sizeof(info));

    if (strlen(nome) == 0 || strlen(titulo) == 0) return;

    printf("[PARSER] Relacao | %-20s | %-10s | %s\n", nome, papel, titulo);

    unsigned long id_pessoa = hash_string(nome);
    unsigned long id_filme  = hash_string(titulo);
    TRelation r = relation_criar(id_pessoa, id_filme, papel, info);
    db_inserir_relacao(db, &r);
}

/* ============================================================
 * SEÇÃO 2: FUNÇÕES DE LEITURA DE ARQUIVO
 *   Cada função abre seu arquivo, itera linha a linha e
 *   chama o parser correspondente.
 * ============================================================ */

/* ------------------------------------------------------------
 * file_reader_ler_nodes
 *   Abre Nodes.txt e processa cada linha não vazia.
 * ------------------------------------------------------------ */
void file_reader_ler_nodes(TDB *db) {
    FILE *f = fopen(FILE_NODES, "r");
    if (!f) {
        fprintf(stderr, "[ERRO] Arquivo '%s' nao encontrado.\n", FILE_NODES);
        return;
    }

    printf("\n--- Lendo %s ---\n", FILE_NODES);
    char linha[MAX_LINHA];

    while (fgets(linha, sizeof(linha), f)) {
        /* Remove '\r' e '\n' do final */
        linha[strcspn(linha, "\r\n")] = '\0';

        /* Pula linhas em branco */
        if (str_linha_vazia(linha)) continue;

        _parsear_linha_node(linha, db);
    }

    fclose(f);
    printf("--- Fim de %s ---\n\n", FILE_NODES);
}

/* ------------------------------------------------------------
 * file_reader_ler_relacoes
 *   Abre Relationships.txt e processa cada linha não vazia.
 * ------------------------------------------------------------ */
void file_reader_ler_relacoes(TDB *db) {
    FILE *f = fopen(FILE_RELATIONSHIPS, "r");
    if (!f) {
        fprintf(stderr, "[ERRO] Arquivo '%s' nao encontrado.\n", FILE_RELATIONSHIPS);
        return;
    }

    printf("\n--- Lendo %s ---\n", FILE_RELATIONSHIPS);
    char linha[MAX_LINHA];

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (str_linha_vazia(linha)) continue;
        _parsear_linha_relacao(linha, db);
    }

    fclose(f);
    printf("--- Fim de %s ---\n\n", FILE_RELATIONSHIPS);
}

/* ------------------------------------------------------------
 * file_reader_carregar_tudo
 *   Pipeline completo: primeiro nodes, depois relações.
 *   As relações dependem dos IDs gerados pelos nodes.
 * ------------------------------------------------------------ */
void file_reader_carregar_tudo(TDB *db) {
    if (!db) return;
    file_reader_ler_nodes(db);
    file_reader_ler_relacoes(db);
}