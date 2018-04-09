#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../headers/program_input.h"
#include "../utils/argument_parser.h"
#include "../utils/child_utils.h"

static const int OP_ERROR = -1;

int main(int argc, char *argv[]){
    ProgramInput input;
    if(parse_arguments(&input, argc, argv) == OP_ERROR){
        printf("Error while parsing arguments in child program.\n");
        exit(13);
    }
    perform_communication(&input);
    return 0;
}
