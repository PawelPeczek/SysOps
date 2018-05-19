#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/times.h>
#include "../headers/program_input.h"
#include "../headers/image_repres.h"
#include "../headers/filter_repres.h"
#include "../headers/thr_work_specif.h"
#include "../utils/argument_parser.h"
#include "../utils/file_reader.h"
#include "../utils/file_writer.h"
#include "../utils/image_transformation_utils.h"


/*
*   Functions' declarations AREA
*/

int _allocate_output_buffer(const image_repres * const source, image_repres * output);
void _start_thread_computation(const program_input * const prog_input, thr_work_specif * const specif);
void wait_for_all_threads(const pthread_t * const threads, int thread_no, const thr_work_specif * const specif);
void _free_thr_work_specif(const thr_work_specif * const specif);
void _prepare_spec_copy_for_threads(thr_work_specif * const specif_buff,  const thr_work_specif * const original, int thr_no);
void _print_time_stats(int thr_no, const thr_work_specif * const specif, const clock_t t_start, 
const struct tms * const tms_start, const clock_t t_end, const struct tms * const tms_end);
/*
*   Functions' declarations AREA END
*/

int main(int argc, const char * argv[]){
    program_input input;
    if(parse_arguments(argv, argc, &input) == -1){
        print_help();
        exit(2);
    }
    image_repres source;
    if(read_source_file(input.source_file, &source) == -1){
        exit(3);
    }
    filter_repres filter;
    if(read_filter_file(input.filter_file, &filter) == -1){
        free(source.buffer);
        exit(4);
    }
    image_repres output;
    if(_allocate_output_buffer(&source, &output) == -1){
        free(filter.buffer);
        exit(5);
    }
    thr_work_specif specif;
    specif.filter = &filter;
    specif.source = &source;
    specif.output = &output;
    _start_thread_computation(&input, &specif);
    return 0;
}

int _allocate_output_buffer(const image_repres * const source, image_repres * output){
    output->buffer = (int *)calloc(source->height * source->width, sizeof(int));
    if(output == NULL){
        free(source->buffer);
        return -1;
    } else {
        output->height = source->height;
        output->width = source->width;
        return 0;
    }
}

void _start_thread_computation(const program_input * const prog_input, thr_work_specif * const orig_specif){
    int real_thread_no = prog_input->threads_no;
    if(real_thread_no > orig_specif->source->height){
        real_thread_no = orig_specif->source->height;
        printf("[WARNING] Redicing ammount of threads to: %d\n", real_thread_no);
    }
    orig_specif->lines_to_do = (int)ceil((double)orig_specif->source->height / (double)real_thread_no);
    int start_line = 0;
    pthread_t threads[MAX_THREAD_NO];
    thr_work_specif specifications[MAX_THREAD_NO];
    _prepare_spec_copy_for_threads(specifications, orig_specif, real_thread_no);
    // time measurement starts
    struct tms tms_start;
	clock_t t_start = times(&tms_start);
    for(int i = 0; i < real_thread_no; i++){
        specifications[i].start_line = start_line;
        if(pthread_create(threads + i, NULL, &transform_image, specifications + i) != 0){
            perror("Error while thread start.");
            _free_thr_work_specif(orig_specif);
            exit(13);
        }
        start_line += orig_specif->lines_to_do;
    }
    wait_for_all_threads(threads, real_thread_no, orig_specif);
    // time measurement starts
    struct tms tms_end;
	clock_t t_end = times(&tms_end);
    if(write_image(prog_input->output_file, orig_specif->output) == -1){
        printf("Error in writing!\n");  
        _free_thr_work_specif(orig_specif);
        exit(14);
    }
    _print_time_stats(real_thread_no, orig_specif, t_start, &tms_start, t_end, &tms_end);
    _free_thr_work_specif(orig_specif);
}

void _prepare_spec_copy_for_threads(thr_work_specif * const specif_buff,  const thr_work_specif * const original, int thr_no){
    for(int i = 0; i < thr_no; i++){
        specif_buff[i].filter = original->filter;
        specif_buff[i].lines_to_do = original->lines_to_do;
        specif_buff[i].output = original->output;
        specif_buff[i].source = original->source;
    }
}

void wait_for_all_threads(const pthread_t * const threads, int thread_no, const thr_work_specif * const specif){
    for(int i = 0; i < thread_no; i++){
        if(pthread_join(*(threads + i), NULL) != 0){
            perror("Error while thread joining.");
            _free_thr_work_specif(specif);
            exit(14);
        }
    }
}

void _print_time_stats(int thr_no, const thr_work_specif * const specif, const clock_t t_start, 
const struct tms * const tms_start, const clock_t t_end, const struct tms * const tms_end){
    printf("Time statistics given set-up:\nThread number: %d\nImage size: %d x %d\nFilter size: %d\n",
           thr_no, specif->source->width, specif->source->height, specif->filter->c);
    printf("[TIME]: %ld\n", t_end - t_start);
    printf("[USER TIME]: %ld\n", tms_end->tms_utime - tms_start->tms_utime); 
}

void _free_thr_work_specif(const thr_work_specif * const specif){
    free(specif->filter->buffer);    
    free(specif->source->buffer); 
    free(specif->output->buffer);   
}