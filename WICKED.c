#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFSIZE 2048;

//guarda as transformações
typedef struct lligada {
    char *transformacao;
    char *nome_executavel;
    int id;
    struct lligada *prox;
} *Transformacao;

Transformacao readConfig(char *config){
    int fd;
    char line[BUFFSIZE];
    char *delimiter;
    if((fd = open(config, O_RDONLY)) < 0){
        perror("Error opening file!!");
        _exit(-1);
    }

    Transformacao curr, prev, temp;
    curr = prev = temp = NULL;
    // 1 é o tamanho do buffer, aqui lemos caracter a caracter
    while(readln(fd, line, 1) > 0){
        temp = malloc(sizeof(struct lligada));
        if (curr != NULL){
            prev->prox = temp;
        }
        else{
            curr = temp;
        }
        //strtok divide a linha no caracter " "
        //guarda o nome da tranformação
        delimiter = strtok(line," ");
        temp->transformacao = malloc(sizeof(line) * (strlen(delimiter)+1));
        strcpy(temp->transformacao, delimiter);

        //guarda o id da transformação
        delimiter = strtok(NULL, " ");
        temp->id = atoi(delimiter);
    }
    }


}