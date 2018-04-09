#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "argument_parser.h"

static const int NUM_OF_DEFAULT_IN_ARGV = 1;
static const int ARG_NUM = 2;
static const int MODE_ARG = 2;
static const int L_ARG = 1;
static const int OP_OK = 0;
static const int OP_ERROR = -1;

/*
*   Functions' declarations AREA
*/

bool _is_argc_valid(int argc);
bool _is_number_valid(char* number);
bool _is_mode_valid(char* mode);
void _fullfil_input(ProgramInput* input, int argc, char *argv[]);
void _set_mode(ProgramInput* input, char mode);

/*
*   Functions' declarations AREA END
*/

int parse_arguments(ProgramInput *input, int argc, char *argv[]){
    if((_is_argc_valid(argc) == false) ||
       (_is_number_valid(argv[L_ARG]) == false) ||
       (_is_mode_valid(argv[MODE_ARG]) == false)){
        return OP_ERROR;
    }
    _fullfil_input(input, argc, argv);
    return OP_OK;

}


bool _is_argc_valid(int argc){
    argc -= NUM_OF_DEFAULT_IN_ARGV;
    return (argc == ARG_NUM);
}

bool _is_number_valid(char* number){
    int parsed_num = atoi(number);
    return (parsed_num > 0);
}

bool _is_mode_valid(char* mode){
    return (
        (strlen(mode) == 1) &&
        ((mode[0] == '1') || (mode[0] == '2') || (mode[0] == '3')) 
    );
}

void _fullfil_input(ProgramInput* input, int argc, char *argv[]){
    input->L = atoi(argv[L_ARG]);
    _set_mode(input, argv[MODE_ARG][0]); 
}

void _set_mode(ProgramInput* input, char mode){
    switch(mode){
        case '1':
            input->info_close_child = KILL_ASYNC;
            break;
        case '2':
            input->info_close_child = KILL_SEQ;
            break;
        case '3':
            input->info_close_child = RT;
            break;
    }
}


void printHelp(){
    printf("program_name L mode\n");
    printf("Modes:\n");
    printf("1 - SIGUSR1, SIGUSR2 async with kill\n");
    printf("2 - SIGUSR1, SIGUSR2 sync with kill\n");
    printf("3 - SIGRT async sith kill\n");
}
