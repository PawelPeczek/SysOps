#include "program_input.h"

void initialize_struct(ProgramInput* input){
    input->info_child_create = false;
    input->info_child_request = false;
    input->info_child_response = false;
    input->info_close_child = false;
    input->info_real_time_sig = false;
    input->K = 1;
    input->N = 1;
}