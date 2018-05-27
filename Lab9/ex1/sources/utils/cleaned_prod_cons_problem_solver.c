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
int _initialize_all_mut_and_cond_vars();
int _safe_mutex_default_init(pthread_mutex_t mutex);
void _destroy_all_mutexes();
int _safe_cond_init(pthread_cond_t cond);
void _destroy_all_cond_vars();
void _safe_cond_destroy(pthread_cond_t cond);
void _clean_up();
void _safe_mutex_destroy(pthread_mutex_t mutex);
int _start_all_threads(const program_input * const input);
void _dealoate_ctrl_struct();
void _wait_for_end_condition(const program_input * const input);

void * producer_thread(void * dumb);
void _wait_until_buffer_full();
void _take_both_indicies();
void _give_both_indices();
void _read_line_and_save(int idx_to_operate);      
void _inform_one_consumer_if_now_buff_not_empty(); 
bool _wait_until_buffer_empty_and_decide_if_all_done();
void _inform_one_producer_if_now_buff_not_full();
bool _read_from_buffer_and_decide_if_terminate(int buff_idx);
void * consumer_thread(void * dumb);
void * _timer_thread(void * time_to_sleep);
void _inform_all_consumer_if_now_buff_not_empty();
void add_line_to_buffer(char * const line);
char * produce_line();
char * _consume_buffer();

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
    CTRL_STRUCT->mutexes = (pthread_mutex_t*) calloc(input->buff_size, sizeof(pthread_mutex_t));
    if(CTRL_STRUCT->mutexes == NULL){
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
    if(_initialize_all_mut_and_cond_vars() == -1){
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
    CTRL_STRUCT->elems_in_buff = 0;
    CTRL_STRUCT->producers_number = input->prod_numb;
    CTRL_STRUCT->consuments_number = input->consuments_numb;
}

int _initialize_all_mut_and_cond_vars(){
    for(int i = 0; i < CTRL_STRUCT->buff_size; i++){
        if(_safe_mutex_default_init(CTRL_STRUCT->mutexes[i]) == -1){
            _destroy_all_mutexes(CTRL_STRUCT);
            return -1;
        }
    }
    if(_safe_mutex_default_init(CTRL_STRUCT->buffer_empty_mutex) == -1 ||
       _safe_mutex_default_init(CTRL_STRUCT->buffer_full_mutex) == -1 ||
       _safe_mutex_default_init(CTRL_STRUCT->elems_mutex) == -1 ||
       _safe_mutex_default_init(CTRL_STRUCT->prod_mutex) == -1 ||
       _safe_mutex_default_init(CTRL_STRUCT->cons_mutex) == -1){
        _destroy_all_mutexes(CTRL_STRUCT);
        return -1;
    }
    if(_safe_cond_init(CTRL_STRUCT->buffer_full) == -1 ||
       _safe_cond_init(CTRL_STRUCT->buffer_empty) == -1){
        _destroy_all_mutexes(CTRL_STRUCT);
        _destroy_all_cond_vars(CTRL_STRUCT);
        return -1;
    }
    return 0;
}

int _safe_mutex_default_init(pthread_mutex_t mutex){
    if(pthread_mutex_init(&mutex, NULL) != 0){
        perror("Mutex initialization failed!\n");
        return -1;
    }
    return 0;
}

int _safe_cond_init(pthread_cond_t cond){
    if(pthread_cond_init(&cond, NULL) != 0){
        printf("Error while initializing conditional variable!\n");
        return -1;
    }
    return 0;
}

void _destroy_all_cond_vars(){
    _safe_cond_destroy(CTRL_STRUCT->buffer_full);
    _safe_cond_destroy(CTRL_STRUCT->buffer_empty);
}

void _safe_cond_destroy(pthread_cond_t cond){
    if(pthread_cond_destroy(&cond) != 0){
        printf("Error while destroying conditional variable!\n");
    }
}

void _destroy_all_mutexes(){
    for(int i = 0; i < CTRL_STRUCT->buff_size; i++){
        pthread_mutex_destroy(&(CTRL_STRUCT->mutexes[i]));
    }
    pthread_mutex_destroy(&(CTRL_STRUCT->buffer_empty_mutex));
    pthread_mutex_destroy(&(CTRL_STRUCT->buffer_full_mutex));
    pthread_mutex_destroy(&(CTRL_STRUCT->elems_mutex)); 
    pthread_mutex_destroy(&(CTRL_STRUCT->cons_mutex));
    pthread_mutex_destroy(&(CTRL_STRUCT->prod_mutex));
}

void _safe_mutex_destroy(pthread_mutex_t mutex){
    if(pthread_mutex_destroy(&mutex) != 0){
        perror("Mutex destruction failed!\n");
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
    if(CTRL_STRUCT->mutexes != NULL){
        _destroy_all_mutexes();
        free(CTRL_STRUCT->mutexes);
    }
    if(CTRL_STRUCT->buffer != NULL){
        for(int i = 0; i < CTRL_STRUCT->buff_size; i++){
            free(CTRL_STRUCT->buffer[i]);
        }
        free(CTRL_STRUCT->buffer);
    }
    _destroy_all_cond_vars();
    if(CTRL_STRUCT->source_file != NULL) {
        fclose(CTRL_STRUCT->source_file);
    }
    free(CTRL_STRUCT);
}

/*
*   THREADING AREA
*/

void * producer_thread(void * dumb){
    bool should_inform = false;
    while(true){
        char * produced_line = produce_line();
        if(produced_line == NULL){
            pthread_mutex_lock(&(CTRL_STRUCT->elems_mutex));
            CTRL_STRUCT->all_source_read ++;
            if(CTRL_STRUCT->all_source_read == CTRL_STRUCT->producers_number){
                should_inform = true;
            }
            pthread_mutex_unlock(&(CTRL_STRUCT->elems_mutex));
            if(should_inform == true){
                _inform_all_consumer_if_now_buff_not_empty();
            }
            break;
        } else {
            add_line_to_buffer(produced_line);
        }
    }        
    return (void *) NULL;
}

void add_line_to_buffer(char * const line){
    pthread_mutex_lock(&(CTRL_STRUCT->prod_mutex));
    _wait_until_buffer_full();
    pthread_mutex_lock(&(CTRL_STRUCT->mutexes[CTRL_STRUCT->prod_idx]));
    CTRL_STRUCT->buffer[CTRL_STRUCT->prod_idx] = line;
    pthread_mutex_lock(&(CTRL_STRUCT->elems_mutex));    
    CTRL_STRUCT->elems_in_buff ++;
    _inform_one_consumer_if_now_buff_not_empty();
    pthread_mutex_unlock(&(CTRL_STRUCT->elems_mutex));            
    pthread_mutex_unlock(&(CTRL_STRUCT->mutexes[CTRL_STRUCT->prod_idx]));
    pthread_mutex_unlock(&(CTRL_STRUCT->prod_mutex));
}

void _wait_until_buffer_full(){
    pthread_mutex_lock(&(CTRL_STRUCT->buffer_full_mutex));
    if(CTRL_STRUCT->elems_in_buff == CTRL_STRUCT->buff_size){
        pthread_cond_wait(&(CTRL_STRUCT->buffer_full), &(CTRL_STRUCT->buffer_full_mutex));
    }
    CTRL_STRUCT->prod_idx = (CTRL_STRUCT->prod_idx + 1) % CTRL_STRUCT->buff_size;
    pthread_mutex_unlock(&(CTRL_STRUCT->buffer_full_mutex));
}

void _inform_one_consumer_if_now_buff_not_empty(){
    pthread_mutex_lock(&(CTRL_STRUCT->buffer_empty_mutex));
    pthread_cond_signal(&(CTRL_STRUCT->buffer_empty));
    pthread_mutex_unlock(&(CTRL_STRUCT->buffer_empty_mutex));
}

void _inform_all_consumer_if_now_buff_not_empty(){
    pthread_mutex_lock(&(CTRL_STRUCT->buffer_empty_mutex));
    pthread_cond_broadcast(&(CTRL_STRUCT->buffer_empty));
    pthread_mutex_unlock(&(CTRL_STRUCT->buffer_empty_mutex));
}

char * produce_line(){
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
    pthread_mutex_lock(&(CTRL_STRUCT->cons_mutex));
    if(_wait_until_buffer_empty_and_decide_if_all_done() == true){
        pthread_mutex_unlock(&(CTRL_STRUCT->cons_mutex)); 
        return NULL;
    }
    pthread_mutex_lock(&(CTRL_STRUCT->mutexes[CTRL_STRUCT->cons_idx]));    
    char * line = CTRL_STRUCT->buffer[CTRL_STRUCT->cons_idx];
    CTRL_STRUCT->buffer[CTRL_STRUCT->cons_idx] = NULL;
    pthread_mutex_lock(&(CTRL_STRUCT->elems_mutex));    
    CTRL_STRUCT->elems_in_buff --;
    pthread_mutex_unlock(&(CTRL_STRUCT->elems_mutex));            
    pthread_mutex_unlock(&(CTRL_STRUCT->mutexes[CTRL_STRUCT->cons_idx]));
    _inform_one_producer_if_now_buff_not_full();        
    pthread_mutex_unlock(&(CTRL_STRUCT->cons_mutex));   
    return line;
}

bool _wait_until_buffer_empty_and_decide_if_all_done(){
    pthread_mutex_lock(&(CTRL_STRUCT->buffer_empty_mutex));
    if(CTRL_STRUCT->elems_in_buff == 0){
        if(CTRL_STRUCT->all_source_read == CTRL_STRUCT->producers_number){
            pthread_mutex_unlock(&(CTRL_STRUCT->buffer_empty_mutex));
            return true;
        }        
        pthread_cond_wait(&(CTRL_STRUCT->buffer_empty), &(CTRL_STRUCT->buffer_empty_mutex));
    }
    CTRL_STRUCT->cons_idx = (CTRL_STRUCT->cons_idx + 1) % CTRL_STRUCT->buff_size;
    pthread_mutex_unlock(&(CTRL_STRUCT->buffer_empty_mutex));
    return false;
}

void _inform_one_producer_if_now_buff_not_full(){
    pthread_mutex_lock(&(CTRL_STRUCT->buffer_full_mutex));
    pthread_cond_signal(&(CTRL_STRUCT->buffer_full));
    pthread_mutex_unlock(&(CTRL_STRUCT->buffer_full_mutex));
}

void _inform_all_producer_if_now_buff_not_full(){
    pthread_mutex_lock(&(CTRL_STRUCT->buffer_full_mutex));
    pthread_cond_broadcast(&(CTRL_STRUCT->buffer_full));
    pthread_mutex_unlock(&(CTRL_STRUCT->buffer_full_mutex));
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