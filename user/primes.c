#include "user/user.h"
void cal(int p[]);
int main(int argc, char *argv[]){
    if(!(argc==1)){   
        // printf("sleep 参数错误");
        fprintf(2, "pingpong 参数错误\n");
        exit(0);
    }
    printf("primes\n");
    int p[2];
    pipe(p);
    for(int i=2;i<=35;++i){
        write(p[1],&i,4);
    }
    close(p[1]);
    cal(p);
    exit(0);
}
void cal(int p[]){
    int firstNum;
    if(read(p[0],&firstNum,4)==0){
        close(p[0]);
        return ;
    } 
    printf("prime %d\n",firstNum);
    int pipeNext[2];
    pipe(pipeNext);
    int x;
    while(read(p[0],&x,4)!=0){
        if(x%firstNum==0) continue;
        write(pipeNext[1],&x,4);
    }
    close(pipeNext[1]);
    if(fork()==0){
        cal(pipeNext);
    }else{
        wait((int*)0);
    }
    return ;
}