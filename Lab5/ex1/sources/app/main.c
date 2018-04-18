#include <stdio.h>
#include <stdlib.h>
#include "../headers/program_input.h"
#include "../utils/argument_parser.h"
#include "../utils/file_reader.h"
#include "../utils/interpreter.h"
#include "../headers/pipe_args.h"

static const int OP_ERROR = -1;

int main(int argc, const char* argv[]){
    ProgramInput* input = parseArguments(argc, argv);
    if(input == NULL){
        printHelp();
        exit(1);
    }
    int opStatus = proceedBatchInterpretation(input);
    if(opStatus == OP_ERROR){
        perror("Error while interptering.");
    }
    free(input);
    return 0;
}