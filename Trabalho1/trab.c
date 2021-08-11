#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>       

// numero de vezes que o programa deve rodar
int num_primordial;

// vetor que indica quando deve encerrar o programa
int * continue_rodando; 

// nao foi necessario criar um mutex para o vetor continue_rodando apesar de ser compartilhado
// pois a ideia eh as threads terem sua posicao fixa no vetor e apenas acessarem ela

typedef struct infectado{
    pthread_t thrd_id;                    // identificacao da thread
    int id;                              // identificacao do infectado
    unsigned int cnt_aplicacao;         // quantidade de vezes que aplicou a vacina
    pthread_mutex_t * mutex;           // mutex compartilhado
    sem_t * bancada;                  // vetor de semaforos para a bancada
}   infectado;


typedef struct laboratorio{
    pthread_t thrd_id;                  // identificacao da thread
    int id;                            // identificacao do laboratorio
    unsigned int cnt_reestoque;      // quantidade de vezes que reestocou
    pthread_mutex_t * mutex;        // mutex compartilhado
    sem_t * bancada;               // vetor de semaforos para a bancada
}   laboratorio;


// funcao que verifica se o programa deve encerrar
bool verifica_termino (int * continue_rodando){
    int i;    
    for (i=0; i < 6; i++){
        // nao terminou todas ainda, retorna falso
        if (continue_rodando[i] == 0) return false;
    }
    // atingiu o numero minimo para todos processos, retorna true para encerrar
    return true;
}

// LABORATORIO 1 - INJECAO E VIRUS MORTO (produz na bancada[0] e bancada[1])
// LABORATORIO 2 - VIRUS MORTO E INSUMO SECRETO (produz na bancada[2] e bancada[3])
// LABORATORIO 3 - INJECAO E INSUMO SECRETO (produz na bancada[4] e bancada[5])

// Virus morto - VM          Insumo secreto - IS           Injecao - INJ
// Bancada: [INJ][VM][VM][IS][INJ][IS]

void * thrd_laboratorio (void * arg){
    int prod1, prod2;
    laboratorio * lab = (laboratorio*) arg;

    // enquanto verifica_termino retornar falso, continua rodando
    while (!verifica_termino(continue_rodando)){

        if (lab->id == 0){
            // regiao critica
            pthread_mutex_lock(lab->mutex);
            // pega valores dos semaforos das suas posicoes (indica se ali tem produto ou nao)
            sem_getvalue(&lab->bancada[0], &prod1);
            sem_getvalue(&lab->bancada[1], &prod2);

            // se faltar os 2 produtos o laboratorio reestoca
            if (prod1 == 0 && prod2 == 0){
                // avisa que esta reestocado
                sem_post(&lab->bancada[0]);
                sem_post(&lab->bancada[1]);

                lab->cnt_reestoque++;

                // se a thread ja executou o minimo de vezes que deveria, indica pelo 1
                if (lab->cnt_reestoque == num_primordial){
                    continue_rodando[3] = 1;
                }
            }
            
         pthread_mutex_unlock(lab->mutex);
        }

        else if (lab->id == 1){
            // regiao critica
            pthread_mutex_lock(lab->mutex);
            // pega valores dos semaforos das suas posicoes (indica se ali tem produto ou nao)
            sem_getvalue(&lab->bancada[2], &prod1);
            sem_getvalue(&lab->bancada[3], &prod2);
            
            // se faltar os 2 produtos o laboratorio reestoca
            if (prod1 == 0 && prod2 == 0){
                // avisa que esta reestocado
                sem_post(&lab->bancada[2]);
                sem_post(&lab->bancada[3]);
                
                lab->cnt_reestoque++;

                // se a thread ja executou o minimo de vezes que deveria, indica pelo 1
                if (lab->cnt_reestoque == num_primordial){
                    continue_rodando[4] = 1;
                }
            }
            
         pthread_mutex_unlock(lab->mutex);
        }

        else if (lab->id == 2){
            // regiao critica
            pthread_mutex_lock(lab->mutex);
            // pega valores dos semaforos das suas posicoes (indica se ali tem produto ou nao)
            sem_getvalue(&lab->bancada[4], &prod1);
            sem_getvalue(&lab->bancada[5], &prod2);

            // se faltar os 2 produtos o laboratorio reestoca
            if (prod1 == 0 && prod2 == 0){
                // avisa que esta reestocado
                sem_post(&lab->bancada[4]);
                sem_post(&lab->bancada[5]);

                lab->cnt_reestoque++;

                // se a thread ja executou o minimo de vezes que deveria, indica pelo 1
                if (lab->cnt_reestoque == num_primordial){
                    continue_rodando[5] = 1;
                }
            }

         pthread_mutex_unlock(lab->mutex);
        }

    } // end while
    return NULL;
}

