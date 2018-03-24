#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include "../../headers/program_input.h"

ProgramInput* parseArguments(int argc, const char* argv[]);
void printHelp();

#endif