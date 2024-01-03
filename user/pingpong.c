#include "user/user.h"

int main(int argc, char *argv[]){
    if(!(argc==1)){   
        // printf("sleep 参数错误");
        fprintf(2, "pingpong 参数错误\n");
        exit(0);
    }
    int p2child[2];
    int child2p[2];
    pipe(p2child);
    pipe(child2p);
    if(fork()==0){
        char buf[100];
        sleep(3);
        read(p2child[0],buf,5);
        printf("%d: received ping\n",getpid());
        write(child2p[1],"parent hello",13);
    }else{
        write(p2child[1],"hello",6);
        wait((int*)0);
        char buf[100];
        read(child2p[0],buf,13);
        printf("%d: received pong\n",getpid());
    }
    exit(0);
}