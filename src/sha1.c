#include "sha1.h"


char SHA1_STRING[41];

int sha1_from_string (struct sha1_context * context, char * string)
{
	
	if (strlen(string) != 40)
		return -1;

	context->H[0] = hex_string_to_uint32(&(string[ 0]));
	context->H[1] = hex_string_to_uint32(&(string[ 8]));
	context->H[2] = hex_string_to_uint32(&(string[16]));
	context->H[3] = hex_string_to_uint32(&(string[24]));
	context->H[4] = hex_string_to_uint32(&(string[32]));
	
	return 0;

}



char * sha1_to_string (struct sha1_context * context)
{

	snprintf(SHA1_STRING, 41, "%08x%08x%08x%08x%08x",
	         context->H[0], context->H[1], context->H[2],
	         context->H[3], context->H[4]);
	return SHA1_STRING;

}



inline void sha1_bloom_filter (void * context, hashdb_bloom_filter_result_t * result)
{

	result->one   = ((struct sha1_context *)context)->H[0] & HASHDB_BLOOM_FILTER_MASK;
	result->two   = ((struct sha1_context *)context)->H[1] & HASHDB_BLOOM_FILTER_MASK;
	result->three = ((struct sha1_context *)context)->H[2] & HASHDB_BLOOM_FILTER_MASK;
	result->four  = ((struct sha1_context *)context)->H[3] & HASHDB_BLOOM_FILTER_MASK;

}



inline int sha1_compare (void * a, void * b)
{

	if (((struct sha1_context *) a)->H[0] < ((struct sha1_context *) b)->H[0])
		return -1;
	else if (((struct sha1_context *) a)->H[0] > ((struct sha1_context *) b)->H[0])
		return 1;
	else
	{
		if (((struct sha1_context *) a)->H[1] < ((struct sha1_context *) b)->H[1])
			return -1;
		else if (((struct sha1_context *) a)->H[1] > ((struct sha1_context *) b)->H[1])
			return 1;
		else
		{
			if (((struct sha1_context *) a)->H[2] < ((struct sha1_context *) b)->H[2])
				return -1;
			else if (((struct sha1_context *) a)->H[2] > ((struct sha1_context *) b)->H[2])
				return 1;
			else
			{
				if (((struct sha1_context *) a)->H[3] < ((struct sha1_context *) b)->H[3])
					return -1;
				else if (((struct sha1_context *) a)->H[3] > ((struct sha1_context *) b)->H[3])
					return 1;
				else
				{
					if (((struct sha1_context *) a)->H[4] < ((struct sha1_context *) b)->H[4])
						return -1;
					else if (((struct sha1_context *) a)->H[4] > ((struct sha1_context *) b)->H[4])
						return 1;
				}
			}
		}
	}

	return 0;
	
}





