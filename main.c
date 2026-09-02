#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <omp.h>
#include <pthread.h>
#include <ctype.h>
#include "times.h"

#define X_MIN -2.0
#define X_MAX 1.0
#define Y_MIN -1.5
#define Y_MAX 1.5

typedef struct Pixel{
    double x;
    double y;
    int iteracoes;
} Pixel;

typedef struct argThread{
    int ** imagem;
    int largura;
    int altura;
    int max_iteracoes;
    int linha_inicio;
    int linha_fim;
    int estrategia; //1 ou 2
    int tid;
    int num_threads;
} argThread;

int calcularIteracoes(complex double z, complex double c, int max_iteracoes, int n){    //fórmula z_novo = z_atual² + c
    if (cabs(z) > 2.0 || n >= max_iteracoes){
        return n;
    }
    return calcularIteracoes(z * z + c, c, max_iteracoes, n + 1);
}

Pixel calcularPixel(double x, double y, int largura, int altura, int max_iteracoes){    //calcula a posição do pixel no plano complexo
    Pixel p;
    p.x = x;
    p.y = y;

    double parte_real = X_MIN + (X_MAX - X_MIN) * x / largura;  //posição horizontal do pixel no plano complexo
    double parte_imaginaria = Y_MIN + (Y_MAX - Y_MIN) * y / altura; //posição vertical do pixel no plano complexo
    complex double c = parte_real + parte_imaginaria * I;   //fórmula de número complexo: C = a + b * i
    p.iteracoes = calcularIteracoes(0, c, max_iteracoes, 0);    //calcula o número de iterações pro pixel
    return p;
}

void * calcularLinhas(void * arg){
    argThread * args = (argThread *)arg;

    if(args->estrategia == 1){  //para pthreads1
        for(int y = args->linha_inicio; y < args->linha_fim; y ++){
            for(int x = 0; x < args->largura; x ++){
                Pixel result = calcularPixel(x, y, args->largura, args->altura, args->max_iteracoes);       
                args->imagem[y][x] = result.iteracoes;
            }
        }
    } 
    if(args->estrategia == 2){  //para pthreads2
        for(int y = args->tid; y < args->altura; y += args->num_threads){
            for(int x = 0; x < args->largura; x ++){
                Pixel result = calcularPixel(x, y, args->largura, args->altura, args->max_iteracoes);  
                args->imagem[y][x] = result.iteracoes;    
            }
        }
    }
    return NULL;
}

void serial(int ** imagem, int largura, int altura, int max_iteracoes){ //percorre e calcula cada pixel da matriz imagem.
    for(int y = 0; y < altura; y ++){
        for(int x = 0; x < largura; x ++){
            Pixel result = calcularPixel(x, y, largura, altura, max_iteracoes);
            imagem[y][x] = result.iteracoes;
        }
    }
}

void openmp(int ** imagem, int largura, int altura, int max_iteracoes, int num_threads){
    #pragma omp parallel for num_threads(num_threads)
    for(int y = 0; y < altura; y ++){
        for(int x = 0; x < largura; x ++){
            Pixel result = calcularPixel(x, y, largura, altura, max_iteracoes);
            imagem[y][x] = result.iteracoes;
        }
    }
}

void pthread1(int ** imagem, int largura, int altura, int max_iteracoes, int num_threads){
    argThread arg[num_threads];
    pthread_t threads[num_threads];

    int div_threads = altura / num_threads; //divide as linhas da imagem entre as threads

    for(int i = 0; i < num_threads; i ++){
        arg[i].imagem = imagem;
        arg[i].largura = largura;
        arg[i].altura = altura;
        arg[i].max_iteracoes = max_iteracoes;
        arg[i].linha_inicio = i * div_threads;
        arg[i].linha_fim = (i + 1) * div_threads;
        arg[i].estrategia = 1;
        
        int nova_thread = pthread_create(&threads[i], NULL, calcularLinhas, &arg[i]);
        if(nova_thread != 0){
            fprintf(stderr, "Falha ao criar thread.\n");
            return;
        }
    }
    for (int i = 0; i < num_threads; i ++){
        pthread_join(threads[i], NULL);
    }
}

void pthread2(int ** imagem, int largura, int altura, int max_iteracoes, int num_threads){
    argThread arg[num_threads];
    pthread_t threads[num_threads];

    for(int i = 0; i < num_threads; i ++){
        arg[i].imagem = imagem;
        arg[i].largura = largura;
        arg[i].altura = altura;
        arg[i].max_iteracoes = max_iteracoes;
        arg[i].tid = i;
        arg[i].num_threads = num_threads;
        arg[i].estrategia = 2;
        int nova_thread = pthread_create(&threads[i], NULL, calcularLinhas, &arg[i]);
        if(nova_thread != 0){
            fprintf(stderr, "Falha ao criar thread.\n");
            return;
        }
    }
    for(int i = 0; i < num_threads; i ++){
        pthread_join(threads[i],NULL);
    }
}

