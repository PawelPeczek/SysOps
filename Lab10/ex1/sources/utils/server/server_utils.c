#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include "./server_utils.h"
#include "./server_hashtable.h"

/*
*   Functions' declarations area
*/

void _process_request(int req_no);
void terminal_loop();
void * _socket_thread(void * input);
void _dispath_request(char operator);
void _proceed_calculation_on_cluster(op_request * request);
void _prepare_network_socket(server_input * input);
void _prepare_local_socket(server_input * input);
void _configure_pool();
void _handle_sockets();
void _handle_incomming_message(int conn_fd);
void _disatch_incomming_message(int conn_fd, MESSAGE_TYPES msg_type);
void _clean_up_socket(int conn_fd, int msg_type);
void _clear_dumb_data(int conn_fd, size_t size);
void _pong_received(int conn_fd);
void _mark_pong_received(const char * name);
void _handle_new_client(int conn_fd, msg_with_name * req);
int _insert_client_to_serv_structs(const char * name, int fd);
void _deactivate_client_having_mutex(const char * name);
void _destroy_unreachable_requests_having_mutex( op_list_node * it);
void _redispath_tasks_having_mutex(op_list tasks);
void _op_resp_received(int conn_fd);
void _dealloc_op_list_node_having_mutex(op_list tasks);
void _remove_active_op_request(const char * name, int request_id);
void _remove_op_req_from_list(op_list_node sentinel, int request_id);
void _try_send_op_request_having_mutex(op_request * request, client_entry * client);
void _destroy_unreachable_requests_having_mutex(op_list_node * it);
void * _ping_thread(void * dumb);
void _ping_all();
void _check_all();
void _clean_up();
void _deallocate_client_entries();
void _deallocate_op_list(op_list_node sentinel);
void _accept_incomming_connection(int fd);
void _init_global_vars();
void _conn_req_received(int conn_fd);
void _remove_task_from_active(op_response * response);

/* 
*   Functions' declarations area END
*/

/*
*   GLOBAL VARIABLES
*/

#define ADD_OPT_NO 1
#define REM_OPT_NO 2
#define MUL_OPT_NO 3
#define DIV_OPT_NO 4
#define MAX_NO_PENDING_CONN 5

long unsigned int GLOBAL_OP_COUNTER = 0;

pthread_mutex_t clients_mutex;
sem_t srv_cfg_done;

pthread_t connection_thrd;
pthread_t ping_thread;

const char * local_socket_filename = NULL;

static clients_hash_map * CLIENTS_HM = NULL;
static client_cyclic_list * CLIENTS_LIST = NULL;

int NET_SOCKET = -1;
int LOCAL_SOCKET = -1;
int POOL_FD = -1;

/*
*   GLOBAL VARIABLES END
*/

/*
*   SIGNAL HANDLERS
*/

void _SIGINT_signals_handler(int sig_no){
    pthread_cancel(connection_thrd);
    pthread_cancel(ping_thread); 
    _clean_up();
    exit(1);
}

/*
*   SIGNAL HANDLERS END
*/



void start_server(server_input * input){
    struct sigaction action;
    sigfillset(&action.sa_mask);
    action.sa_handler = &_SIGINT_signals_handler;
    action.sa_flags = SA_RESTART; 
    sigaction(SIGINT, &action, NULL); 
    _init_global_vars();
    if(pthread_create(&connection_thrd, NULL, &_socket_thread, (void *)input) != 0 ||
      pthread_create(&ping_thread, NULL, &_ping_thread, (void *)NULL) != 0){
        perror("Cannot create threads");
        exit(3);
    }
    sem_wait(&srv_cfg_done);
    terminal_loop();
    pthread_cancel(connection_thrd);
    pthread_cancel(ping_thread);    
    pthread_join(connection_thrd, NULL);
    pthread_join(ping_thread, NULL);
}

