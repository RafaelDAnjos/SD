#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <windows.h>

int* continua; /* vetor de inteiros para indicar se o barbeiro vai continuar rodando */


typedef struct thread_cliente_info{
    int id;                     /*identificador do cliente*/
    int num_cadeiras;           /*numero de cadeiras total na barbearia*/
    int num_barbeiros;          /*numero de barbeiros total na barbearia*/
    int pos_fila;               /*posição do cliente nas cadeiras da barberia, ps: não usei fila*/
    int* visor;                 /*essa variável funciona como um monitor, onde o barbeiro escreve o número dele, e o cliente que acordou ele lê esse número*/
    int* v;                     /*quantidade de clientes esperando nas cadeiras*/
    sem_t* escreve_visor;       /*semaforo para impedir dois barbeiros de escreverem no visor ao mesmo tempo*/
    sem_t* le_visor;            /*semaforo para impedir dois clientes de lerem o visor ao mesmo tempo*/
    sem_t* mutex;               /*semaforo para preservar área crítica*/
    sem_t* acorda_barber;       /*semaforo utilizado para o cliente acordar o barbeiro*/
    sem_t* barbeiro;            /*vetor de semaforos de barbeiro, representa as cadeiras dos barbeiros*/
    sem_t* barbeiro_ocupado;    /*semaforo que indica se um barbeiro está ocupado ou não*/
    sem_t* cadeiras;            /*vetor de semaforos de cadeiras dos clientes*/
    sem_t* cortado;             /*semaforo que indica se o barbeiro finalizou o corte do cliente*/
    pthread_t thread_id;        /*identificador da thread*/


}thread_cliente_info;

typedef struct thread_barbeiro_info{
    int id;                     /*identificador do barbeiros*/
    int num_cadeiras;           /*numero total de cadeiras da barbearia*/
    int num_barbeiros;          /*numero total de barbeiros da barbearia*/
    int corte_minimo;           /*numero minimo de cortes que cada barbeiro tem que realizar*/
    int qtd_cortes;             /*quantidade de cortes que um barbeiro já executou*/
    int* visor;                 /*essa variável funciona como um monitor, onde o barbeiro escreve o número dele, e o cliente que acordou ele lê esse número*/
    int* v;                     /*quantidade de clientes esperando nas cadeiras*/
    int* qtd;                   /*variável que indica quantos barbeiros já atingiram a meta de cortes, usado para interromper a geração de clientes*/
    sem_t* escreve_visor;       /*semaforo para impedir dois barbeiros de escreverem no visor ao mesmo tempo*/
    sem_t* le_visor;            /*semaforo para impedir dois clientes de lerem o visor ao mesmo tempo*/
    sem_t* mutex;               /*semaforo para preservar área crítica*/
    sem_t* acorda_barber;       /*semaforo utilizado para o cliente acordar o barbeiro*/
    sem_t* barbeiro;            /*vetor de semaforos de barbeiro, representa as cadeiras dos barbeiros*/
    sem_t* barbeiro_ocupado;    /*semaforo que indica se um barbeiro está ocupado ou não*/
    sem_t* cadeiras;            /*vetor de semaforos de cadeiras dos clientes*/
    sem_t* cortado;             /*semaforo que indica se o barbeiro finalizou o corte do cliente*/
    pthread_t thread_id;        /*identificador da thread*/

}thread_barbeiro_info;

