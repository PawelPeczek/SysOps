#pragma once

#include <stdbool.h>
#include <mqueue.h>

const static int CLIENTS_MAXNUM = 512;
    
typedef struct {
    bool *slot_free;
    mqd_t *clents_qs_ids;
} Clients;

int establish_msgq();
int start_server();