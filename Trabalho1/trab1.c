#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <windows.h>

#define VIRUS_MORTO 1
#define INSUMO 2
#define INJECAO 3

int num_vezes;
int contlab0 = 0;
int contlab1 = 0;
int contlab2 = 0;
int continfect0 = 0;
int continfect1 = 0;
int continfect2 = 0;

typedef struct thread_infect_info{
    int id_infect;
    int numvezes;
    int cont;
    pthread_t thread_id;
    pthread_mutex_t* mutex;
    sem_t* sem_bancada;
    int* prod_bancada;

}thread_infect_info;

typedef struct thread_lab_info{
    int id_lab;
    int numvezes;
    int cont;
    pthread_t thread_id;
    pthread_mutex_t* mutex;
    sem_t* sem_bancada;
    int* prod_bancada;

}thread_lab_info;


void print_lab(void* arg, int prod, int prod2){
    thread_lab_info* lab = (thread_lab_info*)arg;
    if(prod == VIRUS_MORTO){

        printf("Virus-morto esta disponivel no lab %d \n", lab->id_lab);
    }
    else if(prod == INSUMO){

        printf("Insumo esta disponivel no lab %d \n", lab->id_lab);

    }else if(prod == INJECAO){
        
        printf("Injecao esta disponivel no lab %d \n", lab->id_lab);
    }

    if(prod2 == VIRUS_MORTO){
        
        printf("Virus-morto esta disponivel no lab %d \n", lab->id_lab);
    }
    else if(prod2 == INSUMO){
        
        printf("Insumo esta disponivel no lab %d \n", lab->id_lab);
    
    }else if(prod2 == INJECAO){

        printf("Injecao esta disponivel no lab %d \n", lab->id_lab);
    }
}


