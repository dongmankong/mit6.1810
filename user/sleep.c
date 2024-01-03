// #include <iostream>
#include "user/user.h"
// #include "user/ulib.c"
// #include <stdio.h>

int main(int argc, char *argv[]){
    if(!(argc==2)){   
        // printf("sleep 参数错误");
        fprintf(2, "sleep 参数错误\n");
        exit(0);
    }
    int num=atoi(argv[1]);
    sleep(num);
    exit(0);
}