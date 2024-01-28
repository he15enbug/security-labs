#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
void main(){
	char* shell = getenv("MYSHELL");
	// **IMPORTANT: this is only suitable for 32-bit environment, add -m32 flag when compiling**
	if(shell) printf("%x\n", (unsigned int)shell);
}
