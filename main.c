#include <stdio.h>
#include <stdlib.h>
#include <complex.h>

#define X_MIN -2.0
#define X_MAX 1.0
#define Y_MIN -1.5
#define Y_MAX 1.5

typedef struct Pixel{
    double x;
    double y;
    int iteracoes;
} Pixel;

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

void serial(int ** imagem, int largura, int altura, int max_iteracoes){ //percorre e calcula cada pixel da matriz imagem.
    for(int y = 0; y < altura; y ++){
        for(int x = 0; x < largura; x ++){
            Pixel result = calcularPixel(x, y, largura, altura, max_iteracoes);
            imagem[y][x] = result.iteracoes;
        }
    }
}

void salvarArquivo(int ** imagem, int largura, int altura, int max_iteracoes){
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
    salvarArquivo(imagem, largura, altura, max_iteracoes);

    //liberando a memória da matriz:
    for(int y = 0; y < altura; y ++){
        free(imagem[y]);
    }
    free(imagem);

    return 0;
}