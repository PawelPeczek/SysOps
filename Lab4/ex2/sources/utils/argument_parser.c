#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "argument_parser.h"

static const int NUM_OF_DEFAULT_IN_ARGV = 1;
static const int MIN_ARG_NUM = 2;
static const int MAX_ARG_NUM = 3;
static const int MODE_ARG = 3;
static const int K_ARG = 2;
static const int N_ARG = 1;
static const int OP_OK = 0;
static const int OP_ERROR = -1;

/*
*   Functions' declarations AREA
*/

bool _is_argc_valid(int argc);
bool _is_number_valid(char* number);
bool _is_mode_valid(char* mode);
bool _is_K_fits_N(int K, int N);
bool _dispatchable_mode(char mode);
char _char_to_upper(char c);
void _fullfil_input(ProgramInput* input, int argc, char *argv[]);
void _set_modes(ProgramInput* input, char* modes);
void _set_mode(ProgramInput* input, char mode);

/*
*   Functions' declarations AREA END
*/

int parse_arguments(ProgramInput *input, int argc, char *argv[]){
    if((_is_argc_valid(argc) == false) ||
       (_is_number_valid(argv[N_ARG]) == false) ||
       (_is_number_valid(argv[K_ARG]) == false) ||
       (_is_K_fits_N(atoi(argv[K_ARG]), atoi(argv[N_ARG])) == false) ||
       ((argc > MIN_ARG_NUM + NUM_OF_DEFAULT_IN_ARGV) && (_is_mode_valid(argv[MODE_ARG])) == false)){
        return OP_ERROR;
    }
    _fullfil_input(input, argc, argv);
    return OP_OK;

}


bool _is_argc_valid(int argc){
    argc -= NUM_OF_DEFAULT_IN_ARGV;
    return ((argc >= MIN_ARG_NUM) && (argc <= MAX_ARG_NUM));
}

bool _is_number_valid(char* number){
    int parsed_num = atoi(number);
    return (parsed_num > 0);
}

bool _is_mode_valid(char* mode){
    for(int i = 0; mode[i] != '\0'; i++){
        if(_dispatchable_mode(mode[i]) == false)
            return false;
    }
    return true;
}

bool _is_K_fits_N(int K, int N){
    return (N % K == 0);
}

bool _dispatchable_mode(char mode){
    mode = _char_to_upper(mode);
    char* ref = "ABCDE";
    for(int i = 0; ref[i] != '\0'; i++){
        if(ref[i] == mode){
            return true;
        }
    }
    return false;
}

char _char_to_upper(char c){
    if(c > 96) c -= 32;
    return c;
}

void _fullfil_input(ProgramInput* input, int argc, char *argv[]){
    initialize_struct(input);
    input->N = atoi(argv[N_ARG]);
    input->K = atoi(argv[K_ARG]);
    if(argc > MIN_ARG_NUM + NUM_OF_DEFAULT_IN_ARGV)
        _set_modes(input, argv[MODE_ARG]); 
}

void _set_modes(ProgramInput* input, char* modes){
    for(int i = 0; modes[i] != '\0'; i++){
        _set_mode(input, modes[i]);
    }
}

void _set_mode(ProgramInput* input, char mode){
    switch(mode){
        case 'A':
            input->info_child_create = true;
            break;
        case 'B':
            input->info_child_request = true;
            break;
        case 'C':
            input->info_child_response = true;
            break;
        case 'D':
            input->info_real_time_sig = true;
            break;
        case 'E':
            input->info_close_child = true;
            break;
    }
}


void printHelp(){
    printf("program_name N K [A-E]{0, 5}\n");
    printf("Options:\n");
    printf("A - info about children creation\n");
    printf("B - info about children request\n");
    printf("C - info about response to children\n");
    printf("D - info about children RT signal\n");
    printf("E - info about children termination\n");
}