void _init_global_vars(){
    if(atexit(&_clean_up) != 0){
        perror("Cannot set atexit");
        exit(12);
    }
    // down at the beginning
    sem_init(&srv_cfg_done, 0, 0);
    pthread_mutex_init(&clients_mutex, NULL);
    CLIENTS_HM = init_hash_map(1);
    CLIENTS_LIST = init_list();
    if(CLIENTS_HM == NULL || CLIENTS_LIST == NULL){
        printf("Cannot create internal server structs\n");
        exit(11);
    }
}   

/*
*   MAIN (TERMINAL) THREAD
*/

void terminal_loop(){
    while(true){
        printf("Operations to choose (type only the part after \":\"):\n");
        printf("->\tadd: 1 op1 op2\n");
        printf("->\trem: 2 op1 op2\n");        
        printf("->\tmul: 3 op1 op2\n");        
        printf("->\tdiv: 4 op1 op2\n");
        printf("->\tquit: 5\n");            
        int opt_num;
        scanf("%d", &opt_num);
        if(opt_num == 5) {
            break;
        } 
        _process_request(opt_num);
    }
}

void _process_request(int req_no){
    switch(req_no){
        case ADD_OPT_NO:
            _dispath_request('+');
            break;
        case REM_OPT_NO:
            _dispath_request('-');
            break;
        case MUL_OPT_NO:
            _dispath_request('*');
            break;
        case DIV_OPT_NO:
            _dispath_request('/');
            break;
    }
}

void _dispath_request(char operator){
    op_request * request = (op_request *)calloc(1, sizeof(op_request));
    if(request == NULL) {
        perror("Error with allocationg request struct");
        return ;
    }
    request->type = OPER_REQ;
    request->op_id = GLOBAL_OP_COUNTER;
    GLOBAL_OP_COUNTER ++;
    scanf("%d %d", &(request->operand_a), &(request->operand_b));
    request->operation = operator;
    _proceed_calculation_on_cluster(request);
}

void _proceed_calculation_on_cluster(op_request * request){
    client_entry * client = get_next_to_send_task(CLIENTS_LIST);
    if(client == NULL){
        printf("Cannot register tasks -> no workers\n");
        free(request);
        return ;
    }
    _try_send_op_request_having_mutex(request, client);
}

/*
*   MAIN (TERMINAL) THREAD END
*/


/*
*   SOCKET THREAD
*/

void * _socket_thread(void * data){
    if(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0){
        perror("Cannot set PTHREAD_CANCEL_ASYNCHRONOUS on ping thread");
        return (void *) NULL;
    }
    server_input * input = (server_input *) data;
    _prepare_network_socket(input);
    _prepare_local_socket(input);
    _configure_pool();
    // twice for both waiting threads
    sem_post(&srv_cfg_done);
    sem_post(&srv_cfg_done);    
    while(true){
        _handle_sockets();
    }
    return (void *) NULL;
}

void _prepare_network_socket(server_input * input){
    NET_SOCKET = socket(AF_INET, SOCK_STREAM, 0);
    if(NET_SOCKET == -1){
        perror("Error initializing socket");
        exit(13);
    }
    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = input->listen_port_big_end;
    struct in_addr IP;
    IP.s_addr = INADDR_ANY;
    socket_addr.sin_addr = IP;
    if(bind(NET_SOCKET, (const struct sockaddr *)&socket_addr, sizeof(struct sockaddr_in)) == -1){
        perror("Error while binding socket");
        exit(14);
    }
    if(listen(NET_SOCKET, MAX_NO_PENDING_CONN) == -1){
        perror("Error while starting listening at network socket");
        exit(15);
    }    
}

void _prepare_local_socket(server_input * input){
    local_socket_filename = input->local_socket;
    LOCAL_SOCKET = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(LOCAL_SOCKET == -1){
        perror("Error initializing socket");
        exit(13);
    }
    struct sockaddr_un socket_addr;
    socket_addr.sun_family = AF_LOCAL;
    strncpy(socket_addr.sun_path, input->local_socket, UNIX_PATH_MAX);
    if(bind(LOCAL_SOCKET, (const struct sockaddr *)&socket_addr, sizeof(struct sockaddr_un)) == -1){
        perror("Error while binding socket");
        exit(14);
    }
    if(listen(LOCAL_SOCKET, MAX_NO_PENDING_CONN) == -1){
        perror("Error while starting listening at local socket");
        exit(15);
    }
}

