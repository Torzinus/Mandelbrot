#include <time.h>
#include <stdio.h>

double medirTempo(struct timespec start, struct timespec end){
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

void gravarTempo(const char * metodo, double tempo){
FILE * arquivo = fopen("times.txt", "a");
    if (arquivo == NULL){
        fprintf(stderr, "Falha ao abrir o arquivo de tempo.\n");
        return;
    }
    fprintf(arquivo, "%s: %fs\n", metodo, tempo);
    fclose(arquivo);
}