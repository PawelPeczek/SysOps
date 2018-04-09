#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include "parent_utils.h"

ProgramInput* input;
static int SIG_got = 0;  
static int SIG_sent = 0;
static const int OP_ERROR = -1;
static const int OP_OK = 0;
static const int CHILD_ERROR_STATUS = 13;
pid_t CHILD_PID = 0;

/*
*   Functions' declarations area
*/

void _set_signals_handlers();
void _print_stats();
int _send_async(pid_t child_pid);
int _send_initial_SIGUSR1(pid_t child_pid);
int _send_rt(pid_t child_pid);
void _set_rt_handlers();
int _dispatch_requests(pid_t pid);

/*
*   Functions' declarations area END
*/


/*
*   Signals handlers begin
*/

void _SIGINT_hanlder(int sig_no){
    kill(CHILD_PID, SIGUSR2);
    int status;
    waitpid(CHILD_PID, &status, 0);
    _print_stats();
    exit(1);
}

void _SIGCONT_empty_handler(int sig_no){
    // empty body
}

void _SIGUSR1_async_handler(int sig_no, siginfo_t* info, void* ucontext){
    SIG_got++;
}

void _SIGUSR1_sync_handler(int sig_no, siginfo_t* info, void* ucontext){
    SIG_got++;
    if(SIG_got == input->L){
        _print_stats();
        kill(info->si_pid, SIGUSR2);
        exit(0);
    } else {
        if(kill(info->si_pid, SIGUSR1) == OP_ERROR){
            kill(info->si_pid, SIGKILL);
            exit(13);
        }
        SIG_sent++;
    }
}

void _SIGRT_handler(int sig_no, siginfo_t* info, void* ucontext){
    SIG_got++;
    if(SIG_got == input->L){
        _print_stats();
        if(kill(info->si_pid, SIGRTMIN + 1) == OP_ERROR){
            kill(info->si_pid, SIGKILL);
            exit(13);
        }
        exit(0);
    }
}

/*
*   Signals handlers end
*/

int main_task(ProgramInput* _input){
    input = _input;
    _set_signals_handlers();
    pid_t pid = fork();
    if(pid == 0){
        char L[12], Mode[2];
        sprintf(L, "%d", input->L);
        sprintf(Mode, "%d", input->info_close_child);
        char *argv[] = {"child_program", L, Mode, NULL };
        execvp("./child_program", argv);
        // error!
        perror("Error while loading executable.");
        exit(13);
    } else {
        CHILD_PID = pid;
        pause(); // waiting for children to set handlers.
        int status;
        if((_dispatch_requests(pid) == OP_ERROR) ||
           (waitpid(pid, &status, 0) == OP_ERROR) ||
           (WEXITSTATUS(status) == CHILD_ERROR_STATUS)){
               return OP_ERROR;
        }
    }
    return OP_OK;
}

int _dispatch_requests(pid_t pid){
    int opStatus;
    switch(input->info_close_child){
        case KILL_ASYNC:
            opStatus = _send_async(pid);  
            break;
        case KILL_SEQ:
            opStatus = _send_initial_SIGUSR1(pid);
            break;
        case RT:
            opStatus = _send_rt(pid);
            break;
    }
    return opStatus;
}

void _set_signals_handlers(){
    signal(SIGINT, &_SIGINT_hanlder);
    signal(SIGCONT, &_SIGCONT_empty_handler);
    struct sigaction action;
    sigfillset(&action.sa_mask);
    switch(input->info_close_child){
        case KILL_ASYNC:
            action.sa_sigaction = &_SIGUSR1_async_handler;
            break;
        case KILL_SEQ:
            action.sa_sigaction = &_SIGUSR1_sync_handler;
            break;
        case RT:
            _set_rt_handlers();
            break;
    }
    if(input->info_close_child != RT){
        action.sa_flags = SA_RESTART | SA_SIGINFO; 
        sigaction(SIGUSR1, &action, NULL); 
    }    
}

void _set_rt_handlers(){
    struct sigaction action;
    action.sa_sigaction = &_SIGRT_handler;
    sigfillset(&action.sa_mask); 
    action.sa_flags = SA_RESTART | SA_SIGINFO; 
    sigaction(SIGRTMIN, &action, NULL); 
}

void _print_stats(){
    printf("PARENT STATS: %d signals recieved. %d signals sent.\n", SIG_got, SIG_sent);
}

int _send_async(pid_t child_pid){
    for(int i = 0; i < input->L; i++){
        if(kill(child_pid, SIGUSR1) == OP_ERROR){
            return OP_ERROR;
        }
        SIG_sent++;
    }
    if(kill(child_pid, SIGUSR2) == OP_ERROR){
        return OP_ERROR;
    }
    _print_stats();
    return OP_OK;
}

int _send_initial_SIGUSR1(pid_t child_pid){
    if(kill(child_pid, SIGUSR1) == OP_ERROR) {
        return OP_ERROR;
    } else {
        SIG_sent++;
        return OP_OK;
    }
}

int _send_rt(pid_t child_pid){
    for(int i = 0; i < input->L; i++){
        if(kill(child_pid, SIGRTMIN) == OP_ERROR){
            return OP_ERROR;
        }
        SIG_sent++;
    }
    return OP_OK;
}