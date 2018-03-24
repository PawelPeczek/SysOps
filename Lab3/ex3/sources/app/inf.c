#include <stdio.h>
#include <stdlib.h>

int recurr(int n){
    return recurr(n + 1);
}

int main(void){
    int i = recurr(0);
    printf("%d\n", i);
    return 0;
}