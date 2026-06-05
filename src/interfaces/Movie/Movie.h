#define MAX_TITLE 150
#define MAX_TAGLINE 200

typedef struct {
    long id;                  // ID único do filme
    char title[MAX_TITLE];    // Título do filme
    int released;             // Ano de lançamento
    char tagline[MAX_TAGLINE];// Slogan/Frase de efeito do filme
} Movie;