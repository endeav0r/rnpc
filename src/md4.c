#include "md4.h"

char MD4_STRING[33];

int md4_from_string (struct md4_context * context, char * string)
{
	
	if (strlen(string) != 32)
		return -1;

	context->A = hex_string_to_uint32(&(string[ 0]));
	context->B = hex_string_to_uint32(&(string[ 8]));
	context->C = hex_string_to_uint32(&(string[16]));
	context->D = hex_string_to_uint32(&(string[24]));
	
	return 0;

}



char * md4_to_string (struct md4_context * context)
{

	snprintf(MD4_STRING, 33, "%08x%08x%08x%08x",
	         context->A, context->B, context->C, context->D);
	return MD4_STRING;

}



inline void md4_bloom_filter (void * context, hashdb_bloom_filter_result_t * result)
{

	result->one   = ((struct md4_context *)context)->A;// & HASHDB_BLOOM_FILTER_MASK;
	result->two   = ((struct md4_context *)context)->B;// & HASHDB_BLOOM_FILTER_MASK;
	result->three = ((struct md4_context *)context)->C;// & HASHDB_BLOOM_FILTER_MASK;
	result->four  = ((struct md4_context *)context)->D;// & HASHDB_BLOOM_FILTER_MASK;

}



inline int md4_compare (void * a, void * b)
{

	if (((struct md4_context *) a)->A < ((struct md4_context *) b)->A)
		return -1;
	else if (((struct md4_context *) a)->A > ((struct md4_context *) b)->A)
		return 1;
	else
	{
		if (((struct md4_context *) a)->B < ((struct md4_context *) b)->B)
			return -1;
		else if (((struct md4_context *) a)->B > ((struct md4_context *) b)->B)
			return 1;
		else
		{
			if (((struct md4_context *) a)->C < ((struct md4_context *) b)->C)
				return -1;
			else if (((struct md4_context *) a)->C > ((struct md4_context *) b)->C)
				return 1;
			else
			{
				if (((struct md4_context *) a)->D < ((struct md4_context *) b)->D)
					return -1;
				else if (((struct md4_context *) a)->D > ((struct md4_context *) b)->D)
					return 1;
			}
		}
	}

	return 0;
	
}



void md4_password (struct md4_context * context, unsigned char * data, int data_len)
{
	
	int i;
	
	context->A = md4_A;
	context->B = md4_B;
	context->C = md4_C;
	context->D = md4_D;
	
	data_len %= 55;

	// xoring the M words against themselves is faster than a call to memset
	// right now we are setting everything to zero
	for (i = 0; i < 16; i++)
		context->M[i] ^= context->M[i];
	
	
	/*
	* The following lines were benched and were faster than memcpy
	* We are copying over data into context->M one word at a time
	* until there is less than one word of data to copy, at which point
	* we copy over the remaining bytes by making use of this
	* elaborate-but-hopefully-faster-than-setting-up-another-loop switch/case
	* statement.
	*/
	
	// copy over the words
	for (i = 0; i < data_len / 4; i++)
		context->M[i] = ((unsigned int *) data)[i];
	
	// how many bytes are left?
	switch (data_len & 3)
	{
		case 1 :
			// i was counting words, now it's counting bytes. we multiply
			// i by 4 real quick to do the conversion, then copy the data
			i <<= 2; // *= 4
			((unsigned char *) context->M)[i] = data[i];
			break;
		case 2 :
			i <<= 2;
			((unsigned char *) context->M)[i] = data[i];
			i++;
			((unsigned char *) context->M)[i] = data[i];
			break;
		case 3 :
			i <<= 2;
			((unsigned char *) context->M)[i] = data[i];
			i++;
			((unsigned char *) context->M)[i] = data[i];
			i++;
			((unsigned char *) context->M)[i] = data[i];
			break;
	}
	
	// According to the RFC, we append a 1 bit, and then pad out with zeros
	// until the first 14 words (448 bits) are filled
	((unsigned char *) context->M)[data_len] = 0x80;
	
	// We then place a 64-bit integer in the last 64 bits. This number counts
	// the number of BITS of data in the hash.
	context->M[14] = data_len * 8;

	md4_transform(context);
	
}



