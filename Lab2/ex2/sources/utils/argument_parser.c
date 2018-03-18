#define _XOPEN_SOURCE 700
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "argument_parser.h"

static const int REQ_PARAM_NUM = 3;
static const int OPT_PARAM_NUM = 1;
static const int OPT_PARAM = 4;
static const int PATH = 1;
static const int COMP = 2;
static const int TIME = 3;
static const int OP_ERROR = -1;
static const int OP_OK = 0;

/*
*   Function declarations
*/

bool _argsChecked(int argc, const char* argv[]);
bool _argLenOk(const char* arg);
bool _sndArgOK(const char* sndArg);
bool _optArgOk(int argc, const char* argv[]);
bool _fourthArgOK(const char* fthArg);
int _fulfillInput(ProgramInput* input, int argc, const char* argv[]);
time_t _parseTime(const char* time);

/*
*   Function declarations END
*/

void printHelp(){
    printf("program_name path {< = >} date [n]\n"
           "date\tYYYY-MM-DD HH:MM:SS\n"
           "n - nftw mode\n");
}

ProgramInput* parseArguments(int argc, const char* argv[]){
    if(!_argsChecked(argc, argv)) return NULL;
    ProgramInput* input = calloc(1, sizeof(ProgramInput));
    if(_fulfillInput(input, argc, argv) == OP_ERROR){
        free(input);
        return NULL;
    }
    return input;
}

bool _argsChecked(int argc, const char* argv[]){
    return (
        (argc >= 1 + REQ_PARAM_NUM) &&
        (argc <= 1 + REQ_PARAM_NUM + OPT_PARAM_NUM) &&
        _argLenOk(argv[PATH]) &&
        _sndArgOK(argv[COMP]) &&
        _argLenOk(argv[TIME]) &&
        _optArgOk(argc, argv)
    );
}

bool _argLenOk(const char* arg){
    return (strlen(arg) > 0);
}

bool _sndArgOK(const char* sndArg){
    return (
        strlen(sndArg) == 1 &&
        (sndArg[0] == '>' || sndArg[0] == '<' || sndArg[0] == '=')
    );
}

bool _optArgOk(int argc, const char* argv[]){
    if(argc == 1 + REQ_PARAM_NUM) return true;
    else return _fourthArgOK(argv[OPT_PARAM]);
}

bool _fourthArgOK(const char* fthArg){
    return (
        strlen(fthArg) == 1 &&
        (fthArg[0] == 'n')
    );
}

int _fulfillInput(ProgramInput* input, int argc, const char* argv[]){
    input->path = argv[PATH];
    input->cmp = argv[COMP][0];
    input->time = _parseTime(argv[TIME]);
    if(input->time == OP_ERROR) return OP_ERROR;
    if(argc == 1 + REQ_PARAM_NUM + OPT_PARAM_NUM)
        input->nftw = true;
    else
        input->nftw = false;
    return OP_OK;
}

time_t _parseTime(const char* time){
    struct tm tm;
    strptime(time, "%Y-%m-%d %H:%M:%S", &tm);
    return mktime(&tm);
}