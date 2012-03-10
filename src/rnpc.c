/*
* If we remember one thing, it is this:
* "DON'T PUT IT IN THE LOOP"
*/


#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "crypt.h"
#include "md4.h"
#include "md5.h"
#include "mscache.h"
#include "nt.h"
#include "sha1.h"
#include "ssha.h"

#include "brute.h"
#include "dict.h"
#include "hashdb.h"
#include "mangle.h"
#include "misc.h"
#include "salt.h"


#define RNPC_ATTACK_NONE       0
#define RNPC_ATTACK_WORDLIST   1
#define RNPC_ATTACK_BRUTEFORCE 2

#define RNPC_HASH_ALGO_MD5     1
#define RNPC_HASH_ALGO_NT      2
#define RNPC_HASH_ALGO_SHA1    3
#define RNPC_HASH_ALGO_MSCACHE 4
#define RNPC_HASH_ALGO_CRYPT   5
#define RNPC_HASH_ALGO_SSHA    6

#define RNPC_SALT_NONE   0
#define RNPC_SALT_SINGLE 1
#define RNPC_SALT_FILE   2
#define RNPC_SALT_HASH   3 // mscache/crypt salt is contained with hash

#define RNPC_MAX_THREADS 8

#define RNPC_THREAD_WORDS_CACHE_N 1024
#define RNPC_THREAD_WORDS_CACHE_SIZE 256

#define RNPC_KB_WAIT_TIME 10

// yes, this struct does have a gravitational pull. it also holds
// the state of the entire cracker



typedef struct rnpc_thread_info_s
{

	pthread_t thread;

	struct rnpc_s * rnpc;
	
	void * hash_context;
	
	void * salted_hash_context; // for mscache/crypt and hashes with the salt built-in
	                            // we store the context of the hash here and skip the
	                            // bloom filter when checking to see if we've found the hash
	
	char words_cache[RNPC_THREAD_WORDS_CACHE_N][RNPC_THREAD_WORDS_CACHE_SIZE];
	
	char mangle_word[MANGLE_WORD_SIZE];
	char brute_word[BRUTE_WORD_SIZE];
	char salt_word[SALT_WORD_SIZE];

} rnpc_thread_info_t;



typedef struct rnpc_s
{
	// these are all set in process_command_line_arguments()
	int    allow_stdin_updates; // if we have -d stdin, this is set to 0.
	                            // otherwise, input on stdin will print out
	                            // the cracker's status
	int    attack_type; // brute/wordlist etc
	int    brute_length; // length of passwords in brute force attack
	int    hash_algo; // an int that tells us what hash algo to use
	int    num_threads; // number of threads to run in parallel
	int    output_to_file; // if 1, then we append hashes found to a file
	int    using_mangle; // an int that tells us what mangling rule to use
	int    using_salt; // an int that tell us if we're using a salt, and what kind

	char * brute_charset; // charset for use in a brute force attack
	char * dictionary_filename; // filename of word dictionary
	char * hash_filename; // filename where all the hashes are
	char * mangle_settings; // this is the string, straight off the command line,
	                        // that is used to set up the mangler
	char * output_filename;
	char * salt; // the current salt goes here. this is malloced and strcpy(ied)
	             // from the command line, or loaded from a dictionary
	char * salt_filename; // name of salt file, from -s
		                     
	hashdb_t *          hashdb;
	dictionary_file_t * dictionary; // this is the dictionary for wordlists
	dictionary_file_t * salt_file_dict; // this is the dictionary for a salt
	                                    // file, selected by -s
	
	struct rnpc_thread_info_s thread_infos[RNPC_MAX_THREADS];

	// these are all set in init_hash(), based on value of rnpc_hash_algo
	char * (* hash_to_string)    (void * context);
	int    (* hash_from_string)  (void * context, char * string);
	void   (* hash_password)     (void * context, unsigned char * data, int length);
	void   (* hash_bloom_filter) (void * context, hashdb_bloom_filter_result_t * result);
	int    (* hash_compare)      (void * a, void * b);
	void   (* hash_set_salt)     (void * context, unsigned char * salt, int salt_len);
	
	// salt_function is for a function in salt.c, where-as hash_set_salt is for
	// hashes that have hash-specific salts, such as mscache or crypt

	void (* salt_function)     (unsigned char * plaintext, int plaintext_length,
		                        unsigned char * salt     , int salt_length,
		                        unsigned char * salt_word);

	void * hash_context;
	int    hash_context_size;
	
	int guesses;
	int guesses2;
	time_t bench_time; // this is used to keep track of how many hashes we are
	                   // trying per second. of course, it is not precise
	
	pthread_mutex_t guesses_lock;
	
	int threads_running;
	
	pthread_mutex_t threads_running_lock;
	
	int super_fucking_contest_fuck_you; // because they want their fucking outputs
	                                    // formatted a super fucking goddamn way
} rnpc_t;



