#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

static const int no_of_sec = 10;
static const int SIEVE = 1;
static const int OP_ERROR = -1;

void _SIGALRM_HANDLER(int sig_no){
    return ;
}

int main(int argc, char *argv[]){
    if(argc <= SIEVE) return 13;
    signal(SIGALRM, &_SIGALRM_HANDLER);
    srand((unsigned int)atoi(argv[SIEVE]));
    int rnd_time = rand() % no_of_sec + 1;
    sleep(rnd_time);
    int choosenRT = rand() % (SIGRTMAX - SIGRTMIN + 1);
    //kill((int)getppid(), SIGUSR1);
    union sigval value;
    value.sival_int = (int)getpid();
    if(sigqueue((int)getppid(), SIGUSR1, value) == OP_ERROR){
        perror("Error in sigqueue in child process.");
        kill((int)getppid(), SIGKILL);
    }
    pause();
    printf("Proces with PID %d get permissions to work.\n", (int)getpid());
    value.sival_int = rnd_time;
    if(sigqueue((int)getppid(), SIGRTMIN + choosenRT, value) == OP_ERROR){
        perror("Error in sending SIGRT in child process.");
        kill((int)getppid(), SIGKILL);
    }
    exit(rnd_time);
}
