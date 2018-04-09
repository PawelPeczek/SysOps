#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include "signals_utils.h"
#include "queue.h"

Queue* permissionsQueue;
static const ProgramInput* input;
static const int OP_OK = 0;
static const int OP_ERROR = -1;

/*
*   Functions declarations AREA
*/

int _send_batch_response();
void _kill_em_all();
void _clean_global_variables();
void _set_signals_handlers();
void _set_rt_handlers();
int _wait_for_children_terminate();
int _create_children();

/*
*   Functions declarations AREA END
*/


/*
*   Signal handlers
*/

void _SIGUSR1_handler(int sig_no, siginfo_t* info, void* ucontext){
    if(input->info_child_request == true)
        printf("Child request. PID: %d\n", info->si_pid);
    enqueue(permissionsQueue, info->si_pid);
    if(permissionsQueue->num_of_elems == input->K){
        int opStatus = _send_batch_response();
        if(opStatus == OP_ERROR){
            _kill_em_all();
            _clean_global_variables();
            exit(1);
        }
    }
}

void _SIGINT_handler(int sig_no){
    _clean_global_variables();
    printf("SIGINT handler fired in parent proces\n");
    _kill_em_all();
}

void _SIGRTs_handler(int sig_no, siginfo_t* info, void* ucontext){
    //printf("RT HANDLER\n");
    if(input->info_real_time_sig == true)
        printf("Child RT signal. PID: %d, Signal no.: %d\n", info->si_pid, sig_no);
}


/*
*   Signal handlers END
*/


int main_task(ProgramInput* _input){
    input = _input;
    srand(time(NULL));
    _set_signals_handlers();
    if(_create_children() == OP_ERROR){
        // cleaning
        perror("Error while creating child process.");
        _clean_global_variables();
        _kill_em_all();
        return OP_ERROR;
    }
    return _wait_for_children_terminate();
}

/*
*   Functions definitions AREA
*/

int _send_batch_response(){
    while(permissionsQueue->num_of_elems != 0){
        int childPID = dequeue(permissionsQueue);
        if(input->info_child_response == true)
            printf("Sending response to child with PID: %d\n", childPID);
        int opStaus = kill(childPID, SIGALRM);
        if(opStaus == OP_ERROR) return OP_ERROR;
    }
    return OP_OK;
}

void _set_signals_handlers(){
    permissionsQueue = queue_init();
    struct sigaction action;
    action.sa_sigaction = &_SIGUSR1_handler;
    sigfillset(&action.sa_mask); 
    action.sa_flags = SA_RESTART | SA_SIGINFO; 
    sigaction(SIGUSR1, &action, NULL); 
    signal(SIGINT, &_SIGINT_handler);
    _set_rt_handlers();
}


void _set_rt_handlers(){
    for(int i = SIGRTMIN; i <= SIGRTMAX; i++){
        struct sigaction action;
        action.sa_sigaction = &_SIGRTs_handler;
        sigemptyset(&action.sa_mask); 
        action.sa_flags = SA_RESTART | SA_SIGINFO; 
        sigaction(i, &action, NULL); 
    }
}

int _wait_for_children_terminate(){
    int no_of_finished_prc = 0;
    while(no_of_finished_prc < input->N){
        int status;
        int PID = waitpid(-1, &status, 0);
        if(PID == OP_ERROR){
            _clean_global_variables();
            _kill_em_all();
            return OP_ERROR;
        }
        no_of_finished_prc++;
        if(input->info_close_child == true)
            printf("Child process with PID: %d has finished. Status: %d\n", PID, WEXITSTATUS(status));
        usleep(100);
    }
    return OP_OK;
}

int _create_children(){
    for(int i = 0; i < input->N; i++){
        pid_t PID = fork();
        char str[20];
        sprintf(str, "%d", rand());
        char *argv[] = {"child_program", str, NULL };
        if(PID == 0){
            execvp("./child_program", argv);
            // error!
            perror("Error while loading executable.");
            _clean_global_variables();
            exit(13);
        } else if (PID == OP_ERROR){
            //printf("PID == OP_ERROR\n");
            return OP_ERROR;
        } 
        if(input->info_child_create == true){
            printf("Child created with PID: %d\n", PID);
        }
    }
    return OP_OK;
}

void _kill_em_all(){
    //printf("_kill_em_all fired\n");
    int opStatus = kill(0, SIGKILL);
    if(opStatus == OP_ERROR){
        perror("Error while killing all children processes.");
    }
}

void _clean_global_variables(){
    dealloc(permissionsQueue);
}

/*
*   Functions definitions AREA END
*/