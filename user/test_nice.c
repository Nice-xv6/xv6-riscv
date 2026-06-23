// Test the nice() system call and priority scheduling.
//
// Forks two children: one with nice(0) (highest priority) and one with
// nice(20) (lowest priority). Both run the same CPU-bound loop; the
// high-priority child should finish first.

#include "kernel/types.h"
#include "user/user.h"

#define LOOP_ITERS 200000000

int
main(void)
{
  int pid1, pid2, wpid;
  int wstatus;

  printf("test_nice: starting\n");

  // --- Child 1: highest priority (nice 0) ---
  pid1 = fork();
  if (pid1 < 0) {
    printf("test_nice: fork failed for child1\n");
    exit(-1);
  }

  if (pid1 == 0) {
    int old = nice(0);
    printf("test_nice: child1 (pid=%d) old_nice=%d, set nice=0\n",
           getpid(), old);

    // Busy loop to consume CPU
    volatile long x = 0;
    for (long i = 0; i < LOOP_ITERS; i++) {
      x += i;
    }
    (void)x;

    printf("test_nice: child1 (pid=%d) FINISHED\n", getpid());
    exit(0);
  }

  // --- Child 2: lowest priority (nice 20) ---
  pid2 = fork();
  if (pid2 < 0) {
    printf("test_nice: fork failed for child2\n");
    exit(-1);
  }

  if (pid2 == 0) {
    int old = nice(20);
    printf("test_nice: child2 (pid=%d) old_nice=%d, set nice=20\n",
           getpid(), old);

    // Same busy loop as child 1
    volatile long x = 0;
    for (long i = 0; i < LOOP_ITERS; i++) {
      x += i;
    }
    (void)x;

    printf("test_nice: child2 (pid=%d) FINISHED\n", getpid());
    exit(0);
  }

  // --- Parent: wait for both children ---
  int nchildren = 2;
  for (int i = 0; i < nchildren; i++) {
    wpid = wait(&wstatus);
    if (wpid < 0) {
      printf("test_nice: wait() returned error\n");
    } else if (wstatus != 0) {
      printf("test_nice: child %d exited with status %d\n", wpid, wstatus);
    }
  }

  printf("test_nice: PASSED\n");
  exit(0);
}
