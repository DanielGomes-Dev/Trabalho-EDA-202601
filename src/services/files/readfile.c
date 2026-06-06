#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINHA 512

// ============================================================================
// 1. DEFINIÇÕES DE TIPOS E INTERFACES (Simulando as suas .h)
// ============================================================================

typedef struct {
    unsigned long id;
    char titulo[100];
    int ano;
    char tagline[150];
} TMovie;

typedef struct {
    unsigned long id;
    char nome[100];
    int ano_nascimento;
} TPerson;

typedef struct {
    unsigned long id_pessoa;
    unsigned long id_filme;
    char papel[50];         
    char info_adicional[100];
} TMoviePerson;

typedef enum { TIPO_INVALIDO = 0, REG_FILME, REG_PESSOA, REG_RELACIONAMENTO } TipoRegistro;

typedef struct {
    TipoRegistro tipo;
    union {
        TMovie filme;
        TPerson pessoa;
        TMoviePerson rel;
    } conteudo;
} TRegister;

// Definição do tipo do ponteiro de função (callback)
typedef void (*LinhaCallback)(char *linha);

// ============================================================================
// 2. IMPLEMENTAÇÃO DAS FUNÇÕES CONSTRUTORAS (Criação das Structs)
// ============================================================================

TMovie criar_movie(unsigned long id, const char* titulo, int ano, const char* tagline) {
    TMovie m;
    m.id = id;
    m.ano = ano;
    
    strncpy(m.titulo, titulo, sizeof(m.titulo) - 1);
    m.titulo[sizeof(m.titulo) - 1] = '\0';
    
    strncpy(m.tagline, tagline, sizeof(m.tagline) - 1);
    m.tagline[sizeof(m.tagline) - 1] = '\0';
    
    printf("   [STRUCT CRIADA] Filme: %s (ID: %lu)\n", m.titulo, m.id);
    return m;
}

TPerson criar_person(unsigned long id, const char* nome, int ano_nascimento) {
    TPerson p;
    p.id = id;
    p.ano_nascimento = ano_nascimento;
    
    strncpy(p.nome, nome, sizeof(p.nome) - 1);
    p.nome[sizeof(p.nome) - 1] = '\0';
    
    printf("   [STRUCT CRIADA] Pessoa: %s (ID: %lu)\n", p.nome, p.id);
    return p;
}

TMoviePerson criar_movie_person(unsigned long id_pessoa, unsigned long id_filme, const char* papel, const char* info_adicional) {
    TMoviePerson mp;
    mp.id_pessoa = id_pessoa;
    mp.id_filme = id_filme;
    
    strncpy(mp.papel, papel, sizeof(mp.papel) - 1);
    mp.papel[sizeof(mp.papel) - 1] = '\0';
    
    strncpy(mp.info_adicional, info_adicional, sizeof(mp.info_adicional) - 1);
    mp.info_adicional[sizeof(mp.info_adicional) - 1] = '\0';
    
    printf("   [STRUCT CRIADA] Relacao: Pessoa %lu -> %s -> Filme %lu\n", mp.id_pessoa, mp.papel, mp.id_filme);
    return mp;
}

// ============================================================================
// 3. FUNÇÕES UTILITÁRIAS E PARSER
// ============================================================================

// --- Função de Hash Polinomial ---
unsigned long gerar_hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Função auxiliar para remover espaços/quebras extras
char* trim(char *str) {
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

// Função genérica de leitura de arquivos texto
void read_arquivo(char *nome_arquivo, LinhaCallback callback) {
    FILE *arquivo = fopen(nome_arquivo, "r");

    if (!arquivo) {
        printf("Erro crasso: O arquivo '%s' nao foi encontrado.\n", nome_arquivo);
        return;
    }

    char linha[MAX_LINHA];
    while (fgets(linha, sizeof(linha), arquivo)) {
        linha[strcspn(linha, "\r\n")] = '\0';

        if (strlen(trim(linha)) == 0) {
            continue;
        }

        char linha_copia[MAX_LINHA];
        strcpy(linha_copia, linha);

        callback(linha_copia);
    }

    fclose(arquivo);
}

// ----------------------------------------------------
// Tratamento do arquivo Nodes.txt
// ----------------------------------------------------
void tratar_linha_nodes(char *linha) {
    char *token_tipo = strtok(linha, "|");
    if (!token_tipo) return;

    token_tipo = trim(token_tipo);
    TipoRegistro tipo_detectado = TIPO_INVALIDO;

    if (strcmp(token_tipo, "Movie") == 0) {
        tipo_detectado = REG_FILME;

        char *token_titulo   = strtok(NULL, "|");
        char *token_ano      = strtok(NULL, "|");
        char *token_tagline  = strtok(NULL, "|");

        if (token_titulo && token_ano) {
            char *titulo = trim(token_titulo);
            int ano = atoi(trim(token_ano));
            char *tagline = token_tagline ? trim(token_tagline) : "";

            printf("[PARSER] Tipo: Movie  | Titulo: %-30s | Ano: %d\n", titulo, ano);

            TRegister reg;
            reg.tipo = REG_FILME;
            
            unsigned long id = gerar_hash(titulo);
            reg.conteudo.filme = criar_movie(id, titulo, ano, tagline);

            // TODO no futuro: inserir_arvore_b(reg);
        }

    } else if (strcmp(token_tipo, "Person") == 0) {
        tipo_detectado = REG_PESSOA;

        char *token_nome = strtok(NULL, "|");
        char *token_nasc = strtok(NULL, "|");

        if (token_nome) {
            char *nome = trim(token_nome);
            int ano_nascimento = token_nasc ? atoi(trim(token_nasc)) : 0;

            printf("[PARSER] Tipo: Person | Nome:   %-30s | Nasc: %d\n", nome, ano_nascimento);

            TRegister reg;
            reg.tipo = REG_PESSOA;
            
            unsigned long id = gerar_hash(nome);
            reg.conteudo.pessoa = criar_person(id, nome, ano_nascimento);

            // TODO no futuro: inserir_arvore_b(reg);
        }
    }

    if (tipo_detectado == TIPO_INVALIDO) {
        printf("[AVISO] Linha ignorada ou inválida em Nodes.\n");
    }
}

// ----------------------------------------------------
// Tratamento do arquivo Relationships.txt
// ----------------------------------------------------
void tratar_linha_relationships(char *linha) {
    char *start_token = strtok(linha, "|");
    if (!start_token || strcmp(trim(start_token), "START Person") != 0) return;

    char *token_nome   = strtok(NULL, "|");
    char *token_papel  = strtok(NULL, "|");
    char *end_movie    = strtok(NULL, "|"); 
    char *token_filme  = strtok(NULL, "|");
    char *token_info   = strtok(NULL, "|"); 

    if (token_nome && token_papel && token_filme) {
        char *nome = trim(token_nome);
        char *papel = trim(token_papel);
        char *filme = trim(token_filme);
        char *info_adicional = token_info ? trim(token_info) : "";

        printf("[PARSER] Relacao: %s -> [%s] -> %s\n", nome, papel, filme);

        TRegister reg;
        reg.tipo = REG_RELACIONAMENTO;

        unsigned long id_pessoa = gerar_hash(nome);
        unsigned long id_filme  = gerar_hash(filme);

        reg.conteudo.rel = criar_movie_person(id_pessoa, id_filme, papel, info_adicional);

        // TODO no futuro: inserir_arvore_b(reg);
    }
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
