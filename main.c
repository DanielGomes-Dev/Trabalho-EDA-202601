// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <ctype.h>


// #include "src/Node/Movie/TMovie.h"
// #include "src/Node/Person/TPerson.h"
#include "src/services/files/readfile.h"




// // Função auxiliar para remover marcações do tipo "" caso existam na linha
// void remover_source(char *linha) {
//     char *ptr = strchr(linha, ']');
//     if (ptr != NULL && linha[0] == '[') {
//         // Move o ponteiro para o início real do dado após o conteúdo de source
//         memmove(linha, ptr + 1, strlen(ptr + 1) + 1);
//     }
// }

// void get_movie() {
//     char titulo[100] = "";
//     char ano_str[10] = "";
//     char tagline[200] = "";
//     char *token;

//     // Captura o Título
//     token = strtok(NULL, "|");
//     if (token) { 
//         strcpy(titulo, token); 
//         limpar_espacos(titulo); 
//     }

//     // Captura o Ano
//     token = strtok(NULL, "|");
//     if (token) { 
//         strcpy(ano_str, token); 
//         limpar_espacos(ano_str); 
//     }

//     // Captura a Tagline
//     token = strtok(NULL, "|");
//     if (token) { 
//         strcpy(tagline, token); 
//         limpar_espacos(tagline); 
//     }

//     int ano = atoi(ano_str);

//     // Exibição formatada do dado estruturado resultante
//     printf("[FILME] Titulo: %s | Ano: %d | Slogan: %s\n\n", titulo, ano, tagline);
// }


// void get_person() {
//     char nome[100] = "";
//     char nasc_str[50] = "";
//     char *token;

//     // Captura o Nome
//     token = strtok(NULL, "|");
//     if (token) { 
//         strcpy(nome, token); 
//         limpar_espacos(nome); 
//     }

//     // Captura o Ano de Nascimento
//     token = strtok(NULL, "|");
//     if (token) { 
//         strcpy(nasc_str, token); 
//         limpar_espacos(nasc_str); 
//     }

//     int ano_nasc = 0;
//     if (strstr(nasc_str, "no birth year") != NULL || strlen(nasc_str) == 0) {
//         ano_nasc = -1; // Código padrão para ano não informado
//     } else {
//         ano_nasc = atoi(nasc_str);
//     }

//     // Exibição formatada do dado estruturado resultante
//     if (ano_nasc != -1) {
//         printf("[PESSOA] Nome: %s | Nascimento: %d\n", nome, ano_nasc);
//     } else {
//         printf("[PESSOA] Nome: %s | Nascimento: Não Informado\n", nome);
//     }
// }


int main() {

    readfile();
    return 0;
}