void _configure_pool(){
    POOL_FD = epoll_create1(0);
    if(POOL_FD == -1){
        perror("Error while creating pool");
        exit(17);
    }
    struct epoll_event local_event;
    local_event.events = EPOLLIN | EPOLLET;
    local_event.data.fd = LOCAL_SOCKET;
    struct epoll_event net_event;
    net_event.events = EPOLLIN | EPOLLET;
    net_event.data.fd = NET_SOCKET;
    if(epoll_ctl(POOL_FD, EPOLL_CTL_ADD, NET_SOCKET, &net_event) == -1 ||
       epoll_ctl(POOL_FD, EPOLL_CTL_ADD, LOCAL_SOCKET, &local_event) == -1){
        perror("Error while adding sockets to pool");
        exit(18);
    }
}

void _handle_sockets(){
    struct epoll_event incomming_event;
    if(epoll_wait(POOL_FD, &incomming_event, 1, -1) == -1){
        perror("Error epool_wait");
        exit(19);
    }
    if(incomming_event.data.fd == NET_SOCKET || incomming_event.data.fd == LOCAL_SOCKET){
        _accept_incomming_connection(incomming_event.data.fd);
    } else {
        _handle_incomming_message(incomming_event.data.fd);
    }
}

void _accept_incomming_connection(int fd){
    struct sockaddr addr;
    socklen_t addrlen;
    int client_conn_fd = accept(fd, &addr, &addrlen);
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = client_conn_fd;
    if(epoll_ctl(POOL_FD, EPOLL_CTL_ADD, client_conn_fd, &event) == -1){
        perror("Error while adding sockets to pool");
    }
    printf("SOCKET THREAD -> pending connection accepted with FD [%d]\n", client_conn_fd);        
}

void _handle_incomming_message(int conn_fd){
    int msg_type;
    if(recv(conn_fd, (void *)&msg_type, sizeof(int), MSG_PEEK | MSG_NOSIGNAL) == -1){
        perror("Error reading from socket");
        printf("SOCKET THREAD -> exit\n");
        exit(20);
    }
    _disatch_incomming_message(conn_fd, msg_type);
}

void _disatch_incomming_message(int conn_fd, MESSAGE_TYPES msg_type){
    switch(msg_type){
        case PONG:
            _pong_received(conn_fd);
            break;
        case CONN_REQ:
            _conn_req_received(conn_fd);
            break;
        case OPER_RES:
            _op_resp_received(conn_fd);
            break;
        default:
            printf("SOCKET THREAD -> decoded unknown message\n");                                                            
            _clean_up_socket(conn_fd, msg_type);
    }
}

void _clean_up_socket(int conn_fd, int msg_type){
    switch(msg_type){
        case PING:
            _clear_dumb_data(conn_fd, sizeof(msg_with_name));
            break;
        case CONN_RESP:
            _clear_dumb_data(conn_fd, sizeof(conn_resp));
            break;
        case OPER_REQ:
            _clear_dumb_data(conn_fd, sizeof(op_request));
            break;
        default:
            printf("WARNING! UNKNOWN MESSAGE FORMAT\n");
    }
}

void _clear_dumb_data(int conn_fd, size_t size){
    ssize_t size_read;
    char buff[64];
    if((size_read = recv(conn_fd, (void *)buff, size, MSG_NOSIGNAL)) == -1){
        perror("Cannot read from socket");
        exit(20);
    }
    if(size != size_read){
        printf("WARNING! CLEARING CONN FROM MESS DATA MAY BE UNSUCCESSFUL!\n");
    }
}

