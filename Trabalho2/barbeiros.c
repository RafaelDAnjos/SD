#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <windows.h>




typedef struct thread_barbeiro_info{
    pthread_t thread_id;    /*VARIAVEL DE IDENTIFICAÇÃO DA THREAD*/
    int id;                 /*ID DO BARBEIRO*/
    int num_cadeiras;       /*NUMERO TOTAL DE CADEIRAS*/
    int num_barbeiros;      /*NUMERO TOTAL DE BARBEIROS*/
    int num_vezes;          /*NUMERO MINIMO DE CORTES PRA CADA BARBEIRO,(META)*/
    int qtd_cortes;         /*QUANTIDADE DE CORTES QUE CADA BARBEIRO REALIZOU*/
    int* termina_cliente;   /*VARIAVEL COMPARTILHADA ENTRE AS THREADS PARA ITERROMPER A GERAÇÃO DE CLIENTES*/
    int* visor;             /*VARIAVEL ONDE O BARBEIRO ESCREVE E O CLIENTE LE, O NUMERO DO BARBEIRO QUE VAI ATENDER!*/                
    int* continua;          /*VETOR ONDE CADA POSIÇÃO INDICA SE AQUELE BARBEIRO JÁ ATINGIU A META OU NÃO*/
    sem_t* escreve_visor;   /*SEMAFORO PRA CONTROLAR QUEM PODE ESCREVER NO VISOR*/
    sem_t* le_visor;        /*SEMAFORO PRA CONTROLAR QUEM PODE LER O VISOR*/
    sem_t* cadeiras;        /*SEMAFORO QUE INDICA A QUANTIDADE DE CADEIRAS DISPONIVEIS, E REGULA QUAL CLIENTE VAI SENTAR NA FILA PRA SER ATENDIDO*/
    sem_t* cad_barbeiros;   /*VETOR DE SEMAFOROS ONDE CADA POSIÇÃO É UM SEMAFORO, QUE REPRESENTA A CADEIRA DE UM BARBEIRO*/
    sem_t* cabelo_cortado;  /*VETOR DE SEMAFOROS, CADA POSIÇÃO INDICA SE AQUELE BARBEIRO TERMINOU DE CORTAR O CABELO DE UM CLIENTE*/

}thread_barbeiro_info;

