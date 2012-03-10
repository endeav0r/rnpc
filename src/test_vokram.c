#include <stdio.h>

#include "vokram.h"



int main (int argc, char * argv[])
{

	int i;

	vokram_discovery_t discovery;
	vokram_t vokram;

	if (argc != 2)
	{
		printf("usage: %s <wordlist_to_train>\n", argv[0]);
		exit(-1);
	}
	
	vokram_discover(&discovery, argv[1]);
	
	vokram_order(&discovery, &vokram);
	
	for (i = 0; i < 10; i++)
		printf("UPPER->%c\n", (int) vokram.type_frequencies[VOKRAM_UPPER][i]);
	
	for (i = 0; i < 10; i++)
		printf("LOWER->%c\n", (int) vokram.type_frequencies[VOKRAM_LOWER][i]);
	printf("\n");
	
	return 0;

}
