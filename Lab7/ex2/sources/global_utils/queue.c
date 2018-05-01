#include "queue.h"

int initialize_queue(Queue* q, int size){
    if(size <= 0 || size > MAX_QUEUE_SIZE){
        return OP_ERROR;
    }
    q->head = 0;
    q->tail = 0;
    q->current_load = 0;
    q->size = size;
    q->instant_client = -1;
    q->barber_sleeping = false;
    return OP_OK;
}

void set_instant_client(Queue* q, int pid){
    q->instant_client = pid;
}

int enqueue(Queue* q, int pid, int* ticket){
    if(q->current_load == q->size){
        return OP_ERROR;
    }
    if(q->current_load != 0){
        q->tail = (q->tail + 1) % q->size;
    }
    q->elems[q->tail] = pid;
    *ticket = q->tail;
    q->current_load++;
    return OP_OK;
}

int dequeue(Queue* q, int* out){
    if(q->current_load == 0){
        return OP_ERROR;
    }
    *out = q->elems[q->head]; 
    q->current_load--;
    if(q->current_load != 0){
        q->head = (q->head + 1) % q->size;
    } 
    return OP_OK;
}