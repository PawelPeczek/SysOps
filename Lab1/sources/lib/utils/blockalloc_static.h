#ifndef BLOCALLOC_STATIC_H_
#define BLOCALLOC_STATIC_H_

#include <string.h>
#include "../../headers/contract.h"

StaticLikeBlockTable* createStaticLikeBlockTable(size_t numOfElements, size_t blockSize);
void freeStaticLikeBlockTable(StaticLikeBlockTable* blockTable);
int deleteSingleBlockFromStaticLikeTable(StaticLikeBlockTable* blockTable, int index);
int insertToStaticLikeBlockTable
    (StaticLikeBlockTable* blockTable, int index, const char* value, size_t len);
const char* getClosestASCIISumValueFromStaticLikeBlockTable
    (StaticLikeBlockTable* blockTable, const char* value, size_t len);


#endif
