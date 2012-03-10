#ifndef crypt_HEADER
#define crypt_HEADER

#define _GNU_SOURCE
#include <crypt.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>


#include "hashdb.h"

#define CRYPT_HASH_SIZE 128
#define CRYPT_SALT_SIZE 128


struct crypt_context
{
	char hash[CRYPT_HASH_SIZE]; // <- Result goes here
	char salt[CRYPT_SALT_SIZE]; // <- USER HAS TO PUT SALT HERE
};

// plaintext must be null_terminated here. plaintext_len really doesn't do anything,
// it's just there to keep function format in line with everything else
void crypt_password (struct crypt_context * context, unsigned char * plaintext, int plaintext_len);
void crypt_set_salt (struct crypt_context * context, unsigned char * salt, int salt_len);

inline void crypt_bloom_filter (void * context, hashdb_bloom_filter_result_t * result);
inline int  crypt_compare      (void * a, void * b);
char *      crypt_to_string    (struct crypt_context * context);
int         crypt_from_string  (struct crypt_context * context, char * string);

#endif
