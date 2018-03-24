#ifndef BLOCALLOC_DYNAMIC_H_
#define BLOCALLOC_DYNAMIC_H_

#include <string.h>
#include "../../headers/contract.h"

DynamicBlockTable* createDynamicBlockTable(size_t numOfElements, size_t blockSize);
void freeDynamicBlockTable(DynamicBlockTable* blockTable);
int deleteSingleBlockFromDynamicTable(DynamicBlockTable* blockTable, int index);
int insertToDynamicBlockTable
    (DynamicBlockTable* blockTable, int index, const char* value, size_t len);
const char* getClosestASCIISumValueFromDynamicBlockTable
    (DynamicBlockTable* blockTable, const char* value, size_t len);

#endif
