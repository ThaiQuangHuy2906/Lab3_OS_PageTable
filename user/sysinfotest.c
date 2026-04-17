#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/sysinfo.h"

void
sinfo(struct sysinfo *info)
{
  if (sysinfo(info) < 0) {
    printf("FAIL: sysinfo failed\n");
    exit(1);
  }
}

void
testmem()
{
  struct sysinfo info;
  uint64 n = 0;
  sinfo(&info);
  n = info.freemem;

  printf("sysinfotest: start freemem: %lu\n", n);

  if (n < 0) {
    printf("FAIL: freemem less than zero\n");
    exit(1);
  }
}

void
testproc()
{
  struct sysinfo info;
  uint64 n = 0;
  sinfo(&info);
  n = info.nproc;

  printf("sysinfotest: start nproc: %lu\n", n);

  if (n < 1) {
    printf("FAIL: nproc less than one\n");
    exit(1);
  }
}

int
main(int argc, char *argv[])
{
  printf("sysinfotest: start\n");
  testmem();
  testproc();
  printf("sysinfotest: OK\n");
  exit(0);
}
