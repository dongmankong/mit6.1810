#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"

void find(char *path,char *name);

int main(int argc, char *argv[])
{
//   int i;
  if(argc < 3){
    int len=strlen(argv[1]);
    memset(argv[1]+len, ' ', DIRSIZ-len);
    // strcpy(buf, argv[1]);
    // printf("%ssssssssssssssssssssssssss\n");

    find(".",argv[1]);
    exit(0);
  }
      int len=strlen(argv[2]);
    memset(argv[2]+len, ' ', DIRSIZ-len);
    // strcpy(buf, argv[2]);
    find(argv[1],argv[2]);
    exit(0);
}

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
//   char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}
void find(char *path,char *name){
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    // printf("%s    %s\n",path,name);
    if((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }
    if(fstat(fd, &st) < 0){
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }
    switch(st.type){
        case T_DEVICE:
        case T_FILE:
            // printf("%s\n",fmtname(path));
            // printf("%s\n",fmtname(name));
            // printf("%d\n",strcmp(fmtname(path),name));
            if(strcmp(fmtname(path),name)==0){
                // printf("?????????????????????\n");
                printf("%s\n",path);
            }
            break;

        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
                printf("ls: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0)
                    continue;
                // printf("%s\n",de.name);
                
                if(strcmp(de.name,".")==0 || strcmp(de.name,"..")==0){
                    continue;
                }
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if(stat(buf, &st) < 0){
                    printf("ls: cannot stat %s\n", buf);
                    continue;
                }
                // printf("%s\n",buf);
                find(buf,name);
            }
            break;
  }
  close(fd);
  
}