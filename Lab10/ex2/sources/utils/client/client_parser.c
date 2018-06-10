#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <inttypes.h>
#include "./client_parser.h"
#include "../../headers/communication_contract.h"


/*
*   Functions' declarations AREA
*/

bool _is_input_valid(int argc, const char* argv[], client_input * const input);
bool _is_argc_valid(int argc);
bool _is_number_valid(const char* number);
bool _is_file_path_valid(const char* path);
bool _is_name_valid(const char * name);
void _fullfil_input(int argc, const char *argv[], client_input * const input);
bool _is_port_valid(int argc, const char* argv[]);
bool _is_argc_valid_if_ip_mode(int argc, const char* argv[]);

/*
*   Functions' declarations AREA END
*/

int parse_client_args(int argc, const char * argv[], client_input * const input){
    if(_is_input_valid(argc, argv, input)){
        _fullfil_input(argc, argv, input);
        return 0;
    } else {
        return -1;
    }
}

bool _is_input_valid(int argc, const char* argv[], client_input * const input){
    return (
        _is_argc_valid(argc) &&
        _is_number_valid(argv[CONN_TYPE]) &&
        _is_name_valid(argv[NAME]) &&
        _is_port_valid(argc, argv) &&
        _is_argc_valid_if_ip_mode(argc, argv) &&
        _is_file_path_valid(argv[ADDRESS])     
    );
}

bool _is_name_valid(const char * name){
    size_t len = strlen(name);
    return (len > 0 && len <= UNIX_PATH_MAX);
}

bool _is_port_valid(int argc, const char* argv[]){
    if((argc - 1) > MIN_REQ_PARAM_NO){
        return _is_number_valid(argv[PORT]);
    } else {
        return true;
    }
}

bool _is_argc_valid_if_ip_mode(int argc, const char* argv[]){
    if(atoi(argv[CONN_TYPE]) == IP_MODE){
        return (argc - 1) == MAX_REQ_PARAM_NO;
    } else {    
        return true;
    }
}

bool _is_argc_valid(int argc){
    argc -= NO_STD_PARAM;
    return (argc >= MIN_REQ_PARAM_NO && argc <= MAX_REQ_PARAM_NO);
}

bool _is_number_valid(const char* number){
    int parsed_num = atoi(number);
    return ((parsed_num >= 0) && (parsed_num <= MAX_PORT_NUM));
}

bool _is_file_path_valid(const char* path){
    size_t len = strlen(path);
    return (len > 0 && len <= UNIX_PATH_MAX);
}

void _fullfil_input(int argc, const char *argv[], client_input * const input){
    input->name = argv[NAME];
    input->conn_type = atoi(argv[CONN_TYPE]);
    input->addres = argv[ADDRESS];
    if(input->conn_type == 1){
        uint16_t port_no = (uint16_t)atoi(argv[PORT]);
        input->listen_port_big_end = htons(port_no);
    }
}


void print_client_help(){
    printf("VALID INPUT:\n./client client_name conn_type address [port]\n");
    printf("client_name max len (with \\0): %d\n", UNIX_PATH_MAX);
    printf("conn_type 0 -> LOCAL\n");
    printf("conn_type 1 -> IPv4\n");
    printf("address -> if IPv4 mode -> dot-decimal\n");      
    printf("port -> required only when IPv4 mode selected\n");      
}