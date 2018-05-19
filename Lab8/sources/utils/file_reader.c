#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "./file_reader.h"
#include "./file_utils.h"


/*
*   Functions' declarations AREA
*/

int _omit_uninportant_part_of_header(FILE * const file);
int _omit_grey_scale_info(FILE * const file);
int _get_source_dimensions(FILE * const file, int * const height, int * const width);
int _get_filter_dimension(FILE * const file, int * const c);
void _handle_dim_read_error(FILE * const file);
int _get_next_dim(FILE * const file, int* dim);
int _allocate_memory(void ** buffer, int elem_no, size_t elem_size);
int _fullfil_integer_buffer(FILE * const file, int * const buff, int elem_no);
int _fullfil_float_buffer(FILE * const file, float * const buff, int elem_no);
void close_file(FILE* const file);

/*
*   Functions' declarations AREA END
*/

int read_source_file(const char* file_name, image_repres * const output){
    FILE* file;
    if((open_file_to_read(file_name, &file) == -1) || 
       (_omit_uninportant_part_of_header(file) == -1)){
        return -1;
    }
    int width, height;
    if(_get_source_dimensions(file, &width, &height) == -1 ||
       _omit_grey_scale_info(file) == -1){
        return -1;
    }
    int * buffer;
    if(_allocate_memory((void **)&buffer, width * height, sizeof(int)) == -1){
        close_file(file);
        return -1;
    }
    if(_fullfil_integer_buffer(file, buffer, width * height) == -1){      
        return -1;
    } else {
        close_file(file);
        output->buffer = buffer;
        output->height = height;
        output->width = width;        
        return 0;
    }
}

int read_filter_file(const char * file_name, filter_repres * const output){
    FILE * file;
    if(open_file_to_read(file_name, &file) == -1){
        return -1;
    }
    int c;
    if(_get_filter_dimension(file, &c) == -1){
        return -1;
    }
    float * buffer;
    if(_allocate_memory((void **)&buffer, c*c, sizeof(float)) == -1){
        close_file(file);        
        return -1;
    }
    if(_fullfil_float_buffer(file, buffer, c * c) == -1){
        printf("Error with filfilling.\n");
        return -1;
    } else {
        close_file(file);
        output->buffer = buffer;
        output->c = c;
        return 0;
    }
}


int _omit_uninportant_part_of_header(FILE * const file){
    if(fseek(file, 2, SEEK_SET) == -1){
        perror("Could not seek in file.");
        close_file(file);
        return 1;
    } else {
        return 0;
    }
}

int _omit_grey_scale_info(FILE * const file){
    // we assume fixed 255 vlue so check is unnecesssary!
    int dumb;
    if(fscanf(file, "%d", &dumb) == EOF){
        perror("Cannot ommit grey scale info.");
        close_file(file);
        return  -1;
    } else {
        return 0;
    }
}

int _get_source_dimensions(FILE * const file, int * const width, int * const height){
    if((_get_next_dim(file, width) == -1) ||
       (_get_next_dim(file, height) == -1)){
        _handle_dim_read_error(file);
        return -1;
    } else {
        return 0;
    }
}

int _get_filter_dimension(FILE * const file, int * const c){
    if(_get_next_dim(file, c) == -1){
        _handle_dim_read_error(file);
        return -1;
    } else {
        return 0;
    }
}

void _handle_dim_read_error(FILE * const file){
    perror("Error while reading dimensions.");
    close_file(file);
}

int _get_next_dim(FILE * const file, int* dim){
    if((fscanf(file, "%d", dim) == EOF) || (*dim <= 0)){
        return -1;
    } else {
        return 1;
    }
}

int _allocate_memory(void ** buffer, int elem_no, size_t elem_size){
    *buffer = calloc(elem_no, elem_size);
    if(*buffer == NULL){
        perror("Memory allocation error");
        return -1;
    } else {
        return 0;
    }
}

int _fullfil_integer_buffer(FILE * const file, int * const buff, int elem_no){
    int * it = buff;
    bool errorFlag = false;
    for(int i = 0; i < elem_no; i++){
        if(fscanf(file, "%d", it) == EOF){
            errorFlag = true;
            break;
        }
        it++;
    }
    if(errorFlag){
        close_file(file);
        free(buff);
        return -1;
    } else {
        return 0;
    }
}

int _fullfil_float_buffer(FILE * const file, float * const buff, int elem_no){
    float * it = buff;
    bool error_flag = false;
    for(int i = 0; i < elem_no; i++){
        if(fscanf(file, "%f", it) == EOF){
            error_flag = true;
            break;
        }
        it++;
    }
    if(error_flag == true){
        close_file(file);
        free(buff);
        return -1;
    } else {
        return 0;
    }
}
