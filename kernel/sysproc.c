#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void) //int pgaccess(void *base, int len, void *mask);
{
  // lab pgtbl: your code here
//my
  uint64 a0;
  int a1;
  uint64 a2;
  argaddr(0,&a0);
  argint(1,&a1);
  argaddr(2,&a2);
  if(a1>32) return -1;
  unsigned int abits=0;
  pagetable_t p=myproc()->pagetable;

  for(int i=0;i<a1;++i){
    pte_t *pte=walk(p,a0+PGSIZE*i,1);
    if(*pte & PTE_A){
      // printf("?????????????     %d        \n",i);
      abits|=(1<<i);
      *pte -=PTE_A;
    }
  }
  // printf("sssssssssss\n");
  // for(int i=31;i>=0;--i){
  //   if(abits & (1<<i)){
  //     printf("1");
  //   }else{
  //     printf("0");
  //   }
  // }
  // printf("\n");
  if(copyout(p,a2,(char *)&abits,sizeof(abits))<0) return -1;

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
