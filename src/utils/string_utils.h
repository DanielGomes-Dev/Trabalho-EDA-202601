#ifndef STRING_UTILS_H
#define STRING_UTILS_H

/* ============================================================
 * UTILITÁRIO: Manipulação de Strings
 *
 * Funções auxiliares para parsing das linhas dos arquivos
 * Nodes.txt e Relationships.txt.
 * ============================================================ */

/* Extrai o campo de índice `indice_campo` de uma linha
   delimitada por '|', com trim de espaços.
   Retorna 1 se extraiu com sucesso, 0 caso contrário.
   `destino` recebe o resultado terminado em '\0'. */
int str_extrair_campo(const char *linha, int indice_campo,
                      char *destino, int tam_max);

/* Remove espaços/tabs/newlines do início e fim da string
   (in-place).  Retorna `s`. */
char *str_trim(char *s);

/* Verifica se a linha é completamente em branco ou vazia.
   Retorna 1 se vazia, 0 se tem conteúdo. */
int str_linha_vazia(const char *linha);

#endif /* STRING_UTILS_H */