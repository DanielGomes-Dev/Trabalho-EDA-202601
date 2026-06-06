Olá! É um excelente desafio que você tem em mãos. Como seu professor de Estruturas de Dados, devo dizer que implementar uma **Árvore B/B+ em Memória Secundária** é um dos ritos de passagem mais importantes do curso. Você está saindo do mundo confortável da memória RAM (onde usamos ponteiros como `*prox`) e entrando no mundo real dos Bancos de Dados (onde usamos `offsets` e arquivos de disco).

### 1. A Visão Geral: O que o seu código faz?

Imagine que você está construindo o seu próprio mini-SGBD (Sistema Gerenciador de Banco de Dados) estilo Neo4j ou banco relacional. O fluxo do seu programa se divide em duas grandes fases:

1. **Fase de ETL (Extração e Transformação):** O seu código lê arquivos de texto bruto (`Nodes.txt` e `Relationships.txt`), limpa as strings, converte os nomes/títulos em números inteiros (usando uma função de *Hash*) para servir como Chave Primária (ID).
2. **Fase de Carga (Indexação em Disco):** Para cada registro lido e tratado, ele insere esse ID em uma Árvore B armazenada em disco. Ao invés de usar `malloc` para criar nós, você usa `fwrite` para gravar blocos de bytes diretamente em um arquivo `.dat`. Para conectar pais e filhos, no lugar de ponteiros de memória, você guarda a posição (o *offset* em bytes, capturado via `ftell`) onde o nó filho foi salvo no arquivo.

Abaixo, reescrevi todo o seu código, organizando-o na ordem exata de execução (da `main` até o salvamento no disco), comentado linha a linha para que o fluxo fique cristalino.

---

### 2. O Código Comentado Passo a Passo

#### Passo 1: O Ponto de Partida (`main.c`)

Tudo começa aqui. Inicializamos as raízes das árvores e chamamos o processador de arquivos.

```c
#include "src/services/files/readfile.h"
#include <locale.h>

int main() {
    // Permite que o programa imprima acentos e caracteres especiais corretamente no terminal
    setlocale(LC_ALL, "Portuguese");
    
    // Na memória secundária, não usamos "NULL" para indicar vazio, usamos -1.
    // -1 significa que a árvore ainda não tem nenhuma raiz gravada no arquivo.
    long raiz_filmes = -1;
    long raiz_pessoas = -1;
    long raiz_relacoes = -1;
    
    // O grau mínimo 't' da Árvore B. 
    // Isso define que cada nó terá no máximo (2t - 1) chaves (ou seja, 5 chaves) e 2t filhos.
    int t = 3; 
    
    // Um contador global para nomear os arquivos das folhas sequencialmente (ex: folha_001.bin)
    int total_folhas = 0;

    // Dispara a leitura dos arquivos texto e inicia o processo de montagem das árvores
    readfile(&raiz_filmes, &raiz_pessoas, &raiz_relacoes, t, &total_folhas);
    
    return 0;
}

```

#### Passo 2: As Estruturas de Domínio (`readfile.h` e `TARVBM_MEMORIASECUNDARIA.h`)

Antes de processar, precisamos definir os moldes (structs) dos dados e da nossa árvore.

```c
#ifndef TARVBM_H
#define TARVBM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fórmulas matemáticas da Árvore B baseadas no grau 't'
#define MAX_CHAVES ((t * 2) - 1)
#define MAX_FILHOS (t * 2)

// =========================================================
// A ESSÊNCIA DA ÁRVORE EM DISCO
// =========================================================
typedef struct arvbm {
    int nchaves;       // Quantas chaves este nó possui atualmente
    int folha;         // Booleano: 1 se for nó folha, 0 se for nó interno
    int id_folha;      // ID usado para criar o arquivo isolado desta folha (se for folha)
    long prox_folha;   // Encadeamento das folhas (característica de Árvore B+)
    
    // Vetores estáticos são essenciais para salvar em disco! 
    // Se fossem ponteiros dinâmicos, o fwrite salvaria o endereço de memória, não os dados.
    int chave[100];    
    long filho[101];   // Offsets (posições em bytes no arquivo) dos nós filhos
} TARVBM;

// (Declarações das funções omitidas para focar na lógica principal da implementação)

#endif

```

