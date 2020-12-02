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
    sem_t* semaphore_count;
    sem_t* semphore_barrier;
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
    
    sem_wait(pacote->semaphore_count);
    if(*(pacote->thread_counter) == pacote->numthread-1){
    
        *(pacote->thread_counter) = 0;
        *(pacote->tempo_thread) = clock();
        sem_post(pacote->semaphore_count);
        for(i=0;i<pacote->numthread-1;i++){
            sem_post(pacote->semphore_barrier);
        }
    
    }
    else{
        
        (*pacote->thread_counter)++;
        
        sem_post(pacote->semaphore_count);
        sem_wait(pacote->semphore_barrier);
    }
    
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
    
    int thread_counter = 0;
    

    thread_info* pacotes = (thread_info*)malloc(num_threads*sizeof(thread_info));
    long long bloco = tamvet/num_threads;
    long long i;
    int tempo;
    void* s;
    long long somatotal = 0;
    int* vet = (int*)malloc(tamvet*sizeof(int));
    sem_t semaforo_count,semaforo_barrier;
    sem_init(&semaforo_count,0,1);
    sem_init(&semaforo_barrier,0,0);


    geraVetor(vet,tamvet);
    for(i=0;i<num_threads;i++){
        pacotes[i].id_thread = i;
        pacotes[i].inicio = bloco*i;
        pacotes[i].numthread = num_threads;
        pacotes[i].semaphore_count = &semaforo_count;
        pacotes[i].semphore_barrier = &semaforo_barrier;
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
        error_thread = pthread_create(&pacotes[i].thread_id,NULL,threadSomaVetor,&(pacotes[i]));
       /* if(error_thread!=0){
            printf("\n------------------Deu erro aqui garai-------------\n");

        }*/
    }

    for (i=0;i<num_threads;i++){
        error_thread  = pthread_join(pacotes[i].thread_id,&s);
        somatotal+= (long long)s;
        /*if(error_thread!=0){
            printf("\n------------------Deu pau na recepcao-------------\n");

        }*/

    }
    tempo = clock() - tempo;

    printf("\n%d\nsoma:%d",tempo,somatotal);




    return 0;

}