void salvarArquivoSerial(int ** imagem, int largura, int altura, int max_iteracoes){
    FILE * arquivo = fopen("mandelbrot_hcs4_serial.pgm", "w");
    if (arquivo == NULL){
        fprintf(stderr, "Falha ao abrir o arquivo serial.\n");
        return;
    }
    for (int y = 0; y < altura; y ++){
        for(int x = 0; x < largura; x++){
            int intensidade = 255 * imagem[y][x] / max_iteracoes;   //calcula a intensidade do pixel de acordo com o número de iterações
            if (x > 0) {
                fprintf(arquivo, " ");
            }
            fprintf(arquivo, "%d", intensidade);
        }
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
}

void salvarArquivoOpenmp(int ** imagem, int largura, int altura, int max_iteracoes){
    FILE * arquivo = fopen("mandelbrot_hcs4_openmp.pgm", "w");
    if (arquivo == NULL){
        fprintf(stderr, "Falha ao abrir o arquivo openmp.\n");
        return;
    }
    for(int y = 0; y < altura; y ++){
        for(int x = 0; x < largura; x++){
            int intensidade = 255 * imagem[y][x] / max_iteracoes;
            if(x > 0) {
                fprintf(arquivo, " ");
            }
            fprintf(arquivo, "%d", intensidade);
        }
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
}

void salvarArquivoPthread1(int ** imagem, int largura, int altura, int max_iteracoes){
    FILE * arquivo = fopen("mandelbrot_hcs4_pthreads1.pgm", "w");
    if (arquivo == NULL){
        fprintf(stderr, "Falha ao abrir o arquivo pthreads1.\n");
        return;
    }
    for(int y = 0; y < altura; y ++){
        for(int x = 0; x < largura; x++){
            int intensidade = 255 * imagem[y][x] / max_iteracoes;
            if(x > 0) {
                fprintf(arquivo, " ");
            }
            fprintf(arquivo, "%d", intensidade);
        }
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
}

void salvarArquivoPthread2(int ** imagem, int largura, int altura, int max_iteracoes){
    FILE * arquivo = fopen("mandelbrot_hcs4_pthreads2.pgm", "w");
    if (arquivo == NULL){
        fprintf(stderr, "Falha ao abrir o arquivo pthreads2.\n");
        return;
    }
    for(int y = 0; y < altura; y ++){
        for(int x = 0; x < largura; x++){
            int intensidade = 255 * imagem[y][x] / max_iteracoes;
            if(x > 0) {
                fprintf(arquivo, " ");
            }
            fprintf(arquivo, "%d", intensidade);
        }
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
}

int main(int argc, char *argv[]){

    if(argc != 5){
        fprintf(stderr, "O programa precisa de obrigatoriamente 4 argumentos.\n");
        return 1;
    }

    int largura = atoi(argv[1]);
    int altura = atoi(argv[2]);
    int max_iteracoes = atoi(argv[3]);
    int num_threads = atoi(argv[4]);

    //tratamento de erros:
    for (int i = 0; argv[1][i] != '\0'; i++) {
        if(!isdigit(argv[1][i])){
            fprintf(stderr, "A largura precisa ser um número inteiro positivo.\n");
            return 1;
        }
    }

    for (int i = 0; argv[2][i] != '\0'; i++) {
        if(!isdigit(argv[2][i])){
            fprintf(stderr, "A altura precisa ser um número inteiro positivo.\n");
            return 1;
        }
    }

    for (int i = 0; argv[3][i] != '\0'; i++) {
        if(!isdigit(argv[3][i])){
            fprintf(stderr, "O número máximo de iterações precisa ser um número inteiro positivo.\n");
            return 1;
        }
    }

    for (int i = 0; argv[4][i] != '\0'; i++) {
        if(!isdigit(argv[4][i])){
            fprintf(stderr, "O número de threads precisa ser um número inteiro positivo.\n");
            return 1;
        }
    }

    if(altura > 8000 || largura > 8000){
        fprintf(stderr, "Dimensões muito grandes. O máximo permitido é 8000.\n");
        return 1;
    }
    if(max_iteracoes > 2000){
        fprintf(stderr, "Número de iterações muito grande. O máximo permitido é 2000.\n");
        return 1;
    }
    if(num_threads > 2000){
        fprintf(stderr, "Número de threads muito grande. O máximo permitido é 2000.\n");
        return 1;
    }

    int ** imagem = (int **)malloc(altura * sizeof(int *));
    if (imagem == NULL) {
        fprintf(stderr, "Falha ao alocar a memória.\n");
        return 1; 
    }
    for (int y = 0; y < altura; y ++){
        imagem[y] = (int *)malloc(largura * sizeof(int));
        if (imagem[y] == NULL){
            fprintf(stderr, "Falha ao alocar a memória.\n");
            return 1;
        }
    }

    FILE *f = fopen("times.txt", "w");  //limpa o arquivo de tempo pra poder gravar os novos tempos
    if (f != NULL){
        fclose(f);
    }

    //salvando nos arquivos:
    struct timespec start, end; 
    clock_gettime(CLOCK_MONOTONIC, &start); //inicia a medição de tempo
    serial(imagem, largura, altura, max_iteracoes);
    clock_gettime(CLOCK_MONOTONIC, &end);   //finaliza a medição de tempo
    gravarTempo("Serial", medirTempo(start, end));
    salvarArquivoSerial(imagem, largura, altura, max_iteracoes);

    clock_gettime(CLOCK_MONOTONIC, &start);
    openmp(imagem, largura, altura, max_iteracoes, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &end);
    gravarTempo("OpenMP", medirTempo(start, end));
    salvarArquivoOpenmp(imagem, largura, altura, max_iteracoes);

    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread1(imagem, largura, altura, max_iteracoes, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &end);
    gravarTempo("Pthreads1", medirTempo(start, end));
    salvarArquivoPthread1(imagem, largura, altura, max_iteracoes);

    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread2(imagem, largura, altura, max_iteracoes, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &end);
    gravarTempo("Pthreads2", medirTempo(start, end));
    salvarArquivoPthread2(imagem, largura, altura, max_iteracoes);

    //liberando a memória da matriz:
    for(int y = 0; y < altura; y ++){
        free(imagem[y]);
    }
    free(imagem);

    return 0;
}