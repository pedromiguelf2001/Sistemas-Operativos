
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "structs.h"

char s2c_fifo_name[512];
char * error = "FIFO pipe already exists!";

/*
    Esta função copia os argumentos passados à função main para um processo
    que irá ser enviado pelo pipe client to server
*/

void copy_argv(Process *p, int argc, char** argv){
    for(int i = 0; i < argc;i++){
        strcpy(p->argv[i],argv[i]);
    }
}

/*
    A função recebe um path para um pipe e escreve lá a informação dos 
    processos a serem executados.
*/
void proc_file(int fd, int argc, char **argv){
    Process process;
    int pid = getpid();
    process.pid = pid;
    process.argc = argc;
    copy_argv(&process, argc, argv);
    write(fd, &process, sizeof(Process));
}


/*
    Esta função recebe as respostas do servidor através do pipe server to client 
*/
void reply(int fd, int argc, char ** argv){
    Reply reply;
    char * curr = malloc(1024);
    int s2c_fifo = open(s2c_fifo_name, O_RDONLY); //Abre o pipe em questão com a intenção de o ler
    while(1){
        write(s2c_fifo, &reply, sizeof(Reply));
        while (read(s2c_fifo, &reply, sizeof(Reply))>0){ //Enquanto estiver a ler
            for(int i = 0; i < reply.argc; i++){
                if(reply.argc != 1024){
                    write(1,reply.argv[i],strlen(reply.argv[i]));
                    curr = strcpy(curr,reply.argv[0]);
                
                    curr = strtok(curr," ");
                }
                
                
            
            }
            
            //Compara as 2 strings, se forem diferentes então fecha o pipe
            if (!strcmp("The files have been processed successfully!\n", reply.argv[0])){
                
                close(s2c_fifo);
                unlink(s2c_fifo_name);
                _exit(0);
                
            }

            if(!strcmp(curr,"Current")){
                fflush(stdout);
                unlink(s2c_fifo_name);
                _exit(0);
            }
            
            
          
        }
        
    }
}


// A função fecha o pipe server to client
void closer(int signum){
    unlink(s2c_fifo_name);
    exit(0);
}

int main(int argc, char** argv){
    signal(SIGINT, closer);
    signal(SIGTERM, closer);
    //signal(SIGSEGV,closer);
    signal(SIGQUIT,closer);
    fflush(stdout);

    
    sprintf(s2c_fifo_name, "tmp/%d",(int)getpid());

    // Abrimos o pipe server to client
    if((mkfifo(s2c_fifo_name, 0666)) == -1){
        write(1,error,strlen(error));
        return 1;
    }
    //O programa é executado sem argumentos, aparece uma in 
    if (argc == 1){
        char proc_file_help [1024];
        char status_help [1024];
        int pfh_size = sprintf(proc_file_help, "%s proc-file \"input-file\" \"output-file\" transformation1 transformation2 ...\n",argv[0]);
        int sh_size = sprintf(status_help, "%s status\n", argv[0]); 
        write(1,proc_file_help,pfh_size);
        write(1, status_help, sh_size);
    }


    // Comandos certos mas com argumentos errados
    if(!strcmp(argv[1], "proc-file") && argc <= 4){
        char not_enough_arguments[1024];
        int nea_size = sprintf(not_enough_arguments, "sdstore: invalid command -> try adding some transformations :)\n");
        write(1, not_enough_arguments, nea_size);
    }
    else if (!strcmp(argv[1], "status")&& argc > 2){
        char too_many_arguments[1024];
        int tma_size = sprintf(too_many_arguments, "sdstore: invalid command -> this command doesn't recieve any arguments!\n");
        write(1, too_many_arguments, tma_size);
    }
    //Comandos corretos, logo enviamos para o pip client to server
    else if (
                (!strcmp(argv[1], "proc-file")  && argc >  4)   ||
                (!strcmp(argv[1], "status")     && argc == 2)
            ){
        int c2s_fifo = open("tmp/c2s_fifo", O_WRONLY);
        
        proc_file(c2s_fifo, argc, argv);
        
        reply(c2s_fifo,argc, argv);
        close(c2s_fifo);
    }
    // Comandos inexistentes
    else{
        char undefined_command[1024];
        int uc_size = sprintf(undefined_command, "sdstore:  command %s is undefined!\n",argv[1]);
        write(1,undefined_command,uc_size);
    }


    return 0;
}
