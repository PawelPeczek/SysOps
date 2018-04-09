#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include "signal_utils.h"


static const int OP_OK = 0;
static const int OP_ERROR = -1;
bool allow_loop = true;
/*
*   Functions' declarations area
*/

void _print_date_inf();

/*
*   Functions' declarations area END
*/
void SIGINT_handler(int sing_no){
    printf("SIGINT detected.\n");
    exit(0);
}

void SIGSTP_handler(int sign_no){
    if(allow_loop == true){
        allow_loop = false;
        printf("Waiting for CTRl+Z for continue or CTRL+C to stop.\n");
    } else {
        allow_loop = true;
    }
}

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

void main_task(){
    while(true){
       _print_date_inf();
    }
}

void _print_date_inf(){
    time_t rawtime;
    struct tm* timeinfo;
    while(allow_loop == true){
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        printf ( "Current local time and date: %s", asctime (timeinfo));
    }
}