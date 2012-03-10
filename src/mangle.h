#ifndef mangle_HEADER
#define mangle_HEADER

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANGLE_DEBUG 0

#define MANGLE_WORD_SIZE 2048

#define MANGLE_OPTION_THREADSAFE 1

#define TWO_POW_1  2
#define TWO_POW_2  4
#define TWO_POW_3  8
#define TWO_POW_4  16
#define TWO_POW_5  32
#define TWO_POW_6  64
#define TWO_POW_7  128
#define TWO_POW_8  256
#define TWO_POW_9  512
#define TWO_POW_10 1024
#define TWO_POW_11 2048
#define TWO_POW_12 4096
#define TWO_POW_13 8192
#define TWO_POW_14 16384
#define TWO_POW_15 32768
#define TWO_POW_16 65536
#define TWO_POW_17 131072
#define TWO_POW_18 262144
#define TWO_POW_19 524288
#define TWO_POW_20 1048576

#define LOWER_TO_UPPER_DIFFERENCE 32

#define MANGLE_SPECIAL_CHARS "!$@#%^&*?.+\\-_=`~()|"
#define MANGLE_SPECIAL_CHARS_LENGTH 20
#define MANGLE_SPECIAL_CHARS_FEW "!@#$"
#define MANGLE_SPECIAL_CHARS_FEW_LENGTH 4


#define MANGLE_MAX_RULES 10

typedef struct mangle_data_s
{
	unsigned int options;
	int num_rules; // number of mangline rules (set in mangle_load_from_string)
	int current_rule; // are we currently mangling rule 1, 2, 3, what?
	int rule_seeds[MANGLE_MAX_RULES]; // internal counter for the current rule
	char * plaintext; // this points to the plaintext given in mangle_set_plaintext
	                  // BE AWARE THE PLAINTEXT IS NOT COPIED OVER
	char MANGLE_WORDS[MANGLE_MAX_RULES][MANGLE_WORD_SIZE]; // the current mangled word
	                                   // from each rule is stored here
	int    (* total_functions[MANGLE_MAX_RULES])  (char * plaintext); // pointers to the
	                                   // total_size functions for each respective rule
	char * (* mangle_functions[MANGLE_MAX_RULES]) (char * MANGLE_WORD, char * plaintext, int seed);
	                                   // pointers to the mangle functions for each
	                                   // rule
	pthread_mutex_t lock; // lock for threadsafe operations
} mangle_data_t;



// creates the mangle struct, zeroes things, returns it
mangle_data_t * mangle_data_create ();

// frees all memory
void mangle_data_destroy (mangle_data_t * data);

inline void mangle_options_set (mangle_data_t * data, unsigned int options);

// given a string (like "1,2"), initializes all the mangling rules
int mangle_load_from_string (mangle_data_t * data, char * string);

// this doesn't copy over plaintext, just sets a pointer! user beware
inline void mangle_set_plaintext (mangle_data_t * data, char * plaintext);

// the bread and butter. copies the mangled word over to mangle_word.
// returns 1 on success, 0 on failure
int mangle (mangle_data_t * data, char * mangle_word);


// returns the total number of mangle possibilities for a word



/*
	a = 4
	b = 3
	e = 3
	g = 3
	i = 4
	l = 4
	o = 3
	s = 4
	t = 3
*/
inline void mangle_char (char * c, int * seed);

// MixCaSe
int total_mangles0 (char * word);
char * mangle0 (char * MANGLE_WORD, char * word, int seed);

// 1337
int total_mangles1 (char * word);
char * mangle1 (char * MANGLE_WORD, char * word, int seed);

// append single digit
inline int total_mangles2 (char * word);
char * mangle2 (char * MANGLE_WORD, char * word, int seed);

// append special_M
int total_mangles3 (char * word);
char * mangle3 (char * MANGLE_WORD, char * word, int seed);

// UPPERCASE first character
int total_mangles4 (char * word);
char * mangle4 (char * MANGLE_WORD, char * word, int seed);

// append two digits
int total_mangles5 (char * word);
char * mangle5 (char * MANGLE_WORD, char * word, int seed);

// append year between 1921 and 2020
int total_mangles6 (char * word);
char * mangle6 (char * MANGLE_WORD, char * word, int seed);

// UPPERCASE whole word
int total_mangles7 (char * word);
char * mangle7 (char * MANGLE_WORD, char * word, int seed);

// Prepend a digit
int total_mangles8 (char * word);
char * mangle8 (char * MANGLE_WORD, char * word, int seed);

// Prepend UPPERCASE character
int total_mangles9 (char * word);
char * mangle9 (char * MANGLE_WORD, char * word, int seed);

#endif
