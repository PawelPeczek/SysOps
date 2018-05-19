#pragma once

#include "../headers/filter_repres.h"
#include "../headers/image_repres.h"

int read_source_file(const char* file_name, image_repres * const output);
int read_filter_file(const char * file_name, filter_repres * const output);