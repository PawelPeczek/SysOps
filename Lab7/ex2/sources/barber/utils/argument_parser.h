#pragma once

#include "../../headers/contract.h"
#include "../headers/program_input.h"

int parse_arguments(ProgramInput *input, int argc, const char *argv[]);
void print_help();