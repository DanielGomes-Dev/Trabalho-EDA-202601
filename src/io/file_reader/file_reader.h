#ifndef FILE_READER_H
#define FILE_READER_H

/* ============================================================
 * LEITOR DE ARQUIVOS
 *
 * Responsável por ler Nodes.txt e Relationships.txt,
 * fazer o parsing de cada linha e popular o banco de dados.
 *
 * Cada função tem uma única responsabilidade:
 *   - file_reader_ler_nodes()       → lê e processa Nodes.txt
 *   - file_reader_ler_relacoes()    → lê e processa Relationships.txt
 *   - file_reader_carregar_tudo()   → chama ambas em sequência
 * ============================================================ */

#include "../../io/db.h"

#define FILE_NODES         "files/Nodes.txt"
#define FILE_RELATIONSHIPS "files/Relationships.txt"
#define MAX_LINHA          512

/* Lê e indexa todos os registros de Nodes.txt */
void file_reader_ler_nodes(TDB *db);

/* Lê e indexa todos os registros de Relationships.txt */
void file_reader_ler_relacoes(TDB *db);

/* Chama as duas leituras em sequência (pipeline completo) */
void file_reader_carregar_tudo(TDB *db);

#endif /* FILE_READER_H */