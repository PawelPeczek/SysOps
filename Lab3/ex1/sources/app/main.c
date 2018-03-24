#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include "../headers/program_input.h"
#include "../utils/argument_parser.h"
#include "../utils/file_utils.h"


int main(int argc, const char* argv[]){
    ProgramInput* input = parseArguments(argc, argv);
    if(input == NULL) {
        printHelp();
        exit(1);
    }

    int opStatus;
    if(input->nftw == true) opStatus = proceedFileSystemTraversalFTW(input);
    else opStatus = proceedFileSystemTraversal(input);
    if(opStatus == -1){
        perror("Error while traversal");
        free(input);
        exit(2);
    }
    printf("ALL OK\n");
    free(input);
    return 0;
}