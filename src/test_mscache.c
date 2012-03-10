#include <stdio.h>
#include <string.h>


#include "mscache.h"
#include "nt.h"
#include "md4.h"

int main ()
{

	struct md4_context montext;
	struct nt_context nontext;
	struct mscache_context context;
	char plaintext[] = "password";
	char username[] = "username";
	
	
	md4_password(&montext, (unsigned char *) plaintext, strlen(plaintext));
	printf("%s\n", md4_to_string(&montext));
	
	
	nt_password(&nontext, (unsigned char *) plaintext, strlen(plaintext));
	printf("%s\n", nt_to_string(&nontext));
	
	
	mscache_set_salt(&context, (unsigned char *) username,  strlen(username));
	mscache_password(&context, (unsigned char *) plaintext, strlen(plaintext));
	printf("%s\n", mscache_to_string(&context));
	
	
	return 0;

}
