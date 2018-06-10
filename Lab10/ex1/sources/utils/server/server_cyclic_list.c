#include <stdlib.h>
#include <stdio.h>
#include "./server_cyclic_list.h"

client_cyclic_list * init_list(){
    client_cyclic_list * list = (client_cyclic_list *)calloc(1, sizeof(client_cyclic_list));
    if(list == NULL){
        perror("Mem alloc error");
        return NULL;
    }
    list->left_sent.client = list->right_sent.client = NULL;
    list->left_sent.next = &(list->right_sent);
    list->left_sent.prev = &(list->right_sent);
    list->right_sent.next = &(list->left_sent);
    list->right_sent.prev = &(list->left_sent);
    list->next_to_take = NULL;
    return list;
}

int delete_from_list(client_cyclic_list * list, list_node* to_delete){
    to_delete->prev->next = to_delete->next;
    to_delete->next->prev = to_delete->prev;
    if(list->next_to_take == to_delete){
        list->next_to_take = to_delete->next;
        if(list->next_to_take == &(list->right_sent)){
            list->next_to_take = list->left_sent.next;
            if(list->next_to_take == &(list->right_sent)){
                list->next_to_take = NULL;
            }
        }
    }
    free(to_delete);
    return 0;
}

list_node * add_to_list(client_cyclic_list * list, client_entry * entry){
    list_node * new_node = (list_node *)calloc(1, sizeof(list_node));
    if(new_node == NULL){
        perror("Mem alloc error");
        return NULL;
    }
    new_node->client = entry;
    new_node->prev = &(list->left_sent);
    new_node->next = list->left_sent.next;
    list->left_sent.next = new_node;
    if(new_node->next == &(list->right_sent)){
        list->right_sent.prev = new_node;
    }
    if(list->next_to_take == NULL){
        list->next_to_take = new_node;
    } 
    return new_node;
}   

void dealloc_list(client_cyclic_list ** list){
    list_node * it = (*list)->left_sent.next;
    while(it != &((*list)->right_sent)){
        list_node * pom = it;
        it = it->next;
        free(pom);
    }
    free(*list);
    *list = NULL;    
}

client_entry * get_next_to_send_task(client_cyclic_list * list){
    if(list->next_to_take == NULL){
        return NULL;
    } 
    client_entry * next_client = list->next_to_take->client;
    list->next_to_take = list->next_to_take->next;
    if(list->next_to_take == &(list->right_sent)){
        list->next_to_take = list->left_sent.next;
    }
    return next_client;
}
