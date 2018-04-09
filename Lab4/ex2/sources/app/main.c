#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../headers/program_input.h"
#include "../utils/argument_parser.h"
#include "../utils/signals_utils.h"

static const int OP_ERROR = -1;

int main(int argc, char *argv[]){
    ProgramInput input;
    if(parse_arguments(&input, argc, argv) == OP_ERROR){
        printHelp();
        return 1;
    }
    if(main_task(&input) == OP_ERROR){
        perror("Error while parent main-task processing.");
        return 2;
    }
    return 0;
}
