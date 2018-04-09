#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include "../utils/signal_utils.h"



int main(int argc, char *argv[]){
    set_signal_handlers();
    main_task();
    return 0;
}

