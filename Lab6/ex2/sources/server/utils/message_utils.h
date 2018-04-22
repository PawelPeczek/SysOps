#pragma once

#include "../../headers/contract.h"

void handle_mirror_request(struct my_msgbuf* request, struct my_msgbuf* response);
void handle_calc_request(struct my_msgbuf* request, struct my_msgbuf* response);
void handle_time_request(struct my_msgbuf* request, struct my_msgbuf* response);