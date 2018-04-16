#include <stdio.h>
#include <stdlib.h>
#include "../headers/program_input.h"
#include "../utils/argument_parser.h"
#include "../utils/file_reader.h"
#include "../utils/interpreter.h"
#include "../headers/pipe_args.h"

int main(int argc, const char* argv[]){
    ProgramInput* input = parseArguments(argc, argv);
    if(input == NULL){
        printHelp();
        exit(1);
    }
    printf("[INPUT DEBUG] path: %s\n", input->batchFilePath);
    // proceedBatchInterpretation(input);
    // FILE* file = fopen(input->batchFilePath, "r");
    // if(file == NULL){
    //     perror("Could not open file.");
    //     exit(13);
    // }
    // PipeArgs **parsed = preprocessLineOfFile(file);
    // printf("Hey!\n");
    // if(parsed != NULL){
    //     for(int i = 0; parsed[i] != NULL; i++){
    //     printf("Depacked pipe number: %d\n", i);
    //         for(int j = 0; parsed[i]->args[j]!= NULL; j++){
    //             printf("%s - is redirected: %d\n", parsed[i]->args[j], parsed[i]->is_redirected);

    //         }
    //     }
    // } else {
    //     printf("Error while processing...\n");
    // }
    proceedBatchInterpretation(input);
    free(input);
    return 0;
}