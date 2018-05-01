#include <stdlib.h>
#include <stdio.h>
#include "../../headers/contract.h"
#include "../headers/client_input.h"
#include "../utils/syncro_utils.h"
#include "../utils/argument_parser.h"

int main(int argc, const char* argv[]){
    ClientInput input;
    if(parse_arguments(&input, argc, argv) == OP_ERROR){
        print_help();
        exit(1);
    }
    if(client_loop(&input) == OP_ERROR){
        printf("Error while performing client loop.\n");
        exit(3);
    }
    return 0;
}