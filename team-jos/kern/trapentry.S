/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps
	
.text

.globl unktraphandler;
.type unktraphandler, @function;
	.align 2;		
unktraphandler:			
	pushl $0;
	pushl $9;
	jmp _alltraps;

	
/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
/*
	TRAPHANDLER(div0handler, T_DIVIDE)
	TRAPHANDLER(div0handler, T_DEBUG)	
	TRAPHANDLER(nmihandler, T_NMI)
	TRAPHANDLER(nmihandler, T_NMI)
	TRAPHANDLER(syscallhandler, T_SYSCALL) */

	TRAPHANDLER_NOEC(th0, 0)
	TRAPHANDLER_NOEC(th1, 1)
	TRAPHANDLER(th2, 2)
	TRAPHANDLER_NOEC(th3, 3)
	TRAPHANDLER(th4, 4)
	TRAPHANDLER(th5, 5)
	TRAPHANDLER(th6, 6)
	TRAPHANDLER(th7, 7)
	TRAPHANDLER(th8, 8)
	//TRAPHANDLER(th9, 9)
	TRAPHANDLER(th10, 10)
	TRAPHANDLER(th11, 11)
	TRAPHANDLER(th12, 12)
	TRAPHANDLER(th13, 13)
	TRAPHANDLER(th14, 14)
	//TRAPHANDLER(th15, 15)
	TRAPHANDLER(th16, 16)
	TRAPHANDLER(th17, 17)
	TRAPHANDLER(th18, 18)
	TRAPHANDLER(th19, 19)
	TRAPHANDLER_NOEC(th48, 48)

	
/*
 * Lab 3: Your code here for _alltraps
 */


_alltraps:
	pushl %ds;
	pushl %es;
	pushal;
	pushl $GD_KD;
	popl %ds;
	pushl $GD_KD;
	popl %es;
	pushl %esp;
	call trap;
