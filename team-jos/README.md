

# A Very Basic OS

This is a lab that will get you building a OS from nearly nothing,
running on an emulator called QEMU that simulates a modern x86 process
(similar to the one that's in your computer, although we'll only use
it in 32 bit mode).  In the process we'll learn a little bit about IO
and a bit about how the OS runs progress and gets invoked through
system calls.

# Setup
You need to have access to a Linux machine. You have three options: (1) use your own Linux machine, (2) install a Linux virtual machine (through virtual box) on your Windows or MAC. 

Though other things will work, [we suggest you use docker desktop as described here for an environment that is very similar to the way your work will be tested](https://users.cs.duke.edu/~chase/cps310/docker.html).

Once you're in your docker environment checkout the repository that contains this file and you're ready to begin!

## Compiling and Running the OS
To compile the OS, cd into the directory you just downloaded and type
```bash
make
```
You should see it compile a lot of stuff, but there should be no errors.  It should finish with "You need to add a Makefile rule for kernel.img!"

# The Bootloader

The first thing that the computer does after powering on is read the
bootloader from the first sector of the harddisk into memory and start
it running. A disk is divided into sectors, where each sector is 512
bytes. All reading and writing to the disk must be in whole sectors --
it is impossible to read or write a single byte. The bootloader is
required to fit into Sector 0 of the disk, be exactly 512 bytes in
size, and end with the special hexadecimal code "55 AA."  Since there
is not much that can be done with a 510 byte program, the whole
purpose of the bootloader is to load the larger operating system from
the disk to memory and execute it.

A mostly-complete bootloader has already been implemented for you.
We'll dig into its code in a second, but for now lets get it running
on QEMU - our emulator.  All the following commands should be run in
the root directory of your OS project (i.e. the one with kern, boot,
subdirectories in it).

First, build the bootloader binary

    make obj/boot/boot
    
That's the code we want, but need to place this code on a simulated
disk.  Let's make a big blank file that will serve as our hard disk.
To make our process easier, we'll make that file in obj/kern/kernel.img.

    dd if=/dev/zero of=obj/kern/kernel.img count=10000 status=noxfer

Now let's place our newly created bootloader in the first sector

    dd if=obj/boot/boot of=obj/kern/kernel.img status=noxfer conv=notrunc

Ok!  Now to run the OS without the graphic interface, you issue the following command.

```bash
make qemu-curses
```
a virtual text-based should pop up, take a little while to boot, and
then fill with 'BLBLBLBL...'.  That indicates that our BootLoader is
starting correctly.

Quit it by saying Ctrl-a x

## Makefile

We'll be rebuilding this image a lot, so let's simplify our lives by
making obj/kern/kernel.img build itself via the Makefile.

The file to edit is kern/Makefrag - there's a comment in the bottom
indicating where you should add your new rule.  Here's what my version
looks like:

    $(OBJDIR)/kern/kernel.img: $(OBJDIR)/boot/boot
	    dd if=/dev/zero of=$(OBJDIR)/kern/kernel.img count=10000 2>/dev/null
	    dd if=$(OBJDIR)/boot/boot of=$(OBJDIR)/kern/kernel.img status=noxfer conv=notrunc

A few details for those unfamiliar with make.

1.  The thing to the left of the : on the first line is the target -
    i.e. the file that this particular make rule builds.  That
    corresponds to obj/kern/kernel.img - the file we want to create.
2.  The things to the right of the colon are dependencies - there's
    only 1 now (obj/boot/boot) but you'll add more soon.  When these
    files change, it will trigger kernel.img to be recreated (what we
    want).  You'll add new dependencies by placing them after the
    current one, separated by spaces.
3.  The lines below are the commands to build the kernel.img.  They
    are indented by TABS (it must be tabs).  You'll add new ones as
    you need.
4.  When you run make, you should see your dd commands running as the
    (mostly) last command.  If you don't something is messed up.
    Usually a "make clean" followed by a make will get you back on the
    right path.

## Basic IO and our bootloader

Bootloaders are often written in assembly but ours is written mostly
in C.  Take a look at boot/main.c - you can see that it's reading
sectors off the disk into memory.  Its loading is the much larger
binary that will be our operating system kernel into memory.

Or actually, that is what it would do, except that right now it:

1.  Fills the screen with BLs
2.  Fails

Exactly where it fails is the question.  To figure that out, let's add
some simple printing capabilities to our bootloader.

### How to do output?

You have to write a special part of memory connected to the video
card. Video memory starts at address 0xB8000. Every byte of video
memory refers to the location of a character on the screen. In text
mode, the screen is organized as 25 lines with 80 characters per
line. Each character takes up two bytes of video memory. The first
byte is the ASCII code for the character and the second byte tells
what color to draw the character. The memory is organized
line-by-line. To draw the letter 'A' at the beginning of the third
line down, you would have to do the following:

1. Compute the address relative to the beginning of video memory: 80 * (3-1) = 160
2. Multiply that by 2 bytes / character: 160 * 2 = 320
3. Convert that to hexadecimal 320 = 0x140
4. Add that to 0xB8000: 0xB8000+0x140 = 0xB8140 - this is the address in memory
5. Write the letter 'A' to address 0xB8140. The letter A is represented in ASCII by the hexadecimal number 0x41.
6. Write the color white (0x7) to address 0xB8141

With this understanding, you should be able to understand how the big BLBLBLBL for loop functions.

### Write a putchar function

This function draws a given character at a particular row and column on
the screen.

BTW, before you do this remove the code that draws BL everywhere, that
was just to make it obvious that the bootloader works.

If you add this function correctly and uncomment the calls in bootmain
you should be able to figure out where your boot loader is stalling.

Note that you have to be careful not to add too much code or the
bootloader will grow beyond max size and won't work anymore.

### Why is it failing?

So your print should reveal that the system is failing on the "magic"
value in the elf header.

What is the problem?  The system expects to load an elf-formatted
executable file starting at the second sector of the hard drive.  This
is the *kernel* - the main executable of our OS.  Unlike the
bootloader, the kernel isn't size limited and can be stored in any way
we find convenient.  We're opting to store it in the ELF (that is,
linux executable) format because that is a really easy one to get gdb
to output.

