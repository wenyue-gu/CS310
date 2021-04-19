---
title: Virtual Memory Lab
layout: togit
---

# Virtual Memory Lab

In this assignment, we will be augmenting a language virtual machine.
You can see the details [here](
https://en.wikipedia.org/wiki/Virtual_machine#Process_virtual_machines)
but the basic jist is that a VM is a program that "hosts" programs in
another programming language, providing a device independent layer for
cross platform execution.  For example, the java virtual machine
executes java programs.  A virtual machine is actually similar to an
operating system in a lot of ways, especially when the hosted language
is actually compiled at runtime into assembly language and run
directly on the CPU.  This is known as Just-In-Time compilation and
our VM will do something pretty similar.

# Forth

The language our virtual machine will be hosting is known as
[Forth](https://en.wikipedia.org/wiki/Forth_(programming_language)).
It isn't important for you to fully understand this language, but you
might find it handy to know a bit.  The wikipedia page is a good
introduction, and if you'd like to try some code you can type "make
interactive" with the provided Makefile.  This will build an interactive forth
that you can issue commands to and see output:

    $ make interactive
    ./jonesforth.bin forth/jonesforth.f 
    Welcome to forth! Press ^D to quit. 
    3 2 + .
    5

But when we actually use the code for this project we will want for it to
both take input as C strings and output C strings.  For that, take a
look at [paged\_forth.c](paged_forth.c).  To run the code:

    $ make pagedforth.bin
    gcc -no-pie -ggdb -Wall -c paged_forth.c -o paged_forth.o
    gcc -no-pie -ggdb -Wall -c forth/forth_embed.c -o forth_embed.o
    gcc -no-pie -ggdb -Wall -c forth/myjf.S -o jonesforth.o
    gcc -no-pie -ggdb -Wall -o pagedforth.bin forth_embed.o jonesforth.o paged_forth.o

    $ ./paged_forth.bin 2
    finished loading starter forth
    OUTPUT: 4999 5000 finished successfully 
    done

The comments in [paged\_forth.c](paged_forth.c) should be mostly self explanatory.  There's some interesting stuff in [paged\_forth\_lib.c](paged_forth_lib.c) though.  Look at the bottom the initialize\_forth function.  A few notes:

1. mmap is mostly a more powerful version of malloc.  We'll be using a
   lot of its features in the assignment, so you might want to
   familiarize yourself with the man page.
2. the forth\_data struct is a structure that represents the total
   state of a forth system (sort of similar to the ucontext struct of
   the userspace threading assignments).  However, it does not
   allocate space for the forth returnstack, stack or heap.  This
   memory must be allocated by the user and then be passed to the
   initialize\_forth\_data function which sets the forth_data struct
   up.  The existing code does this for you.
# PART 1

## Part 1: Goals

So like most programs, forth expects memory to be a continuous space
with the stack at the top (high memory) and the heap at the bottom
(low memory).  forth uses both parts of memory.  It uses the stack to
store its data stack.  It uses the heap to store its function
definitions and (some) globals.  These two memory locations grow
towards each other but hopefully never get so large they overlap.

Like an operating system, we want to start the stack and the heap very
far apart to accommodate the largest possible programs.  But also like
an operating system, we'd prefer if we didn't have to allocate all
that memory upfront because most programs won't use all of it.  In the
paged_forth.c right now, we allocate the entire (20 page) memory block
upfront with that call to mmap.

We want to change that.  Here's the plan:

1.  We want to initially pass forth pointers to a memory region that
    is actually invalid
2.  Then we will set up a signal handler that detects segmentation
    faults
3.  We will determine the address that is failing and determine which
    page that bad address corresponds to
4.  We will use mmap to allocate memory for that exact page, at the
    particular address we need it to be (note the mmap must be placed
    at a page boundary)
5.  We will then return from the signal handler and allow the erroring
    code to run again, which should succeed because the requested
    address is now valid
7.  Code will continue until another invalid page is referenced
8.  Eventually we will have loaded only the parts of the stack/heap we
    actually used, while leaving unreferenced memory unallocated

## How to do it

Take a look at [segfault\_catch\_example.c](segfault_catch_example.c).
This shows how to correctly install a signal handler to catch a
segfault and then resume operations after the segfault is resolved.
Use that code as your starting point to make changes to paged_forth_lib.c.

You'll be modifying the code in initialize\_forth to do any setup you need and you will be adding a handler function.

The code in segfault\_catch\_example.c just always maps the same page,
but you'll need to edit it.  Your code will need to use some pointer
math to determine in which page (of the 20 possible) address that
causes the segfault resides.  Then, you'll compute the beginning of
that page, and then you'll mmap it.


You'll want to modify the line in initialize\_forth so that the
stackheap is not actually allocated.

    void* stackheap = mmap((void*) STACKHEAP_MEM_START, stackheap_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);

becomes

    void* stackheap = (void*) STACKHEAP_MEM_START;

When you map a page in your handler, you should call

    page_map_callback(fault_address, pagenum, NO_UNMAP);

This will print out a detailed message (that is also used for our
tests).

## Testing

Start by invoking
    
    $ ./paged_forth.bin 1

If you look at this code for this test case, you'll see it doesn't actually execute forth at all - instead it just explicitly references specific addresses.  That makes it easy to see whats going on if you mess up.

Here's what your output should look like:

    PMC-TC1 Mapped page 19 for address 0xf9f9fff8.
    PMC-TC1 Mapped page 0 for address 0xf9f8c000.
    PMC-TC1 Mapped page 1 for address 0xf9f8d000.
    PMC-TC1 Mapped page 18 for address 0xf9f9efff.
    done

Once that works go ahead and try

    $ ./paged_forth.bin 2
    
You should see a whole bunch of different pages being loaded, but in the end you should get the same output from the correctly running forth program.  We've also provided you with the correct pattern of addresses in the file TC2.output - make sure yours matches before you go on to step 2.

# PART 2

## Part 2: Goals

Now we would like to further improve our forth's memory footprint by
writing infrequently used memory pages to files on the disk.

Here's the plan:

1. Our forth will have a maximum number of pages we want to keep in
   memory at one time - max_pages that is passed to initialize.  This
   can be set to anything 3 or higher (less than that and my
   implementation infinite loops).
2. When we catch a segfault indicating we need to load a new page, we
   will see if we're already at our max pages.
3. If we are, we will first write our oldest memory page onto disk,
   then unmmap it from memory.
4. Then we will map the newly required page into memory.  If it's a
   page we have loaded before, we will load that page from the file
   instead.
5. We'll probably have to update some metadata to keep track of what
   pages have been in memory the longest

## Mmap

This problem is made much easier by the fact that mmap had a feature
that allows us to map a file to a particular region of memory.  If we
do this, the memory region will be populated with the file's contents
when it is created.  Also, any modifications to the memory will
automatically be transmitted back to the file.  So mmap will handle
most of the work of file operations.

*Take a look at [mmap\_file\_example.c](mmap_file_example.c) for details.*

Note that this example removes MAP_ANONYMOUS from the mmap parameters.
This is important because anonymous means "not mapping to a disk a file"
which was true in the first part but is no longer true in part 2.

## Tracking the oldest

It is important that we select the right page to unmap when a new page
needs to be swapped in.  Otherwise we can generate infinite loops.
Imagine for example that we use algorithm that always swaps out the
smallest numbered block.  Imagine our code is attempted to execute an
assembly language instruction stored in block 1 to an address stored
in block
2.

If blocks 1 and 3 and 4 are loaded, this will cause a segfault and
we'll replace block 1 with 2.

Now blocks 2 and 3 and 4 are loaded, this will cause a segfault and
we'll replace block 2 with block 1.

Hopefully you can see the loop.  But if we always swap the oldest
block, we can avoid this problem (regardless of the order in the above
example, we will eventually get to a place where both 1 and 2 are
loaded and we can proceed).  Even better might be to track the most
recently used blocks, but unfortunately our system can only detect
when blocks are loaded not when an already loaded block is accessed.

You can use any mechanism you like to detect the oldest (and they
should all be equivalent) but here's my approach:

I store a "priority" for every page in the system.  When a block
enters, I give it the best priority which is the max allowed number of
blocks.

When I load another page, I loop through all existing pages and drop
their priority by 1.  If any page drops to 0 priority (only one should
at a time at most) I munmap it.

## Page states

A page in this system can be in 3 states - active, never mapped
before, or swapped out.

If you need to swap in a page for the first time, you need to open a
file descriptor and create a file of the appropriate size.

When a page is munmapped, you don't have to lose the file descriptor.
Instead, keep it but (obviously) munumap the memory.  Mark it as
swapped out.

When a page is swapped in, reuse the file descriptor but remmap the
file.

## Testing

Note that you will want to modify your call to page_map_callback to pass an unmap parameter if that call did indeed unmap some page.

First run

    $ ./paged_forth.bin 3
    
My output looks like this:

    PMC-TC3 Mapped page 19 for address 0xf9f9fff8.
    PMC-TC3 Mapped page 0 for address 0xf9f8c000.
    PMC-TC3 Mapped page 1 for address 0xf9f8d000.
    PMC-TC3 Mapped page 18 for address 0xf9f9efff.  Unmapped 19.
    PMC-TC3 Mapped page 19 for address 0xf9f9f000.  Unmapped 0.
    done

Once that works try 

    $ ./paged_forth.bin 3
    
You should again see a correctly running forth program but this time you should see pages both getting unmapped and mapped.  As usual you can see the pattern in TC4.output.

If that looks good, try running the automated tests:

    $ bash tests.bash
    
If those pass you a ready to submit.  Note that our automated tests run with slightly different parameters, so it is possible if your code has unforseen bugs you could pass the tests locally but fail on the automated server.

