#ifndef MAIN_H
#define MAIN_H
#ifdef DYNAMIC
#include "../headers/contract.h"
extern  DynamicBlockTable* (*createDynamicBlockTable)(size_t, size_t);
extern  void (*freeDynamicBlockTable)(DynamicBlockTable*);
extern  int (*deleteSingleBlockFromDynamicTable)(DynamicBlockTable*, int);
extern  int (*insertToDynamicBlockTable)(DynamicBlockTable*, int, const char*, size_t);
extern  const char* (*getClosestASCIISumValueFromDynamicBlockTable)(DynamicBlockTable*, const char*, size_t);

extern  StaticLikeBlockTable* (*createStaticLikeBlockTable)(size_t, size_t);
extern  void (*freeStaticLikeBlockTable)(StaticLikeBlockTable*);
extern  int (*deleteSingleBlockFromStaticLikeTable)(StaticLikeBlockTable*, int);
extern  int (*insertToStaticLikeBlockTable)(StaticLikeBlockTable*, int, const char*, size_t);
extern  const char* (*getClosestASCIISumValueFromStaticLikeBlockTable)(StaticLikeBlockTable*, const char*, size_t);
#endif
#endif
