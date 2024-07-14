#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/utils.h"

// ./client execute 100     -u      "prog-a arg-1 (...) arg-n"
// argv[0]  argv[1] argv[2] argv[3] argv[4]
// argc = 5


int main(int argc, char *argv[]) {
    char *control = argv[1];

    if (strcmp(control, "execute") != 0 && strcmp(control, "status") != 0) {  // se nao for execute nem status
        perror("Comando inexistente\n");
        perror("Uso: ./client execute 100 -u \"prog-a arg-1 (...) arg-n\"\n");
        perror("Ou\n");
        perror("Uso: ./client status\n");
        return -1;  // erro

    } else if (strcmp(control, "execute") == 0 && argc != 5) {  // se for execute, tem que ter 5 argumentos
        perror("Argumentos incorretos\n");
        perror("Uso: ./client execute 100 -u \"prog-a arg-1 (...) arg-n\"\n");
        return -1;  // erro

    } else if (strcmp(control, "status") == 0 && argc != 2) {  // se for status, tem que ter 2 argumentos
        perror("Argumentos incorretos\n");
        perror("Uso: ./client status\n");
        return -1;  // erro
    }

    int fd = open("../tmp/input_pipe", O_WRONLY, 0666);
    if (fd == -1) {
        perror("Canal inexistente ou não conectou ao canal");
        return -1;  // erro
    }

    // // como só quero escrever para o canal, fecha-se o descritor de leitura do pipe
    // close(0);

    PEDIDO *p;
    if (strcmp(control, "status") == 0) {
        p = cria_status();
    } else {
        // cria um execute
        int tempoprevexec = atoi(argv[2]);
        char *flag = argv[3];
        char *argumentos = argv[4];

        if(strcmp(flag, "-u") == 0) p = cria_execute(tempoprevexec, flag, argumentos);
        else if (strcmp(flag, "-p") == 0) p = cria_execute(tempoprevexec, flag, argumentos);
        else{
            perror("Flag incorreta\n");
            perror("Uso: ./client execute 100 -u \"prog-a arg-1 (...) arg-n\"\n");
            perror("Ou\n");
            perror("Uso: ./client execute 100 -p \"prog-a arg-1 ... | prog-b arg-1 ... | ... | prog-n arg-1 ...\"\n");
            return -1;  // erro
        }


    }

    

    int bytes_written = write(fd, p, sizeof(PEDIDO));
    close(fd);
    if (bytes_written == -1) {
        perror("Erro ao escrever no fifo");
        return -1;  // erro
    }


    if (strcmp(control, "status") == 0) { // le o resultado do status do status_output e escreve no stdout

        int fd3 = open("../tmp/status_output", O_RDONLY, 0666);
        if (fd3 == -1) {
            perror("Canal inexistente ou não conectou ao canal");
            return -1;  // erro
        }

        char buffer[MAX_STATUS_BUFFER_SIZE];
        read(fd3, buffer, MAX_STATUS_BUFFER_SIZE);
        write(STDOUT_FILENO, buffer, strlen(buffer));
        close(fd3);

        

    } else { // execute: le o id da tarefa do output_pipe e escreve no stdout
        int fd2 = open("../tmp/output_pipe", O_RDONLY, 0666);
        if (fd2 == -1) {
            perror("Canal inexistente ou não conectou ao canal");
            return -1;  // erro
        }
        char buffer[100 + sizeof(int)];
        int id;
        read(fd2, &id, sizeof(int)); // lê o id da tarefa
        sprintf(buffer, "ID DA TAREFA: %d\n", id);
        write(STDOUT_FILENO, buffer, strlen(buffer));
        close(fd2);
    }


    close(fd);
    return 0;
}