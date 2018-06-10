#include <stdlib.h>
#include <stdio.h>
#include "../headers/client_input.h"
#include "../utils/server/server_parser.h"
#include "../utils/server/server_utils.h"

int main(int argc, const char* argv[]){
    server_input input;
    if(parse_server_args(argc, argv, &input) == -1){
        print_server_help();
        exit(4);
    }
    start_server(&input);
    return 0;
}