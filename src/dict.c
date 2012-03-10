#include "dict.h"



dictionary_file_t * dictionary_file_open (char * filename)
{

	dictionary_file_t * d;
	
	d = (dictionary_file_t *) malloc(sizeof(dictionary_file_t));

	d->buffer_cursor = 0;
	d->buffer_size = 0;
	d->eof = 0;
	d->options = 0;
	
	pthread_mutex_init(&(d->lock), NULL);

	if (strcmp(filename, "stdin") == 0)
		d->fh = stdin;
	else
		d->fh = fopen(filename, "r");
	if (d->fh == NULL)
	{
		free (d);
		return NULL;
	}
	
	return d;

}



inline void dictionary_set_options(dictionary_file_t * d, unsigned int options)
{
	d->options = options;
}


char * dictionary_next_word (dictionary_file_t * d)
{
	
	int bytes_read; // how many bytes are read when we are reading from the
	                // file into our buffer
	int i; // our location in d->buffer while we try to find the end of
	       // the current word
	int j; // used when moving around the current buffer before we load more
	       // of the file into the buffer
	char * word_to_return; // points inside d->buffer to the beginning of the
	                       // word to return, that we null-terminate before
	                       // returning

	i = d->buffer_cursor;
	while (1)
	{
		// check to see if we need to reset the buffer
		if (i >= d->buffer_size)
		{
			// if we're at end of file, then we need to INSURE the word is
			// null-terminated, and return, or return NULL if i = 0
			if (d->eof)
			{
				// if i is at the beginning, just return null
				if (i == 0)
				{
					return NULL;
				}
				// the current string is of length 0. return NULL
				else if (i == d->buffer_cursor)
				{
					return NULL;
				}
				// if we have room, set next char to 0 to insure
				// null-terminated, set->buffer_cursor, return
				else if (i < DICTIONARY_BUFFER_SIZE)
				{
					d->buffer[i] = 0;
					word_to_return = &(d->buffer[d->buffer_cursor]);
					d->buffer_cursor = i;
					fflush(stdout);
					return word_to_return;
				}
				// if we don't have room, copy buffer over to beginning,
				// null-terminate, return
				else
				{
					for (j = 0; j < d->buffer_size - d->buffer_cursor; j++)
						d->buffer[j] = d->buffer[d->buffer_cursor + j];
					i = i - d->buffer_cursor;
					d->buffer_cursor = 0;
					d->buffer_size = j;
					// if j == DICTIONARY_BUFFER_SIZE, then we're screwed
					// anyway, just return NULL
					if (j == DICTIONARY_BUFFER_SIZE)
					{
						return NULL;
					}
					d->buffer[j] = 0;
					return d->buffer;
				}
			}
			
			// we move everything in the buffer starting from d->buffer_cursor
			// to d->buffer_size back to location 0;
			for (j = 0; j < d->buffer_size - d->buffer_cursor; j++)
				d->buffer[j] = d->buffer[d->buffer_cursor + j];
			// we adjust i to its new correct location now, before we alter any
			// other d->buffer_* variables
			i = i - d->buffer_cursor;
			// now we reset d->buffer_cursor to location 0, and the size of
			// this whole thing should be j's value
			d->buffer_cursor = 0;
			d->buffer_size = j;
			// now we read in the next chunk from the file, enough to fill out
			// d->buffer
			bytes_read = fread(&(d->buffer[d->buffer_size]),
			             1,
			             DICTIONARY_BUFFER_SIZE - d->buffer_size,
			             d->fh);
			// add the bytes_read to d->buffer_size, and see if we filled up
			// the buffer
			d->buffer_size += bytes_read;
			// if we read less, set the eof marker (even if it's not the eof,
			// we ain't readin' no moar file anyways)
			if (d->buffer_size < DICTIONARY_BUFFER_SIZE)
				d->eof = 1;
			// everything should be good to go from here.				
		}
		// if we find a newline char, null-terminate and set newline_found
		if (d->buffer[i] == '\n')
		{
			d->buffer[i] = 0;
			if (d->buffer[i-1] == '\r')
				d->buffer[i-1] = 0;
			word_to_return = &(d->buffer[d->buffer_cursor]);
			d->buffer_cursor = i + 1;
			return word_to_return;
		}
		/*
		}
		// non newline char and newline_found? set word_to_return, reset
		// db->buffer_cursor, and return
		else if (newline_found)
		{
			word_to_return = &(d->buffer[d->buffer_cursor]);
			d->buffer_cursor = i;
			return word_to_return;
		}
		*/
		i++;
	}

	// this statement should never be execute
	return NULL;
	
}



int dictionary_next_word_threadsafe (dictionary_file_t * d, char * word, int word_length)
{

	char * next_word;

	pthread_mutex_lock(&(d->lock));
	
	next_word = dictionary_next_word(d);
	
	if (next_word == NULL)
	{
		pthread_mutex_unlock(&(d->lock));
		return 0;
	}
	
	strncpy(word, next_word, word_length);
	
	pthread_mutex_unlock(&(d->lock));
	
	return 1;

}



int dictionary_next_word_threadsafe_many (dictionary_file_t * d, char ** words,
                                          int word_length, int num_words)
{

	int i;
	char * next_word;
	
	pthread_mutex_lock(&(d->lock));

	for (i = 0; i < num_words; i++)
	{
		next_word = dictionary_next_word(d);
		
		if (next_word == NULL)
			break;
		else
		{
			strncpy(words + (i * word_length / 8), next_word, word_length - 1);
		}
	}
	
	pthread_mutex_unlock(&(d->lock));
	
	return i;
	
}



void dictionary_file_close (dictionary_file_t * d)
{

	fclose(d->fh);
	pthread_mutex_destroy(&(d->lock));
	free(d);
	
}
