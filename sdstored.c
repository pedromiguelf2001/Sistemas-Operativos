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

typedef struct pedido{
    int tamanho;
    char *transf[20];
    pid_t pid;
    struct pedido *prox;
} *Pedido;


typedef struct processos {
    Process p;
    struct processos *prox;
} *Processos;

int server_pid;
Conf global;
Pedido pedidos = NULL;
Processos queue = NULL;

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
            if(count > temp->atual) return 0;
            }
            temp = temp->prox;
        }
        
    }
    return 1;
}









int pipe_Line(int argc, char **files,char *trans[], Conf config, int pid){
    int comandos = argc - 4;
    printf("Numero de Comandos %d\n",comandos);
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
    if(!strcmp("nop", trans[0]) && comandos == 3){
        printf(("Juro que nao meti nada na bebida\n"));
        sleep(10);
    }
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
        if(i == 0 && comandos <= 1){
            if(fork() == 0){
                dup2(dest,1);
                dup2(ori,0);
                int ret = execlp(path,barra[i],files[0],files[1],NULL);
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
               close(p[i][1]);
               int ret = execlp(path,barra[i],files[0],files[1],NULL);
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
               int ret = execlp(path,barra[i],files[0],files[1],NULL);
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
                int ret = execlp(path,barra[i],files[0],files[1],NULL);
                perror("error executing command");
                _exit(ret);
            }
            else{
                close(p[i-1][0]);
                close(p[i][1]);
            }
       }
    }
    printf("Complete %s, com %d comandos\n",trans[0],comandos);
    return 0;
}

int atualiza_Struct(int n, char *trans[],char **files, Conf config, int pid){
    Conf temp = config;    
    for(int i = 0; i < n; i++){
        while(temp){
            if(!strcmp(temp->transformacao,trans[i])){
                if(temp->atual > 0){
                    temp->atual = temp->atual - 1;
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
    return 1;
}


void closer(int signum){
    unlink("tmp/c2s_fifo");
    unlink("tmp/waiting");
    exit(0);
}


void send_status(Conf config,int pid){
    char * msg = malloc(2048);
    char * line = malloc(64);
    
    strcat(msg, "Current server configuration: (name | maximum processes | available processes)\n");
    Conf temp = config;
    while (temp){
        sprintf(line,"%s | %d | %d\n",temp->transformacao,temp->max,temp->atual);
        strcat(msg,line);
            
        temp = temp->prox;
        
    }
    strcat(msg,"Warning: This is a snapshot of a specific time frame, it may not reflect the present status\n");
    reply(msg, pid, 0);

}
void enqueue(Process pro){
    if(!queue){
        queue->p = pro;
        queue->prox = NULL;
    }
    else{
        Processos temp = queue;
        while(temp->prox){
            temp = temp->prox;
        }
        temp->prox->p = pro;
        temp->prox->prox = NULL;
    }
}
void dequeue(){
    if(queue){
        int tryagain = open("tmp/c2s_fifo",O_WRONLY);
        write(tryagain,&queue->p,sizeof(Process));
        queue = queue->prox;
    }
}



void handler(int signum){
    Pedido ola = pedidos;
    Conf l = global;
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("Pid handler %d\n",pid);
    fflush(stdout);
    Pedido temp = pedidos;
    Pedido aux = temp;
    int flag = 0;
    while(temp){
        if(temp->pid == pid){
            printf("IGUAL\n");
            flag = 1;
            aux = temp;
        }
        temp = temp->prox;
    }
    if(flag){
    for(int i = 0; i < aux->tamanho; i++){
        while(l){
            if(!strcmp(l->transformacao,aux->transf[i])){
                if(l->atual >= 0){
                    l->atual = l->atual + 1;
                    
                    break;
                }
                else{
                    return;
                }
            }
            l = l->prox;
        }
        l = global;
    }
    Conf o = global;
    printf("-----------------Depois-----------------------\n");

        while(o){
            printf("%s | %d | %d\n",o->transformacao,o->max,o->atual);
            
            o = o->prox;
            fflush(stdout);   
        }
        dequeue();
    }
    
}


void addPedido(int n,char *t[], pid_t pide){
    Pedido novo;
    novo = malloc(sizeof(struct pedido));
    novo->tamanho = n;
    novo->pid = pide;
    novo->prox = NULL;
    for(int i = 0; i < n; i++){
        novo->transf[i] = malloc(BUFFSIZE);
        strcpy(novo->transf[i],t[i]);
    }
    Pedido temp = pedidos;
    if(!temp){
        pedidos = novo;
    }
    else{
        while(temp->prox){
        temp = temp->prox;
        }
        temp->prox = novo;
    }
}





int main(int argc, char *argv[]) {
    char *files[2];
    int fd[2];
    int status;
    server_pid = getpid();
    printf("%d\n", server_pid);
    signal(SIGCHLD,handler);
    signal(SIGINT, closer);
    signal(SIGTERM, closer);
    //signal(SIGSEGV, closer);
    global = readConfig(argv[1]);
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
            if(process.argc == 2 && !strcmp(process.argv[1],"status")){         
                send_status(global, process.pid);
            }
            if(process.argc >= 5 && !strcmp(process.argv[1],"proc-file")){
                reply("please wait\n", process.pid, 0);
                for(int i = 0; i < 2; i++){
                    files[i] = malloc(BUFFSIZE);
                    strcpy(files[i],process.argv[i+2]);
                }
                char *transf[process.argc-4];
                for(int i = 0; i < process.argc-4; i++){
                    transf[i] = malloc(BUFFSIZE);
                    strcpy(transf[i],process.argv[i+4]);
                }
                pipe(fd);
                
                if(possivel(process.argc-4,transf,global)){
                    // printf("!!!!!%s | %d\n",transf[0],process.argc-4);

                    try:

                    if(possivel_atual(process.argc-4, transf,global)){




                        atualiza_Struct(process.argc-4, transf,files, global,process.pid);
                        if(fork() == 0){
                            pid_t pide = getpid();
                            printf("Pid FORk %d\n",pide);
                            fflush(stdout);
                            char pi[BUFFSIZE];
                            sprintf(pi,"%d",pide);
                            close(fd[0]);
                            write(fd[1],&pi,sizeof(pi));
                            close(fd[1]);
                            pipe_Line(process.argc,files,transf,global,process.pid);
                            reply(
                            "The files have been processed successfully!\n"
                            , process.pid, 1);
                            _exit(0);
                    }

                    close(fd[1]);
                    char buff[BUFFSIZE];
                    int aux = 0;
                    while(read(fd[0],&buff,BUFFSIZE)>0){
                        aux = atoi(buff);
                    }
                    close(fd[0]);
                    addPedido(process.argc-4, transf, aux);

                } 
                else{
                    if(fork()==0){
                        goto try;
                    }
                    
                }

                
            
            }
            else{
                    write(1,"Wrong number of transformations",strlen("Wrong number of transformations"));
                    return -1;
                }
            }
        }
        close(c2s_fifo);
    }
    return 0;
}