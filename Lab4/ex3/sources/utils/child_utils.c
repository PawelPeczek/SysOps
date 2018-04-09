#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "child_utils.h"

ProgramInput* input;
static int SIG_got = 0;  
static int SIG_sent = 0;
static const int OP_ERROR = -1;

/*
*   Functions' declarations AREA
*/

void _set_signals_handlers();
void _printStats();
void _set_rt_handlers();
void _SIGRTMIN_plus_one_handler(siginfo_t* info);
void _SIGRTMIN_handler(siginfo_t* info);

/*
*   Functions' declarations AREA END
*/

/*
*   Signals handlers
*/

void _SIGUSR1_handler(int sig_no, siginfo_t* info, void* ucontext){
    if(input->info_close_child != RT){
        SIG_got++;
        if(kill(info->si_pid, SIGUSR1) == OP_ERROR){
            printf("Child error while sending SIGUSR1 to parent.\n");
            exit(13);
        }
        SIG_sent++;
    }
}

void _SIGUSR2_handler(int sig_no){
    _printStats();
    exit(0);
}


void _SIGRT_handler(int sig_no, siginfo_t* info, void* ucontext){
    if(input->info_close_child == RT){
        if(sig_no == SIGRTMIN) {
            _SIGRTMIN_handler(info);
        } else if (sig_no == SIGRTMIN + 1){
            _SIGRTMIN_plus_one_handler(info);
        }
    }
}

void _SIGRTMIN_handler(siginfo_t* info){
    SIG_got++;
    if(kill(info->si_pid, SIGRTMIN) == OP_ERROR){
        _printStats();
        exit(13);
    }
    SIG_sent++;
}

void _SIGRTMIN_plus_one_handler(siginfo_t* info){
    _printStats();
    exit(0);
}


/*
*   Signals handlers END
*/

void perform_communication(ProgramInput* _input){
    input = _input;
    _set_signals_handlers();
    while(true){
        pause();
    }
}

void _set_signals_handlers(){
    struct sigaction action;
    action.sa_sigaction = &_SIGUSR1_handler;
    sigfillset(&action.sa_mask); 
    sigdelset(&action.sa_mask, SIGUSR2);
    action.sa_flags = SA_RESTART | SA_SIGINFO; 
    sigaction(SIGUSR1, &action, NULL); 
    struct sigaction action2;
    action2.sa_handler = &_SIGUSR2_handler;
    sigfillset(&action2.sa_mask); 
    action2.sa_flags = SA_RESTART; 
    sigaction(SIGUSR2, &action2, NULL);
    sigset_t all;
    sigfillset(&all);
    sigprocmask(SIG_BLOCK, &all, NULL);
    sigset_t unblock;
    sigemptyset(&unblock);
    sigaddset(&unblock, SIGUSR1);
    sigaddset(&unblock, SIGUSR2);
    sigprocmask(SIG_UNBLOCK, &unblock, NULL);
    _set_rt_handlers();
    kill((int)getppid(), SIGCONT); // signalling that handlers have been set.
}

void _set_rt_handlers(){
    if(input->info_close_child != RT) return ;
    sigset_t unblock;
    sigemptyset(&unblock);
    for(int i = SIGRTMIN; i <= SIGRTMIN + 1; i++){
        struct sigaction action;
        action.sa_sigaction = &_SIGRT_handler;
        sigfillset(&action.sa_mask); 
        sigdelset(&action.sa_mask, SIGUSR2);
        action.sa_flags = SA_RESTART | SA_SIGINFO; 
        sigaction(i, &action, NULL); 
        sigaddset(&unblock, i);
    }
    sigprocmask(SIG_UNBLOCK, &unblock, NULL);
}

void _printStats(){
    printf("CHILD STATS: %d signals recieved. %d signals sent.\n", SIG_got, SIG_sent);
}