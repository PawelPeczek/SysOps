#define _GNU_SOURCE
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include "slave_utils.h"

static const int OP_ERROR = -1;
static const int OP_OK = 0;
static const int BUFF_MAX_SIZE = 512;

/*
*   Functions' declarations
*/

int _perform_communication(FILE* fifo, int N);
int _prepare_data_to_send(char* buff);

/*
*   Functions' declarations END
*/


int run_client(SlaveInput* program_input){
    srand(time(NULL));
    printf("Slave process with PID: %d\n", (int)getpid());
    FILE* fifo;
    if((fifo = fopen(program_input->fifo_path, "w")) == NULL){
        return OP_ERROR;
    }
    return _perform_communication(fifo, program_input->N);
}

int _perform_communication(FILE* fifo, int N){
    char buff[BUFF_MAX_SIZE];
    char data_to_send[BUFF_MAX_SIZE];
    for(int i = 0; i < N; i ++){
        if(_prepare_data_to_send(buff) < OP_OK ||
           sprintf(data_to_send, "<%d> %s", (int)getpid(), buff) < OP_OK ||
           fputs(data_to_send, fifo) == EOF){
            return OP_ERROR;
        }
        fflush(fifo);
        printf("Data sent: %s", data_to_send);
        sleep(1 + rand()%2);
    }
    return OP_OK;
}

int _prepare_data_to_send(char* buff){
    FILE* date_output = popen("date", "r");
    if(date_output == NULL){
        return OP_ERROR;
    }
    if(fgets(buff, BUFF_MAX_SIZE, date_output) == NULL){
        return OP_ERROR;
    }
    if(pclose(date_output) == OP_ERROR){
        return OP_ERROR;
    } else {
        return OP_OK;
    }
}