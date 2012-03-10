#include <stdio.h>

#include "dict.h"

int main (int argc, char * argv[])
{

	char * word;
	dictionary_file_t * d;
	

	if (argc != 2)
	{
		printf("usage: %s <dictionary filename>\n", argv[0]);
		printf("loads a dictionary and runs it through the dictionary code, ");
		printf("printing the words out one on a line.\nUse it to diff ");
		printf("against the original dictionary and insure proper working ");
		printf("of the dictionary code.\n");
		exit(-1);
	}
	else
	{
		d = dictionary_file_open(argv[1]);
		if (d == NULL)
		{
			printf("error opening file: %s\n", argv[1]);
			exit(-1);
		}
		while ((word = dictionary_next_word(d)) != NULL)
		{
			printf("%s\r\n", word);
		}
		dictionary_file_close(d);
	}
	
	return 0;

}
