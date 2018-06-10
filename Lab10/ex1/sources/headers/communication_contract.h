#pragma once

#define MAX_NAME_LEN 32
#define UNIX_PATH_MAX 108

typedef enum {
    PING,
    PONG,
    CONN_REQ,
    CONN_RESP,
    OPER_REQ,
    OPER_RES
} MESSAGE_TYPES;

typedef struct {
    MESSAGE_TYPES type;
    char name[UNIX_PATH_MAX];
} msg_with_name;

typedef struct {
    MESSAGE_TYPES type;
    bool conn_accepted;
} conn_resp;

typedef struct {
    MESSAGE_TYPES type;
    int op_id;
    int operand_a;
    int operand_b;
    char operation;
} op_request;

typedef struct {
    MESSAGE_TYPES type;
    char name[UNIX_PATH_MAX];
    int op_id;
    int result;
} op_response;