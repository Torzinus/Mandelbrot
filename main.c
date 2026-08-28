#include <stdio.h>
#include <stdlib.h>

#define X_MIN -2.0
#define X_MAX 1.0
#define Y_MIN -1.5
#define Y_MAX 1.5

int main(int argc, char *argv[]){
    int altura = atoi(argv[1]);
    int largura = atoi(argv[2]);
    int max_iteracoes = atoi(argv[3]);
    int num_threads = atoi(argv[4]);

    //lembrar de filtrar os argumentos inválidos aqui
    
    printf("Imagem %dx%d\nInterações máximas: %d\nN threads: %d\n", largura, altura, max_iteracoes, num_threads);
    return 0;
}