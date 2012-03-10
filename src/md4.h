#ifndef md4_HEADER
#define md4_HEADER

/*
	Only use for md4 hashes < ~50 bytes
	fasssttttttttt
	
*/

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "hashdb.h"
#include "misc.h"


#define md4_A 0x67452301
#define md4_B 0xefcdab89
#define md4_C 0x98badcfe
#define md4_D 0x10325476

#define md4_rotl(x, s) (((x) << s) | ((x) >> (32 - s)))
#define md4_F(x, y, z) ((x & y) | ((~x) & z))
#define md4_G(x, y, z) ((x & y) | (x & z) | (y & z))
#define md4_H(x, y, z) (x ^ y ^ z)
#define md4_l2bendian(x) (((x) << 24) | ((x) >> 24) | (((x) & 0xFF00) << 8) | (((x) & 0xFF0000) >> 8))

struct md4_context
{
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
	uint32_t M[16];
};

/*
* Computes the md4 hash of data, and leaves that hash in context.
* @param context Allocated memory of a md4_context struct. md4_password will
*   fill this context.
* @param data the data to md4 hash
* @param data_len the length of data pointed to by data
*/
void md4_password (struct md4_context * context, unsigned char * data, int data_len);

/*
* Fills out the hashdb_bloom_filter_result_t struct for use in the bloom filter
* @param context the md4_context to use to fill result
* @param result the result to fill
*/
inline void md4_bloom_filter (void * context, hashdb_bloom_filter_result_t * result);

/*
* Compares two md4_context structs and returns -1, 0, or 1. Useful for sorting
*  contexts/creating binary trees from contexts, etc
* @param a first md4_context
* @param b second md4_context
*/
inline int md4_compare (void * a, void * b);

char * md4_to_string (struct md4_context * context);

/*
* Fills out a md4_context struct to reflect an ASCII representation of the
*  md4 hash in base 16 given in a passed string.
* @param context the context that will be filled from the string
* @param string the string used to fill out context. must be null-terminated,
*  and of length 32 (and it helps if it contains a md4 hash :p)
*/
int md4_from_string (struct md4_context * context, char * string);

inline void md4_transform (struct md4_context * context);

#endif