The good news is that the sector reading and elf format parsing code
is all working well - we just aren't including any kernel on our
simulated disk.

## Adding the Kernel to the Makefile

So the Makefile is already setup to build the kernel elf executable.
To build it type:

    make obj/kern/kernel

It may already be up-to-date if you have done a make all at some point.

To add it to your image file, execute this:

    dd if=obj/kern/kernel of=obj/kern/kernel.img status=noxfer seek=1 conv=notrunc

BTW, this dd command should output way more than one record outputted.
This is because the kernel has the freedom to be much bigger than the
bootloader!

That "seek=1" is the key parameter.  It starts the kernel at sector 1
(e.g. the one after sector 0, which is where our bootloader expects
our kernel to start).

Now if you run "make qemu" you should see a message that says "OS
Started".  Woo!

Add obj/kern/kernel to your img file's dependency list and add that dd
command to the build commands for the image.  Now your img file should
build every time your kernel is updated.

# The Kernel

## I/O

The way an OS displays characters is outputting to video memory exactly
as we did with the putchar functions in the bootloader.  The way it
inputs is by reading from another special part of memory representing
the keyboard.  We could use these basic primitive operations to build
our own I/O functions, eventually getting to something comparable to
printf-style IO functions Linux provides.  We COULD do that, but it
would be tedious and wouldn't teach you much more than how many edge
cases a real IO library needs to account for.

Instead take a look at kern/init.c in the function i386_init.  This is
where our kernel really starts (minus some magic in entry.S we'll talk
about in a future lab).

You should see this line:

    cprintf("OS Started\n");
    
cprintf is a pretty fully featured implementation of printf, including
outputting integers pointers etc.  Everything you need!  You can see
its implementation in console.c if you're curious.  There are also
some useful input functions, should you want that.  The complete list
is in console.h.

## Debugging

One of the bad things about OS coding is that when OSes fail, they
tend to fail in a catastrophic way.  Sometimes (depending on where you
are) it might not even be possible to print.  But we can use a
debugger.  Here's how:

    make qemu-curses-gdb
    
This will start qemu in a special debugging mode.  But it can't run
the debugger itself - it expects to get connected to by a running gdb
on the host computer.  In a different console window, type

    make gdb

If you're using our docker desktop setup, to get another shell while
the first one is busy open a second terminal and type "docker attach
<name of the container>".

You should see a bunch of text, then get connected to a (gdb) prompt.
The whole system is waiting in case you'd like to issue some commands
before the OS begins to run (you do).  Let's set a breakpoint so it
will stop at some point.  Go into init.c and figure out the line that
contains the "OS Started" cprintf.  Then, if the line was 999 you'd
type:

    (gdb) b init.c:999

Of course replace 999 with the true line number.  You should see a
response indicating the breakpoint was created successfully.  Then
type:

    (gdb) continue

Hint "c" would work as well.  This starts the OS running, until we hit
our breakpoint.  Which should happen right away.  Now we can use all
our usual gdb commands.  For example, if you type:

    (gdb) s

You'll step into the printf function, and can start to see how it
works.

From here, if you type:

    (gdb) n
    
...you'll walk through the individual lines of the function (i.e. step
over rather than step into).

Final trick, typing

    (gdb) p varname

Will print the value of a particular variable.

Anywhoo - there's more to be said about gdb but that should get you
started.  There's no particular task I'd like you to use gdb for at
this point but remember that this tool exists when you run into
trouble.

