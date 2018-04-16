#include <stdio.h>
#include <stdlib.h>
#include "../headers/slave_input.h"
#include "../utils/slave_arg_parser.h"
#include "../utils/slave_utils.h"

static const int OP_ERROR = -1;

int main(int argc, const char* argv[]){
    SlaveInput* program_input = (SlaveInput*)calloc(1, sizeof(SlaveInput));
    if(program_input == NULL){
        perror("Error while allocation memory");
        exit(13);
    }
    if(parse_slave_args(argc, argv, program_input) == OP_ERROR){
        printf("Error while parsing arguments!\n");
        print_help_slave();
        free(program_input);
        exit(14);
    }
    if(run_client(program_input) == OP_ERROR){
        perror("Error while slave execution.");
        free(program_input);
        exit(15);
    }
    free(program_input);
    return 0;
}