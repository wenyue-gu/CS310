#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h> 
#include <atomic>
#include <linux/futex.h>

/*

Please use these futex functions rather than calling the syscall directly.

In the AG350 tests we annotate these functions with some extra stuff
for our testing purposes.

man 2 futex can give you the details on the system calls invoked here.

*/


/*

First parameter is the address to use for synchronization.
2nd parameter is the expected value

It's probably not a good idea to use the return value from futex_wait
for much - remember, even if the result is not EAGAIN that's not an
assurance a futex_wake happened.  Sprious wakeups can happen.

 */
int futex_wait(volatile std::atomic<int>* uaddr, int val) {
    // we don't bother with timeouts in the implementation

    int futex_result = syscall(SYS_futex, (int*) uaddr, FUTEX_WAIT_PRIVATE, val, NULL, NULL, 0);
    // EAGAIN indicates we did not match the value
    // result indicates were were woken up
    if(futex_result != 0) {
        if(errno != EAGAIN) {
            perror("error returned by futex wait");
            assert(0);
        }
    }
    return futex_result;
}

/*
First parameter is the synchronization address
Second parameter is the number to awaken

returns the number awoken, not that you usually care
 */
int futex_wake(volatile std::atomic<int>* uaddr, int nr) {
    int futex_result = syscall(SYS_futex, uaddr, FUTEX_WAKE_PRIVATE, nr, NULL, NULL, 0);
    assert(futex_result != -1);
    return futex_result;
}
