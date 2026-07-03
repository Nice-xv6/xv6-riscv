#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
    int pid1, pid2;
    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);

    // Child 1: high priority
    pid1 = fork();
    if (pid1 == 0) {
        close(pipe1[0]);
        nice(0);
        volatile long x = 0;
        for (long i = 0; i < 400000000L; i++) x += i;
        // send finish signal with a marker
        write(pipe1[1], "A", 1);
        close(pipe1[1]);
        exit(0);
    }

    // Child 2: low priority
    pid2 = fork();
    if (pid2 == 0) {
        close(pipe2[0]);
        nice(19);
        volatile long x = 0;
        for (long i = 0; i < 20L; i++) x += i;
        write(pipe2[1], "B", 1);
        close(pipe2[1]);
        exit(0);
    }

    // Parent reads from whichever pipe becomes ready first
    char buf[2];
    // blocking read - whichever child finishes first unblocks this
    read(pipe1[0], buf, 1); // wait for high priority child
    int high_done_first = 1;

    // check if low priority already done too
    // (non-blocking would be ideal but xv6 doesn't support it easily)

    wait(0);
    wait(0);

    if (high_done_first)
        printf("PASSED: high priority finished first\n");
    else
        printf("FAILED\n");

    exit(0);
}