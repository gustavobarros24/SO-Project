#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char **parsearg(char* argcommand){
    
    char **parsedargcommand = malloc(sizeof(char *));
    char *token;
    token = strtok(argcommand, " ");
    if(token == NULL) {
        printf("String vazia");
        return NULL;
    }
    int i = 0;
    while(token){
        parsedargcommand[i] = token;
        i++;
        token = strtok(NULL, " ");
        parsedargcommand = realloc(parsedargcommand, sizeof(char *)*(i+1));
    }
    parsedargcommand[i] = NULL;
    return parsedargcommand;
}


char ***parse_pipes(char* argcommand){
    // TODO implementar para fazer parsing quando há uma pipeline de tarefas. 
    // Idea: separar por '|' e para cada linha do array fazer parsearg do token
    return NULL;
}