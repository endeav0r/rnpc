#include "brute.h"

inline int64_t total_brutes (char * charset, int length)
{

	int i;
	int64_t strlen_charset;
	int64_t result;
	
	strlen_charset = (int64_t) strlen(charset);
	result = (int64_t) 1;
	for (i = 0; i < length; i++)
		result *= strlen_charset;
	
	return result;

}

inline char * brute (char * charset, int length, char * brute_word, int64_t seed)
{

	int i;
	int64_t strlen_charset;

	strlen_charset = (int64_t) strlen(charset);
	
	for (i = 0; i < length; i++)
	{
		brute_word[i] = charset[seed % strlen_charset];
		seed /= strlen_charset;
	}
	
	brute_word[i] = 0;

	return brute_word;

}
