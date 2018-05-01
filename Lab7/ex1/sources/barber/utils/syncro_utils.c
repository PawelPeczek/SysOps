// #define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdbool.h>
#include <time.h>
#include "syncro_utils.h"
#include "../../global_utils/queue.h"


union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                       (Linux specific) */
};

#define ADDITIONAL_SEM_REQUIRED 7

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

int _set_signal_handlers();
int _initialize_shared_memory(ProgramInput* input);
int _initialize_semaphores(ProgramInput* input);
void _clean_IPC();
void _main_loop();
void _wait(int sem_no);
void _signal(int sem_no);
void _print_time_msg(const char* msg);
void _print_time_pid_msg(const char* msg, int pid);
void _let_all_waiting_on_BAR_CHK_STAT_go();

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
    if(atexit(&_clean_IPC) == OP_ERROR){
        perror("Error while setting atexit function.");
        return OP_ERROR;
    }
    return OP_OK;
}

int _initialize_shared_memory(ProgramInput* input){
    key_t key;
    if((key = ftok(PATH, PROJ_NO)) == OP_ERROR){
        perror("Error while generating key");
        return OP_ERROR;
    }
    if((QUEUE_MEM_ID = shmget(key, sizeof(Queue), IPC_CREAT | IPC_EXCL | 0774)) == OP_ERROR){
        perror("Error while creating queue shared memory segment.");
        return OP_ERROR;
    } 
    QUEUE = (Queue*)shmat(QUEUE_MEM_ID, NULL, 0);
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
    key_t key;
    if((key = ftok(PATH, PROJ_NO)) == OP_ERROR){
        perror("Error while generating key");
        return OP_ERROR;
    }
    BARBER_READY = input->queue_size;
    WAIT_ROOM_READY = BARBER_READY + 1;
    BARBER_SLEEPING = WAIT_ROOM_READY + 1;
    CLIPPING = BARBER_SLEEPING + 1;
    CLIENT_ACTION = CLIPPING + 1;
    BARBER_CHECKING_READY = CLIENT_ACTION + 1;
    BARBER_CHECKING_STATUS = BARBER_CHECKING_READY + 1;
    SEMAPHORE_SET_ID = semget(key, input->queue_size + ADDITIONAL_SEM_REQUIRED, IPC_CREAT | IPC_EXCL | 0774);
    if(SEMAPHORE_SET_ID == OP_ERROR){
        perror("Error while creating semaphore set.");
        return OP_ERROR;
    }
    union semun arg;
    arg.val = 0;
    for(int i = 0; i < input->queue_size; i++){
        if(semctl(SEMAPHORE_SET_ID, i, SETVAL, arg) == OP_ERROR){
            perror("Error while setting initial value of semaphore.");
            return OP_ERROR;
        }
    }
    if(semctl(SEMAPHORE_SET_ID, CLIPPING, SETVAL, arg) == OP_ERROR ||
       semctl(SEMAPHORE_SET_ID, CLIENT_ACTION, SETVAL, arg) == OP_ERROR ||
       semctl(SEMAPHORE_SET_ID, BARBER_SLEEPING, SETVAL, arg) == OP_ERROR){
        perror("Error while setting initial value of semaphore.");
        return OP_ERROR;
    }
    arg.val = 1;
    if(semctl(SEMAPHORE_SET_ID, BARBER_READY, SETVAL, arg) == OP_ERROR ||
       semctl(SEMAPHORE_SET_ID, WAIT_ROOM_READY, SETVAL, arg) == OP_ERROR ||
       semctl(SEMAPHORE_SET_ID, BARBER_CHECKING_READY, SETVAL, arg) == OP_ERROR ||
       semctl(SEMAPHORE_SET_ID, BARBER_CHECKING_STATUS, SETVAL, arg) == OP_ERROR){
        perror("Error while setting initial value of semaphore.");
        return OP_ERROR;
    }
    return OP_OK;
}

void _main_loop(){
    int pid;
    while(true){
        _print_time_msg("BARBER IS WAITING TO CHECK WR");
        _wait(BARBER_CHECKING_READY);
        QUEUE->barber_checking = true;
        // setting BARBER_CHECKING_STATUS on 0
        _wait(BARBER_CHECKING_STATUS);
        _signal(BARBER_CHECKING_READY);
        _wait(WAIT_ROOM_READY);
        _print_time_msg("BARBER STOPPED WAITING TO CHECK WR");
        if(QUEUE->current_load == 0){
            _wait(BARBER_READY);
            _print_time_msg("The barber is falling asleep");
            QUEUE->barber_sleeping = true;
            _signal(BARBER_READY);
            _signal(WAIT_ROOM_READY);
            _wait(BARBER_CHECKING_READY);
            QUEUE->barber_checking = false;
            _let_all_waiting_on_BAR_CHK_STAT_go();
            _signal(BARBER_CHECKING_READY);
            _wait(BARBER_SLEEPING);
            _print_time_msg("The barber is waking up");
            pid = QUEUE->instant_client;
            _signal(CLIPPING);
        } else {
            _signal(QUEUE->head);
            if(dequeue(QUEUE, &pid) == OP_ERROR){
                printf("Trying to dequeue from empty queue.");
                exit(5);
            }
            _print_time_pid_msg("The barber is asking a person from queue", pid);
            _signal(WAIT_ROOM_READY);
            _wait(BARBER_CHECKING_READY);
            QUEUE->barber_checking = false;
            _let_all_waiting_on_BAR_CHK_STAT_go();
            _signal(BARBER_CHECKING_READY);
            _signal(CLIPPING);
        }
        _wait(CLIENT_ACTION);
        _print_time_pid_msg("The barber is starting clipping.", pid);
        _print_time_pid_msg("The barber ended up clipping.", pid);
        _signal(CLIPPING);
        _wait(CLIENT_ACTION);
    }
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

void _let_all_waiting_on_BAR_CHK_STAT_go(){
    for(int i = 0; i <= QUEUE->waiting_counter; i++){
        _signal(BARBER_CHECKING_STATUS);
    }
    QUEUE->waiting_counter = 0;
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

void _clean_IPC(){
    if(QUEUE != NULL && shmdt(QUEUE) == OP_ERROR){
        perror("Error while closing shared memory segment.");
    }
    if(QUEUE_MEM_ID != -1 && shmctl(QUEUE_MEM_ID, IPC_RMID, NULL) == OP_ERROR){
        perror("Error while trying to delete shared memory segment.");
    }
    if(SEMAPHORE_SET_ID != -1 && semctl(SEMAPHORE_SET_ID, 0, IPC_RMID) == OP_ERROR){
        perror("Error while trying to delete semaphore set.");
    }
}

