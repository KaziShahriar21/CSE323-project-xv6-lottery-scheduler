#ifndef _PSTAT_H_
#define _PSTAT_H_
#include "types.h"
#include "param.h"

struct pstat {
  int inuse[NPROC];    // whether this slot of the process table is in use (1 or 0)
  int tickets[NPROC];  // the number of tickets this process has
  int pid[NPROC];      // the PID of each process
  int ticks[NPROC];    // the number of ticks each process has accumulated (total CPU time)
  int ppid[NPROC];     // the PID of each process's parent (0 if none)
  int nchildren[NPROC];// how many other processes have this one as their parent
  uint memsize[NPROC]; // memory size of each process, in bytes
  int nschedule[NPROC];// how many times each process has been picked to run
  int lastburst[NPROC];// length (in ticks) of each process's most recent CPU burst
  int nexchange[NPROC];// number of times each process has taken part in a ticket exchange
  int pcexchange[NPROC];// number of those exchanges that were specifically with a parent or child
  int last_pc_partner[NPROC];// pid of the most recent parent-child exchange partner (-1 if never)
  int last_pc_before[NPROC]; // this process's own ticket count immediately before that exchange
  int last_pc_after[NPROC];  // this process's own ticket count immediately after that exchange


};

#endif // _PSTAT_H_
