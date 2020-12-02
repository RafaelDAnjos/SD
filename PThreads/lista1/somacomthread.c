#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct thread_info{

    pthread_t thread_id;
    int id_thread;
    int inicio;
    int fim;
    int* vet;

}thread_info;
void geraVetor(int* vet, long long tamvet){
    long long i;

    

    for(i =0;i<tamvet;i++){

        vet[i] = rand()%10;
    }
    

}

void * threadSomaVetor(void* args){
    thread_info* pacote = args;
    long long soma =0;
    long long i;
    for( i=pacote->inicio;i<pacote->fim;i++){
        soma += pacote->vet[i];
    }
    return (void*)soma;
}
/*argc numero de argumentos passados, argv lista de argumentos*/

int main( int argc, char *argv[ ]){
    long long tamvet = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    int error_thread;

    thread_info* pacotes = (thread_info*)malloc(num_threads*sizeof(thread_info));
    long long bloco = tamvet/num_threads;
    long long i;
    int tempo;
    void* s;
    long long somatotal = 0;
    int* vet = (int*)malloc(tamvet*sizeof(int));
    geraVetor(vet,tamvet);
    for(i=0;i<num_threads;i++){
        pacotes[i].id_thread = i;
        pacotes[i].inicio = bloco*i;

        if(i == num_threads-1){
            pacotes[i].fim = tamvet;
            
        }
        else{
            pacotes[i].fim = bloco*(i+1);
            
        }
        pacotes[i].vet = vet;
        printf("a thread %d vai somar de %d ate %d \n",pacotes[i].id_thread,pacotes[i].inicio,pacotes[i].fim);
    } 

    tempo = clock();
    for(i = 0; i<num_threads;i++){
        error_thread = pthread_create(&pacotes[i].thread_id,NULL,threadSomaVetor,&(pacotes[i]));
        if(error_thread!=0){
            

        }
    }

    for (i=0;i<num_threads;i++){

        error_thread  = pthread_join(pacotes[i].thread_id,&s);
        somatotal+= (long long)s;
        if(error_thread!=0){
            

        }

    }
    tempo = clock() - tempo;

    printf("\n%d\nsoma:%d",tempo,somatotal);




    return 0;

}