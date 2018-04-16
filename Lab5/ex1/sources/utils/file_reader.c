#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include "file_reader.h"

static const int NULL_MARGIN = 1;
static const int OP_OK = 0;
static const int OP_ERROR = -1;

/*
*   Functions declaratiosn area
*/

int _getParametersNumber(const char* line, int len);
void _dispathLineCharacterCount
    (char character, int* chunkCounter, bool* isArgOpen, bool* isPrevoisuCharSpace, bool* isQuoteOpen);
int _dispathLineCharacterExtract(const char* line, int i, int* startIdx, int* endIdx, bool* isArgOpen, 
    bool* isPrevoisuCharSpace, bool* isQuoteOpen, int* idxInArgs, char** args);
bool _setFlagIfArgStarted(char character, bool* isArgOpen, bool* isQuoteOpen, bool* isPrevoiusSpace);
bool _isArgEnded(char character, bool isArgOpen, bool isQuoteOpen, bool isPrevoiusSpace);
bool _isNewArgOpenInLastChar(char characted, bool* isArgOpen, bool* isQuoteOpen, bool* isPrevoisuCharSpace);
void _toggleStatus(bool* status);
int _fulfill_pipe_args(const char* line, PipeArgs** args, int number_of_pipes);
int _fulfill_args_table(const char* line, int len, char** args, int number_of_chunks);
char* _extractArgument(const char* line, int startIdx, int endIdx);

int _count_pipe_chunks(const char* line);
bool _is_pipe_line_valid(const char* line);
bool _are_quotes_valid(const char* line);
bool _are_pipe_separators_valid(const char* line);
bool _line_start_with_pipe(const char* line);
bool _line_end_with_pipe(const char* line);
bool _no_following_pipes(const char* line);
bool _is_last_redirection_valid(const char* line);
bool _is_only_one_argument_after_pos(const char* line, int pos);
int _deternime_next_pipe_upp_bound(const char* line, int lower_pipe_bound);
void _correct_lower_boud(const char * line, int* lower_bound);
PipeArgs** _allocate_args(int num_of_chunks);
int _process_one_piece_of_pipe(const char* line, PipeArgs** args, int lower_pipe_bound, int upper_pipe_bound, int proc_elem_idx);


/*
*   Functions declaratiosn area end
*/

void freePipeArgs(PipeArgs** args){
    if(args == NULL){
        return ;
    }
    for(int  i = 0; args[i] != NULL; i++){
        for(int j = 0; args[i]->args[j] != NULL; j++){
            free(args[i]->args[j]);
        }
    }
   free(args);
}

PipeArgs** preprocessLineOfFile(FILE *stream){
    char* line;
    size_t len = 0;
    ssize_t nread;
    int num_of_pipes;
    //printf("Before read and validation\n");
    if(((nread = getline(&line, &len, stream)) == -1) ||
       (_is_pipe_line_valid(line) == false) ||
       (num_of_pipes = _count_pipe_chunks(line)) == 0) 
        return NULL;
   // printf("Trynig to alocate args\n");
    PipeArgs** args = _allocate_args(num_of_pipes);
    if(args == NULL) {
        return NULL;
    }
    //printf("Num of pipes: %d\n", num_of_pipes);
    int opStatus = _fulfill_pipe_args(line, args, num_of_pipes);
    if(opStatus == OP_ERROR){
        freePipeArgs(args);
        return NULL;
    } else {
        return args;
    }
}

PipeArgs** _allocate_args(int num_of_chunks){
    PipeArgs** args = (PipeArgs**)calloc(num_of_chunks + NULL_MARGIN, sizeof(PipeArgs*));
    if(args == NULL) 
        return NULL;
    for(int i = 0; i < num_of_chunks; i++){
        args[i] = (PipeArgs*)calloc(num_of_chunks + NULL_MARGIN, sizeof(PipeArgs));
        if(args[i] == NULL){
            freePipeArgs(args);
            return NULL;
        }
    }
    return args;
}

