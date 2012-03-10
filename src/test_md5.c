#include <stdio.h>
#include <string.h>


#include "md5.h"

int main ()
{

	struct md5_context context;
	char plaintext[] = "rainbowsandpwniespassword";
	
	char md5_of_password[] = "5f4dcc3b5aa765d61d8327deb882cf99";
	
	md5_password(&context, (unsigned char *) plaintext, strlen(plaintext));
	
	printf("%08x%08x%08x%08x\n%s\n",
	       context.A,
	       context.B,
	       context.C,
	       context.D,
	       md5_of_password);
	
	return 0;

}