inline void add_to_guesses (rnpc_t * rnpc, int guesses)
{
	pthread_mutex_lock(&(rnpc->guesses_lock));

	if (guesses & 0x40000000)
	{
		rnpc->guesses2++;
		guesses -= 0x40000000;
	}
	
	rnpc->guesses += guesses;
	if (rnpc->guesses & 0x40000000)
	{
		rnpc->guesses -= 0x40000000;
		rnpc->guesses2++;
	}
	
	pthread_mutex_unlock(&(rnpc->guesses_lock));
}



// this is taken somewhat shamelessly and with much thanks to
// http://cc.byexamples.com/2007/04/08/non-blocking-user-input-in-loop-without-ncurses/
int kbhit ()
{
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &fds);
}



inline void print_status (rnpc_t * rnpc)
{

	int64_t total_guesses;
	int64_t guesses_per_second;

	pthread_mutex_lock(&(rnpc->guesses_lock));
	total_guesses = 0;
	total_guesses += (int64_t) rnpc->guesses2;
	total_guesses *= 0x40000000;
	total_guesses += (int64_t) rnpc->guesses;
	
	guesses_per_second = total_guesses;
	if (time(NULL) == rnpc->bench_time)
		guesses_per_second = -1;
	else
		guesses_per_second /= (int64_t) (time(NULL) - rnpc->bench_time);
	pthread_mutex_unlock(&(rnpc->guesses_lock));
	
	printf("guesses (total:sec) > %lld : %lld\n",
	        (long long int) total_guesses,
	        (long long int) guesses_per_second);
	printf("total elapsed time: %d seconds\n", (int) (time(NULL) - rnpc->bench_time));
	fflush(stdout);

}



void print_help ()
{
	// keep it alphabetical
	printf("Rainbows and Pwnies Cracker - www.rainbowsandpwnies.com\n");
	printf("Crack the hashes, win the con, rule the world\n");
	printf("\n");
	printf("-a Algorithm\n");
	printf("    <1>  MD5\n");
	printf("    <2>  NT\n");
	printf("    <3>  SHA1\n");
	printf("    <4>  MSCACHE\n");
	printf("    <5>  CRYPT\n");
	printf("    <6>  {SSHA}\n");
	printf("    <10> MD5(SALT . PLAINTEXT)\n");
	printf("-b DISABLED Brute Force Attack (requires -c and -l)\n");
	printf("-c DISABLED <charset> (for -b: ie -c 0123456789)\n"); 
	printf("-d <dictionary file> (for wordlist attack, requires -m)\n");
	printf("-f f stands for fuck you contest. it's for contest-output mode\n");
	printf("-h <hash file> (each hash on seperate line)\n");
	printf("-l DISABLED <length> (of strings bruteforce should create)\n");
	printf("-m Mangling Rulesets\n");
	printf("    Rules can be combined with ,\n");
	printf("    special_M(any) = !$@#%%^&*?.+\\-_=`~()| ... special_F(ew) = !@#$\n");
	printf("    <0> MiXeD cAsE\n");
	printf("    <1> 1337\n");
	printf("    <2> append single digit\n");
	printf("    <3> append single special_M\n");
	printf("    <4> UPPERCASE first character\n");
	printf("    <5> append two digits\n");
	printf("    <6> append year between 1921 and 2020\n");
	printf("    <7> UPPERCASE whole word\n");
	printf("    <8> Prepend digit\n");
	printf("    <9> Prepend UPPERCASE characters\n");
	printf("-n <num_threads> launches up to %d threads to crack in parallel\n", RNPC_MAX_THREADS);
	printf("-o <filename> *APPENDS* cracked hashes to file when done instead of stdout\n");
	printf("-S <single ASCII salt>\n");
	printf("-s <salt filename> loads salts from a file\n");
	printf("\n");
	printf("Examples:\n");
	printf("rnpc -a 1 -m 4,2 -d words.txt -h hashes.txt\n");
	printf("rnpc -a 1 -b -c 0123456789 -l 4 -h hashes.txt (creates 0000-9999)\n");
}



