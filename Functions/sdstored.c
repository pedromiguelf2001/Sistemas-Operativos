#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <wait.h>

#define BUFFSIZE 2048

typedef struct lligada {
    char *transformacao;
    int max;
    int atual;
    struct lligada *prox;
} *Conf;


ssize_t readln(int fd, char *line, size_t size) {
	ssize_t res = 0;
	ssize_t i = 0;

	while ((res = read(fd, &line[i], size)) != 0 && ((char) line[i] != '\n'))
		i += res;

	return i;
}

// Guarda na lista ligada as transformacoes existentes e os seus max's
Conf readConfig(char *config){
    int fd;
    char line[BUFFSIZE];
    char *delimiter;
    if((fd = open(config, O_RDONLY)) < 0){
        perror("Error opening file!!");
        _exit(-1);
    }

    Conf curr, prev, temp;
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
        
        //guarda o nome da tranformação
        delimiter = strtok(line," ");
        temp->transformacao = malloc(sizeof(line) * (strlen(delimiter)+1));
        strcpy(temp->transformacao, delimiter);

        //guarda o max da transformação
        delimiter = strtok(NULL, " ");
        temp->max = atoi(delimiter);
        temp->atual = temp->max;
        temp->prox = NULL;
        printf("%s | %d | %d\n",temp->transformacao, temp->max, temp->atual);
        prev = temp;
        }
        return curr;
}


int possivel(Conf config,char *trans[]){
    
}

int pipe_Line(int argc, char *argv[],char *trans[]){
    int comandos = argc - 4;
    int n_pipes = comandos-1;
    int p[n_pipes][2];
    char* barra[comandos];
    for(int j = 0; j < comandos; j++){
        char* t = malloc(BUFFSIZE);
        strcat(t,"/");
        strcat(t,trans[j]);
        barra[j] = t;
    }
    for(int i = 0; i < comandos; i++){
        char* path = malloc(BUFFSIZE);
        strcat(path,"/home/pedrof/Desktop/SO/Projeto/Functions/");
        strcat(path,trans[i]);
        //printf("%s\n",path);
        //printf("%s\n",barra[i]);
        int ori;
        int dest;
        pid_t pid;
        int status;
        int save = dup(STDOUT_FILENO);
        ori = open(argv[2], O_RDONLY, 0600);
        dest = open(argv[3], O_CREAT | O_TRUNC | O_WRONLY, 0600);
        if(i == 0 && comandos <= 1){
           if(fork() == 0){
                dup2(dest,1);
                dup2(ori,0);
                int ret = execl(path,barra[i],argv[2],argv[3],NULL);
                perror("error executing command");
                _exit(ret);
            } 
        }
        else if(i == 0 && comandos > 1){
            pipe(p[i]);
            if(pipe(p[i]) == -1){
                perror("PIPE:");
                return -1;
            }
            if(fork() == 0){
                close(p[i][0]);
                dup2(p[i][1],1);
                dup2(ori,0);
                //dup2(dest,1);
                close(p[i][1]);
                int ret = execl(path,barra[i],argv[2],argv[3],NULL);
                perror("error executing command");
                _exit(ret);
            }
            else{
                close(p[i][1]);
            }
        }
        else if(i == (comandos-1)){
            if(fork() == 0){
                dup2(p[i-1][0],0);
                dup2(dest,1);
                close(p[i-1][0]);
                int ret = execl(path,barra[i],argv[2],argv[3],NULL);
                perror("error executing command");
                _exit(ret);
            }
            else{
                close(p[i-1][0]);
            }
        }
        else{
            pipe(p[i]);
            if(fork() == 0){
                close(p[i][0]);
                dup2(p[i-1][0],0);
                close(p[i-1][0]);
                dup2(p[i][1],1);
                close(p[i][1]);
                int ret = execl(path,barra[i],argv[1],argv[2],NULL);
                perror("error executing command");
                _exit(ret);
            }
            else{
                close(p[i-1][0]);
                close(p[i][1]);
            }
        }
    }
    return 0;
}






int main(int argc, char *argv[]) {
    Conf config = readConfig(argv[1]);
    if(possivel(trans,config)){
    if(!strcmp(argv[1],"proc-file")){
        if(argc > 4){
            int n = 4;
            int n_pipes = n-1;
            int p[n_pipes][2];
            char* trans[argc-2];
            for(int i = 0; i < argc-4; i++){
                trans[i] = argv[i+4];
                //printf("%s\n",trans[i]);
            }
            pipe_Line(argc,argv,trans);
        }
        else{
            perror("Erro: Argumentos Insuficientes!\n");
            _exit(-1);
            }
        }
    }
    else if(!strcmp(argv[1],"status")){
        // fazer
    }
    return 0;
}