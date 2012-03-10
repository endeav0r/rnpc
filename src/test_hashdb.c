#include <stdio.h>
#include <string.h>
#include <time.h>

#include "hashdb.h"
#include "md5.h"

#define TEST_HASHDB_PLAINTEXT_1  "wow hello everyone"
#define TEST_HASHDB_PLAINTEXT_2 " today we will be testing something"
#define TEST_HASHDB_PLAINTEXT_3  "mainly the hashdb"
#define TEST_HASHDB_PLAINTEXT_4  "we want to see how well it works"
#define TEST_HASHDB_PLAINTEXT_5  "and if it performs as expected"
#define TEST_HASHDB_PLAINTEXT_6  "and we will find out"
#define TEST_HASHDB_PLAINTEXT_7  "oh yes we will"
#define TEST_HASHDB_PLAINTEXT_8  "and it will work exactly as expected"
#define TEST_HASHDB_PLAINTEXT_9  "because we do things as expected"
#define TEST_HASHDB_PLAINTEXT_10 "we are the expected"

#define TEST_HASHDB_NOT_FOUND_1 "this value will not be found"
#define TEST_HASHDB_NOT_FOUND_2 "nor this one"
#define TEST_HASHDB_NOT_FOUND_3 "this is your dignity... 404"


int main ()
{

	char * hash_to_string;
	struct md5_context context;
	hashdb_t * hashdb;
	
	hashdb = hashdb_create(md5_bloom_filter, 
	                       md5_compare, 
	                       sizeof(struct md5_context));
	
	printf("hashdb created\n");
	
	
	
	
	printf("loading hashes\n");
	
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_1, strlen(TEST_HASHDB_PLAINTEXT_1));
	printf("%08x%08x%08x%08x\n", context.A, context.B, context.C, context.D);
	hashdb_add_hash(hashdb, (void *) &context);
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_2, strlen(TEST_HASHDB_PLAINTEXT_2));
	printf("%08x%08x%08x%08x\n", context.A, context.B, context.C, context.D);
	hashdb_add_hash(hashdb, (void *) &context);
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_3, strlen(TEST_HASHDB_PLAINTEXT_3));
	printf("%08x%08x%08x%08x\n", context.A, context.B, context.C, context.D);
	hashdb_add_hash(hashdb, (void *) &context);
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_4, strlen(TEST_HASHDB_PLAINTEXT_4));
	printf("%08x%08x%08x%08x\n", context.A, context.B, context.C, context.D);
	hashdb_add_hash(hashdb, (void *) &context);
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_5, strlen(TEST_HASHDB_PLAINTEXT_5));
	printf("%08x%08x%08x%08x\n", context.A, context.B, context.C, context.D);
	hashdb_add_hash(hashdb, (void *) &context);
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_6, strlen(TEST_HASHDB_PLAINTEXT_6));
	printf("%08x%08x%08x%08x\n", context.A, context.B, context.C, context.D);
	hashdb_add_hash(hashdb, (void *) &context);
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_7, strlen(TEST_HASHDB_PLAINTEXT_7));
	printf("%08x%08x%08x%08x\n", context.A, context.B, context.C, context.D);
	hashdb_add_hash(hashdb, (void *) &context);
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_8, strlen(TEST_HASHDB_PLAINTEXT_8));
	printf("%08x%08x%08x%08x\n", context.A, context.B, context.C, context.D);
	hashdb_add_hash(hashdb, (void *) &context);
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_9, strlen(TEST_HASHDB_PLAINTEXT_9));
	printf("%08x%08x%08x%08x\n", context.A, context.B, context.C, context.D);
	hashdb_add_hash(hashdb, (void *) &context);
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_10, strlen(TEST_HASHDB_PLAINTEXT_10));
	printf("%08x%08x%08x%08x\n", context.A, context.B, context.C, context.D);
	hashdb_add_hash(hashdb, (void *) &context);
	
	printf("hashes loaded\n");
	
	
	
	
	printf("searching for hashes not in db\n");
	
	md5_password(&context, (unsigned char *) TEST_HASHDB_NOT_FOUND_1, strlen(TEST_HASHDB_NOT_FOUND_1));
	if (hashdb_check_hash(hashdb, (void *) &context))
		printf("hash found, BAD\n");
	else
		printf("hash not found, GOOD\n");
	md5_password(&context, (unsigned char *) TEST_HASHDB_NOT_FOUND_2, strlen(TEST_HASHDB_NOT_FOUND_2));
	if (hashdb_check_hash(hashdb, (void *) &context))
		printf("hash found, BAD\n");
	else
		printf("hash not found, GOOD\n");
	md5_password(&context, (unsigned char *) TEST_HASHDB_NOT_FOUND_3, strlen(TEST_HASHDB_NOT_FOUND_3));
	if (hashdb_check_hash(hashdb, (void *) &context))
		printf("hash found, BAD\n");
	else
		printf("hash not found, GOOD\n");
		
		
		
	
	printf("searching for hashes in db\n");
	
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_1, strlen(TEST_HASHDB_PLAINTEXT_1));
	if (hashdb_check_hash(hashdb, (void *) &context))
		printf("hash found, GOOD\n");
	else
		printf("hash not found, BAD\n");
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_5, strlen(TEST_HASHDB_PLAINTEXT_5));
	if (hashdb_check_hash(hashdb, (void *) &context))
		printf("hash found, GOOD\n");
	else
		printf("hash not found, BAD\n");
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_9, strlen(TEST_HASHDB_PLAINTEXT_9));
	if (hashdb_check_hash(hashdb, (void *) &context))
		printf("hash found, GOOD\n");
	else
		printf("hash not found, BAD\n");
		
		
		
		
	printf("testing hashlist\n");
	
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_1, strlen(TEST_HASHDB_PLAINTEXT_1));
	hashdb_add_plaintext(hashdb, (void *) &context, TEST_HASHDB_PLAINTEXT_1);
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_2, strlen(TEST_HASHDB_PLAINTEXT_2));
	hashdb_add_plaintext(hashdb, (void *) &context, TEST_HASHDB_PLAINTEXT_2);
	md5_password(&context, (unsigned char *) TEST_HASHDB_PLAINTEXT_3, strlen(TEST_HASHDB_PLAINTEXT_3));
	hashdb_add_plaintext(hashdb, (void *) &context, TEST_HASHDB_PLAINTEXT_3);
	
	printf("hashes added, printing out result\n");
	
	hashdb_iterator_reset(hashdb);
	hashdb_iterate(hashdb);
	while (hashdb->plaintext != NULL)
	{
		hash_to_string = md5_to_string((struct md5_context *) hashdb->context);
		printf("%s %s\n", hash_to_string, hashdb->plaintext);
		hashdb_iterate(hashdb);
	}
	
	
	
	
	printf("\n--------------------------\n");
	printf("testing for speed\n");
	
	double johnny_bench_called;
	int i;
	
	md5_password(&context, (unsigned char *) TEST_HASHDB_NOT_FOUND_1, strlen(TEST_HASHDB_NOT_FOUND_1));
	johnny_bench_called = (double) clock();
	for (i = 0; i < 50000000; i++)
	{
		hashdb_check_hash(hashdb, (void *) &context);;
	}
	johnny_bench_called = ((double) clock()) - johnny_bench_called;
	johnny_bench_called /= (double) CLOCKS_PER_SEC;
	johnny_bench_called = 50000000 / johnny_bench_called;
	printf("%f checks per second\n", johnny_bench_called);
	
	
	printf("destroying hashdb\n");
	
	hashdb_destroy(hashdb);
	
	printf("done\n");
	
	return 0;

}
	
