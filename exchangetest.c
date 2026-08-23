// exchangetest.c -- validates exchangetickets()
#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
ticketsof(struct pstat *ps, int pid)
{
  int k;
  for(k = 0; k < NPROC; k++)
    if(ps->inuse[k] && ps->pid[k] == pid)
      return ps->tickets[k];
  return -1;
}

int
main(int argc, char *argv[])
{
  int pid, mypid;
  struct pstat ps;

  settickets(40);
  mypid = getpid();

  pid = fork();
  if(pid == 0){
    settickets(5);
    sleep(30);
    exit();
  }

  sleep(10);
  getpinfo(&ps);
  printf(1, "before exchange: parent=%d tickets, child=%d tickets\n",
         ticketsof(&ps, mypid), ticketsof(&ps, pid));

  if(exchangetickets(pid) < 0){
    printf(1, "exchangetickets failed\n");
  } else {
    getpinfo(&ps);
    printf(1, "after exchange:  parent=%d tickets, child=%d tickets\n",
           ticketsof(&ps, mypid), ticketsof(&ps, pid));
  }

  wait();
  exit();
}
