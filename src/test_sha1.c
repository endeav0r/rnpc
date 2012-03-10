#include <stdio.h>
#include <string.h>

#include "sha1.h"

int main ()
{

	//char string[] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
	//char string[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz00112233445566778899";
	char string[] = "hello";
	struct sha1_context context;
	
	sha1_password(&context, (unsigned char *) string, strlen(string));
	
	printf("strlen(string): %d\n", strlen(string));
	printf("\n");
	
	printf("%08x %08x %08x %08x %08x\n",
	        context.H[0],
	        context.H[1],
	        context.H[2],
	        context.H[3],
	        context.H[4]);
	
	return 0;

}
