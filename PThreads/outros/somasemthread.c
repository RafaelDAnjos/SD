#include <stdio.h>
#include <stdlib.h>



void gera_vetor(int* vet, long long tamvet){
    int i;
    for(i =0;i<tamvet;i++){
        vet[i] = rand();
    }

}

long long somavetor(int*vet,long long tamvet){
    long long soma;
    int i;
    for( i=0;i<tamvet;i++){
        soma += vet[i];
    }
    return soma;
}
/*argc numero de argumentos passados, argv lista de argumentos*/

int main( int argc, char *argv[ ]){
    long long tamvet;
    int * vetint;
    long long soma;

    tamvet = atoll(argv[1]);
    vetint = (int*)malloc(tamvet*sizeof(int));
    gera_vetor(vetint,tamvet);
    soma = somavetor(vetint,tamvet);
    printf("%lld",soma);
}