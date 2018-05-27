#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "./argument_parser.h"

#define REQUIRED_ARGC 2
#define CONFIG_FILE 1
#define SRC_BUFF_SIZE 256

/*
*   Functions' definitions area
*/

FILE * _try_open_config(int argc, const char * argv[]);
int _try_read_config(FILE * const config_file, program_input * const input);
bool _less_comparator(const int a, const int b);
bool _more_comparator(const int a, const int b);
bool _equal_comparator(const int a, const int b);
bool _is_config_valid(const program_input * const input);
void _parse_verbose_mode(program_input * const input, const int verbose);
void _parse_seek_mode_or_set_default(program_input * const input, const char seek_mode);

/* 
*   Functions' definitions area END
*/

int parse_arguments(int argc, const char * argv[], program_input * const input){
    FILE * config_file = _try_open_config(argc, argv);
    if(config_file == NULL){
        return -1;
    }
    printf("Config file opened.\n");
    if(_try_read_config(config_file, input) == -1){
        return -1;
    }
    fclose(config_file);
    return 0;
}

FILE * _try_open_config(int argc, const char * argv[]){
    if(REQUIRED_ARGC != argc){
        return NULL;
    }
    FILE * file = fopen(argv[CONFIG_FILE], "r"); 
    if(file == NULL){
        perror("Error while reading config.");
    }
    return file;
}

int _try_read_config(FILE * const config_file, program_input * const input){
    char * buff = (char *)calloc(SRC_BUFF_SIZE, sizeof(char));
    if(buff == NULL){
        perror("Error while memory allocation.");
        return -1;
    }
    char seek_mode;
    int verbose;
    int matched_elems = fscanf(config_file, "%d %d %d %d %c %d %d %s",
                               &(input->prod_numb), &(input->consuments_numb),
                               &(input->buff_size), &(input->compare_value),
                               &seek_mode, &(verbose), &(input->nk), buff);
    input->file_name = buff;
    if(matched_elems != INPUT_ELEMS_REQUIRED){
        printf("Inomplete config!\n");
        free(buff);
        return -1;
    }
    _parse_verbose_mode(input, verbose);
    _parse_seek_mode_or_set_default(input, seek_mode); 
    if(_is_config_valid(input) == false){
        printf("Wrong config!\n");
        free(buff);
        return -1;
    }
    return 0;
}

void _parse_seek_mode_or_set_default(program_input * const input, const char seek_mode){
    switch(seek_mode){
        case '<':
            input->compare_fun = &_less_comparator;
            break;
        case '>':
            input->compare_fun = &_more_comparator;
            break;
        default:
            input->compare_fun = &_equal_comparator;
    }
}

void _parse_verbose_mode(program_input * const input, const int verbose){
    if(verbose <= 0){
        input->verbose = false;
    } else {
        input->verbose = true;
    }
}


bool _less_comparator(const int a, const int b){
    return a < b;
}


bool _more_comparator(const int a, const int b){
    return a > b;
}

bool _equal_comparator(const int a, const int b){
    return a == b;
}


bool _is_config_valid(const program_input * const input){ 
    return ((input->prod_numb > 0) && (input->consuments_numb > 0) &&
            (input->buff_size > 1) && (input->nk >= 0));
}

void print_help(){
    printf("VALID INPUT:\n"
           "program_name config_file\n\n"
           "VALID CONTENT OF config_file:\n"
           "prod_num cons_num buff_size L seek_mode verbose_mode nk file_name\n"
           "WHERE:\n"
           "-> seek_mode = [ < | = | > ]\n"
           "-> verbose_mode = [ 0 | 1 ]\n");    
}