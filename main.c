#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <omp.h>
#include <pthread.h>

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
        pthread_create(&threads[i], NULL, calcularLinhas, &arg[i]);
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
        pthread_create(&threads[i], NULL, calcularLinhas, &arg[i]);
    }
    for(int i = 0; i < num_threads; i ++){
        pthread_join(threads[i],NULL);
    }
}

void salvarArquivoSerial(int ** imagem, int largura, int altura, int max_iteracoes){
    FILE * arquivo = fopen("mandelbrot_hcs4_serial.pgm", "w");
    if (arquivo == NULL){
        return;
    }
    for (int y = 0; y < altura; y ++){
        for(int x = 0; x < largura; x++){
            int intensidade = 255 * imagem[y][x] / max_iteracoes;   //calcula a intensidade do pixel de acordo com o número de iterações
            fprintf(arquivo, "%d ", intensidade);
        }
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
}

void salvarArquivoOpenmp(int ** imagem, int largura, int altura, int max_iteracoes){
    FILE * arquivo = fopen("mandelbrot_hcs4_openmp.pgm", "w");
    if (arquivo == NULL){
        return;
    }
    for(int y = 0; y < altura; y ++){
        for(int x = 0; x < largura; x++){
            int intensidade = 255 * imagem[y][x] / max_iteracoes;
            fprintf(arquivo, "%d ", intensidade);
        }
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
}

void salvarArquivoPthread1(int ** imagem, int largura, int altura, int max_iteracoes){
    FILE * arquivo = fopen("mandelbrot_hcs4_pthreads1.pgm", "w");
    if (arquivo == NULL){
        return;
    }
    for(int y = 0; y < altura; y ++){
        for(int x = 0; x < largura; x++){
            int intensidade = 255 * imagem[y][x] / max_iteracoes;
            fprintf(arquivo, "%d ", intensidade);
        }
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
}

void salvarArquivoPthread2(int ** imagem, int largura, int altura, int max_iteracoes){
    FILE * arquivo = fopen("mandelbrot_hcs4_pthreads2.pgm", "w");
    if (arquivo == NULL){
        return;
    }
    for(int y = 0; y < altura; y ++){
        for(int x = 0; x < largura; x++){
            int intensidade = 255 * imagem[y][x] / max_iteracoes;
            fprintf(arquivo, "%d ", intensidade);
        }
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
}

int main(int argc, char *argv[]){
    int largura = atoi(argv[1]);
    int altura = atoi(argv[2]);
    int max_iteracoes = atoi(argv[3]);
    int num_threads = atoi(argv[4]);

    //lembrar de filtrar os argumentos inválidos aqui

    int ** imagem = (int **)malloc(altura * sizeof(int *));
    if (imagem == NULL) {
        return 1;   //falha ao alocar memória
    }
    for (int y = 0; y < altura; y ++){
        imagem[y] = (int *)malloc(largura * sizeof(int));
        if (imagem[y] == NULL){
            return 1;
        }
    }

    serial(imagem, largura, altura, max_iteracoes);
    salvarArquivoSerial(imagem, largura, altura, max_iteracoes);
    
    openmp(imagem, largura, altura, max_iteracoes, num_threads);
    salvarArquivoOpenmp(imagem, largura, altura, max_iteracoes);

    pthread1(imagem, largura, altura, max_iteracoes, num_threads);
    salvarArquivoPthread1(imagem, largura, altura, max_iteracoes);

    pthread2(imagem, largura, altura, max_iteracoes, num_threads);
    salvarArquivoPthread2(imagem, largura, altura, max_iteracoes);

    //liberando a memória da matriz:
    for(int y = 0; y < altura; y ++){
        free(imagem[y]);
    }
    free(imagem);

    return 0;
}