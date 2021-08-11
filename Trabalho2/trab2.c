#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <windows.h>

int v = 0;
int num_cadeiras_cliente;
int visor;
sem_t escreve_visor;
sem_t le_visor;

typedef struct thread_cliente_info{
    int id;
    int num_cadeiras;
    pthread_t thread_id;
    sem_t* barbeiro;
    sem_t* cadeiras_barbearia;
    sem_t* exclusao_mutua;

}thread_cliente_info;

typedef struct thread_barbeiro_info{
    int id;
    int num_cadeiras;
    int corte_minimo;
    int qtd_cortes;
    pthread_t thread_id;
    sem_t* barbeiro;
    sem_t* cadeiras_barbearia;
    sem_t* exclusao_mutua;
}thread_barbeiro_info;

void* thread_cliente(void* argc){
    thread_cliente_info* cliente = (thread_cliente_info*)argc;
    int minha_cadeira;

    printf("O cliente %d ta tentando entrar na barbearia \n", cliente->id);
    /*O cliente tenta pegar o mutex pra entrar na região crítica*/
    sem_wait(cliente->exclusao_mutua);
    /*O cliente verifica se tem cadeira disponível pra sentar, se tiver ele se senta, se não ele vai embora*/
    if(v<num_cadeiras_cliente){
        printf("O cliente %d entrou na barbearia\n",cliente->id);
        v = v+1;
        printf("existem %d clientes esperando\n", v);
        /*ele senta na cadeira dos clientes e acorda o barbeiro*/
        sem_post(cliente->cadeiras_barbearia);
        /*Cliente libera o mutex*/
        sem_post(cliente->exclusao_mutua);
        /*Cliente espera até o barbeiro colocar ele na cadeira*/
        sem_wait(&le_visor);
        printf("Cliente %d, pegou o leitor do visor\n",cliente->id);
        minha_cadeira = visor;
        sem_post(&escreve_visor);
        sem_wait(cliente->barbeiro[minha_cadeira]);

        printf("O cliente %d esta tendo o cabelo cortado, pelo barbeiro %d!\n",cliente->id,minha_cadeira);

    }else{
        sem_post(cliente->exclusao_mutua);
        
        printf("Barbearia lotada, o cliente %d foi embora\n",cliente->id);

    }
    

    printf("Cliente %d foi embora!\n",cliente->id);
    
    return NULL;

}
void* thread_barbeiro(void* argc){
    thread_barbeiro_info * pacote = (thread_barbeiro_info*)argc;
    int i = 0;
    /*o barbeiro corta apenas x cabelos*/
    while(i<pacote->corte_minimo){
        printf("O barbeiro %d esta dormindo\n",pacote->id);
    /*a thread do barbeiro dorme aqui enquanto o numero de clientes na cadeira for igual a 0 */
        sem_wait(pacote->cadeiras_barbearia);
        printf("Barbeiro %d Acordou!\n",pacote->id);
        /*o barbeiro acorda e tenta pegar o mutex pra entrar na região crítica*/
        sem_wait(pacote->exclusao_mutua);
        
        /* o barbeiro pega o mutex e coloca o cliente sentado na sua cadeira */
        printf("Barbeiro %d pronto pra cortar cabelo \n",pacote->id);
        v = v - 1;
        /* o barbeiro incrementa o semaforo dele mesmo para avisar ao cliente que ele ta disponivel*/
        sem_post(pacote->barbeiro[pacote->id]);
        /* o barbeiro libera o mutex e corta o cabelo do cliente*/
        sem_post(pacote->exclusao_mutua);
        sem_wait(&escreve_visor);
        visor = pacote->id;
        sem_post(&le_visor);

        pacote->qtd_cortes++;
        Sleep(3000);

        i++;
    }


    return NULL;
}


int main(int argc, char* argv[]){
    int num_barbeiros = atoi(argv[1]);
    int num_cadeiras = atoi(argv[2]);
    int num_vezes = atoi(argv[3]);
    int i,j;
    sem_t cadeiras_cliente;
    sem_t* barbeiro = (sem_t*)malloc(num_barbeiros*sizeof(sem_t));
    sem_t exclusao_mutua;
    thread_cliente_info* cliente ;
    thread_barbeiro_info* barbeiro_info = (thread_barbeiro_info*)malloc(num_barbeiros*sizeof(thread_barbeiro_info));
    j=0;
    num_cadeiras_cliente = num_cadeiras;
    /*iniciando o semaforo dos barbeiros, das cadeiras da barbearia, e o mutex*/
    for(i=0;i<num_barbeiros;i++);{
        sem_init(&barbeiro[i],0,0);

    }
    sem_init(&escreve_visor,0,1);
    sem_init(&le_visor,0,0);
    sem_init(&cadeiras_cliente,0,0);
    sem_init(&exclusao_mutua,0,1);


    printf("Sao %d barbeiros;\n",num_barbeiros);
    printf("Sao %d cadeiras para clientes;\n",num_cadeiras);
    printf("Cada barbeiro deve cortar ao menos %d cabelos;\n",num_vezes);

    
    /* inicializando os barbeiros e dando create na thread do barbeiro*/
    printf("iniciando o barbeiro\n");
    for(i=0;i<num_barbeiros;i++){
            
        barbeiro_info[i].id = i;
        barbeiro_info[i].qtd_cortes = 0;
        barbeiro_info[i].num_cadeiras = num_cadeiras;
        barbeiro_info[i].cadeiras_barbearia = &cadeiras_cliente;
        barbeiro_info[i].barbeiro = barbeiro;
        barbeiro_info[i].exclusao_mutua = &exclusao_mutua;
        barbeiro_info[i].corte_minimo = num_vezes;

        pthread_create(&barbeiro_info[i].thread_id,NULL,thread_barbeiro,&(barbeiro_info[i]));

    }
    printf("iniciando infinitos clientes \n");
    /*iniciando infinitos clientes*/
    while(1){
        cliente = (thread_cliente_info*)malloc(sizeof(thread_cliente_info));
        cliente->id = j;
        j++;
        cliente->cadeiras_barbearia = &cadeiras_cliente;
        cliente->barbeiro = barbeiro;
        cliente->exclusao_mutua = &exclusao_mutua;
        pthread_create(&cliente[0].thread_id,NULL,thread_cliente,&(cliente[0]));
        Sleep(3000);

    }





    return 0;
}