int _count_pipe_chunks(const char* line){
    if(line == NULL) return 0;
    const char* it = line;
    int num_of_chunks = 1;
    while(*it != '\0'){
        if(*it == '|' || *it == '>') num_of_chunks++;
        it++;
    }
    return num_of_chunks;
}

bool _is_pipe_line_valid(const char* line){
    //printf("In is pipe valid\n");
    if(_are_quotes_valid(line) == false ||
       _are_pipe_separators_valid(line) == false ||
       _is_last_redirection_valid(line) == false ||
       _no_following_pipes(line) == false){
           return false;
    } else {
        return true;
    }
}

bool _are_quotes_valid(const char* line){
    //printf("in _are_quotes_valid\n");
    const char* it = line;
    bool quote_open = false;
    while(*it != '\0'){
        //printf("Processing char: %c\n", *it);
        if(*it == '"'){
            if(quote_open == false){
                quote_open = true;
            } else {
                quote_open = false;
            }
        }
        it++;
    }
    if(quote_open == true){
        return false;
    } else {
        return true;
    }
}

bool _are_pipe_separators_valid(const char* line){
    //printf("in _are_pipe_separators_valid\n");
    if(_line_start_with_pipe(line) == true ||
       _line_end_with_pipe(line) == true){
        return false;
    } else {
        return true;;
    }
}

bool _line_start_with_pipe(const char* line){
    //printf("in _line_start_with_pipe\n");
    return (line != NULL && *line == '|');
}

bool _line_end_with_pipe(const char* line){
   // printf("in _line_end_with_pipe\n");
    const char* it = line;
    char last_non_space_char = 'a';
    while(*it != '\0'){
       if(*it != ' ') last_non_space_char = *it;
       it++;
    }
    return (last_non_space_char == '|');
}

bool _no_following_pipes(const char* line){
    const char* it = line;
    char last_non_space_char = 'a';
    bool quote_open = false;
    while(*it != '\0'){
        if(*it == '"'){
           if(quote_open == false){
                quote_open = true;
            } else {
                quote_open = false;
            }
        } else {
            if(*it == '|' && quote_open == false){
                if(last_non_space_char == *it){
                    return false;
                }
            }
        }
        if(*it != ' '){
            last_non_space_char = *it;
        }    
        it++;
    }
    return true;
}

bool _is_last_redirection_valid(const char* line){
    //printf("in _is_last_redirection_valid\n");
    bool is_redirection_found = false;
    const char* it = line;
    int pos_of_redir = 0, pos_counter = 0;
    while(*it != '\0'){
        if(*it == '>'){
            if(is_redirection_found == false){
                is_redirection_found = true;
                pos_of_redir = pos_counter;
            } else {
                return false;
            }
        } else if(*it == '|' && is_redirection_found == true){
            return false;
        }
        pos_counter++;
        it++;
    }
    return _is_only_one_argument_after_pos(line, pos_of_redir);
}

bool _is_only_one_argument_after_pos(const char* line, int pos){
    //printf("in _is_only_one_argument_after_pos\n");
    bool is_quote_opened = false, space_encountered = false;
    const char* it = line + pos;
    if(*it != '>') {
        return true;
    }
    it ++;
    while(*it != '\0' && *it == ' '){
       // printf("Processing init char: %c\n", *it);
        it++;
    }
    while(*it != '\0' && *it != '\n'){
      //  printf("Processing char: %c\n", *it);
        if(*it == '"'){
            if(is_quote_opened == true){
                is_quote_opened = false;
            } else {
                is_quote_opened = false;
            }
        } else {
            if(*it == ' '){
                space_encountered = true;
            } else if (is_quote_opened == false && space_encountered == true) {
          //      printf("done - return false\n");
                return false;
            }
        }
        it++;
    }
  //  printf("done\n");
    return true;
}

int _getParametersNumber(const char* line, int len){
   // printf("in _getParametersNumber\n");
    int chunkCounter = 0, posOfPossibleLF = len - 1;
    bool isPrevoisuCharSpace = false, isQuoteOpen = false, isArgOpen = false;
    for(int i = 0; i < posOfPossibleLF; i ++){
       _dispathLineCharacterCount
        (line[i], &chunkCounter, &isArgOpen, &isPrevoisuCharSpace, &isQuoteOpen);
    }
    // The last char can only start new arg if it is not \n or inappropriate in parser - unfinished "..." block
    if(_isNewArgOpenInLastChar(line[posOfPossibleLF], &isArgOpen, &isQuoteOpen, &isPrevoisuCharSpace))
        chunkCounter++;
    return chunkCounter;
}

