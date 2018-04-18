#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include "interpreter.h"
#include "file_reader.h"
#include <sys/stat.h>
#include "../headers/pipe_args.h"
#include "argument_parser.h"

static const int OP_OK = 0;
static const int OP_ERROR = -1;



/*
* Declarations of functions
*/

int _proceedInterpretationLoop(FILE* fileStream, ProgramInput* input);
void _freeArgs(char** args);
void _printError(int line);
int _count_number_of_pipe_chunks(PipeArgs** args);
int _initialize_descriptors(int* descriptors, int N);
int _invokeProcess(PipeArgs** args, FILE* file, int* fd, int idx, int num_of_chunks);
void _close_unnecesery_pipes(int* fd, int idx, int num_of_chunks);
void _close_all_pipes(int* fd, int num_of_chunks);
void _print_output(int descriptor);
int _handle_pipe(PipeArgs** args, ProgramInput* input, FILE* file);
void _save_output(int descriptor, const char* filename);
void _print_output(int descriptor);
void _close_unnecesery_pipes_in_parent(int* fd, int num_of_chunks);

/*
* Declarations of functions END
*/

int proceedBatchInterpretation(ProgramInput* input){
    FILE* fileStream = fopen(input->batchFilePath, "r");
    if(fileStream == NULL){
        perror("Could not open file.");
        return OP_ERROR;
    }
    int opStatus = _proceedInterpretationLoop(fileStream, input);
    fclose(fileStream);
    return opStatus;
}

int _proceedInterpretationLoop(FILE* fileStream, ProgramInput* input){
    PipeArgs** args;
    int line = 1, opStatus = OP_OK;
    while((args = preprocessLineOfFile(fileStream)) != NULL){  
        opStatus = _handle_pipe(args, input, fileStream);
        freePipeArgs(args);
        if(opStatus == OP_ERROR) break;
        line ++;
    }
    if(opStatus == OP_ERROR) _printError(line);
    return opStatus;
}


int _handle_pipe(PipeArgs** args, ProgramInput* input, FILE* file){
    int N = _count_number_of_pipe_chunks(args);
    int descriptors[2*N];
    if(_initialize_descriptors(descriptors, N) == OP_ERROR){
        _close_all_pipes(descriptors, N);
        return OP_ERROR;
    }
    for(int i = 0; i < N; i++){
       if(_invokeProcess(args, file, descriptors, i, N) == OP_ERROR){
           _close_all_pipes(descriptors, N);
           return OP_ERROR;
       }
    }
    _close_unnecesery_pipes_in_parent(descriptors, N);
    if(args[N] == NULL){
        _print_output(descriptors[2*N - 2]);
    } else {
        _save_output(descriptors[2*N - 2], args[N]->args[0]);
    }
    close(descriptors[2*N - 2]);
    return OP_OK;
}

int _initialize_descriptors(int* descriptors, int N){
    for(int i = 0; i < N; i++){
        if(pipe(descriptors + 2*i) == OP_ERROR){
            perror("Error while creating pipes.");
            return OP_ERROR;
        }
    }
    return OP_OK;
}

int _count_number_of_pipe_chunks(PipeArgs** args){
    int ctr = 0;
    while((*args) != NULL){ 
        if((*args)->is_redirected == false){
            ctr++;
        }
        args++;
    }
    return ctr;
}

int _invokeProcess(PipeArgs** args, FILE* file, int* fd, int idx, int num_of_chunks){
    pid_t pid = fork();
    if(pid == 0){
        fclose(file);
        _close_unnecesery_pipes(fd, idx, num_of_chunks);
        int output;
        if((idx != 0 && ((dup2(fd[2*(idx - 1)], STDIN_FILENO) == OP_ERROR) | (close(fd[2*(idx - 1)]) == OP_ERROR))) ||
           ((dup2(fd[2*idx + 1], STDOUT_FILENO) == OP_ERROR) | (close(fd[2*idx + 1]) == OP_ERROR)) ||
           (output = execvp(args[idx]->args[0], args[idx]->args)) == OP_ERROR){
            printf("Error in child!\n");
            freePipeArgs(args);
            exit(output);
        }
    }
    if(pid == -1){
        return OP_ERROR;
    } else {
        return OP_OK;
    }
}

void _close_unnecesery_pipes(int* fd, int idx, int num_of_chunks){
    for(int i = 0; i < num_of_chunks; i++){
        if(idx != i){
            close(fd[2*i + 1]);
        }
        if((idx - 1 != i) && (i > 0)){
            close(fd[2*i]);
        }
    }
}

void _close_unnecesery_pipes_in_parent(int* fd, int num_of_chunks){
    for(int i = 0; i < 2*num_of_chunks; i++){
        if(i != 2*(num_of_chunks -1)){
            close(fd[i]);
        }
    }
}

void _close_all_pipes(int* fd, int num_of_chunks){
    for(int i = 0; i < num_of_chunks; i++){
        close(fd[2*i]);
            close(fd[2*i + 1]);
    }
}

void _print_output(int descriptor){
    int BUFF_SIZE = 1024;
    char data[BUFF_SIZE];
    int num_read;
    printf("[COMMAND OUTPUT]\n");
    while((num_read = read(descriptor, data, BUFF_SIZE)) > 0){
        write(STDOUT_FILENO, data, num_read);
    }
    printf("\n");
}

void _save_output(int descriptor, const char* filename){
    int save_dscrp = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    if(save_dscrp < 0){
        printf("Couldn't save output in file\nInstead the output will be printed out.\n");
        _print_output(descriptor);
    } else {
        printf("Output saving to file...\n");
        int BUFF_SIZE = 1024;
        char data[BUFF_SIZE];
        int num_read;
        while((num_read = read(descriptor, data, BUFF_SIZE)) > 0){
            write(save_dscrp, data, num_read);
        }
        close(save_dscrp);
    }
}

void _printError(int line){
    printf("There was and error while processing batch command in line %d\n", line);
}
