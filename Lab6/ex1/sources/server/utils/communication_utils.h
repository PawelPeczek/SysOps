#pragma once

#include <stdbool.h>

const static int CLIENTS_MAXNUM = 512;
    
typedef struct {
    bool *slot_free;
    int *clents_qs_ids;
} Clients;

int establish_msgq();
int start_server();