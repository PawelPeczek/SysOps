#pragma once

#include <stdbool.h>
#include "../../headers/communication_contract.h"
#include "../../headers/server_input.h"
#include "./server_cyclic_list.h"

typedef struct op_list_node {
    struct op_list_node * next;
    op_request * request;
} op_list_node;

typedef struct {
    op_list_node sentinel;
} op_list;

typedef struct client_entry {
    char name[UNIX_PATH_MAX];
    op_list active_requests;
    struct list_node * ptr_to_list;
    bool pong_received;
    int socket_fd;
} client_entry;


void start_server(server_input * input);