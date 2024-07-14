#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/utils.h"
#include "parser.h"

int id = 0;  // variavel global para ter um id unico para cada tarefa

int getId() {
    return id++;
}

long timestamp(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
}

int executarpedido(char **argumentos, int id, char *output_folder) {
    printf("EXECUTAR PEDIDO\n");
    printf("ID: %d\n", id);
    printf("PROGRAMA: %s %s\n", argumentos[0], argumentos[1]);

    struct timeval start, end;
    long elapsed_time;

    gettimeofday(&start, NULL);
    pid_t childpid = fork();

    if (childpid == -1) {
        perror("fork");
        return -1;
    } else if (childpid == 0) {
        // Child Process
        char filename[MAX_ARG_SIZE];  // Ensure enough space for file name and null terminator
        sprintf(filename, "%s/%d.txt", output_folder, id);

        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDOUT_FILENO) == -1) { // Redirect stdout to the file
            perror("dup2 stdout");
            exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDERR_FILENO) == -1) { // Redirect stderr to the file
            perror("dup2 stderr");
            exit(EXIT_FAILURE);
        }

        close(fd);  // Close the file descriptor after duplication
    
        if (execvp(argumentos[0], argumentos) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        // Nao redireciona o stdout e stderr de volta para o original porque o child termina aqui

    } else {
        // Parent Process
        int status;
        if (waitpid(childpid, &status, 0) == -1) {
            perror("waitpid");
            return -1;
        }
        if (WIFEXITED(status)) {
            gettimeofday(&end, NULL);
            elapsed_time = timestamp(start, end);
            printf("Elapsed time: %ld microseconds\n", elapsed_time);
            int fd = open("tmp/tarefas.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
            if (fd == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            char buff[MAX_FILE_BUFFER_SIZE];
            ssize_t buffsize = sprintf(buff,"Id = %d / Programa = %s %s / TempoExec = %lu\n", id, argumentos[0], argumentos[1], elapsed_time);
            ssize_t byteswritten = write(fd, buff, buffsize);
            if(byteswritten == -1){
                perror("write");
                close(fd);
                exit(EXIT_FAILURE);
            }
            close(fd);
        } else {
            printf("Child process terminated abnormally.\n");
            return -1;
        }
    }
    return 0;
}

int count_commands(char *pipeline) {
    int num_commands = 0;
    char *copy = strdup(pipeline);
    char *token = strtok(copy, "|");
    while (token != NULL) {
        num_commands++;
        token = strtok(NULL, "|");
    }
    free(copy);
    return num_commands;
}


int execute_pipeline(char *pipeline, int id, char *output_folder) {
    printf("PIPELINE: %s\n", pipeline);

    char *copy = strdup(pipeline);

    int num_commands = count_commands(pipeline);

    char **commands = malloc(sizeof(char *) * num_commands);  // Allocate memory for the array of commands

    char *token = strtok(pipeline, "|");

    for (int i = 0; i < num_commands; i++) {
        commands[i] = strdup(token);  // Duplicate the token to store in the commands array
        token = strtok(NULL, "|");
        printf("COMMAND %d: %s\n", i, commands[i]);
    }

    int prev_fd[2], next_fd[2];  // File descriptors for communication between processes
    prev_fd[0] = -1;             // Initialize to invalid file descriptors
    prev_fd[1] = -1;
    
    for (int i = 0; i < num_commands; i++) {
        pipe(next_fd);  // Create a pipe for communication with the next command

        struct timeval start, end;
        long elapsed_time;

        gettimeofday(&start, NULL);
        pid_t childpid = fork();

        if (childpid == -1) {
            perror("fork");
            return -1;
        } else if (childpid == 0) {
            // Child Process
            if (prev_fd[0] != -1) {  // If not the first command, read from previous command's pipe
                close(STDIN_FILENO);
                dup2(prev_fd[0], STDIN_FILENO);
                close(prev_fd[0]);  // Close the read end of the previous pipe
                close(prev_fd[1]);  // Close the write end of the previous pipe
            }

            if (i < num_commands - 1) {  // If not the last command, write to the next command's pipe
                close(STDOUT_FILENO);
                dup2(next_fd[1], STDOUT_FILENO);
                close(next_fd[0]);  // Close the read end of the current pipe
                close(next_fd[1]);  // Close the write end of the current pipe
            }


            if (i == num_commands - 1) {  // If the last command, redirect output to a file
                char filename[MAX_ARG_SIZE];  // Ensure enough space for file name and null terminator
                sprintf(filename, "%s/%d.txt", output_folder, id);

                int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                close(STDOUT_FILENO);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            char **args = parsearg(commands[i]);
            if (execvp(args[0], args) == -1) {
                perror("execvp");  // This will be reached only if execvp fails
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent Process
            if (prev_fd[0] != -1) {  // If not the first command, close the previous pipe
                close(prev_fd[0]);
                close(prev_fd[1]);
            }
            prev_fd[0] = next_fd[0];  // Set the current pipe as the previous pipe for the next iteration
            prev_fd[1] = next_fd[1];

            if (i == num_commands - 1) {  // If the last command, wait for it to finish
                int status;
                if (waitpid(childpid, &status, 0) == -1) {
                    perror("waitpid");
                    return -1;
                }

                if (WIFEXITED(status)) {
                    gettimeofday(&end, NULL);
                    elapsed_time = timestamp(start, end);
                    printf("Elapsed time: %ld microseconds\n", elapsed_time);

                    int fd = open("tmp/tarefas.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
                    if (fd == -1) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    char buff[MAX_FILE_BUFFER_SIZE];
                    ssize_t buffsize = sprintf(buff,"Id = %d / Programa = %s / TempoExec = %lu\n", id,copy, elapsed_time);
                    ssize_t byteswritten = write(fd, buff, buffsize);
                    if(byteswritten == -1){
                        perror("write");
                        close(fd);
                        exit(EXIT_FAILURE);
                    }
                    close(fd);

                } else {
                    printf("Child process terminated abnormally.\n");
                    return -1;
                }
            }
        }
    }

    // Free memory allocated for commands
    for (int i = 0; i < num_commands; i++) {
        free(commands[i]);
    }
    free(commands);

    return 0;
}

// ./orchestrator output_folder parallel-tasks sched-policy
// Argumentos:
// 1. output-folder: pasta onde são guardados os ficheiros com o output de tarefas executadas.
// 2. parallel-tasks: número de tarefas que podem ser executadas em paralelo.
// 3. sched-policy: identificador da política de escalonamento, caso o servidor suporte várias políticas.

// ./orchestrator output_folder 1 FCFS

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Argumentos insuficientes");
        printf("Uso: ./orchestrator output_folder number-of-parallel-tasks sched-policy");
        return -1;  // erro
    }

    char *output_folder = argv[1];
    int parallel_tasks = atoi(argv[2]);
    int politica_escalonamento = SJF;  // é smallest job first por default

    // nao é case sensitive
    if (strcasecmp(argv[3], "FCFS") == 0) {
        politica_escalonamento = FCFS;
        printf("Servidor iniciado em FCFS\n");
    } else if (strcasecmp(argv[3], "SJF") == 0) {
        politica_escalonamento = SJF;
        printf("Servidor iniciado em SJF\n");
    } else {
        printf("Servidor iniciado em %s por default\n", politica_escalonamento == SJF ? "SJF" : "FCFS");
    }

    int input_pipe = mkfifo("tmp/input_pipe", 0666);
    int output_pipe = mkfifo("tmp/output_pipe", 0666);
    int status_output = mkfifo("tmp/status_output", 0666);
    if (input_pipe == -1 || output_pipe == -1 || status_output == -1) {
        perror("Canal não aberto");
        return -1;  // erro
    }

    inicializa_arrays_de_pedidos();

    int fd;
    int fd2;
    int fd3;

    

        while (1) {
        // leitura de pedidos
        // lê do input_pipe, adiciona à queue e devolve o id no output_pipe se for execute. Se for status, faz um fork

            fd = open("tmp/input_pipe", O_RDONLY, 0666);
            if (fd == -1) {
                perror("Canal inexistente ou não conectou ao input_pipe");
                return -1;  // erro
            }
            PEDIDO *p = malloc(sizeof(PEDIDO));
            int bytes_read = read(fd, p, sizeof(PEDIDO));
            if (bytes_read == -1) {
                perror("Erro ao ler do fifo");
                close(fd);
                return -1;  // erro
            } else if (bytes_read == 0) {
                // Se read retornou 0, significa que não há mais dados para ler
                printf("Canal vazio\n");
                close(fd);
                continue;
            }

            close(fd);
            printf("PEDIDO LIDO\n\n");

            if (p->comando == STATUS) {
                printf("STATUS\n");

                // fork para status nao impedir a leitura de novos pedidos

                int pid2 = fork();
                if (pid2 == -1) {
                    perror("Erro no fork");
                    return -1;  // erro
                } else if (pid2 == 0) {
                    // processo filho executa o status e imprime no status_output

                    int fd3 = open("tmp/status_output", O_WRONLY, 0666);
                    if (fd3 == -1) {
                        perror("Canal inexistente ou não conectou ao status_output");
                        return -1;  // erro
                    }

                    char buffer[MAX_STATUS_BUFFER_SIZE];

                    //da write de todos os pedidos nas queues para o buffer
                    write_status_to_buffer(buffer);

                    // da write do buffer para o status_output
                    int bytes_written = write(fd3, buffer, strlen(buffer));

                    close(fd3);

                } else {  // processo pai antes ignorava o filho e continuava a ler pedidos mas isso causava problemas de concorrencia
                    //signal(SIGCHLD, SIG_IGN);
                    // agora espera que o filho termine
                    int status;
                    waitpid(pid2, &status, 0);
                    continue;  // volta para ler pedidos
                }

            } else if (p->comando == EXECUTE) {
                // é execute
                // mete o pedido na queue
                p->id = getId();  // dá update ao id
                // adicionar_pedido(p);  // adiciona o pedido à hashtable
                // printf("PEDIDO ADICIONADO À HASHTABLE\n\n");
                adicionar_na_queue(p, politica_escalonamento);
                printf("PEDIDO ADICIONADO A QUEUE\n\n");


                // escreve o id do pedido no output_pipe
                fd2 = open("tmp/output_pipe", O_WRONLY, 0666);
                if (fd2 == -1) {
                    perror("Canal inexistente ou não conectou ao output_pipe");
                    return -1;  // erro
                }

                int bytes_written = write(fd2, &p->id, sizeof(int));
                close(fd2);
                if (bytes_written == -1) {
                    perror("Erro ao escrever no fifo output_pipe");
                    return -1;  // erro
                }
            } else {
                if (p->comando == NONE) {
                    printf("Comando NONE\n");
                    print_pedido(p);
                    printf("\n");
                }
                printf("Erro: Comando invalido\n");
                return -1;  // erro
            }





            // execução de pedidos da queue
          
            PEDIDO *proximo = proximo_a_executar();
            if (proximo != NULL) { // se p não é NULL, tem um pedido a executar (se for NULL a queue tá vazia entao continua o loop)

                // execute

                if (strcmp(proximo->flag, "-u") == 0) {
                    char **programa_e_argumentos = parsearg(proximo->argumentos);
                    executarpedido(programa_e_argumentos, proximo->id, output_folder);
                    free(programa_e_argumentos);
                    pedido_executado(proximo);
                } else if (strcmp(proximo->flag, "-p") == 0) {
                    execute_pipeline(proximo->argumentos, proximo->id, output_folder);
                    pedido_executado(proximo);
                } else {
                    printf("Flag incorreta\n");
                    printf("Uso: ./client execute 100 -u \"prog-a arg-1 (...) arg-n\"\n");
                    printf("Ou\n");
                    printf("Uso: ./client execute 100 -p \"prog-a arg-1 ... | prog-b arg-1 ... | ... | prog-n arg-1 ...\"\n");
                    return -1;  // erro
                }
                
            }

            free(p);

        }


        

    int fifo = unlink("tmp/input_pipe");
    int fifo2 = unlink("tmp/output_pipe");
    if (fifo == -1) {
        perror("Canal não foi fechado");
        return -1;  // erro
    }
    if (fifo2 == -1) {
        perror("Canal não foi fechado");
        return -1;  // erro
    }
    return 0;
}