void _dispathLineCharacterCount
    (char character, int* chunkCounter, bool* isArgOpen, bool* isPrevoisuCharSpace, bool* isQuoteOpen){
    if(_setFlagIfArgStarted(character, isArgOpen, isQuoteOpen, isPrevoisuCharSpace)){
        (*chunkCounter) += 1;
    } else if(_isArgEnded(character, *isArgOpen, *isQuoteOpen, *isPrevoisuCharSpace)) {
        _toggleStatus(isArgOpen);
    } else if(character == '"') {
        _toggleStatus(isQuoteOpen);
         *isPrevoisuCharSpace = false;
    } else if(character == ' '){
        *isPrevoisuCharSpace = true;
    }
}

bool _setFlagIfArgStarted(char character, bool* isArgOpen, bool* isQuoteOpen, bool* isPrevoiusSpace){
    if(character != ' ' && (*isArgOpen) == false && (*isQuoteOpen) == false){
        if(character == '"') _toggleStatus(isQuoteOpen);
        _toggleStatus(isArgOpen);
        *isPrevoiusSpace = false;
        return true;
    } else {
        return false;
    }
}

bool _isArgEnded(char character, bool isArgOpen, bool isQuoteOpen, bool isPrevoiusSpace){
    return (character == ' ' && isArgOpen == true && isPrevoiusSpace == false && isQuoteOpen == false);
}

bool _isNewArgOpenInLastChar(char characted, bool* isArgOpen, bool* isQuoteOpen, bool* isPrevoisuCharSpace){
    return (
        _setFlagIfArgStarted(characted, isArgOpen, isQuoteOpen, isPrevoisuCharSpace) &&
        characted != '\n' && characted != '"'
    );
}

void _toggleStatus(bool* status){
    if((*status) == true) *status = false;
    else *status = true; 
}

int _fulfill_pipe_args(const char* line, PipeArgs** args, int number_of_pipes){
    int lower_pipe_bound = 0, upper_pipe_bound = 0;
    for(int i = 0; i < number_of_pipes; i++){
       // printf("Processing pipe chunk: %d\n", i+1);
        upper_pipe_bound =  _deternime_next_pipe_upp_bound(line, lower_pipe_bound);
        if(lower_pipe_bound == upper_pipe_bound) {
            return OP_ERROR;
        }
      //  printf("Lower bound: %d. Upper bound: %d\n", lower_pipe_bound, upper_pipe_bound);
        //printf("Left to work with: %s\n", line + lower_pipe_bound);
        if(upper_pipe_bound - lower_pipe_bound > 0){
            if(_process_one_piece_of_pipe(line, args, lower_pipe_bound, upper_pipe_bound, i) == OP_ERROR){
                return OP_ERROR;
            }
        }
        lower_pipe_bound = upper_pipe_bound;
        _correct_lower_boud(line, &lower_pipe_bound);
       // printf("To next iteration: Lower bound: %d. Upper bound: %d\n", lower_pipe_bound, upper_pipe_bound);
    }
    return OP_OK;
}

int _process_one_piece_of_pipe(const char* line, PipeArgs** args, int lower_pipe_bound, int upper_pipe_bound, int proc_elem_idx){
   // printf("There is something to process...\n");
    int params_num = _getParametersNumber(line + lower_pipe_bound, upper_pipe_bound - lower_pipe_bound);
   // printf("In this sub_element I found %d params\n", params_num);
    if(params_num == 0) {
        return OP_ERROR;
    }
    args[proc_elem_idx]->args = (char**)calloc(params_num + NULL_MARGIN, sizeof(char*));
    if(args[proc_elem_idx]->args == NULL) {
        return OP_ERROR;
    }
   // printf("Allocation [OK]\n");
    if(_fulfill_args_table(line + lower_pipe_bound, upper_pipe_bound - lower_pipe_bound, args[proc_elem_idx]->args, params_num) == OP_ERROR){
        return OP_ERROR;
        }
   // printf("Args table fulfilled [OK]\n");
    if(lower_pipe_bound != 0 && line[lower_pipe_bound - 1] == '>') {
        args[proc_elem_idx]->is_redirected = true;
    } else {
        args[proc_elem_idx]->is_redirected = false;
    }
    return OP_OK;
}

