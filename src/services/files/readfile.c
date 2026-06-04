#include "../../interfaces/Movie/TMovie.h"
#include "../../interfaces/Person/TPerson.h"
#include "readfile.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINHA 512

// Remove espaços em branco do início e fim de uma string (Versão otimizada com ponteiros)
void limpar_espacos(char *str) {
    char *inicio = str;
    while (isspace((unsigned char)*inicio)) inicio++;

    if (inicio != str) {
        memmove(str, inicio, strlen(inicio) + 1);
    }

    int fim = strlen(str) - 1;
    while (fim >= 0 && isspace((unsigned char)str[fim])) {
        str[fim--] = '\0';
    }
}

// Função auxiliar interna para isolar a repetição do strtok + strcpy + limpar_espacos
static void extrair_campo(char *destino, size_t tamanho_max) {
    char *token = strtok(NULL, "|");
    if (token) {
        strncpy(destino, token, tamanho_max - 1);
        destino[tamanho_max - 1] = '\0';
        limpar_espacos(destino);
    } else {
        destino[0] = '\0';
    }
}

TMovie get_movie(char *linha) {
    TMovie filme;
    char titulo[100], ano_str[10], tagline[200];

    // Descarta o prefixo "Movie" e inicia o fatiamento
    strtok(linha, "|");

    // Extração limpa dos campos usando a função auxiliar
    extrair_campo(titulo, sizeof(titulo));
    extrair_campo(ano_str, sizeof(ano_str));
    extrair_campo(tagline, sizeof(tagline));

    filme.ano_lancamento = atoi(ano_str);
    strcpy(filme.titulo, titulo);
    strcpy(filme.tagline, tagline);
    
    printf("[FILME] Titulo: %s | Ano: %d | Slogan: %s\n\n", titulo, filme.ano_lancamento, tagline);
    return filme;
}

TPerson get_person(char *linha) {
    TPerson pessoa;
    char nome[100], nasc_str[50];

    // Descarta o prefixo "Person" e inicia o fatiamento
    strtok(linha, "|");

    extrair_campo(nome, sizeof(nome));
    extrair_campo(nasc_str, sizeof(nasc_str));

    if (strstr(nasc_str, "no birth year") || strlen(nasc_str) == 0) {
        pessoa.ano_nascimento = -1;
        printf("[PESSOA] Nome: %s | Nascimento: Nao Informado\n\n", nome);
    } else {
        pessoa.ano_nascimento = atoi(nasc_str);
        printf("[PESSOA] Nome: %s | Nascimento: %d\n\n", nome, pessoa.ano_nascimento);
    }

    strcpy(pessoa.nome, nome);
    return pessoa;
}

void get_type(char *line, char *tipo_destino) {
    char *posicao_pipe = strchr(line, '|');
    
    if (posicao_pipe != NULL) {
        int tamanho_tipo = posicao_pipe - line;
        if (tamanho_tipo > 19) tamanho_tipo = 19;

        strncpy(tipo_destino, line, tamanho_tipo);
        tipo_destino[tamanho_tipo] = '\0';
        limpar_espacos(tipo_destino);
    } else {
        tipo_destino[0] = '\0'; 
    }
}

void readfile() {
    FILE *arquivo = fopen("files/Nodes.txt", "r");
    if (!arquivo) {
        printf("Erro crasso: O arquivo 'files/Nodes.txt' nao foi encontrado.\n");
        return;
    }

    char linha[MAX_LINHA];
    char tipo[20];
    
    printf("--- PROCESSANDO REGISTROS HOLLYWOOD ---\n\n");

    while (fgets(linha, sizeof(linha), arquivo)) {
        linha[strcspn(linha, "\n")] = '\0'; // Sanitiza o fim da linha
        
        get_type(linha, tipo);

        if (strcmp(tipo, "Movie") == 0) {
            TMovie filme = get_movie(linha);
            // Aqui entra sua árvore B+ ou Lista
        } else if (strcmp(tipo, "Person") == 0) {
            TPerson pessoa = get_person(linha); // Corrigido o tipo da variável recebida (era TMovie por engano)
            // Aqui entra sua árvore B+ ou Lista
        }
    }

    fclose(arquivo);
}