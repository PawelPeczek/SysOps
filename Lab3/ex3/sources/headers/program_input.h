#pragma once


typedef struct {
    const char* batchFilePath;
    int maxCPUTime;
    int maxVirtMemSizeMB;
} ProgramInput;