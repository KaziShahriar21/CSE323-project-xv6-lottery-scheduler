// validate.c -- exercises getpinfo() thoroughly: forking, CPU burn, bad input
#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

void
printrow(struct pstat *ps, int pid)
{
  int i;
  for(i = 0; i < NPROC; i++){
    if(ps->inuse[i] && ps->pid[i] == pid){
      printf(1, "pid=%d ppid=%d tickets=%d ticks=%d nsched=%d burst=%d children=%d mem=%d\n",
             ps->pid[i], ps->ppid[i], ps->tickets[i], ps->ticks[i],
             ps->nschedule[i], ps->lastburst[i], ps->nchildren[i], ps->memsize[i]);
      return;
    }
  }
  printf(1, "pid=%d not found in table!\n", pid);
}

int
main(int argc, char *argv[])
{
  struct pstat ps;
  int mypid = getpid();
  int i;
  volatile int j;

  printf(1, "=== Test 1: NULL pointer handling ===\n");
  if(getpinfo(0) == -1)
    printf(1, "PASS: getpinfo(0) correctly returned -1\n");
  else
    printf(1, "FAIL: getpinfo(0) should have returned -1\n");

  printf(1, "\n=== Test 2: fork() and nchildren ===\n");
  for(i = 0; i < 3; i++){
    if(fork() == 0){
      for(j = 0; j < 50000000; j++)  // small busy loop so children live a moment
        ;
      exit();
    }
  }
  getpinfo(&ps);
  printrow(&ps, mypid);
  printf(1, "(expect children=3 for pid=%d, since we just forked 3)\n", mypid);

  for(i = 0; i < 3; i++)
    wait();

  printf(1, "\n=== Test 3: CPU burn -> ticks/nschedule/burst should be nonzero ===\n");
  for(j = 0; j < 500000000; j++)
    ;
  getpinfo(&ps);
  printrow(&ps, mypid);
  printf(1, "(expect ticks, nsched, and burst > 0 now, after a long busy loop)\n");

  printf(1, "\n=== Test 4: memsize grows after sbrk ===\n");
  getpinfo(&ps);
  printrow(&ps, mypid);
  sbrk(4096 * 10);  // grow heap by 10 pages
  getpinfo(&ps);
  printrow(&ps, mypid);
  printf(1, "(expect mem to have grown by roughly 40960 bytes between these two rows)\n");

  exit();
}
