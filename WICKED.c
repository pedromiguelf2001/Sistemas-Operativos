#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFSIZE 2048

//guarda as transformações
typedef struct lligada {
    char *transformacao;
    char *nome_executavel;
    char *extensao;
    int id;
    struct lligada *prox;
} *Transformacao;


ssize_t readln(int fd, char *line, size_t size) {
	ssize_t res = 0;
	ssize_t i = 0;

	while ((res = read(fd, &line[i], size)) != 0 && ((char) line[i] != '\n'))
		i += res;

	return i;
}






Transformacao readConfig(char *config, char *origem){
    int fd;
    char line[BUFFSIZE];
    char *delimiter;
    if((fd = open(config, O_RDONLY)) < 0){
        perror("Error opening file!!");
        _exit(-1);
    }

    Transformacao curr, prev, temp;
    curr = prev = temp = NULL;

    // prev = malloc(sizeof(struct lligada));
    // prev->prox = temp;
    // strcpy(prev->extensao,"txt");

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
        temp->extensao = malloc(BUFFSIZE);
        temp->transformacao = malloc(sizeof(line) * (strlen(delimiter)+1));
        strcpy(temp->transformacao, delimiter);

        //guarda o id da transformação
        delimiter = strtok(NULL, " ");
        temp->id = atoi(delimiter);

        //printf("%d",temp->id);

        temp->prox = NULL;

        prev = temp;
        }
        return curr;
}

void teste(Transformacao t,char *origem){
    Transformacao temp = t;
    Transformacao prev = NULL;
    printf("%s\n",origem);

    while(temp){

        if(prev == NULL){
            prev = malloc(sizeof(struct lligada));
            prev->extensao = malloc(BUFFSIZE);
            char *formato;
            formato = malloc(BUFFSIZE);
            formato = strtok(origem,".");
            formato = strtok(NULL,".");
            printf("%s\n",formato);
            strcpy(prev->extensao,formato);
        }

        if (!strcmp(temp->transformacao,"bcompress")){
            strcpy(temp->extensao,"bzip"); 
        }
        else if (!strcmp(temp->transformacao,"bdecompress")){
            strcpy(temp->extensao,prev->extensao); 
        }

        else if (!strcmp(temp->transformacao,"gcompress")){
            strcpy(temp->extensao,"gzip"); 
        }
        else if (!strcmp(temp->transformacao,"gdecompress")){
            strcpy(temp->extensao,prev->extensao); 
        }

        else if (!strcmp(temp->transformacao,"encrypt")){
            strcpy(temp->extensao,"cpt"); 
        }
        else if (!strcmp(temp->transformacao,"decrypt")){
            strcpy(temp->extensao,prev->extensao); 
        }

        else if (!strcmp(temp->transformacao,"nop")){
            strcpy(temp->extensao,prev->extensao); 
        }
        printf("%s\n",temp->extensao);
        prev = temp;
        temp = temp->prox;
        
    }
}


int main(int argc, char* argv[]) {
	int src;
    int dst;
	ssize_t n_read;
	char line[BUFFSIZE];
	
	if(argc != 3){
        perror("Invalid number of arguments!!");
        _exit(-1);
    }
	
	src = open(argv[1], O_RDONLY);
	if(src == -1){
        perror("erro na abertura do ficheiro de input");
        _exit(-1);
    } 

	Transformacao trans = readConfig(argv[1],argv[2]);
    
    teste(trans,argv[2]);

	return EXIT_SUCCESS;
}