typedef struct thread_cliente_info{
    pthread_t thread_id;    /*VARIAVEL DE IDENTIFICAÇÃO DA THREAD*/
    int id;                 /*ID DO CLIENTE*/
    int num_cadeiras;       /*NUMERO TOTAL DE CADEIRAS*/
    int num_barbeiros;      /*NUMERO TOTAL DE BARBEIROS*/
    int num_vezes;          /*NUMERO MINIMO DE CORTES PRA CADA BARBEIRO,(META)*/
    int* visor;             /*VARIAVEL ONDE O BARBEIRO ESCREVE E O CLIENTE LE, O NUMERO DO BARBEIRO QUE VAI ATENDER!*/
    sem_t* escreve_visor;   /*SEMAFORO PRA CONTROLAR QUEM PODE ESCREVER NO VISOR*/
    sem_t* le_visor;        /*SEMAFORO PRA CONTROLAR QUEM PODE LER O VISOR*/
    sem_t* cadeiras;        /*SEMAFORO QUE INDICA A QUANTIDADE DE CADEIRAS DISPONIVEIS, E REGULA QUAL CLIENTE VAI SENTAR NA FILA PRA SER ATENDIDO*/
    sem_t* cad_barbeiros;   /*VETOR DE SEMAFOROS ONDE CADA POSIÇÃO É UM SEMAFORO, QUE REPRESENTA A CADEIRA DE UM BARBEIRO*/
    sem_t* cabelo_cortado;  /*VETOR DE SEMAFOROS, CADA POSIÇÃO INDICA SE AQUELE BARBEIRO TERMINOU DE CORTAR O CABELO DE UM CLIENTE*/
    

}thread_cliente_info;
/*RECEBE UM PACOTE DE INFORMAÇÕES SOBRE O BARBEIRO, PERCORRE O VETOR DE INTEIROS 'CONTINUA', CASO ACHE ALGUMA POSIÇÃO == 1, SIGNIFICA QUE TODOS OS
BARBEIROS DEVEM CONTINUAR RODANDO, POIS EXISTE UM BARBEIRO QUE AINDA NÃO ATINGIU A META, ENTÃO RETORNA 1, CASO CONTRÁRIO TODOS OS BARBEIROS 
TERMINARAM, ENTÃO RETORNA 0.*/
int continua_rodando(thread_barbeiro_info* barbeiro){
    int i;
    for(i=0;i<barbeiro->num_barbeiros;i++){
        if(barbeiro->continua[i]==1){
            return 1;
        }
    }
    return 0;
}
/*ESSA É A FUNÇÃO DO BARBEIRO:
ELA RECEBE COMO PARAMETRO UM PONTEIRO PRA VOID, QUE É O PACOTE DE INFORMAÇÕES DO BARBEIRO, E RETORNA A QUANTIDADE DE CABELOS QUE AQUELE BARBEIRO 
CORTOU.
1- O BARBEIRO SETA A QUANTIDADE DE CABELOS CORTADOS PRA 0.
2- CHAMA A FUNÇÃO CONTINUA RODANDO E PASSA O PACOTE DE INFORMAÇÕES DO BARBEIRO COMO PARAMETRO.
3- SE CONTINUAR RODANDO:
    3.1- O BARBEIRO TENTA PEGAR O SEMAFORO PRA ESCREVER NO VISOR, LOGO APENAS UM BARBEIRO PODE ESCREVER POR VEZ.
    3.2- VERIFICA SE É PRA PARAR DE RODAR, CASO SEJA RETORNA A QUANTIDADE DE CORTES, CASO CONTRÁRIO CONTINUA A EXECUÇÃO.
    3.3- O BARBEIRO ESCREVE SEU ID NO VISOR
    3.4- O BARBEIRO INDICA QUE O CLIENTE JÁ PODE LER O VISOR COM SEGURANÇA.
    3.5- O BARBEIRO ESPERA O CLIENTE SENTAR NA CADEIRA DELE.
    3.6- VERIFICA SE É PRA PARAR DE RODAR, CASO SEJA RETORNA A QUANTIDADE DE CORTES, CASO CONTRÁRIO CONTINUA A EXECUÇÃO.
    3.7- O BARBEIRO CORTA O CABELO DO CLIENTE, E INCREMENTA EM 1 A QUANTIDADE DE CABELOS CORTADOS.
    3.8- SINALIZA PRO CLIENTE QUE TERMINOU DE CORTAR O CABELO DELE.
    3.9- VERIFICA SE ELE JÁ ATINGIU A META DE CORTES, CASO ELE TENHA ATINGIDO MUDA O VALOR DA VARIVEL CONTINUA[POSIÇÃO BARBEIRO] PARA 0,
        INDICANDO ASSIM QUE ELE ATINGIU A META, TAMBEM INCREMENTA 1 NA VARIAVEL TERMINA CLIENTE, INDICANDO ASSIM QUE ELE JÁ ATINGIU A META.
        3.9.1- VERIFICA SE É PRA PARAR DE RODAR, CASO SEJA RETORNA A QUANTIDADE DE CORTES, CASO CONTRÁRIO CONTINUA A EXECUÇÃO.
4- SE NAO CONTINUAR RODANDO ACABA O WHILE E ELE RETORNA A QUANTIDADE DE CABELOS CORTADOS.
*/
void* thread_barbeiro(void* argc){
    thread_barbeiro_info* barbeiro = (thread_barbeiro_info*)argc;
    barbeiro->qtd_cortes = 0;
    /*printf("Ta entrando aqui\n\n");*/
    /*printf("continua: %d\n\n",barbeiro->continua[0]);*/
    while(continua_rodando(barbeiro)){
        /*printf("O barbeiro %d esta dormindo\n",barbeiro->id);*/
        sem_wait(barbeiro->escreve_visor);
        if (continua_rodando(barbeiro)==0){
            return (void*)barbeiro->qtd_cortes;
        }
        *(barbeiro->visor) = barbeiro->id;
        sem_post(barbeiro->le_visor);
        /*printf("O barbeiro %d esta esperando um cliente sentar na cadeira\n",barbeiro->id);*/
        sem_wait(&barbeiro->cad_barbeiros[barbeiro->id]);
        if (continua_rodando(barbeiro)==0){
            return (void*)barbeiro->qtd_cortes;
        }
        /*Sleep(3000);*/
        barbeiro->qtd_cortes++;
        /*printf("qtd cortes %d\n",barbeiro->qtd_cortes);*/
        sem_post(&barbeiro->cabelo_cortado[barbeiro->id]);
        if(barbeiro->qtd_cortes == barbeiro->num_vezes){
            /*printf("Barbeiro %d atingiu a meta de cortes\n",barbeiro->id);*/
            barbeiro->continua[barbeiro->id] = 0;
            *(barbeiro->termina_cliente) = *(barbeiro->termina_cliente)+1;
            if (continua_rodando(barbeiro)==0){
                /*printf("entrou pra terminar\n");*/
                return (void*)barbeiro->qtd_cortes;
            } 

        }
        
    }


    
    return (void*)barbeiro->qtd_cortes;
}
/*ESSA É A FUNÇÃO DO CLIENTE:
ELA RECEBE COMO PARAMETRO UM PONTEIRO PRA VOID QUE É O PACOTE DE INFORMAÇÕES SOBRE O CLIENTE, E RETORNA NULO.
1- O CLIENTE TENTA SENTAR EM UMA DAS CADEIRAS DA BARBEARIA.
    1.1- APÓS CONSEGUIR SENTAR EM UMA CADEIRA, O CLIENTE FICA AGUARDANDO O BARBEIRO LIBERAR ELE PARA LER COM SEGURANÇA O VISOR.
    1.2- ELE LE O VISOR, E SALVA NUMA VARIAVEL LOCAL CHAMADA MINHA_CADEIRA.
    1.3- ELE SINALIZA QUE JÁ TERMINOU DE LER A VARIAVEL, ENTÃO OUTRO BARBEIRO PODE ESCREVER NO VISOR COM SEGURANÇA.
    1.4- ELE LEVANTA A CADEIRA DE CLIENTE QUE ELE ESTAVA OCUPANDO.
    1.5- ELE SINALIZA PRO BARBEIRO QUE SENTOU NA CADEIRA DELE.
    1.6- ELE ESPERA TER SEU CABELO CORTADO.
    1.7- ELE VAI EMBORA RETORNANDO NULO.
2- SE NÃO CONSEGUIR SE SENTAR, ELE VAI EMBORA DA BARBEARIA RETORNANDO NULO.
*/
void* thread_cliente(void* argc){
    thread_cliente_info* cliente = (thread_cliente_info*)argc;
    int minha_cadeira;
    /*printf("Cliente %d esta tentando entrar na barbearia\n", cliente->id);*/
    if(sem_trywait(cliente->cadeiras) == 0){
        sem_wait(cliente->le_visor);
        minha_cadeira = *(cliente->visor);
        sem_post(cliente->escreve_visor);
        sem_post(cliente->cadeiras);
        sem_post(&cliente->cad_barbeiros[minha_cadeira]);
        /*printf("O barbeiro %d ta cortando o cabelo do cliente %d\n",minha_cadeira,cliente->id);*/
        sem_wait(&cliente->cabelo_cortado[minha_cadeira]);
    }else{
        /*printf("O cliente %d foi embora! Barbearia Lotada \n",cliente->id);*/
    }
    return NULL;
}

