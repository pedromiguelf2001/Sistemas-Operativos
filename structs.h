#ifndef STRUCTS_H
#define STRUCTS_H


typedef struct process{
    int pid;
    int argc;
    char argv[32][32];
}Process;

typedef struct reply{
    
    int argc;
    char argv[256][2048];
    int end_flag;
}Reply;

#endif