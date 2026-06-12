#include "hash.h"

/* ------------------------------------------------------------
 * hash_string  (algoritmo djb2)
 *   Percorre cada byte da string e combina com a hash atual.
 *   hash = hash * 33 + c   (equivalente a hash<<5 + hash + c)
 * ------------------------------------------------------------ */
unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/* ------------------------------------------------------------
 * hash_para_int
 *   Trunca o hash para caber em um int com sinal.
 *   Usa o operador módulo com o maior primo que cabe em int
 *   positivo para reduzir colisões no espaço 31-bit.
 * ------------------------------------------------------------ */
int hash_para_int(unsigned long hash) {
    /* 2147483647 == 2^31 - 1 (maior primo de Mersenne 31-bit) */
    return (int)(hash % 2147483647UL);
}