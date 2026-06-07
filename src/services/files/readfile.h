#ifndef READFILE_H
#define READFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Recebe apenas a raiz única e o arquivo de índice único
void readfile(FILE *arq_indice, long *raiz, int t_grau, int *cont_folhas);

#endif