int initialize_hashdb (rnpc_t * rnpc)
{

	// load hash file as a dictionary file, process hash strings to the
	// appropriate context, and add to the hashdb.
	
	int hashes_loaded;
	int result;
	char * word;
	dictionary_file_t * dict;
	
	if (rnpc->hash_filename == NULL)
	{
		printf("it would be helpful if you gave us some hashes to crack (hint: -h <filename>)\n");
		return -1;
	}
	
	dict = dictionary_file_open (rnpc->hash_filename);
	if (dict == NULL)
	{
		printf("invalid filename for hashes: %s\n", rnpc->hash_filename);
		return -1;
	}
	
	rnpc->hashdb = hashdb_create(rnpc->hash_bloom_filter,
	                             rnpc->hash_compare,
	                             rnpc->hash_context_size);
	
	word = dictionary_next_word(dict);
	hashes_loaded = 0;
	while (word != NULL)
	{
		result = rnpc->hash_from_string(rnpc->hash_context, word);
		if (result)
			printf("found \"%s\" in hash file, not a valid hash\n", word);
		else
			hashdb_add_hash(rnpc->hashdb, rnpc->hash_context);
		word = dictionary_next_word(dict);
		hashes_loaded++;
		if (hashes_loaded % 10000 == 0)
			printf("%d hashes loaded\n", hashes_loaded);
	}
	
	dictionary_file_close(dict);
	
	return 0;

}

	

int init_hash (rnpc_t * rnpc)
{
	rnpc->hash_set_salt = NULL;
	switch (rnpc->hash_algo)
	{
		case 1 :
			rnpc->hash_to_string    = md5_to_string;
			rnpc->hash_from_string  = md5_from_string;
			rnpc->hash_password     = md5_password;
			rnpc->hash_bloom_filter = md5_bloom_filter;
			rnpc->hash_compare      = md5_compare;
			rnpc->hash_context      = (void *) malloc(sizeof(struct md5_context));
			rnpc->hash_context_size = sizeof(struct md5_context);
			return 0;
		case 2 :
			rnpc->hash_to_string    = nt_to_string;
			rnpc->hash_from_string  = nt_from_string;
			rnpc->hash_password     = nt_password;
			rnpc->hash_bloom_filter = nt_bloom_filter;
			rnpc->hash_compare      = nt_compare;
			rnpc->hash_context      = (void *) malloc(sizeof(struct nt_context));
			rnpc->hash_context_size = sizeof(struct nt_context);
			return 0;
		case 3 :
			rnpc->hash_to_string    = sha1_to_string;
			rnpc->hash_from_string  = sha1_from_string;
			rnpc->hash_password     = sha1_password;
			rnpc->hash_bloom_filter = sha1_bloom_filter;
			rnpc->hash_compare      = sha1_compare;
			rnpc->hash_context      = (void *) malloc(sizeof(struct sha1_context));
			rnpc->hash_context_size = sizeof(struct sha1_context);
			return 0;
		case 4 :
			rnpc->hash_to_string    = mscache_to_string;
			rnpc->hash_from_string  = mscache_from_string;
			rnpc->hash_password     = mscache_password;
			rnpc->hash_bloom_filter = mscache_bloom_filter;
			rnpc->hash_compare      = mscache_compare;
			rnpc->hash_context      = (void *) malloc(sizeof(struct mscache_context));
			rnpc->hash_context_size = sizeof(struct mscache_context);
			rnpc->hash_set_salt     = mscache_set_salt;
			return 0;
		case 5 :
			rnpc->hash_to_string    = crypt_to_string;
			rnpc->hash_from_string  = crypt_from_string;
			rnpc->hash_password     = crypt_password;
			rnpc->hash_bloom_filter = crypt_bloom_filter;
			rnpc->hash_compare      = crypt_compare;
			rnpc->hash_context      = (void *) malloc(sizeof(struct crypt_context));
			rnpc->hash_context_size = sizeof(struct crypt_context);
			rnpc->hash_set_salt     = crypt_set_salt;
			return 0;
		case 6 :
			rnpc->hash_to_string    = ssha_to_string;
			rnpc->hash_from_string  = ssha_from_string;
			rnpc->hash_password     = ssha_password;
			rnpc->hash_bloom_filter = ssha_bloom_filter;
			rnpc->hash_compare      = ssha_compare;
			rnpc->hash_context      = (void *) malloc(sizeof(struct ssha_context));
			rnpc->hash_context_size = sizeof(struct ssha_context);
			rnpc->hash_set_salt     = ssha_set_salt;
			return 0;
	}
	
	return 1;
}



