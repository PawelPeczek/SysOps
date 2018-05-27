#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "./prod_cons_problem_solver.h"

/*
* GLOBAL VARS
*/

threading_control_struct * CTRL_STRUCT = NULL;
pthread_t * THREADS = NULL;
int THREADS_NO = 0;

/*
* GLOBAL VARS END
*/

/*
* MACROS
*/

#define REPORT_IF_VERBOSE(type, thread, message)({\
        if((CTRL_STRUCT->verbose_mode) == true){\
            printf("%s <%ld> %s\n", (type), (thread), (message));\
        }\
    })

#define REPORT_BUFFER_INDEX_IF_VERBOSE(type, thread, idx)({\
        if((CTRL_STRUCT->verbose_mode) == true){\
            printf("%s <%ld> will be operating on buffer idx: %d\n", (type), (thread), (idx));\
        }\
    })


/*
* MACROS END
*/

/*
*   Functions' definitions area
*/

int _initialize_thr_ctrl_struct(const program_input * const input);
void _initialize_parameters(const program_input * const input);
int _initialize_all_sem();
int _safe_sem_default_init(sem_t * sem);
void _destroy_all_semaphores();
void _clean_up();
void _safe_sem_destroy(sem_t sem);
int _start_all_threads(const program_input * const input);
void _dealoate_ctrl_struct();
void _wait_for_end_condition(const program_input * const input);

void * producer_thread(void * dumb);
void _wait_until_buffer_full();    
void * consumer_thread(void * dumb);
void * _timer_thread(void * time_to_sleep);
void _wake_up_all_suspended_consumer();
void _add_line_to_buffer(char * const line);
char * _produce_line();
char * _consume_buffer();
bool _is_something_to_consume();
void _sem_pos_to_given_value(sem_t * sem, int val, const char * who, const char * desc);

/*
*   Functions' definitions area END
*/

int solve_prod_cons_problem(const program_input * const input){
    if(atexit(&_clean_up) != 0){
        perror("atexit.");
        return -1;
    }
    if(_initialize_thr_ctrl_struct(input) == -1){
        return -1;
    }
    free(input->file_name);
    THREADS_NO = input->prod_numb + input->consuments_numb + 1;
    THREADS = (pthread_t *) calloc(THREADS_NO, sizeof(pthread_t));
    if(THREADS == NULL){
        return -1;
    }
    if(_start_all_threads(input) == -1){
        return -1;
    }
    _wait_for_end_condition(input);
    return 0;
}

void _wait_for_end_condition(const program_input * const input){
    if(input->nk == 0){
        for(int i = 0; i < THREADS_NO - 1; i++){
            pthread_join(THREADS[i], NULL);
        }
    } else {
        // timer is going to stop everything
        pause();
    }
}

int _initialize_thr_ctrl_struct(const program_input * const input){
    printf("In _initialize_thr_ctrl_struct\n");
    CTRL_STRUCT = (threading_control_struct *)calloc(1, sizeof(threading_control_struct));
    CTRL_STRUCT->buffer = (char **)calloc(input->buff_size, sizeof(char *));
    if(CTRL_STRUCT->buffer == NULL){
        perror("Error while memory allocation.");        
        free(input->file_name);
        return -1;
    }
    _initialize_parameters(input);
    CTRL_STRUCT->semaphores = (sem_t *) calloc(input->buff_size, sizeof(sem_t));
    if(CTRL_STRUCT->semaphores == NULL){
        perror("Error while memory allocation.");        
        free(input->file_name);
        free(CTRL_STRUCT->buffer);
        return -1;
    }
    printf("Trying to open file %s\n", input->file_name);
    FILE * source = fopen(input->file_name, "r");
    if(source == NULL){
        perror("Error while opening source file!");
        free(input->file_name);
        free(CTRL_STRUCT->buffer);
        return -1;
    }
    CTRL_STRUCT->source_file = source;
    if(_initialize_all_sem() == -1){
        perror("Error while mutex and cond init.");
        free(input->file_name);
        free(CTRL_STRUCT->buffer);
        fclose(CTRL_STRUCT->source_file);
        return -1;
    }
    return 0;
}