void* thread_lab(void* arg){
    thread_lab_info* lab = (thread_lab_info*)arg;
    
    int p1,p2;
    while (contlab0<=num_vezes || contlab1 <= num_vezes || contlab2 <= num_vezes)
    {
        if(lab->id_lab == 0){
            pthread_mutex_lock(lab->mutex);
            printf("lab 0 pegou a bancada \n");
            sem_getvalue(&lab->sem_bancada[0],&p1);
            sem_getvalue(&lab->sem_bancada[1],&p2);
            if(p1 == 0 && p2 == 0){
                sem_post(&lab->sem_bancada[0]);
                sem_post(&lab->sem_bancada[1]);
                lab->prod_bancada[0] = VIRUS_MORTO;
                lab->prod_bancada[1] = INSUMO;
                /*Chamar uma função que printa que foi produzido*/
                contlab0+=1;
                lab->cont+=1;
                print_lab(arg,VIRUS_MORTO,INSUMO);
                pthread_mutex_unlock(lab->mutex);
                printf("lab 0 largou a bancada\n");
            }else{
                pthread_mutex_unlock(lab->mutex);
                Sleep(1500);
            }

        }else if(lab->id_lab == 1 ){
            pthread_mutex_lock(lab->mutex);
            printf("O laboratorio 1 pegou a bancada \n");
            sem_getvalue(&lab->sem_bancada[2],&p1);
            sem_getvalue(&lab->sem_bancada[3],&p2);
            if(p1 == 0 && p2 == 0){
                sem_post(&lab->sem_bancada[2]);
                sem_post(&lab->sem_bancada[3]);
                lab->prod_bancada[2] = VIRUS_MORTO;
                lab->prod_bancada[3] = INJECAO;
                /*Chamar uma função que printa que foi produzido*/
                contlab1+=1;
                lab->cont+=1;
                print_lab(arg,VIRUS_MORTO,INJECAO);
                pthread_mutex_unlock(lab->mutex);
                printf("O laboratorio 1 largou a bancada\n");
            }else{
                pthread_mutex_unlock(lab->mutex);
                Sleep(1500);
            }

        }else{
            pthread_mutex_lock(lab->mutex);
            printf("O laboratorio 2 pegou a bancada \n");
            sem_getvalue(&lab->sem_bancada[4],&p1);
            sem_getvalue(&lab->sem_bancada[5],&p2);
            if(p1 == 0 && p2 == 0){
                sem_post(&lab->sem_bancada[4]);
                sem_post(&lab->sem_bancada[5]);
                lab->prod_bancada[4] = INJECAO;
                lab->prod_bancada[5] = INSUMO;
                /*Chamar uma função que printa que foi produzido*/
                contlab2+=1;
                lab->cont+=1;
                print_lab(arg,INJECAO,INSUMO);
                pthread_mutex_unlock(lab->mutex);
                printf("O laboratório 2 largou a bancada\n");
            }else{
                pthread_mutex_unlock(lab->mutex);
                Sleep(1500);
                
            }
    }
        

        }
    
    
    
    return NULL;
}
void* thread_infect(void* arg){
    thread_infect_info* infect = (thread_infect_info*)arg;
    
    int p1,p2,p3,p4;
    
    while (continfect0 <= num_vezes || continfect1 <= num_vezes || continfect2 <= num_vezes )
    {
        printf("infectado %d tentando usar a bancada \n", infect->id_infect);
        if(infect->id_infect == 0){

            pthread_mutex_lock(infect->mutex);
            printf("infectado 0 pegou a bancada \n");
            sem_getvalue(&infect->sem_bancada[1],&p1);
            sem_getvalue(&infect->sem_bancada[5],&p2);
            sem_getvalue(&infect->sem_bancada[3],&p3);
            sem_getvalue(&infect->sem_bancada[4],&p4);
            
            if((p1 == 1 || p2 == 1) && (p3 == 1 || p4 == 1 ) ){
                if(p1 == 1 && p3 == 1){

                    sem_wait(&infect->sem_bancada[1]);
                    sem_wait(&infect->sem_bancada[3]);
                    continfect0+=1;
                    infect->cont+=1;
                    printf("O infectado 0 está injetando a vacina!");


                }else if(p2 == 1 && p4 == 1 ){
                    
                    sem_wait(&infect->sem_bancada[5]);
                    sem_wait(&infect->sem_bancada[4]);
                    continfect0+=1;
                    infect->cont+=1;
                    printf("O infectado 0 está injetando a vacina!");

                }else if(p1 == 1 && p4 == 1){
                    
                    sem_wait(&infect->sem_bancada[1]);
                    sem_wait(&infect->sem_bancada[4]);
                    continfect0+=1;
                    infect->cont+=1;
                    printf("O infectado 0 está injetando a vacina!");

                }else if(p2 == 1 && p3 == 1){
                    
                    sem_wait(&infect->sem_bancada[5]);
                    sem_wait(&infect->sem_bancada[3]);
                    continfect0+=1;
                    infect->cont+=1;
                    printf("O infectado 0 está injetando a vacina!");
                }
            }
            pthread_mutex_unlock(infect->mutex);
            printf("O infectado 0 largou a bancada\n");

        }else if(infect->id_infect == 1){
            pthread_mutex_lock(infect->mutex);
            printf("O infectado 1 pegou a bancada\n");
            sem_getvalue(&infect->sem_bancada[0],&p1);
            sem_getvalue(&infect->sem_bancada[2],&p2);
            sem_getvalue(&infect->sem_bancada[4],&p3);
            sem_getvalue(&infect->sem_bancada[3],&p4);
            if((p1 == 1 || p2 == 1) && (p3 == 1 || p4 == 1 ) ){
                if(p1 == 1 && p3 == 1){

                    sem_wait(&infect->sem_bancada[0]);
                    sem_wait(&infect->sem_bancada[4]);
                    continfect1+=1;
                    infect->cont+=1;
                    printf("O infectado 1 está injetando a vacina!\n");

                }else if(p2 == 1 && p4 == 1 ){

                    sem_wait(&infect->sem_bancada[2]);
                    sem_wait(&infect->sem_bancada[3]);
                    continfect1+=1;
                    infect->cont+=1;
                    printf("O infectado 1 está injetando a vacina!\n");

                }else if(p1 == 1 && p4 == 1){

                    sem_wait(&infect->sem_bancada[0]);
                    sem_wait(&infect->sem_bancada[3]);
                    continfect1+=1;
                    infect->cont+=1;
                    printf("O infectado 1 está injetando a vacina!\n");

                }else if(p2 == 1 && p3 == 1){

                    sem_wait(&infect->sem_bancada[2]);
                    sem_wait(&infect->sem_bancada[4]);
                    continfect1+=1;
                    infect->cont+=1;
                    printf("O infectado 1 está injetando a vacina!\n");

                }
            }
            pthread_mutex_unlock(infect->mutex);
            printf("O infectado 1 largou a bancada\n");
        }else if(infect->id_infect == 2){
            pthread_mutex_lock(infect->mutex);
            printf("O infectado 2 pegou a bancada\n");
            sem_getvalue(&infect->sem_bancada[1],&p1);
            sem_getvalue(&infect->sem_bancada[5],&p2);
            sem_getvalue(&infect->sem_bancada[0],&p3);
            sem_getvalue(&infect->sem_bancada[2],&p4);

            if((p1 == 1 || p2 == 1) && (p3 == 1 || p4 == 1 ) ){
                if(p1 == 1 && p3 == 1){
                    
                    sem_wait(&infect->sem_bancada[1]);
                    sem_wait(&infect->sem_bancada[0]);
                    continfect2+=1;
                    infect->cont+=1;
                    printf("O infectado 2 está injetando a vacina!\n");

                }else if(p2 == 1 && p4 == 1 ){

                    sem_wait(&infect->sem_bancada[5]);
                    sem_wait(&infect->sem_bancada[2]);
                    continfect2+=1;
                    infect->cont+=1;
                    printf("O infectado 2 está injetando a vacina!\n");

                }else if(p1 == 1 && p4 == 1){
                    
                    sem_wait(&infect->sem_bancada[1]);
                    sem_wait(&infect->sem_bancada[2]);
                    continfect2+=1;
                    infect->cont+=1;
                    printf("O infectado 2 está injetando a vacina!\n");

                }else if(p2 == 1 && p3 == 1){

                    sem_wait(&infect->sem_bancada[5]);
                    sem_wait(&infect->sem_bancada[0]);
                    continfect2+=1;
                    infect->cont+=1;
                    printf("O infectado 2 está injetando a vacina!\n");

                }
            }
            pthread_mutex_unlock(infect->mutex);
            printf("O infectado 2 largou a bancada \n");
        }
        Sleep(1000);
    }

    
    

    return NULL;

}


