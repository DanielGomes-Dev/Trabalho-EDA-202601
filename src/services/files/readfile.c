#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../arvorebm/TARVBM_MEMORIASECUNDARIA.h"

// Nomes dos arquivos de índice em disco para cada árvore
#define ARQ_INDICE_FILMES   "arvore_filmes.dat"
#define ARQ_INDICE_PESSOAS  "arvore_pessoas.dat"
#define ARQ_INDICE_RELACOES "arvore_relacoes.dat"

#define MAX_LINHA 512

// ============================================================================
// 1. DEFINIÇÕES DE TIPOS E INTERFACES
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

// Estrutura interna para controle das árvores
typedef struct {
    long *offset_filmes;
    long *offset_pessoas;
    long *offset_relacoes;
    int t_grau;
    int *contador_folhas;
} TControleArvore;

// ============================================================================
// 2. FUNÇÕES UTILITÁRIAS DE STRING (REFEITAS E BLINDADAS)
// ============================================================================

unsigned long gerar_hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Extrai o campo limpando espaços em branco do início e fim de forma garantida
int extrair_campo(char *origem, int indice_campo, char *destino, int tam_max) {
    int i = 0, campo_atual = 0, j = 0;
    destino[0] = '\0';

    // 1. Localiza o início do campo desejado pulando os caracteres '|'
    while (origem[i] != '\0' && campo_atual < indice_campo) {
        if (origem[i] == '|') {
            campo_atual++;
        }
        i++;
    }

    // Se não encontrou o índice do campo solicitado
    if (campo_atual != indice_campo || origem[i] == '\0') {
        return 0;
    }

    // 2. Pula os espaços em branco iniciais do campo (Left Trim)
    while (origem[i] != '\0' && origem[i] != '|' && isspace((unsigned char)origem[i])) {
        i++;
    }

    // 3. Copia os caracteres válidos para o destino até o delimitador ou fim da linha
    while (origem[i] != '\0' && origem[i] != '|') {
        if (j < tam_max - 1) {
            destino[j++] = origem[i];
        }
        i++;
    }
    destino[j] = '\0';

    // 4. Remove os espaços em branco que sobraram no final (Right Trim)
    j--;
    while (j >= 0 && isspace((unsigned char)destino[j])) {
        destino[j] = '\0';
        j--;
    }

    return (strlen(destino) > 0);
}

// ============================================================================
// 3. FUNÇÕES CONSTRUTORAS (Criação das Structs)
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
// 4. OPERAÇÕES DE INSERÇÃO NA ÁRVORE
// ============================================================================

void inserir_arvore_b(TRegister reg, TControleArvore *ctrl) {
    FILE *arq_arvore = NULL;
    int chave_int = 0; 

    switch (reg.tipo) {
        case REG_FILME:
            arq_arvore = fopen(ARQ_INDICE_FILMES, "rb+");
            if (!arq_arvore) arq_arvore = fopen(ARQ_INDICE_FILMES, "wb+");
            if (!arq_arvore) return;

            chave_int = (int)reg.conteudo.filme.id;
            printf("   [ÁRVORE B+] Indexando Filme '%s' no arquivo %s...\n", reg.conteudo.filme.titulo, ARQ_INDICE_FILMES);
            *(ctrl->offset_filmes) = TARVBM_insere(arq_arvore, *(ctrl->offset_filmes), chave_int, ctrl->t_grau, ctrl->contador_folhas);
            break;

        case REG_PESSOA:
            arq_arvore = fopen(ARQ_INDICE_PESSOAS, "rb+");
            if (!arq_arvore) arq_arvore = fopen(ARQ_INDICE_PESSOAS, "wb+");
            if (!arq_arvore) return;

            chave_int = (int)reg.conteudo.pessoa.id;
            printf("   [ÁRVORE B+] Indexando Pessoa '%s' no arquivo %s...\n", reg.conteudo.pessoa.nome, ARQ_INDICE_PESSOAS);
            *(ctrl->offset_pessoas) = TARVBM_insere(arq_arvore, *(ctrl->offset_pessoas), chave_int, ctrl->t_grau, ctrl->contador_folhas);
            break;

        case REG_RELACIONAMENTO:
            arq_arvore = fopen(ARQ_INDICE_RELACOES, "rb+");
            if (!arq_arvore) arq_arvore = fopen(ARQ_INDICE_RELACOES, "wb+");
            if (!arq_arvore) return;

            chave_int = (int)reg.conteudo.rel.id_pessoa; 
            printf("   [ÁRVORE B+] Indexando Relação (Pessoa ID: %d) no arquivo %s...\n", chave_int, ARQ_INDICE_RELACOES);
            *(ctrl->offset_relacoes) = TARVBM_insere(arq_arvore, *(ctrl->offset_relacoes), chave_int, ctrl->t_grau, ctrl->contador_folhas);
            break;

        default:
            return;
    }

    fclose(arq_arvore);
}

// ============================================================================
// 5. PARSERS DE LINHA INDEPENDENTES
// ============================================================================

