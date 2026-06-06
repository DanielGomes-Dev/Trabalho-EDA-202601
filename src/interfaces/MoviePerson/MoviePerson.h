// Como representar um Relacionamento (o que está no outro arquivo)
typedef struct {
    unsigned long id_pessoa;      // Quem fez
    unsigned long id_filme;       // Em qual filme
    char papel[50];               // "ACTED_IN", "DIRECTED", etc.
} TMoviePerson;