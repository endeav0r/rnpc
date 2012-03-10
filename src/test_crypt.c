#include <stdio.h>
#include <string.h>
#include <time.h>

#include "crypt.h"

int main ()
{

	struct crypt_context context;
	char plaintext[] = "password";
	
	strcpy(context.salt, "$6$ksdfjsoigh$asdf");
	
	crypt_password(&context, (unsigned char *) plaintext, strlen(plaintext));
	
	printf("%s\n", crypt_to_string(&context));
	
	crypt_from_string(&context, "endeavormac:$6$ksdfjsoigh$Dm7i1XyX1wno8dHD.TtBO7An4227mETbw4ilH5sOM3FL3FgHGpzMfmQormRGKtUE/lxCPrgCcmslJ.KueUezF/");
	printf("%s\n", context.hash);
	printf("to_string: %s\n", crypt_to_string(&context));
	
	int i;
	double johnny_bench_called;
	
	johnny_bench_called = (double) clock();
	for (i = 0; i < 100; i++)
	{
		crypt_password(&context, (unsigned char *) plaintext, strlen(plaintext));
	}
	johnny_bench_called = ((double) clock()) - johnny_bench_called;
	johnny_bench_called /= (double) CLOCKS_PER_SEC;
	johnny_bench_called = 100 / johnny_bench_called;
	printf("%f crypts per second\n", johnny_bench_called);
	
	return 0;

}

