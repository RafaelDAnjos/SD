#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>


typedef struct thread_info{

    pthread_t thread_id;
    int id_thread;
    int inicio;
    int fim;
    int* vet;
    int numthread;
    pthread_mutex_t * mutex;
    pthread_cond_t * condicional;
    int* thread_counter;
    int* tempo_thread;
    

}thread_info;
void geraVetor(int* vet, long long tamvet){
    long long i;

    

    for(i =0;i<tamvet;i++){

        vet[i] = rand()%10;
    }
    

}

void * threadSomaVetor(void* args){
    thread_info* pacote = args;
    long long soma = 0;
    long long i;
#ifndef BARRIER
#define BARRIER
    pthread_mutex_lock(pacote->mutex);
    (*pacote->thread_counter)++;
    if(*(pacote->thread_counter) == pacote->numthread){
    
        *(pacote->thread_counter) = 0;
        *(pacote->tempo_thread) = clock();
        pthread_cond_broadcast(pacote->condicional);
    
    }
    else{

        while(pthread_cond_wait(pacote->condicional,pacote->mutex)!=0);

    }

    pthread_mutex_unlock(pacote->mutex);
#endif

    for( i=pacote->inicio;i<pacote->fim;i++){
        soma += pacote->vet[i];
        
    }
    return (void*)soma;

}
/*argc numero de argumentos passados, argv lista de argumentos*/

int main( int argc, char *argv[ ]){
    long long tamvet = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    
    
    int thread_counter = 0;
    

    thread_info* pacotes = (thread_info*)malloc(num_threads*sizeof(thread_info));
    long long bloco = tamvet/num_threads;
    long long i;
    int tempo;
    void* s;
    long long somatotal = 0;
    int* vet = (int*)malloc(tamvet*sizeof(int));
    pthread_mutex_t mutex;
    pthread_cond_t condicional;
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&condicional,NULL);




    geraVetor(vet,tamvet);
    for(i=0;i<num_threads;i++){
        pacotes[i].id_thread = i;
        pacotes[i].inicio = bloco*i;
        pacotes[i].numthread = num_threads;
        pacotes[i].mutex = &mutex;
        pacotes[i].condicional = &condicional;
        pacotes[i].thread_counter = &thread_counter;
        pacotes[i].tempo_thread = &tempo;
        
        if(i == num_threads-1){
            pacotes[i].fim = tamvet;
            
        }
        else{
            pacotes[i].fim = bloco*(i+1);
            
        }
        pacotes[i].vet = vet;
        printf("a thread %d vai somar de %d ate %d \n",pacotes[i].id_thread,pacotes[i].inicio,pacotes[i].fim);
    } 

    
    for(i = 0; i<num_threads;i++){
        pthread_create(&pacotes[i].thread_id,NULL,threadSomaVetor,&(pacotes[i]));
        /* if(error_thread!=0){
            printf("\n------------------Deu erro aqui garai-------------\n");

        }*/
    }

    for (i=0;i<num_threads;i++){
        pthread_join(pacotes[i].thread_id,&s);
        somatotal+= (long long)s;
        /*if(error_thread!=0){
            printf("\n------------------Deu pau na recepcao-------------\n");

        }*/

    }
    tempo = clock() - tempo;

    printf("\n%d\nsoma: %lld",tempo,somatotal);

    pthread_mutex_destroy(mutex);
    pthread_cond_destroy(condicional);
    free(vet);
    free(pacotes);




    return 0;

}