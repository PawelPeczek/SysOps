#include <stdio.h>
#include <stdlib.h>
#include "../headers/master_input.h"
#include "../utils/master_arg_parser.h"
#include "../utils/master_utils.h"

static const int OP_ERROR = -1;

int main(int argc, const char* argv[]){
    MasterInput* program_input = (MasterInput*)calloc(1, sizeof(MasterInput));
    if(program_input == NULL){
        perror("Error while allocation memory");
        exit(13);
    }
    if(parse_master_args(argc, argv, program_input) == OP_ERROR){
        printf("Error while parsing arguments!\n");
        print_help_master();
        free(program_input);
        exit(14);
    }
    if(run_echo_server(program_input) == OP_ERROR){
        perror("Error while master execution.");
        free(program_input);
        exit(15);
    }
    free(program_input);
    return 0;
}