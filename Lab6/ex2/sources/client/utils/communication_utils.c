#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <mqueue.h>
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

const char* FIXED_PATHNAME = "/servername";
char RANDOM_PATHNAME[9];
mqd_t SERVER_Q_ID = -1;
mqd_t OWN_Q = -1;
int CLIENT_ID = -1;

/*
*   Functions' declarations AREA
*/

int _initialize_comunication();
int _perform_communication_loop();
int _prepare_request(struct my_msgbuf *request);
int _process_one_line(struct my_msgbuf* buffer);
int _read_response();
void _reset_global_vars();
void _proceed_requests_from_file(struct my_msgbuf *request);
bool _compare_first_i_chars(const char* s1, const char* s2, int num);
void _perform_cl_end_response();
void _set_signals_handlers();
void _generate_random_path();
int _finish_login(struct my_msgbuf* buffer);

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
    struct my_msgbuf buffer;
    buffer.id = CLIENT_ID;
    buffer.mtype = (long)CL_END;
    if(SERVER_Q_ID != -1 && mq_send(SERVER_Q_ID, (char *)&buffer, sizeof(struct my_msgbuf), 1) == OP_ERROR){
        perror("Cannot communicate with server.");
    }
    if(SERVER_Q_ID != -1 && mq_close(SERVER_Q_ID) == OP_ERROR){
        perror("Error while trying to close own message queue.");
    }
    if(OWN_Q != -1 && mq_close(OWN_Q) == OP_ERROR){
        perror("Error while trying to close own message queue.");
    }
    if(OWN_Q != -1 && mq_unlink(RANDOM_PATHNAME) == OP_ERROR){
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
    if((SERVER_Q_ID = mq_open(FIXED_PATHNAME, O_WRONLY)) == OP_ERROR){
        perror("Error while establishing connection with server.");
        return OP_ERROR;
    }
    srand(time(NULL));
    _generate_random_path();
    struct mq_attr attrib;
    attrib.mq_msgsize = sizeof(struct my_msgbuf);
    attrib.mq_flags = 0;  
    attrib.mq_maxmsg = 10;  
    if((OWN_Q = mq_open(RANDOM_PATHNAME, O_CREAT | O_EXCL | O_RDONLY, 0773, &attrib)) == OP_ERROR){
        perror("Cannot open unique message queue.");
        return OP_ERROR;
    }
    return _initialize_comunication();
}

void _generate_random_path(){
    RANDOM_PATHNAME[0] = '/';
    for(int i = 1; i < 9; i++){
        RANDOM_PATHNAME[i] = 97 + rand() % 26;
    }
    RANDOM_PATHNAME[8] = '\0'; 
}

void _set_signals_handlers(){
    struct sigaction action;
    action.sa_handler = &_SIGINT_handler;
    sigfillset(&action.sa_mask); 
    action.sa_flags = SA_RESTART; 
    sigaction(SIGINT, &action, NULL); 
}

int _initialize_comunication(){
    struct my_msgbuf buffer;
    buffer.id = (int)getpid();
    buffer.mtype = (long)INIT;
    snprintf(buffer.mtext, MAX_MSG_TEXT_SIZE, "%s", RANDOM_PATHNAME);
    if(mq_send(SERVER_Q_ID, (char *)&buffer, sizeof(struct my_msgbuf), 1) == OP_ERROR){
        perror("Cannot communicate with server.");
        return OP_ERROR;
    }
    bool initBack = false;
    while(initBack == false){
        if(mq_receive(OWN_Q, (char *)&buffer, sizeof(struct my_msgbuf), NULL) == OP_ERROR){
            perror("Cannot read messages from server.");
            return OP_ERROR;
        } else if(buffer.mtype == (long)INIT) {
            initBack = true;
            return _finish_login(&buffer);
        }
    } 
    return OP_OK; 
}

int _finish_login(struct my_msgbuf* buffer){
    if(strcmp(buffer->mtext, "forbidden") == 0){
        printf("Error is to busy to serve another client.\n");
        return OP_ERROR;
    }
    CLIENT_ID = buffer->id;
    return _perform_communication_loop();
}

int _perform_communication_loop(){
    struct my_msgbuf buffer;
    buffer.id = CLIENT_ID; 
    printf("COMMUNICATION LOOP:\n");
    do {    
        if(OWN_Q != -1 && fgets(buffer.mtext, MAX_MSG_TEXT_SIZE, stdin) != NULL){
            if(_compare_first_i_chars(buffer.mtext, "QUIT", 4) == true){
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
        if(mq_send(SERVER_Q_ID, (char *)buffer, sizeof(struct my_msgbuf), 1) == OP_ERROR){
            return OP_ERROR;
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
    if(mq_receive(OWN_Q, (char *)&buffer, sizeof(struct my_msgbuf), NULL) != OP_ERROR){
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
    mq_send(SERVER_Q_ID, (char *)&buffer, sizeof(struct my_msgbuf), 1);
    if(mq_close(SERVER_Q_ID) == OP_ERROR){
        perror("Error while trying to close server queue.");
    }
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
            if(mq_send(SERVER_Q_ID, (char *)request, sizeof(struct my_msgbuf), 1) == OP_ERROR){
                fclose(file);
                return ;
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
