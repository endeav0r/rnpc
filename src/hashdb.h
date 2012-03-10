#ifndef hashdb_HEADER
#define hashdb_HEADER

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHDB_SHOW_BLOOM_FILTER_MISSES 0

#define HASHDB_OPTIONS_THREADSAFE 1

/*
* This is where we 
*  A) load the hashes we want to crack
*  B) compare generated hashes to those hashes
*/

/*
* The rundown (theory)
*  User adds hashes to hashdb. These hashes go A) into the bloom_filter, and
*  B) into the hash_tree. Hashes are looked up against the bloom_filter first
*  (because it's fast), and if there's a match, we trace it down the hash_tree
*  to confirm it's not a false positive. If we find it in the hash tree, we
*  return a 1, and the user then adds that hash/plaintext pair to the hash
*  list
*/

/*
* NOTE ABOUT COMPARE FUNCTION FOR TREE
* compare_function (context placing/finding, context in tree)
*/



#define HASHDB_BLOOM_FILTER_SIZE 0x00FFFFFF
#define HASHDB_BLOOM_FILTER_MASK 0x0FFFFFFF

#define HASHDB_TREES_SIZE 0x000010000
#define HASHDB_TREES_MASK 0x00000FFFF

/*
* This struct requires a hash-dependent function that returns -1, 0, or 1
* based on the context of that hash, which can be used for ordering the nodes.
* The format for this function is:
*
* int custom_hash_compare (void * a, void * b);
*
* where a and b are both pointers to instances of the hash's context
*
* This is an AA tree
*/
typedef struct hashdb_hash_tree_s
{
	void * context; // this is hash dependent. for example, if we're attacking
	                // a md5 hash, this will be set to an instance of
	                // (struct md5_context)
	struct hashdb_hash_tree_s * left;
	struct hashdb_hash_tree_s * right;
	int level; // AA tree level
	int found; // you know, thought about adding deleting of nodes from the tree,
	           // but then i was drinking, and i was like no, fuck it, fuck that,
	           // just add a stupid fucking 4 byte integer, and when the shit
	           // isn't found, make it 0, and when it is, make it 1, and then that
	           // way we'll only find shit once. yeah. that'll work. fuck it.
	           // P.S. stop drinking
} hashdb_hash_tree_t;



typedef struct hashdb_hash_list_s
{
	void * context;
	char * plaintext;
	char * salt;
	struct hashdb_hash_list_s * next;
} hashdb_hash_list_t;



/*
* This struct requires a hash-dependent function that modifies
* this struct, ready to be inserted/checked against the hash filter. Basically,
* supplied a context of the hash and a pointer to this struct, the hash
* function must fill out this struct. HASHDB_BLOOMFILTER_MASK is provided to
* make this process painless and quick. An example is provided:
*
* void custom_hash_bloom_filter (void * c,
*                                hashdb_bloom_filter_result_s * r)
* {
* 	r->one =   ((struct context *) c)->A & HASH_BLOOM_FILTER_MASK;
*  	r->two =   ((struct context *) c)->B & HASH_BLOOM_FILTER_MASK;
* 	r->three = ((struct context *) c)->C & HASH_BLOOM_FILTER_MASK;
*  	r->four =  ((struct context *) c)->D & HASH_BLOOM_FILTER_MASK;
* }
* 
* hash functions who's context do not fit this neatly will need to make do
*/
typedef struct hashdb_bloom_filter_result_s
{

	unsigned int one;
	unsigned int two;
	unsigned int three;
	unsigned int four;

} hashdb_bloom_filter_result_t;



typedef struct hashdb_s
{

	unsigned int options;

	int context_size;
	
	pthread_mutex_t lock;

	unsigned char * bloom_filter;
	
	hashdb_hash_tree_t * trees[HASHDB_TREES_SIZE];
	
	hashdb_hash_list_t * first;
	hashdb_hash_list_t * last;
	hashdb_hash_list_t * iterator;
	
	void * context;
	char * plaintext;
	char * salt;
	
	void (* bloom_filter_function) (void *, 
	                                     hashdb_bloom_filter_result_t *);
	
	int  (* compare_function) (void *, void *);

} hashdb_t;


/*
* This function creates a hashdb object, prepares it to start receiving hashes,
* and returns that object to the user.
* The hash_bloom_filter_function and context_compare functions should be of
* the format found above. context_size is the size of the hash's context
* struct in bytes (IE sizeof(struct md5_context))
*/
hashdb_t * hashdb_create (void (* hash_bloom_filter_function) 
                               (void *, hashdb_bloom_filter_result_t *),
                          int (* hash_compare_function) (void *, void *),
                          int context_size);
                          
inline void hashdb_set_options (hashdb_t * hashdb, unsigned int options);

/*
* Pass the context for the hash to this function, and it will be placed in
* both the bloom filter and the hash tree, ready to be checked against
*/
void hashdb_add_hash (hashdb_t * hashdb, void *);

/*
* Insert a node into the AA tree
*/
hashdb_hash_tree_t * hashdb_tree_insert (hashdb_t * hashdb, hashdb_hash_tree_t * tree, void * context);

/*
* Skew function for our AA tree
*/
inline hashdb_hash_tree_t * hashdb_tree_skew   (hashdb_t * hashdb, hashdb_hash_tree_t * tree);

/*
* Split function for our AA tree
*/
inline hashdb_hash_tree_t * hashdb_tree_split  (hashdb_t * hashdb, hashdb_hash_tree_t * tree);

/*
* Pass the context for a hash here, and it will be checked against the bloom
* filter and then the hash tree. Returns 0 if the hash was not found, 1 if
* the hash was found.
*/
int hashdb_check_hash (hashdb_t * hashdb, void *);

/*
* Pass both the context for the hash, and the plaintext to the hash. This adds
* the hash to the list of found hashes
*/
void hashdb_add_plaintext (hashdb_t * hashdb, void * context, char * plaintext);

/*
* Pass the context for the hash, the plaintext and the salt;
*/
void hashdb_add_salted_plaintext (hashdb_t * hashdb, void * context, char * plaintext, char * salt);

/*
* Places the hash iterator to the beginning, ready to iterate through the list
* of found hashes
*/
void hashdb_iterator_reset (hashdb_t * hashdb);

/*
* Iterates the hashdb iterator to the next hash in the hash list. The plaintext
* and the context for this hash will be pointed to by hashdb->plaintext and
* hashdb->context (you will need to cast hashdb->context to the appropriate
* hash context).
* Returns 1 as long as there is a valid result. When it reaches the end of the
* list, returns 0
*/
int hashdb_iterate (hashdb_t * hashdb);

/*
* Destroys/frees all memory
*/
void hashdb_destroy(hashdb_t * hashdb);

// used internally by hashdb_destroy
void hashdb_destroy_tree (hashdb_hash_tree_t * tree);
	
#endif
