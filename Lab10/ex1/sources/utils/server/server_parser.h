#pragma once

#include "../../headers/server_input.h"

#define REQ_PARAM_NO 2
#define NO_STD_PARAM 1
#define PORT_NO 1
#define L_SOC_SRC 2
#define MAX_PORT_NUM 65535


int parse_server_args(int argc, const char * argv[], server_input * const input);
void print_server_help();