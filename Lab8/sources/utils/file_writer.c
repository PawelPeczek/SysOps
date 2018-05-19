#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "./file_writer.h"
#include "./file_utils.h"

#define BUFF_SIZE 32

/*
*   Functions' declarations AREA
*/

int _save_header(FILE * const file, const image_repres * const image);
int _save_content(FILE * const file, const image_repres * const image);
int _save_white_space(FILE * const file, int idx, int width);
int _safe_put_char(FILE * const file, char c);
int _safe_write_number(FILE * const file, int value);


/*
*   Functions' declarations AREA END
*/

int write_image(const char * file_name, const image_repres * const image){
    FILE* file;
    if((open_file_to_write(file_name, &file) == -1) ||
       (_save_header(file, image) == -1) ||
       (_save_content(file, image) == -1)){
        return -1;
    } else {
        return 0;
    }
}

int _save_header(FILE * const file, const image_repres * const image){
    char buff[BUFF_SIZE];
    if(snprintf(buff, BUFF_SIZE, "P2\n%d %d\n255\n", image->width, image->height) < 0){
        perror("Formating header error");
        free(image->buffer);
        close_file(file);
        return -1;
    }
    size_t header_len = strlen(buff);
    if(fwrite(buff, sizeof(char), header_len, file) != header_len){
        perror("Header saving error");
        free(image->buffer);
        close_file(file);
        return -1;
    }
    return 0;
}

int _save_content(FILE * const file, const image_repres * const image){
    int elem_number = image->height * image->width;
    bool err_flag = false;
    for(int i = 0; i < elem_number; i++){
        if((_safe_write_number(file, image->buffer[i]) == -1) ||
           (_save_white_space(file, i, image->width) == -1)){
            err_flag = true;
            break;
        }
    }
    if(err_flag == true){
        free(image->buffer);
        return -1;
    } else {
        return 0;
    }
}

int _save_white_space(FILE * const file, int idx, int width){
    if((idx + 1) % 32 == 0){
        return _safe_put_char(file, '\n');
    } else {
        return _safe_put_char(file, ' ');
    }
}

int _safe_put_char(FILE * const file, char c){
    if(fputc(c, file) == EOF){
        perror("Error while saving white space");
        close_file(file);
        return -1;
    } else {
        return 0;
    }
}

int _safe_write_number(FILE * const file, int value){
    char buff[BUFF_SIZE];
    if(snprintf(buff, BUFF_SIZE, "%d", value) < 0){
        perror("Error with formating value to save");
        close_file(file);
        return -1;
    }
    size_t value_len = strlen(buff);
    if(fwrite(buff, sizeof(char), value_len, file) != value_len){
        perror("Value saving error");
        close_file(file);
        return -1;
    }
    return 0;
}