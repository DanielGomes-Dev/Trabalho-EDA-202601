#define MAX_NAME 100

typedef struct {
    long id;               // ID único da pessoa (ex: 1, 2, 99)
    char name[MAX_NAME];   // Nome da pessoa (completado com '\0')
    int born;              // Ano de nascimento (0 se não informado)
} Person;