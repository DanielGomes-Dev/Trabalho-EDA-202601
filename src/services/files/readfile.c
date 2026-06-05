#include "../../interfaces/Movie/Movie.h"
#include "../../interfaces/Person/Person.h"
#include "readfile.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../arvorebm/TARVBM.h"

#define MAX_LINHA 512

// Definição do tipo do ponteiro de função (callback)
// Recebe a string da linha limpa para que cada arquivo trate da sua forma
typedef void (*LinhaCallback)(char *linha);

typedef enum {
    TIPO_INVALIDO = 0,
    TIPO_MOVIE,
    TIPO_PERSON
} TipoNode;


// Função auxiliar para remover espaços/quebras extras se necessário
char* trim(char *str) {
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

// Esta função agora é GENÉRICA. Ela só lê e repassa o trabalho para o callback.
void read_arquivo(char *nome_arquivo, LinhaCallback callback) {
    FILE *arquivo = fopen(nome_arquivo, "r");

    // Verificação imediata do ponteiro do arquivo
    if (!arquivo) {
        printf("Erro crasso: O arquivo '%s' nao foi encontrado.\n", nome_arquivo);
        return;
    }

    char linha[MAX_LINHA];
    while (fgets(linha, sizeof(linha), arquivo)) {
        // Sanitização: remove \n e \r (essencial para compatibilidade Windows/Linux)
        linha[strcspn(linha, "\r\n")] = '\0';

        // Ignora linhas puramente vazias antes de enviar ao callback
        if (strlen(trim(linha)) == 0) {
            continue;
        }

        // Criamos uma cópia local para enviar ao callback, protegendo a string original do buffer
        char linha_copia[MAX_LINHA];
        strcpy(linha_copia, linha);

        // Invoca a função de tratamento (Callback) passando a linha limpa
        callback(linha_copia);
    }

    fclose(arquivo);
}

// ----------------------------------------------------
// Nossos Callbacks provisórios (Apenas para testar o fluxo)
// ----------------------------------------------------

void tratar_linha_nodes(char *linha) {
    // 1. Captura o primeiro token (Tipo do Node)
    char *token_tipo = strtok(linha, "|");
    if (!token_tipo) return;

    token_tipo = trim(token_tipo);
    TipoNode tipo_detectado = TIPO_INVALIDO;

    // 2. Desvia o fluxo dependendo do tipo encontrado
    if (strcmp(token_tipo, "Movie") == 0) {
        tipo_detectado = TIPO_MOVIE;

        char *token_titulo   = strtok(NULL, "|");
        char *token_ano      = strtok(NULL, "|");
        char *token_tagline  = strtok(NULL, "|");

        // Validação mínima (Título e Ano são obrigatórios)
        if (token_titulo && token_ano) {
            char *titulo = trim(token_titulo);
            int ano = atoi(trim(token_ano));
            
            // Tratamento caso o filme não tenha tagline
            char *tagline = token_tagline ? trim(token_tagline) : "";

            // Exibição de teste do que foi extraído
            printf("[PARSER] Tipo: Movie  | Titulo: %-30s | Ano: %d | Tagline: %s\n", 
                   titulo, ano, tagline);

            // TODO no próximo passo:
            // Movie m = criar_movie(titulo, ano, tagline);
            // inserir_arvore_b(m);
        }

    } else if (strcmp(token_tipo, "Person") == 0) {
        tipo_detectado = TIPO_PERSON;

        char *token_nome = strtok(NULL, "|");
        char *token_nasc = strtok(NULL, "|");

        if (token_nome) {
            char *nome = trim(token_nome);
            int ano_nascimento = 0;

            if (token_nasc) {
                ano_nascimento = atoi(trim(token_nasc));
            }

            // Exibição de teste do que foi extraído
            printf("[PARSER] Tipo: Person | Nome:   %-30s | Nasc: %d\n", 
                   nome, ano_nascimento);

            // TODO no próximo passo:
            // Person p = criar_person(nome, ano_nascimento);
            // inserir_arvore_b(p);
        }
    }

    // Se a linha estivesse corrompida ou com tipo não mapeado
    if (tipo_detectado == TIPO_INVALIDO) {
        printf("[AVISO] Linha ignorada ou inválida.\n");
    }
}

void tratar_linha_relationships(char *linha) {
    // Provisório: Apenas para ver se o callback está sendo chamado corretamente
    printf("[Callback Relationships] Recebeu: %s\n", linha);
}

// ----------------------------------------------------
// Fluxo Principal de Leitura
// ----------------------------------------------------

void readfile() {
    printf("--- Iniciando leitura de Nodes ---\n");
    read_arquivo("files/Nodes.txt", tratar_linha_nodes);

    printf("\n--- Iniciando leitura de Relationships ---\n");
    read_arquivo("files/Relationships.txt", tratar_linha_relationships);
}