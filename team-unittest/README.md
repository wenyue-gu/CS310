# Unit Test Lab

In lab section we augment a simple c unit test framework using
processes.

To get stared, compile the code in simple\_test\_lib.c and main.c like this

    make simple_tests.bin

When you run it, you should see something like this:

    starting setup
    starting setup
    starting setup
    starting test 1
    starting test 3
    starting test 2
    Test Failed: test 1 data not set properly
    ending test 2
    Test Passed
    Test Failed: test 3 always fails
    UnitTestLab report: passed 1 failed 2 crashed 0 timed out 0

This code run three tests in parallel using pthreads.  The tests are
run in parallel because they have a long setup method.  Unfortunately,
test1 and test2 modify a shared global memory structure which means
they interfere with each other and one usually fails.  If we made the
tests run serially they'd both pass, but where's the fun in that?

Understand the working of the given test code before you proceed.

#### Step 1: Run tests in parallel using fork instead of pthreads

Rather than using pthreads, we're going to use process parallelism. We
will fork before each test - the child will run the test and the
parent will continue spawning tests until all tests have been spawned
and then wait for the tests to complete with wait.

For this to be a viable strategy, we need to have a relatively limited
number of tests.  Otherwise we'd have to be careful and wait for some
tests to complete before starting more so we aren't starting 5000
processes at once.  But for simplicity we'll say we can safely start
every thread in parallel.

To make this change, you'll have to completely rewrite the code in
run\_all\_tests to remove all the pthreads stuff and use fork/wait
instead.  You'll also be able to remove the run\_test function.

When you finish, the results should look like this:

    starting setup
    starting setup
    starting setup
    starting test 1
    starting test 2
    starting test 3
    ending test 1
    ending test 2
    Test Done
    Test Done
    Test Done
    UnitTestLab report: passed 0 failed 0 crashed 0 timed out 0

You'll notice a couple things about this output:

