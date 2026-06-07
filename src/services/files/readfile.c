#include "readfile.h"
#include "../../../src/services/arvorebm/TARVBM_MEMORIASECUNDARIA.h"

// Arquivo de índice ÚNICO conforme exigido pelo enunciado
#define ARQ_INDICE "indice.dat"

// ============================================================================
// 1. UTILITÁRIOS DE STRING
// ============================================================================

static unsigned long gerar_hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

// Extrai o campo de índice indice_campo delimitado por '|', com trim
static int extrair_campo(char *origem, int indice_campo,
                         char *destino, int tam_max) {
    int i = 0, campo_atual = 0, j = 0;
    destino[0] = '\0';

    while (origem[i] != '\0' && campo_atual < indice_campo) {
        if (origem[i] == '|') campo_atual++;
        i++;
    }
    if (campo_atual != indice_campo || origem[i] == '\0') return 0;

    // left trim
    while (origem[i] != '\0' && origem[i] != '|' && isspace((unsigned char)origem[i]))
        i++;

    // copia
    while (origem[i] != '\0' && origem[i] != '|') {
        if (j < tam_max - 1) destino[j++] = origem[i];
        i++;
    }
    destino[j] = '\0';

    // right trim
    j--;
    while (j >= 0 && isspace((unsigned char)destino[j]))
        destino[j--] = '\0';

    return (int)(strlen(destino) > 0);
}

// ============================================================================
// 2. CONSTRUTORES
// ============================================================================

static TMovie criar_movie(unsigned long id, const char *titulo,
                          int ano, const char *tagline) {
    TMovie m;
    m.id  = id;
    m.ano = ano;
    strncpy(m.titulo,   titulo,   sizeof(m.titulo)   - 1); m.titulo[sizeof(m.titulo)-1]   = '\0';
    strncpy(m.tagline,  tagline,  sizeof(m.tagline)  - 1); m.tagline[sizeof(m.tagline)-1] = '\0';
    return m;
}

static TPerson criar_person(unsigned long id, const char *nome, int ano_nasc) {
    TPerson p;
    p.id = id;
    p.ano_nascimento = ano_nasc;
    strncpy(p.nome, nome, sizeof(p.nome) - 1); p.nome[sizeof(p.nome)-1] = '\0';
    return p;
}

static TMoviePerson criar_rel(unsigned long id_pessoa, unsigned long id_filme,
                              const char *papel, const char *info) {
    TMoviePerson mp;
    mp.id_pessoa = id_pessoa;
    mp.id_filme  = id_filme;
    strncpy(mp.papel,         papel, sizeof(mp.papel)         - 1); mp.papel[sizeof(mp.papel)-1]                 = '\0';
    strncpy(mp.info_adicional, info,  sizeof(mp.info_adicional) - 1); mp.info_adicional[sizeof(mp.info_adicional)-1] = '\0';
    return mp;
}

// ============================================================================
// 3. INSERÇÃO NA ÁRVORE ÚNICA
// ============================================================================

static void inserir(FILE *arq_indice, long *raiz,
                    TRegister reg, unsigned long chave,
                    int t, int *cont_folhas) {
    *raiz = TARVBM_insere(arq_indice, *raiz, reg, chave, t, cont_folhas);
}

// ============================================================================
// 4. PARSERS
// ============================================================================

