#ifndef MACROS_H_
#define MACROS_H_

#include <errno.h>

#define NOT_NULL(x) ({\
    if ((x) == NULL) {\
      errno = 12;\
      break;\
    }\
  })

#define INVALID_REFERENCE(x) ({\
    if ((x) == NULL) {\
      errno = 1;\
      break;\
    }\
  })

#define INVALID_SIZE(x) ({\
    if ((x) < 1) {\
      errno = 1;\
      break;\
    }\
  })

#define ERROR_OP_STATUS(x) ({\
    if ((x) != STATUS_OK) {\
      break;\
    }\
  })

#define CONDITION_FAILURE(x) ({\
    if ((x) == false) {\
      errno = 1;\
      break;\
    }\
  })

static int const STATUS_ERROR = -1;
static int const STATUS_OK = 0;

#endif