#### Passo 3: O Processador de Arquivos (`readfile.c`)

Esta é a ponte entre os arquivos `.txt` brutos e a nossa Árvore B.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../arvorebm/TARVBM_MEMORIASECUNDARIA.h"

// Arquivos onde a estrutura estrutural da árvore será salva
#define ARQ_INDICE_FILMES   "arvore_filmes.dat"
#define ARQ_INDICE_PESSOAS  "arvore_pessoas.dat"
#define ARQ_INDICE_RELACOES "arvore_relacoes.dat"
#define MAX_LINHA 512

// (Structs TMovie, TPerson, etc., omitidas aqui pois já estão claras no seu código original)

// Função de Hash: Pega uma string (ex: "The Matrix") e transforma em um número inteiro único (ID)
// Essa conversão é necessária porque nossa árvore atual ordena números inteiros (int mat).
unsigned long gerar_hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// O "Coração" do ETL: Lê o arquivo txt de cima a baixo
void readfile(long *r_filmes, long *r_pessoas, long *r_relacoes, int t_grau, int *cont_folhas) {
    // Agrupa os ponteiros de controle para não ter que passar 5 parâmetros em todas as funções
    TControleArvore ctrl = { r_filmes, r_pessoas, r_relacoes, t_grau, cont_folhas };
    char linha[MAX_LINHA];
    
    FILE *f_nodes = fopen("files/Nodes.txt", "r");
    if (f_nodes) {
        printf("--- Iniciando leitura de Nodes ---\n");
        // Lê linha por linha até o fim do arquivo
        while (fgets(linha, sizeof(linha), f_nodes)) {
            // Remove o "Enter" (\n ou \r) do final da linha
            linha[strcspn(linha, "\r\n")] = '\0';
            
            // Pula a linha se ela for composta apenas por espaços vazios
            int i = 0, vazia = 1;
            while(linha[i] != '\0') {
                if(!isspace((unsigned char)linha[i])) { vazia = 0; break; }
                i++;
            }
            if (vazia) continue;

            // Envia a linha válida para ser interpretada e transformada em struct
            processar_nodes(linha, &ctrl);
        }
        fclose(f_nodes);
    }
    // (O processo de Relationships.txt faz a mesma coisa, chamando processar_relationships)
}

// Analisa a linha, cria a Struct e pede para a Árvore B inserir
void processar_nodes(char *linha, TControleArvore *ctrl) {
    char tipo[30] = {0};
    // Extrai o primeiro campo antes do pipe '|' para saber se é Filme ou Pessoa
    extrair_campo(linha, 0, tipo, sizeof(tipo));

    if (strcmp(tipo, "Movie") == 0) {
        char titulo[100] = {0}, ano_str[10] = {0}, tagline[150] = {0};
        extrair_campo(linha, 1, titulo, sizeof(titulo));
        extrair_campo(linha, 2, ano_str, sizeof(ano_str));
        // ... extração da tagline omitida por brevidade ...

        if (strlen(titulo) > 0) {
            TRegister reg;
            reg.tipo = REG_FILME;
            // Gera o número inteiro que servirá como chave na Árvore B
            unsigned long id = gerar_hash(titulo);
            reg.conteudo.filme = criar_movie(id, titulo, atoi(ano_str), tagline);
            
            // Dispara a inserção na Árvore Secundária
            inserir_arvore_b(reg, ctrl);
        }
    } 
    // Lógica para 'Person' ocorre de forma simétrica...
}

// Gerencia a abertura dos arquivos binários da árvore correta
void inserir_arvore_b(TRegister reg, TControleArvore *ctrl) {
    FILE *arq_arvore = NULL;
    int chave_int = 0; 

    // Escolhe o arquivo correto com base no tipo de entidade
    switch (reg.tipo) {
        case REG_FILME:
            // Tenta abrir o arquivo para leitura e escrita (rb+)
            arq_arvore = fopen(ARQ_INDICE_FILMES, "rb+");
            // Se falhar (arquivo não existe), cria um novo (wb+)
            if (!arq_arvore) arq_arvore = fopen(ARQ_INDICE_FILMES, "wb+");
            if (!arq_arvore) return;

            chave_int = (int)reg.conteudo.filme.id;
            
            // A mágica acontece: Insere a chave e atualiza o offset da raiz (caso ela mude/divida)
            *(ctrl->offset_filmes) = TARVBM_insere(arq_arvore, *(ctrl->offset_filmes), chave_int, ctrl->t_grau, ctrl->contador_folhas);
            break;
            
        // (Casos de PESSOA e RELACIONAMENTO ocorrem da mesma forma)
    }
    fclose(arq_arvore);
}

