#include "person.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------
 * person_criar
 *   Preenche e retorna um TPerson.  Usa strncpy com terminação
 *   garantida para evitar buffer overflow.
 * ------------------------------------------------------------ */
TPerson person_criar(unsigned long id, const char *nome, int ano_nascimento) {
    TPerson p;

    p.id_pessoa       = id;
    p.ano_nascimento  = ano_nascimento;

    strncpy(p.nome, nome, MAX_NOME_PESSOA - 1);
    p.nome[MAX_NOME_PESSOA - 1] = '\0';

    return p;
}

/* ------------------------------------------------------------
 * person_imprimir
 *   Exibe os dados de uma pessoa de forma legível.
 * ------------------------------------------------------------ */
void person_imprimir(const TPerson *p) {
    if (!p) return;
    printf("Pessoa  | ID: %-20lu | Nome: %-30s | Nasc: %d\n",
           p->id_pessoa, p->nome, p->ano_nascimento);
}