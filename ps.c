// ps.c -- validates getpinfo() / pstat by dumping extended process info
#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[])
{
  struct pstat ps;
  int i;

  if(getpinfo(&ps) < 0){
    printf(1, "ps: getpinfo failed\n");
    exit();
  }

  printf(1, "PID\tPPID\tTICKETS\tTICKS\tNSCHED\tBURST\tCHILDREN\tMEM\n");
  for(i = 0; i < NPROC; i++){
    if(ps.inuse[i]){
      printf(1, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
             ps.pid[i], ps.ppid[i], ps.tickets[i], ps.ticks[i],
             ps.nschedule[i], ps.lastburst[i], ps.nchildren[i],
             ps.memsize[i]);
    }
  }

  exit();
}
