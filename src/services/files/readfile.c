#include "readfile.h"

#define MAX_LINHA 512

// Função auxiliar para remover espaços em branco do início e fim de uma string
void limpar_espacos(char *str) {
    int inicio = 0;
    while (isspace((unsigned char)str[inicio])) {
        inicio++;
    }
    
    int fim = strlen(str) - 1;
    while (fim >= inicio && isspace((unsigned char)str[fim])) {
        fim--;
    }
    
    int i;
    for (i = inicio; i <= fim; i++) {
        str[i - inicio] = str[i];
    }
    str[i - inicio] = '\0';
}


void readfile (){
    FILE *arquivo = fopen("Nodes.txt", "r");


    char linha[MAX_LINHA];
    
    printf("--- PROCESSANDO REGISTROS HOLLYWOOD ---\n\n");

    while (fgets(linha, sizeof(linha), arquivo)) {
        // 1. Limpeza inicial do rastro de tags do arquivo
        remover_source(linha);
        limpar_espacos(linha);
        
        // Se a linha ficou vazia após a limpeza, pula
        if (strlen(linha) == 0) continue;

        // 2. Extração do primeiro token (Tipo do Nó) para direcionamento
        char *token = strtok(linha, "|");
        if (token == NULL) continue;
        
        char tipo[20];
        strcpy(tipo, token);
        limpar_espacos(tipo);

        // 3. Processamento modular baseado no tipo verificado
        if (strcmp(tipo, "Movie") == 0) {
            get_movie();
        } else if (strcmp(tipo, "Person") == 0) {
            get_person();
        }
    }

    fclose(arquivo);

}