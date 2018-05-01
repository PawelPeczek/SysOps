#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>
#include "syncro_utils.h"
#include "../../global_utils/queue.h"


#define ADDITIONAL_SEM_REQUIRED 5

const char* PATH = "/etc";
const char* SEM_BASE = "barber_semaphore_";
int QUEUE_MEM_ID = -1;
int WAIT_ROOM_READY;
int BARBER_READY;
int BARBER_SLEEPING;
int CLIPPING;
int CLIENT_ACTION;
Queue* QUEUE = NULL;
sem_t** SEMAPHORES = NULL;

/*
*   Functions' declaration AREA
*/

int _set_signal_handlers();
int _initialize_shared_memory(ProgramInput* input);
int _initialize_semaphores(ProgramInput* input);
void _clean_POSIX();
void _main_loop();
void _wait(int sem_no);
void _signal(int sem_no);
void _print_time_msg(const char* msg);
void _print_time_pid_msg(const char* msg, int pid);

/*
*   Functions' declaration AREA END
*/

/*
*   SIGNALS HANDLERS
*/

void _SIGINT_handler(int signo){
    exit(2);
}

/*
*   SIGNALS HANDLERS END
*/


int barber_loop(ProgramInput* input){
    if(_set_signal_handlers() == OP_ERROR || 
       _initialize_shared_memory(input) == OP_ERROR ||
       _initialize_semaphores(input) == OP_ERROR){
        return OP_ERROR;
    } 
    _main_loop();
    return OP_OK;
}


int _set_signal_handlers(){
    struct sigaction action;
    action.sa_handler = &_SIGINT_handler;
    if(sigfillset(&action.sa_mask) == OP_ERROR){
        perror("Error while filling mask signal set.");
        return OP_ERROR;
    }
    action.sa_flags = SA_RESTART; 
    if(sigaction(SIGINT, &action, NULL) == OP_ERROR ||
       sigaction(SIGTERM, &action, NULL) == OP_ERROR){
        perror("Error while setting signal handlers.");
        return OP_ERROR;
    }
    if(atexit(&_clean_POSIX) == OP_ERROR){
        perror("Error while setting atexit function.");
        return OP_ERROR;
    }
    return OP_OK;
}

int _initialize_shared_memory(ProgramInput* input){
    if((QUEUE_MEM_ID =  shm_open(PATH, O_CREAT | O_EXCL | O_RDWR, 0774)) == OP_ERROR){
        perror("Error while creating queue shared memory segment.");
        return OP_ERROR;
    } 
    if(ftruncate(QUEUE_MEM_ID, sizeof(Queue)) == OP_ERROR){
        perror("Error while truncating shared memory segment.");
        return OP_ERROR;
    }
    QUEUE = (Queue*)mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, QUEUE_MEM_ID, 0);
    if((void *)QUEUE == (void *)OP_ERROR){
        perror("Error while fetching shared memory chunk");
        return OP_ERROR;
    }
    if(initialize_queue(QUEUE, input->queue_size) == OP_ERROR){
        printf("Error while initializing queue.\n");
        return OP_ERROR;
    }
    return OP_OK;
}

int _initialize_semaphores(ProgramInput* input){
    SEMAPHORES = (sem_t**)calloc(input->queue_size + ADDITIONAL_SEM_REQUIRED, sizeof(sem_t*));
    if(SEMAPHORES == NULL){
        perror("Error while alocating memory.");
        return OP_ERROR;
    }   
    CLIPPING = input->queue_size;
    CLIENT_ACTION = CLIPPING + 1;
    BARBER_SLEEPING = CLIENT_ACTION + 1;
    BARBER_READY = BARBER_SLEEPING + 1;
    WAIT_ROOM_READY = BARBER_READY + 1;
    char buffer[32];
    for(int i = 0; i < BARBER_READY; i++){
        sprintf(buffer, "%s%d", SEM_BASE, i);
        SEMAPHORES[i] = sem_open(buffer, O_CREAT | O_EXCL | O_RDWR, 0774, 0);
        if(SEMAPHORES[i] == SEM_FAILED){
            perror("Error while setting initial value of semaphore.");
            return OP_ERROR;
        }
    }
    for(int i = BARBER_READY; i <= WAIT_ROOM_READY; i++){
        sprintf(buffer, "%s%d", SEM_BASE, i);
        SEMAPHORES[i] = sem_open(buffer, O_CREAT | O_EXCL | O_RDWR, 0774, 1);
        if(SEMAPHORES[i] == SEM_FAILED){
            perror("Error while setting initial value of semaphore.");
            return OP_ERROR;
        }
    }
    return OP_OK;
}

