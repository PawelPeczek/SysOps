#pragma once

#include "./server_utils.h"

typedef struct client_node {
    struct client_node * next;
    struct client_entry * client;
} client_node;

typedef struct clients_hash_map {
    client_node ** HT;
    int curr_size;
    int max_size;
    int init_size;
} clients_hash_map;

clients_hash_map * init_hash_map(int init_max_size);
int add_to_hash_map_if_slot_free(clients_hash_map ** hm_strct, client_entry * client);
int rem_from_hash_map(clients_hash_map ** hm_strct, const char * name);
void dealloc_hash_map(clients_hash_map ** hm_strct);
client_entry * get_client_by_name(clients_hash_map * hm_strct, const char * name);