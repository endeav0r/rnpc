#include "crypt.h"



void crypt_password(struct crypt_context * context, unsigned char * plaintext, int plaintext_len)
{

	struct crypt_data data;
	char * crypt_string;

	data.initialized = 0;
	
	crypt_string = crypt_r((char *) plaintext, context->salt, &data);
	
	strncpy(context->hash, crypt_string, CRYPT_HASH_SIZE);
	
}



void crypt_set_salt (struct crypt_context * context, unsigned char * salt, int salt_len)
{

	int i;
	int in_hash = 0;
	int hash_i = 0;

	for (i = 0; i < salt_len; i++)
	{
		if (in_hash == 0)
		{
			if (salt[i] == ':')
			{
				in_hash = 2;
				continue;
			}
			if (salt[i] == '$')
				in_hash = 1;
		}
		else if (in_hash == 2)
		{
			if (salt[i] == ':')
				break;
		}
		if (in_hash)
		{
			if (salt[i] == ':')
				break;
			context->salt[hash_i++] = salt[i];
		}
	}
	
	context->salt[hash_i] = 0;
	
}		



inline void crypt_bloom_filter (void * context, hashdb_bloom_filter_result_t * result)
{

	int i;
	int start_of_hash;
	int strlen_hash;
	
	strlen_hash = strlen(((struct crypt_context *) context)->hash);

	if (((struct crypt_context *) context)->hash[0] != '$')
		start_of_hash = 2;
	else
	{
		start_of_hash = 3;
		for (i = 3; i < strlen_hash; i++)
		{
			if (((struct crypt_context *) context)->hash[i] == '$')
			{
				start_of_hash = i;
				break;
			}
		}
	}
	
	if (start_of_hash + 4  > strlen_hash)
		result->one   = 0;
	else
		result->one   = *((uint32_t *) &(((struct crypt_context *) context)->hash[start_of_hash  ]))
		               & HASHDB_BLOOM_FILTER_MASK;
		
	if (start_of_hash + 8  > strlen_hash)
		result->two   = 0;
	else
		result->two   = *((uint32_t *) &(((struct crypt_context *) context)->hash[start_of_hash+ 4]))
		               & HASHDB_BLOOM_FILTER_MASK;
		
	if (start_of_hash + 12 > strlen_hash)
		result->three = 0;
	else
		result->three = *((uint32_t *) &(((struct crypt_context *) context)->hash[start_of_hash+ 8]))
		                 & HASHDB_BLOOM_FILTER_MASK;
		
	if (start_of_hash + 16 > strlen_hash)
		result->four  = 0;
	else
		result->four  = *((uint32_t *) &(((struct crypt_context *) context)->hash[start_of_hash+12]))
		                 & HASHDB_BLOOM_FILTER_MASK;

}
		


inline int crypt_compare (void * a, void * b)
{
	return strcmp(((struct crypt_context *) a)->hash,
	              ((struct crypt_context *) b)->hash);
	              
}



char * crypt_to_string (struct crypt_context * context)
{
	return context->hash;
}



int crypt_from_string (struct crypt_context * context, char * string)
{

	int i;
	int in_hash = 0;
	int hash_i = 0;
	
	for (i = 0; i < strlen(string); i++)
	{
		if (in_hash == 0)
		{
			if (string[i] == ':')
			{
				in_hash = 2;
				continue;
			}
			if (string[i] == '$')
				in_hash = 1;
		}
		else if (in_hash == 2)
		{
			if (string[i] == ':')
				break;
		}
		if (in_hash)
		{
			if (string[i] == ':')
				break;
			context->hash[hash_i++] = string[i];
		}
	}
	
	context->hash[hash_i] = 0;
	
	if (hash_i < 2)
		return -1;

	return 0;

}
