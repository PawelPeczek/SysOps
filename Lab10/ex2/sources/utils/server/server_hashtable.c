#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "./server_hashtable.h"

/*
*   Functions' declarations area
*/

unsigned long hash(const char * str);
int _init_sentinels(client_node ** ht, int size);
void _dealloc_sentinels(client_node ** ht, int size);
clients_hash_map * _enlarge_hash_map(clients_hash_map ** hm_strct);
int _add_to_slot_list_if_name_free(client_node * head_sent, client_entry * const client);
int _copy_hm_to_new_struct(clients_hash_map * new_hm_strct, clients_hash_map * old_hm_strct);
int _remove_from_ht_slots(client_node * head_sent, const char * name);
clients_hash_map * _reduce_hash_map(clients_hash_map ** hm_strct);
client_entry * _find_client_in_slot_list(client_node * head_sent, const char * name);
void _deallocate_slot_list(client_node * head_sent);
int copy_all_index_list_to_new_hm_struct(clients_hash_map * new_hm_strct, client_node * head_sent);

/* 
*   Functions' declarations area END
*/

clients_hash_map * init_hash_map(int init_max_size){
    clients_hash_map * hash_map = (clients_hash_map *)calloc(1, sizeof(clients_hash_map));
    if(hash_map == NULL){
        perror("Mem alloc error");
        return NULL;
    }
    hash_map->curr_size = 0;
    hash_map->max_size = init_max_size;
    hash_map->init_size = init_max_size;
    hash_map->HT = (client_node **)calloc(init_max_size, sizeof(client_node *));
    if(hash_map->HT == NULL || _init_sentinels(hash_map->HT, init_max_size) == -1){
        perror("Mem alloc error");
        free(hash_map);
        return NULL;
    }
    return hash_map;
}

/*
* If name of client is occupied -> then -1 is returned
*/
int add_to_hash_map_if_slot_free(clients_hash_map ** hm_strct, client_entry * client) {
    if((*hm_strct)->curr_size == (*hm_strct)->max_size){
        clients_hash_map * new_hm_struct = _enlarge_hash_map(hm_strct);
        if(new_hm_struct == NULL){
            perror("Error while re-allocation of hash map");
            return -1;
        }
        *hm_strct = new_hm_struct;
    }
    unsigned long idx = hash(client->name) % (*hm_strct)->max_size;
    if(_add_to_slot_list_if_name_free((*hm_strct)->HT[idx], client) == 0){
        (*hm_strct)->curr_size++;
    } else {
        return -1;
    }
    return 0;
}

int rem_from_hash_map(clients_hash_map ** hm_strct, const char * name){
    unsigned long idx = hash(name) % (*hm_strct)->max_size;
    if(_remove_from_ht_slots((*hm_strct)->HT[idx], name) == -1){
        perror("Cannot find entry to delete");
        return -1;
    }
    (*hm_strct)->curr_size--;
    if((*hm_strct)->curr_size > (*hm_strct)->init_size && (*hm_strct)->curr_size < ((*hm_strct)->max_size / 4)){
        clients_hash_map * new_hm_struct = _reduce_hash_map(hm_strct);
        if(new_hm_struct == NULL){
            perror("Error while re-allocation of hash map");
            return -1;
        }
        *hm_strct = new_hm_struct;
    }
    return 0;
}

void dealloc_hash_map(clients_hash_map ** hm_strct){
    for(int i = 0; i < (*hm_strct)->max_size; i++){
        _deallocate_slot_list((*hm_strct)->HT[i]);
    }
    free((*hm_strct)->HT);
    free(*hm_strct);
    *hm_strct = NULL;  
}


client_entry * get_client_by_name(clients_hash_map * hm_strct, const char * name){
    unsigned long idx = hash(name) % hm_strct->max_size;
    return _find_client_in_slot_list(hm_strct->HT[idx], name);
}


client_entry * _find_client_in_slot_list(client_node * head_sent, const char * name){
    client_node * it =  head_sent->next;
    while(it != NULL){
        if(strcmp(it->client->name, name) == 0){
            return it->client;
        }
        it = it->next;
    }
    return NULL;
}