/*Função que diz se existe algum barbeiro desocupado, caso algum barbeiro esteja desocupado(dormindo)
retorna 1, caso os barbeiros estejam ocupados retorna 0*/
int barbeiro_desocupado(thread_cliente_info* cliente){
    /*i = variavel pra loop, value = variavel de retorno do valor do semaforo, cont = variavel que conta quantos barbeiros estão ocupados*/
    int i ,value,cont; 
    cont = 0;
    /*Passa por todos os barbeeiros e verifica se eles estão ocupados, se tiver ocupado ele soma 1 na variavel cont*/
    for(i=0;i<cliente->num_barbeiros;i++){
        sem_getvalue(&cliente->barbeiro[i],&value);
        if(value == 1){
            cont++;

        }
    }
    /*caso o contador seja igual ao numero de barbeiros, todos os barbeiros estão ocupados então ele retorna 0*/
    if(cont == cliente->num_barbeiros){
    
        return 0;
    }

    /*Se ele nao retorna no if anterior, algum barbeiro esta desocupado, logo ele retorna 1*/
    return 1;
}
/*Essa função é responsável por fazer um cliente sentar em uma das cadeiras de clientes disponíveis na barbearia*/
void* senta_cadeira(thread_cliente_info* cliente){
    /*i = variavel pra loop, value = variavel de retorno do valor do semaforo*/
    int i,value;
    /*Percorre a lista de semaforos de cadeiras dos clientes procurando uma cadeira vazia, caso ache uma cadeira vazia,
    o cliente se senta, se nao achar nenhuma cadeira vazia ele não faz nada e retorna NULL*/
    for(i=0;i<cliente->num_cadeiras;i++){
        /*o cliente da um lock no mutex para impedir que dois clientes alterem a cadeira ao mesmo tempo*/
        sem_wait(cliente->mutex);
        /*pega o valor de cada cadeira*/
        sem_getvalue(&cliente->cadeiras[i],&value); 
        /*se o valor daquele semaforo for 1, quer dizer que a cadeira esta vazia, então o cliente da wait naquela cadeira
        e guarda qual posição da lista de cadeiras ele está*/
        if(value == 1){
            cliente->pos_fila = i;
            /*O cliente soma +1 na variavel de quantidade de clientes esperando nas cadeiras*/
            *(cliente->v) = *(cliente->v)+1;
            sem_wait(&cliente->cadeiras[i]);
            /* ele libera o mutex para que outros clientes possam se sentar, e retorna NULL*/
            sem_post(cliente->mutex);
            return NULL;
        }
    /*Aqui ele nao conseguiu sentar na cadeira, porem ainda não liberou o mutex, então o cliente libera o mutex
    e em seguida ele retorna NULL*/
    sem_post(cliente->mutex);
    }
    
    return NULL;

}
/*Função responsavel por fazer o cliente se levantar da cadeira*/
void levanta_cadeira(thread_cliente_info* cliente){
    /*o cliente da um wait no mutex para impedir que outro cliente altere o semaforo ao mesmo tempo que ele
    o cliente da um post na cadeira que ele estava sentado(ele levanta), e por fim, ele libera o mutex
    para que outra thread possa pegar*/
    sem_wait(cliente->mutex);
    sem_post(&cliente->cadeiras[cliente->pos_fila]);
    sem_post(cliente->mutex);

}
/*Função que verifica se o barbeiro deve continuar rodando, recebe o numero de barbeiros, caso seja para continuar rodando retorna 1
caso seja pra parar de rodar recebe 0*/
int continua_rodando( int n){
    int i; /*i = variavel para proporcionar o loop*/
    /*percorre a variavel continua, onde cada posição é um indicativo se um barbeiro atingiu a meta de cortes, ex: continua[0] é indicativo do barbeiro[0],
    sendo 1 == não atingiu, e 0 == atingiu;
    caso algum barbeiro tenha um indicativo continua[i] == 1, nenhum barbeiro deve parar de rodar então retorna 1, caso todos os barbeiros
    tenham atingido a meta de cortes ele retorna 0, indicando que todos os barbeiros devem encerrar */
    for(i=0;i<n;i++){

        if(continua[i]==1){
            return 1;
        }

    }
    return 0;
}
/*Essa é a thread do cliente*/
void* thread_cliente(void* argc){
    thread_cliente_info* cliente = (thread_cliente_info*)argc;
    /*num_barbeiro == posição(indicativo) de qual barbeiro vai atender o cliente;*/
    int num_barbeiro;
    // printf("O cliente %d chegou!\n",cliente->id);
    /*O cliente chega na barbearia e verifica se a fila está vazia e se tem algum barbeiro desocupado,
    caso essas duas condições sejam atendidas ao mesmo tempo ele entra no if, se não, segue pro else*/

    // printf("Barbeiros estao ocupados\n");
    // printf("O cliente %d vai tentar sentar na cadeira\n",cliente->id);
    /*O cliente sabe que os barbeiros estão ocupados, ou que ja tem gente esperando pra ser atendido, então ele verifica se existe uma
    cadeira vaga pra ele sentar, se existir ele entra no if, se não ele vai pro else*/
    if(*(cliente->v)<cliente->num_cadeiras){
        // printf("Existe uma cadeira vaga, o cliente %d vai se sentar\n", cliente->id);
        /*Como existe uma cadeira vaga o cliente senta em uma cadeira chamando a função senta_cadeira()*/
        senta_cadeira(cliente);
        // printf("são %d clientes esperando\n",*(cliente->v));
        /*o cliente então espera um barbeiro desocupar*/
        sem_wait(cliente->barbeiro_ocupado);
        /*Como o barbeiro ficou desocupado(barbeiro dormiu), o cliente vai acordar o barbeiro*/
        sem_post(cliente->acorda_barber);
        /*O cliente então fica esperando(ele espera no wait) o barbeiro que ele acordou escrever no visor qual cadeira é para ele se sentar,
        e liberar o semaforo le_visor*/
        sem_wait(cliente->le_visor);
        /*o cliente le qual barbeiro vai atende-lo*/
        num_barbeiro = *(cliente->visor);
        /*Agora que garnti, que o cliente leu o valor do barbeiro correto libero o semaforo escreve_visor, para que outro barbeiro
        possa escrever no visor.*/
        sem_post(cliente->escreve_visor);
        /*O cliente desocupa a cadeira de cliente que ele estava ocupando*/
        levanta_cadeira(cliente);
        
        // printf("O cliente %d acordou o barbeiro %d \n",cliente->id,num_barbeiro);
        /*O cliente da um post no semaforo do barbeiro que vai atenter ele(ele senta na cadeira do barbeiro)*/
        sem_post(&cliente->barbeiro[num_barbeiro]);
        
        // printf("O barbeiro %d esta cortando o cabelo do cliente %d\n",num_barbeiro,cliente->id);
        /*O cliente fica esperando até ter seu cabelo cortado*/
        sem_wait(cliente->cortado);
        // printf("\n\n\n TA QUEBRANDO AQUI!\n\n\n");
        pthread_exit(NULL);

    }else{
        /*A barbearia esta lotada e o cliente vai embora*/

        // printf("Barbearia Lotada! cliente %d foi embora!\n",cliente->id);
        pthread_exit(NULL);
        return NULL;

    }

    
    

    return NULL;
}