void processar_nodes(char *linha, TControleArvore *ctrl) {
    char tipo[30] = {0};
    extrair_campo(linha, 0, tipo, sizeof(tipo));

    if (strcmp(tipo, "Movie") == 0) {
        char titulo[100] = {0}, ano_str[10] = {0}, tagline[150] = {0};
        extrair_campo(linha, 1, titulo, sizeof(titulo));
        extrair_campo(linha, 2, ano_str, sizeof(ano_str));
        extrair_campo(linha, 3, tagline, sizeof(tagline));

        if (strlen(titulo) > 0) {
            int ano = atoi(ano_str);
            printf("[PARSER] Tipo: Movie  | Titulo: %-30s | Ano: %d\n", titulo, ano);
            
            TRegister reg;
            reg.tipo = REG_FILME;
            unsigned long id = gerar_hash(titulo);
            reg.conteudo.filme = criar_movie(id, titulo, ano, tagline);
            inserir_arvore_b(reg, ctrl);
        }
    } 
    else if (strcmp(tipo, "Person") == 0) {
        char nome[100] = {0}, nasc_str[10] = {0};
        extrair_campo(linha, 1, nome, sizeof(nome));
        extrair_campo(linha, 2, nasc_str, sizeof(nasc_str));

        if (strlen(nome) > 0) {
            int ano_nasc = atoi(nasc_str);
            printf("[PARSER] Tipo: Person | Nome:    %-30s | Nasc: %d\n", nome, ano_nasc);
            
            TRegister reg;
            reg.tipo = REG_PESSOA;
            unsigned long id = gerar_hash(nome);
            reg.conteudo.pessoa.id = id; // Garante atribuição correta
            reg.conteudo.pessoa = criar_person(id, nome, ano_nasc);
            inserir_arvore_b(reg, ctrl);
        }
    }
}

void processar_relationships(char *linha, TControleArvore *ctrl) {
    char start_token[30] = {0};
    extrair_campo(linha, 0, start_token, sizeof(start_token));

    if (strcmp(start_token, "START Person") != 0) return;

    char nome[100] = {0}, papel[50] = {0}, filme[100] = {0}, info[100] = {0};
    extrair_campo(linha, 1, nome, sizeof(nome));
    extrair_campo(linha, 2, papel, sizeof(papel));
    // Campo 3 ("END Movie") é ignorado na lógica de extração estruturada
    extrair_campo(linha, 4, filme, sizeof(filme));
    extrair_campo(linha, 5, info, sizeof(info));

    if (strlen(nome) > 0 && strlen(filme) > 0) {
        printf("[PARSER] Relacao: %s -> [%s] -> %s\n", nome, papel, filme);

        TRegister reg;
        reg.tipo = REG_RELACIONAMENTO;
        unsigned long id_pessoa = gerar_hash(nome);
        unsigned long id_filme  = gerar_hash(filme);
        reg.conteudo.rel = criar_movie_person(id_pessoa, id_filme, papel, info);
        
        inserir_arvore_b(reg, ctrl);
    }
}

// ============================================================================
// 6. FLUXO PRINCIPAL DE LEITURA
// ============================================================================
void readfile(long *r_filmes, long *r_pessoas, long *r_relacoes, int t_grau, int *cont_folhas) {
    TControleArvore ctrl = { r_filmes, r_pessoas, r_relacoes, t_grau, cont_folhas };
    char linha[MAX_LINHA];
    
    // --- Lendo Nodes.txt ---
    FILE *f_nodes = fopen("files/Nodes.txt", "r");
    if (f_nodes) {
        printf("--- Iniciando leitura de Nodes ---\n");
        while (fgets(linha, sizeof(linha), f_nodes)) {
            // Remove caracteres de nova linha de forma segura
            linha[strcspn(linha, "\r\n")] = '\0';
            
            // LINHA DE DEBUG: Mostra o que o fgets acabou de pegar do arquivo
            printf("[DEBUG BUFFER] Linha lida: %s\n", linha);

            // Ignora linhas completamente em branco ou vazias
            int i = 0, vazia = 1;
            while(linha[i] != '\0') {
                if(!isspace((unsigned char)linha[i])) { vazia = 0; break; }
                i++;
            }
            if (vazia) continue;

            processar_nodes(linha, &ctrl);
        }
        fclose(f_nodes);
    } else {
        printf("Erro crasso: O arquivo 'files/Nodes.txt' nao foi encontrado.\n");
    }

    // --- Lendo Relationships.txt ---
    FILE *f_rel = fopen("files/Relationships.txt", "r");
    if (f_rel) {
        printf("\n--- Iniciando leitura de Relationships ---\n");
        while (fgets(linha, sizeof(linha), f_rel)) {
            linha[strcspn(linha, "\r\n")] = '\0';
            
            int i = 0, vazia = 1;
            while(linha[i] != '\0') {
                if(!isspace((unsigned char)linha[i])) { vazia = 0; break; }
                i++;
            }
            if (vazia) continue;

            processar_relationships(linha, &ctrl);
        }
        fclose(f_rel);
    } else {
        printf("Erro crasso: O arquivo 'files/Relationships.txt' nao foi encontrado.\n");
    }
}