void sha1_password (struct sha1_context * context, unsigned char * data, int data_len)
{
	
	int i;
	int data_processed;
	int data_processed_div_four;
	int loop_cache;
		
	context->H[0] = sha1_H0;
	context->H[1] = sha1_H1;
	context->H[2] = sha1_H2;
	context->H[3] = sha1_H3;
	context->H[4] = sha1_H4;
	

	// we need to process multiple blocks of data
	if (data_len >= 56)
	{
	
		data_processed ^= data_processed;
	
		while (data_processed < data_len - 56)
		{
			// these are the only two potential places we need to zero out
			// and i'd rather go ahead and XOR than waste a conditional to
			// see if it's necessary
			context->M[14] ^= context->M[14];
			context->M[15] ^= context->M[15];
			// copy over memory by words
			loop_cache = (data_len - data_processed) / 4;
			if (loop_cache > 16)
				loop_cache = 16;
			data_processed_div_four = data_processed / 4;
			for (i ^= i; i < loop_cache; i++)
				context->M[i] = sha1_l2bendian(((unsigned int *) data)[i + data_processed_div_four]);
			// copy over whatever is left
			///// printf("[B] i: %d, loop_cache: %d\n", i, loop_cache);
			i <<= 2; // *= 4
			if (loop_cache < 16)
			{
				switch (data_len & 3) // % 4
				{
					case 0 :
						((unsigned char *) context->M)[i+3] = 0x80;
						
						break;
					case 1 :
						((unsigned char *) context->M)[i+3] = data[i];
						((unsigned char *) context->M)[i+2] = 0x80;
						i++;
						break;
					case 2 :
						((unsigned char *) context->M)[i+3] = data[i];
						((unsigned char *) context->M)[i+2] = data[i+1];
						((unsigned char *) context->M)[i+1] = 0x80;
						i += 2;
						break;
					case 3 :
						((unsigned char *) context->M)[i+3] = data[i];
						((unsigned char *) context->M)[i+2] = data[i+1];
						((unsigned char *) context->M)[i+1] = data[i+2];
						((unsigned char *) context->M)[i  ] = 0x80;
						i += 3;
						break;			
				}
			}
			// hash the shit
			sha1_transform(context);
			///// printf("[A] data_processed: %d, i: %d\n", data_processed, i);
			data_processed += i;
		}
				
		// copy over by words, zero out the rest
		loop_cache = (data_len - data_processed) / 4;
		data_processed_div_four = data_processed / 4;
		///// printf("data_len: %d, data_processed: %d, loop_cache: %d\n", data_len, data_processed, loop_cache);
		for (i ^= i; i < loop_cache; i++)
			context->M[i] = sha1_l2bendian(((unsigned int *) data)[i + data_processed_div_four]);
		for (i = loop_cache; i < 16; i++)
			context->M[i] ^= context->M[i];
	
		// copy over less than word length stuff, append 1 bit
		if ((loop_cache > 0) || (data_processed % 64 == 0))
		{
			i = loop_cache << 2; // *= 4
			switch (data_len - data_processed - i)
			{
				case 0 :
					((unsigned char *) context->M)[i+3] = 0x80;
					break;
				case 1 :
					((unsigned char *) context->M)[i+3] = data[i];
					((unsigned char *) context->M)[i+2] = 0x80;
					break;
				case 2 :
					((unsigned char *) context->M)[i+3] = data[i];
					((unsigned char *) context->M)[i+2] = data[i+1];
					((unsigned char *) context->M)[i+1] = 0x80;
					break;
				case 3 :
					((unsigned char *) context->M)[i+3] = data[i];
					((unsigned char *) context->M)[i+2] = data[i+1];
					((unsigned char *) context->M)[i+1] = data[i+2];
					((unsigned char *) context->M)[i  ] = 0x80;
					break;
			}
		}
			
		///// printf("data_len: %d\n", data_len);

		for (i = 0; i < 8; i++)
			///// printf("%02x ", ((unsigned char *) context->M)[i]);
		///// printf("\n");
	
		context->M[15] = data_len * 8;
	
		sha1_transform(context);
		
	}
	else
	{
		loop_cache = data_len / 4;
		for (i ^= i; i < loop_cache; i++)
			context->M[i] = sha1_l2bendian(((unsigned int *) data)[i]);
		while (i < 16)
		{
			context->M[i] ^= context->M[i];
			i++;
		}
	
		// copy over less than word length stuff, append 1 bit
		i = loop_cache << 2; // *= 4
		switch (data_len & 3)
		{
			case 0 :
		
				((unsigned char *) context->M)[i+3] = 0x80;
				break;
			case 1 :
				((unsigned char *) context->M)[i+3] = data[i];
				((unsigned char *) context->M)[i+2] = 0x80;
				break;
			case 2 :
				((unsigned char *) context->M)[i+3] = data[i];
				((unsigned char *) context->M)[i+2] = data[i+1];
				((unsigned char *) context->M)[i+1] = 0x80;
				break;
			case 3 :
				((unsigned char *) context->M)[i+3] = data[i];
				((unsigned char *) context->M)[i+2] = data[i+1];
				((unsigned char *) context->M)[i+1] = data[i+2];
				((unsigned char *) context->M)[i  ] = 0x80;
				break;
		}
	
		context->M[15] = data_len << 3; // * 8
	
		sha1_transform(context);
		
	}
		
}
	
