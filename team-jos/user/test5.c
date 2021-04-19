#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	char* mem = (char*)0xB8000;
	sys_cputs(mem, 1);
	cprintf("succesfully access video memory at 0xB8000!\n");

	exit();
}