int process_command_line_arguments (rnpc_t * rnpc, int argc, char * argv[])
{

	int c;
	
	rnpc->salt_function = NULL;
	
	rnpc->attack_type         = RNPC_ATTACK_NONE;
	rnpc->allow_stdin_updates = 1;
	rnpc->brute_charset       = NULL;
	rnpc->brute_length        = 0;
	rnpc->hash_filename       = NULL;
	rnpc->num_threads         = 1;
	rnpc->output_filename     = NULL;
	rnpc->output_to_file      = 0;
	rnpc->super_fucking_contest_fuck_you = 0;
	rnpc->using_salt          = RNPC_SALT_NONE;
	rnpc->using_mangle        = 0;

	if (argc == 1)
	{
		print_help();
		return -1;
	}

	while ((c = getopt(argc, argv, "a:bc:d:fh:l:m:n:o:S:s:")) != -1)
	{
		switch (c)
		{
			// keep it alphabetical
			case 'a' :
				if (strcmp("1", optarg) == 0)
					rnpc->hash_algo = RNPC_HASH_ALGO_MD5;
				else if (strcmp("2", optarg) == 0)
					rnpc->hash_algo = RNPC_HASH_ALGO_NT;
				else if (strcmp("3", optarg) == 0)
					rnpc->hash_algo = RNPC_HASH_ALGO_SHA1;
				else if (strcmp("4", optarg) == 0)
				{
					rnpc->hash_algo = RNPC_HASH_ALGO_MSCACHE;
					rnpc->using_salt = RNPC_SALT_HASH;
				}
				else if (strcmp("5", optarg) == 0)
				{
					rnpc->hash_algo = RNPC_HASH_ALGO_CRYPT;
					rnpc->using_salt = RNPC_SALT_HASH;
				}
				else if (strcmp("6", optarg) == 0)
				{
					rnpc->hash_algo = RNPC_HASH_ALGO_SSHA;
					rnpc->using_salt = RNPC_SALT_HASH;
				}
				else if (strcmp("10", optarg) == 0)
				{
					rnpc->hash_algo = RNPC_HASH_ALGO_MD5;
					rnpc->salt_function = salt_1;
				}
				else
				{
					printf("invalid algorithm: %s\n", optarg);
					return -1;
				}
				break;
			case 'b' :
				rnpc->attack_type = RNPC_ATTACK_BRUTEFORCE;
				break;
			case 'c' :
				rnpc->brute_charset = (char *) malloc(strlen(optarg) + 1);
				strcpy(rnpc->brute_charset, optarg);
				break;
			case 'd' :
				rnpc->dictionary_filename = (char *) malloc(strlen(optarg) + 1);
				strcpy(rnpc->dictionary_filename, optarg);
				rnpc->attack_type = RNPC_ATTACK_WORDLIST;
				if (strcmp(rnpc->dictionary_filename, "stdin") == 0)
					rnpc->allow_stdin_updates = 0;
				break;
			case 'f' :
				rnpc->super_fucking_contest_fuck_you = 1;
				printf("and here we find ourselves\n");
				break;
			case 'h' :
				rnpc->hash_filename = (char *) malloc(strlen(optarg) + 1);
				strcpy(rnpc->hash_filename, optarg);
				break;
			case 'l' :
				rnpc->brute_length = atoi(optarg);
				break;
			case 'm' :
				// screw you atoi!
				rnpc->using_mangle = 1;
				rnpc->mangle_settings = (char *) malloc(strlen(optarg) + 1);
				strcpy(rnpc->mangle_settings, optarg);
				break;
			case 'n' :
				rnpc->num_threads = atoi(optarg) % RNPC_MAX_THREADS;
				break;
			case 'o' :
				rnpc->output_to_file = 1;
				rnpc->output_filename = (char *) malloc(strlen(optarg) + 1);
				strcpy(rnpc->output_filename, optarg);
				break;
			case 'S' :
				rnpc->using_salt = RNPC_SALT_SINGLE;
				rnpc->salt = (char *) malloc(strlen(optarg) + 1);
				strcpy(rnpc->salt, optarg);
				break;
			case 's' :
				rnpc->using_salt = RNPC_SALT_FILE;
				rnpc->salt_filename = (char *) malloc(strlen(optarg) + 1);
				strcpy(rnpc->salt_filename, optarg);
				break;
			case '?' :
				if ((optopt == 'a') || (optopt == 'd') || (optopt == 'h')
				    || (optopt == 'm'))
				{
					printf("%c requires an argument\n", optopt);
					return -1;
				}
				else if (isprint(optopt))
				{
					printf("invalid argument %c\n", optopt);
					return -1;
				}
		}
	}
	
	return 0;
	
}



