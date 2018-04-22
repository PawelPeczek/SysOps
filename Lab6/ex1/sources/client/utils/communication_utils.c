#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pwd.h>
#include "communication_utils.h"
#include "../../headers/contract.h"
#include "../../headers/my_errors.h"

const char* FIXED_PATHNAME = "/etc";
int SERVER_Q_ID = -1;
int OWN_Q = -1;
static const int MAX_NO_TRIES = 256;
int CLIENT_ID = -1;

/*
*   Functions' declarations AREA
*/

int _initialize_comunication(key_t key);
int _perform_communication_loop();
int _prepare_request(struct my_msgbuf *request);
int _process_one_line(struct my_msgbuf* buffer);
int _read_response();
void _reset_global_vars();
void _proceed_requests_from_file(struct my_msgbuf *request);
bool _compare_first_i_chars(const char* s1, const char* s2, int num);
void _perform_cl_end_response();
void _set_signals_handlers();
void _check_pending_CL_STOP();

/*
*   Functions' declarations AREA END
*/

/*
*   Signals' handlers
*/

void at_exit_handler(){
    if(SERVER_Q_ID == -1 || OWN_Q == -1){
        return ;
    }
    _check_pending_CL_STOP();
    struct my_msgbuf buffer;
    buffer.id = CLIENT_ID;
    buffer.mtype = (long)CL_END;
    if(SERVER_Q_ID != -1 && msgsnd(SERVER_Q_ID, (void *)&buffer, sizeof(struct my_msgbuf) - sizeof(long), 0) == OP_ERROR){
        perror("Cannot communicate with server.");
    }
    if(OWN_Q != -1 && msgctl(OWN_Q, IPC_RMID, NULL) == OP_ERROR){
        perror("Error while trying to close own message queue.");
    }
}

void _SIGINT_handler(int signo){
    at_exit_handler();
    _reset_global_vars();
    exit(0);
}

/*
*   Signals' handlers END
*/


int establish_communication(){
    atexit(at_exit_handler);
    _set_signals_handlers();
    key_t key = ftok(FIXED_PATHNAME, FIXED_PROJECT_CODE);
    if((SERVER_Q_ID = msgget(key, 0)) == OP_ERROR){
        perror("Error while establishing connection with server.");
        return OP_ERROR;
    }
    srand(time(NULL));
    int gen = rand() % MAX_NO_TRIES;
    int tries = 0;
    struct passwd *pw = getpwuid(getuid());
    key_t priv_key = ftok(pw->pw_dir, gen);
    while(tries < MAX_NO_TRIES && (OWN_Q = msgget(priv_key, IPC_CREAT | IPC_EXCL | 
        S_IRWXU | S_IRWXG | S_IXGRP | S_IWOTH | S_IXOTH)) == OP_ERROR){
        gen = (gen + 1) % MAX_NO_TRIES;
        priv_key = ftok(pw->pw_dir, gen);
        tries++;
    }
    if(tries == MAX_NO_TRIES){
        perror("Cannot open unique message queue.");
        return OP_ERROR;
    }
    return _initialize_comunication(priv_key);
}

void _set_signals_handlers(){
    struct sigaction action;
    action.sa_handler = &_SIGINT_handler;
    sigfillset(&action.sa_mask); 
    action.sa_flags = SA_RESTART; 
    sigaction(SIGINT, &action, NULL); 
}

int _initialize_comunication(key_t key){
    struct my_msgbuf buffer;
    buffer.id = (int)getpid();
    buffer.mtype = (long)INIT;
    snprintf(buffer.mtext, MAX_MSG_TEXT_SIZE, "%d", key);
    if(msgsnd(SERVER_Q_ID, (void *)&buffer, sizeof(struct my_msgbuf) - sizeof(long), 0) == OP_ERROR){
        perror("Cannot communicate with server.");
        return OP_ERROR;
    }
    if(msgrcv(OWN_Q, (void *)&buffer, sizeof(struct my_msgbuf) - sizeof(long), (long)INIT, 0) == OP_ERROR){
        perror("Cannot read messages from server.");
        return OP_ERROR;
    }
    if(strcmp(buffer.mtext, "forbidden") == 0){
        printf("Error is to busy to serve another client.\n");
        return OP_ERROR;
    }
    CLIENT_ID = buffer.id;
    return _perform_communication_loop();
}

