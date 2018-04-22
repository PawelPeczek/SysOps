#define _GNU_SOURCE
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "communication_utils.h"
#include "message_utils.h"
#include "../../headers/contract.h"
#include "../../headers/my_errors.h"

const char* FIXED_PATHNAME = "/servername";
mqd_t PUBLIC_SRV_QID = -1;
int CL_ACK_RECEIVED = 0;
int CL_ACK_REQUIRED = 1;
bool CLOSE_PROCESS_IN_PROGRESS = false;
int CLIENTS_ITERATOR = 0;
Clients* CLIENTS = NULL;


/*
*   Functions' definition AREA
*/

int _retry_open_q();
int _allocc_clients_struct();
void _free_clients_struct();
int _handle_clients_communication();
void _handle_client_request(struct my_msgbuf* request);
bool _is_number_valid(int id);
bool _request_needs_response(long type);
void _dispatch_request(struct my_msgbuf* request, struct my_msgbuf* response);
int _is_free_space_available();
void _handle_init_request(struct my_msgbuf* request, struct my_msgbuf* response);
void _handle_cl_end_request(struct my_msgbuf* request);
void _handle_end_request(struct my_msgbuf* request, struct my_msgbuf* response);
void _handle_cl_ack_response(struct my_msgbuf* request);
void _handle_undefined_request(struct my_msgbuf* response);
void _broadcast_cl_edn();
void _close_all_client_queries();
void _set_signals_handlers();

/*
*   Functions' definition AREA END
*/


/*
*   Signals handlers
*/

void _SIGINT_handler(int signo){
    if(CLIENTS == NULL){
        return ;
    }
    CL_ACK_RECEIVED = 0;
    int num_of_cl_ack_req = 0;
    for(int i = 0; i < CLIENTS_MAXNUM; i++){
        if(CLIENTS->slot_free[i] == false){
            num_of_cl_ack_req++;
        }
    }
    CL_ACK_REQUIRED = num_of_cl_ack_req;
    CLOSE_PROCESS_IN_PROGRESS = true;
    struct my_msgbuf buffer;
    _broadcast_cl_edn();
    while(CL_ACK_RECEIVED < CL_ACK_REQUIRED){
        if(mq_receive(PUBLIC_SRV_QID, (char *)&buffer, sizeof(struct my_msgbuf), NULL) != OP_ERROR){
            if(buffer.mtype == CL_ACK){
                CLIENTS->slot_free[buffer.id] = true;
                CLIENTS->clents_qs_ids[buffer.id] = 0;
                CL_ACK_RECEIVED++;
            }
        }
    }
    if(mq_close(PUBLIC_SRV_QID) == OP_ERROR) {
        perror("Cannot close server queue");
    }
    if(mq_unlink(FIXED_PATHNAME) == OP_ERROR){
        perror("Cannot unlink server queue");
    }
    _free_clients_struct();
    exit(0);
}

void _exit_handler(){
    _SIGINT_handler(0);
}


/*
*   Signals handlers END
*/


int establish_msgq(){
    _set_signals_handlers();
    struct mq_attr attrib;
    attrib.mq_msgsize = sizeof(struct my_msgbuf);
    attrib.mq_flags = 0;  
    attrib.mq_maxmsg = 10;  
    if((PUBLIC_SRV_QID = mq_open(FIXED_PATHNAME, O_CREAT | O_EXCL | O_RDONLY, 0773, &attrib)) == OP_ERROR){
        if(errno == EEXIST){
            return _retry_open_q();
        } else {
            perror("Cannot create message queue.");
            return OP_ERROR;
        }
    }
    if(_allocc_clients_struct() == OP_ERROR){
        return OP_ERROR;
    }
    return OP_OK;
}

int start_server(){
    atexit(_exit_handler);
    if(PUBLIC_SRV_QID == OP_ERROR || _allocc_clients_struct() == OP_ERROR){
        return OP_ERROR;
    }
    return _handle_clients_communication();
}

void _set_signals_handlers(){
    struct sigaction action;
    action.sa_handler = &_SIGINT_handler;
    sigfillset(&action.sa_mask); 
    action.sa_flags = SA_RESTART; 
    sigaction(SIGINT, &action, NULL); 
}

int _retry_open_q(){
    if(mq_unlink(FIXED_PATHNAME) == OP_ERROR){
        perror("Error while trying to release occupied message queue.");
        return OP_ERROR;
    }
    struct mq_attr attrib;
    attrib.mq_msgsize = sizeof(struct my_msgbuf);
    attrib.mq_flags = 0;  
    attrib.mq_maxmsg = 100;
    if((PUBLIC_SRV_QID = mq_open(FIXED_PATHNAME, O_CREAT | O_EXCL | O_RDWR, 0773, &attrib)) == OP_ERROR){
        perror("Error while trying to solve creation message queue error.");
        return OP_ERROR;
    }
    return OP_OK;
}


int _allocc_clients_struct(){
    CLIENTS = (Clients*)calloc(1, sizeof(Clients));
    if(CLIENTS == NULL){
        perror("Out of memory.");
        return OP_ERROR;
    }
    CLIENTS->clents_qs_ids = (int*)calloc(CLIENTS_MAXNUM, sizeof(int));
    CLIENTS->slot_free = (bool*)calloc(CLIENTS_MAXNUM, sizeof(bool));
    if(CLIENTS->clents_qs_ids == NULL || CLIENTS->slot_free == NULL){
        perror("Out of memory.");
        _free_clients_struct();
        return OP_ERROR;
    }
    for(int i = 0; i < CLIENTS_MAXNUM; i++){
        CLIENTS->slot_free[i] = true;
    }
    return OP_OK;
}

