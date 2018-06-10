#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/un.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include "./client_utils.h"
#include "./client_parser.h"
#include "../../headers/communication_contract.h"

/*
*   Functions' declarations AREA
*/
void _client_clean_up();
void _establish_connection(client_input * input);
void _establish_IPv4_conn(client_input * input);
void _establish_local_conn(client_input * input);
void _send_init_message();
void _communication_loop();
void _dispath_request(MESSAGE_TYPES type);
void _handle_conn_response();
void _handle_ping_request();
void _handle_oper_request();
int _do_math(op_request * req);

/*
*   Functions' declarations AREA END
*/


/*
*   GLOBAL VARS
*/

int CONNECTION_FD = -1;
const char * CL_NAME;

void start_client(client_input * input){
    printf("CLIENT STARTED\n");
    if(atexit(&_client_clean_up) != 0){
        perror("atexit failed");
        return ;
    }
    CL_NAME = input->name;
    _establish_connection(input);
    printf("CONNECTION ESTABLISHED\n");
    _communication_loop();
}

void _establish_connection(client_input * input){
    printf("Conn type: %d IP_MODE: %d\n", input->conn_type, IP_MODE);
    if(input->conn_type == IP_MODE){
        _establish_IPv4_conn(input);
    } else {
        _establish_local_conn(input);
    }
}

void _establish_IPv4_conn(client_input * input){
    struct sockaddr_in addr;
    if(inet_pton(AF_INET, input->addres, &addr) != 1){
        perror("Error while translating address");
        exit(7);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = input->listen_port_big_end;
    if((CONNECTION_FD = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket() error");
        exit(8);
    }
    if(connect(CONNECTION_FD, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
        perror("AF_INET connect() error");
        shutdown(CONNECTION_FD, SHUT_RDWR);
        close(CONNECTION_FD);
        exit(9);
    }
    printf("IPv4 CONN -> TRY TO SEND INIT MSG\n");
    _send_init_message();
    printf("IPv4 CONN -> INIT MSG SEND\n");
}

void _establish_local_conn(client_input * input){
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, input->addres, UNIX_PATH_MAX);
        if((CONNECTION_FD = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket() error");
        exit(8);
    }
    if(connect(CONNECTION_FD, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1){
        perror("connect() error");
        shutdown(CONNECTION_FD, SHUT_RDWR);
        close(CONNECTION_FD);
        exit(9);
    }
    printf("LOCAL CONN -> TRY TO SEND INIT MSG\n");
    _send_init_message();
    printf("LOCAL CONN -> INIT MSG SEND\n");
}

void _send_init_message(){
    printf("_send_init_message FIRED!\n");
    msg_with_name open_msg;
    open_msg.type = CONN_REQ;
    strncpy(open_msg.name, CL_NAME, UNIX_PATH_MAX);
    printf("Name: %s\n", open_msg.name);
    printf("CONNECTION_FD: %d\n", CONNECTION_FD);
    if(send(CONNECTION_FD, (void *)&open_msg, sizeof(msg_with_name), MSG_NOSIGNAL) == -1){
        perror("Cannot send init message");
        shutdown(CONNECTION_FD, SHUT_RDWR);
        close(CONNECTION_FD);
        exit(10);
    }
    printf("_send_init_message DONE\n");
}

void _communication_loop(){
    printf("START COMMUNICATION LOOP\n");
    while(true){
        int type;
        printf("TRYING TO READ MSG\n");    
        ssize_t read_bytes = recv(CONNECTION_FD, (void *)&type, sizeof(int), MSG_PEEK);   
        if(read_bytes != sizeof(int)){
            perror("Unable to read msg");
            exit(11);
        }
        printf("MSG READ, type: %d\n", type);            
        _dispath_request((MESSAGE_TYPES)type);
    }
}

void _dispath_request(MESSAGE_TYPES type){
    switch(type){
        case CONN_RESP:
            printf("GOT MESSAGE OF TYPE CONN_RESP\n");
            _handle_conn_response();
            break;
        case PING:
            printf("GOT MESSAGE OF TYPE PING\n");        
            _handle_ping_request();
            break;
        case OPER_REQ:
            printf("GOT MESSAGE OF TYPE OPER_REQ\n");        
            _handle_oper_request();
            break;
        default:
            printf("GOT MESSAGE WRONG TYPE \n");
            perror("Bad message type");
            exit(18);
    }
}

void _handle_conn_response(){
    conn_resp resp;
    ssize_t read_bytes = recv(CONNECTION_FD, (void *)&resp, sizeof(conn_resp), MSG_NOSIGNAL);   
    if(read_bytes != sizeof(resp)){
        perror("Error reading conn_resp");
        exit(14);
    }
    if(resp.conn_accepted == false){
        printf("Wring name. Connection refused by server.\n");
        exit(15);
    }
}

void _handle_ping_request(){
    msg_with_name resp;
    ssize_t read_bytes = read(CONNECTION_FD, (void *)&resp, sizeof(msg_with_name));
    if(read_bytes != sizeof(msg_with_name)){
        perror("Error while ping message read");
        exit(16);
    }
    resp.type = PONG;
    if(write(CONNECTION_FD, (void *)&resp, sizeof(msg_with_name)) != sizeof(msg_with_name)){
        perror("Error while ping response");
        exit(17);
    }
}
 
void _handle_oper_request(){
    op_request req;
    ssize_t read_bytes = read(CONNECTION_FD, (void *)&req, sizeof(op_request));
    if(read_bytes != sizeof(op_request)){
        perror("erroe while op_request read");
        exit(14);
    }
    op_response resp;
    strncpy(resp.name, CL_NAME, UNIX_PATH_MAX);
    resp.op_id = req.op_id;
    resp.result = _do_math(&req);
    resp.type = OPER_RES;
    if(write(CONNECTION_FD, (void *)&resp, sizeof(op_response)) != sizeof(op_response)){
        perror("Error while operation response");
        exit(15);
    }
    printf("RESPONSE FOR OPERATION SEND CORRECTLY!\n");
}

int _do_math(op_request * req){
    printf("COUNTING OPERATION WITH ID: %d\n", req->op_id);
    int res;
    switch(req->operation){
        case '+':
            res = req->operand_a + req->operand_b;
            break;
        case '-':
            res = req->operand_a - req->operand_b;
            break;
        case '*':
            res = req->operand_a * req->operand_b;
            break;
        case '/':
            if(req->operand_b == 0){
                res = INT_MAX;
            } else {
                res = req->operand_a / req->operand_b;
            }   
            break;
        default:
            res = INT_MAX;
    }
    return res;
}
 

void _client_clean_up(){
    printf("_client_clean_up fired\n");
    if(CONNECTION_FD != -1){       
        shutdown(CONNECTION_FD, SHUT_RDWR);
        close(CONNECTION_FD);
    }
}