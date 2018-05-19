#pragma once

#define MAX_THREAD_NO 32

typedef struct {
    int threads_no;
    const char* source_file;
    const char* filter_file;
    const char* output_file;
} program_input;