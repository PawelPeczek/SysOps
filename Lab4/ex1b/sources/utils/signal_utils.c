#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include "signal_utils.h"


static const int OP_OK = 0;
static const int OP_ERROR = -1;

/*
*   Functions' declarations area
*/

int _init_bash();

/*
*   Functions' declarations area END
*/


int set_signal_handlers(){
    signal(SIGTSTP, &SIGSTP_handler);
    struct sigaction act_signation;
    act_signation.sa_handler = (&SIGINT_handler);
    if((sigemptyset(&act_signation.sa_mask) == OP_ERROR) ||
       (sigaddset(&act_signation.sa_mask, SIGTSTP) == OP_ERROR)){
        return OP_ERROR;
    }
    act_signation.sa_flags = 0;
    sigaction(SIGINT, &act_signation, NULL);
    return OP_OK;
}

void SIGSTP_handler(int sign_no){
    static int pid = -1;
    static bool program_paused = true;
    if(program_paused == true){
        program_paused = false;
        pid = _init_bash();
    } else {
        program_paused = true;
        if(kill(pid, SIGKILL) == OP_ERROR){
            perror("Error while trying to kill the child proces.");
            exit(3);
        }
    }
}

int _init_bash(){
    int pid;
    if((pid = fork()) == 0){
        // in new process
        execlp("bash", "bash", "date_inf_loop.sh", (char *)NULL);
        // error of execlp -> the end of main program
        perror("Error while trying to execute bash.");
        kill(getppid(), SIGINT);
        exit(1);
    }
    // in main process
    if(pid == -1){
        perror("Error while do fork().");
        exit(2);
    }
    return pid;
}

void SIGINT_handler(int sing_no){
    printf("SIGINT detected.\n");
    if(kill(0, SIGKILL) == OP_ERROR){
        perror("Error while trying to kill the child proces.");
        exit(3);
    }
    exit(0);
}