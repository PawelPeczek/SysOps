#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "./argument_parser.h"

#define NUM_OF_DEFAULT_IN_ARGV 1
#define REQ_NUM_OF_ARG 4
#define THR_NO 1
#define SOURCE_FILE 2
#define FILTER_FILE 3
#define OUTPUT_FILE 4


/*
*   Functions' declarations AREA
*/

bool _is_input_valid(const char* argv[], int argc, program_input* const input);
bool _is_argc_valid(int argc);
bool _is_number_valid(const char* number);
bool _is_file_path_valid(const char* path);
void _fullfil_input(const char *argv[], int argc, program_input* const input);

/*
*   Functions' declarations AREA END
*/

int parse_arguments(const char* argv[], int argc, program_input* const input){
    if(_is_input_valid(argv, argc, input)){
        _fullfil_input(argv, argc, input);
        return 0;
    } else {
        return -1;
    }
}

bool _is_input_valid(const char* argv[], int argc, program_input* const input){
    return (
        _is_argc_valid(argc) &&
        _is_number_valid(argv[THR_NO]) &&
        _is_file_path_valid(argv[SOURCE_FILE]) &&
        _is_file_path_valid(argv[FILTER_FILE]) &&
        _is_file_path_valid(argv[OUTPUT_FILE])       
    );
}

bool _is_argc_valid(int argc){
    argc -= NUM_OF_DEFAULT_IN_ARGV;
    return (argc == REQ_NUM_OF_ARG);
}

bool _is_number_valid(const char* number){
    int parsed_num = atoi(number);
    return ((parsed_num > 0) && (parsed_num <= MAX_THREAD_NO));
}

bool _is_file_path_valid(const char* path){
    return (strlen(path) > 0);
}

void _fullfil_input(const char *argv[], int argc, program_input* const input){
    input->threads_no = atoi(argv[THR_NO]);
    input->source_file = argv[SOURCE_FILE];
    input->filter_file = argv[FILTER_FILE];
    input->output_file = argv[OUTPUT_FILE];  
}

void print_help(){
    printf(
        "VALID OPTIONS:\n"
        "program_name threads_no source_file filter_file output_file\n"
    );
}