// where the magic happens. lots of code for lots of different situations.
// we are trying to as much logic as possible out of the loops
int64_t crack_wordlist (rnpc_thread_info_t * thread)
{

	int guesses;  // cache number of guesses made by thread before sending it
	              // to add_to_guesses()

	int salt_length;
	int strlen_plaintext;
	
	int words_cached;
	
	mangle_data_t * mangle_data;
	
	// we try to keep the coniditionals out of the loops. however, 
	// this really drags out the code.
	// complaints to noreply@rainbowsandpwnies.com
	
	guesses = 0;
	
	// MANGLE
	if (thread->rnpc->using_mangle)
	{
		mangle_data = mangle_data_create();
		mangle_load_from_string(mangle_data, thread->rnpc->mangle_settings);
		if (mangle_data == NULL)
		{
			return -1;
		}
		// SALT_HASH + MANGLE
		if (thread->rnpc->using_salt == RNPC_SALT_HASH)
		{
			words_cached = 1;
			while (1)
			{
				words_cached = dictionary_next_word_threadsafe_many(thread->rnpc->dictionary,
				                                             thread->words_cache,
				                                             RNPC_THREAD_WORDS_CACHE_SIZE,
				                                             RNPC_THREAD_WORDS_CACHE_N);
				if (words_cached == 0)
					break;
				while (words_cached)
				{
					words_cached--;
					mangle_set_plaintext(mangle_data, thread->words_cache[words_cached]);
					while (mangle(mangle_data, thread->mangle_word))
					{
						thread->rnpc->hash_password(thread->hash_context,
						                            (unsigned char *) thread->mangle_word,
						                            strlen(thread->mangle_word));
						if (thread->rnpc->hash_compare(thread->hash_context, thread->salted_hash_context) == 0)
						{
							hashdb_add_plaintext(thread->rnpc->hashdb, thread->hash_context,
							                     thread->mangle_word);
						}
						guesses++;
						if (guesses & 0x100000)
						{
							add_to_guesses(thread->rnpc, guesses);
							guesses ^= guesses;
						}
					}
				}
			}
		}
		// NO SALT + MANGLE
		else if (thread->rnpc->using_salt == RNPC_SALT_NONE)
		{
			words_cached = 1;
			while (1)
			{
				words_cached = dictionary_next_word_threadsafe_many(thread->rnpc->dictionary,
					                                         thread->words_cache,
					                                         RNPC_THREAD_WORDS_CACHE_SIZE,
					                                         RNPC_THREAD_WORDS_CACHE_N);
				if (words_cached == 0)
					break;
				while (words_cached)
				{
					words_cached--;
					mangle_set_plaintext(mangle_data, thread->words_cache[words_cached]);
					while (mangle(mangle_data, thread->mangle_word))
					{
						thread->rnpc->hash_password(thread->hash_context,
							                        (unsigned char *) thread->mangle_word,
							                        strlen(thread->mangle_word));
						if (hashdb_check_hash(thread->rnpc->hashdb, thread->hash_context))
						{
							hashdb_add_plaintext(thread->rnpc->hashdb,
								                 thread->hash_context,
								                 thread->mangle_word);
						}
						guesses++;
						if (guesses & 0x100000)
						{
							add_to_guesses(thread->rnpc, guesses);
							guesses ^= guesses;
						}
					}
				}
			}
		}
		// SALT + MANGLE
		else
		{
			salt_length = strlen(thread->rnpc->salt);
			words_cached = 1;
			while (1)
			{
				words_cached = dictionary_next_word_threadsafe_many(thread->rnpc->dictionary,
					                                         thread->words_cache,
					                                         RNPC_THREAD_WORDS_CACHE_SIZE,
					                                         RNPC_THREAD_WORDS_CACHE_N);
				if (words_cached == 0)
					break;
				while (words_cached)
				{
					words_cached--;
					mangle_set_plaintext(mangle_data, thread->words_cache[words_cached]);
					while (mangle(mangle_data, thread->mangle_word))
					{
						strlen_plaintext = strlen(thread->mangle_word);
						thread->rnpc->salt_function((unsigned char *) thread->mangle_word, 
							                        strlen_plaintext,
							                        (unsigned char *) thread->rnpc->salt,
							                        salt_length,
							                        (unsigned char *) thread->salt_word);
						thread->rnpc->hash_password(thread->hash_context,
							                (unsigned char *) thread->salt_word,
							                strlen_plaintext + salt_length);
						if (hashdb_check_hash(thread->rnpc->hashdb, thread->hash_context))
						{
							hashdb_add_salted_plaintext(thread->rnpc->hashdb,
								                        thread->hash_context,
								                        thread->mangle_word,
								                        thread->rnpc->salt);
						}
						guesses++;
						if (guesses & 0x100000)
						{
							add_to_guesses(thread->rnpc, guesses);
							guesses ^= guesses;
						}
					}
				}
			}
		}
		mangle_data_destroy(mangle_data);
	}
	// NO MANGLE
	else
	{
		// SALT + NO_MANGLE
		if ((thread->rnpc->using_salt == RNPC_SALT_SINGLE) || (thread->rnpc->using_salt == RNPC_SALT_FILE))
		{
			salt_length = strlen(thread->rnpc->salt);
			words_cached = 1;
			while (1)
			{
				words_cached = dictionary_next_word_threadsafe_many(thread->rnpc->dictionary,
					                                         thread->words_cache,
					                                         RNPC_THREAD_WORDS_CACHE_SIZE,
					                                         RNPC_THREAD_WORDS_CACHE_N);
				if (words_cached == 0)
					break;
				while (words_cached)
				{
					words_cached--;
					strlen_plaintext = strlen(thread->words_cache[words_cached]);
					thread->rnpc->salt_function((unsigned char *) thread->words_cache[words_cached], 
						                        strlen_plaintext,
						                        (unsigned char *) thread->rnpc->salt,
						                        salt_length,
						                        (unsigned char *) thread->salt_word);
					thread->rnpc->hash_password(thread->hash_context,
						                (unsigned char *) thread->salt_word,
						                strlen_plaintext + salt_length);

					if (hashdb_check_hash(thread->rnpc->hashdb, thread->hash_context))
					{
						hashdb_add_salted_plaintext(thread->rnpc->hashdb,
							                        thread->hash_context,
							                        thread->words_cache[words_cached],
							                        thread->rnpc->salt);
					}
					guesses++;
					if (guesses & 0x100000)
					{
						add_to_guesses(thread->rnpc, guesses);
						guesses ^= guesses;
					}
				}
			}
		}
		// SALT_HASH + NO MANGLE
		else if (thread->rnpc->using_salt == RNPC_SALT_HASH)
		{
			words_cached = 1;
			while (1)
			{
				words_cached = dictionary_next_word_threadsafe_many(thread->rnpc->dictionary,
					                                         thread->words_cache,
					                                         RNPC_THREAD_WORDS_CACHE_SIZE,
					                                         RNPC_THREAD_WORDS_CACHE_N);
				if (words_cached == 0)
					break;
				while (words_cached)
				{
					words_cached--;
					thread->rnpc->hash_password(thread->hash_context,
						          (unsigned char *) thread->words_cache[words_cached], 
						          strlen(thread->words_cache[words_cached]));
					if (thread->rnpc->hash_compare(thread->salted_hash_context, thread->hash_context) == 0)
					{
						hashdb_add_plaintext(thread->rnpc->hashdb, thread->hash_context, 
						                     thread->words_cache[words_cached]);
					}
					guesses++;
					if (guesses & 0x100000)
					{
						add_to_guesses(thread->rnpc, guesses);
						guesses ^= guesses;
					}
				}
			}
		}
		// NO SALT + NO MANGLE
		else
		{
			words_cached = 1;
			while (1)
			{
				words_cached = dictionary_next_word_threadsafe_many(thread->rnpc->dictionary,
					                                         thread->words_cache,
					                                         RNPC_THREAD_WORDS_CACHE_SIZE,
					                                         RNPC_THREAD_WORDS_CACHE_N);
				if (words_cached == 0)
					break;
				while (words_cached)
				{
					words_cached--;
					thread->rnpc->hash_password(thread->hash_context,
						          (unsigned char *) thread->words_cache[words_cached], 
						          strlen(thread->words_cache[words_cached]));
					if (hashdb_check_hash(thread->rnpc->hashdb, thread->hash_context))
					{
						hashdb_add_plaintext(thread->rnpc->hashdb, thread->hash_context, 
						                     thread->words_cache[words_cached]);
					}
					guesses++;
					if (guesses & 0x100000)
					{
						add_to_guesses(thread->rnpc, guesses);
						guesses ^= guesses;
					}
				}
			}
		}
	}
	
	add_to_guesses(thread->rnpc, guesses);
	
	pthread_mutex_lock(&(thread->rnpc->threads_running_lock));
	thread->rnpc->threads_running--;
	pthread_mutex_unlock(&(thread->rnpc->threads_running_lock));
	
	return 0;
	
}