int _perform_communication_loop(){
    struct my_msgbuf buffer;
    buffer.id = CLIENT_ID; 
    printf("COMMUNICATION LOOP:\n");
    do {    
        if(OWN_Q != -1 && fgets(buffer.mtext, MAX_MSG_TEXT_SIZE, stdin) != NULL){
            if(_compare_first_i_chars(buffer.mtext, "QUIT", 4) == true){
                _check_pending_CL_STOP();
                break;
            } 
            if(_process_one_line(&buffer) == OP_ERROR){
                perror("Error while processing command line request.");
                return OP_ERROR;
            }
        } else {
            break;
        }
    } while(true);
    return OP_OK;
}

int _process_one_line(struct my_msgbuf* buffer){
    if(_prepare_request(buffer) == OP_OK){
        if(msgsnd(SERVER_Q_ID, (void *)buffer, sizeof(struct my_msgbuf) - sizeof(long), 0) == OP_ERROR){
            return OP_ERROR;
        } else {
            _check_pending_CL_STOP();
        }
        int op_status = _read_response();
        if (op_status == OP_ERROR){
            return OP_ERROR;
        }
    }
    return OP_OK;
}

int _prepare_request(struct my_msgbuf *request){
    if(_compare_first_i_chars(request->mtext, "MIRROR", 6) == true) {
        request->mtype = (long)MIRROR;
    } else if(_compare_first_i_chars(request->mtext, "CALC", 4) == true){
        request->mtype = (long)CALC;
    } else if (_compare_first_i_chars(request->mtext, "TIME", 4) == true){
        request->mtype = (long)TIME;
    } else if(_compare_first_i_chars(request->mtext, "END", 3) == true) {
        request->mtype = (long)END;
    } else if (_compare_first_i_chars(request->mtext, "FILE", 4) == true) {
        _proceed_requests_from_file(request);
        return OP_ERROR;
    } else {
        printf("Undefined request.\n");
        return OP_ERROR;
    }
    return OP_OK;
}

bool _compare_first_i_chars(const char* s1, const char* s2, int num){
    for(int i = 0; i < num; i++){
        if(s1[i] != s2[i] || s1[i] == '\0' || s2[i] == '\0'){
            return false;
        }
    }
    return true;
}

int _read_response(){
    if(OWN_Q == -1) {
        return OP_ERROR;
    }
    struct my_msgbuf buffer;
    if(msgrcv(OWN_Q, (void *)&buffer, sizeof(struct my_msgbuf) - sizeof(long), 0, 0) != OP_ERROR){
        if(buffer.mtype == (long)CL_END){
            _perform_cl_end_response();
            return OP_ERROR;
        } else {
            printf("Response from server: %s\n", buffer.mtext);
            return OP_OK;
        }
    } else {
        perror("Error while reading server message.");
        return OP_ERROR;
    }
}

void _perform_cl_end_response(){
    struct my_msgbuf buffer;
    buffer.id = CLIENT_ID;
    buffer.mtype = (long)CL_ACK;
    msgsnd(SERVER_Q_ID, (void *)&buffer, sizeof(struct my_msgbuf) - sizeof(long), 0);
    msgctl(OWN_Q, IPC_RMID, NULL);
    _reset_global_vars();
}

void _reset_global_vars(){
    SERVER_Q_ID = -1;
    OWN_Q = -1;
    CLIENT_ID = -1;
}

void _proceed_requests_from_file(struct my_msgbuf *request){
    int start_idx = 4;
    while(request->mtext[start_idx] == ' '){
        start_idx++;
    }
    for(int i = 0; request->mtext[i] != '\0'; i++){
        if(request->mtext[i] == 10 || request->mtext[i] == 13){
            request->mtext[i] = '\0'; 
        }
    }
    FILE* file = fopen(request->mtext + start_idx, "r");
    if(file == NULL){
        perror("Cannot open file");
        return ;
    }
    request->id = CLIENT_ID;
    while(fgets(request->mtext, MAX_MSG_TEXT_SIZE, file) != NULL){
        printf("%s\n", request->mtext);
        if(_prepare_request(request) == OP_OK){
            if(msgsnd(SERVER_Q_ID, (void *)request, sizeof(struct my_msgbuf) - sizeof(long), 0) == OP_ERROR){
                fclose(file);
                return ;
             } else {
                _check_pending_CL_STOP();
            }
            int op_status = _read_response();
            if (op_status == OP_ERROR){
                fclose(file);
                return ;
            }
        }
    }
    fclose(file);
}

void _check_pending_CL_STOP(){
    struct my_msgbuf buffer;
    if(msgrcv(OWN_Q, (void *)&buffer, sizeof(struct my_msgbuf) - sizeof(long), (long)CL_END, IPC_NOWAIT) != OP_ERROR){
        _perform_cl_end_response();
    }
}