// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <ctype.h>


// #include "src/Node/Movie/TMovie.h"
// #include "src/Node/Person/TPerson.h"
#include "src/services/files/readfile.h"
#include <locale.h>







int main() {
    setlocale(LC_ALL, "Portuguese");
    // Exemplo de chamada dentro do main.c:
    long raiz_filmes = -1;
    long raiz_pessoas = -1;
    long raiz_relacoes = -1;
    int t = 3; // Valor lido no menu
    int total_folhas = 0;

    readfile(&raiz_filmes, &raiz_pessoas, &raiz_relacoes, t, &total_folhas);
    return 0;
}