void _initialize_parameters(const program_input * const input){
    printf("In _initialize_parameters\n");
    CTRL_STRUCT->prod_idx = 0;
    CTRL_STRUCT->cons_idx = 0;
    CTRL_STRUCT->all_source_read = 0;
    CTRL_STRUCT->verbose_mode= input->verbose;
    CTRL_STRUCT->compare_fun = input->compare_fun;
    CTRL_STRUCT->compare_value = input->compare_value;
    CTRL_STRUCT->buff_size = input->buff_size;
    CTRL_STRUCT->consuments_number = input->consuments_numb;
    CTRL_STRUCT->producers_number = input->prod_numb;
}

int _initialize_all_sem(){
    for(int i = 0; i < CTRL_STRUCT->buff_size; i++){
        if(_safe_sem_default_init(&(CTRL_STRUCT->semaphores[i])) == -1){
            _destroy_all_semaphores(CTRL_STRUCT);
            return -1;
        }
    }
    if(_safe_sem_default_init(&(CTRL_STRUCT->all_source_read_sem)) == -1 ||
       _safe_sem_default_init(&(CTRL_STRUCT->cons_sem)) == -1 ||
       _safe_sem_default_init(&(CTRL_STRUCT->prod_sem)) == -1){
        _destroy_all_semaphores();
        return -1;
    }
    if(sem_init(&(CTRL_STRUCT->empty_count_sem), 0, CTRL_STRUCT->buff_size) != 0 ||
       sem_init(&(CTRL_STRUCT->fill_count_sem), 0, 0) != 0){
        perror("Error while creating semaphore");
        _destroy_all_semaphores();
        return -1;
    }
    return 0;
}

int _safe_sem_default_init(sem_t * sem){
    if(sem_init(sem, 0, 1) != 0){
        perror("Semaphore initialization failed!\n");
        return -1;
    }
    return 0;
}


void _destroy_all_semaphores(){
    for(int i = 0; i < CTRL_STRUCT->buff_size; i++){
        _safe_sem_destroy(CTRL_STRUCT->semaphores[i]);
    }
    _safe_sem_destroy(CTRL_STRUCT->prod_sem);
    _safe_sem_destroy(CTRL_STRUCT->cons_sem);
    _safe_sem_destroy(CTRL_STRUCT->fill_count_sem); 
    _safe_sem_destroy(CTRL_STRUCT->empty_count_sem);
    _safe_sem_destroy(CTRL_STRUCT->all_source_read_sem);
}


void _safe_sem_destroy(sem_t sem){
    if(sem_destroy(&sem) != 0){
        perror("Semaphore destruction failed!\n");
    }
}

int _start_all_threads(const program_input * const input){
    for(int i = 0; i < input->prod_numb; i ++){
        if(pthread_create(THREADS + i, NULL, &producer_thread, (void *)NULL) == -1){
            perror("Error while creating thread");
            return -1;
        }
    }
    int upper_bound = input->consuments_numb + input->prod_numb;
    for(int i = input->prod_numb; i < upper_bound; i ++){
        if(pthread_create(THREADS + i, NULL, &consumer_thread, (void *)NULL) == -1){
            perror("Error while creating thread");
            return -1;
        }                      
    }
    if(input->nk > 0 && pthread_create(THREADS + upper_bound, NULL, &_timer_thread, (void *)&(input->nk)) == -1){
        perror("Error while creating thread");
        return -1;
    }
    return 0;
}

void _clean_up(){
    printf("_clean_up() fired!\n");
    if(CTRL_STRUCT != NULL){
        _dealoate_ctrl_struct();
    }
    free(THREADS);
}

void _dealoate_ctrl_struct(){
    if(CTRL_STRUCT->semaphores != NULL){
        _destroy_all_semaphores();
        free(CTRL_STRUCT->semaphores);
    }
    if(CTRL_STRUCT->buffer != NULL){
        for(int i = 0; i < CTRL_STRUCT->buff_size; i++){
            free(CTRL_STRUCT->buffer[i]);
        }
        free(CTRL_STRUCT->buffer);
    }
    if(CTRL_STRUCT->source_file != NULL) {
        fclose(CTRL_STRUCT->source_file);
    }
    free(CTRL_STRUCT);
}