void _pong_received(int conn_fd){
    msg_with_name pong;
    ssize_t size_read = read(conn_fd, (void *)&pong, sizeof(msg_with_name));
    if(size_read == sizeof(msg_with_name)){
        _mark_pong_received(pong.name);
    } else {
        printf("WARNING! INCOMPATIBLE PONG MSG FORMAT RECEIVE!\n");
    }    
}

void _mark_pong_received(const char * name){
    pthread_mutex_lock(&clients_mutex);
    client_entry * client = get_client_by_name(CLIENTS_HM, name);
    if(client != NULL){
        client->pong_received = true;
        pthread_mutex_unlock(&clients_mutex);
    } else {
        pthread_mutex_unlock(&clients_mutex);
        printf("WARNING! RECEIVED PONG FROM UNREGISTERED CLIENT %s!\n", name);
    }
}

void _conn_req_received(int conn_fd){
    msg_with_name conn_req;
    ssize_t size_read = 
        recv(conn_fd, (void *)&conn_req, sizeof(msg_with_name), MSG_NOSIGNAL);
    if(size_read == sizeof(msg_with_name)){
        _handle_new_client(conn_fd, &conn_req);
    } else {
        printf("WARNING! INCOMPATIBLE CONN_REQUEST MSG FORMAT RECEIVE!\n");
    }    
}

void _handle_new_client(int conn_fd, msg_with_name * req){
    conn_resp response;
    response.type = CONN_RESP;
    if(get_client_by_name(CLIENTS_HM, req->name) != NULL){
        response.conn_accepted = false;
    } else {
        if(_insert_client_to_serv_structs(req->name, conn_fd) == -1){
            response.conn_accepted = false;
        } else {
            response.conn_accepted = true;
        }
    }
    if(response.conn_accepted == false){
        // just closing connection
        if(epoll_ctl(POOL_FD, EPOLL_CTL_DEL, conn_fd, NULL) == -1){
            perror("Error while adding sockets to pool");
        }
        shutdown(conn_fd, SHUT_RDWR);
        close(conn_fd);
    } else {
        int op_status = send(conn_fd, (const void *)&response, sizeof(conn_resp), MSG_NOSIGNAL); 
        if(op_status == -1){
            pthread_mutex_lock(&clients_mutex);
            _deactivate_client_having_mutex(req->name);
            pthread_mutex_unlock(&clients_mutex);            
        }
    }    
}

int _insert_client_to_serv_structs(const char * name, int fd){
    client_entry * client = (client_entry *)calloc(1, sizeof(client_entry));
    if(client == NULL){
        perror("Mem alloc error");
        return -1;
    }
    pthread_mutex_lock(&clients_mutex);
    list_node * cl_node = add_to_list(CLIENTS_LIST, client);
    if(cl_node == NULL){
        pthread_mutex_unlock(&clients_mutex);
        free(client);
        return -1;
    }
    client->ptr_to_list = cl_node;
    strncpy(client->name, name, UNIX_PATH_MAX);
    // active by default;
    client->pong_received = true;
    client->socket_fd = fd;                                           
    if(add_to_hash_map_if_slot_free(&CLIENTS_HM, client) == -1){
        if(delete_from_list(CLIENTS_LIST, cl_node) == -1){
            pthread_mutex_unlock(&clients_mutex);    
            printf("[FATAL ERROR] Cohesion error in internal structs.");
            shutdown(fd, SHUT_RDWR);
            close(fd);
            exit(27);
        }
        pthread_mutex_unlock(&clients_mutex);    
        return -1;
    }
    pthread_mutex_unlock(&clients_mutex);    
    return 0;
}

