// edgetest.c -- deliberately tries to break every Phase 2 syscall,
// the way an instructor grading this would. Prints PASS/FAIL for each.
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
ticketsof(int pid)
{
  struct pstat ps;
  int k;
  getpinfo(&ps);
  for(k = 0; k < NPROC; k++)
    if(ps.inuse[k] && ps.pid[k] == pid)
      return ps.tickets[k];
  return -999;  // not found
}

int
main(int argc, char *argv[])
{
  int r, pid, mypid;

  printf(1, "=== settickets() edge cases ===\n");

  r = settickets(0);
  check(r == -1, "settickets(0) should fail");

  r = settickets(-5);
  check(r == -1, "settickets(-5) should fail");

  r = settickets(1);
  check(r == 0 && ticketsof(getpid()) == 1, "settickets(1) should succeed and take effect");

  r = settickets(100);
  check(r == 0 && ticketsof(getpid()) == 100, "settickets(100) should succeed and take effect");

  printf(1, "\n=== getpinfo() edge cases ===\n");
  check(getpinfo(0) == -1, "getpinfo(NULL) should fail");

  printf(1, "\n=== fork() ticket inheritance (direct, no child settickets) ===\n");
  settickets(37);
  mypid = getpid();
  pid = fork();
  if(pid == 0){
    // Child: don't call settickets at all. Just report what it inherited.
    printf(1, "child inherited %d tickets (parent had 37)\n", ticketsof(getpid()));
    exit();
  }
  wait();

  printf(1, "\n=== transfertickets() edge cases ===\n");
  settickets(20);
  mypid = getpid();

  r = transfertickets(99999, 5);
  check(r == -1, "transfertickets() to a nonexistent PID should fail");

  r = transfertickets(mypid, 0);
  check(r == -1, "transfertickets() of 0 tickets should fail");

  r = transfertickets(mypid, -5);
  check(r == -1, "transfertickets() of a negative amount should fail");

  pid = fork();
  if(pid == 0){
    settickets(3);
    sleep(20);
    exit();
  }
  sleep(5);
  r = transfertickets(pid, 100);
  check(r == -1, "transfertickets() of more than we have should fail");
  check(ticketsof(mypid) == 20, "...and our own ticket count should be unchanged after the failed attempt");

  wait();

  printf(1, "\n=== transfertickets() valid case, boundary check ===\n");
  settickets(20);
  mypid = getpid();
  pid = fork();
  if(pid == 0){
    settickets(3);
    sleep(20);
    exit();
  }
  sleep(5);

  r = transfertickets(pid, 19);
  check(r == 0, "transfertickets(pid, 19) leaving exactly 1 ticket should succeed");
  check(ticketsof(mypid) == 1, "...caller should now have exactly 1 ticket");

  r = transfertickets(pid, 1);
  check(r == -1, "transfertickets() attempting to go below 1 ticket should fail");

  kill(pid);
  wait();

  printf(1, "\n=== exchangetickets() edge cases ===\n");
  settickets(50);
  mypid = getpid();

  r = exchangetickets(99999);
  check(r == -1, "exchangetickets() with a nonexistent PID should fail");

  pid = fork();
  if(pid == 0){
    settickets(7);
    sleep(20);
    exit();
  }
  sleep(5);

  int before_parent = ticketsof(mypid);
  int before_child = ticketsof(pid);
  r = exchangetickets(pid);
  check(r == 0, "exchangetickets() with a valid PID should succeed");
  check(ticketsof(mypid) == before_child && ticketsof(pid) == before_parent,
        "...and tickets should be fully swapped");

  kill(pid);
  wait();

  printf(1, "\n=== Results: %d passed, %d failed ===\n", passed, failed);

  exit();
}
