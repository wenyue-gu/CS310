#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	char* mem = (char*)0xB8000;
	char a = *mem;
	cprintf("succesfully access video memory at 0xB8000! The value is %c\n", a);

	exit();
}
