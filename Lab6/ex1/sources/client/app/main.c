#include <stdlib.h>
#include "../utils/communication_utils.h"
#include "../../headers/my_errors.h"

int main(int argc, const char* argv[]){
    if(establish_communication() == OP_ERROR){
        exit(13);
    }
    return 0;
}