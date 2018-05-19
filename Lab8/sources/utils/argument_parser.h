#pragma once
#include "../headers/program_input.h"

int parse_arguments(const char* argv[], int argc, program_input* const input);
void print_help();