int main(int argc,char* argv[]){
    thread_infect_info* infectados = (thread_infect_info*)malloc(3*sizeof(thread_infect_info));
    thread_lab_info* labs = (thread_lab_info*)malloc(3*sizeof(thread_lab_info));
    pthread_mutex_t mutex;
    sem_t* sem_bancada = (sem_t*)malloc(6*sizeof(sem_t));
    int* prod_bancada = (int*)malloc(6*sizeof(int));
    int i;
    int numvezes = atoi(argv[1]);
    num_vezes = numvezes;

    pthread_mutex_init(&mutex,NULL);
    printf("Numero de vezes: %d",numvezes);
    printf("Iniciando bancada \n");

    for(i=0; i<6; i++){
        sem_init(&sem_bancada[i],0,0);
    }

    printf("Inicializando infectados e laboratórios \n");

    for(i=0;i<3;i++){
        infectados[i].mutex = &mutex;
        infectados[i].prod_bancada = prod_bancada;
        infectados[i].sem_bancada = sem_bancada;
        infectados[i].id_infect = i;
        infectados[i].numvezes = numvezes;
        infectados[i].cont = 0;

        labs[i].mutex = &mutex;
        labs[i].prod_bancada = prod_bancada;
        labs[i].sem_bancada = sem_bancada;
        labs[i].id_lab = i;
        labs[i].numvezes = numvezes;
        labs[i].cont = 0;
    }

    printf("Criando threads infectados \n");
    for(i=0; i<3; i++){

        pthread_create(&infectados[i].thread_id,NULL,thread_infect,&(infectados[i]));
    }
    

    printf("Criando threads laboratorios\n");
    for(i=0; i<3; i++){

        pthread_create(&labs[i].thread_id,NULL,thread_lab,&(labs[i]));

    }

    printf("Dando join nos infectados\n");
    for(i=0; i<3;i++){

    pthread_join(infectados[i].thread_id,NULL);
    
    }

    printf("Dando join nos laboratorios \n");
    
    for(i=0; i<3;i++){

        pthread_join(labs[i].thread_id,NULL);
        
    }

    printf("O laboratório 0 produziu: %d produtos\n",contlab0);
    printf("O laboratório 1 produziu: %d produtos\n",contlab1);
    printf("O laboratório 2 produziu: %d produtos\n",contlab2);

    printf("O infectado 0 se injetou: %d vezes\n",continfect0);
    printf("O infectado 1 se injetou: %d vezes\n",continfect1);
    printf("O infectado 2 se injetou: %d vezes\n",continfect2);









    return 0;
}