#ifndef _CONTRACT_H
#define _CONTRACT_H

#define MAX_MSG_TEXT_SIZE 128

extern const char* FIXED_PATHNAME;
static const int FIXED_PROJECT_CODE = 42; 

struct my_msgbuf {
    long mtype;
    int id;
    char mtext[MAX_MSG_TEXT_SIZE];
};

typedef enum {
    INIT = 1,
    MIRROR,
    CALC,
    TIME,
    END,
    CL_STOP,
    CL_ACK,
    CL_END
} CONTRACT;

#endif