#ifndef brute_HEADER
#define brute_HEADER

#include <inttypes.h>
#include <string.h>

#define BRUTE_WORD_SIZE 64

inline int64_t total_brutes (char * charset, int length);

inline char * brute (char * charset, // characters to use in a null-terminated string
                     int length, // length of word to make
                     char * brute_word, // where we store the resulting word
                     int64_t seed // a unique number that we use to make this word
                     );

#endif
