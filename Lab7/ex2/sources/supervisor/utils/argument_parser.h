#pragma once

#include "../../headers/contract.h"
#include "../headers/supervisor_input.h"

int parse_arguments(SupervisorInput *input, int argc, const char *argv[]);
void print_help();