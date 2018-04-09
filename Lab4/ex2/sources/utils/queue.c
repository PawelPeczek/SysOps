#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

static const int OP_OK = 0;
static const int OP_ERROR = 0;

Queue* queue_init(){
    Queue* queue = (Queue*)calloc(1, sizeof(Queue));
    if(queue == NULL) {
        perror("Cannot allocate memory.");
        return NULL;
    }
    queue->num_of_elems = 0;
    queue->first= NULL;
    queue->last = NULL;
    return queue;
}

int enqueue(Queue* queue, int pid){
    struct Node* new_elem = (struct Node*)calloc(1, sizeof(struct Node));
    if(new_elem == NULL){
        dealloc(queue);
        return OP_ERROR;
    }
    new_elem->pid = pid;
    if(queue->num_of_elems == 0){
        queue->first = queue->last = new_elem;
        new_elem->next = new_elem->prev = NULL;
    } else {
        new_elem->next = NULL;
        new_elem->prev = queue->last;
        queue->last->next = new_elem;
        queue->last = new_elem;
    }
    queue->num_of_elems++;
    return OP_OK;
}

int dequeue(Queue* queue){
    if(queue->num_of_elems == 0)
        return OP_ERROR;
    // printf("deq operation, num of elems: %d!\n", queue->num_of_elems);
    struct Node* head = queue->first;
    // printf("deq operation head set!\n");
    queue->first = head->next;
    //printf("deq operation new head set!\n");
    if(queue->first != NULL)
        queue->first->prev = NULL;
    head->next = NULL;
    int pid = head->pid;
    free(head);
    queue->num_of_elems--;
    return pid;
}

void dealloc(Queue* queue){
    struct Node* it = queue->first;
    while(it != NULL){
        struct Node* tmp = it;
        it = it->next;
        free(tmp);
    }
    free(queue);
}