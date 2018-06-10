#pragma once

#include <netinet/in.h>

typedef struct {
    const char * name;
    int conn_type;
    const char * addres;
    in_port_t listen_port_big_end;
} client_input;