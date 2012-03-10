#ifndef vokram_HEADER
#define vokram_HEADER

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"

// VOKRAM_ZERO AS IN (char) 0x00

#define VOKRAM_UPPER   0
#define VOKRAM_LOWER   1
#define VOKRAM_DIGIT   2
#define VOKRAM_SPECIAL 3
#define VOKRAM_ZERO    4
#define VOKRAM_NONE    5

// these are the only special characters we are looking for
#define VOKRAM_SPECIAL_CHARACTERS "!$@#%^&*?.+\\-_=`~()|"


#define VOKRAM_DEFAULT_SIZE 64
char VOKRAM_DEFAULT;

// this struct is for parsing a wordlist and gathering the information
// needed for some quick markov-like plaintext generation. we don't use
// this to actuall generate our markov-like plaintexts (come on, we wouldn't
// do that to you with 64-bit ints!)
// discovery, name chosen from daft punk album
typedef struct vokram_discovery_s
{
	int64_t type_places[20][6];
	int64_t type_followers[6][6];
	int64_t ascii_frequencies[256];
} vokram_discovery_t;



typedef struct vokram_s
{
	int  type_places[20][6];
	int  type_followers[6][6];
	char type_frequencies[6][26];
} vokram_t;



// given the filename of a wordlist, fills a vokram_discovery struct with
// information
int vokram_discover (vokram_discovery_t * discovery, char * filename);

// originally was: vokram_is_special
// returns 1 is the character is in VOKRAM_SPECIAL_CHARACTERS
// 0 if it is not
inline int vokram_rides_short_buf (char * c);

// takes the data from vokram_discovery_t and uses it to set up vokram
void vokram_order (vokram_discovery_t * discovery, vokram_t * vokram);

// this function is used to order an array of characters based on a score, IE
// we use this when creating vokram_t from vokram_discovery_s
void vokram_ascii_merge_sort (char * chars, int64_t * score, int length);

char * vokram_make_word (int seed);

#endif