static void processar_nodes(char *linha, FILE *arq_indice,
                             long *raiz, int t, int *cont_folhas) {
    char tipo[30] = {0};
    extrair_campo(linha, 0, tipo, sizeof(tipo));

    if (strcmp(tipo, "Movie") == 0) {
        char titulo[100]={0}, ano_str[10]={0}, tagline[150]={0};
        extrair_campo(linha, 1, titulo,   sizeof(titulo));
        extrair_campo(linha, 2, ano_str,  sizeof(ano_str));
        extrair_campo(linha, 3, tagline,  sizeof(tagline));
        if (!strlen(titulo)) return;

        int ano = atoi(ano_str);
        unsigned long id = gerar_hash(titulo);

        TRegister reg;
        reg.tipo           = REG_FILME;
        reg.conteudo.filme = criar_movie(id, titulo, ano, tagline);

        printf("[NODES] Filme: %-40s | Ano: %d | Hash: %lu\n", titulo, ano, id);
        inserir(arq_indice, raiz, reg, id, t, cont_folhas);

    } else if (strcmp(tipo, "Person") == 0) {
        char nome[100]={0}, nasc_str[10]={0};
        extrair_campo(linha, 1, nome,     sizeof(nome));
        extrair_campo(linha, 2, nasc_str, sizeof(nasc_str));
        if (!strlen(nome)) return;

        int ano_nasc = atoi(nasc_str);
        unsigned long id = gerar_hash(nome);

        TRegister reg;
        reg.tipo            = REG_PESSOA;
        reg.conteudo.pessoa = criar_person(id, nome, ano_nasc);

        printf("[NODES] Pessoa: %-40s | Nasc: %d | Hash: %lu\n", nome, ano_nasc, id);
        inserir(arq_indice, raiz, reg, id, t, cont_folhas);
    }
}

static void processar_relationships(char *linha, FILE *arq_indice,
                                    long *raiz, int t, int *cont_folhas) {
    char token[30] = {0};
    extrair_campo(linha, 0, token, sizeof(token));
    if (strcmp(token, "START Person") != 0) return;

    char nome[100]={0}, papel[50]={0}, filme[100]={0}, info[100]={0};
    extrair_campo(linha, 1, nome,  sizeof(nome));
    extrair_campo(linha, 2, papel, sizeof(papel));
    // campo 3 = "END Movie" — ignorado
    extrair_campo(linha, 4, filme, sizeof(filme));
    extrair_campo(linha, 5, info,  sizeof(info));

    if (!strlen(nome) || !strlen(filme)) return;

    unsigned long id_pessoa = gerar_hash(nome);
    unsigned long id_filme  = gerar_hash(filme);

    // Chave da relação: combinação dos dois hashes (XOR rotacionado)
    unsigned long chave_rel = id_pessoa ^ (id_filme << 1) ^ (id_filme >> 1);

    TRegister reg;
    reg.tipo        = REG_RELACIONAMENTO;
    reg.conteudo.rel = criar_rel(id_pessoa, id_filme, papel, info);

    printf("[RELS]  %s -> [%s] -> %s | Hash: %lu\n", nome, papel, filme, chave_rel);
    inserir(arq_indice, raiz, reg, chave_rel, t, cont_folhas);
}

// ============================================================================
// 5. FUNÇÃO PRINCIPAL
// ============================================================================

void readfile(FILE *arq_indice, long *raiz, int t_grau, int *cont_folhas) {
    char linha[MAX_LINHA];

    // ---- Nodes.txt ----
    FILE *f = fopen("files/Nodes.txt", "r");
    if (!f) { printf("[ERRO] 'files/Nodes.txt' nao encontrado.\n"); }
    else {
        printf("=== Lendo Nodes.txt ===\n");
        while (fgets(linha, sizeof(linha), f)) {
            linha[strcspn(linha, "\r\n")] = '\0';
            // ignora linha em branco
            int vazia = 1;
            for (int i = 0; linha[i]; i++)
                if (!isspace((unsigned char)linha[i])) { vazia = 0; break; }
            if (vazia) continue;
            processar_nodes(linha, arq_indice, raiz, t_grau, cont_folhas);
        }
        fclose(f);
    }

    // ---- Relationships.txt ----
    f = fopen("files/Relationships.txt", "r");
    if (!f) { printf("[ERRO] 'files/Relationships.txt' nao encontrado.\n"); }
    else {
        printf("\n=== Lendo Relationships.txt ===\n");
        while (fgets(linha, sizeof(linha), f)) {
            linha[strcspn(linha, "\r\n")] = '\0';
            int vazia = 1;
            for (int i = 0; linha[i]; i++)
                if (!isspace((unsigned char)linha[i])) { vazia = 0; break; }
            if (vazia) continue;
            processar_relationships(linha, arq_indice, raiz, t_grau, cont_folhas);
        }
        fclose(f);
    }
}