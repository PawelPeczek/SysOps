#include "./file_utils.h"

/*
*   Functions' declarations AREA
*/

int _safe_open_file(const char* file_name, FILE ** file, const char * mode);

/*
*   Functions' declarations AREA END
*/


int open_file_to_write(const char* file_name, FILE ** file){
    return _safe_open_file(file_name, file, "w");
}

int open_file_to_read(const char* file_name, FILE ** file){
    return _safe_open_file(file_name, file, "r");
}

void close_file(FILE* const file){
    if(fclose(file) == EOF){
        perror("Unable to close file");
    }
}

int _safe_open_file(const char* file_name, FILE ** file, const char * mode){
    *file = fopen(file_name, mode);
    if(file == NULL){
        perror("Coild not open file.");
        return -1;
    } else {
        return 0;
    }
}