void crack_brute (rnpc_t * rnpc)
{
/*
	int64_t i;
	int salt_length;
	int64_t count = 0;
	
	char * plaintext;
	unsigned char * salted_plaintext;
	
	// NO SALT
	if ((rnpc->using_salt == RNPC_SALT_NONE) || (rnpc->using_salt == RNPC_SALT_HASH))
	{
		for (i = 0; i < total_brutes(rnpc->brute_charset, rnpc->brute_length); i++)
		{
			plaintext = brute(rnpc->brute_charset, rnpc->brute_length, i);
			rnpc->hash_password(rnpc->hash_context,
			                    (unsigned char *) plaintext, 
			                    rnpc->brute_length);
			if (hashdb_check_hash(rnpc->hashdb, rnpc->hash_context))
			{
				printf("+");
				hashdb_add_plaintext(rnpc->hashdb, rnpc->hash_context, plaintext);
			}
		}
	}
	// SALT
	else
	{
		salt_length = strlen(rnpc->salt);
		for (i = 0; i < total_brutes(rnpc->brute_charset, rnpc->brute_length); i++)
		{
			plaintext = brute(rnpc->brute_charset, rnpc->brute_length, i);
			salted_plaintext = rnpc->salt_function((unsigned char *) plaintext,
				                            rnpc->brute_length,
				                             (unsigned char *) rnpc->salt,
				                             salt_length);
			rnpc->hash_password(rnpc->hash_context, salted_plaintext, rnpc->brute_length + salt_length);
			if (hashdb_check_hash(rnpc->hashdb, rnpc->hash_context))
			{
				printf("+");
				hashdb_add_salted_plaintext(rnpc->hashdb, rnpc->hash_context, plaintext, rnpc->salt);
			}
			count++;
		}
	}
*/
}



