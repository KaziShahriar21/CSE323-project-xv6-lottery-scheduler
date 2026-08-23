// lottotest.c -- validates lottery proportionality (3:2:1 ticket ratio)
// Forks three CPU-bound children with 30/20/10 tickets, samples getpinfo
// periodically, and prints CSV: sample,ticksA,ticksB,ticksC
#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[])
{
  int pids[3];
  int tix[3] = {30, 20, 10};
  int i, j, k;
  struct pstat ps;

  for(i = 0; i < 3; i++){
    pids[i] = fork();
    if(pids[i] == 0){
      settickets(tix[i]);
      volatile unsigned int spin;
      for(;;){
        for(spin = 0; spin < 30000000; spin++)
          ;
      }
    }
  }

  printf(1, "sample,ticksA(30),ticksB(20),ticksC(10)\n");
  for(j = 0; j < 20; j++){
    sleep(20);
    if(getpinfo(&ps) < 0){
      printf(1, "lottotest: getpinfo failed\n");
      break;
    }
    printf(1, "%d", j);
    for(i = 0; i < 3; i++){
      int t = 0;
      for(k = 0; k < NPROC; k++){
        if(ps.inuse[k] && ps.pid[k] == pids[i]){
          t = ps.ticks[k];
          break;
        }
      }
      printf(1, ",%d", t);
    }
    printf(1, "\n");
  }

  
  // ---- Human-readable summary, printed every run ----
  // Captured BEFORE kill/wait, while the children are still alive and
  // still have an entry in the process table.
  {
    int final[3];
    int total = 0;
    int barwidth = 50;   // widest bar, in characters, for the top process
    int maxticks = 0;
    char label[3] = {'A', 'B', 'C'};

    getpinfo(&ps);
    for(i = 0; i < 3; i++){
      final[i] = 0;
      for(k = 0; k < NPROC; k++){
        if(ps.inuse[k] && ps.pid[k] == pids[i]){
          final[i] = ps.ticks[k];
          break;
        }
      }
      total += final[i];
      if(final[i] > maxticks)
        maxticks = final[i];
    }

    printf(1, "\n=== Summary ===\n");
    printf(1, "Ticket ratio requested: 30 : 20 : 10  (i.e. 3 : 2 : 1)\n\n");

    for(i = 0; i < 3; i++){
      int barlen, pct, j;
      if(maxticks > 0)
        barlen = (final[i] * barwidth) / maxticks;
      else
        barlen = 0;
      pct = (total > 0) ? (final[i] * 100) / total : 0;

      printf(1, "%c (%d tix) | ", label[i], tix[i]);
      for(j = 0; j < barlen; j++)
        printf(1, "#");
      printf(1, "  %d ticks (%d%%)\n", final[i], pct);
    }

    printf(1, "\nExpected split: 50%% : 33%% : 17%%   |   Total ticks measured: %d\n", total);
  }

  for(i = 0; i < 3; i++)
    kill(pids[i]);
  for(i = 0; i < 3; i++)
    wait();

  exit();
}

