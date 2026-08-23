// transfertest.c -- validates transfertickets()
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

  settickets(20);
  mypid = getpid();

  pid = fork();
  if(pid == 0){
    settickets(5);
    printf(1, "child (pid %d) starting with 5 tickets\n", getpid());
    sleep(30);
    getpinfo(&ps);
    printf(1, "child (pid %d) now has %d tickets\n",
           getpid(), ticketsof(&ps, getpid()));
    exit();
  }

  sleep(10);
  getpinfo(&ps);
  printf(1, "before transfer: parent=%d tickets, child=%d tickets\n",
         ticketsof(&ps, mypid), ticketsof(&ps, pid));

  if(transfertickets(pid, 10) < 0){
    printf(1, "transfertickets failed\n");
  } else {
    getpinfo(&ps);
    printf(1, "after transfer:  parent=%d tickets, child=%d tickets\n",
           ticketsof(&ps, mypid), ticketsof(&ps, pid));
  }

  wait();
  exit();
}
