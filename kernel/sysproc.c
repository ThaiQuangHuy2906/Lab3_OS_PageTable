#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "kernel/sysinfo.h"

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
  if(n < 0)
    n = 0;
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

uint64
sys_trace(void)
{
  int mask;

  // Gọi argint trực tiếp, không đưa vào điều kiện if
  argint(0, &mask);

  // Gán mask vào biến trace_mask của tiến trình hiện tại
  myproc()->trace_mask = mask;

  return 0; // Thành công
}

uint64
sys_sysinfo(void)
{
  uint64 st_addr; // Biến chứa địa chỉ ảo từ user
  struct sysinfo info;
  struct proc *p = myproc();

  // Gọi argaddr trực tiếp, không ép vào điều kiện if vì nó trả về void
  argaddr(0, &st_addr);

  // Bắt lỗi nếu user truyền con trỏ NULL
  if(st_addr == 0)
    return -1;

  // Lấy dữ liệu hệ thống
  info.freemem = freemem();
  info.nproc = nproc();

  // Copy dữ liệu từ kernel space ra user space an toàn
  if(copyout(p->pagetable, st_addr, (char *)&info, sizeof(info)) < 0)
    return -1;

  return 0; // Thành công
}

uint64
sys_ptree(void)
{
  uint64 buf;
  int max;

  // Lấy 2 tham số: argaddr cho con trỏ (buf), argint cho số nguyên (max)
  argaddr(0, &buf);
  argint(1, &max);

  // Defensive Programming: Check con trỏ NULL và giới hạn max
  if(buf == 0 || max <= 0)
    return -1;

  // Gọi hàm logic trong proc.c để xử lý
  return get_ptree(buf, max);
}

uint64
sys_pgaccess(void)
{
  uint64 va;
  int n;
  uint64 abits_addr;

  // Get arguments: virtual address, number of pages, pointer to bitmask
  argaddr(0, &va);
  argint(1, &n);
  argaddr(2, &abits_addr);

  // Limit to maximum 64 pages to avoid excessive scanning
  if(n > 64)
    n = 64;

  struct proc *p = myproc();
  unsigned int bitmask = 0;

  // Scan each page and check if PTE_A bit is set
  for(int i = 0; i < n; i++) {
    pte_t *pte = walk(p->pagetable, va + i * PGSIZE, 0);
    if(pte == 0) {
      // Page not mapped, skip
      continue;
    }
    if(*pte & PTE_V) {
      // Check if accessed bit is set
      if(*pte & PTE_A) {
        // Set corresponding bit in bitmask
        bitmask |= (1U << i);
        // Clear the accessed bit
        *pte &= ~PTE_A;
      }
    }
  }

  // Copy bitmask to user space
  if(copyout(p->pagetable, abits_addr, (char *)&bitmask, sizeof(bitmask)) < 0)
    return -1;

  return 0;
}
