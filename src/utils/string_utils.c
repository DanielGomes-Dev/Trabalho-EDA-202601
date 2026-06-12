#include "string_utils.h"
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------
 * str_extrair_campo
 *   Percorre `linha` contando separadores '|'.
 *   Ao alcançar `indice_campo`, copia o texto até o próximo
 *   '|' ou '\0', aplicando trim nas duas extremidades.
 * ------------------------------------------------------------ */
int str_extrair_campo(const char *linha, int indice_campo,
                      char *destino, int tam_max) {
    int i = 0, campo_atual = 0, j = 0;
    destino[0] = '\0';

    /* Pula os campos anteriores */
    while (linha[i] != '\0' && campo_atual < indice_campo) {
        if (linha[i] == '|') campo_atual++;
        i++;
    }

    /* Não encontrou o campo */
    if (campo_atual != indice_campo || linha[i] == '\0') return 0;

    /* Left trim */
    while (linha[i] != '\0' && linha[i] != '|' && isspace((unsigned char)linha[i]))
        i++;

    /* Copia o conteúdo */
    while (linha[i] != '\0' && linha[i] != '|') {
        if (j < tam_max - 1) destino[j++] = linha[i];
        i++;
    }
    destino[j] = '\0';

    /* Right trim */
    j--;
    while (j >= 0 && isspace((unsigned char)destino[j])) {
        destino[j] = '\0';
        j--;
    }

    return (strlen(destino) > 0);
}

/* ------------------------------------------------------------
 * str_trim
 *   Modifica a string in-place: remove whitespace no início
 *   (movendo o ponteiro implicitamente via memmove) e no fim.
 * ------------------------------------------------------------ */
char *str_trim(char *s) {
    if (!s) return s;

    /* Right trim */
    int n = (int)strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[--n] = '\0';
    }

    /* Left trim: desloca os caracteres */
    int inicio = 0;
    while (s[inicio] && isspace((unsigned char)s[inicio])) inicio++;
    if (inicio > 0) memmove(s, s + inicio, n - inicio + 1);

    return s;
}

/* ------------------------------------------------------------
 * str_linha_vazia
 *   Retorna 1 se a string for NULL, vazia, ou só whitespace.
 * ------------------------------------------------------------ */
int str_linha_vazia(const char *linha) {
    if (!linha) return 1;
    while (*linha) {
        if (!isspace((unsigned char)*linha)) return 0;
        linha++;
    }
    return 1;
}