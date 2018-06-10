#pragma once

#include "./server_utils.h"


typedef struct list_node {
    struct list_node * next;
    struct list_node * prev;
    struct client_entry * client;
} list_node;

typedef struct {
    list_node left_sent;
    list_node right_sent;
    list_node * next_to_take;
} client_cyclic_list;

client_cyclic_list * init_list();
int delete_from_list(client_cyclic_list * list, list_node * to_delete);
list_node * add_to_list(client_cyclic_list * list, struct client_entry * entry);        
void dealloc_list(client_cyclic_list ** list);
struct client_entry * get_next_to_send_task(client_cyclic_list * list);