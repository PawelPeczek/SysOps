#pragma once

#include <stdio.h>

int open_file_to_read(const char* file_name, FILE ** file);
int open_file_to_write(const char* file_name, FILE ** file);
void close_file(FILE* const file);