1. Test 1 and Test 2 now pass and Test 3 fails (if you look at the
   code you'll see Test 3 always fails).
2. We aren't displaying Passed/Failed or the error message.  We also
   aren't correctly counting results in the report. We'll handle that
   in subsequent steps.  Note that the "Test Done" message is printed
   by the parent thread when a wait() completes, not by the children
   processes.  That's the way we want you to do it to facilitate the
   later steps.

#### Step 2: Getting test results

Using processes has made test 1 and test 2 pass but it's made other
things harder.  We used to pass around strings to get the test results
but now that our tests run in child processes we can't access the memory
of the children.

We'll start by using one part of our children that we can easily
access - the exit code.  We'll make the children exit with a code of 0
(success) if the test passes but 1 if the test failed.  So in the
child our code will look something like this:

    char* tresult = test_funcs[i]();
    if(tresult == TEST_PASSED) {
        exit(0);
    } else {
        exit(1);
    }

Then we'll use the stat_loc output that we can retrieve from wait to
access the access code.  Hint: check out "main 3 wait" for the details
and pay particular attention to WEXITSTATUS.

Once we get this working our output will look like this (this shows
test 1 and 2 passing and test 3 failing):

    starting setup
    starting setup
    starting setup
    starting test 1
    starting test 2
    starting test 3
    ending test 1
    ending test 2
    Test Passed
    Test Failed
    Test Passed
    UnitTestLab report: passed 2 failed 1 crashed 0 timed out 0

Note that:

1. The tests complete in a random order so it's not clear what test
   failed or succeed when we return from wait.  We'll leave that till
   step 6.
2. We can't print the text of the error, because we only have the exit
   code and there's no way to pass a string through that.  We'll leave
   that till step 6.
3. The report should now look right - make sure you're incrementing
   the correct variables so it does.

#### Step 3: Run tests in parallel, but only do the setup once

When you fork, your child is a new process with a separate memory from
the parent -- we're using that fact to let test 1 and test 2 work
correctly.  But interestingly, that separate memory is initially a
copy of the parent's memory...we can exploit that.

Modify the code so that the parent runs setup before forking, and
utilizes the memory "copy" of fork to give each process its own setup
memory space while only having to run setup once.

This should be a small change, and the results should look something like this:

    starting setup
    starting test 1
    starting test 2
    starting test 3
    ending test 1
    ending test 2
    Test Passed
    Test Failed
    Test Passed

Note that setup only happens once.

#### Step 4: Have a special result for crashed tests

In C unit tests, if one test crashes can cause a segmentation fault or
other error that crashes the test runner as well and stops other tests
from running.  But when we run our tests in a separate process, we are
protected if a child crashes.

Uncomment test 4 in the main.c main which always crashes.

Then modify your code in run\_all\_tests so that it detects a crash
and displays an appropriate message.

To detect a crash, you won't be able to look at the status of a
crashed process because status is something that only gets set if your
process exits normally.  Instead, take a look at WIFEXITED in wait
manpage.

When you've finished the output should look similar to this:

    starting setup
    starting test 1
    starting test 2
    starting test 3
    starting test 4
    ending test 1
    ending test 2
    Test Crashed
    Test Passed
    Test Failed
    Test Passed
    UnitTestLab report: passed 1 failed 2 crashed 1 timed out 0

#### Step 5: Use pipes to display test errors

For this stage, we'll use interprocess communication to pass the error
result from the child to the parent.  This step requires more code
than the others.

The mechanism we will use is Unix pipes.  Pipes create a communication
channel between to processes, where one process writes to the pipe and
the other reads from the pipe.  Take a look at "man 3 pipe" which
includes some good discussion plus example code.

In our solution, if an error occurs the test process should use a pipe
to communicate the error text back to the parent.  Something like this:

    char* tresult = test_funcs[i]();
    if(tresult == TEST_PASSED) {
        close(pipeResult[1]);
        exit(0);
    } else {
        write(pipeResult[1], tresult, strlen(tresult));
        close(pipeResult[1]);
        exit(1);
    }

To do this successfully in the parent, we will need to keep an array
of pipe values for each of the children because we don't know which
children might fail.  We will also need to loop through the children
in order, rather than just allowing wait to handling finishing
processes as they are done (otherwise we don't know what pipe in our
array to check once we detect a failure).

To do this, we'll store the child process ids in the parent and then
use the command waitpid (as usual, look at the man page for details)
to wait for a particular process to finish.

If you get garbage printing after your test message, ensure your
string is properly terminated with a 0 character.  You can either do
this by sending the 0 character over the pipe, or by figuring out how
many characters are read from the pipe (its the return value of read
and then inserting a 0 character after those characters in your input
array).

When you're done the output should look like this:

    starting setup
    starting test 1
    starting test 2
    ending test 1
    starting test 3
    ending test 2
    starting test 4
    Test Passed
    Test Passed
    Test Failed: test 3 always fails
    Test Crashed
    UnitTestLab report: passed 2 failed 1 crashed 1 timed out 0

Note that that test failed message is still being printed by the parent NOT the child.

#### Step 6: Stop tests that run forever after 3 seconds

Now that we've moved to processes, we also have the ability to stop
tests that run in an infinite loop.  We want to add the feature that
any test that runs longer that 3 seconds will be aborted.

Uncomment test 5 which enters an infinite loop.

The easiest way to do this is to schedule a alarm signal in the child.
Then I handle that alarm signal to cause a special exit code.

[You can see an introduction to signals
here.](https://rhit-csse.github.io/csse332/Homework/UserspaceThreadsLab2/sigintro.html)


When you finished this the output should look similar to this:

    starting setup
    starting test 1
    starting test 2
    ending test 1
    starting test 3
    ending test 2
    starting test 4
    starting test 5
    Test Passed
    Test Passed
    Test Failed: test 3 always fails
    Test Crashed
    Test Timed Out
    UnitTestLab report: passed 2 failed 1 crashed 1 timed out 1

# Running tests

One you can the simple tests working try the automated unit tests.  You can run

    bash tests.bash
    
to try them out.  Getting the tests to work shouldn't require any additional features.



