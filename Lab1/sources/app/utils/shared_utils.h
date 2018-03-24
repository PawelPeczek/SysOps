#ifndef SHARED_UTILS_H_
#define SHARED_UTILS_H_

#include <time.h>
#include <sys/times.h>

void cleanBuffer(char* buffer, int len);
void generateRandomValue(char* buffer, int maxLen);
void printTimeStatsFromGivenTimePoint
    (const char* opName, time_t stRTtick, struct timespec* stRT, struct tms* stTMS);

#endif
