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


/* Note that this particular barrier implementation only allows one
   barrier (but one can be reused, as long as the iteration variable
   is keep up to date).

   To be slightly fancier, we could put these various fields in a
   struct and then this would be library that would allow several
   variables */

pthread_cond_t waiting_for_barrier_open_cv;
pthread_mutex_t mutex;

// how many threads are there total
int threadnum;
// how many threads are waiting at the barrier?
int threads_at_barrier;
// how many times has the barrier opened?
int barrier_opening_count;

// place any useful initialization code you need here the parameter is
// the number of threads that will be waiting for the barrier.
//
// i.e. the barrier will open when threadnum threads are waiting
void initialize_barrier(int threadnum_input) {
    threadnum = threadnum_input;
    threads_at_barrier = 0;
    barrier_opening_count = 0;
    
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&waiting_for_barrier_open_cv, NULL);
}

/*
  The interation parameter indicates how many times a particular
  thread has waited at this barrier before.

  So the first time the thread calls wait_at_barrier the value should
  be 0, 2nd time 1, etc.

 */
void wait_at_barrier(int iteration) {

    // sanity check on the algorithm: if we're just about to start
    // waiting at the barrier, the barier couldn't have opened yet
    // right?
    assert(barrier_opening_count == iteration);

    // ok, lock to mutex to increment the finished count and wait
    // on the others if we need
    assert(pthread_mutex_lock(&mutex) == 0);
    threads_at_barrier++;
    if(threads_at_barrier == threadnum) {
        // ok open the barrier
        threads_at_barrier = 0;
        barrier_opening_count++;
        // broadcast to waiting threads that the barrier is open
        assert(pthread_cond_broadcast(&waiting_for_barrier_open_cv) == 0);
        // release the mutex
        assert(pthread_mutex_unlock(&mutex) == 0);
    } else {
        // we're not the last so we must wait
        while(1) {
            // here's where we wait at the barrier
            assert(pthread_cond_wait(&waiting_for_barrier_open_cv, &mutex) == 0);
            // it is possible though unlikely to wake up early.
            // If so we just loop around and wait again
            if(barrier_opening_count > iteration) {
                // barrier is not open for next iteration!
                // free the mutex we just got
                assert(pthread_mutex_unlock(&mutex) == 0);
                // and go on through the barrier
                return;
            } 
        }                
    }
}