void _deactivate_client_having_mutex(const char * name){
    client_entry * client = get_client_by_name(CLIENTS_HM, name);
    if(client == NULL){ 
        return ;
    }
    _redispath_tasks_having_mutex(client->active_requests);
    if(rem_from_hash_map(&CLIENTS_HM, name) == -1 ||
       delete_from_list(CLIENTS_LIST, client->ptr_to_list) == -1){
        pthread_mutex_unlock(&clients_mutex);
        printf("[FATAL ERROR] Cohesion error in internal structs.");
        if(epoll_ctl(POOL_FD, EPOLL_CTL_DEL, client->socket_fd, NULL) == -1){
            perror("Error while adding sockets to pool");
        }
        shutdown(client->socket_fd, SHUT_RDWR);
        close(client->socket_fd);
        exit(27);
    }
    if(epoll_ctl(POOL_FD, EPOLL_CTL_DEL, client->socket_fd, NULL) == -1){
            perror("Error while deleting sockets to pool");
    }
    shutdown(client->socket_fd, SHUT_RDWR);
    close(client->socket_fd);
    free(client);
}

void _redispath_tasks_having_mutex(op_list tasks){
    op_list_node * it = tasks.sentinel.next;
    if(it != NULL){
        printf("Need to redispath tasks!\n");
    }
    while(it != NULL){
        client_entry * client = get_next_to_send_task(CLIENTS_LIST);
        if(client == NULL){
            printf("NO CLIENT AVAILABLE!\n");
            _destroy_unreachable_requests_having_mutex(it);
            break;
        }   
        _try_send_op_request_having_mutex(it->request, client);
        it = it->next;
    }
    _dealloc_op_list_node_having_mutex(tasks);
}

void _destroy_unreachable_requests_having_mutex(op_list_node * it){
    printf("Lack of clients, destroying all remaining tasks.");
    while(it != NULL){
        printf("Removing task [%d] %d %c %d", it->request->op_id,
               it->request->operand_a, it->request->operation, it->request->operand_b);
        free(it->request);
        it = it->next;
    }
}

void _try_send_op_request_having_mutex(op_request * request, client_entry * client){
    op_list_node * new_node = (op_list_node *)calloc(1, sizeof(op_list_node));
    new_node->request = request;
    ssize_t sent_bytes = send(client->socket_fd, (const void *)request, sizeof(op_request), MSG_NOSIGNAL);
    if(new_node == NULL || sent_bytes != sizeof(op_request)){
        free(new_node);
        printf("Cannot send task to client %s\n", client->name);
        printf("Removing task [%d] %d %c %d", request->op_id,
                request->operand_a, request->operation, request->operand_b);
        free(request);
    } else {
        new_node->next = client->active_requests.sentinel.next;
        client->active_requests.sentinel.next = new_node;
    }
}

void _dealloc_op_list_node_having_mutex(op_list tasks){
    op_list_node * it = tasks.sentinel.next;
    op_list_node * tmp;
    while(it != NULL){
        tmp = it;
        it = it->next;
        free(tmp);
    }
}

void _op_resp_received(int conn_fd){
    op_response response;
    ssize_t bytes_read = recv(conn_fd, (void *)&response, sizeof(op_response), MSG_NOSIGNAL);
    if(bytes_read == sizeof(response)){
        if(response.result == INT_MAX){
            printf("[OPERATION RESULT]\t <id:%d> ERROR\n", response.op_id);            
        } else {
            printf("[OPERATION RESULT]\t <id:%d> %d\n", response.op_id, response.result);
        }
        _remove_task_from_active(&response);
    } else {
        printf("Operation result didn't received properly\n");
    }
}

void _remove_task_from_active(op_response * response){
    client_entry * client = get_client_by_name(CLIENTS_HM, response->name);
    if(client == NULL){
        return ;
    }
    op_list_node * prev = &(client->active_requests.sentinel);
    op_list_node * next = prev->next;
    while(next != NULL){
        if(next->request->op_id == response->op_id){
            prev->next = next->next;
            free(next->request);
            free(next);
            break;
        }
        prev = next;
        next = next->next;
    }
}
 

void _remove_active_op_request(const char * name, int request_id){
    pthread_mutex_lock(&clients_mutex);
    client_entry * client = get_client_by_name(CLIENTS_HM, name);
    if(client == NULL){
        pthread_mutex_unlock(&clients_mutex);
        return ;
    }
    _remove_op_req_from_list(client->active_requests.sentinel, request_id);
    pthread_mutex_unlock(&clients_mutex);    
}