void* thread_barbeiro(void* argc){
    thread_barbeiro_info* barber = (thread_barbeiro_info*)argc;
    barber->qtd_cortes = 0;
    /*O barbeiro chama a função continua_rodando, para verificar se deve continuar a rodar ou não.*/
    while(continua_rodando(barber->num_barbeiros)){
        
        // printf("O barbeiro %d esta dormindo \n",barber->id);
        /* o barbeiro verifica se ele já atingiu a meta de cortes, se ele já tiver atingido ele verifica se deve continuar rodando,
        caso ele deva encerrar sua execução ele retorna a quantidade de cortes que ele realizou*/
        if(continua[barber->id] == 0){
            if(continua_rodando(barber->num_barbeiros)==0){
                return (void*)barber->qtd_cortes;

            }
        }
        /*O barbeiro dorme no wait até ser acordado por algum cliente*/
        sem_post(barber->barbeiro_ocupado);
        sem_wait(barber->acorda_barber);
        /*O barbeiro verifica se ele foi acordado por algum cliente ou se foi um sinal para ele finalizar a execução,
        esse sinal impede que o barbeiro espere por um cliente eternamente, pois em determinado momento os clientes param de serem gerados,
        caso ele deva parar de rodar, ele retorna a quantidade de cortes que ele fez, caso ele deva continuar, ele prossegue sua execução*/
        if(continua_rodando(barber->num_barbeiros)==0){
                return (void*)barber->qtd_cortes;

        }
        // printf("O barbeiro %d Acordou \n",barber->id);
        /*O barbeiro pega o semaforo de escrever no visor(caso seja o primeiro barbeiro a escrever no visor),
        caso contrário ele espera até um cliente mandar um sinal dizendo que ele pode pegar esse semaforo*/
        sem_wait(barber->escreve_visor);
        /*O barbeiro escreve seu ID no visor, para possibilitar o cliente identificar quem ele é.*/
        *(barber->visor) = barber->id;
        /*O barbeiro libera o cliente que o acordou para ler o visor*/
        sem_post(barber->le_visor);
        /*Caso o cliente que esteja sendo atendido tenha vindo das cadeiras de clientes, o barbeiro diminui em 1 o 
        numero de clientes esperando na fila*/
        /*O barbeiro tenta pegar o mutex, para entrar na área crítica*/
        sem_wait(barber->mutex);
        if(*(barber->v)>0){
            *(barber->v) = *(barber->v)-1;
        }
        /*O barbeiro libera o mutex, saindo da região crítica*/
        sem_post(barber->mutex);
        /*O barbeiro corta o cabelo do cliente*/
        // Sleep(3000);
        sem_post(barber->cortado);
        /*Após cortar o cabelo ele incrementa sua própria quantidade de cortes*/
        barber->qtd_cortes++;
        /*o barbeiro diz que o cabelo do cliente foi cortado, liberando o semaforo onde o cliente estava preso*/
        /*O barbeiro indica que esta desocupdo*/
        sem_wait(&barber->barbeiro[barber->id]);
        
        /*Quando o barbeiro atinge a meta de cortes, ele muda o vetor de inteiros continua na sua própria posição para 0
        ou seja, continua[id_barbeiro] = 0, indicando que ele ja atingiu a meta.*/
        if(barber->qtd_cortes==barber->corte_minimo){
            printf("O barbeiro %d atingiu a meta de cortes\n", barber->id);
            continua[barber->id] = 0;
            /*Aqui o barbeiro precisa entrar em uma segunda região crítica, então ele se apossa do mutex, para indicar através da variável qtd
            que ele já alcançou a meta*/
            sem_wait(barber->mutex);
            *(barber->qtd) = *(barber->qtd)+1;
            /*ele libera o mutex e sai da região crítica*/
            sem_post(barber->mutex);
        }
        
    }
    /*O barbeiro retorna a quantidade de cortes que ele realizou*/
    return (void*)barber->qtd_cortes;
}

