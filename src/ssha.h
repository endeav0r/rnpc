#ifndef ssha_HEADER
#define ssha_HEADER

#include <stdlib.h>
#include <string.h>

#include "hashdb.h"
#include "misc.h"
#include "sha1.h"

// md4( md4(unicode(password)) + unicode(lower(username)) )

struct ssha_context
{
	int salt_length;
	unsigned char salt[16];
	struct sha1_context context;
};

// plaintext must be null_terminated here. plaintext_len really doesn't do anything,
// it's just there to keep function format in line with everything else
void ssha_password (struct ssha_context * context, unsigned char * plaintext, int plaintext_len);

void ssha_set_salt (struct ssha_context * context, unsigned char * salt, int salt_len);

inline void ssha_bloom_filter (void * context, hashdb_bloom_filter_result_t * result);
inline int  ssha_compare      (void * a, void * b);
char *      ssha_to_string    (struct ssha_context * context);
int         ssha_from_string  (struct ssha_context * context, char * string);

#endif
