#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <stdlib.h>
#include "./server_parser.h"
#include "../../headers/communication_contract.h"
/*
*   Functions' declarations AREA
*/

bool _is_input_valid(int argc, const char* argv[], server_input * const input);
bool _is_argc_valid(int argc);
bool _is_number_valid(const char* number);
bool _is_file_path_valid(const char* path);
void _fullfil_input(int argc, const char *argv[], server_input * const input);

/*
*   Functions' declarations AREA END
*/

int parse_server_args(int argc, const char * argv[], server_input * const input){
    if(_is_input_valid(argc, argv, input)){
        _fullfil_input(argc, argv, input);
        return 0;
    } else {
        return -1;
    }
}

bool _is_input_valid(int argc, const char* argv[], server_input * const input){
    return (
        _is_argc_valid(argc) &&
        _is_number_valid(argv[PORT_NO]) &&
        _is_file_path_valid(argv[L_SOC_SRC])     
    );
}

bool _is_argc_valid(int argc){
    argc -= NO_STD_PARAM;
    return (argc == REQ_PARAM_NO);
}

bool _is_number_valid(const char* number){
    int parsed_num = atoi(number);
    return ((parsed_num > 0) && (parsed_num <= MAX_PORT_NUM));
}

bool _is_file_path_valid(const char* path){
    size_t len = strlen(path);
    return (len > 0 && len <= UNIX_PATH_MAX);
}

void _fullfil_input(int argc, const char *argv[], server_input * const input){
    input->local_socket = argv[L_SOC_SRC];
    uint16_t port_no = (uint16_t)atoi(argv[PORT_NO]);
    input->listen_port_big_end = htons(port_no);
}


void print_server_help(){
    printf("VALID INPUT:\n./prog_name port_num lockal_socket_src\n");
}
