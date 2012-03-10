#include "salt.h"


// (SALT . PLAINTEXT)
void salt_1 (unsigned char * plaintext, int plaintext_length,
             unsigned char * salt     , int salt_length,
             unsigned char * salt_word)
{
	
	strcpy((char *) salt_word, (char *) salt);
	strcat((char *) salt_word, (char *) plaintext);
	
	#if SALT_DEBUG == 1
		salt_word[plaintext_length + salt_length] = 0;
	#endif

}