int main(int argc, char* argv[]){
    /*----------DECLARAÇÃO DE VARIÁVEIS-------------*/
    int num_barbeiros = atoi(argv[1]);/*NUMERO DE BARBEIROS RECEBIDO ATRAVÉS DE CLI*/
    int num_cadeiras = atoi(argv[2]);/*NUMERO DE CADEIRAS RECEBIDO ATRAVÉS DE CLI*/
    int num_vezes = atoi(argv[3]);/*NUMERO MINIMO DE VEZES RECEBIDO ATRAVÉS DE CLI*/ 
    int i = 0;/*VARIAVEL INTEIRA PARA LOOPS FOR*/
    int j = 0;/*VARIAVEL INTEIRA PARA ID DOS CLIENTES GERADOS*/
    int v = 0;/*NUMERO DE CLIENTES ESPERANDO NAS CADEIRAS DE CLIENTE DA BARBEARIA*/
    int qtd = 0;/*QUANTIDADE DE BARBEIROS QUE JA ATINGIRAM A META*/
    int cortes;/*VARIAVEL PARA RECEBER O RETORNO DOS BARBEIROS*/
    int error; /*VARIAVEL PARA PEGAR O ERRO DA CRIACAO DA THREAD*/
    int visor=9999;/*VISOR ONDE OS BARBEIROS ESCREVEM SEU ID, E OS CLIENTES LEEM QUAL BARBEIRO BAI ATENDE-LO*/
    void* s;/*VARIAVEL PARA RECEBER O RETORNO DOS BARBEIROS*/

    
    sem_t escreve_visor;/*SEMAFORO PARA CONTROLAR QUEM ESCREVE NO VISOR*/
    sem_t le_visor;/*SEMAFORO PARA CONTROLAR QUEM LE O VISOR*/
    sem_t mutex;/*SEMAFORO PRA EXCLUSÃO MUTUA NO GERAL*/
    sem_t acorda_barber;/*SEMAFORO ONDE O CLIENTE ACORDA O BARBEIRO, O BARBEIRO ESPERA NESSE SEMAFORO ATE SER ACORDADO*/
    sem_t cortado;/*SEMAFORO ONDE O BARBEIRO LIBERA O CLIENTE E MANDA ELE EMBORA*/
    sem_t barbeiro_ocupado;/*SEMAFORO QUE INDICA QUE O BARBEIRO ESTA OCUPADO OU DESOCUPADO*/
    
    sem_t* barbeiro = (sem_t*)malloc(num_barbeiros*sizeof(sem_t));/*UM VETOR DE SEMAFOROS, ONDE CADA POSIÇÃO É UM SEMAFORO DE UM BARBEIRO*/
    sem_t* cadeiras =  (sem_t*)malloc(num_cadeiras*sizeof(sem_t));/*UM VETOR DE SEMAFOROS, ONDE CADA POSIÇÃO É UM SEMAFORO INDICANDO UMA CADEIRA*/

    /*PACOTE DE INFORMAÇÕES DOS BARBEIROS, CADA POSIÇÃO É O PACOTE DE UM BARBEIRO*/
    thread_barbeiro_info* barber_info = (thread_barbeiro_info*)malloc(num_barbeiros*sizeof(thread_barbeiro_info));
    thread_cliente_info* cliente; /*PACOTE DE INFORMAÇÕES DO CLIENTE*/
    
    
    
    if(num_vezes>0){

        /*---------------INICIANDO VARIAVEL CONTINUA-----------------*/
        continua = (int*)malloc(num_barbeiros*sizeof(int));
        /*INICIA TODAS AS POSIÇÕES DE CONTINUA COM 1, INDICANDO QUE NENHUM BARBEIRO ATINGIU A META*/
        for(i=0;i<num_barbeiros;i++){
            continua[i] = 1;
        }
        
        /*-----------------INICIANDO OS SEMAFOROS-----------------*/
        sem_init(&escreve_visor,0,1);
        sem_init(&le_visor,0,0);
        sem_init(&mutex,0,1);
        sem_init(&cortado,0,0);
        sem_init(&acorda_barber,0,0);
        sem_init(&barbeiro_ocupado,0,0);
        

        for(i=0;i<num_barbeiros;i++){
            sem_init(&barbeiro[i],0,0);
        
        }
        for(i=0;i<num_cadeiras;i++){
        
            sem_init(&cadeiras[i],0,1);
        }
        /*------------------PREENCHENDO OS PACOTES DE INFORMAÇÃO DOS BARBEIROS-----------------------*/
        for(i=0;i<num_barbeiros;i++){
            barber_info[i].barbeiro = barbeiro;
            barber_info[i].cadeiras = cadeiras;
            barber_info[i].corte_minimo = num_vezes;
            barber_info[i].escreve_visor = &escreve_visor;
            barber_info[i].id = i;
            barber_info[i].le_visor = &le_visor;
            barber_info[i].mutex = &mutex;
            barber_info[i].num_cadeiras = num_cadeiras;
            barber_info[i].visor = &visor;
            barber_info[i].v = &v;
            barber_info[i].qtd = &qtd;
            barber_info[i].acorda_barber = &acorda_barber;
            barber_info[i].cortado = &cortado;
            barber_info[i].barbeiro_ocupado = &barbeiro_ocupado;
            barber_info[i].num_barbeiros = num_barbeiros;

        }
        /*---------------------------INICIANDO AS THREADS DOS BARBEIROS------------------------------*/
        for(i=0;i<num_barbeiros;i++){
            error = pthread_create(&barber_info[i].thread_id,NULL,thread_barbeiro,&(barber_info[i]));
            if(error != 0 ){
                printf("Erro na criacao da thread barbeiro %d\n",i);
                return -1*i;
            }
        }
        /*-------------------------PREENCHENDO E INICIANDO INFINITOS CLIENTES---------------------------*/
        /*OS CLIENTES SÃO GERADOR ATÉ TODOS OS BARBEIROS ATINGIREM A META, QUANDO TODOS ATINGEM NENHUM CLIENTE É GERADO MAIS*/
        while(qtd<num_barbeiros){
            
            cliente = (thread_cliente_info*)malloc(sizeof(thread_cliente_info));
            cliente->id = j;
            j++;
            cliente->barbeiro = barbeiro;
            cliente->cadeiras = cadeiras;
            cliente->escreve_visor = &escreve_visor;
            cliente->mutex = &mutex;
            cliente->le_visor = &le_visor;
            cliente->mutex = &mutex;
            cliente->num_cadeiras = num_cadeiras;
            cliente->num_barbeiros = num_barbeiros;
            cliente->visor = &visor;
            cliente->v = &v;
            cliente->acorda_barber = &acorda_barber;
            cliente->cortado = &cortado;
            cliente->barbeiro_ocupado = &barbeiro_ocupado;

            error = pthread_create(&cliente[0].thread_id,NULL,thread_cliente,&(cliente[0]));
            if(error!=0){
                //printf("Erro na criacao da thread cliente %d\n",j);
                // return -1*j;
            }
            //  Sleep(100);
            
        }
        /*MANDA UM SINAL PARA TODOS OS BARBEIROS, CASO ALGUM ESTEJA DORMINDO, ELE VAI FINALIZAR SUA EXECUÇÃO, POIS NÃO TEM MAIS CLIENTES, E TODOS OS 
        BARBEIROS JA ATINGIRAM A META*/
        for(i=0;i<num_barbeiros;i++){
            sem_post(&acorda_barber);

        }
        /*--------------------------JOIN DOS BARBEIROS------------------------*/
        /*RECUPERO AS INFORMAÇÕES DOS BARBEIROS E PRINTO QUANTOS CORTES CADA UM REALIZOU*/
        for(i=0;i<num_barbeiros;i++){

            error = pthread_join(barber_info[i].thread_id,&s);
            if(error!=0){

                printf("Erro no join do barbeiro %d",i);
                
            }
            cortes = (int)s;
            printf("O barbeiro %d cortou %d cabelos!\n",i,cortes);

        }

        
    }else{
            for(i=0;i<num_barbeiros;i++){

            printf("O barbeiro %d cortou %d cabelos!\n",i,0);

        }
        
    }

    


    return 0;
}