// LABORATORIO 1 - INJECAO E VIRUS MORTO (produz na bancada[0] e bancada[1])
// LABORATORIO 2 - VIRUS MORTO E INSUMO SECRETO (produz na bancada[2] e bancada[3])
// LABORATORIO 3 - INJECAO E INSUMO SECRETO (produz na bancada[4] e bancada[5])


// Virus morto - VM          Insumo secreto - IS           Injecao - INJ
// Bancada: [INJ][VM][VM][IS][INJ][IS]


//                                                      INJ         VM          VM          INJ
// INFECTADO 1 - INJECAO E VIRUS MORTO (procura por bancada[0], bancada[1], bancada[2] e bancada[4])

//                                                              VM         VM          IS          IS
// INFECTADO 2 - VIRUS MORTO E INSUMO SECRETO (procura por bancada[1], bancada[2], bancada[3] e bancada[5])

//                                                         INJ        IS          INJ         IS
// INFECTADO 3 - INJECAO E INSUMO SECRETO (procura por bancada[0], bancada[3], bancada[4] e bancada[5])

void * thrd_infectado (void * arg){
    int prod1, prod2, prod3, prod4;
    infectado * infec = (infectado*) arg;

    while (!verifica_termino(continue_rodando)){

        if (infec->id == 0){
            
            // regiao critica
            pthread_mutex_lock(infec->mutex);

            sem_getvalue(&infec->bancada[0], &prod1); // injecao 
            sem_getvalue(&infec->bancada[4], &prod2); // injecao
            sem_getvalue(&infec->bancada[1], &prod3); // virus morto
            sem_getvalue(&infec->bancada[2], &prod4); // virus morto

            // verifica possibilidades de combinacoes de produtos disponiveis
            if (prod1 == 1 && prod3 == 1){
                sem_wait(&infec->bancada[0]);
                sem_wait(&infec->bancada[1]);
                infec->cnt_aplicacao++;
            }
            else if (prod1 == 1 && prod4 == 1){
                sem_wait(&infec->bancada[0]);
                sem_wait(&infec->bancada[2]);
                infec->cnt_aplicacao++;
            }
            else if (prod2 == 1 && prod3 == 1){
                sem_wait(&infec->bancada[4]);
                sem_wait(&infec->bancada[1]);
                infec->cnt_aplicacao++;
            }
            else if (prod2 == 1 && prod4 == 1){
                sem_wait(&infec->bancada[4]);
                sem_wait(&infec->bancada[2]);
                infec->cnt_aplicacao++;
            }

            // se a thread ja executou o minimo de vezes que deveria, indica pelo 1
            if (infec->cnt_aplicacao == num_primordial){
                continue_rodando[0] = 1;
            }

            pthread_mutex_unlock(infec->mutex);
        }

        else if (infec->id == 1){
            // regiao critica
            pthread_mutex_lock(infec->mutex);

            sem_getvalue(&infec->bancada[3], &prod1); // insumo secreto 
            sem_getvalue(&infec->bancada[5], &prod2); // insumo secreto 
            sem_getvalue(&infec->bancada[1], &prod3); // virus morto
            sem_getvalue(&infec->bancada[2], &prod4); // virus morto

            // verifica possibilidades de combinacoes de produtos disponiveis
            if (prod1 == 1 && prod3 == 1){
                sem_wait(&infec->bancada[3]);
                sem_wait(&infec->bancada[1]);
                infec->cnt_aplicacao++;
            }
            else if (prod1 == 1 && prod4 == 1){
                sem_wait(&infec->bancada[3]);
                sem_wait(&infec->bancada[2]);
                infec->cnt_aplicacao++;
            }
            else if (prod2 == 1 && prod3 == 1){
                sem_wait(&infec->bancada[5]);
                sem_wait(&infec->bancada[1]);
                infec->cnt_aplicacao++;
            }
            else if (prod2 == 1 && prod4 == 1){
                sem_wait(&infec->bancada[5]);
                sem_wait(&infec->bancada[2]);
                infec->cnt_aplicacao++;
            }

            // se a thread ja executou o minimo de vezes que deveria, indica pelo 1
            if (infec->cnt_aplicacao == num_primordial){
                continue_rodando[1] = 1;
            }

            pthread_mutex_unlock(infec->mutex);
        }

        else if (infec->id == 2){
            // regiao critica
            pthread_mutex_lock(infec->mutex);

            sem_getvalue(&infec->bancada[3], &prod1); // insumo secreto 
            sem_getvalue(&infec->bancada[5], &prod2); // insumo secreto 
            sem_getvalue(&infec->bancada[0], &prod3); // injecao 
            sem_getvalue(&infec->bancada[4], &prod4); // injecao

            // verifica possibilidades de combinacoes de produtos disponiveis
            if (prod1 == 1 && prod3 == 1){
                sem_wait(&infec->bancada[3]);
                sem_wait(&infec->bancada[0]);
                infec->cnt_aplicacao++;
            }
            else if (prod1 == 1 && prod4 == 1){
                sem_wait(&infec->bancada[3]);
                sem_wait(&infec->bancada[4]);
                infec->cnt_aplicacao++;
            }
            else if (prod2 == 1 && prod3 == 1){
                sem_wait(&infec->bancada[5]);
                sem_wait(&infec->bancada[0]);
                infec->cnt_aplicacao++;
            }
            else if (prod2 == 1 && prod4 == 1){
                sem_wait(&infec->bancada[5]);
                sem_wait(&infec->bancada[4]);
                infec->cnt_aplicacao++;
            }

            // se a thread ja executou o minimo de vezes que deveria, indica pelo 1
            if (infec->cnt_aplicacao == num_primordial){
                continue_rodando[2] = 1;
            }

            pthread_mutex_unlock(infec->mutex);
        }

    } // end while
    return NULL;
}

