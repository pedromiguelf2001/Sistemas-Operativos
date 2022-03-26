#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFSIZE 2048


//guarda as transformações
typedef struct lligada {
    char *transformacao;
    char *nome_executavel;
    char *extensao;
    char *extensao_ant;
    //estado é para as funçoes aux
    int estado;
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
        temp->extensao = malloc(BUFFSIZE);
        temp->transformacao = malloc(sizeof(line) * (strlen(delimiter)+1));
        strcpy(temp->transformacao, delimiter);


        //guarda o id da transformação
        delimiter = strtok(NULL, " ");
        temp->id = atoi(delimiter);


        temp->prox = NULL;

        prev = temp;
        }
        return curr;
}

Transformacao armazenaExtensao(Transformacao t,char *origem){
    Transformacao temp = t;
    Transformacao inicio = t;
    Transformacao prev = NULL;
    Transformacao aux_bcompress = NULL;
    Transformacao aux_gcompress = NULL;
    Transformacao aux_encrypt = NULL;
    char *copia;
    copia = malloc(BUFFSIZE);
    strcpy(copia,origem);
    char *formato;
    formato = malloc(BUFFSIZE);
    formato = strtok(copia,".");
    formato = strtok(NULL,".");
    

    //Transformações aux para as funções bcompress, gcompress, encrypt
    aux_bcompress = malloc(sizeof(struct lligada));
    aux_bcompress->extensao_ant = malloc(BUFFSIZE);
    strcpy(aux_bcompress->extensao_ant,formato);
    aux_bcompress->estado = 0;

    aux_gcompress = malloc(sizeof(struct lligada));
    aux_gcompress->extensao_ant = malloc(BUFFSIZE);
    strcpy(aux_gcompress->extensao_ant,formato);
    aux_gcompress->estado = 0;

    aux_encrypt = malloc(sizeof(struct lligada));
    aux_encrypt->extensao_ant = malloc(BUFFSIZE);
    strcpy(aux_encrypt->extensao_ant,formato);
    aux_encrypt->estado = 0;

    

    if(prev == NULL){
            prev = malloc(sizeof(struct lligada));
            prev->extensao = malloc(BUFFSIZE);
            strcpy(prev->extensao,formato);
        }

    while(temp){

        //Bcompress
        if (!strcmp(temp->transformacao,"bcompress")){
            strcpy(temp->extensao,"bzip");
            aux_bcompress->estado = 1;
            strcpy(aux_bcompress->extensao_ant, prev->extensao);
        }

        else if (!strcmp(temp->transformacao,"bdecompress")){
            if (aux_bcompress->estado == 1){
                strcpy(temp->extensao,aux_bcompress->extensao_ant); 
                aux_bcompress->estado = 0;
            }
            else{
                if (temp->prox == NULL){
                    strcpy(temp->extensao,"txt");
                }
                else{
                    if (!strcmp(temp->prox->transformacao, "bdecompress"))
                        strcpy(temp->extensao,"bzip"); 
                    if (!strcmp(temp->prox->transformacao, "gdecompress"))
                        strcpy(temp->extensao,"gzip"); 
                    if (!strcmp(temp->prox->transformacao, "decrypt"))
                        strcpy(temp->extensao,"cpt");    
                    if (!strcmp(temp->prox->transformacao, "nop"))
                        strcpy(temp->extensao,"txt");      
                    }
            }
        }



        //Gcompress
        else if (!strcmp(temp->transformacao,"gcompress")){
            strcpy(temp->extensao,"gzip");
            aux_gcompress->estado = 1;
            strcpy(aux_gcompress->extensao_ant,prev->extensao); 
        }

        else if (!strcmp(temp->transformacao,"gdecompress")){
            if (aux_gcompress->estado == 1){
                strcpy(temp->extensao,aux_gcompress->extensao_ant); 
                aux_gcompress->estado = 0;
            }
            else{
                if (temp->prox == NULL){
                    strcpy(temp->extensao,"txt");
                }
                else{
                    if (!strcmp(temp->prox->transformacao, "bdecompress"))
                        strcpy(temp->extensao,"bzip"); 
                    if (!strcmp(temp->prox->transformacao, "gdecompress"))
                        strcpy(temp->extensao,"gzip"); 
                    if (!strcmp(temp->prox->transformacao, "decrypt"))
                        strcpy(temp->extensao,"cpt");    
                    if (!strcmp(temp->prox->transformacao, "nop"))
                        strcpy(temp->extensao,"txt");      
                    }
            }
        }



        //Encrypt
        else if (!strcmp(temp->transformacao,"encrypt")){
            strcpy(temp->extensao,"cpt");
            aux_encrypt->estado = 1;
            strcpy(aux_encrypt->extensao_ant,prev->extensao); 
        }

        else if (!strcmp(temp->transformacao,"decrypt")){
            if (aux_encrypt->estado == 1){
                strcpy(temp->extensao,aux_encrypt->extensao_ant); 
                aux_encrypt->estado = 0;
            }
            else{
                if (temp->prox == NULL){
                    strcpy(temp->extensao,"txt");
                }
                else{
                    if (!strcmp(temp->prox->transformacao, "bdecompress"))
                        strcpy(temp->extensao,"bzip"); 
                    if (!strcmp(temp->prox->transformacao, "gdecompress"))
                        strcpy(temp->extensao,"gzip"); 
                    if (!strcmp(temp->prox->transformacao, "decrypt"))
                        strcpy(temp->extensao,"cpt");    
                    if (!strcmp(temp->prox->transformacao, "nop"))
                        strcpy(temp->extensao,"txt");      
                    }
                }
            }
        }



        //Nop
        else if (!strcmp(temp->transformacao,"nop")){
            strcpy(temp->extensao,prev->extensao); 
        }



        prev = temp;
        temp = temp->prox;
        
    }
    return t;
}