void _remove_op_req_from_list(op_list_node sentinel, int request_id){
    op_list_node * prev = &sentinel;
    op_list_node * next = prev->next;
    while(next != NULL){
        if(next->request->op_id == request_id){
            prev->next = next->next;
            free(next);
            break;
        } else {
            prev = next;
            next = next->next;
        }
    }
}
/*
*   SOCKET THREAD END
*/

/*
*   PING THREAD
*/

void * _ping_thread(void * dumb){
    if(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0){
        perror("Cannot set PTHREAD_CANCEL_ASYNCHRONOUS on ping thread");
        return (void *) NULL;
    }
    sem_wait(&srv_cfg_done);
    while(true){
        sleep(15);
        _ping_all();
        sleep(5);
        _check_all();
    }
    return (void *) NULL;
} 

void _ping_all(){
    pthread_mutex_lock(&clients_mutex);
    list_node *it = CLIENTS_LIST->left_sent.next;
    while(it != &(CLIENTS_LIST->right_sent)){
        it->client->pong_received = false;
        msg_with_name ping;
        ping.type = PING;
        strncpy(ping.name, it->client->name, UNIX_PATH_MAX);
        printf("PING THREAD SENDING PING TO %s [%d]\n", ping.name, it->client->socket_fd);        
        ssize_t sent_bytes = send(it->client->socket_fd, (const void *)&ping, sizeof(msg_with_name), MSG_NOSIGNAL);
        list_node * to_deactivate = it;
        it = it->next;
        if(sent_bytes != sizeof(msg_with_name)){
            printf("PING THREAD need to deactivate client %s!\n", ping.name);
            _deactivate_client_having_mutex(to_deactivate->client->name);
        }
    }
    pthread_mutex_unlock(&clients_mutex);    
}


void _check_all(){
    pthread_mutex_lock(&clients_mutex);
    list_node *it = CLIENTS_LIST->left_sent.next;
    while(it != &(CLIENTS_LIST->right_sent)){
        list_node *tmp = it;
        it = it->next;
        if(tmp->client->pong_received == false){
            _deactivate_client_having_mutex(tmp->client->name);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/*
*   PING THREAD END
*/

/*
*   CLEAN UP
*/

void _clean_up(){
    if(local_socket_filename != NULL){
        unlink(local_socket_filename);
    }
    sem_destroy(&srv_cfg_done);
    pthread_mutex_destroy(&clients_mutex);
    if(CLIENTS_LIST != NULL){
        _deallocate_client_entries();
        dealloc_list(&CLIENTS_LIST);
        CLIENTS_LIST = NULL;
    }
    if(CLIENTS_HM != NULL){
        dealloc_hash_map(&CLIENTS_HM);
        CLIENTS_HM = NULL;
    }
    if(NET_SOCKET != -1){
        shutdown(NET_SOCKET, SHUT_RDWR);
        close(NET_SOCKET);
        NET_SOCKET = -1;
    }
    if(LOCAL_SOCKET != -1){
        shutdown(LOCAL_SOCKET, SHUT_RDWR);
        close(LOCAL_SOCKET);
        LOCAL_SOCKET = -1;        
    }
}

void _deallocate_client_entries(){
    if(CLIENTS_LIST == NULL) {
        return ;
    }
    list_node *it = CLIENTS_LIST->left_sent.next;
    while(it != &(CLIENTS_LIST->right_sent)){
        if(it->client != NULL){
            _deallocate_op_list(it->client->active_requests.sentinel);
            shutdown(it->client->socket_fd, SHUT_RDWR);
            close(it->client->socket_fd);
            list_node * to_del = it;
            it = it->next;
            free(to_del->client);
        }
    }
}

void _deallocate_op_list(op_list_node sentinel){
    op_list_node * prev = sentinel.next;
    op_list_node * next;
    while(prev != NULL){
        next = prev->next;
        free(prev->request);
        free(prev);
        prev = next;
    }
}