int crack (rnpc_t * rnpc)
{

	int i;
	char c;
	pthread_attr_t attr;
	
	// read() was being a bitch down there after kbhit(), so this right here
	// makes stdin non-blocking
	if (rnpc->allow_stdin_updates)
	{
		int flags = fcntl(0, F_GETFL, 0);
		fcntl(0, F_SETFL, flags | O_NONBLOCK);
	}
	
	rnpc->threads_running = rnpc->num_threads;
		
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	hashdb_set_options(rnpc->hashdb, HASHDB_OPTIONS_THREADSAFE);
	if (rnpc->attack_type == RNPC_ATTACK_WORDLIST)
	{
		rnpc->dictionary = dictionary_file_open(rnpc->dictionary_filename);
		if (rnpc->dictionary == NULL)
		{
			printf("error loading hashes from %s\n", rnpc->dictionary_filename);
			return -1;
		}
		dictionary_set_options(rnpc->dictionary, DICTIONARY_OPTIONS_THREADSAFE);
		for (i = 0; i < rnpc->num_threads; i++)
			pthread_create(&(rnpc->thread_infos[i].thread), &attr,
			               crack_wordlist, (void *) &(rnpc->thread_infos[i]));
		pthread_attr_destroy(&attr);
		while (1)
		{
			pthread_mutex_lock(&(rnpc->threads_running_lock));
			if (rnpc->threads_running == 0)
			{
				pthread_mutex_unlock(&(rnpc->threads_running_lock));
				break;
			}
			pthread_mutex_unlock(&(rnpc->threads_running_lock));
			usleep(RNPC_KB_WAIT_TIME);
			if (rnpc->allow_stdin_updates)
			{
				if (kbhit())
				{
					print_status(rnpc);
					read(0, &c, sizeof(c));
				}
			}
		}
		//for (i = 0; i < rnpc->num_threads; i++)
			//pthread_join(rnpc->thread_infos[i].thread, NULL);
		dictionary_file_close(rnpc->dictionary);
	}

	/*
		else if (rnpc->attack_type == RNPC_ATTACK_BRUTEFORCE)
			crack_brute(rnpc);
	*/
	
	return 0;

}



int rnpc_salt_hash (rnpc_t * rnpc)
{

	int i;

	char * hash_from_file;
	dictionary_file_t * hashes_file;
	
	for (i = 0; i < RNPC_MAX_THREADS; i++)
		rnpc->thread_infos[i].salted_hash_context = malloc(rnpc->hash_context_size);

	hashes_file = dictionary_file_open(rnpc->hash_filename);
	if (hashes_file == NULL)
	{
		printf("couldn't open %s\n", rnpc->hash_filename);
		return -1;
	}
	
	hash_from_file = dictionary_next_word(hashes_file);
	
	while (hash_from_file != NULL)
	{
		// need to salt each thread
		for (i = 0; i < RNPC_MAX_THREADS; i++)
		{
			rnpc->hash_set_salt(rnpc->thread_infos[i].hash_context, (unsigned char *) hash_from_file,
		                    strlen(hash_from_file));
		    rnpc->hash_from_string(rnpc->thread_infos[i].salted_hash_context, hash_from_file);
		}
		crack(rnpc);
		hash_from_file = dictionary_next_word(hashes_file);
	}
	dictionary_file_close(hashes_file);

	return 0;

}
		


