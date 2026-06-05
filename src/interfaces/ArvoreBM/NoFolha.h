#define T 3                       // Grau mínimo (exemplo: t=3)
#define MAX_CHAVES (2 * T - 1)    // Máximo de chaves por nó (5)

#include "../Person/Person.h"

typedef struct {
    int num_chaves;                 // Quantidade de registros nesta folha (máx: MAX_CHAVES)
    Person registros[MAX_CHAVES];   // Vetor contendo os dados estáticos das pessoas
    
    // Encadeamento da lista ligada (Árvore B+)
    char nome_arquivo_anterior[32]; // String com o nome do arquivo folha anterior (ex: "folha_001.bin")
    char nome_arquivo_proximo[32];  // String com o nome do arquivo folha próximo (ex: "folha_003.bin")
} NoFolhaPerson;