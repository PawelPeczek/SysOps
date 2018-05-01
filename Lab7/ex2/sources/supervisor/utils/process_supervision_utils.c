#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../../headers/contract.h"
#include "process_supervision_utils.h"


/*
*   Functions' declarations AREA
*/

void _invoke_process(SupervisorInput* input);

/*
*   Functions' declarations AREA END
*/


int supervise_processes(SupervisorInput* input){
    for(int i = 0; i < input->no_clients; i++){
        pid_t PID = fork();
        if(PID == 0){
            _invoke_process(input);
        }
    }
    for (int i = 0; i < input->no_clients; ++i){
        wait(NULL);
    }
    return OP_OK;
}

void _invoke_process(SupervisorInput* input){
    if(execlp("./client", "client", input->no_clipping, NULL) == OP_ERROR){
        perror("Error while creating client");
        exit(7);
    }
}