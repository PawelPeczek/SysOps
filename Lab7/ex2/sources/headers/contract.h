#pragma once

#define MAX_QUEUE_SIZE 64
#define PROJ_NO 1918
#define OP_ERROR -1
#define OP_OK 0

extern const char* PATH;
extern const char* SEM_BASE;

#define WAIT(x) ({\
    if (sem_wait(x) == OP_ERROR) {\
      perror("Error while wait()");\
      break;\
    }\
  })

#define SIGNAL(x) ({\
    if (sem_post(x) == OP_ERROR) {\
      perror("Error while signal()");\
      break;\
    }\
  })
