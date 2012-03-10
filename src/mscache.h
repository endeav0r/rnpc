#ifndef mscache_HEADER
#define mscache_HEADER

#include <stdlib.h>
#include <string.h>

#include "hashdb.h"
#include "md4.h"

// md4( md4(unicode(password)) + unicode(lower(username)) )
#define MSCACHE_USERNAME_LEN 128

struct mscache_context
{
	int username_length;
	char username[MSCACHE_USERNAME_LEN];
	struct md4_context context;
};

// plaintext must be null_terminated here. plaintext_len really doesn't do anything,
// it's just there to keep function format in line with everything else
void mscache_password (struct mscache_context * context, unsigned char * plaintext, int plaintext_len);

void mscache_set_salt (struct mscache_context * context, unsigned char * salt, int salt_len);

inline void mscache_bloom_filter   (void * context, hashdb_bloom_filter_result_t * result);
inline int  mscache_compare        (void * a, void * b);
char *      mscache_to_string (struct mscache_context * context);
int         mscache_from_string (struct mscache_context * context, char * string);

#endif