# Running Programs

So having a kernel is cool but the purpose of an OS is running other
programs.  In this lab, our goal will be to:

1.  Load one program into memory
2.  Run it
3.  Have it print something

## Getting hello onto disk

So our OS doesn't yet have a filesystem in an reasonable way - and we
won't build one (perhaps in some distant future, we'll add a lab 2
where you build one).  But if we're going to load a file it has to be
on our disk *someplace*, right?

The program we're talking about using can already be built - do it
like this:

    make obj/user/badhello

The key is we'll put it on this disk at a special fixed location after
the kernel.  But given that the kernel size is going to increase, we
need to put it far enough out it won't overwrite chunks of the kernel.
How far is that?  It depend how much code you expect to write.

With my kernel, it seemed like 2000 sectors gave my kernel enough
space to grow.  So my dd command looks like this:

    dd if=obj/user/badhello of=obj/kern/kernel.img status=noxfer seek=2000 conv=notrunc

You can put the file where you'd like, but just be sure you keep an
eye on the size your kernel builds to and adjust things in case it
gets too large.

Add obj/user/badhello as a dependency of your image in your Makefile and
add the dd command to your set of image build commands.

## Reading hello file into memory

For file loading you will edit kern/init.c.

Now that hello is on disk we need to read it into memory.  We'll
actually do that in two steps:

1. Read the whole file into memory in one giant chunk
2. Copying the various parts of the file to where they need to go in
   system memory
   
This part is about step 1.

To read the file into memory our OS needs to access the disk - that
means the IDE protocol.  But we've actually implemented this for you -
take a look at ide\_read in ide.c/.h.

This takes a sector to start at, a place for the data to go, and a
number of sectors to read.

The place to start at is the sector number where your hello binary
begins on the disk.  You set that with your dd command in the previous
step.

As for the number of sectors to read, this is a tricker question.  If
we had a file system, we'd be able to compute the file's size and use
that.  But without one we have no idea where the file actually ends.
What we'll do is just load MAX\_RW sectors, which is the max number of
sectors that can be loaded in a single ide command.  We'll assume our
program is not going to be more than MAX\_RW sectors, which is a safe
assumption for now.

As for the place for the data to go, you'll have to allocate an array
with MAX_RW*SECTSIZE bytes (BTW, use uint8\_t as the data type to
represent a byte).  Do it as a global in init.c (I've placed a comment
to indicate where).

If you do it correctly, you should be able to find the ELF header in
the binary you loaded:

	struct Elf *header = (struct Elf*) binary;
	if(header->e_magic == ELF_MAGIC) {
		cprintf("I found the ELF header!")
	}

Be sure to test this before you continue on.

## Putting the file data into the correct memory region

    void (*load_code(uint8_t *binary))()

We're going to write a function load_code in init.c (it's already
started, look below i386\_init).  The purpose of this function to take
a binary corresponding to an elf file and copy it into the correct
regions of memory.

You may note that load_code has a really weird looking function
declaration.  This is because it returns a function pointer, and
function pointer syntax in C is super ugly.

