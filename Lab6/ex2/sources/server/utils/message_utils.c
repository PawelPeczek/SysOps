#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "message_utils.h"
#include "../../headers/my_errors.h"

#define MINIM_LEN_OF_CALC_REQ 5
#define LEN_OF_MATH_CMD 3

/*
*   Functions' declarations AREA
*/

void _dispatch_math_operation(struct my_msgbuf* request, struct my_msgbuf* response);
int _get_operand(char* buff, int* proces_ptr, size_t len);
int _prepare_data_to_send(char* buff);
void _supply_operands(char* txt, int*proces_ptr, size_t len, int* op1, int* op2);

/*
*   Functions' declarations AREA END
*/


void handle_mirror_request(struct my_msgbuf* request, struct my_msgbuf* response){
    int len = 0;
    for(int i = 6; request->mtext[i] != '\0'; i++){
        if(request->mtext[i] == 10 || request->mtext[i] == 10){
            break;
        }
        len++;
    }
    for(int i = 0; i < (int)len; i++){
        response->mtext[i] = request->mtext[5 + len - i];
    }
    response->mtext[len] = '\0';
}

void handle_calc_request(struct my_msgbuf* request, struct my_msgbuf* response){
    size_t len = strlen(request->mtext);
    if(len < MINIM_LEN_OF_CALC_REQ){
        snprintf(response->mtext, MAX_MSG_TEXT_SIZE, "error");
    } else {
        _dispatch_math_operation(request, response);
    }
}

void handle_time_request(struct my_msgbuf* request, struct my_msgbuf* response){
    if(_prepare_data_to_send(response->mtext) == OP_ERROR){
        snprintf(response->mtext, MAX_MSG_TEXT_SIZE, "error");
    }
}

int _prepare_data_to_send(char* buff){
    FILE* date_output = popen("date", "r");
    if(date_output == NULL){
        return OP_ERROR;
    }
    if(fgets(buff, MAX_MSG_TEXT_SIZE, date_output) == NULL){
        return OP_ERROR;
    }
    if(pclose(date_output) == OP_ERROR){
        return OP_ERROR;
    } else {
        return OP_OK;
    }
}


void _dispatch_math_operation(struct my_msgbuf* request, struct my_msgbuf* response){
    int proces_ptr = 4;
    char operation[4];
    size_t len = strlen(request->mtext);
    while(proces_ptr < len && request->mtext[proces_ptr] == ' '){
        proces_ptr++;
    }
    for(int i = 0; i < LEN_OF_MATH_CMD; i++){
        if(proces_ptr >= len) break;
        operation[i] = request->mtext[proces_ptr];
        proces_ptr++;
    }
    operation[3] = '\0';
    while(proces_ptr < len && request->mtext[proces_ptr] == ' '){
        proces_ptr++;
    }
    bool error = false;
    int op1, op2, res;
    _supply_operands(request->mtext, &proces_ptr, len, &op1, &op2);
    if(strcmp(operation, "ADD") == 0){
        res = op1 + op2;
    } else if(strcmp(operation, "MUL") == 0){
        res = op1 * op2;
    } else if(strcmp(operation, "DIV") == 0){
        if(op2 != 0){
            res = op1 / op2;
        } else {
            error = true;
        }
    } else if(strcmp(operation, "SUB") == 0){
        res = op1 - op2;
    } else {
        error = true;
    }
    if(error == true){
        snprintf(response->mtext, MAX_MSG_TEXT_SIZE, "error");
    } else {
        snprintf(response->mtext, MAX_MSG_TEXT_SIZE, "result: %d", res);
    }
}

void _supply_operands(char* txt, int*proces_ptr, size_t len, int* op1, int* op2){
    *op1 = _get_operand(txt, proces_ptr, len);
        while(*proces_ptr < len && txt[*proces_ptr] == ' '){
            (*proces_ptr)++;
        }
    *op2 = _get_operand(txt, proces_ptr, len);
}

int _get_operand(char* buff, int* proces_ptr, size_t len){
    char operand[len];
    int op_ptr = 0;
    while(*proces_ptr < len && buff[*proces_ptr] != ' '){
        operand[op_ptr] = buff[*proces_ptr];
        op_ptr++;
        (*proces_ptr)++;
    }
    if(op_ptr == 0){
        return 0;
    } else {
        operand[op_ptr] = '\0';
        return atoi(operand);
    }
}