void * producer_thread(void * dumb){
    while(true){
        char * produced_line = _produce_line();
        if(produced_line == NULL){
            sem_wait(&(CTRL_STRUCT->all_source_read_sem));
            CTRL_STRUCT->all_source_read ++;
            if(CTRL_STRUCT->all_source_read == CTRL_STRUCT->producers_number){
                _wake_up_all_suspended_consumer();
            }
            sem_post(&(CTRL_STRUCT->all_source_read_sem));
            break;
        } else {
            _add_line_to_buffer(produced_line);  
        }
    }        
    return (void *) NULL;
}

void _add_line_to_buffer(char * const line){
    sem_wait(&(CTRL_STRUCT->empty_count_sem));
    sem_wait(&(CTRL_STRUCT->prod_sem));             
    CTRL_STRUCT->prod_idx = (CTRL_STRUCT->prod_idx + 1) % CTRL_STRUCT->buff_size;  
    sem_wait(&(CTRL_STRUCT->semaphores[CTRL_STRUCT->prod_idx]));
    CTRL_STRUCT->buffer[CTRL_STRUCT->prod_idx] = line;
    sem_post(&(CTRL_STRUCT->semaphores[CTRL_STRUCT->prod_idx]));
    sem_post(&(CTRL_STRUCT->prod_sem));
    sem_post(&(CTRL_STRUCT->fill_count_sem));
}

void _wake_up_all_suspended_consumer(){
    _sem_pos_to_given_value(&(CTRL_STRUCT->fill_count_sem), CTRL_STRUCT->consuments_number ,"[PRODUCER]", "signalling suspended consumers.");
}

void _sem_pos_to_given_value(sem_t * sem, int val, const char * who, const char * desc){
    int curr_sem_val;
    sem_getvalue(sem, &curr_sem_val);
    while(curr_sem_val < CTRL_STRUCT->consuments_number){
        sem_post(sem);
        sem_getvalue(sem, &curr_sem_val);
    }
}

char * _produce_line(){
    char * line_buffer;
    size_t size = 0;
    int res = getline(&line_buffer, &size, CTRL_STRUCT->source_file);
    if(res == -1){
        return NULL;
    } else {
        return line_buffer;
    }
}    


void * consumer_thread(void * dumb){
    while(true){
        char * line = _consume_buffer();
        if(line == NULL){
            break;
        } else if(CTRL_STRUCT->compare_fun(strlen(line), CTRL_STRUCT->compare_value) == true) {
            printf("MATCHED LINE: <%ld> %s", pthread_self(), line);
        }
        free(line);
    }
    return (void *) NULL;
}

char * _consume_buffer(){
    if(_is_something_to_consume() == false){
        return NULL;
    }
    sem_wait(&(CTRL_STRUCT->fill_count_sem));
    sem_wait(&(CTRL_STRUCT->cons_sem));      
    CTRL_STRUCT->cons_idx = (CTRL_STRUCT->cons_idx + 1) % CTRL_STRUCT->buff_size;
    sem_wait(&(CTRL_STRUCT->semaphores[CTRL_STRUCT->cons_idx]));    
    char * line = CTRL_STRUCT->buffer[CTRL_STRUCT->cons_idx];
    CTRL_STRUCT->buffer[CTRL_STRUCT->cons_idx] = NULL;
    sem_post(&(CTRL_STRUCT->semaphores[CTRL_STRUCT->cons_idx]));
    sem_post(&(CTRL_STRUCT->cons_sem));   
    sem_post(&(CTRL_STRUCT->empty_count_sem));   
    return line;
}

bool _is_something_to_consume(){
    sem_wait(&(CTRL_STRUCT->all_source_read_sem));
    int fill_count_sem_val;
    sem_getvalue(&(CTRL_STRUCT->fill_count_sem), &fill_count_sem_val);
    if(CTRL_STRUCT->all_source_read == CTRL_STRUCT->producers_number && fill_count_sem_val == 0){
        sem_post(&(CTRL_STRUCT->all_source_read_sem));
        sem_post(&(CTRL_STRUCT->fill_count_sem));      
        return false;
    }      
    sem_post(&(CTRL_STRUCT->all_source_read_sem));
    return true;
}
 
void * _timer_thread(void * time_to_sleep){
    sleep(*((int *)time_to_sleep));
    printf("Timer is terminationg a process...\n");
    exit(13);
    return (void *) NULL;
}


/*
*   THREADING AREA END
*/