int main (int argc, char **argv){
    // numero de vezes que cada thread devera executar sua funcao primordial
    num_primordial = atoll(argv[1]);

    pthread_mutex_t mutex;
    sem_t * bancada;
    int errorThread, i;

    // - - - - CREATE STRUCTURES - - - -
    infectado *infectados;
    laboratorio *laboratorios;
    infectados = malloc(sizeof(infectado) * 3);
    laboratorios = malloc(sizeof(laboratorio) * 3);
    bancada = malloc(sizeof(sem_t) * 6);
    continue_rodando = malloc(sizeof(int) * 6);

    // - - - - INIT MUTEX AND SEMAPHORES - - - -
    pthread_mutex_init (&mutex, NULL);

    for (i=0; i < 6; i++){
        sem_init(&bancada[i], 0, 0);
    }

    if (infectados == NULL || laboratorios == NULL || bancada == NULL){
        return -1;
    }

    for (i=0; i < 6; i++){
        continue_rodando[i] = 0;
    }

    // - - - - FILL STRUCTURES - - - -
    for (i=0; i < 3; i++){
        (laboratorios+i)->id = i;
        (laboratorios+i)->cnt_reestoque = 0;
        (laboratorios+i)->mutex = &mutex;
        (laboratorios+i)->bancada = bancada;
    }

    for (i=0; i < 3; i++){
        (infectados+i)->id = i;
        (infectados+i)->cnt_aplicacao = 0;
        (infectados+i)->mutex = &mutex;
        (infectados+i)->bancada = bancada;
    }

    // - - - - CREATE THREADS - - - -
    for (i=0; i < 3; i++){
        errorThread = pthread_create(&((laboratorios+i)->thrd_id), NULL, thrd_laboratorio, laboratorios+i);
        if (errorThread != 0){
            printf("erro na criacao de laboratorios %d \n", i);
            return (-1 * i);
        }
    }

    for (i=0; i < 3; i++){
        errorThread = pthread_create(&((infectados+i)->thrd_id), NULL, thrd_infectado, infectados+i);
        if (errorThread != 0){
            printf("erro na criacao de infectados %d \n", i);
            return (-1 * i);
        }
    }


    // - - - - JOIN THREADS - - - -

    for (i=0; i < 3; i++){
        errorThread = pthread_join((laboratorios+i)->thrd_id, NULL);
        if (errorThread != 0){
            printf("erro na recepcao de laboratorios %d \n", i);
            return (-2 * i);
        }
    }    

    for (i=0; i < 3; i++){
        errorThread = pthread_join((infectados+i)->thrd_id, NULL);
        if (errorThread != 0){
            printf("erro na recepcao de infectados %d \n", i);
            return (-2 * i);
        }
    }   

    // - - - - PRINT RESULTS - - - -
    for (i=0; i < 3; i++){
        printf("Infectado %d: %d \n", (infectados+i)->id, (infectados+i)->cnt_aplicacao);
    }

    for (i=0; i < 3; i++){
        printf("Laboratorio %d: %d \n", (laboratorios+i)->id, (laboratorios+i)->cnt_reestoque);
    }
    

    // - - - - END VARIABLES - - - -
    pthread_mutex_destroy(&mutex);
    sem_destroy(bancada);
    free(continue_rodando);
    free(infectados);
    free(laboratorios);
    free(bancada);

    return 0;
}