void constroiFicheiro(char *nome_ficheiro, char *extensao, char *final){
        strcat(final,nome_ficheiro);
        strcat(final,".");
        strcat(final,extensao);
}







void correrConfs(Transformacao t, char *ficheiro){
    int status;

    Transformacao temp = t;
    Transformacao prev = NULL;
    char *copia;
    copia = malloc(BUFFSIZE);
    strcpy(copia,ficheiro);
    char *nome_ficheiro;
    nome_ficheiro = malloc(BUFFSIZE);
    char *formato;
    formato = malloc(BUFFSIZE);
    nome_ficheiro = strtok(copia,".");
    
    formato = strtok(NULL,".");
    

    prev = malloc(sizeof(struct lligada));
    prev->extensao = malloc(BUFFSIZE);
    strcpy(prev->extensao,formato);
    
    //path = "/home/pedromiguelf/Desktop/SO/Projeto/Trabalho-SO/Functions"
    

    while(temp){
        char *origem;
        origem = malloc(BUFFSIZE);
        constroiFicheiro(nome_ficheiro,prev->extensao,origem);
        //printf("origem: %s\n",origem);
        char *destino;
        destino = malloc(BUFFSIZE);
        constroiFicheiro(nome_ficheiro,temp->extensao,destino);
        //printf("destino: %s\n",destino);
        //printf("-----------------\n");
        int ori;
        int dest;
        pid_t pid;
        
        if (!strcmp(temp->transformacao,"encrypt")){
            ori = open(origem, O_RDONLY, 0600);
            dest = open(destino, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(ori, STDIN_FILENO);
            dup2(dest, STDOUT_FILENO);
            close(ori);
            close(dest);
            pid = fork();
            if (pid == 0){
                int ret = execl("/home/pedrof/Desktop/SO/Projeto/Functions/encrypt","/encrypt",origem, destino,NULL);
                perror("error executing command");
                _exit(ret);
            }	
            else{
                wait(&status); 
            }
        }
        if (!strcmp(temp->transformacao,"bcompress")){
            ori = open(origem, O_RDONLY, 0600);
            dest = open(destino, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(ori, STDIN_FILENO);
            dup2(dest, STDOUT_FILENO);
            close(ori);
            close(dest);
            pid = fork();
            if (pid == 0){
                int ret = execl("/home/pedrof/Desktop/SO/Projeto/Functions/bcompress","/bcompress",origem, destino,NULL);
                perror("error executing command");
                _exit(ret);
            }	
            else{
                wait(&status);
            }
        }
        if (!strcmp(temp->transformacao,"gcompress")){
            ori = open(origem, O_RDONLY, 0600);
            dest = open(destino, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(ori, STDIN_FILENO);
            dup2(dest, STDOUT_FILENO);
            close(ori);
            close(dest);
            pid = fork();
            if (pid == 0){
                int ret = execl("/home/pedrof/Desktop/SO/Projeto/Functions/gcompress","/gcompress",origem, destino,NULL);
                perror("error executing command");
                _exit(ret);
            }	
            else{
                wait(&status);
            }
        }
        if (!strcmp(temp->transformacao,"bdecompress")){
            ori = open(origem, O_RDONLY, 0600);
            dest = open(destino, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(ori, STDIN_FILENO);
            dup2(dest, STDOUT_FILENO);
            close(ori);
            close(dest);
            pid = fork();
            if (pid == 0){
                int ret = execl("/home/pedrof/Desktop/SO/Projeto/Functions/bdecompress","/bdecompress",origem, destino,NULL);
                perror("error executing command");
                _exit(ret);
            }	
            else{
                wait(&status);
            }
        }
        if (!strcmp(temp->transformacao,"decrypt")){
            ori = open(origem, O_RDONLY, 0600);
            dest = open(destino, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(ori, STDIN_FILENO);
            dup2(dest, STDOUT_FILENO);
            close(ori);
            close(dest);
            pid = fork();
            if (pid == 0){
                int ret = execl("/home/pedrof/Desktop/SO/Projeto/Functions/decrypt","/decrypt",origem, destino,NULL);
                perror("error executing command");
                _exit(ret);
            }	
            else{
                wait(&status);
            }
        }
        if (!strcmp(temp->transformacao,"gdecompress")){
            ori = open(origem, O_RDONLY, 0600);
            dest = open(destino, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(ori, STDIN_FILENO);
            dup2(dest, STDOUT_FILENO);
            close(ori);
            close(dest);
            pid = fork();
            if (pid == 0){
                int ret = execl("/home/pedrof/Desktop/SO/Projeto/Functions/gdecompress","/gdecompress",origem, destino,NULL);
                perror("error executing command");
                _exit(ret);
            }	
            else{
                wait(&status);
            }
        }
        // if (!strcmp(temp->transformacao,"nop")){
        //     ori = open(origem, O_RDONLY, 0600);
        //     //dest = open(destino, O_CREAT | O_TRUNC | O_WRONLY, 0600);
        //     dup2(ori, STDIN_FILENO);
        //     //dup2(dest, STDOUT_FILENO);
        //     close(ori);
        //     //close(dest);
        //     pid = fork();
        //     if (pid == 0){
        //         int ret = execl("/home/pedrof/Desktop/SO/Projeto/Functions/nop","/nop",origem,NULL);
        //         perror("error executing command");
        //         _exit(ret);
        //     }	
        //     else{
        //         wait(&status);
        //     }
        // }
        
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
    
    Transformacao t = armazenaExtensao(trans,argv[2]);

    correrConfs(t, argv[2]);

	return EXIT_SUCCESS;
}