inline void md4_transform (struct md4_context * context)
{


	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	
	a = context->A;
	b = context->B;
	c = context->C;
	d = context->D;
	// what follows is the transforms as they are done according to the RFC.
	// The actual operations can be found in md4.h
	
	// round 1
	a = md4_rotl(a + md4_F(b, c, d) + context->M[0], 3);
	d = md4_rotl(d + md4_F(a, b, c) + context->M[1], 7);
	c = md4_rotl(c + md4_F(d, a, b) + context->M[2], 11);
	b = md4_rotl(b + md4_F(c, d, a) + context->M[3], 19);

	a = md4_rotl(a + md4_F(b, c, d) + context->M[4], 3);
	d = md4_rotl(d + md4_F(a, b, c) + context->M[5], 7);
	c = md4_rotl(c + md4_F(d, a, b) + context->M[6], 11);
	b = md4_rotl(b + md4_F(c, d, a) + context->M[7], 19);
	
	a = md4_rotl(a + md4_F(b, c, d) + context->M[8],  3);
	d = md4_rotl(d + md4_F(a, b, c) + context->M[9],  7);
	c = md4_rotl(c + md4_F(d, a, b) + context->M[10], 11);
	b = md4_rotl(b + md4_F(c, d, a) + context->M[11], 19);
	
	a = md4_rotl(a + md4_F(b, c, d) + context->M[12], 3);
	d = md4_rotl(d + md4_F(a, b, c) + context->M[13], 7);
	c = md4_rotl(c + md4_F(d, a, b) + context->M[14], 11);
	b = md4_rotl(b + md4_F(c, d, a) + context->M[15], 19);
	
	// round 2
	a = md4_rotl(a + md4_G(b, c, d) + context->M[0]  + 0x5a827999, 3);
	d = md4_rotl(d + md4_G(a, b, c) + context->M[4]  + 0x5a827999, 5);
	c = md4_rotl(c + md4_G(d, a, b) + context->M[8]  + 0x5a827999, 9);
	b = md4_rotl(b + md4_G(c, d, a) + context->M[12] + 0x5a827999, 13);
	
	a = md4_rotl(a + md4_G(b, c, d) + context->M[1]  + 0x5a827999, 3);
	d = md4_rotl(d + md4_G(a, b, c) + context->M[5]  + 0x5a827999, 5);
	c = md4_rotl(c + md4_G(d, a, b) + context->M[9]  + 0x5a827999, 9);
	b = md4_rotl(b + md4_G(c, d, a) + context->M[13] + 0x5a827999, 13);
	
	a = md4_rotl(a + md4_G(b, c, d) + context->M[2]  + 0x5a827999, 3);
	d = md4_rotl(d + md4_G(a, b, c) + context->M[6]  + 0x5a827999, 5);
	c = md4_rotl(c + md4_G(d, a, b) + context->M[10] + 0x5a827999, 9);
	b = md4_rotl(b + md4_G(c, d, a) + context->M[14] + 0x5a827999, 13);
	
	a = md4_rotl(a + md4_G(b, c, d) + context->M[3]  + 0x5a827999, 3);
	d = md4_rotl(d + md4_G(a, b, c) + context->M[7]  + 0x5a827999, 5);
	c = md4_rotl(c + md4_G(d, a, b) + context->M[11] + 0x5a827999, 9);
	b = md4_rotl(b + md4_G(c, d, a) + context->M[15] + 0x5a827999, 13);
	
	// round 3
	a = md4_rotl(a + md4_H(b, c, d) + context->M[0]  + 0x6ed9eba1, 3);
	d = md4_rotl(d + md4_H(a, b, c) + context->M[8]  + 0x6ed9eba1, 9);
	c = md4_rotl(c + md4_H(d, a, b) + context->M[4]  + 0x6ed9eba1, 11);
	b = md4_rotl(b + md4_H(c, d, a) + context->M[12] + 0x6ed9eba1, 15);
	
	a = md4_rotl(a + md4_H(b, c, d) + context->M[2]  + 0x6ed9eba1, 3);
	d = md4_rotl(d + md4_H(a, b, c) + context->M[10] + 0x6ed9eba1, 9);
	c = md4_rotl(c + md4_H(d, a, b) + context->M[6]  + 0x6ed9eba1, 11);
	b = md4_rotl(b + md4_H(c, d, a) + context->M[14] + 0x6ed9eba1, 15);
	
	a = md4_rotl(a + md4_H(b, c, d) + context->M[1]  + 0x6ed9eba1, 3);
	d = md4_rotl(d + md4_H(a, b, c) + context->M[9]  + 0x6ed9eba1, 9);
	c = md4_rotl(c + md4_H(d, a, b) + context->M[5]  + 0x6ed9eba1, 11);
	b = md4_rotl(b + md4_H(c, d, a) + context->M[13] + 0x6ed9eba1, 15);
	
	a = md4_rotl(a + md4_H(b, c, d) + context->M[3]  + 0x6ed9eba1, 3);
	d = md4_rotl(d + md4_H(a, b, c) + context->M[11] + 0x6ed9eba1, 9);
	c = md4_rotl(c + md4_H(d, a, b) + context->M[7]  + 0x6ed9eba1, 11);
	b = md4_rotl(b + md4_H(c, d, a) + context->M[15] + 0x6ed9eba1, 15);
	
	context->A = md4_l2bendian(md4_A + a);
	context->B = md4_l2bendian(md4_B + b);
	context->C = md4_l2bendian(md4_C + c);
	context->D = md4_l2bendian(md4_D + d);
	
}
