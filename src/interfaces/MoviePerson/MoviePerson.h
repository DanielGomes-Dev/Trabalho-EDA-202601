typedef enum {
    ACTED_IN,   // Atuou no filme
    DIRECTED,   // Dirigiu o filme
    PRODUCED,   // Produziu o filme
    WROTE,      // Escreveu o roteiro do filme
    UNKNOWN     // Caso apareça outro tipo não mapeado
} RoleType;

typedef struct {
    long start_id;     // ID da Pessoa (geralmente)
    long end_id;       // ID do Filme (geralmente)
    RoleType role;     // O papel exercido
} Relationship;