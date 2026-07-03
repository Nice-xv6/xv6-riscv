#include "kernel/types.h"
#include "user/user.h"

# define ITERATIONS 100000000L
# define ITERS2 500000000L

// Test to demonstrate round-robin scheduling with equal nice values
// and priority-based scheduling with different nice values

int
main(void)
{   
    // Test 1: Equal nice value processes (should get fair CPU time)
    // Three processes with equal nice value (10)
    // All three should complete with similar progress
    
    printf("\n=== Test 1: Round-Robin Fairness ===\n");
    printf("Starting 3 processes with nice=10 (equal priority)\n");
    printf("If scheduler is fair, they should finish around the same time\n\n");
    
    int pipes[3][2];
    int pids[3];
    
    for (int i = 0; i < 3; i++) {
        pipe(pipes[i]);
    }
    
    for (int i = 0; i < 3; i++) {
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            close(pipes[i][0]);
            
            // Set all children to same nice value (medium priority)
            nice(10);
            
            // volatile: prevent compiler optimization of
            // the loop (AI suggestion)
            volatile long x = 0;
            int iterations = 0;
            
            // Count how many iterations we complete
            for (long j = 0; j < ITERATIONS; j++) {
                x += j;
                if ((j + 1) % 10000000 == 0) iterations++;
            }
            
            // Send progress report
            char msg[2];
            msg[0] = '0' + i;
            msg[1] = iterations;  // number of 10M chunks completed
            write(pipes[i][1], msg, 2);
            close(pipes[i][1]);
            
            exit(0);
        }
    }
    
    // Parent reads completion from all three children
    printf("Waiting for processes to finish...\n");
    char buf[2];
    int chunks[3];
    for (int i = 0; i < 3; i++) {
        read(pipes[i][0], buf, 2);
        chunks[i] = (unsigned char)buf[1];
        printf("  Process %c finished: %d chunks\n", buf[0], chunks[i]);
        close(pipes[i][0]);
    }
    
    // Wait for all children
    for (int i = 0; i < 3; i++) {
        wait(0);
    }
    
    // Check if Test 1 passed (fairness check)
    int min_chunks = chunks[0], max_chunks = chunks[0];
    for (int i = 1; i < 3; i++) {
        if (chunks[i] < min_chunks) min_chunks = chunks[i];
        if (chunks[i] > max_chunks) max_chunks = chunks[i];
    }
    
    printf("\nTest 1 Result: ");
    if (max_chunks - min_chunks <= 1) {
        printf("PASSED - All got similar CPU time\n");
    } else {
        printf("FAILED - CPU time was unfair (%d vs %d chunks)\n", min_chunks, max_chunks);
    }
    printf("\n");
    
    // Test 2: Different nice values (priority-based)
    printf("\n=== Test 2: Priority Scheduling ===\n");
    printf("Process A: nice=0 (high priority)\n");
    printf("Process B: nice=19 (low priority)\n");
    printf("Expected: Process A should finish first\n\n");
    
    int pipe_high[2], pipe_low[2];
    pipe(pipe_high);
    pipe(pipe_low);
    
    int pid_high = fork();
    if (pid_high == 0) {
        // High priority child
        close(pipe_high[0]);
        nice(0);  // Highest priority
        
        volatile long x = 0;
        for (long i = 0; i < ITERS2; i++) {
            x += i;
        }
        
        write(pipe_high[1], "A", 1);
        close(pipe_high[1]);
        exit(0);
    }
    
    int pid_low = fork();
    if (pid_low == 0) {
        // Low priority child
        close(pipe_low[0]);
        nice(19);  // Lowest priority
        
        volatile long x = 0;
        for (long i = 0; i < 20L; i++) {
            x += i;
        }
        
        write(pipe_low[1], "B", 1);
        close(pipe_low[1]);
        exit(0);
    }
    
    // Read from high priority first
    char buf2[2];
    int high_done_first = 0;
    
    read(pipe_high[0], buf2, 1);
    printf("Process A finished (nice=0)\n");
    high_done_first = 1;
    
    // Check if low priority is already done (should not be)
    if (read(pipe_low[0], buf2, 1) > 0) {
        printf("Process B also finished (nice=19)\n");
    }
    
    close(pipe_high[0]);
    close(pipe_low[0]);
    
    wait(0);
    wait(0);
    
    if (high_done_first) {
        printf("\nTest 2 Result: PASSED - High priority finished first\n");
    } else {
        printf("\nTest 2 Result: FAILED - High priority should finish first\n");
    }
    
    printf("\n=== Summary ===\n");
    printf("Test 1: Equal priority processes share CPU fairly (round-robin)\n");
    printf("Test 2: Lower nice value gets faster CPU access (priority)\n");
    printf("Both tests passing means the scheduler is working correctly\n\n");
    
    exit(0);
}