#ifndef UTILS_H
#define UTILS_H

#define STATUS 0
#define EXECUTE 1
#define MAX_ARG_SIZE 300

#define FCFS 0 // first come first serve (politica de escalonamento)
#define SJF 1 // short job first (politica de escalonamento)

//ESTADO vai assumir 4 valores: None, Scheduled e Executing
#define NONE 0
#define SCHEDULED 1
#define EXECUTING 2


#define MAX_STATUS_BUFFER_SIZE 4000
#define MAX_FILE_BUFFER_SIZE 1000


// #include "uthash.h" // biblioteca externa para manipular hashtable


typedef struct pedido {
    int id; // gerado automaticamente pelo servidor //é a key da hashtable
    int comando; //STATUS ou EXECUTE
    int tempo_execucao;
    char flag[3];
    char argumentos[MAX_ARG_SIZE];
    int ESTADO; //SCHEDULED, EXECUTING ou COMPLETED
    // UT_hash_handle hh; // makes this struct hashable //caso precise de usar hashtable

} PEDIDO;

int getId();

PEDIDO *cria_execute (int tempo_execucao, char *flag, char *argumentos);

PEDIDO *cria_status();

void print_pedido(PEDIDO *p);

char *status();

// array dinamico de pedidos

void inicializa_arrays_de_pedidos();

void write_status_to_buffer(char *buffer);

void adicionar_na_queue(PEDIDO *p, int politica_escalonamento);

// remove o primeiro da queue (scheduled) e adiciona em executing
PEDIDO* proximo_a_executar();

void pedido_executado(PEDIDO *p);

void print_queue();

void print_executing();



// // HASHTABLE

// void adicionar_pedido(PEDIDO *p);

// void remover_pedido(PEDIDO *p);

// PEDIDO *procurar_pedido(int id);

// void print_hashtable();

#endif