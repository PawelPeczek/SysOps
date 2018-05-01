// #define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>
#include <stdio.h>
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

int _get_shared_memory();
int _get_semaphores();
void _clean_POSIX();
void _main_loop(ClientInput* input);
void _wait(int sem_no);
void _signal(int sem_no);
void _print_time_msg(const char* msg);
void _print_time_pid_msg(const char* msg, int pid);
bool _proceed_service();

/*
*   Functions' declaration AREA END
*/

int client_loop(ClientInput* input){
    if(_get_shared_memory() == OP_ERROR ||
       _get_semaphores() == OP_ERROR ||
       atexit(&_clean_POSIX) != OP_OK){
        return OP_ERROR;
    } 
    _main_loop(input);
    return OP_OK;
}


int _get_shared_memory(){
    if((QUEUE_MEM_ID = shm_open(PATH, O_RDWR, 0)) == OP_ERROR){
        perror("Error while creating queue shared memory segment.");
        return OP_ERROR;
    } 
    QUEUE = (Queue*)mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, QUEUE_MEM_ID, 0);
    if((void *)QUEUE == (void *)OP_ERROR){
        perror("Error while fetching shared memory chunk");
        return OP_ERROR;
    }
    return OP_OK;
}

int _get_semaphores(){
    SEMAPHORES = (sem_t**)calloc(QUEUE->size + ADDITIONAL_SEM_REQUIRED, sizeof(sem_t*));
    if(SEMAPHORES == NULL){
        perror("Error while alocating memory.");
        return OP_ERROR;
    }   
    CLIPPING = QUEUE->size;
    CLIENT_ACTION = CLIPPING + 1;
    BARBER_SLEEPING = CLIENT_ACTION + 1;
    BARBER_READY = BARBER_SLEEPING + 1;
    WAIT_ROOM_READY = BARBER_READY + 1;
    char buffer[32];
    for(int i = 0; i <= WAIT_ROOM_READY; i++){
        sprintf(buffer, "%s%d", SEM_BASE, i);
        SEMAPHORES[i] = sem_open(buffer, O_RDWR);
        if(SEMAPHORES[i] == SEM_FAILED){
            perror("Error while setting initial value of semaphore.");
            return OP_ERROR;
        }
    }
    return OP_OK;
}

void _main_loop(ClientInput* input){
    bool err_flag = true;
    while(input->no_clipping > 0){
        //_print_time_msg("WAIT FOR WAIT_ROOM_READY");
        WAIT(SEMAPHORES[WAIT_ROOM_READY]);
        //_print_time_msg("WAIT FOR BARBER_READY");
        WAIT(SEMAPHORES[BARBER_READY]);
        if(QUEUE->barber_sleeping == true){
            QUEUE->instant_client = (int)getpid();
            SIGNAL(SEMAPHORES[WAIT_ROOM_READY]);
            _print_time_msg("Client is trying to walk up the barber");
            QUEUE->barber_sleeping = false;
            SIGNAL(SEMAPHORES[BARBER_READY]);
            SIGNAL(SEMAPHORES[BARBER_SLEEPING]);
            //_print_time_msg("WAIT FOR CLIPPING");
            WAIT(SEMAPHORES[CLIPPING]);            
            if(_proceed_service() == false) {
                break;
            }
            input->no_clipping--;
        } else if (QUEUE->current_load != QUEUE->size){
            // sitting in waiting room
            SIGNAL(SEMAPHORES[BARBER_READY]);
            int ticket;
            if(enqueue(QUEUE, (int)getpid(), &ticket) == OP_ERROR){
                printf("Error while enqueueing client.\n");
                exit(3);
            }
            _print_time_msg("Taking a seat in waiting room.");
            SIGNAL(SEMAPHORES[WAIT_ROOM_READY]);
            //_print_time_msg("WAIT FOR ticket");
            WAIT(SEMAPHORES[ticket]);
            // service
            WAIT(SEMAPHORES[CLIPPING]);
            if(_proceed_service() == false) {
                break;
            }
            input->no_clipping--;
        } else {
            // not served -> going away
            SIGNAL(SEMAPHORES[BARBER_READY]);
            _print_time_msg("No chairs in waiting room. Going away without being served.");
            SIGNAL(SEMAPHORES[WAIT_ROOM_READY]);
        }
        if(input->no_clipping == 0){
            // deactivation error flag if execution reaches this point at last iteration
            err_flag = false;
        }
    }
    if(err_flag == true){
        perror("Error while proceeding client loop.");
    }
}

bool _proceed_service(){
    do{
        _print_time_msg("Client is sitting on a chair");
        SIGNAL(SEMAPHORES[CLIENT_ACTION]);
        WAIT(SEMAPHORES[CLIPPING]);
        _print_time_msg("Client is serverd and going away.");
        SIGNAL(SEMAPHORES[CLIENT_ACTION]);
        return true;
    } while(true);
    return false;
}

void _print_time_msg(const char* msg){
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("%ld\t%ld\t%s.<%d>\n", tp.tv_sec, tp.tv_nsec, msg, (int)getpid());
}

void _clean_POSIX(){
    if(QUEUE != NULL && munmap(QUEUE, sizeof(Queue)) == OP_ERROR){
        perror("Error while closing shared memory segment.");
    }
    if(SEMAPHORES != NULL){
        for(int i = 0; i <= WAIT_ROOM_READY; i++){
            if(SEMAPHORES[i] != SEM_FAILED && SEMAPHORES[i] != NULL && sem_close(SEMAPHORES[i]) == OP_ERROR){
                perror("Error while trying to close semaphore.");
            }
        }
        free(SEMAPHORES);
    }
}

