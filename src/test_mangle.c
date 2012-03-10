#include <stdio.h>
#include <string.h>

#include "mangle.h"



int main ()
{
	
	char * mangle_word;
	char plaintext[] = "thisisatest";
	char rule[5];
	strcpy(rule, "4,2");
	
	mangle_data_t * mangle_data;
	
	mangle_data = mangle_data_create();
	
	mangle_load_from_string(mangle_data, rule);
	mangle_set_plaintext(mangle_data, plaintext);
	
	mangle_word = mangle(mangle_data);
	while (mangle_word != NULL)
	{
		printf("%s\n", mangle_word);
		mangle_word = mangle(mangle_data);
	}
	
	return 0;

}
