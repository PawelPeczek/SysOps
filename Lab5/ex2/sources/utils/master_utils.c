#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "master_utils.h"

static const int OP_ERROR = -1;
static const int OP_OK = 0;

/*
*   Functions' declarations
*/

void _read_from_fifo(FILE* fifo);
int _run_fifo(MasterInput* program_input);

/*
*   Functions' declarations END
*/

int run_echo_server(MasterInput* program_input){
    if(_run_fifo(program_input) == OP_ERROR){
        return OP_ERROR;
    } else {
        return OP_OK;
    }    
}

int _run_fifo(MasterInput* program_input){
    if(mkfifo(program_input->fifo_path, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH) == OP_ERROR){
        return OP_ERROR;
    }
    FILE* fifo;
    if((fifo = fopen(program_input->fifo_path, "r")) == NULL){
        printf("Error while opening fifo\n");
        return OP_ERROR;
    }
    printf("FIFO OPENED IN MASTER!\n");
    _read_from_fifo(fifo);
    fclose(fifo);
    return OP_OK;
}

void _read_from_fifo(FILE* fifo){
    char* line;
    size_t len = 0;
    while(getline(&line, &len, fifo) != OP_ERROR){
        printf("%s", line);
        free(line);
        len = 0;
    }
}