#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "./image_transformation_utils.h"

/*
*   Functions' declarations AREA
*/

int imax(int a, int b);
int imin(int a, int b);
void _deterine_bounds(const thr_work_specif * const specif, int * start_idx, int * end_next_idx);
float _compute_single_pixel(const thr_work_specif * const specif, int x, int y);

/*
*   Functions' declarations AREA END
*/


void * transform_image(void* packed_specif){
    const thr_work_specif * const specif = (const thr_work_specif * const)packed_specif;
    int start_idx, end_next_idx;
    _deterine_bounds(specif, &start_idx, &end_next_idx);
    for(int i = start_idx; i < end_next_idx; i++){
        int x = i / specif->source->width;
        int y = i % specif->source->width;
        int round_s_xy = (int)roundf(_compute_single_pixel(specif, x, y));
        specif->output->buffer[i] = round_s_xy; 
    }
    return (void *)NULL;
}

int imax(int a, int b){
    return a > b ? a : b; 
}

int imin(int a, int b){
    return a < b ? a : b; 
}


void _deterine_bounds(const thr_work_specif * const specif, int * start_idx, int * end_next_idx){
    *start_idx = specif->start_line * specif->source->width;
    *end_next_idx = (specif->start_line + specif->lines_to_do) * specif->source->width;
    if(*end_next_idx > (specif->source->height * specif->source->width)){
        *end_next_idx = specif->source->height * specif->source->width;
    }
}

float _compute_single_pixel(const thr_work_specif * const specif, int x, int y){
    int ceil_x = (int)ceilf(x / 2);
    int ceil_y = (int)ceilf(y / 2);
    int c = specif->filter->c;
    float s_xy = 0;
    for(int i = 0; i < c; i++){
        for(int j = 0; j < c; j++){
            int sourc_x = imin(imax(0, x - ceil_x + i), specif->source->width);
            int sourc_y = imin(imax(0, x - ceil_y + j), specif->source->height);
            int source_idx = sourc_x * specif->source->width + sourc_y;
            s_xy += specif->source->buffer[source_idx] * specif->filter->buffer[i * c + j];
        }
    }
    return s_xy;
}