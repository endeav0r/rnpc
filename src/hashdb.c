#include "hashdb.h"


/*
* This function creates a hashdb object, prepares it to start receiving hashes,
* and returns that object to the user.
* The hash_bloom_filter_function and context_compare functions should be of
* the format found above.
*/
hashdb_t * hashdb_create (void (* bloom_filter_function) 
                               (void *, hashdb_bloom_filter_result_t *),
                          int (* compare_function) (void *, void *),
                          int context_size)
{

	int i;

	hashdb_t * hashdb;
	
	hashdb = (hashdb_t *) malloc(sizeof(hashdb_t));
	
	hashdb->bloom_filter = (unsigned char *) malloc(HASHDB_BLOOM_FILTER_SIZE);
	
	memset(hashdb->bloom_filter, 0, HASHDB_BLOOM_FILTER_SIZE);
	
	pthread_mutex_init(&(hashdb->lock), NULL);
	
	for (i = 0; i < HASHDB_TREES_SIZE; i++)
		hashdb->trees[i] = NULL; // NULL technically doesn't have to be 0
	
	hashdb->first = NULL;
	hashdb->last = NULL;
	hashdb->iterator = NULL;
	
	hashdb->plaintext = NULL;
	hashdb->context = NULL;
	
	hashdb->bloom_filter_function = bloom_filter_function;
	hashdb->compare_function = compare_function;
	hashdb->context_size = context_size;
	
	return hashdb;

}



inline void hashdb_set_options (hashdb_t * hashdb, unsigned int options)
{
	hashdb->options = options;
}



/*
* Pass the context for the hash to this function, and it will be placed in
* both the bloom filter and the hash tree, ready to be checked against
*/
void hashdb_add_hash (hashdb_t * hashdb, void * context)
{

	unsigned int mask;
	unsigned char value;
	hashdb_bloom_filter_result_t result;
	
	// add to bloom_filter
	hashdb->bloom_filter_function(context, &result);

	value = result.one & 0xFF;
	mask  = result.one >> 8;
	hashdb->bloom_filter[mask] |= value;
	value = result.two & 0xFF;
	mask  = result.two >> 8;
	hashdb->bloom_filter[mask] |= value;
	value = result.three & 0xFF;
	mask  = result.three >> 8;
	hashdb->bloom_filter[mask] |= value;
	value = result.four & 0xFF;
	mask  = result.four >> 8;
	hashdb->bloom_filter[mask] |= value;
	
	// add to binary tree
	hashdb->trees[result.one & HASHDB_TREES_MASK] = hashdb_tree_insert(hashdb,
	                                  hashdb->trees[result.one & HASHDB_TREES_MASK],
	                                  context);
	
}



/*
* Insert a node into the AA tree
*/
hashdb_hash_tree_t * hashdb_tree_insert (hashdb_t * hashdb, hashdb_hash_tree_t * tree, void * context)
{

	int compare;
	
	if (tree == NULL)
	{
		tree = (hashdb_hash_tree_t *) malloc(sizeof(hashdb_hash_tree_t));
		tree->context = (void *) malloc(hashdb->context_size);
		memcpy(tree->context, context, hashdb->context_size);
		tree->left = NULL;
		tree->right = NULL;
		tree->found = 0;
		tree->level = 1;
		return tree;
	}

	compare = hashdb->compare_function(context, tree->context);
	if (compare < 0)
		tree->left = hashdb_tree_insert(hashdb, tree->left, context);
	else if (compare > 0)
		tree->right = hashdb_tree_insert(hashdb, tree->right, context);
	else
		return NULL;
	
	tree = hashdb_tree_skew(hashdb, tree);
	tree = hashdb_tree_split(hashdb, tree);
	
	return tree;
	
}



/*
* Skew function for our AA tree
*/
inline hashdb_hash_tree_t * hashdb_tree_skew (hashdb_t * hashdb, hashdb_hash_tree_t * tree)
{

	hashdb_hash_tree_t * result;

	if (tree == NULL)
		return NULL;
	else if (tree->left == NULL)
		return tree;
	else if (tree->level == tree->left->level)
	{
		result = tree->left;
		tree->left = result->right;
		result->right = tree;
		return result;
	}
	else
		return tree;

}



/*
* Split function for our AA tree
*/
inline hashdb_hash_tree_t * hashdb_tree_split (hashdb_t * hashdb, hashdb_hash_tree_t * tree)
{

	hashdb_hash_tree_t * result;

	if (tree == NULL)
		return NULL;
	else if (tree->right == NULL)
		return tree;
	else if (tree->right->right == NULL)
		return tree;
	else if (tree->level == tree->right->right->level)
	{
		result = tree->right;
		tree->right = result->left;
		result->left = tree;
		result->level = result->level + 1;
		return result;
	}
	else
		return tree;

}



