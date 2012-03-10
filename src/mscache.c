#include "mscache.h"



// borrowing heavily from nt.c here
void mscache_password (struct mscache_context * context, unsigned char * plaintext, int plaintext_len)
{

	int i;
	int nt_hash_length;
	char nt_hash_data[56];
	
	nt_hash_length ^= nt_hash_length;
	
	for (i ^= i; i < plaintext_len % 56; i++)
	{
		nt_hash_data[nt_hash_length++] = plaintext[i];
		nt_hash_data[nt_hash_length] ^= nt_hash_data[nt_hash_length];
		nt_hash_length++;
	}
	
	md4_password(&(context->context), (unsigned char *) &nt_hash_data, nt_hash_length);
	
	((unsigned int *) nt_hash_data)[0] = md4_l2bendian(context->context.A);
	((unsigned int *) nt_hash_data)[1] = md4_l2bendian(context->context.B);
	((unsigned int *) nt_hash_data)[2] = md4_l2bendian(context->context.C);
	((unsigned int *) nt_hash_data)[3] = md4_l2bendian(context->context.D);
	
	for (i = 0; i < context->username_length % 40; i++)
		nt_hash_data[i+16] = context->username[i];
	
	md4_password(&(context->context),
	             (unsigned char *) &nt_hash_data, 
	             16 + context->username_length);

}



void mscache_set_salt (struct mscache_context * context, unsigned char * salt, int salt_len)
{
	int i;
	
	context->username_length = 0;
	for (i = 0; i < salt_len % (MSCACHE_USERNAME_LEN - 1); i++)
	{
		if (salt[i] == ':')
			break;
		context->username[context->username_length++] = (char) salt[i];
		context->username[context->username_length] ^= context->username[context->username_length];
		context->username_length++;
	}

} 



inline void mscache_bloom_filter (void * context, hashdb_bloom_filter_result_t * result)
{
	md4_bloom_filter((void *) &(((struct mscache_context *) context)->context), result);
}


 
inline int mscache_compare (void * a, void * b)
{
	return md4_compare((void *) &(((struct mscache_context *) a)->context),
	                   (void *) &(((struct mscache_context *) b)->context));
}



char * mscache_to_string (struct mscache_context * context)
{
	return md4_to_string(&(context->context));
}



int mscache_from_string (struct mscache_context * context, char * string)
{

	int i;
	int hash_i = 0;
	int found_colon = 0;
	char hash[33];


	for (i = 0; i < strlen(string); i++)
	{
		if ((found_colon) && (hash_i < 33))
			hash[hash_i++] = string[i];
		if (string[i] == ':')
		{
			if (found_colon)
				break;
			else
				found_colon = 1;
		}
	}
	
	hash[hash_i] = 0;
		
	return md4_from_string(&(context->context), hash);
	
}
