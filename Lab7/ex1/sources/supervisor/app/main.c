#include <stdlib.h>
#include <stdio.h>
#include "../../headers/contract.h"
#include "../headers/supervisor_input.h"
#include "../utils/process_supervision_utils.h"
#include "../utils/argument_parser.h"

int main(int argc, const char* argv[]){
    SupervisorInput input;
    if(parse_arguments(&input, argc, argv) == OP_ERROR){
        print_help();
        exit(1);
    }
    supervise_processes(&input);
    return 0;
}