/*
* Pass the context for a hash here, and it will be checked against the bloom
* filter and then the hash tree. Returns 0 if the hash was not found, 1 if
* the hash was found.
*/
int hashdb_check_hash (hashdb_t * hashdb, void * context)
{

	//unsigned int mask;
	//unsigned int value;
	int compare;

	hashdb_bloom_filter_result_t result;
	hashdb_hash_tree_t * tree;
	
	//__builtin_prefetch(&(hashdb->trees));
	
	hashdb->bloom_filter_function(context, &result);
	
	/*
	__builtin_prefetch(&(hashdb->bloom_filter[result.one >> 8]));
	__builtin_prefetch(&(hashdb->bloom_filter[result.two >> 8]));
	__builtin_prefetch(&(hashdb->bloom_filter[result.three >> 8]));
	__builtin_prefetch(&(hashdb->bloom_filter[result.four >> 8]));
		
	value = result.one & 0xFF;
	mask  = result.one >> 8;
	if ((hashdb->bloom_filter[mask] & value) != value)
		return 0;
	

	value = result.two & 0xFF;
	mask  = result.two >> 8;
	if ((hashdb->bloom_filter[mask] & value) != value)
		return 0;
		
	value = result.three & 0xFF;
	mask  = result.three >> 8;
	if ((hashdb->bloom_filter[mask] & value) != value)
	    return 0;
	value = result.four & 0xFF;
	mask  = result.four >> 8;
	if ((hashdb->bloom_filter[mask] & value) != value)
		return 0;
	*/
	
	tree = hashdb->trees[result.one & HASHDB_TREES_MASK];
	
	while (tree != NULL)
	{
		compare = hashdb->compare_function(context, tree->context);
		
		if (compare == 0)
		{
			if (tree->found == 0)
			{
				tree->found = 1;
				return 1;
			}
			break;
		}
		
		else if (compare < 0)
			tree = tree->left;
			
		else if (compare > 0)
			tree = tree->right;
	}
	
	#if HASHDB_SHOW_BLOOM_FILTER_MISSES == 1
		printf("-");
	#endif
	
	return 0;
	
}
				


/*
* Pass both the context for the hash, and the plaintext to the hash. This adds
* the hash to the list of found hashes
*/
void hashdb_add_plaintext (hashdb_t * hashdb, void * context, char * plaintext)
{

	pthread_mutex_lock(&(hashdb->lock));

	if (hashdb->last == NULL)
	{
		hashdb->last = (hashdb_hash_list_t *) malloc(sizeof(hashdb_hash_list_t));
		hashdb->first = hashdb->last;
	}
	else
	{
		hashdb->last->next = (hashdb_hash_list_t *) malloc(sizeof(hashdb_hash_list_t));
		hashdb->last = hashdb->last->next;
	}
	
	hashdb->last->context = (void *) malloc(hashdb->context_size);
	memcpy(hashdb->last->context, context, hashdb->context_size);
	
	hashdb->last->plaintext = (char *) malloc(strlen(plaintext) + 1);
	strcpy(hashdb->last->plaintext, plaintext);
	
	hashdb->last->salt = NULL;
	
	hashdb->last->next = NULL;
	
	pthread_mutex_unlock(&(hashdb->lock));

}

				


/*
* Pass both the context for the hash, and the plaintext to the hash. This adds
* the hash to the list of found hashes
*/
void hashdb_add_salted_plaintext (hashdb_t * hashdb, void * context, char * plaintext, char * salt)
{

	pthread_mutex_lock(&(hashdb->lock));

	if (hashdb->last == NULL)
	{
		hashdb->last = (hashdb_hash_list_t *) malloc(sizeof(hashdb_hash_list_t));
		hashdb->first = hashdb->last;
	}
	else
	{
		hashdb->last->next = (hashdb_hash_list_t *) malloc(sizeof(hashdb_hash_list_t));
		hashdb->last = hashdb->last->next;
	}
	
	hashdb->last->context = (void *) malloc(hashdb->context_size);
	memcpy(hashdb->last->context, context, hashdb->context_size);
	
	hashdb->last->plaintext = (char *) malloc(strlen(plaintext) + 1);
	strcpy(hashdb->last->plaintext, plaintext);
	
	hashdb->last->salt = (char *) malloc(strlen(salt) + 1);
	strcpy(hashdb->last->salt, salt);
	
	hashdb->last->next = NULL;
	
	pthread_mutex_unlock(&(hashdb->lock));

}



/*
* Places the hash iterator to the beginning, ready to iterate through the list
* of found hashes
*/
void hashdb_iterator_reset (hashdb_t * hashdb)
{

	hashdb->iterator = hashdb->first;

}



/*
* Iterates the hashdb iterator to the next hash in the hash list. The plaintext
* and the context for this hash will be pointed to by hashdb->plaintext and
* hashdb->context (you will need to cast hashdb->context to the appropriate
* hash context).
*/
int hashdb_iterate (hashdb_t * hashdb)
{

	if (hashdb->iterator == NULL)
	{
		hashdb->plaintext = NULL;
		hashdb->context   = NULL;
		hashdb->salt      = NULL;
		return 0;
	}
	else
	{
		hashdb->plaintext = hashdb->iterator->plaintext;
		hashdb->context   = hashdb->iterator->context;
		hashdb->salt      = hashdb->iterator->salt;
	}
	
	hashdb->iterator = hashdb->iterator->next;
	return 1;

}



void hashdb_destroy_tree (hashdb_hash_tree_t * tree)
{

	if (tree == NULL)
		return;

	hashdb_destroy_tree(tree->right);
	hashdb_destroy_tree(tree->left);
	
	free(tree->context);
	
	free(tree);
	
}



/*
* Destroys/frees all memory
*/
void hashdb_destroy(hashdb_t * hashdb)
{

	int i;

	hashdb_hash_list_t * current, * next;
	
	current = hashdb->first;
	
	while (current != NULL)
	{
		next = current->next;
		free(current->plaintext);
		free(current->context);
		free(current);
		current = next;
	}
	
	pthread_mutex_destroy(&(hashdb->lock));
	
	for (i = 0; i < HASHDB_TREES_SIZE; i++)
		hashdb_destroy_tree(hashdb->trees[i]);
	
	free(hashdb->bloom_filter);
	
	free(hashdb);

}
