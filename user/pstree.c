#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/ptree.h"
#include "user/user.h"

#define MAXP 64

static int
exists_pid(struct ptreeinfo *a, int n, int pid)
{
  int i;
  for (i = 0; i < n; i++) {
    if (a[i].pid == pid)
      return 1;
  }
  return 0;
}

static void
print_tree(struct ptreeinfo *a, int n, int pid, int depth)
{
  int i, j;

  for (i = 0; i < n; i++) {
    if (a[i].pid == pid) {
      for (j = 0; j < depth; j++)
        printf("  ");
      printf("%d %s state=%d mem=%d\n",
             a[i].pid, a[i].name, a[i].state, (int)a[i].memsize);

      for (j = 0; j < n; j++) {
        if (a[j].ppid == a[i].pid)
          print_tree(a, n, a[j].pid, depth + 1);
      }
      return;
    }
  }
}

int
main(int argc, char *argv[])
{
  struct ptreeinfo buf[MAXP];
  int n, i;

  n = ptree(buf, MAXP);
  if (n < 0) {
    fprintf(2, "pstree: ptree failed\n");
    exit(1);
  }

  for (i = 0; i < n; i++) {
    if (buf[i].ppid == 0 || !exists_pid(buf, n, buf[i].ppid))
      print_tree(buf, n, buf[i].pid, 0);
  }

  exit(0);
}