void _main_loop(){
    int pid;
    while(true){
        _print_time_msg("THE BARBER IS HANGING ON WAIT ROOM");
        WAIT(SEMAPHORES[WAIT_ROOM_READY]);
        _print_time_msg("THE BARBER STOPPED HANGING ON WAIT ROOM");
        if(QUEUE->current_load == 0){
            WAIT(SEMAPHORES[BARBER_READY]);
            _print_time_msg("The barber is falling asleep");
            QUEUE->barber_sleeping = true;
            SIGNAL(SEMAPHORES[BARBER_READY]);
            SIGNAL(SEMAPHORES[WAIT_ROOM_READY]); 
            WAIT(SEMAPHORES[BARBER_SLEEPING]);           
            _print_time_msg("The barber is waking up");
            pid = QUEUE->instant_client;
            SIGNAL(SEMAPHORES[CLIPPING]);             
        } else {
            SIGNAL(SEMAPHORES[QUEUE->head]);
            if(dequeue(QUEUE, &pid) == OP_ERROR){
                printf("Trying to dequeue from empty queue.");
                exit(5);
            }
            _print_time_pid_msg("The barber is asking a person from queue", pid);
            SIGNAL(SEMAPHORES[WAIT_ROOM_READY]);
            SIGNAL(SEMAPHORES[CLIPPING]);
        }
        WAIT(SEMAPHORES[CLIENT_ACTION]);
        _print_time_pid_msg("The barber is starting clipping.", pid);
        _print_time_pid_msg("The barber ended up clipping.", pid);
        SIGNAL(SEMAPHORES[CLIPPING]);
        _print_time_msg("THE BARBER IS HANGING ON CLIENT ACTION");
        WAIT(SEMAPHORES[CLIENT_ACTION]);
        _print_time_msg("THE BARBER STOPED HANGING ON CLIENT ACTION");
    }
}

void _print_time_msg(const char* msg){
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("%ld\t%ld\t%s.\n", tp.tv_sec, tp.tv_nsec, msg);
}

void _print_time_pid_msg(const char* msg, int pid){
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("%ld\t%ld\t %s. Client no. <%d>\n", tp.tv_sec, tp.tv_nsec, msg, pid);
}

void _clean_POSIX(){
    printf("ATEXIT\n");
    if(QUEUE != NULL && munmap(QUEUE, sizeof(Queue)) == OP_ERROR){
        perror("Error while closing shared memory segment.");
    }
    if(QUEUE_MEM_ID != -1 && shm_unlink(PATH) == OP_ERROR){
        perror("Error while trying to delete shared memory segment.");
    }
    if(SEMAPHORES != NULL){
        char buffer[32];
        for(int i = 0; i <= WAIT_ROOM_READY; i++){
            sprintf(buffer, "%s%d", SEM_BASE, i);
            if(SEMAPHORES[i] != SEM_FAILED && SEMAPHORES[i] != NULL && sem_close(SEMAPHORES[i]) == OP_ERROR){
                perror("Error while trying to close semaphore.");
            }
            if(SEMAPHORES[i] != SEM_FAILED && SEMAPHORES[i] != NULL && sem_unlink(buffer) == OP_ERROR){
                perror("Error while unlinking semaphore");
            }
        }
        free(SEMAPHORES);
    }
}

