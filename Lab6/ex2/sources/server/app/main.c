#include <stdlib.h>
#include "../utils/communication_utils.h"
#include "../../headers/my_errors.h"

int main(int argc, const char* argv[]){
    if(establish_msgq() == OP_ERROR){
        exit(13);
    }
    if(start_server() == OP_ERROR){
        exit(14);
    }
    return 0;
}