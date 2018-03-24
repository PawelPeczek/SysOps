#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 500
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ftw.h>
#include "./file_utils.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static const int OP_ERROR = -1;
static const int OP_OK = 0;

static const ProgramInput* NFTW_INPUT;
static const int NFTW_MAXOPEN = 1024;

/*
* Function declaration area
*/

int _FSTraversal(const char* path, const ProgramInput* input);
void _printIfValid(const char* name, const struct stat* stat, const ProgramInput* input);
bool _isElementValidToPrint(const struct stat* stat, const ProgramInput* input);
bool _cmpOperation(char c, time_t first, time_t second);
void _printPath(const char* name);
void _printSize(off_t size);
void _printChmod(mode_t mode);
void _printLastModDate(time_t modTime);
int _preparePath(const char* prevPath, const char* dirName, char* buffer);
bool _isDirectoryToFollow(const char* name, struct stat* stat);
int _nftwUtilFun(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);

/*
* Function declaration area END
*/

int proceedFileSystemTraversal(const ProgramInput* input){
    return _FSTraversal(input->path, input);
}

int _FSTraversal(const char* path, const ProgramInput* input){
    char* pathBuffer = calloc(PATH_MAX, sizeof(char));
    DIR* directory = opendir(path);
    if(directory == NULL) return OP_ERROR;
    int opStatus = OP_OK;
    struct dirent* curr;
    struct stat buff;
    while((curr = readdir(directory)) != NULL){
        if(((opStatus = _preparePath(path, curr->d_name, pathBuffer)) == OP_ERROR) ||
           (opStatus = lstat(pathBuffer, &buff) == OP_ERROR)){
            break;
        }
        _printIfValid((const char*)pathBuffer, &buff, input);
        int fork_res = 0;
        if(_isDirectoryToFollow(pathBuffer, &buff) && ((fork_res = fork()) == 0)){
            opStatus = _FSTraversal(pathBuffer, input);
            break;
        }
    }
    free(pathBuffer);
    if(closedir(directory) == -1) return OP_ERROR;
    else return opStatus;
}

int _preparePath(const char* prevPath, const char* dirName, char* buffer){
    if((realpath(prevPath, buffer) == NULL) ||
       (strlen(buffer) + strlen("/") + strlen(dirName) >= PATH_MAX) ||
       (strcat(buffer, "/") == NULL) ||
       (strcat(buffer, dirName) == NULL)){
        return OP_ERROR;
    } else {
        return OP_OK;
    }
}

bool _isDirectoryToFollow(const char* name, struct stat* stat){
    int len = strlen(name);
    return (
        (S_ISDIR(stat->st_mode) != 0) &&
        (access(name, R_OK) == OP_OK) && // can our process access (read) the directory 
        name[len - 1] != '.' // to omit .. and . directories
    );
}

void _printIfValid(const char* name, const struct stat* stat, const ProgramInput* input){
    if(_isElementValidToPrint(stat, input)){
        _printPath(name);
        _printSize(stat->st_size);
        _printChmod(stat->st_mode);
        _printLastModDate(stat->st_mtime);
    }
}

bool _isElementValidToPrint(const struct stat* stat, const ProgramInput* input){
    return(
        (S_ISREG(stat->st_mode) != 0) &&
        _cmpOperation(input->cmp ,stat->st_mtime, input->time)
    );
}

bool _cmpOperation(char c, time_t first, time_t second){
    bool result = false;
    switch(c){
        case '=':
            result = (first == second);
            break;
        case '>':
            result = (first > second);
            break;
        case '<':
            result = (first < second);
            break;
    }
    return result;
}

void _printPath(const char* name){
    printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, name);
}

void _printSize(off_t size){
    printf(ANSI_COLOR_GREEN "Size: %lluB\t" ANSI_COLOR_RESET, (unsigned long long)size);
}

void _printChmod(mode_t mode){
    printf(ANSI_COLOR_YELLOW);
    printf( (S_ISDIR(mode)) ? "d" : "-");
    printf( (mode& S_IRUSR) ? "r" : "-");
    printf( (mode& S_IWUSR) ? "w" : "-");
    printf( (mode& S_IXUSR) ? "x" : "-");
    printf( (mode& S_IRGRP) ? "r" : "-");
    printf( (mode& S_IWGRP) ? "w" : "-");
    printf( (mode& S_IXGRP) ? "x" : "-");
    printf( (mode& S_IROTH) ? "r" : "-");
    printf( (mode& S_IWOTH) ? "w" : "-");
    printf( (mode& S_IXOTH) ? "x" : "-");
    printf("\t" ANSI_COLOR_RESET);
}

void _printLastModDate(time_t modTime){
    printf(ANSI_COLOR_BLUE "Last modif: %s\n" ANSI_COLOR_RESET, ctime(&modTime));
}


int proceedFileSystemTraversalFTW(const ProgramInput* input){
    NFTW_INPUT = input;
    char* path = calloc(PATH_MAX, sizeof(char));
    if((realpath(input->path, path) == NULL) ||
       (nftw(path, &_nftwUtilFun, NFTW_MAXOPEN, FTW_PHYS) == OP_ERROR)){
        free(path);
        return OP_ERROR;
    }
    free(path);
    return OP_OK;    
}

int _nftwUtilFun(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    if(typeflag == FTW_F)
        _printIfValid(fpath, sb, NFTW_INPUT);
    return OP_OK;
}