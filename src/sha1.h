#ifndef sha1_HEADER
#define sha1_HEADER

/*
	Only use for md4 hashes < ~50 bytes
	fasssttttttttt
	
*/

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "hashdb.h"
#include "misc.h"



#define sha1_H0 0x67452301
#define sha1_H1 0xEFCDAB89
#define sha1_H2 0x98BADCFE
#define sha1_H3 0x10325476
#define sha1_H4 0xC3D2E1F0

#define sha1_rotl(x, s) (((x) << s) | ((x) >> (32 - s)))

#define sha1_F1(b, c, d) ((b & c) | ((~b) & d))
#define sha1_F2(b, c, d) (b ^ c ^ d)
#define sha1_F3(b, c, d) ((b & c) | (b & d) | (c & d))
#define sha1_F4(b, c, d) (b ^ c ^ d)

#define sha1_K1 0x5A827999
#define sha1_K2 0x6ED9EBA1
#define sha1_K3 0x8F1BBCDC
#define sha1_K4 0xCA62C1D6
        
#define sha1_P1(a, b, c, d, e, w) \
               e += sha1_rotl(a, 5) + sha1_F1(b, c, d) + w + sha1_K1; \
               b = sha1_rotl(b, 30);
        
#define sha1_P2(a, b, c, d, e, w) \
               e += sha1_rotl(a, 5) + sha1_F2(b, c, d) + w + sha1_K2; \
               b = sha1_rotl(b, 30);

#define sha1_P3(a, b, c, d, e, w) \
               e += sha1_rotl(a, 5) + sha1_F3(b, c, d) + w + sha1_K3; \
               b = sha1_rotl(b, 30);
        
#define sha1_P4(a, b, c, d, e, w) \
               e += sha1_rotl(a, 5) + sha1_F4(b, c, d) + w + sha1_K4; \
               b = sha1_rotl(b, 30);

#define sha1_G(x, y, z) ((x & y) | (x & z) | (y & z))
#define sha1_H(x, y, z) (x ^ y ^ z)
#define sha1_l2bendian(x) (((x) << 24) | ((x) >> 24) | (((x) & 0xFF00) << 8) | (((x) & 0xFF0000) >> 8))

struct sha1_context
{
	uint32_t H[5];
	uint32_t M[16];
};

void sha1_password (struct sha1_context * context, unsigned char * data, int data_len);
void sha1_transform (struct sha1_context * context);

inline void sha1_bloom_filter (void * context, hashdb_bloom_filter_result_t * result);
inline int  sha1_compare (void * a, void * b);
char *      sha1_to_string (struct sha1_context * context);
int         sha1_from_string (struct sha1_context * context, char * string);

#endif
