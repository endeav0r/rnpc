#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char * argv[])
{

	if (argc != 3)
	{
		printf("usage: %s <max_line_length> <filename>\n", argv[0]);
		exit(-1);
	}
	
	FILE * fh;
	char word[1024];
	char new_word[1024];
	int max_length;
	int i;
	
	fh = fopen(argv[2], "r");
	if (fh == NULL)
	{
		printf("could not open %s\n", argv[2]);
		exit(-2);
	}
	
	max_length = atoi(argv[1]);
	
	while (fgets(word, 1024, fh) != NULL)
	{
		if (strlen(word) > max_length)
		{
			strncpy(new_word, word, max_length + 1);
			strcpy(word, new_word);
		}
		
		for (i = 0; i < strlen(word); i++)
		{
			if ((word[i] == '\r') || (word[i] == '\n'))
				word[i] = 0;
		}
		printf("%s\n", word);
	}
	
	return 0;

}
