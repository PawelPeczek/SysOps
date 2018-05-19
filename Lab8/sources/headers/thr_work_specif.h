#pragma once

#include "./filter_repres.h"
#include "./image_repres.h"

typedef struct {
    int start_line;
    int lines_to_do;
    const image_repres * source;
    const filter_repres * filter;
    image_repres * output;
} thr_work_specif;