void _correct_lower_boud(const char* line, int* lower_bound){
    while(*(line + *lower_bound) != '\0' && 
          line[*lower_bound] != '|' && 
          line[*lower_bound] != '>'){
              (*lower_bound)++;
    }
    if(*(line + *lower_bound) != '\0') {
         (*lower_bound)++;
    }
}

int _deternime_next_pipe_upp_bound(const char* line, int lower_pipe_bound){
    int pos = lower_pipe_bound;
    bool quotation_opened = false;
    while(*(line + pos) != '\0'){
        if(line[pos] == '"'){
            if(quotation_opened == true){
                quotation_opened = false;
            } else {
                quotation_opened = true;
            }
        }
        if((quotation_opened == false) && (line[pos] == '|' || line[pos] == '>')){
            break;
        }
        pos ++;
    }
    pos--;
    return pos;
}

int _fulfill_args_table(const char* line, int len, char** args, int number_of_chunks){
    int startIdx = 0, endIdx = 0, idxInArgs = 0, posOfPossibleLF = len - 1, opStatus = OP_OK;
    bool isPrevoisuCharSpace = false, isQuoteOpen = false, isArgOpen = false;
    for(int i = 0; i < posOfPossibleLF; i ++){
        opStatus = _dispathLineCharacterExtract(line, i, &startIdx, &endIdx, &isArgOpen, &isPrevoisuCharSpace,
        &isQuoteOpen, &idxInArgs, args);
    }
    if(opStatus == OP_ERROR) 
        return OP_ERROR;
    // last char handle
    if(isArgOpen == true){
        if(line[posOfPossibleLF] != '\n') args[idxInArgs] = _extractArgument(line, startIdx, len);
        else args[idxInArgs] = _extractArgument(line, startIdx, posOfPossibleLF);
    } 
    else if (_isNewArgOpenInLastChar(line[posOfPossibleLF], &isArgOpen, &isQuoteOpen, &isPrevoisuCharSpace)){
        args[idxInArgs] = _extractArgument(line, posOfPossibleLF, len);
    }
    return OP_OK;
}

int _dispathLineCharacterExtract(const char* line, int i, int* startIdx, int* endIdx, bool* isArgOpen, 
bool* isPrevoisuCharSpace, bool* isQuoteOpen, int* idxInArgs, char** args){
    if(_setFlagIfArgStarted(line[i], isArgOpen, isQuoteOpen, isPrevoisuCharSpace)){
        *startIdx = i;
    } else if(_isArgEnded(line[i], *isArgOpen, *isQuoteOpen, *isPrevoisuCharSpace)) {
        _toggleStatus(isArgOpen);
        *endIdx = i;
        if((args[*idxInArgs] = _extractArgument(line, *startIdx, *endIdx)) == NULL){
            return OP_ERROR;
        }
        *idxInArgs += 1;
    } else if(line[i] == '"') {
        _toggleStatus(isQuoteOpen);
        *isPrevoisuCharSpace = false;
    } else if(line[i] == ' '){
        *isPrevoisuCharSpace = true;
    }
    return OP_OK;
}

char* _extractArgument(const char* line, int startIdx, int endIdx){
    if(line[startIdx] == '"') startIdx++;
    if(line[endIdx - 1] == '"') endIdx--;
    char* extractedArg = (char*)calloc(endIdx - startIdx + 1, sizeof(char)); // + 1 - right exclusive! and \0 size
    if(extractedArg == NULL) return NULL;
    strncpy(extractedArg, line + startIdx, endIdx - startIdx);
    return extractedArg;
}
