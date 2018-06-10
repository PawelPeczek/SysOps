#include <stdlib.h>
#include <stdio.h>
#include "../headers/client_input.h"
#include "../utils/client/client_parser.h"
#include "../utils/client/client_utils.h"

int main(int argc, const char* argv[]){
    client_input input;
    if(parse_client_args(argc, argv, &input) == -1){
        print_client_help();
        exit(4);
    }
    start_client(&input);
    return 0;
}