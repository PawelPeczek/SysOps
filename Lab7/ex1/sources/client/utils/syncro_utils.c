// #define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include "syncro_utils.h"
#include "../../global_utils/queue.h"

const char* PATH = "/etc";
int QUEUE_MEM_ID = -1;
int SEMAPHORE_SET_ID = -1;
int WAIT_ROOM_READY;
int BARBER_READY;
int BARBER_SLEEPING;
int CLIPPING;
int CLIENT_ACTION;
int BARBER_CHECKING_READY;
int BARBER_CHECKING_STATUS;
Queue* QUEUE = NULL;

/*
*   Functions' declaration AREA
*/

int _get_shared_memory();
int _get_semaphores();
void _clean_IPC();
void _main_loop(ClientInput* input);
void _wait(int sem_no);
void _signal(int sem_no);
void _print_time_msg(const char* msg);
void _print_time_pid_msg(const char* msg, int pid);
void _proceed_service();

/*
*   Functions' declaration AREA END
*/

int client_loop(ClientInput* input){
    if(_get_shared_memory() == OP_ERROR ||
       _get_semaphores() == OP_ERROR ||
       atexit(&_clean_IPC) != OP_OK){
        return OP_ERROR;
    } 
    _main_loop(input);
    return OP_OK;
}


int _get_shared_memory(){
    key_t key;
    if((key = ftok(PATH, PROJ_NO)) == OP_ERROR){
        perror("Error while generating key");
        return OP_ERROR;
    }
    if((QUEUE_MEM_ID = shmget(key, 0, 0)) == OP_ERROR){
        perror("Error while creating queue shared memory segment.");
        return OP_ERROR;
    } 
    QUEUE = (Queue*)shmat(QUEUE_MEM_ID, NULL, 0);
    if((void *)QUEUE == (void *)OP_ERROR){
        perror("Error while fetching shared memory chunk");
        return OP_ERROR;
    }
    return OP_OK;
}

int _get_semaphores(){
    key_t key;
    if((key = ftok(PATH, PROJ_NO)) == OP_ERROR){
        perror("Error while generating key");
        return OP_ERROR;
    }
    BARBER_READY = QUEUE->size;
    WAIT_ROOM_READY = BARBER_READY + 1;
    BARBER_SLEEPING = WAIT_ROOM_READY + 1;
    CLIPPING = BARBER_SLEEPING + 1;
    CLIENT_ACTION = CLIPPING + 1;
    BARBER_CHECKING_READY = CLIENT_ACTION + 1;
    BARBER_CHECKING_STATUS = BARBER_CHECKING_READY + 1;
    if((SEMAPHORE_SET_ID = semget(key, 0, 0)) == OP_ERROR){
        perror("Error while creating semaphore set.");
        return OP_ERROR;
    }
    return OP_OK;
}

void _main_loop(ClientInput* input){
    while(input->no_clipping > 0){
        _wait(BARBER_CHECKING_READY);
        if(QUEUE->barber_checking == true){
            QUEUE->waiting_counter++;
            _signal(BARBER_CHECKING_READY);
            _wait(BARBER_CHECKING_STATUS);
        } else {
            _signal(BARBER_CHECKING_READY);
        }
        //_print_time_msg("WAIT FOR WAIT_ROOM_READY");
        _wait(WAIT_ROOM_READY);
        //_print_time_msg("WAIT FOR BARBER_READY");
        _wait(BARBER_READY);
        if(QUEUE->barber_sleeping == true){
            QUEUE->instant_client = (int)getpid();
            _signal(WAIT_ROOM_READY);
            _print_time_msg("Client is trying to walk up the barber");
            QUEUE->barber_sleeping = false;
            _signal(BARBER_READY);
            _signal(BARBER_SLEEPING);
            //_print_time_msg("WAIT FOR CLIPPING");
            _wait(CLIPPING);
            _proceed_service();
            input->no_clipping--;
        } else if (QUEUE->current_load != QUEUE->size){
            // sitting in waiting room
            _signal(BARBER_READY);
            int ticket;
            if(enqueue(QUEUE, (int)getpid(), &ticket) == OP_ERROR){
                printf("Error while enqueueing client.\n");
                exit(3);
            }
            _print_time_msg("Taking a seat in waiting room.");
            _signal(WAIT_ROOM_READY);
            //_print_time_msg("WAIT FOR ticket");
            _wait(ticket);
            // service
            _wait(CLIPPING);
            _proceed_service();
            input->no_clipping--;
        } else {
            // not served -> going away
            _signal(BARBER_READY);
            _print_time_msg("No chairs in waiting room. Going away without being served.");
            _signal(WAIT_ROOM_READY);
        }
    }
}

void _proceed_service(){
    _print_time_msg("Client is sitting on a chair");
    _signal(CLIENT_ACTION);
    _wait(CLIPPING);
    _print_time_msg("Client is serverd and going away.");
    _signal(CLIENT_ACTION);
}

void _wait(int sem_no){
    struct sembuf buf;
    buf.sem_flg = 0;
    buf.sem_num = sem_no;
    buf.sem_op = -1;
    if(semop(SEMAPHORE_SET_ID, &buf, 1) == OP_ERROR){
        perror("Error while waiting at semaphore.");
        exit(3);
    }
}

void _signal(int sem_no){
    struct sembuf buf;
    buf.sem_flg = 0;
    buf.sem_num = sem_no;
    buf.sem_op = 1;
    if(semop(SEMAPHORE_SET_ID, &buf, 1) == OP_ERROR){
        perror("Error while signal the semaphore.");
        exit(4);
    }
}

void _print_time_msg(const char* msg){
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("%ld\t%ld\t%s.<%d>\n", tp.tv_sec, tp.tv_nsec, msg, (int)getpid());
}

void _clean_IPC(){
    if(QUEUE != NULL && shmdt(QUEUE) == OP_ERROR){
        perror("Error while closing shared memory segment.");
    }
}

