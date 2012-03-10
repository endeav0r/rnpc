#include "nt.h"

char NT_STRING[33];

int nt_from_string (struct nt_context * context, char * string)
{
	
	return md4_from_string((struct md4_context *) context, string);

}



char * nt_to_string (struct nt_context * context)
{

	snprintf(NT_STRING, 33, "%08x%08x%08x%08x",
	         context->A, context->B, context->C, context->D);
	return NT_STRING;

}



void nt_bloom_filter (void * context, hashdb_bloom_filter_result_t * result)
{

	result->one   = ((struct nt_context *)context)->A;// & HASHDB_BLOOM_FILTER_MASK;
	result->two   = ((struct nt_context *)context)->B;// & HASHDB_BLOOM_FILTER_MASK;
	result->three = ((struct nt_context *)context)->C;// & HASHDB_BLOOM_FILTER_MASK;
	result->four  = ((struct nt_context *)context)->D;// & HASHDB_BLOOM_FILTER_MASK;

}



int nt_compare (void * a, void * b)
{

	if (((struct nt_context *) a)->A < ((struct nt_context *) b)->A)
		return -1;
	else if (((struct nt_context *) a)->A > ((struct nt_context *) b)->A)
		return 1;
	else
	{
		if (((struct nt_context *) a)->B < ((struct nt_context *) b)->B)
			return -1;
		else if (((struct nt_context *) a)->B > ((struct nt_context *) b)->B)
			return 1;
		else
		{
			if (((struct nt_context *) a)->C < ((struct nt_context *) b)->C)
				return -1;
			else if (((struct nt_context *) a)->C > ((struct nt_context *) b)->C)
				return 1;
			else
			{
				if (((struct nt_context *) a)->D < ((struct nt_context *) b)->D)
					return -1;
				else if (((struct nt_context *) a)->D > ((struct nt_context *) b)->D)
					return 1;
			}
		}
	}

	return 0;
	
}



// anything longer than 56 bytes is going to break our super efficient
// md4 anyway

void nt_password (struct nt_context * context, unsigned char * plaintext, int length)
{

	int i;
	int nt_hash_length;
	char nt_hash_data[56];
	
	nt_hash_length ^= nt_hash_length;
	
	for (i ^= i; i < length % 22; i++)
	{
		nt_hash_data[nt_hash_length++] = plaintext[i];
		nt_hash_data[nt_hash_length] ^= nt_hash_data[nt_hash_length];
		nt_hash_length++;
	}
	
	md4_password((struct md4_context *) context, (unsigned char *) &nt_hash_data, nt_hash_length);

}
