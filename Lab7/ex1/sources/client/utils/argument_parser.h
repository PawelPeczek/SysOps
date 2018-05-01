#pragma once

#include "../../headers/contract.h"
#include "../headers/client_input.h"

int parse_arguments(ClientInput *input, int argc, const char *argv[]);
void print_help();