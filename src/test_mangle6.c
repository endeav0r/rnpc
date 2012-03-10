#include <stdio.h>
#include <time.h>

#include "mangle.h"



int main ()
{

	int i;
	int possibilities;
	char plaintext[] = "password";
	
	possibilities = total_mangles6(plaintext);

	for (i = 0; i < possibilities; i++)
		printf("%s\n", mangle6(plaintext, i));
	
	
	printf("\n--------------------------\n");
	printf("testing for speed\n");
	
	double johnny_bench_called;
	
	johnny_bench_called = (double) clock();
	for (i = 0; i < 10000000; i++)
	{
		mangle6(plaintext, i);
	}
	johnny_bench_called = ((double) clock()) - johnny_bench_called;
	johnny_bench_called /= (double) CLOCKS_PER_SEC;
	johnny_bench_called = 10000000 / johnny_bench_called;
	printf("%f mangles per second\n", johnny_bench_called);
	
	
	return 0;

}
