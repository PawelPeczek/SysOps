#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "argument_parser.h"

#define CAPACITY_ARG 1
#define NUM_OF_DEFAULT_IN_ARGV 1
#define REQ_ARG_NUM 1

/*
*   Functions' declaration AREA
*/

bool _is_argc_valid(int argc);
bool _is_number_valid(const char* number);
void _fullfil_input(ClientInput* input, int argc, const char *argv[]);

/*
*   Functions' declaration AREA END
*/

void print_help(){
    printf("VALID INPUT:\nprog_name number_of_clipping\n");
}

int parse_arguments(ClientInput *input, int argc, const char *argv[]){
    if((_is_argc_valid(argc) == false) ||
       (_is_number_valid(argv[CAPACITY_ARG]) == false)){
        return OP_ERROR;
    }
    _fullfil_input(input, argc, argv);
    return OP_OK;
}

bool _is_argc_valid(int argc){
    argc -= NUM_OF_DEFAULT_IN_ARGV;
    return (argc == REQ_ARG_NUM);
}

bool _is_number_valid(const char* number){
    int parsed_num = atoi(number);
    return (parsed_num > 0);
}

void _fullfil_input(ClientInput* input, int argc, const char *argv[]){
    input->no_clipping = atoi(argv[CAPACITY_ARG]);
}