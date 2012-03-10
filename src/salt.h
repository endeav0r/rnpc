#ifndef salt_HEADER
#define salt_HEADER

#include <stdlib.h>
#include <string.h>

#define SALT_DEBUG 0

#define SALT_WORD_SIZE 1024

// (SALT . PLAINTEXT)
void salt_1 (unsigned char * plaintext, // plaintext 
            int plaintext_length, // length
            unsigned char * salt, // the salt
            int salt_length, // length
            unsigned char * salt_word // where we store the result
            );
               
#endif
