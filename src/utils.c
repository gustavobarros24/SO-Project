#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //include para write
#include <fcntl.h>
#include <errno.h>



PEDIDO *cria_execute(int tempo_execucao, char *flag, char *argumentos) {
    PEDIDO *p = malloc(sizeof(PEDIDO));
    p->id = -1;
    p->comando = EXECUTE;
    p->tempo_execucao = tempo_execucao;
    if (flag != NULL) {
        strcpy(p->flag, flag);
        p->flag[2] = '\0';
    } else {
        p->flag[0] = '\0';
    }

    strcpy(p->argumentos, argumentos);
    p->ESTADO = NONE;
    return p;
}

PEDIDO *cria_status() {
    PEDIDO *p = malloc(sizeof(PEDIDO));
    p->id = -1;
    p->comando = STATUS;
    p->tempo_execucao = 0;  // como o tempo de execução é zero, se for usado o algoritmo SJF (shortest job first), o servidor vai dar prioridade aos status
    p->flag[0] = '\0';
    p->argumentos[0] = '\0';
    p->ESTADO = NONE;
    return p;
}

void print_pedido(PEDIDO *p) {
    printf("\n");
    printf("======= PEDIDO =======\n");
    printf("ID: %d\n", p->id);
    if (p->comando == STATUS) {
        printf("Comando: STATUS\n");
    } else if (p->comando == EXECUTE) {
        printf("Comando: EXECUTE\n");
    }
    printf("Tempo de execução: %d\nFlag: %s\nArgumentos: %s\n", p->tempo_execucao, p->flag, p->argumentos);
    if (p->ESTADO == NONE) {
        printf("Estado: None\n");
    } else if (p->ESTADO == SCHEDULED) {
        printf("Estado: Scheduled\n");
    } else if (p->ESTADO == EXECUTING) {
        printf("Estado: Executing\n");
    }
    printf("======================\n");
}



PEDIDO *clone_pedido(PEDIDO *p) {
    PEDIDO *clone = malloc(sizeof(PEDIDO));
    clone->id = p->id;
    clone->comando = p->comando;
    clone->tempo_execucao = p->tempo_execucao;
    strcpy(clone->flag, p->flag);
    strcpy(clone->argumentos, p->argumentos);
    clone->ESTADO = p->ESTADO;
    return clone;
}




// array dinamico de pedidos

// Variável global para armazenar o array de pedidos
typedef struct array_pedidos {
    int max_pedidos;
    int num_pedidos;
    PEDIDO **array;
} ARRAY_PEDIDOS;

struct array_pedidos *queue = NULL; //queue de pedidos agendados
struct array_pedidos *executing = NULL; //pedidos que estao a ser executados


void write_status_to_buffer(char* buffer) {
    int offset = 0;

    // Writing "Executing" section
    offset += sprintf(buffer + offset, "\nExecuting:\n\n");
    for (int i = 0; i < executing->num_pedidos; i++) {
        offset += sprintf(buffer + offset, "%d %s\n", executing->array[i]->id, executing->array[i]->argumentos);
    }

    // Writing "Scheduled" section
    offset += sprintf(buffer + offset, "\nScheduled:\n\n");
    for (int i = 0; i < queue->num_pedidos; i++) {
        offset += sprintf(buffer + offset, "%d %s\n", queue->array[i]->id, queue->array[i]->argumentos);
    }

    // Writing "Executed" section from file
    offset += sprintf(buffer + offset, "\nExecuted:\n\n");
    int fd_tarefas = open("tmp/tarefas.txt", O_RDONLY, 0666);
    if (fd_tarefas == -1) {
        perror("Ficheiro inexistente ou não conectou ao ficheiro");
        printf("errno: %d\n", errno);
        return;  // erro
    }

    // Read the tasks file
    ssize_t bytes_read;
    char buffer_tarefas[MAX_FILE_BUFFER_SIZE];
    while ((bytes_read = read(fd_tarefas, buffer_tarefas, MAX_FILE_BUFFER_SIZE)) > 0) {
        offset += snprintf(buffer + offset, MAX_FILE_BUFFER_SIZE - offset, "%s", buffer_tarefas);
    }
    
    close(fd_tarefas);
}




void inicializa_arrays_de_pedidos() {
    queue = malloc(sizeof(struct array_pedidos));
    queue->max_pedidos = 10;
    queue->num_pedidos = 0;
    queue->array = malloc(sizeof(PEDIDO *) * queue->max_pedidos);

    executing = malloc(sizeof(struct array_pedidos));
    executing->max_pedidos = 10;
    executing->num_pedidos = 0;
    executing->array = malloc(sizeof(PEDIDO *) * executing->max_pedidos);
}

