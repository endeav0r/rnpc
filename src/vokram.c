#include "vokram.h"



inline int vokram_rides_short_bus (char c)
{
	int i;
	for (i = 0; i < strlen(VOKRAM_SPECIAL_CHARACTERS); i++)
	{
		if (c == VOKRAM_SPECIAL_CHARACTERS[i])
			return 1;
	}
	return 0;
}



// given the filename of a wordlist, fills a vokram_discovery struct with
// information
// this function is not in a loop, it is done before the cracking starts, and
// there is a lot going on here. this is written for CLARITY, not SPEED
int vokram_discover (vokram_discovery_t * discovery, char * filename)
{

	int i;
	int characters_processed;
	int first;
	char * word;
	dictionary_file_t * dict;
	
	memset(discovery, 0, sizeof(vokram_discovery_t));
	
	dict = dictionary_file_open (filename);
	if (dict == NULL)
	{
		printf("error opening vokram file %s\n", filename);
		return -1;
	}
	
	word = dictionary_next_word(dict);
	
	characters_processed = 0;
	while (word != NULL)
	{
		for (i = 0; i < strlen(word) % 20; i++)
		{
			// places
			if ((word[i] >= 'A') && (word[i] <= 'Z'))
			{
				discovery->type_places[i][VOKRAM_UPPER]++;
				first = VOKRAM_UPPER;
			}
			else if ((word[i] >= 'a') && (word[i] <= 'z'))
			{
				discovery->type_places[i][VOKRAM_LOWER]++;
				first = VOKRAM_LOWER;
			}
			else if ((word[i] >= '0') && (word[i] <= '9'))
			{
				discovery->type_places[i][VOKRAM_DIGIT]++;
				first = VOKRAM_DIGIT;
			}
			else if (vokram_rides_short_bus(word[i]))
			{
				discovery->type_places[i][VOKRAM_SPECIAL]++;
				first = VOKRAM_SPECIAL;
			}
			else 
				first = VOKRAM_NONE;
			
			// frequencies
			discovery->ascii_frequencies[(int) word[i]]++;
			
			// followers
			if ((word[i+1] >= 'A') && (word[i+1] <= 'Z'))
				discovery->type_followers[first][VOKRAM_UPPER]++;
			else if ((word[i+1] >= 'a') && (word[i+1] <= 'z'))
				discovery->type_followers[first][VOKRAM_LOWER]++;
			else if ((word[i+1] >= '0') && (word[i+1] <= '9'))
				discovery->type_followers[first][VOKRAM_DIGIT]++;
			else if (vokram_rides_short_bus(word[i+1]))
				discovery->type_followers[first][VOKRAM_SPECIAL]++;
			else if (word[i+1] == 0)
				discovery->type_followers[first][VOKRAM_ZERO]++;
		}
		
		word = dictionary_next_word(dict);
	}
	
	dictionary_file_close(dict);
	
	return 0;

}



void vokram_ascii_merge_sort (char * chars, int64_t * score, int length)
{
	
	int size_a;
	int size_b;
	int a_i;
	int b_i;
	int tmp_i;
	char tmp[256];
	char * a;
	char * b;
	
	if (length == 1)
		return;
	
	if (length == 2)
	{
		if (score[(int) chars[0]] < score[(int) chars[1]])
		{
			tmp[0] = chars[1];
			chars[1] = chars[0];
			chars[0] = tmp[0];
		}
		return;
	}
	
	a = &(chars[0]);
	b = &(chars[length / 2]);
	size_a = length / 2;
	size_b = length - size_a;
	
	vokram_ascii_merge_sort(a, score, size_a);
	vokram_ascii_merge_sort(b, score, size_b);
	
	a_i = 0;
	b_i = 0;
	for (tmp_i = 0; tmp_i < length; tmp_i++)
	{
		if (a_i == size_a)
			tmp[tmp_i] = b[b_i++];
		else if (b_i == size_b)
			tmp[tmp_i] = a[a_i++];
		else
		{
			if (score[(int) a[a_i]] > score[(int) b[b_i]])
				tmp[tmp_i] = a[a_i++];
			else
				tmp[tmp_i] = b[b_i++];
		}
	}
	
	memcpy(chars, tmp, length);

}


// once again, this work is done in advance, and we're not overly concerned
// with performance. Some ~inefficient~ sorting may be taking place here
void vokram_order (vokram_discovery_t * discovery, vokram_t * vokram)
{

	int i, j, k;
	int place;

	memset(vokram, 0, sizeof(vokram_t));
	
	printf("type_places\n");
	// type_places
	// 20 * 5 * 5 = 500
	for (i = 0; i < 20; i++)
	{
		// 6 elements? insertion sort
		for (j = 0; j < 5; j++)
		{
			place = 0;
			for (k = 0; k < 5; k++)
			{
				if (discovery->type_places[i][j] < discovery->type_places[i][k])
					place++;
			}
			// check to make sure we haven't already placed something here,
			// IE two types have the same rank
			while(vokram->type_places[i][place] != 0)
				place++;
			vokram->type_places[i][place] = j;
		}
	}
	
	printf("type_followers\n");
	// type_followers
	// 5 * 5 * 5 = 125
	for (i = 0; i < 5; i++)
	{
		for (j = 0; j < 5; j++)
		{
			place = 0;
			for (k = 0; k < 5; k++)
			{
				if (discovery->type_followers[i][j] < discovery->type_places[i][k])
					place++;
			}
			while (vokram->type_places[i][place] != 0)
				place++;
			vokram->type_places[i][place] = j;
		}
	}
	
	printf("type_frequencies\n");
	// UPPER
	for (i = 0; i < 26; i++)
		vokram->type_frequencies[VOKRAM_UPPER][i] = 'A' + (char) i;
	vokram_ascii_merge_sort(vokram->type_frequencies[VOKRAM_UPPER],
	                        discovery->ascii_frequencies, 26);
	
	// LOWER
	for (i = 0; i < 26; i++)
		vokram->type_frequencies[VOKRAM_LOWER][i] = 'a' + (char) i;
	vokram_ascii_merge_sort(vokram->type_frequencies[VOKRAM_LOWER],
	                        discovery->ascii_frequencies, 26);

	// DIGIT
	for (i = 0; i < 10; i++)
		vokram->type_frequencies[VOKRAM_DIGIT][i] = '0' + (char) i;
	vokram_ascii_merge_sort(vokram->type_frequencies[VOKRAM_DIGIT],
	                        discovery->ascii_frequencies, 10);
	
	// SPECIAL
	for (i = 0; i < strlen(VOKRAM_SPECIAL_CHARACTERS); i++)
		vokram->type_frequencies[VOKRAM_SPECIAL][i] = VOKRAM_SPECIAL_CHARACTERS[i];
	vokram_ascii_merge_sort(vokram->type_frequencies[VOKRAM_SPECIAL],
	                        discovery->ascii_frequencies,
	                        strlen(VOKRAM_SPECIAL_CHARACTERS));
	
}
