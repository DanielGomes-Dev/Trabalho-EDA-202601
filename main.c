/* ============================================================
 * main.c – Ponto de entrada do programa
 *
 * Responsabilidade: única – inicializar o banco, chamar o
 * carregamento de dados e entregar o controle ao menu.
 *
 * O main NÃO faz parsing, NÃO manipula árvores diretamente
 * e NÃO imprime resultados de consulta.  Cada uma dessas
 * responsabilidades pertence ao módulo correspondente.
 *
 * Fluxo de execução:
 *   1. Configura locale para suporte a acentos
 *   2. Abre o banco de dados (3 árvores B em disco)
 *   3. Carrega os arquivos Nodes.txt e Relationships.txt
 *   4. Exibe o menu interativo
 *   5. Fecha o banco e libera memória
 * ============================================================ */

#include <stdio.h>
#include <locale.h>

#include "src/io/db.h"
#include "src/io/file_reader/file_reader.h"
#include "src/io/menu/menu.h"

/* Grau mínimo padrão da árvore B (pode ser alterado no menu) */
#define T_PADRAO 3

int main(void) {
    /* Passo 1: configura locale para UTF-8 / acentuação */
    setlocale(LC_ALL, "");

    /* Passo 2: abre o banco de dados com grau mínimo T_PADRAO */
    TDB *db = db_abrir(T_PADRAO);
    if (!db) {
        fprintf(stderr, "[ERRO FATAL] Nao foi possivel abrir o banco de dados.\n");
        return 1;
    }

    /* Passo 3: carrega os dados dos arquivos texto */
    file_reader_carregar_tudo(db);

    /* Passo 4: exibe o menu interativo até o usuário sair */
    menu_principal(db);

    /* Passo 5: fecha o banco e libera recursos */
    db_fechar(db);

    return 0;
}