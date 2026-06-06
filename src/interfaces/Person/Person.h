#define MAX_NAME 100
// Como representar uma Pessoa no banco de dados
typedef struct {
    unsigned long id_pessoa;      // O Hash do Nome (Gerado pela função da professora)
    char nome[100];               // Espaço fixo para o nome
    int ano_nascimento;
} TPerson;