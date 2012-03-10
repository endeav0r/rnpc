#include <stdio.h>
#include <string.h>
#include <time.h>

#include "dict.h"
#include "md4.h"
#include "md5.h"
#include "mangle.h"
#include "nt.h"
#include "sha1.h"


int main ()
{

	int i;
	int strlen_plaintext;
	double johnny_bench_called;
	char plaintext[] = "password";
	char * mangle_result;
	struct md4_context context;
	
	for (i = 0; i < 1000; i++)
	{
		mangle_result = mangle(plaintext, i);
		printf("%s\n", mangle_result);
	}
	
	strlen_plaintext = strlen(plaintext);
	
	md4_password(&context, (unsigned char *) plaintext, strlen_plaintext);
	printf("%08x%08x%08x%08x\n8a9d093f14f8701df17732b2bb182c74\n",
	       context.A,
	       context.B,
	       context.C,
	       context.D);


	
	printf("\n-----------------------\n");
	printf("md4 time! hashing: %s\n", plaintext);	
	johnny_bench_called = (double) clock();
	for (i = 0; i < 5000000; i++)
	{
		md4_password(&context, (unsigned char *) plaintext, strlen_plaintext);
	}
	johnny_bench_called = ((double) clock()) - johnny_bench_called;
	johnny_bench_called /= (double) CLOCKS_PER_SEC;
	johnny_bench_called = 5000000 / johnny_bench_called;
	printf("%f md4 hashes per second\n", johnny_bench_called);


	struct nt_context nontext;
	printf("\n-----------------------\n");
	printf("nt hash time! hashing: %s\n", plaintext);	
	nt_password(&nontext, (unsigned char *) plaintext, strlen_plaintext);
	printf("%08x%08x%08x%08x\n8846f7eaee8fb117ad06bdd830b7586c\n",
	        nontext.A, nontext.B, nontext.C, nontext.D);

	johnny_bench_called = (double) clock();
	for (i = 0; i < 5000000; i++)
	{
		nt_password(&nontext, (unsigned char *) plaintext, strlen_plaintext);
	}
	johnny_bench_called = ((double) clock()) - johnny_bench_called;
	johnny_bench_called /= (double) CLOCKS_PER_SEC;
	johnny_bench_called = 5000000 / johnny_bench_called;
	printf("%f nt hashes per second\n", johnny_bench_called);


	
	struct md5_context fivext;
	printf("\n-----------------------\n");
	printf("md5 time! hashing: %s\n", plaintext);	
	johnny_bench_called = (double) clock();
	for (i = 0; i < 5000000; i++)
	{
		md5_password(&fivext, (unsigned char *) plaintext, strlen_plaintext);
	}
	johnny_bench_called = ((double) clock()) - johnny_bench_called;
	johnny_bench_called /= (double) CLOCKS_PER_SEC;
	johnny_bench_called = 5000000 / johnny_bench_called;
	printf("%f md5 hashes per second\n", johnny_bench_called);



	struct sha1_context shontext;
	printf("\n-----------------------\n");
	printf("sha1 time! hashing: %s\n", plaintext);	
	johnny_bench_called = (double) clock();
	for (i = 0; i < 5000000; i++)
	{
		sha1_password(&shontext, (unsigned char *) plaintext, strlen_plaintext);
	}
	johnny_bench_called = ((double) clock()) - johnny_bench_called;
	johnny_bench_called /= (double) CLOCKS_PER_SEC;
	johnny_bench_called = 5000000 / johnny_bench_called;
	printf("%f sha1 hashes per second\n", johnny_bench_called);


	
	printf("\n-----------------------\n");
	printf("mangle time! mangling: %s\n", plaintext);
	
	johnny_bench_called = (double) clock();
	for (i = 0; i < 5000000; i++)
	{
		mangle_result = mangle(plaintext, i);
	}
	johnny_bench_called = ((double) clock()) - johnny_bench_called;
	johnny_bench_called /= (double) CLOCKS_PER_SEC;
	johnny_bench_called = 5000000 / johnny_bench_called;
	printf("%f mangles per second\n", johnny_bench_called);




	
	
	
	printf("\n----------------------\n");
	printf("testing dictionary loader\n");
	
	char * dictionary_word;
	dictionary_file_t * d;
	
	d = dictionary_file_open("test_dict.txt");
	if (d == NULL)
		printf("error opening dictionary\n");
	else
	{
		while (1)
		{
			dictionary_word = dictionary_next_word(d);
			if (dictionary_word == NULL)
			{
				printf("dictionary_next_word returns NULL, no more words!\n");
				break;
			}
			else
				printf("%s\n", dictionary_word);
		}
		dictionary_file_close(d);
	}
		
	
	return 1;

}
