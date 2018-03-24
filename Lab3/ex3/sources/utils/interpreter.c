#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "interpreter.h"
#include "file_reader.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static const int OP_OK = 0;
static const int OP_ERROR = -1;

/*
* Declarations of functions
*/

int _proceedInterpretationLoop(FILE* fileStream, ProgramInput* input);
int _invokeProcess(char** args, ProgramInput* input, int line, FILE* fileStream);
void _freeArgs(char** args);
void _printError(int line);
void _logResourcesUsage(struct rusage* rusage, int line);

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
    char ** args;
    int line = 1, opStatus = OP_OK;
    while((args = preprocessLineOfFile(fileStream)) != NULL){        
        opStatus = _invokeProcess(args, input, line, fileStream);
        _freeArgs(args);
        if(opStatus == OP_ERROR) break;
        line ++;
    }
    if(opStatus == OP_ERROR) _printError(line);
    return opStatus;
}

int _invokeProcess(char** args, ProgramInput* input, int line, FILE* fileStream){
    int opStatus = OP_OK;
    pid_t child_pid = fork();
    if(child_pid != 0){
        // parent
        struct rusage rusage;
        wait(&opStatus);
        getrusage(RUSAGE_CHILDREN, &rusage);
        _logResourcesUsage(&rusage, line);
    } else {
        // new process
        struct rlimit mem_lim, cpu_lim;
        mem_lim.rlim_max = mem_lim.rlim_cur = input->maxVirtMemSizeMB * 1024 * 1024;
        cpu_lim.rlim_max = cpu_lim.rlim_cur = input->maxCPUTime;
        setrlimit(RLIMIT_AS, &mem_lim);
        setrlimit(RLIMIT_CPU, &mem_lim);
        printf(ANSI_COLOR_RESET"\n");
        int output = execvp(args[0], args);
        // if could not load -> the program reaches this point -> we must break
        _freeArgs(args);
        free(input);
        fclose(fileStream);
        exit(output);
    }
    if(opStatus != OP_OK) return OP_ERROR;
    else return OP_OK;
}

void _freeArgs(char** args){
    int i = 0;
    while(args[i] != NULL){
        free(args[i]);
        i++;
    }
    free(args);
}

void _printError(int line){
    printf("There was and error while processing batch command in line %d\n", line);
}

void _logResourcesUsage(struct rusage* rusage, int line){
    printf(ANSI_COLOR_RED"=================================\n");
    printf("\tStats for line %d\n", line);
    printf("=================================\n"ANSI_COLOR_RESET);
    printf("User CPU time: "ANSI_COLOR_GREEN"%ld.%lds\n"ANSI_COLOR_RESET, rusage->ru_utime.tv_sec, rusage->ru_utime.tv_usec);
    printf("System CPU time: "ANSI_COLOR_GREEN"%ld.%lds\n"ANSI_COLOR_RESET, rusage->ru_stime.tv_sec, rusage->ru_stime.tv_usec);
    printf("RSS: "ANSI_COLOR_GREEN"%fMB\n"ANSI_COLOR_RESET, (float)rusage->ru_maxrss / 1024);
    printf("Page faults without I/O act.: "ANSI_COLOR_GREEN"%ld\n"ANSI_COLOR_RESET, rusage->ru_minflt);
    printf("Page faults with I/O act.: "ANSI_COLOR_GREEN"%ld\n"ANSI_COLOR_RESET, rusage->ru_majflt);
    printf("Filesystem inputs: "ANSI_COLOR_GREEN"%ld\n"ANSI_COLOR_RESET, rusage->ru_inblock);
    printf("Filesystem outputs: "ANSI_COLOR_GREEN"%ld\n"ANSI_COLOR_RESET, rusage->ru_oublock);
    printf("No. of times when process gived up CPU before time slice has ended: "
          ANSI_COLOR_GREEN"%ld\n"ANSI_COLOR_RESET, rusage->ru_nvcsw);
    printf("No. of times when scheduler forced process to give up CPU "
          ANSI_COLOR_GREEN"%ld\n"ANSI_COLOR_RESET, rusage->ru_nivcsw);
    printf(ANSI_COLOR_RED"=================================\n"ANSI_COLOR_RESET);
}