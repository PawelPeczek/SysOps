#pragma once

#include "../../headers/client_input.h"

#define MAX_REQ_PARAM_NO 4
#define MIN_REQ_PARAM_NO 3
#define NO_STD_PARAM 1
#define NAME 1
#define CONN_TYPE 2
#define ADDRESS 3
#define PORT 4
#define MAX_PORT_NUM 65535
#define IP_MODE 1


int parse_client_args(int argc, const char * argv[], client_input * const input);
void print_client_help();