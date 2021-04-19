# Concurrency with Atomic Instructions

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Concurrency with Atomic Instructions](#concurrency-with-atomic-instructions)
- [A Lock-free Queue](#a-lock-free-queue)
    - [Compare and Swap](#compare-and-swap)
    - [The Algorithm](#the-algorithm)
    - [Step 1: Dequeue](#step-1-dequeue)
        - [Hints](#hints)
    - [Step 2: Enqueue](#step-2-enqueue)
- [Futex](#futex)
    - [Authors](#authors)
    - [Futex solution](#futex-solution)
        - [Atomic operations](#atomic-operations)
        - [Futex operations](#futex-operations)
        - [Some hints](#some-hints)
        - [Results](#results)
- [Conclusion](#conclusion)

<!-- markdown-toc end -->


As we discussed in class, concurrency primitives like mutexes are possible on multi-cpu systems because of specialized atomic instructions that provide guarantees about memory ordering that ordinary instructions don't have.  But building mutexes is not the only use of these instructions - in this lab, we're going to use them to build some other cool stuff:

1.  A lock-free queue
2.  A more efficient barrier for a concurrent algorithm

# A Lock-free Queue

If you want to make a data structure like a linked list concurrent, its easy to just add a mutex to protect its operations.  The bad news about this approach is that if you're doing a lot of concurrent operations, your threads are going to frequently put in a waiting state as they fight for the mutex.

A lock free structure can ensure that even when multiple threads are accessing a common structure, none of them need to wait (though sometimes their operations might fail and need to be retried).  This means that threads can stay scheduled and avoid the overhead of process switching.

## Compare and Swap

In a lock-free data structure we use specialized guaranteed atomic CPU instructions to provide safe concurrency.  In this particular case we're going to use the compare-and-swap instruction which is a single CPU instruction that operates like this:

    bool CAS(int* addr, int expected, int value) {
       if (*addr == expected) {
          *addr = value;
          return true;
       }
       return false;
    }

Of course, if we were to write that code in C it wouldn't be atomic...you need the magic instruction to do it for you.  It's important to understand the operation though: basically it says "if the value in memory location X is Y change it Z, otherwise don't change it".

We'll be using the giant-size version of this operation that works on 128-bit integers.  This instruction can be invoked from within gcc like this:

    bool __sync_bool_compare_and_swap (__int128 *addr, __int128 expected, __int128 value)

128-bits? The reason we need this giant instruction is to
compare-and-swap 2 64-bit words as a unit.  The first 64-bits will be
a pointer, the second will be a modification counter.  The
modification counter will let us check for unexpected updates - every
time we update the value we will increment the counter.  That means
that if other threads change the pointer in the meantime, even if they
reset to its original value we we'll still get a cas failure because
the modification count will be higher than we expect.

We won't require you to do the annoying bitshifting necessary to pack
two 64 bit values into a 128 bit number.  Instead, we've created a
type pointer_t that represents both fields as 64 bit values and then
does the conversion for you:

    typedef struct {
        struct node_t *pointer;
        uint64_t counter;
    } __attribute__ ((aligned (16))) pointer_t;
    
    // some other stuff omitted
    
    bool cas(pointer_t *src,
             pointer_t expected,
             pointer_t value)
    {
        // feel free to learn about union types but not really necessary 
        // for this class
        union union_type uexp = { .parts = expected };
        union union_type uvalue = { .parts = value };
        return __sync_bool_compare_and_swap((__int128*) src,
                                            uexp.as_int128,
                                            uvalue.as_int128);
    }


That cas function is the one you should use.  But bear in mind we
can't compare and swap arbitrary sized structs - its only because a
pointer_t type can be transformed into a contiguous 128bit number that
this system works.

## The Algorithm

We are going to build a very simple lock free data structure - a
lock-free FIFO queue.  The queue has only 4 operations initialize
(creates the queue), destroy (frees any resources the queue has),
enqueue (inserts value at beginning), and dequeue (removes and returns
value at end).  Only enqueue and dequeue need to be safe to run in
parallel.

With mutexes this structure would be easy to implement but lock-free
is much trickier.  We don't want you develop it yourself: instead read
the algorithm described in [sections 2 - 3 of
this paper](https://gitlab.oit.duke.edu/mph13/compsci310-fall20/-/blob/master/1996_PODC_queues.pdf).  Read through the pseudocode (and comments) provided
carefully and understand what each line does.

## Step 1: Dequeue

To build everything:

    $ make
    $ ./queue_test.out

If all is well you should see a message saying that one test passes.
The reason there is a passing test is because we've provided you with
a basic queue implementation.  This implementation is not safe for concurrent dequeuing though - to see this in action edit queue_test.c's main function (at the bottom) and uncomment the test labeled test\_dequeue\_parallel.

If you run it now you'll probably see a segmentation fault or failure (if not, you might need to tweak the VALUES_PER_THREAD and NUMBER_OF_THREADS at the top of the test file).  This segmentation fault is something gdb won't help you with - at least on my system, the tests pass in debug mode because the debugger reduces the amount of parallelism.  This bug is caused when parallel operations break the queue.

To fix it, implement the solution from [the paper](https://gitlab.oit.duke.edu/mph13/compsci310-fall20/-/blob/master/1996_PODC_queues.pdf) in the dequeue function in queue.c.  Note that although the existing codes uses pointer\_t values, it ignores the counter variable.  Your new version will have to use it.

### Hints

1.  I wrote a function that looks like this:

        bool update_and_increment(
            pointer_t *src,
            pointer_t old_value,
            node_t *new_pointer)

That gets the counter variable from old value, makes a new pointer_t
with the new pointer and the incremented counter, and calls cas.
You'll use that operation in a bunch of places.

2.  Your function should return DEQUEUE\_FAIL if the list is empty.
If it succeeds it should return the value.  Some tests rely on this
(i.e. they don't treat a failure to read as a problem necessarily).
However, having a cas failure is not the same as an empty list - you
should be retrying those (look at the given algorithm for details).

3.  Don't forget to free a node object on dequeue - otherwise your
queue will leak memory.

4.  If you make a mistake, figuring out what's going on can be hard
because doing stuff like reducing the number of threads (so you can
make sense of printed output) tends to disguise the problem.  My best
trick: add assertions to your function - e.g. assert that pointer xyz
is not null.  Once you find an assertion failure, add more assertions
to figure out how that broken state is getting setup.

5.  It can be handy sometimes to recomment out the 2nd test and make
sure the first test is still passing.  If not, your issue isn't
parallelism - you've just broken the list.  The good news is this kind
of problem is much easier to debug.

Once you get things working, the initial test and the dequeue test
should pass.

## Step 2: Enqueue

Uncomment the test_enqueue_parallel test.  You'll probably see a
segmentation fault.  Fix it using the given algorithm.

Once you've gotten that working, you should be able uncomment the
final test which runs both enqueue and dequeue in parallel.  Note that
this test can easily infinite loop on bugs (because dequeues keep
retrying on fails until all the expected values are loaded).

Note that though you're welcome to edit queue_test.c to add additional
information (and of course enable specific tests) only modifications
to queue.c will be picked up by the autograder.


# Futex

While lock-free structures are great at what they do, sometimes
threads do need to wait.  When waiting is occurring, generally you
want the OS to be involved so the waiting process does not get
scheduled (and needlessly consume CPU).  A OS-provided synchronization
primitive like a mutex will do the job, but that involves invoking a
system call on every operation.  A futex is a low-level waiting
mechanism designed to be combined with atomic operations to build
custom synchronization primitives or for specialized concurrent
algorithms that need to be fast.

The 2 futex operations we are about are futex wake and futex wait.
The futex lectures cover most of what you need - the man page also has
the detailed specifics (for example here
https://linux.die.net/man/2/futex).

## Authors

To give you experience working with futex we're going to use a problem
that's well adapted to the futex.  Build and run the authors problem like this:

    make authors_condition_var.out
    ./authors_condition_var.out

Then look at the detailed comments at the top of the authors.c file,
which explains how we want the concurrency to work.

The basic idea is we want a "barrier" - an individual thread can't
pass through the barrier, it must wait until all threads are at the
barrier.  Once every thread is at barrier, the barrier opens and now
all the threads go on to the proofreading stage.

This barrier is "reusable" - that is, after proofreading paper 0 a
thread goes on to write paper 1 and again encounters the barrier.
However, although the barrier has opened once it still blocks for the
2nd iteration.  Make sure you understand how the given code
accomplishes this.

The authors\_condition\_var code uses condition variables/mutexes and
is correct.  The code for the barrier itself is the
barrier_condition.c.  You'll want to fully understand how the
condition variable solution functions before you attempt to write your
futex solution.  Feel free to tweak the sleeps in the author function
and adjust the number of threads and papers to understand how it
operates under different conditions.  Then when you're finished define
the variable PERF\_TESTING at the top of the file and put in some
large values for the threads and papers.  Note how long it takes to
run - hopefully we'll improve that in our futex version.

## Futex solution

Build the futex solution like this:

    make authors_futex.out
    ./authors_futex.out


Your goal is to solve the authors problem using atomic operations and
futexes rather than condition variables and mutexes.  You'll have to
come up with the approach yourself, though it is a little similar to
the given condition variable system.

Write your code in barrier\_futex.c - there's a couple comments there
to help.  Also note that though you can modify authors.c and futex.c
to help you debug, only barrier\_futex.c will be used by the
autograder.

### Atomic operations

For this problem we're going to use some C++ atomic operation builtins
rather than cas.

You can see the full list of them here (plus examples):

https://en.cppreference.com/w/cpp/atomic/atomic

My solution only uses atomic\_load atomic\_store and
atomic\_fetch_add.

### Futex operations

futex.c has a futex\_wait and a futex\_wake function that simplify
calling the futex operations.

### Some hints

1.  You do have to the consider the case where a the last thread
 arrives at the barrier after other threads have added to the count
 but before they have actually called futex\_wait.  The value passed
 into futex\_wait should prevent broadcasts from getting "lost".

2.  Do not attempt to build futex equivalents of condition variables
and mutexes (or crib implementations from the internet).  This problem
is directly solvable with futexes/atomic operations, and isn't too
complicated.

3.  In situations with lots of threads, it is usual for futex_wait to
have spurious returns (i.e. return without a corresponding broadcast).
Your code must correctly discover this and re-wait.

4.  Be sure to robustly test your solution with small numbers of
threads and printing before you turn on PERF\_TEST mode.  Adjust the
sleep values to account for various edge cases, etc.  It is very hard
to tell if your algorithm is working right in PERF\_TEST mode.

### Results

Once you've gotten it working, turn your algorithm to PERF\_TEST mode
and see if you got a speedup verses condition variables and mutexes.

The autograder tests simply runs the authors problem with a few extra
assertions to ensure you're taking the right approach.  So once your
barrier\_futex.c code can run correctly in PERF\_TEST mode you should
be good to submit.

# Conclusion

Hopefully this lab has given you some respect for the potential (and
complexity) of using low level concurrency instructions.

A few takeaways we hope you'll remember:

a. Concurrency primitives are implemented by using a small number of
concurrency-safe CPU instructions

b. Its possible to use these instructions to build useful things like
lock-free data structures and very efficient concurrent algorithms

c. These structures and algorithms are much more complex and error
prone than systems that use higher-level concurrency primitives like
mutexes/condition variables.  It best to hand create them yourself
only when critically necessary.