void _free_clients_struct(){
    if(CLIENTS == NULL){
        return ;
    }
    free(CLIENTS->clents_qs_ids);
    free(CLIENTS->slot_free);
    CLIENTS = NULL;
}

int _handle_clients_communication(){
    struct my_msgbuf buffer;
    bool errFlag = false;
    while(CL_ACK_RECEIVED < CL_ACK_REQUIRED){
        if(mq_receive(PUBLIC_SRV_QID, (char *)&buffer, sizeof(struct my_msgbuf), NULL) != OP_ERROR){
            _handle_client_request(&buffer);
        } else {
            perror("Error while reading a message from queue.");
            errFlag = true;
            break;
        }
    }
    if(errFlag == true){
        return OP_ERROR; 
    } else {
        return OP_OK;
    }
}

void _handle_client_request(struct my_msgbuf* request){
    struct my_msgbuf response;
    if(request->mtype != INIT && _is_number_valid(request->id) == false){
        return ;
    }
    _dispatch_request(request, &response);
    if(_request_needs_response(request->mtype) == true &&
       mq_send(CLIENTS->clents_qs_ids[response.id], (char *)&response, sizeof(struct my_msgbuf), 1) == OP_ERROR){
        perror("Error while sending answer to client.");
    }
}

bool _is_number_valid(int id){
    return (id >=0 && 
            id < CLIENTS_MAXNUM &&
            CLIENTS->slot_free[id] == false);
}

bool _request_needs_response(long type){
    if((int)type == CL_ACK || (int)type == CL_END || (int)type == END || CLOSE_PROCESS_IN_PROGRESS == true) {
        return false;
    } else {
        return true;
    }
}

void _dispatch_request(struct my_msgbuf* request, struct my_msgbuf* response){
    response->id = request->id;
    response->mtype = request->mtype;
    switch((int)request->mtype){
        case INIT:
            _handle_init_request(request, response);
            break;
        case MIRROR:
            handle_mirror_request(request, response);
            break;
        case CALC:
            handle_calc_request(request, response);
            break;
        case TIME:
            handle_time_request(request, response);
            break;
        case END:
            _handle_end_request(request, response);
            break;
        case CL_ACK:
            _handle_cl_ack_response(request);
            break;
        case CL_END:
            _handle_cl_end_request(request);
            break;
        default:
            _handle_undefined_request(response);
    }
}

void _handle_init_request(struct my_msgbuf* request, struct my_msgbuf* response){
    int idx;
    if((idx = _is_free_space_available()) == OP_ERROR){
        snprintf(response->mtext, MAX_MSG_TEXT_SIZE, "forbidden");
    } else {
        mqd_t qid;
        if((qid = mq_open(request->mtext, O_WRONLY)) == OP_ERROR){
            snprintf(response->mtext, MAX_MSG_TEXT_SIZE, "error");
        } else {
            CLIENTS->clents_qs_ids[idx] = qid;
            CLIENTS->slot_free[idx] = false;
            snprintf(response->mtext, MAX_MSG_TEXT_SIZE, "%d", idx);
            response->id = idx;
        }  
    }
}

void _handle_cl_end_request(struct my_msgbuf* request){
    if(mq_close(CLIENTS->clents_qs_ids[request->id]) == OP_ERROR){
        perror("Error with closing a client queue.");
    }
    CLIENTS->clents_qs_ids[request->id] = 0;
    CLIENTS->slot_free[request->id] = true;
}

void _handle_cl_ack_response(struct my_msgbuf* request){
    if(CLOSE_PROCESS_IN_PROGRESS == false){
        return ;
    }
    CL_ACK_RECEIVED++;
}

void _handle_end_request(struct my_msgbuf* request, struct my_msgbuf* response){
    _SIGINT_handler(0);
}

void _broadcast_cl_edn(){
    struct my_msgbuf response;
    response.mtype = (long)CL_END;
    response.id = 0;
    snprintf(response.mtext, MAX_MSG_TEXT_SIZE, "free-queue");
    for(int i = 0; i < CLIENTS_MAXNUM; i++){
        if(CLIENTS->slot_free[i] == false &&
           mq_send(CLIENTS->clents_qs_ids[i], (void *)&response, sizeof(struct my_msgbuf), 1) == OP_ERROR){
            printf("Cannot send CL_END message to a client number %d\n", i);
            CL_ACK_REQUIRED--;
        }
    }
    _close_all_client_queries();
}

void _handle_undefined_request(struct my_msgbuf* response){
    snprintf(response->mtext, MAX_MSG_TEXT_SIZE, "bad-request");
}

int _is_free_space_available(){
    int checked_elements = 0;
    while(checked_elements < CLIENTS_MAXNUM){
        if(CLIENTS->slot_free[CLIENTS_ITERATOR] == true){
            int res = CLIENTS_ITERATOR;
            CLIENTS_ITERATOR = (CLIENTS_ITERATOR + 1) % CLIENTS_MAXNUM;
            return res;
        }
        CLIENTS_ITERATOR = (CLIENTS_ITERATOR + 1) % CLIENTS_MAXNUM;
        checked_elements++;
    }
    return -1;
}

void _close_all_client_queries(){
    for(int i = 0; i < CLIENTS_MAXNUM; i ++){
        if(CLIENTS->slot_free[i] == false){
            if(mq_close(CLIENTS->clents_qs_ids[i]) == OP_ERROR){
                perror("Error with closing a client queue.");
            }
        }
    }
}