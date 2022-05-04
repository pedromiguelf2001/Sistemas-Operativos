#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <wait.h>
#include "structs.h"

#define BUFFSIZE 1024

typedef struct lligada {
    char *transformacao;
    int max;
    int atual;
    struct lligada *prox;
} *Conf;

int server_pid;

ssize_t readln(int fd, char *line, size_t size) {
	ssize_t res = 0;
	ssize_t i = 0;

	while ((res = read(fd, &line[i], size)) != 0 && ((char) line[i] != '\n'))
		i += res;

	return i;
}

void reply(char* msg, int client, int end_flag){
    Reply reply;
    // Carregar informação
    strcpy(reply.argv[0], msg);
    reply.argc = 1;
    reply.end_flag = end_flag;
    int s2c_fifo;
    char path[256];
    sprintf(path, "tmp/%d",client);
    if((s2c_fifo = open(path, O_WRONLY)) == -1){
        perror("Error replying to client");
        _exit(0);
    }
    write(s2c_fifo,&reply,sizeof(Reply));
    close(s2c_fifo);
    if (end_flag) unlink(path);
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


int possivel(int n, char *trans[], Conf config){
    int count;
    Conf temp = config;
    for(int i = 0; i < n; i++){
        count = 0;
        for(int j = i; j < n; j++){
            if(!strcmp(trans[i],trans[j])){
                count++;
                }   
            }
        temp = config; 
        while(temp){
        if(!strcmp(trans[i],temp->transformacao)){
            if(count > temp->max) return 0;
            }
            temp = temp->prox;
        }
        
    }
    return 1;
}

int possivel_atual(int n, char * trans[], Conf config){
    printf("%d\n",n);
    int count;
    Conf temp = config;
    for(int i = 0; i < n; i++){
        count = 0;
        for(int j = i; j < n; j++){
            if(!strcmp(trans[i],trans[j])){
                count++;
                }   
            }
        temp = config; 
        while(temp){
        if(!strcmp(trans[i],temp->transformacao)){
            printf("%s: count:%d | %d\n",trans[i],count,temp->atual);
            if(count > temp->atual) return 0;
            }
            temp = temp->prox;
        }
        
    }
    return 1;
}









int pipe_Line(int argc, char **files,char **trans, Conf config, int pid){
    int comandos = argc - 4;
    Conf temp = config;
    int n_pipes = comandos-1;
    int p[n_pipes][2];
    char* barra[comandos];
    for(int j = 0; j < comandos; j++){
       char* t = malloc(BUFFSIZE);
       strcat(t,"/");
       strcat(t,trans[j]);
       barra[j] = t;
    }
    int waiting_room = open("tmp/waiting", O_WRONLY,0666);
    for(int i = 0; i < comandos; i++){
       char* path = malloc(BUFFSIZE);
       strcat(path,"Functions/");
       strcat(path,trans[i]);
       int ori;
       int dest;
       pid_t pid;
       int status;
       int save = dup(STDOUT_FILENO);
       ori = open(files[0], O_RDONLY, 0600);
       dest = open(files[1], O_CREAT | O_TRUNC | O_WRONLY, 0600);
       if (!strcmp(trans[0], "nop") && comandos == 3){
           printf("he sleepin good\n");
           sleep(10);
       }
        if(i == 0 && comandos <= 1){
            if(fork() == 0){
               dup2(dest,1);
               dup2(ori,0);
               char msg[256];
               sprintf(msg, "Processing: %s\n", trans[i]);
               reply(msg,pid, 0);
               int ret = execlp(path,barra[i],files[0],files[1],NULL);
               perror("error executing command");
               _exit(ret);
            }
            else{
                wait(NULL);
                while (temp){
                    
                    if(!strcmp(trans[i], temp->transformacao)){
                        temp->atual = temp->atual + 1;
                        
                        break;
                    }
                    temp = temp->prox;
                }
                write(waiting_room, &config,sizeof(Conf) );
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
               close(p[i][1]);
               int ret = execlp(path,barra[i],files[0],files[1],NULL);
               perror("error executing command");
               _exit(ret);
           }
           else{
                wait(NULL);
                close(p[i][1]);
                
                temp = config;
                while (temp){
                   if(!strcmp(trans[i], temp->transformacao)){
                        temp->atual = temp->atual + 1;
                        break;
                   }
                   temp = temp->prox;
                }
                    Conf ola = config;
           }
       }
        else if(i == (comandos-1)){
            if(fork() == 0){
               dup2(p[i-1][0],0);
               dup2(dest,1);
               close(p[i-1][0]);
               int ret = execlp(path,barra[i],files[0],files[1],NULL);
               perror("error executing command");
               _exit(ret);
            }
            else{
                close(p[i-1][0]);
                temp = config;
                while (temp){
                   if(!strcmp(trans[i], temp->transformacao)){
                        temp->atual = temp->atual + 1;
                        break;
                   }
                   temp = temp->prox;
                }
                write(waiting_room, &config,sizeof(Conf) );
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
               int ret = execlp(path,barra[i],files[0],files[1],NULL);
               perror("error executing command");
               _exit(ret);
           }
           else{
                close(p[i-1][0]);
                close(p[i][1]);
                temp = config;
                while (temp){
                   if(!strcmp(trans[i], temp->transformacao)){
                        temp->atual = temp->atual + 1;
                        break;
                   }
                   temp = temp->prox;
                }
                write(waiting_room, &config,sizeof(Conf) );
           }
       }
    }
    printf("Complete %s\n",trans[0]);
    return 0;
}
int atualiza_Struct(int n, char *trans[],char **files, Conf config, int pid){
    printf("ene: %d\n",n);
    Conf temp = config;
    int flag;
    for(int i = 0; i < n; i++){
        printf("i:%d , %s\n",i,trans[i]);
        while(temp){
            if(!strcmp(temp->transformacao,trans[i])){
                if(temp->atual > 0){
                    temp->atual = temp->atual - 1;
                    printf("Temp:%s\n",temp->transformacao);
                    break;
                }
                else{
                    return 0;
                }
            }
            temp = temp->prox;
        }
        temp = config;
    }
    Conf ola = config;
    printf("-----------------Atual-----------------------\n");

        while(ola){
            printf("%s | %d | %d\n",ola->transformacao,ola->max,ola->atual);
            
            ola = ola->prox;
            fflush(stdout);   
        }
  
    
    Conf atual;
    Conf aux;
    aux = atual = config;
    int waiting_room = open("tmp/waiting", O_RDONLY | O_NONBLOCK, 0666);  
    if(possivel_atual(n-4, trans,config)){
        printf("N:%d\n",n-4);
        pipe_Line(n,files,trans,config,pid);
    }

    
    while(read(waiting_room,&atual,sizeof(Conf)) > 0){
        while(atual){
            config->atual = atual->atual;
            config = config->prox;
            atual = atual->prox;
        }
        config = atual = aux;
        
        ola = config;
        printf("----------------------------------------\n");

        while(ola){
            printf("%s | %d | %d\n",ola->transformacao,ola->max,ola->atual);
            
            ola = ola->prox;
            fflush(stdout);   
        }
    }
    
        
     
    
    return 1;
}


void closer(int signum){
    unlink("tmp/c2s_fifo");
    unlink("tmp/waiting");
    exit(0);
}




int main(int argc, char *argv[]) {
    char *files[2];
    server_pid = getpid();
    printf("%d\n", server_pid);

    
    signal(SIGINT, closer);
    signal(SIGTERM, closer);

    Conf config = readConfig(argv[1]);
    if (mkfifo("tmp/c2s_fifo", 0666) == -1){
        perror("Estourou!\n");
        _exit(-1);
    }
    if ( mkfifo("tmp/waiting", 0666) == -1){
        perror("Read - Waiting Room");
        _exit(-1);
    }
    Process process;
    while(1){
        // Abre o pipe Client to server
        int c2s_fifo = open("tmp/c2s_fifo", O_RDONLY,0666);
        while(read(c2s_fifo,&process, sizeof(Process)) > 0){
            if(process.argc >= 5 && !strcmp(process.argv[1],"proc-file")){
                if(fork()==0){
                    for(int i = 0; i < 2; i++){
                    files[i] = malloc(BUFFSIZE);
                    strcpy(files[i],process.argv[i+2]);
                    }
                    char *transf[process.argc-4];
                    for(int i = 0; i < process.argc-4; i++){
                        transf[i] = malloc(BUFFSIZE);
                        strcpy(transf[i],process.argv[i+4]);
                    }
                    if(possivel(process.argc-4,transf,config)){
                        if(fork()==0){
                            //if(!strcmp(transf[0],"encrypt")) sleep(7); //Isto prova que está a correr de forma concorrente, como os miudos em columbine quando ouviram os tiros
                            
                            atualiza_Struct(process.argc-4, transf,files, config,process.pid);
                            reply(
                                "The files have been processed successfully!\n"
                                , process.pid, 1);
                        }
                    }
                    else{
                        write(1,"Wrong number of transformations",strlen("Wrong number of transformations"));
                        return -1;
                    }
                }
            }
            else if(process.argc == 2 && !strcmp(argv[1],"status")){

            }
        }
        close(c2s_fifo);
    }
    return 0;

}