```

#### Passo 4: A Mecânica de Disco da Árvore B (`TARVBM_MEMORIASECUNDARIA.c`)

Este é o núcleo duro da sua infraestrutura. Onde manipulamos os bytes diretos.

```c
#include "TARVBM_MEMORIASECUNDARIA.h"

// ============================================================================
// PRIMITIVAS DE ACESSO AO DISCO
// ============================================================================

// Lê um nó (bloco de bytes) do arquivo carregando-o para a memória RAM
TARVBM ler_no(FILE *arq_indice, long offset) {
    TARVBM no;
    memset(&no, 0, sizeof(TARVBM)); // Zera o lixo de memória
    if (offset < 0) return no;
    
    // Pula para a posição exata (offset) do arquivo e lê o tamanho exato da struct
    fseek(arq_indice, offset, SEEK_SET);
    fread(&no, sizeof(TARVBM), 1, arq_indice);
    return no;
}

// Salva um nó da memória RAM de volta no arquivo de índice
void escrever_no(FILE *arq_indice, long offset, TARVBM *no) {
    if (offset < 0 || !no) return;
    
    // Posiciona e sobreescreve
    fseek(arq_indice, offset, SEEK_SET);
    fwrite(no, sizeof(TARVBM), 1, arq_indice);
    fflush(arq_indice); // Força a gravação no disco rígido na mesma hora
    
    // Se for nó folha, o seu design exige salvar uma cópia individual em "folha_X.bin"
    if (no->folha) salvar_no_folha(no, 0);
}

// Cria um nó NOVO no final do arquivo e retorna a sua posição (offset)
long TARVBM_cria(FILE *arq_indice, int t, int folha, int *contador_folhas) {
    TARVBM novo;
    memset(&novo, 0, sizeof(TARVBM));
    novo.folha = folha;
    novo.prox_folha = -1;

    // Gera o ID exclusivo para os arquivos separados das folhas
    if (folha) {
        (*contador_folhas)++;
        novo.id_folha = *contador_folhas;
    } else {
        novo.id_folha = -1;
    }

    // Inicializa todos os filhos virtuais apontando para -1 (NULO de disco)
    for (int i = 0; i < 101; i++) novo.filho[i] = -1;

    // Vai para o final do arquivo de índice e anota em qual byte estamos (offset)
    fseek(arq_indice, 0, SEEK_END);
    long offset = ftell(arq_indice);
    
    // Grava o nó virgem lá no finalzinho
    fwrite(&novo, sizeof(TARVBM), 1, arq_indice);
    fflush(arq_indice); 
    
    if (folha) salvar_no_folha(&novo, t);
    
    // Retorna a "porta da casa" onde este nó acabou de ser construído
    return offset; 
}

// ============================================================================
// LÓGICA DE INSERÇÃO NA ÁRVORE
// ============================================================================

