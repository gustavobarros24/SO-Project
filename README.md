# <p style="text-align: center;">To Do</p>

# Sistemas-Operativos
Projeto de SO

O cliente é o client.c e o servidor é o orchestrator.c

## Para utilizar fazer:
make
./bin/orchestrator output_folder number-of-parallel-tasks sched-policy

(noutro terminal) 
cd bin
./client execute 100 -u "ls -a"


## Funcionalidades Básicas
 - [x] Cliente deve submeter ao servidor a intenção de executar uma tarefa, dando uma indicação de qual tarefa e da duração em milissegundos que necessitam.
 - [x] Transmitir o identificador da tarefa mal a mesma entre no servidor.
 - [x] Servidor escalona as tarefas
 - [x] Servidor executa as tarefas.
 - [x] A informação das tarefas produzidas do standard output ou error devem ser colocadas (pelo servidor) num ficheiro com o nome do indentificador da tarefa.
 - [x] A informação deve ser também mantida em memória (podemos usar um array).
 - [x] Consultar o estado das tarefas no servidor através do cliente.
 - [x] Guardar as tarefas que já terminaram através de um ficheiro para que possam ser consultadas posteriormente SEM pipelines.
 - [x] Guardar as tarefas que já terminaram através de um ficheiro para que possam ser consultadas posteriormente COM pipelines.
 - [x] Makefile
 - [x] Relatório

## Encadeamento de programas, multi-processamento e avaliação
 - [x] Através da opção execute time -p "prog-a [args] | prog-b [args] | prog-c [args]" o cliente suporta execução em pipelines.
 - [x] Pipeline tem que guardar o std_output e o std_error de cada programa em ficheiros com o nome do identificador da tarefa.
 - [x] O cliente tem que receber o indentificador do pipeline.
 - [ ] Processamento de várias tarefas em paralelo (N).
 - [ ] Desenvolver testes que permitem perceber a eficiência da política de escalonamento adotada (comparar com outras provavelmente).
 - [x] Guardar as tarefas que já terminaram através de um ficheiro para que possam ser consultadas posteriormente.
 - [ ] Testar diferentes configurações de paralelização do servidor.
 
## Relatório