A linked program expects to be in a particular region of absolute
memory.  That is, all its globals and functions have been given
explicit addresses.  So to (say) call a particular function your
assembly is "CALL 80018a" with the assumption that the assembly code
for cprintf have already been placed at 80018a (see
obj/user/goodhello.asm for an example of what I'm talking about here).

We've configured our linker to position our program starting from
0x800000 and going upwards in memory for globals.  Our stack will
begin near 0xB00000 and grow downwards - meaning overall there is
about 0x400000 bytes (about 4MB) available for our user programs.

A particular binary format will include blobs of data, and information
about where those blobs of data need to put in memory for the program
to run correctly.  The elf binary format stores an array of program
headers at e\_phoff.  Each of these corresponds to a region that must
be loaded into memory.  I've got some detailed comments in the
load\_code function to help you through it.

Once the data regions are loaded into memory, load\_code has one more
thing to do.  It will return a function pointer to the entry point of
the loaded program.  The entry point is just the first function to run
in the binary - in our user programs it will be called umain.  In elf
binaries, the address of the entry point is in the header.  So you can
do something like this in your load\_code:

    return (void (*)()) header->e_entry;

Implement load\_code. Note that inc/string.h includes implementations
of memcpy and memset.

Once you have it written, you should be able to take the returned
entry point and run it in your i386\_init function.

    void (*loaded_start_func)()  = load_code(binary_to_load);
    loaded_start_func();

If everything is working correctly, you should see "badhello" fill the
screen.  BTW, if you look at badhello.c in the user directory, it's
operation should be obvious to you.

Woo!  Your OS is running a program, just like OSes do!  But there are
some problems.

# User Programs and System Calls

## Running program in user mode

Take a look at user/badhello.c if you haven't yet.  So what makes badhello
bad anyway?  It seems to be running pretty well.  A few things:

1. Badhello is being called like a function from i386\_init.  That
   means its using the kernel's call stack, not its own call stack.
2. Badhello is executing with the permissions of the OS, meaning it
   has the capability to anything the OS can do.  In particular - it
   can access protected memory.  It's accessing the protected video
   memory region 0xB8000 to do its printing.
   
The good news is all these problems have to do with the state of a few
key registers.  The state of the segment registers control if our
programs execute with user or kernel permissions.  The stack register
controls where the stack starts.  We can't control these things
directly from C, but a very small amount of assembly will solve our
problems.

To do this we'll use a handy struct called a Trapframe.  It's
basically a collection of register values.  If you look at
kern/env.h you'll see that there is a function to initialize a new
trapframe with a entry point and a function to run a given trapframe.
Switch your code in i386\_init to initialize a stackframe and run it.
If you do this correctly, you should should get a kernel panic that
looks like this:

    TRAP frame at 0x437de0
      edi  0x00000000
      esi  0x00801000
      ebp  0x00afdffc
      oesp 0x003fffdc
      ebx  0x000b8012
      edx  0x0080005a
      ecx  0x00000062
      eax  0x000b8000
      es   0x----0023
      ds   0x----0023
      trap 0x0000000e Page Fault
      cr2  0x000b8000
      err  0x00000007 [user, write, protection]
      eip  0x00800041
      cs   0x----001b
      flag 0x00000016
      esp  0x00afdff4
      ss   0x----0023
    kernel panic at kern/trap.c:278: unhanlded page fault

Note that it is a page fault and take a close look at the cr2
register.  That register indicates the address that caused the fault.
It's 0xb8000 - the protected address region corresponding to video
memory.  That is to say badhello is running, then crashing when it
attempts to access video memory directly.  That's what we want!

Note that if you get a panic but don't see that special address in
cr2, you've screwed something else up and should debug rather than
going on to the next section.

## goodhello: a program that attempts to use system calls

User programs writing directly to video memory will never be a safe
thing, so we're going to have to stop using badhello.  Let's switch to
goodhello - modify your image build makefile to place goodhello in the
same place on the virtual disk you were previously placing badhello.

If you do it correctly, you should see a kernel panic with the message
"Kernel got unexpected system call 0".

What's going on?

Well, it's not safe to let user programs edit the video memory
directly.  But to allow printing from user programs, there must be
some way to access it indirectly.  This indirect method is going to
have to operate with the permissions of the kernel, but in a very
limited way.

To transition from user mode to kernel mode, we need to execute a
"trap".  A trap is a CPU-supported error handling routine.  We
register them in a big table (e.g. "if a divide-by-zero happens, run
the code at this address").  Only program with kernel permissions can
install a trap, and (if desired) a trap can transition to kernel mode
when it runs its specific code address.

Many things in the trap table occur when a program does something bad
(e.g. divide by zero, attempts to access invalid memory) but a program
can also invoke a specific trap by using the assembly language
interrupt instruction.  We'll designate one special unused trap as a
"system call" trap.  When this trap occurs, we'll switch to kernel
mode and expect that the program is requesting the OS to do some
special operation on its behalf.  We'll do that operation (if its
safe) and then after its done switch back to the running program.

So this is what is happening in goodhello - it's triggering that
special system call interrupt to do printing, but then our kernel
doesn't recognize the system call and panics.  Take a look at the
function trap\_dispatch in kern/trap.c and you can see the trap
handling code.

##  Adding a system call

If you look at systemcall.c, you'll see there are 2 system calls
already implemented (SYS\_cgetc and SYS\_env\_destroy).  Add
SYS\_cputs.  You'll have to both update the sycall function at the
bottom of the file and actually implement the function itself at the
top.  Comments there should let you know what its parameters mean.

Once you add it goodhello should work properly.

# Testing
We use five test cases (test1.c, test2.c, test3.c, test4.c, and test5.c)
to evaluate your solution. We provide the running script in GNUMakefile.
For exmaple, to run the test1, you can use
```bash
make run-test1-nox
```

Each test prints a unique string: (1) OS Started! (2) Test System Call!
(3) Hello World! (4) Page Fault (5) Invalid Pointer.

Please do not hardcode these strings into your source code. We will use
different strings for grading purpose. Each test has 20 points. The overall
lab has 100 points.

# We're done (for now)!

At this point we've got a basic OS that bootloads, runs user programs,
and has basic system calls for input and output.

# Acknowledgments

This lab is a modified version of the jos Lab 2 from MIT.  Although I
have modified the steps and edited the code in various places, all the
credit should go to the original authors and I have kept their names
in place in the source code wherever possible.
