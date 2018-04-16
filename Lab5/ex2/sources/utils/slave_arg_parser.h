#pragma once

#include "../headers/slave_input.h"

int parse_slave_args(int argc, const char* argv[], SlaveInput* programInput);
void print_help_slave();