/*
* http://www.cse.yorku.ca/~oz/hash.html
* NULL-terminated strings!
*/
unsigned long hash(const char *str) {
    unsigned long hash = 5381;
    int i = 0;
    while (str[i] != '\0'){
        hash = ((hash << 5) + hash) + (int)str[i]; /* hash * 33 + c */
        i++;
    }
    return hash;
}

int _init_sentinels(client_node ** ht, int size){
    bool error_occured = false;
    for(int i = 0; i < size; i++){
        ht[i] = calloc(1, sizeof(client_node));
        if(ht[i] == NULL){
            perror("Mem alloc error");
            error_occured = true;
            break;
        }
        ht[i]->client = NULL;
        ht[i]->next = NULL;
    }
    if(error_occured == true){
        _dealloc_sentinels(ht, size);
        return -1;
    } else {
        return 0;
    }
}

void _dealloc_sentinels(client_node ** ht, int size){
    client_node ** it = ht;
    for(int i = 0; i < size; i++){
        free(*it);
        it++;
    }
}

int _add_to_slot_list_if_name_free(client_node * head_sent, client_entry * const client){
    client_node * prev = head_sent;
    client_node * next = prev->next;
    while(next != NULL){
        if(strcmp(next->client->name, client->name) == 0){
            return -1;
        }
        prev = next;
        next = next->next;
    }
    next = calloc(1, sizeof(client_node));
    if(next == NULL){
        perror("Error while mem-alloc");
        return -1;
    }
    prev->next = next;
    next->client = client;
    return 0;
}

/*
*   Preserves old struct if new struct alloc filed!
*/
clients_hash_map * _enlarge_hash_map(clients_hash_map ** old_hm_strct){
    clients_hash_map * new_hm_struct = init_hash_map(2 * (*old_hm_strct)->max_size);
    if(new_hm_struct == NULL){
        return NULL;
    }
    if(_copy_hm_to_new_struct(new_hm_struct, *old_hm_strct) == -1){
        perror("Cannot copy all enries from old hash map to enlarged one");
        dealloc_hash_map(&new_hm_struct);
        return NULL;
    }
    dealloc_hash_map(old_hm_strct);
    return new_hm_struct;
}

/*
*   Preserves old struct if new struct alloc filed!
*/
clients_hash_map * _reduce_hash_map(clients_hash_map ** old_hm_strct){
    clients_hash_map * new_hm_struct = init_hash_map(((*old_hm_strct)->max_size) / 2);
    if(new_hm_struct == NULL){
        return NULL;
    }
    if(_copy_hm_to_new_struct(new_hm_struct, *old_hm_strct) == -1){
        perror("Cannot copy all enries from old hash map to reduced one");
        dealloc_hash_map(&new_hm_struct);
        return NULL;
    }
    dealloc_hash_map(old_hm_strct);
    return new_hm_struct;
}

int _copy_hm_to_new_struct(clients_hash_map * new_hm_strct, clients_hash_map * old_hm_strct){
    for(int i = 0; i < old_hm_strct->max_size; i++){
        if(copy_all_index_list_to_new_hm_struct(new_hm_strct, old_hm_strct->HT[i]) == -1){
            return -1;
        }
    }
    return 0;
}

int copy_all_index_list_to_new_hm_struct(clients_hash_map * new_hm_strct, client_node * head_sent){
    client_node * it = head_sent->next;
    while(it != NULL){
        if(add_to_hash_map_if_slot_free(&new_hm_strct, it->client) == -1){
            return -1;
        }
        it = it->next;
    }
    return 0;
}

int _remove_from_ht_slots(client_node * head_sent, const char * name){
    client_node * prev = head_sent;
    client_node * next = prev->next;
    while(next != NULL){
        if(strcmp(next->client->name, name) == 0){
            prev->next = next->next;
            free(next);
            return 0;
        } 
        prev = next;
        next = next->next;
    }
    return -1;
}

void _deallocate_slot_list(client_node * head_sent){
    client_node * prev = head_sent;
    client_node * next;
    while(prev != NULL){
        next = prev->next;
        free(prev);
        prev = next;
    }
}

