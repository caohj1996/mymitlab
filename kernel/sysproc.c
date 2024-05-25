#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
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

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
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
uint64
sys_pgaccess(void){
  struct proc *p = myproc();
  uint64 arg1;
  uint64 backaddr;
  uint64 mask=0;
  int len;
  argaddr(0,&arg1);
  argint(1,&len);
  argaddr(2,&backaddr);
  pte_t* pte;
  uint64 addr;
  // printf("arg1:%p\n",arg1);
  // printf("len %d\n",len);
  for(uint64 i=0;i<len;i++){
    addr = arg1 + i*PGSIZE;
    if((pte = walk(p->pagetable,addr,0))!=0){
      if((PTE_FLAGS(*pte) & PTE_A)){
        mask = mask | (1<<i);
        *pte = *pte^PTE_A;
        // printf("pte:%p\n",*pte);
        // printf("pte:%p,i:%d\n",PTE_FLAGS(*pte),i);
        // printf("mask %p 1<<i %p\n",mask,(1<<i));
      }
    }
  }
  vmprint(p->pagetable,3);
  if(copyout(p->pagetable,backaddr,(char *)&mask,sizeof(mask))<0){
    return -1;
  }
  return 0;
}