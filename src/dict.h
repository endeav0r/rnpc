#ifndef dict_HEADER
#define dict_HEADER

#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DICTIONARY_OPTIONS_THREADSAFE 1


#define DICTIONARY_BUFFER_SIZE 33554432

typedef struct dictionary_file_s
{
	unsigned int options;
	int buffer_cursor; // where we read from next
	int buffer_size; // the size, in bytes, of actual data in the buffer
	int eof; // 0 = NOT eof, 1 = eof
	char buffer[DICTIONARY_BUFFER_SIZE]; // I/O buffer from file
	pthread_mutex_t lock;
	FILE * fh; // it's a koala bear, figure it out
} dictionary_file_t;



// pass filename. returns a dictionary_file_t object for you on success, and
// a big, happy NULL on failure
dictionary_file_t * dictionary_file_open (char * filename);

// sets the options variable to the bassed bitmask
inline void dictionary_set_options(dictionary_file_t * d, unsigned int options);

// pass it your valid dictionary_file_t object, and you get a pointer to a word
// from the dictionary. returns NULL when no more words exist
// this will not block even if the threadsafe option is set, and IS NOT threadsafe
char * dictionary_next_word (dictionary_file_t * d);

// same as above, but copies over result to a passed char array. this way, when
// all the interntal memory shifts around, your pointed-to word won't be lost
// will copy up to word_length bytes (includes null-terminator)
// returns 1 on success, 0 on failure (IE no more words)
// make sure DICTIONARY_OPTIONS_THREADSAFE is 1 if you want this to block
int dictionary_next_word_threadsafe (dictionary_file_t * d, char * word, int word_length);

// it became apparent that threaded applications, when bouncing the dictionary_file_t
// back and forth between cores, were bouncing it in and out of cache. This was bad.
// Here, we can load several lines at once in a 2-dimensional array passed from a thread,
// and the thread will retain the dictionary_file_t object until its array is filled,
// thereby, HOPEFULLY minimizing cache hits
// words = 2d char array already allocated, word_length = length of each word,
// num_words = number of words.... words[num_words][word_length[
int dictionary_next_word_threadsafe_many (dictionary_file_t * d, char ** words,
                                          int word_length, int num_words);

// closes the dictionary and frees the memory
void dictionary_file_close (dictionary_file_t * d);

#endif
