#include "master_arg_parser.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const int PROG_NAME_IN_ARGV = 1;
static const int PARAM_NUM = 1;
static const int PATH_INDEX = 1;
static const int OP_ERROR = -1;
static const int OP_OK = 0;

/*
*   Functions' declarations
*/

bool _is_param_number_correct(int argc);
bool _is_path_valid(const char* path);

/*
*   Functions' declarations END
*/

int parse_master_args(int argc, const char* argv[], MasterInput* programInput){
    if(_is_param_number_correct(argc) != true ||
       _is_path_valid(argv[PATH_INDEX]) != true){
        return OP_ERROR;
    } 
    programInput->fifo_path = argv[PATH_INDEX];
    return OP_OK;
}

bool _is_param_number_correct(int argc){
    return (argc == PARAM_NUM + PROG_NAME_IN_ARGV);
}

bool _is_path_valid(const char* path){
    return (strlen(path) > 0);
}

void print_help_master(){
    printf("Valid program input:\n");
    printf("master_program_name fifo_src\n");
}