void sha1_transform (struct sha1_context * context)
{

	int i;
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint32_t e;
	uint32_t W[80];	

	for (i = 0; i < 16; i++)
		W[i] = context->M[i];
	
	for (i = 16; i < 80; i++)
		W[i] = sha1_rotl(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);

	/*
	printf("%08x %08x %08x %08x\n", W[ 0], W[ 1], W[ 2], W[ 3]);
	printf("%08x %08x %08x %08x\n", W[ 4], W[ 5], W[ 6], W[ 7]);
	printf("%08x %08x %08x %08x\n", W[ 8], W[ 9], W[10], W[11]);
	printf("%08x %08x %08x %08x\n", W[12], W[13], W[14], W[15]);
	printf("\n");
	*/	
	
	a = context->H[0];
	b = context->H[1];
	c = context->H[2];
	d = context->H[3];
	e = context->H[4];
	
	sha1_P1(a, b, c, d, e, W[0])  // 1
	sha1_P1(e, a, b, c, d, W[1])  // 2
	sha1_P1(d, e, a, b, c, W[2])  // 3
	sha1_P1(c, d, e, a, b, W[3])  // 4
	sha1_P1(b, c, d, e, a, W[4])  // 5
	sha1_P1(a, b, c, d, e, W[5])  // 6
	sha1_P1(e, a, b, c, d, W[6])  // 7
	sha1_P1(d, e, a, b, c, W[7])  // 8
	sha1_P1(c, d, e, a, b, W[8])  // 9
	sha1_P1(b, c, d, e, a, W[9])  // 10
	sha1_P1(a, b, c, d, e, W[10]) // 11
	sha1_P1(e, a, b, c, d, W[11]) // 12
	sha1_P1(d, e, a, b, c, W[12]) // 13
	sha1_P1(c, d, e, a, b, W[13]) // 14
	sha1_P1(b, c, d, e, a, W[14]) // 15
	sha1_P1(a, b, c, d, e, W[15]) // 16
	sha1_P1(e, a, b, c, d, W[16])
	sha1_P1(d, e, a, b, c, W[17])
	sha1_P1(c, d, e, a, b, W[18])
	sha1_P1(b, c, d, e, a, W[19])
	
	sha1_P2(a, b, c, d, e, W[20])
	sha1_P2(e, a, b, c, d, W[21])
	sha1_P2(d, e, a, b, c, W[22])
	sha1_P2(c, d, e, a, b, W[23])
	sha1_P2(b, c, d, e, a, W[24])
	sha1_P2(a, b, c, d, e, W[25])
	sha1_P2(e, a, b, c, d, W[26])
	sha1_P2(d, e, a, b, c, W[27])
	sha1_P2(c, d, e, a, b, W[28])
	sha1_P2(b, c, d, e, a, W[29])
	sha1_P2(a, b, c, d, e, W[30])
	sha1_P2(e, a, b, c, d, W[31])
	sha1_P2(d, e, a, b, c, W[32])
	sha1_P2(c, d, e, a, b, W[33])
	sha1_P2(b, c, d, e, a, W[34])
	sha1_P2(a, b, c, d, e, W[35])
	sha1_P2(e, a, b, c, d, W[36])
	sha1_P2(d, e, a, b, c, W[37])
	sha1_P2(c, d, e, a, b, W[38])
	sha1_P2(b, c, d, e, a, W[39])
	
	sha1_P3(a, b, c, d, e, W[40])
	sha1_P3(e, a, b, c, d, W[41])
	sha1_P3(d, e, a, b, c, W[42])
	sha1_P3(c, d, e, a, b, W[43])
	sha1_P3(b, c, d, e, a, W[44])
	sha1_P3(a, b, c, d, e, W[45])
	sha1_P3(e, a, b, c, d, W[46])
	sha1_P3(d, e, a, b, c, W[47])
	sha1_P3(c, d, e, a, b, W[48])
	sha1_P3(b, c, d, e, a, W[49])
	sha1_P3(a, b, c, d, e, W[50])
	sha1_P3(e, a, b, c, d, W[51])
	sha1_P3(d, e, a, b, c, W[52])
	sha1_P3(c, d, e, a, b, W[53])
	sha1_P3(b, c, d, e, a, W[54])
	sha1_P3(a, b, c, d, e, W[55])
	sha1_P3(e, a, b, c, d, W[56])
	sha1_P3(d, e, a, b, c, W[57])
	sha1_P3(c, d, e, a, b, W[58])
	sha1_P3(b, c, d, e, a, W[59])
	
	sha1_P4(a, b, c, d, e, W[60])
	sha1_P4(e, a, b, c, d, W[61])
	sha1_P4(d, e, a, b, c, W[62])
	sha1_P4(c, d, e, a, b, W[63])
	sha1_P4(b, c, d, e, a, W[64])
	sha1_P4(a, b, c, d, e, W[65])
	sha1_P4(e, a, b, c, d, W[66])
	sha1_P4(d, e, a, b, c, W[67])
	sha1_P4(c, d, e, a, b, W[68])
	sha1_P4(b, c, d, e, a, W[69])
	sha1_P4(a, b, c, d, e, W[70])
	sha1_P4(e, a, b, c, d, W[71])
	sha1_P4(d, e, a, b, c, W[72])
	sha1_P4(c, d, e, a, b, W[73])
	sha1_P4(b, c, d, e, a, W[74])
	sha1_P4(a, b, c, d, e, W[75])
	sha1_P4(e, a, b, c, d, W[76])
	sha1_P4(d, e, a, b, c, W[77])
	sha1_P4(c, d, e, a, b, W[78])
	sha1_P4(b, c, d, e, a, W[79])
	
	context->H[0] += a;
	context->H[1] += b;
	context->H[2] += c;
	context->H[3] += d;
	context->H[4] += e;
	
}
