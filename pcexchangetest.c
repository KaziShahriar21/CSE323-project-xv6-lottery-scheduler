// pcexchangetest.c -- validates the parent-child-specific ticket
// exchange tracking (pcexchange) added on top of exchangetickets().
//
// Part 1: parent <-> child A (a real parent-child pair). Exchange
//         three times, confirming both nexchange and pcexchange climb
//         by 1 each time on both sides, and that ticket balances are
//         actually swapped correctly (verified via getpinfo(), not
//         just the syscall's return value).
// Part 2: child A <-> child B (siblings, NOT parent-child to each
//         other). Confirms nexchange still climbs, but pcexchange
//         stays flat -- proving the relationship check discriminates
//         rather than firing on every exchange.

#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int passed = 0;
int failed = 0;

void
check(int condition, char *description)
{
  if(condition){
    printf(1, "PASS: %s\n", description);
    passed++;
  } else {
    printf(1, "FAIL: %s\n", description);
    failed++;
  }
}

int
statsof(int pid, int *tickets, int *nexchange, int *pcexchange)
{
  struct pstat ps;
  int k;
  getpinfo(&ps);
  for(k = 0; k < NPROC; k++){
    if(ps.inuse[k] && ps.pid[k] == pid){
      *tickets = ps.tickets[k];
      *nexchange = ps.nexchange[k];
      *pcexchange = ps.pcexchange[k];
      return 1;
    }
  }
  return 0;
}

// Fetches the persisted last-parent-child-exchange fields for pid.
// Kept separate from statsof() so existing call sites don't need to
// change just because we're now checking a different set of fields.
int
pcstatsof(int pid, int *partner, int *before, int *after)
{
  struct pstat ps;
  int k;
  getpinfo(&ps);
  for(k = 0; k < NPROC; k++){
    if(ps.inuse[k] && ps.pid[k] == pid){
      *partner = ps.last_pc_partner[k];
      *before  = ps.last_pc_before[k];
      *after   = ps.last_pc_after[k];
      return 1;
    }
  }
  return 0;
}

int
main(int argc, char *argv[])
{
  int pidA, pidB, mypid;
  int pfd[2];
  int i;
  int t_before, ne_before, pc_before;
  int t_after, ne_after, pc_after;
  int ct_before, cne_before, cpc_before;
  int ct_after, cne_after, cpc_after;

  mypid = getpid();
  settickets(50);

  if(pipe(pfd) < 0){
    printf(1, "pipe failed\n");
    exit();
  }

  printf(1, "=== Part 1: parent <-> child A, repeated exchanges ===\n");

  pidA = fork();
  if(pidA == 0){
    // ---- Child A ----
    int pidB_recv, r;
    int a_t0, a_ne0, a_pc0, a_t1, a_ne1, a_pc1;
    int a_partner0, a_before0, a_after0, a_partner1, a_before1, a_after1;

    close(pfd[1]);
    settickets(9);
    sleep(15);  // stay alive through parent's 3 exchanges (Part 1)

    read(pfd[0], &pidB_recv, sizeof(pidB_recv));
    sleep(3);   // let sibling B finish settickets()

    printf(1, "\n=== Part 2 (reported by child A): exchange with sibling B ===\n");
    statsof(getpid(), &a_t0, &a_ne0, &a_pc0);
    pcstatsof(getpid(), &a_partner0, &a_before0, &a_after0);
    r = exchangetickets(pidB_recv);
    check(r == 0, "child A: exchangetickets(childB) should succeed");
    statsof(getpid(), &a_t1, &a_ne1, &a_pc1);
    check(a_ne1 == a_ne0 + 1, "child A: nexchange should increment for sibling exchange");
    check(a_pc1 == a_pc0, "child A: pcexchange should NOT increment (sibling, not parent-child)");
    printf(1, "child A: tickets went from %d to %d after sibling exchange\n", a_t0, a_t1);

    pcstatsof(getpid(), &a_partner1, &a_before1, &a_after1);
    check(a_partner1 == a_partner0 && a_before1 == a_before0 && a_after1 == a_after0,
          "child A: last_pc_* fields should be UNCHANGED by the sibling exchange");
    printf(1, "child A: last pc-exchange still shows pid %d, %d -> %d (unchanged by sibling exchange)\n",
           a_partner1, a_before1, a_after1);

    exit();
  }

  // ---- Parent, Part 1 ----
  sleep(2);  // let child A finish settickets(9)
  for(i = 0; i < 3; i++){
    statsof(mypid, &t_before, &ne_before, &pc_before);
    statsof(pidA, &ct_before, &cne_before, &cpc_before);

    int r = exchangetickets(pidA);
    check(r == 0, "exchangetickets(childA) should succeed");

    statsof(mypid, &t_after, &ne_after, &pc_after);
    statsof(pidA, &ct_after, &cne_after, &cpc_after);

    check(t_after == ct_before, "parent's new ticket count == child A's old count");
    check(ct_after == t_before, "child A's new ticket count == parent's old count");
    check(ne_after == ne_before + 1, "parent's nexchange should increment by 1");
    check(cne_after == cne_before + 1, "child A's nexchange should increment by 1");
    check(pc_after == pc_before + 1, "parent's pcexchange should increment by 1 (parent-child pair)");
    check(cpc_after == cpc_before + 1, "child A's pcexchange should increment by 1 (parent-child pair)");

    printf(1, "  after exchange %d: parent has %d tickets, child A has %d tickets\n",
           i+1, t_after, ct_after);
  }


  printf(1, "\n=== Part 1b: persisted last-parent-child-exchange fields ===\n");
  {
    int partner, before, after;
    int cpartner, cbefore, cafter;

    pcstatsof(mypid, &partner, &before, &after);
    check(partner == pidA, "parent: last_pc_partner should be child A's pid");
    check(before == t_before, "parent: last_pc_before should match the pre-exchange snapshot");
    check(after == t_after, "parent: last_pc_after should match the post-exchange snapshot");
    printf(1, "  parent: last pc-exchange with pid %d, %d -> %d\n", partner, before, after);

    pcstatsof(pidA, &cpartner, &cbefore, &cafter);
    check(cpartner == mypid, "child A: last_pc_partner should be parent's pid");
    check(cbefore == ct_before, "child A: last_pc_before should match the pre-exchange snapshot");
    check(cafter == ct_after, "child A: last_pc_after should match the post-exchange snapshot");
    printf(1, "  child A: last pc-exchange with pid %d, %d -> %d\n", cpartner, cbefore, cafter);
  }

  // ---- Parent forks child B (sibling of A) ----
  pidB = fork();
  if(pidB == 0){
    close(pfd[0]);
    settickets(15);
    sleep(20);  // stay alive long enough for A to exchange with it
    exit();
  }

  close(pfd[0]);
  write(pfd[1], &pidB, sizeof(pidB));
  close(pfd[1]);

  wait();  // reap whichever child exits first
  wait();  // reap the other

  printf(1, "\n=== Results: %d passed, %d failed ===\n", passed, failed);

  exit();
}

