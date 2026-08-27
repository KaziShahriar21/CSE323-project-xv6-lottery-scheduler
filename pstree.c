// pstree.c -- traverses and prints the process tree using ppid from getpinfo()
#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

struct pstat ps;

void
printtree(int pid, int depth)
{
  int i, d;

  // Find this pid's slot and print its own line first.
  for(i = 0; i < NPROC; i++){
    if(ps.inuse[i] && ps.pid[i] == pid){
      for(d = 0; d < depth; d++)
        printf(1, "  ");            // two spaces per depth level, for indentation
      if(depth > 0)
        printf(1, "\\_ ");
      printf(1, "pid=%d tickets=%d ticks=%d\n", pid, ps.tickets[i], ps.ticks[i]);
      break;
    }
  }

  // Now find every process whose ppid is this pid, and recurse into each.
  for(i = 0; i < NPROC; i++){
    if(ps.inuse[i] && ps.ppid[i] == pid){
      printtree(ps.pid[i], depth + 1);
    }
  }
}

int
main(int argc, char *argv[])
{
  if(getpinfo(&ps) < 0){
    printf(1, "pstree: getpinfo failed\n");
    exit();
  }

  printf(1, "Process tree:\n");
  printtree(1, 0);   // PID 1 is init, the root of every process tree in xv6

  exit();
}
