#ifndef nt_HEADER
#define nt_HEADER

#include "md4.h"

// must be identical to md4_context
struct nt_context
{
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
	uint32_t M[16];
};

void nt_password (struct nt_context * context, unsigned char * plaintext, int length);

void nt_bloom_filter (void * context, hashdb_bloom_filter_result_t * result);

int nt_compare (void * a, void * b);

char * nt_to_string (struct nt_context * context);

int nt_from_string (struct nt_context * context, char * string);

#endif
