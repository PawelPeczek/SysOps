#pragma once

struct Node {
    int pid;
    struct Node* next;
    struct Node* prev;
};

typedef struct {
    int num_of_elems;
    struct Node* first;
    struct Node* last;
} Queue;

Queue* queue_init();
int enqueue(Queue* queue, int pid);
int dequeue(Queue* queue);
void dealloc(Queue* queue);