#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define PENSANDO 0
#define FAMINTO 1
#define COMENDO 2

typedef struct thread_filosofo_info{
    int id_filosofo;
    pthread_t thread_id;
    int numFilosofos;
    int tempoComendo;
    int tempoPensando;
    sem_t* mutex;
    sem_t* semfil;
    int* estado;
}thread_filosofo_info;
typedef struct pacotao_info{
    thread_filosofo_info* pacotes;
    thread_filosofo_info* pacote;
}pacotao_info;

void mostrar(void*arg){
    thread_filosofo_info* pacote = (thread_filosofo_info*)arg;
    int i;

    for(i=0;i<pacote->numFilosofos;i++){
        if(pacote->estado[i] == PENSANDO){
            printf("O filosofo %d esta pensando...\n", i);
        }
        if(pacote->estado[i] == FAMINTO){
            printf("O filosofo %d esta com fome...\n", i);
        }
        if(pacote->estado[i] == COMENDO){
            printf("O filosofo %d esta comendo!\n", i);
        }
    }    
    printf("\n");
}

int calcularTempoMedio(int entrada){
	srand(time(NULL));
	return 1 + rand() %((entrada*1000) +1);
}

void pensar(void* arg){
    thread_filosofo_info* pacote = (thread_filosofo_info*)arg;
    usleep(calcularTempoMedio(pacote->tempoPensando));
    }

int calcular_esquerda(int i, int numFilo){
    int esquerda;
	esquerda = (i+numFilo-1)%numFilo;
	return esquerda;
}
int calcular_direita(int i, int numFilo){
    int direita;
	direita = (i+1)%numFilo;
	return direita;
}
void intencao(void* arg){
    thread_filosofo_info* pacote = (thread_filosofo_info*)arg;
    
    int esquerda,direita;
    esquerda = calcular_esquerda(pacote->id_filosofo,pacote->numFilosofos);
    direita = calcular_direita(pacote->id_filosofo,pacote->numFilosofos);
    if(pacote->estado[pacote->id_filosofo] == FAMINTO && pacote->estado[esquerda] != COMENDO && pacote->estado[direita]!= COMENDO){
        pacote->estado[pacote->id_filosofo] = COMENDO;
        mostrar(pacote);
        sem_post(pacote->semfil);
    }
}

void pegar_garfo(void* arg){
    thread_filosofo_info* pacote = (thread_filosofo_info*)arg;
    sem_wait(pacote->mutex);
    pacote->estado[pacote->id_filosofo] = FAMINTO;
    mostrar(arg);
    intencao(arg);
    sem_post(pacote->mutex);
    sem_wait(pacote->semfil);

}
void comer(void* arg){
    thread_filosofo_info* pacote = (thread_filosofo_info*)arg;
    usleep(calcularTempoMedio(pacote->tempoComendo));
}
void devolver_garfo(void* arg0, void* arg1){
    thread_filosofo_info* pacotes = (thread_filosofo_info*)arg0;
    thread_filosofo_info* pacote = (thread_filosofo_info*)arg1;
    int esquerda,direita;
    sem_wait(pacote->mutex);
    pacote->estado[pacote->id_filosofo] = PENSANDO;
    mostrar(pacote);
    esquerda = calcular_esquerda(pacote->id_filosofo,pacote->numFilosofos);
    direita = calcular_direita(pacote->id_filosofo,pacote->numFilosofos);
    intencao(&pacotes[esquerda]);
    intencao(&pacotes[direita]);
    sem_post(pacote->mutex);

}

void* thread_filosofo(void* arg){
    pacotao_info* pacotao = (pacotao_info*)arg;
    
    
    while(1){
        pensar((void*)pacotao->pacote);
        pegar_garfo((void*)pacotao->pacote);
        comer((void*)pacotao->pacote);
        devolver_garfo((void*)pacotao->pacotes,(void*)pacotao->pacote);
        i++;
}
    return NULL;
}

int main(int argc,char* argv[]){
    int numFilo = atoi(argv[1]);
    int tempoComendo = atoi(argv[2]);
    int tempoPensando = atoi(argv[3]);
    sem_t mutex;
    sem_t* semfil = (sem_t*)malloc(numFilo*sizeof(sem_t));
    int i,j;
    thread_filosofo_info* pacotes = (thread_filosofo_info*)malloc(numFilo*sizeof(thread_filosofo_info));
    sem_init(&mutex,0,1);

    for(i=0;i<numFilo;i++){
        pacotes[i].estado = (int*)malloc(numFilo*sizeof(int));
        for(j=0;j<numFilo;j++){
            pacotes[i].estado[j] = 0;
        }
        pacotes[i].id_filosofo = i;
        pacotes[i].numFilosofos = numFilo;
        sem_init(&semfil[i],0,0);
        pacotes[i].semfil = &semfil[i];
        pacotes[i].mutex = &mutex;
        pacotes[i].tempoComendo = tempoComendo;
        pacotes[i].tempoPensando = tempoPensando;
    }
    mostrar((void*)&pacotes[0]);

    for(i=0;i<numFilo;i++){
        pacotao_info* pacotao = (pacotao_info*)malloc(sizeof(pacotao_info));
        pacotao->pacote = &(pacotes[i]);
        pacotao->pacotes = pacotes;
        pthread_create(&pacotes[i].thread_id,NULL,thread_filosofo,pacotao);
    }

    for(i=0; i<numFilo;i++){
        pthread_join(pacotes[i].thread_id,NULL);
    }

    return 0;

}
