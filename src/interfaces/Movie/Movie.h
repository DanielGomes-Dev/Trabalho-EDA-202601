// Como representar um Filme no banco de dados
typedef struct {
    unsigned long id_filme;       // O Hash do Título
    char titulo[100];             // Espaço fixo para o título
    int ano_lancamento;
} TMovie;