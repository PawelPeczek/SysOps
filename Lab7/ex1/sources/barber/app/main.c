#include <stdlib.h>
#include <stdio.h>
#include "../headers/program_input.h"
#include "../utils/argument_parser.h"
#include "../utils/syncro_utils.h"

int main(int argc, const char* argv[]){
    ProgramInput input;
    if(parse_arguments(&input, argc, argv) == OP_ERROR){
        print_help();
        exit(1);
    }
    if(barber_loop(&input) == OP_ERROR){
        printf("Error in barber loop.\n");
        exit(6);
    }
    return 0;
}