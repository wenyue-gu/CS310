/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_ENV_H
#define JOS_KERN_ENV_H

#include <inc/env.h>

extern struct Segdesc gdt[];

extern struct Trapframe env_tf;

void env_init(void);
void initialize_new_trapframe(struct Trapframe *tf, void (*entry_point)());
void run_trapframe(struct Trapframe *tf) __attribute__((noreturn));

#endif // !JOS_KERN_ENV_H
