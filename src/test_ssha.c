#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ssha.h"

int main ()
{

	char plaintext[] = "testing123";
	char hash_string[] = "{SSHA}0c0blFTXXNuAMHECS4uxrj3ZieMoWImr";
	struct ssha_context context1;
	struct ssha_context context2;

	ssha_from_string(&context2, hash_string);

	ssha_set_salt(&context1, (unsigned char *) hash_string, strlen(hash_string));
	ssha_password(&context1, (unsigned char *) plaintext, strlen(plaintext));
	
	printf("%s\n%s\n\n", sha1_to_string(&context1.context),
	                     sha1_to_string(&context2.context));
	
	printf("           %s\n", hash_string);
	printf("generated: %s\n", ssha_to_string(&context1));
	
	printf("comparing... %d\n", ssha_compare(&context1, &context2));
	
	printf("%08x %08x\n", context1.context.H[0], context2.context.H[0]);
	printf("%08x %08x\n", context1.context.H[1], context2.context.H[1]);
	printf("%08x %08x\n", context1.context.H[2], context2.context.H[2]);
	printf("%08x %08x\n", context1.context.H[3], context2.context.H[3]);
	printf("%08x %08x\n", context1.context.H[4], context2.context.H[4]);
	
	return 0;

}

