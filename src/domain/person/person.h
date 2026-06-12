#ifndef PERSON_H
#define PERSON_H

/* ============================================================
 * DOMÍNIO: Pessoa (Ator, Diretor, Produtor, Escritor)
 *
 * Define a estrutura que representa qualquer pessoa do banco.
 * O campo `id_pessoa` é um hash gerado a partir do nome.
 * Todos os campos têm tamanho fixo para facilitar I/O binário.
 * ============================================================ */

#define MAX_NOME_PESSOA 100

typedef struct {
    unsigned long id_pessoa;       /* Hash do nome – chave da árvore B       */
    char          nome[MAX_NOME_PESSOA]; /* Nome completo da pessoa           */
    int           ano_nascimento;  /* Ano de nascimento (0 se desconhecido)   */
} TPerson;

/* Constrói um TPerson preenchido; imprime log se VERBOSE ativado */
TPerson person_criar(unsigned long id, const char *nome, int ano_nascimento);

/* Imprime os campos da pessoa no stdout */
void person_imprimir(const TPerson *p);

#endif /* PERSON_H */