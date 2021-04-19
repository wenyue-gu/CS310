#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h> 
#include <atomic>


// Feel free to edit this file for your testing, but we will not use
// this version in the AG tests.  Only barrier_futex.c code will be
// used.

#define THREADNUM 2
#define PAPERNUM 2

// When you start, use the version that prints.  Then when you're
// done, switch to the performance options below to test your code
// with a lot of parallelism.  
//
//#define PERF_TESTING  
//#define THREADNUM 100
//#define PAPERNUM 1000

/*

  In this problem there are THREADNUM author threads.  Author threads
  all write papers, then they proofread (all of) each others papers.
  Author threads all have PAPERNUM papers that need to be
  written/proofread.

  Both writing and proofreading can happen in paralell.  When an
  author thread finishes proofreading the papers from the last step,
  they can immediately start writing their next paper.  However, they
  can't start proofreading again until all the other authors have
  finished writing for that step (e.g. an author can't start
  proofreading the others' paper #3 until all authors have finished
  writing paper #3).

  Example:

  Author 1 starting writing paper 0
  Author 0 starting writing paper 0
  Author 0 finishing writing paper 0
  Author 1 finishing writing paper 0
  Author 1 starting proofreading all 0s
  Author 0 starting proofreading all 0s
  Author 0 finishing proofreading all 0s
  Author 0 starting writing paper 1
  Author 0 finishing writing paper 1
  Author 1 finishing proofreading all 0s
  Author 1 starting writing paper 1
  Author 1 finishing writing paper 1
  Author 1 starting proofreading all 1s
  Author 0 starting proofreading all 1s
  Author 0 finishing proofreading all 1s
  Author 0 starting writing paper 2
  Author 0 finishing writing paper 2
  Author 1 finishing proofreading all 1s
  Author 1 starting writing paper 2
  Author 1 finishing writing paper 2
  Author 1 starting proofreading all 2s
  Author 0 starting proofreading all 2s
  Author 0 finishing proofreading all 2s
  Author 1 finishing proofreading all 2s
  All done

  In this example note that author 0 finishes writing paper 1 before
  author 1 finishes proofreading all 0s.  However, author 0 does not
  start proofreading all 1s until author 1 finishes writing paper 1.

  This example uses 2 threads but your solution should accomidate any
  number.

*/

// predeclaring wait_at_barrier that should be in the barrier_xxxxx.c file
void wait_at_barrier(int iteration);
void initialize_barrier(int num_threads);

void output_time_difference(const char* name, struct timeval *start, struct timeval *end)
{
    long secs_used=(end->tv_sec - start->tv_sec); //avoid overflow by subtracting first
    long usecs_used=(end->tv_usec - start->tv_usec);
    double secs = secs_used + (double) usecs_used/1000000;
    printf("%s took %f seconds\n",name, secs);
}



void *author(int* num) {
  
    for (int i=0; i < PAPERNUM; i++) {
        #ifndef PERF_TESTING
        printf("Author %d starting writing paper %d\n", *num, i);
        // bugs are move obvious when things take a different amount of time
        // sleep(*num);
        // but we'll start with the simple case
        sleep(*num*2);
        printf("Author %d finishing writing paper %d\n", *num, i);
        #endif

        wait_at_barrier(i);
        
        #ifndef PERF_TESTING
        printf("Author %d starting proofreading all %ds\n", *num, i);
        // bugs are move obvious when things take a different amount of time
        // sleep(*num);
        // but we'll start with the simple case
        sleep(1);
        printf("Author %d finishing proofreading all %ds\n", *num, i);      
        #endif
    }

    return NULL;
}

int main(int argc, char **argv) {

    initialize_barrier(THREADNUM);
    
    pthread_t threads[THREADNUM];
    int thread_ids[THREADNUM];

    struct timeval start, end;
    
    gettimeofday(&start, NULL);    
    for(int i = 0; i < THREADNUM; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, (void * (*)(void *)) author, &thread_ids[i]);
    }

    for(int i = 0; i < THREADNUM; i++) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);

  
    printf("All done\n");
    output_time_difference("all pthreads", &start, &end); 

    return 0;
}
