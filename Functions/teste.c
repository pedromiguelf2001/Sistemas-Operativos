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
    char *barra;
    struct lligada *prox;
} *transformacao;

int main(int argc, char *argv[]) {
    int n = 4;
    int n_pipes = n-1;
    int p[n_pipes][2];
    char* trans[] = {"bcompress","encrypt","decrypt","bdecompress"};
    char* barra[n];
    for(int j = 0; j < n; j++){
        char* t = malloc(BUFFSIZE);
        strcat(t,"/");
        strcat(t,trans[j]);
        barra[j] = t;
        //printf("%s\n",barra[j]);
    } 
    for(int i = 0; i < n; i++){
        char* path = malloc(BUFFSIZE);
        strcat(path,"/home/pedrof/Desktop/SO/Projeto/Functions/");
        strcat(path,trans[i]);
        printf("%s\n",path);
        printf("%s\n",barra[i]);
        int ori;
            int dest;
            pid_t pid;
            int status;
            int save = dup(STDOUT_FILENO);
            ori = open(argv[1], O_RDONLY, 0600);
            dest = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, 0600);
        if(i == 0 && n <= 1){
            printf("hey");
           if(fork() == 0){
                dup2(dest,1);
                dup2(ori,0);
                int ret = execl(path,barra[i],argv[1],argv[2],NULL);
                perror("error executing command");
                _exit(ret);
            } 
        }
        else if(i == 0 && n > 1){
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
                int ret = execl(path,barra[i],argv[1],argv[2],NULL);
                perror("error executing command");
                _exit(ret);
            }
            else{
                close(p[i][1]);
            }
        }
        else if(i == (n-1)){
            if(fork() == 0){
                dup2(p[i-1][0],0);
                dup2(dest,1);
                close(p[i-1][0]);
                int ret = execl(path,barra[i],argv[1],argv[2],NULL);
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
                int ret = execl(path,barra[i],NULL);
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