int main(int argc, char* argv[]){
    int num_barbeiros = atoi(argv[1]);  /*NUMERO TOTAL DE CADEIRAS*/
    int num_cadeiras = atoi(argv[2]);   /*NUMERO TOTAL DE BARBEIROS*/
    int num_vezes = atoi(argv[3]);      /*NUMERO MINIMO DE CORTES PRA CADA BARBEIRO,(META)*/
    int i = 0;                          /*VARIAVEL USADA EM LOOPS FOR*/
    int j = 0;                          /*VARIAVEL USADA PARA GERAR O ID DO CLIENTE*/
    int termina_cliente = 0;            /*VARIAVEL COMPARTILHADA ENTRE AS THREADS PARA ITERROMPER A GERAÇÃO DE CLIENTES*/
    int visor;                          /*VARIAVEL ONDE O BARBEIRO ESCREVE E O CLIENTE LE, O NUMERO DO BARBEIRO QUE VAI ATENDER!*/                
    int cortes =0;                      /*VARIVEL USADA PARA RESGATAR A QUANTIDADE DE CORTES DE CADA BARBEIRO*/
    int  error;
    int value;
    int* continua = (int*)malloc(num_barbeiros*sizeof(int));/*VETOR ONDE CADA POSIÇÃO INDICA SE AQUELE BARBEIRO JÁ ATINGIU A META OU NÃO*/
    void* s;                            /*VARIAVEL PRA PEGAR O RETORNO DA THREAD BARBEIRO*/
    sem_t escreve_visor;                /*SEMAFORO PRA CONTROLAR QUEM PODE ESCREVER NO VISOR*/
    sem_t le_visor;                     /*SEMAFORO PRA CONTROLAR QUEM PODE LER O VISOR*/
    sem_t cadeiras;                     /*SEMAFORO QUE INDICA A QUANTIDADE DE CADEIRAS DISPONIVEIS, E REGULA QUAL CLIENTE VAI SENTAR NA FILA PRA SER ATENDIDO*/
    sem_t* cad_barbeiros = (sem_t*)malloc(num_barbeiros*sizeof(sem_t));/*VETOR DE SEMAFOROS ONDE CADA POSIÇÃO É UM SEMAFORO, QUE REPRESENTA A CADEIRA DE UM BARBEIRO*/
    sem_t* cabelo_cortado = (sem_t*)malloc(num_barbeiros*sizeof(sem_t));/*VETOR DE SEMAFOROS, CADA POSIÇÃO INDICA SE AQUELE BARBEIRO TERMINOU DE CORTAR O CABELO DE UM CLIENTE*/
    thread_barbeiro_info* barbeiros = (thread_barbeiro_info*)malloc(num_barbeiros*sizeof(thread_barbeiro_info));/*PACOTE DE INFORMAÇÕES DO BARBEIRO*/
    thread_cliente_info* cliente;       /*PACOTE DE INFORMAÇÕES DO CLIENTE*/
    
    if(num_vezes>0&&num_cadeiras>0){

        /*---------------------------------------INICIANDO OS SEMAFOROS----------------------------*/
        sem_init(&escreve_visor,0,1);
        sem_init(&le_visor,0,0);
        sem_init(&cadeiras,0,num_cadeiras);
        
        
        for(i=0;i<num_barbeiros;i++){
            sem_init(&cad_barbeiros[i],0,0);
            sem_init(&cabelo_cortado[i],0,0);
            continua[i] = 1;
        }

        /*--------------------------PREENCHENDO OS PACOTES DE INFORMAÇÃO DOS BARBEIROS--------------------------------------------*/

        for(i=0;i<num_barbeiros;i++){
            barbeiros[i].cabelo_cortado = cabelo_cortado;
            barbeiros[i].cad_barbeiros = cad_barbeiros;
            barbeiros[i].termina_cliente = &termina_cliente;
            barbeiros[i].id = i;
            barbeiros[i].continua = continua;
            barbeiros[i].num_barbeiros = num_barbeiros;
            barbeiros[i].num_cadeiras = num_cadeiras;
            barbeiros[i].num_vezes = num_vezes;
            barbeiros[i].le_visor = &le_visor;
            barbeiros[i].visor = &visor;
            barbeiros[i].escreve_visor = &escreve_visor;
        }
        /*--------------------------------INCIANDO AS THREADS DO BARBEIRO--------------------------------*/
        for(i=0;i<num_barbeiros;i++){

            pthread_create(&barbeiros[i].thread_id,NULL,thread_barbeiro,&(barbeiros[i]));

        }
        /*------------------------------------PREENCHENDO E INICIANDO INFINITAS THREADS CLIENTES-------------------------------------------*/
        while(termina_cliente<num_barbeiros){
            cliente = (thread_cliente_info*)malloc(sizeof(thread_cliente_info));
            cliente->cabelo_cortado = cabelo_cortado;
            cliente->cad_barbeiros = cad_barbeiros;
            cliente->cadeiras = &cadeiras;
            
            cliente->id = j;
            j++;
            
            cliente->num_barbeiros = num_barbeiros;
            cliente->num_cadeiras = num_cadeiras;
            cliente->num_vezes = num_vezes;
            cliente->le_visor = &le_visor;
            cliente->visor = &visor;
            cliente->escreve_visor = &escreve_visor;
            
            error =pthread_create(&cliente[0].thread_id,NULL,thread_cliente,&(cliente[0]));
            /*Sleep(1000);*/
            /*printf("termina cliente: %d \n",termina_cliente);*/
            if(error!=0){

            printf("thread error %d\n\n",j);
            }
            free(cliente);
        }
        sem_getvalue(cadeiras,&value);
        printf("lugares de cadeira: %d",value);
        /*-------------------LIBERANDO POSSIVEIS BARBEIROS ONDE ELES PODEM FICAR PRESOS, INDICANDO QUE TODOS JÁ ATINGIRAM A META---------------------*/
        for(i=0;i<num_barbeiros;i++){

            sem_post(&cad_barbeiros[i]);
            sem_post(&escreve_visor);
            
        }
        /*------------------------------------DANDO JOIN NOS BARBEIROS E PRINTANDO QUANTOS CABELOS CADA UM CORTOU--------------------------------------------------------*/
        for(i=0;i<num_barbeiros;i++){

            pthread_join(barbeiros[i].thread_id,&s);
            cortes = (int)s;

            printf("O barbeiro %d cortou %d cabelos\n",i,cortes);

        }
     
    }else{

        for(i=0;i<num_barbeiros;i++){

            printf("O barbeiro %d cortou %d cabelos\n",i,cortes);

        }

    }

    return 0;
}