// private (função auxiliar para adicionar em arrays)
void adiciona_em(ARRAY_PEDIDOS *array_pedidos, int pos, PEDIDO *pedido) {
    if (pedido->comando == STATUS) {
        printf("tentativa de adicionar status num dos arrays, ignorada\n");
        return;
    }
    if (array_pedidos == executing) {
        pedido->ESTADO = EXECUTING;
    } else if (array_pedidos == queue) {
        pedido->ESTADO = SCHEDULED;
    }


    PEDIDO *p = clone_pedido(pedido);
    
    if (pos >= array_pedidos->max_pedidos - 2) {
        // redimensiona o array se 
        array_pedidos->max_pedidos *= 2;
        array_pedidos->array = realloc(array_pedidos->array, sizeof(PEDIDO *) * array_pedidos->max_pedidos);

    }
    // shift para a direita dos elementos a partir da pos, incluindo a pos
    for (int i = array_pedidos->num_pedidos; i > 0 && i > pos ; i--) {
        array_pedidos->array[i] = array_pedidos->array[i - 1];
    }
    // adiciona o novo elemento na pos
    array_pedidos->array[pos] = p;
    array_pedidos->num_pedidos++;
}

void adicionar_na_queue(PEDIDO *p, int politica_escalonamento) {
    if (p->comando == STATUS) {
        printf("tentativa de adicionar status na queue, ignorada\n");
        return;
    }
    if (politica_escalonamento == FCFS) {
        // adiciona no fim da lista
        if (queue->num_pedidos == 0) {
            adiciona_em(queue, 0, p);
            return;
        } else {
            adiciona_em(queue, queue->num_pedidos, p);
            return;
        }
    } else if (politica_escalonamento == SJF) {
        // adiciona tendo em conta o tempo de execução
        if (queue->num_pedidos == 0) {
            adiciona_em(queue, 0, p);
            return;
        }
        for (int i = 0; i < queue->num_pedidos; i++) {
            // se o tempo de execução do novo pedido for menor que o tempo de execução do pedido na posicao i
            if (p->tempo_execucao < queue->array[i]->tempo_execucao) {
                adiciona_em(queue, i, p);
                return;
            }

            // se i for a ultima posicao, adiciona no fim
            if (i == queue->num_pedidos - 1) {
                adiciona_em(queue, i + 1, p);
                return;
            }
        }
    }
}


void remove_pedido_de(ARRAY_PEDIDOS *array_pedidos, PEDIDO *p) {
    for (int i = 0; i < array_pedidos->num_pedidos; i++) {
        if (array_pedidos->array[i]->id == p->id) {
            free(array_pedidos->array[i]);
            // shift para a esquerda dos elementos a partir da posicao i, removendo a posicao i
            for (int j = i; j < array_pedidos->num_pedidos - 1; j++) {
                array_pedidos->array[j] = array_pedidos->array[j + 1];
            }
            array_pedidos->num_pedidos--;
            break;
        }
    }
}


PEDIDO* proximo_a_executar() { // remove o primeiro da queue (scheduled) e adiciona em executing

    if(queue->num_pedidos == 0){
        return NULL;
    }
    PEDIDO* primeiro = clone_pedido(queue->array[0]);
    // remove o primeiro da queue
    remove_pedido_de(queue, primeiro);
    // adiciona no fim da queue de pedidos em execução
    adiciona_em(executing, executing->num_pedidos, primeiro);

    return primeiro;
}

void pedido_executado(PEDIDO *p) {
    // remove o pedido executado da EXECUTING
    remove_pedido_de(executing, p);


    //TODO adiciona o pedido executado no ficheiro de pedidos executados

}






void print_array(ARRAY_PEDIDOS *a) {
    for (int i = 0; i < a->num_pedidos; i++) {
        print_pedido(a->array[i]);
    }
}

int num_do_print_queue = 1; //apenas para debug

void print_queue() {
    printf("-----------QUEUE: %d-----------\n", num_do_print_queue);
    num_do_print_queue++;
    print_array(queue);
    printf("-------------------------------\n");
}

void print_executing() {
    printf("-----------EXECUTING-----------\n");
    print_array(executing);
    printf("-------------------------------\n");

}




    // // HASHTABLE (EM PRINCIPIO NAO VAI SER USADA)

    // // Variável global para armazenar a tabela de hash
    // PEDIDO *pedidos = NULL;

    // // Função para adicionar um pedido à tabela de hash
    // void adicionar_pedido(PEDIDO * p) {
    //     HASH_ADD_INT(pedidos, id, p);
    // }

    // // Função para remover um pedido da tabela de hash
    // void remover_pedido(PEDIDO * p) {
    //     HASH_DEL(pedidos, p);
    // }

    // // Função para buscar um pedido na tabela de hash
    // PEDIDO *procurar_pedido(int id) {
    //     PEDIDO *p = NULL;
    //     HASH_FIND_INT(pedidos, &id, p);
    //     return p;
    // }

    // // Função para imprimir todos os pedidos na tabela de hash
    // void print_hashtable() {
    //     PEDIDO *p, *tmp;
    //     printf("-----------HASTABLE-----------");
    //     HASH_ITER(hh, pedidos, p, tmp) {
    //         print_pedido(p);
    //     }
    //     printf("------------------------------");
    // }