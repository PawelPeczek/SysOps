#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include "../headers/program_input.h"
#include "../utils/parser/argument_parser.h"
#include "../utils/copy_utils/system_copy.h"
#include "../utils/copy_utils/lib_copy.h"
#include "../utils/generation_utils/system_generate.h"
#include "../utils/sort_utils/system_sort.h"
#include "../utils/sort_utils/lib_sort.h"

static const int OP_ERROR = -1;

/*
*   Function declaration area
*/

int dispatchInput(ProgramInput* input);
void printTimeStatsFromGivenTimePoint
(time_t stRTTick, struct timespec* stRT, struct tms* stTMS);

/*
*   End of unction declaration area
*/


int main(int argc, const char* argv[]){
    ProgramInput* input = parseArguments(argc, argv);
    if(input == NULL){
        printHelp();
        exit(1);
    }

    if(dispatchInput(input) == OP_ERROR) {
        perror("Error while processing operation");
        free(input);
        exit(2);
    }
    free(input);
    return 0;
}

int dispatchInput(ProgramInput* input){
    int operationResult;
    struct timespec startRealTime;
    struct tms stTMS;
    clock_gettime(CLOCK_REALTIME, &startRealTime);
    clock_t stCT = times(&stTMS);
    switch(input->mode){
        case SORT:
            if(input->funcTypes == SYSTEM)
                operationResult = sortWithSystemCalls(input);
            else 
                operationResult = sortWithLibCalls(input);
            break;
        case GENERATE:
            operationResult = generateWithSystemCalls(input);
            break;
        case COPY:
            if(input->funcTypes == SYSTEM)
                operationResult = copyWithSystemCalls(input);
            else 
                operationResult = copyWithLibCalls(input);
            break;
    }
    printTimeStatsFromGivenTimePoint(stCT, &startRealTime, &stTMS);
    return operationResult;
}

void printTimeStatsFromGivenTimePoint
(time_t stRTTick, struct timespec* stRT, struct tms* stTMS){
  struct timespec endRT;
  struct tms endTMS;
  clock_gettime(CLOCK_REALTIME, &endRT);
  time_t endRTtick = times(&endTMS);
  long ticsPerSecond = sysconf(_SC_CLK_TCK);
  printf("===================================================================\n");
  double milisecsRT = (endRT.tv_sec - stRT->tv_sec) * 1000 + (double)(endRT.tv_nsec - stRT->tv_nsec) / 1000000;
  printf("[TIME STATS]\nReal time: %lf ms (%ld tics with system CLK_TCK: %ld tics/s)\n",
  milisecsRT, endRTtick - stRTTick, ticsPerSecond);
  printf("User time: %ld tics\n", endTMS.tms_utime - stTMS->tms_utime);
  printf("System time: %ld tics\n", endTMS.tms_stime - stTMS->tms_stime);
  printf("===================================================================\n");
}