/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/magic.h>

#include <kern/env.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>

// Quits everything.  In a future version of the code where we have
// multiple programs running then we'll use the parameter.  But for
// now there is only one program.
static int
sys_env_destroy(envid_t envid)
{
	int r;
	struct Env *e;
	cprintf("program quit.  OS gracefully infinite looping...\n");
	cprintf("If you prefer, you could call shutdown to quit qemu...\n");
	//while(1);

	//shutdown();
	//panic("we should never get here");
	return 0;
}

// Print a string to the system console.
// The string is exactly 'len' characters long.
// Panics on memory errors.
static void
sys_cputs(const char *s, size_t len)
{
  // TODO: YOUR CODE HERE
	// you should be able to use cprintf to print the string
	
	// BUT before you do check that the user has permission to
	// read memory [s, s+len).  Hint: in our OS, user programs
	// always can only access memory in the range
	// 0x800000-0xB00000
	// if the application wants to print outside its memory region
	// print INVALID_POINTER in magic.h

	// cprintf("%d\n",(int)s);
	// cprintf("%d\n",len);
	if((uintptr_t)s<0x800000 || (uintptr_t)s+len >=0xB00000){
		cprintf(INVALID_POINTER);
		panic("something went wrong");
		return;
	}
	cprintf(s);
}

// Read a character from the system console without blocking.
// Returns the character, or 0 if there is no input waiting.
static int
sys_cgetc(void)
{
	return cons_getc();
}

static void
sys_test(void)
{
	cprintf(SYS_TEST);
}

// Dispatches to the correct kernel function, passing the arguments.
int32_t
syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	
	switch (syscallno) {
  // TODO: YOUR CODE HERE
	
	case SYS_cputs:
		sys_cputs((char *)a1,a2);
		return 0;
	case SYS_env_destroy:
		return sys_env_destroy((envid_t) a1);

	case SYS_test:
		sys_test();
		return 0;

	}
	cprintf("Kernel got unexpected system call %d\n", syscallno);
	return -E_INVAL;
}