// Insere a chave verificando se a raiz precisa ser "quebrada" (split)
long TARVBM_insere(FILE *arq_indice, long offset_raiz, int mat, int t, int *contador_folhas) {
    // 1. A árvore está vazia? Cria a raiz!
    if (offset_raiz == -1) {
        long offset = TARVBM_cria(arq_indice, t, 1, contador_folhas);
        TARVBM no = ler_no(arq_indice, offset);
        no.chave[0] = mat;   // Adiciona o primeiro registro
        no.nchaves = 1;      
        escrever_no(arq_indice, offset, &no); // Salva as alterações de volta pro disco
        return offset; // Retorna o offset da nova raiz
    }
    
    // 2. Lê a raiz atual do disco para a memória
    TARVBM T = ler_no(arq_indice, offset_raiz);

    // 3. Verifica "Overflow" (A raiz está estourada, totalmente cheia?)
    // O nó fica cheio quando atinge MAX_CHAVES, que é 2t - 1.
    if (T.nchaves == (2 * t) - 1) {
        // Precisamos criar uma NOVA raiz (S) que ficará acima da velha raiz (T)
        long offset_S = TARVBM_cria(arq_indice, t, 0, contador_folhas);
        TARVBM S = ler_no(arq_indice, offset_S);
        S.nchaves = 0;
        S.folha = 0;
        S.filho[0] = offset_raiz; // O primeiro filho da nova raiz é a raiz antiga inteira
        
        escrever_no(arq_indice, offset_S, &S);
        
        // Agora invocamos a operação fundamental da árvore B: A Divisão (Split).
        // Isso vai rasgar o nó antigo ao meio, subindo o elemento do meio para a nova raiz.
        offset_S = divisao(arq_indice, offset_S, 1, offset_raiz, t, contador_folhas);
        
        // Após arrumar a casa, inserimos a nova chave.
        offset_S = insere_nao_completo(arq_indice, offset_S, mat, t, contador_folhas);
        return offset_S; // Retorna a nova raiz, a árvore cresceu "para cima"!
    }
    
    // 4. Se a raiz não estava cheia, basta descer a árvore recursivamente inserindo
    return insere_nao_completo(arq_indice, offset_raiz, mat, t, contador_folhas);
}

// Caminha pela árvore (já sabendo que o nó atual NÃO está cheio) para achar a folha correta
static long insere_nao_completo(FILE *arq_indice, long offset_x, int mat, int t, int *contador_folhas) {
    // Carrega o nó atual do disco para investigar
    TARVBM x = ler_no(arq_indice, offset_x);
    int i = x.nchaves - 1;
    
    // CASO BASE: Chegamos na folha onde a chave deve morar.
    if (x.folha) {
        // Empurra chaves maiores para a direita para abrir espaço (Insertion Sort interno)
        while ((i >= 0) && (mat < x.chave[i])) {
            x.chave[i + 1] = x.chave[i];
            i--;
        }
        x.chave[i + 1] = mat; // Encaixa a chave
        x.nchaves++;
        
        escrever_no(arq_indice, offset_x, &x); // Consolida em disco
        return offset_x;
    }
    
    // PASSO RECURSIVO: É um nó interno. Acha qual filho devemos descer.
    while ((i >= 0) && (mat < x.chave[i])) i--;
    i++; // 'i' agora é o índice do filho correto para descer
    
    // Trazemos o filho alvo para a memória para fiscalizar
    TARVBM filho_i = ler_no(arq_indice, x.filho[i]);

    // O filho lá embaixo está lotado? Se sim, dividimos ele antes de descer
    if (filho_i.nchaves == ((2 * t) - 1)) {
        offset_x = divisao(arq_indice, offset_x, (i + 1), x.filho[i], t, contador_folhas);
        x = ler_no(arq_indice, offset_x); // Atualiza nossa visão do pai após a divisão
        
        // Após dividir, o valor do meio subiu para o pai. Decidimos se vamos para o filho da esquerda ou direita
        if (mat > x.chave[i]) i++; 
    }
    
    // Desce finalmente para o filho adequado
    x.filho[i] = insere_nao_completo(arq_indice, x.filho[i], mat, t, contador_folhas);
    escrever_no(arq_indice, offset_x, &x);
    return offset_x;
}

```

---

### Resumo Didático da Arquitetura

O que torna esse código fascinante é que a memória RAM atua apenas como uma **área de trânsito**. Você nunca tem a árvore inteira carregada na RAM. Funções como `ler_no` e `escrever_no` puxam blocos pontuais de dados, fazem a mutação (`divisão`, deslocamento de array) e salvam imediatamente de volta (`fflush`). Isso garante persistência e permite que você processe dezenas de Gigabytes de dados consumindo quase nada de memória do computador!

Você tem alguma dúvida específica sobre como a matemática da função `divisao()` manipula os índices $t$ ou gostaria de focar em como otimizar essas chamadas ao disco `fseek`/`fread` que podem estar tornando o processamento um pouco lento?