#pragma once

#include <netinet/in.h>

typedef struct {
    in_port_t listen_port_big_end;
    const char * local_socket;
} server_input;