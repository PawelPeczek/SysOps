#include <stdio.h>
#include <stdlib.h>
#include "../headers/program_input.h"
#include "../utils/argument_parser.h"
#include "../utils/file_reader.h"
#include "../utils/interpreter.h"

int main(int argc, const char* argv[]){
    ProgramInput* input = parseArguments(argc, argv);
    if(input == NULL){
        printHelp();
        exit(1);
    }
    printf("[INPUT DEBUG] path: %s\n", input->batchFilePath);
    proceedBatchInterpretation(input);
    free(input);
    return 0;
}