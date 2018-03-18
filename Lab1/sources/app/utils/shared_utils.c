#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "shared_utils.h"

void cleanBuffer(char* buffer, int len){
  for(int i = 0; i < len; i++){
    buffer[i] = '\0';
  }
}

void generateRandomValue(char* buffer, int maxLen) {
  const char* letters = "abcdefghijkolmnopqrstuwyz";
  int numOfLetters = strlen(letters);
  // range [1; maxLen - 1]
  int randLen = 2 + rand() % (maxLen - 2);
  for(int i = 0; i < randLen - 1; i++){
    buffer[i] = letters[rand() % numOfLetters];
  }
  buffer[randLen - 1] = '\0';
  //printf("generated: %d %s\n", randLen, buffer);
}

void printTimeStatsFromGivenTimePoint
(const char* opName, time_t stRTTick, struct timespec* stRT, struct tms* stTMS){
  struct timespec endRT;
  struct tms endTMS;
  clock_gettime(CLOCK_REALTIME, &endRT);
  time_t endRTtick = times(&endTMS);
  long ticsPerSecond = sysconf(_SC_CLK_TCK);
  printf("===================================================================\n");
  printf("Operation name: %s\n", opName);
  double milisecsRT = (endRT.tv_sec - stRT->tv_sec) * 1000 + (double)(endRT.tv_nsec - stRT->tv_nsec) / 1000000;
  printf("[TIME STATS]\nReal time: %lf ms (%ld tics with system CLK_TCK: %ld tics/s)\n",
  milisecsRT, endRTtick - stRTTick, ticsPerSecond);
  printf("User time: %ld tics\n", endTMS.tms_utime - stTMS->tms_utime);
  printf("System time: %ld tics\n", endTMS.tms_stime - stTMS->tms_stime);
  printf("===================================================================\n");
}
