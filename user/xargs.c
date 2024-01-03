#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"
int main(int argc, char *argv[]){
    char * argvBegin[MAXARG];
    int index=0;
    // printf("%s\n",argv[1]);
    for(int i=1;i<argc;++i){
        argvBegin[index]=argv[i];
        index++;
    }
    char cur;
    while(read(0,&cur,1)!=0 && cur!='\n'){
        // printf("cur: %c\n",cur);
        char buf[100];
        buf[0]=cur;
        int bufIndex=1;
        // printf("start\n");
        while(read(0,&cur,1)!=0 && cur!='\n'){
            // printf("aaaaa:  %c\n",cur);
            buf[bufIndex]=cur;
            bufIndex++;
        }
        // printf("end\n");

        buf[bufIndex]='\0';
        argvBegin[index]=buf;
        if(fork()==0){
            exec(argv[1],argvBegin);
            exit(0);
        }else{
            wait((int*)0);
        }
    }
    exit(0);
}