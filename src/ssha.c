#include "ssha.h"

char SSHA_STRING[128];

// borrowing heavily from nt.c here
void ssha_password (struct ssha_context * context, unsigned char * plaintext, int plaintext_len)
{

	unsigned char sha_hash_data[256];
	
	plaintext_len %= 64;
	
	memcpy(sha_hash_data, plaintext, plaintext_len);
	memcpy(&(sha_hash_data[plaintext_len]), context->salt, context->salt_length);
	
	sha1_password(&(context->context), sha_hash_data, plaintext_len + context->salt_length);

}



void ssha_set_salt (struct ssha_context * context, unsigned char * salt, int salt_len)
{

	int hash_size = 256;
	unsigned char hash[hash_size];
	
	if ((salt[0] == '{') && (salt[5] == '}'))
		base64_decode(hash, &hash_size, &(salt[6]), salt_len - 6);
	else
		base64_decode(hash, &hash_size, salt, salt_len);
	
	context->salt_length = hash_size - 20;
	
	memcpy(context->salt, &(hash[20]), context->salt_length);

} 



inline void ssha_bloom_filter (void * context, hashdb_bloom_filter_result_t * result)
{
	sha1_bloom_filter((void *) &(((struct ssha_context *) context)->context), result);
}


 
inline int ssha_compare (void * a, void * b)
{
	
	return sha1_compare(&(((struct ssha_context *) a)->context),
	                    &(((struct ssha_context *) b)->context));

}



char * ssha_to_string (struct ssha_context * context)
{

	int result_size = 128;
	unsigned char hash[128];
	
	*((uint32_t *) &(hash[ 0])) = sha1_l2bendian(context->context.H[0]);
	*((uint32_t *) &(hash[ 4])) = sha1_l2bendian(context->context.H[1]);
	*((uint32_t *) &(hash[ 8])) = sha1_l2bendian(context->context.H[2]);
	*((uint32_t *) &(hash[12])) = sha1_l2bendian(context->context.H[3]);
	*((uint32_t *) &(hash[16])) = sha1_l2bendian(context->context.H[4]);
	memcpy(&(hash[20]), context->salt, context->salt_length);
	
	strcpy(SSHA_STRING, "{SSHA}");
	
	base64_encode((unsigned char *) &(SSHA_STRING[6]),
	              &result_size, hash, 20 + context->salt_length);
	
	return SSHA_STRING;
	
}



int ssha_from_string (struct ssha_context * context, char * string)
{

	int hash_size = 256;
	unsigned char hash[hash_size];
	
	if ((string[0] == '{') && (string[5] == '}'))
		base64_decode(hash, &hash_size, (unsigned char *) &(string[6]), strlen(string) - 6);
	else
		base64_decode(hash, &hash_size, (unsigned char *) string, strlen(string));
	
	context->context.H[0] = sha1_l2bendian(((uint32_t *) hash)[0]);
	context->context.H[1] = sha1_l2bendian(((uint32_t *) hash)[1]);
	context->context.H[2] = sha1_l2bendian(((uint32_t *) hash)[2]);
	context->context.H[3] = sha1_l2bendian(((uint32_t *) hash)[3]);
	context->context.H[4] = sha1_l2bendian(((uint32_t *) hash)[4]);
	
	return 0;
	
}
