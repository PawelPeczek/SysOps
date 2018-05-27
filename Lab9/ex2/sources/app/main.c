#include <stdlib.h>
#include <stdio.h>
#include "../headers/program_input.h"
#include "../utils/argument_parser.h"
#include "../utils/prod_cons_problem_solver.h"

int main(int argc, const char* argv[]){
    program_input input;
    if(parse_arguments(argc, argv, &input) == -1){
        print_help();
        exit(4);
    }
    if(solve_prod_cons_problem(&input) == -1){
        printf("here!\n");
        exit(13);
    }
    return 0;
}

