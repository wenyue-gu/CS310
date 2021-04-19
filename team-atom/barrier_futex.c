#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h> 
#include <atomic>

/*

In this code please use these functions (implemented in futex.c) for
accessing the linux futex system call (there are some helpful comments
in that file as well).

 */

int futex_wait(volatile std::atomic<int>* uaddr, int val);
int futex_wake(volatile std::atomic<int>* uaddr, int nr);


/* 
   Note that this particular barrier implementation only allows one
   barrier (but one can be reused, as long as the iteration variable
   is keep up to date).

   To be slightly fancier, we could put these various fields in a
   struct and then this would be library that would allow several
   different barriers 
*/

// how many threads are there total
// i.e. the barrier will open when threadnum threads are waiting
int threadnum;

// some variables suitable for syncronizing your threads:

//how many threads are currently waiting to pass through the barrier?
volatile std::atomic<int> threads_at_barrier;
//how many times has the barrier opened?
volatile std::atomic<int> barrier_opening_count;


volatile std::atomic<int> had_opened;

// You can add more variables if you want (though we don't think they
// are needed) but please use these variables.  We rely on them being
// set in our tests.

// place any useful initialization code you need here the parameter is
// the number of threads that will be waiting for the barrier.
//
void initialize_barrier(int threadnum_input) {
    threadnum = threadnum_input;
    threads_at_barrier = 0;
    barrier_opening_count = 0;
    had_opened = 0;
}

/*

  The basic operation of a barrier is that the barrier does not open
  until all threads are ready to pass through the barrier (i.e. all
  threads have called wait_at_barrier).  Once the last thread calls
  wait_at_barrier, the barrier is considered open and the threads
  waiting will unblock.

  A reusable barrier is the same, except after opening once it can be
  used again (just be sure to pass the next iteration number).

  The interation parameter indicates how many times a particular
  thread has waited at this barrier before.

  So the first time the thread calls wait_at_barrier the value should
  be 0, 2nd time 1, etc.

 */
void wait_at_barrier(int iteration) {
    // sanity check on the algorithm: if we're just about to start
    // waiting at the barrier, the barier couldn't have opened yet
    // right?
    int local_open_count = atomic_load(&barrier_opening_count);
    assert(local_open_count == iteration);

    // another sanity check:
    // surely less than threadnum threads are waiting now
    int local_thread_count = atomic_load(&threads_at_barrier);
    assert(local_thread_count >= 0);
    assert(local_thread_count < threadnum);

    atomic_fetch_add(&threads_at_barrier,1);

    if(atomic_load(&threads_at_barrier)==threadnum){
        atomic_store(&threads_at_barrier,0);
        atomic_fetch_add(&barrier_opening_count,1);
        atomic_store(&had_opened,1);
        futex_wake(&had_opened,1);
    }
    else{
        atomic_store(&had_opened,0);
        while(1){
            futex_wait(&had_opened,0);
            if(barrier_opening_count > iteration){
                futex_wake(&had_opened,1);
                return;
            }
        }
    }
    
}