int main (int argc, char * argv[])
{

	int i;
	int plaintexts_found;
	char * hash_string;
	char file_out_buffer[1024];
	
	FILE * fh = NULL;
	
	rnpc_t rnpc;
	
	/********************************
	* PROCESS COMMAND-LINE ARGUMENTS
	*/
	if (process_command_line_arguments(&rnpc, argc, argv))
		return -1;
	
	/********************************
	* POINTERS/MALLOC FOR HASH ALGOS
	*/
	if (init_hash(&rnpc))
	{
		printf("something failed while initializing hash.\n");
		printf("rnpc_hash_algo is %d\n", rnpc.hash_algo);
		return -1;
	}
	
	/*******************
	* INITIALIZE HASHDB
	*/
	printf("initializing hashdb... ");fflush(stdout);
	if (initialize_hashdb(&rnpc))
		return -1;
	printf("done\n");fflush(stdout);
	
	/*************************************
	* INITIALIZE SOME RNPC_T VARIABLES
	*/
	
	rnpc.guesses = 0;
	rnpc.guesses2 = 0;
	rnpc.bench_time = time(NULL);
	pthread_mutex_init(&(rnpc.guesses_lock), NULL);
	pthread_mutex_init(&(rnpc.threads_running_lock), NULL);
	
	// initialize the thread_info_t(s)
	// (need to find a better place to do this)
	for (i = 0; i < RNPC_MAX_THREADS; i++)
	{
		rnpc.thread_infos[i].rnpc = &rnpc;
		rnpc.thread_infos[i].hash_context = malloc(rnpc.hash_context_size);
	}
	
	/**************************************
	* GET OUT THERE AND FIND THOSE HASHES!
	*/
	
	if (rnpc.using_salt == RNPC_SALT_HASH)
	{
		rnpc_salt_hash (&rnpc);
	}
	else if (rnpc.using_salt == RNPC_SALT_FILE)
	{
		rnpc.salt_file_dict = dictionary_file_open(rnpc.salt_filename);
		if (rnpc.salt_file_dict == NULL)
		{
			printf("error loading salts from %s\n", rnpc.salt_filename);
			return -1;
		}
		
		while ((rnpc.salt = dictionary_next_word(rnpc.salt_file_dict)) != NULL)
		{
			if (crack(&rnpc))
				return -1;
		}
		dictionary_file_close(rnpc.salt_file_dict);
	}
	else
	{
		if (crack(&rnpc))
			return -1;
	}
	
	
	
	printf ("\n");
	/*********************
	* OUTPUT FOUND HASHES
	*/
	
	if (rnpc.output_to_file)
	{
		fh = fopen(rnpc.output_filename, "a");
		if (fh == NULL)
		{
			printf("error opening output file, sending to stdout instead\n");
			rnpc.output_to_file = 0;
		}
	}
	
	plaintexts_found = 0;
	hashdb_iterator_reset(rnpc.hashdb);
	while (hashdb_iterate(rnpc.hashdb))
	{
		hash_string = rnpc.hash_to_string(rnpc.hashdb->context);
		if (rnpc.output_to_file)
		{
			if (rnpc.super_fucking_contest_fuck_you == 1)
			{
				snprintf(file_out_buffer, 1024, "%s\n", 
				         rnpc.hashdb->plaintext);
			}
			else
			{
				if (rnpc.hashdb->salt == NULL)
					snprintf(file_out_buffer, 1024, "%s\t%s\n", 
					         hash_string, rnpc.hashdb->plaintext);
				else
					snprintf(file_out_buffer, 1024, "%s\t%s:%s\n", 
					         hash_string, rnpc.hashdb->salt, rnpc.hashdb->plaintext);
			}
			fwrite(file_out_buffer, 1, strlen(file_out_buffer), fh);
		}
		else
		{
			if (rnpc.hashdb->salt == NULL)
				printf("%s\t%s\n", hash_string, rnpc.hashdb->plaintext);
			else
				printf("%s\t%s:%s\n",
				       hash_string, rnpc.hashdb->salt, rnpc.hashdb->plaintext);
		}
		plaintexts_found++;
	}
	printf("%d plaintexts found\n", plaintexts_found);

	print_status(&rnpc);
	
	if (rnpc.output_to_file)
		fclose(fh);
		
	hashdb_destroy(rnpc.hashdb);
	
	return 0;

}
