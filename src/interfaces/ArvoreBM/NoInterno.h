#define T 3                       // Grau mínimo (exemplo: t=3)
#define MAX_CHAVES (2 * T - 1)    // Máximo de chaves por nó (5)
#define MAX_FILHOS (2 * T)        // Máximo de ponteiros/filhos (6)

typedef struct {
    int num_chaves;                 // Quantidade atual de chaves no nó
    long chaves[MAX_CHAVES];        // IDs das chaves indexadas (ex: ID da Pessoa ou Filme)
    long filhos[MAX_FILHOS];        // Offsets em bytes para os nós filhos no arquivo binário
    bool eh_folha_filho;            // TRUE se os filhos deste nó já forem os arquivos folhas
    long meu_offset;                // O offset deste próprio nó no arquivo 'indices.bin'
} NoInterno;