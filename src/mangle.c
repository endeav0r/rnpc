#include "mangle.h"

#define LOWER_TO_UPPER_DIFFERENCE 32

mangle_data_t * mangle_data_create ()
{

	mangle_data_t * data;
	
	data = (mangle_data_t *) malloc(sizeof(mangle_data_t));
	
	data->num_rules = 0;
	data->current_rule = 0;
	
	data->options = 0;
	
	pthread_mutex_init(&(data->lock), NULL);
	
	return data;

}



void mangle_data_destroy (mangle_data_t * data)
{

	pthread_mutex_init(&(data->lock), NULL);

	free(data);

}



inline void mangle_options_set (mangle_data_t * data, unsigned int options)
{

	data->options = options;

}



int mangle_load_from_string (mangle_data_t * data, char * string)
{

	int i;
	int rule;
	int terminate = 0;
	char * beginning_of_rule;
	
	// to make this threadsafe, we have to copy string over to tmp
	char tmp[32];
	
	data->num_rules = 0;
	
	strncpy(tmp, string, 32);
	
	i = 0;
	beginning_of_rule = string;
	while (1)
	{
		if ((tmp[i] == ',') || (tmp[i] == 0))
		{
			if (tmp[i] == 0)
				terminate = 1;
			tmp[i] = 0;
			rule = atoi(beginning_of_rule);
			
			switch (rule)
			{
				case 0 :
					data->total_functions[data->num_rules] = total_mangles0;
					data->mangle_functions[data->num_rules] = mangle0;
					break;
				case 1 :
					data->total_functions[data->num_rules] = total_mangles1;
					data->mangle_functions[data->num_rules] = mangle1;
					break;
				case 2 :
					data->total_functions[data->num_rules] = total_mangles2;
					data->mangle_functions[data->num_rules] = mangle2;
					break;
				case 3 :
					data->total_functions[data->num_rules] = total_mangles3;
					data->mangle_functions[data->num_rules] = mangle3;
					break;
				case 4 :
					data->total_functions[data->num_rules] = total_mangles4;
					data->mangle_functions[data->num_rules] = mangle4;
					break;
				case 5 :
					data->total_functions[data->num_rules] = total_mangles5;
					data->mangle_functions[data->num_rules] = mangle5;
					break;
				case 6 :
					data->total_functions[data->num_rules] = total_mangles6;
					data->mangle_functions[data->num_rules] = mangle6;
					break;
				case 7 :
					data->total_functions[data->num_rules] = total_mangles7;
					data->mangle_functions[data->num_rules] = mangle7;
					break;
				case 8 :
					data->total_functions[data->num_rules] = total_mangles8;
					data->mangle_functions[data->num_rules] = mangle8;
					break;
				case 9 :
					data->total_functions[data->num_rules] = total_mangles9;
					data->mangle_functions[data->num_rules] = mangle9;
					break;
				default :
					return -1;
			}
			data->num_rules++;
			if (terminate)
				break;
			beginning_of_rule = &(tmp[i + 1]);
		}
		i++;
	}
	
	return 0;

}


// also resets the mangle values
inline void mangle_set_plaintext (mangle_data_t * data, char * plaintext)
{

	data->plaintext = plaintext;
	data->current_rule = 0;
	memset(data->rule_seeds, 0, MANGLE_MAX_RULES * sizeof(int));

}
					
					
int mangle (mangle_data_t * data, char * mangle_word)
{

	pthread_mutex_lock(&(data->lock));
	
	while (data->current_rule >= 0)
	{
		// if the first rule has done all it can, it's time for us to be done
		if ((data->current_rule == 0) 
		    && (data->rule_seeds[0] == data->total_functions[0](data->plaintext)))
				break;
		// if this rule has already done all it can, set its rule_seed to 0
		// and go back to the previous rule
		else if ((data->current_rule > 0) && (data->rule_seeds[data->current_rule]
		       == data->total_functions[data->current_rule](data->MANGLE_WORDS[data->current_rule - 1])))
		{
			data->rule_seeds[data->current_rule] = 0;
			data->current_rule--;
		}
		else
		{
			// first rule is a special case and mangles off the plaintext
			if (data->current_rule == 0)
				data->mangle_functions[0](data->MANGLE_WORDS[0], data->plaintext, data->rule_seeds[0]);
			else
			// other rules mangle off the previous rule
				data->mangle_functions[data->current_rule]
				       (data->MANGLE_WORDS[data->current_rule], 
				        data->MANGLE_WORDS[data->current_rule - 1],
				        data->rule_seeds[data->current_rule]);
			// advance rule_i for this rule
			data->rule_seeds[data->current_rule]++;
			// if this is the last rule, copy the word over and return it
			if (data->current_rule == data->num_rules - 1)
			{
				strcpy(mangle_word, data->MANGLE_WORDS[data->current_rule]);
				pthread_mutex_unlock(&(data->lock));
				return 1;
			}
			// otherwise, advance the rule
			else
				data->current_rule++;
		}
	}
	
	pthread_mutex_unlock(&data->lock);
	return 0;

}



int total_mangles0 (char * word)
{

	int i;
	int total = 1;
	
	for (i = 0; i < strlen(word); i++)
	{
		if ((word[i] >= 'a') && (word[i] <= 'z'))
			total *= 2;
	}
	
	return total;

}



char * mangle0 (char * MANGLE_WORD, char * plaintext, int seed)
{

	int i;
	int plaintext_length;
	
	plaintext_length = strlen(plaintext);
	
	// mangle word in the middle
	for (i = 0; i < plaintext_length; i++)
	{
		if ((plaintext[i] >= 'a') && (plaintext[i] <= 'z'))
		{
			if (seed & 1)
				MANGLE_WORD[i] = plaintext[i] - LOWER_TO_UPPER_DIFFERENCE;
			seed >>= 1;
		}
		else
			MANGLE_WORD[i] = plaintext[i];
	}
	
	MANGLE_WORD[i] = 0;
	
	return MANGLE_WORD;

}

		

// returns the total number of mangle possibilities for a word
int total_mangles1 (char * word)
{

	int i;
	int strlen_word;
	int possibilities = 1;
	
	strlen_word = strlen(word);
	
	for (i = 0; i < strlen_word; i++)
	{
		switch (word[i])
		{
			case 'a' :
				possibilities *= 3;
				break;
			case 'b' :
				possibilities *= 2;
				break;
			case 'e' :
				possibilities *= 2;
				break;
			case 'g' :
				possibilities *= 2;
				break;
			case 'i' :
				possibilities *= 3;
				break;
			case 'l' :
				possibilities *= 3;
				break;
			case 'o' :
				possibilities *= 2;
				break;
			case 's' :
				possibilities *= 3;
				break;
			case 't' :
				possibilities *= 2;
				break;
		}
	}
	
	return possibilities;
	
}



/*
	a = 4
	b = 3
	e = 3
	g = 3
	i = 4
	l = 4
	o = 3
	s = 4
	t = 3
*/
inline void mangle_char (char * c, int * seed)
{
	switch (*c)
	{
		case 'a' :
			switch (*seed % 3)
			{
				case 1 :
					*c = '4';
					break;
				case 2 :
					*c = '@';
					break;
			}
			*seed /= 3;
			break;
		case 'b' :
			switch (*seed & 1)
			{
				case 1 :
					*c = '8';
					break;
			}
			*seed >>= 1;
			break;
		case 'e' :
			switch (*seed & 1)
			{
				case 1 :
					*c = '3';
					break;
			}
			*seed >>= 1;
			break;
		case 'g' :
			switch (*seed & 1)
			{
				case 1 :
					*c = '9';
					break;
			}
			*seed >>= 1;
			break;
		case 'i' :
			switch (*seed % 3)
			{
				case 1 :
					*c = '1';
					break;
				case 2 :
					*c = '!';
					break;
			}
			*seed /= 3;
			break;
		case 'l' :
			switch (*seed % 3)
			{
				case 1 :
					*c = '1';
					break;
				case 2 :
					*c = '!';
					break;
			}
			*seed /= 3;
			break;
		case 'o' :
			switch (*seed & 1)
			{
				case 1 :
					*c = '0';
					break;
			}
			*seed >>= 1;
			break;
		case 's' :
			switch (*seed % 3)
			{
				case 1 :
					*c = '5';
					break;
				case 2 :
					*c = '$';
					break;
			}
			*seed /= 3;
			break;
		case 't' :
			switch (*seed & 1)
			{
				case 1 :
					*c = '7';
					break;
			}
			*seed >>= 1;
			break;
	}
}			



char * mangle1 (char * MANGLE_WORD, char * plaintext, int seed)
{

	int i;
	int plaintext_length;
	
	plaintext_length = strlen(plaintext);
	
	// mangle word in the middle
	for (i = 0; i < plaintext_length; i++)
	{
		MANGLE_WORD[i] = plaintext[i];
		mangle_char(&(MANGLE_WORD[i]), &seed);
	}
	
	MANGLE_WORD[i] = 0;
	
	return MANGLE_WORD;

}


// append single digit
inline int total_mangles2 (char * plaintext)
{
	return 10;
}



char * mangle2 (char * MANGLE_WORD, char * plaintext, int seed)
{

	int plaintext_length;
	
	plaintext_length = strlen(plaintext);
	
	if (! seed)
		strcpy(MANGLE_WORD, plaintext);
	
	MANGLE_WORD[plaintext_length] = '0' + (seed % 10);
	MANGLE_WORD[plaintext_length+1] = 0;
	
	return MANGLE_WORD;

}


// append special_M
int total_mangles3 (char * plaintext)
{
	return MANGLE_SPECIAL_CHARS_LENGTH;
}



char * mangle3 (char * MANGLE_WORD, char * plaintext, int seed)
{

	int plaintext_length;
	
	plaintext_length = strlen(plaintext);
	
	if (! seed)
		strcpy(MANGLE_WORD, plaintext);
	
	MANGLE_WORD[plaintext_length] = MANGLE_SPECIAL_CHARS[seed % MANGLE_SPECIAL_CHARS_LENGTH];
	MANGLE_WORD[plaintext_length+1] = 0;
	
	return MANGLE_WORD;

}



int total_mangles4 (char * plaintext)
{
	if ((plaintext[0] >= 'a') && (plaintext[0] <= 'z'))
		return 1;
	else
		return 0;
}



char * mangle4 (char * MANGLE_WORD, char * plaintext, int seed)
{

	if (! seed)
		strcpy(MANGLE_WORD, plaintext);
	if ((MANGLE_WORD[0] >= 'a') && (MANGLE_WORD[0] <= 'z'))
		MANGLE_WORD[0] -= LOWER_TO_UPPER_DIFFERENCE;
	return MANGLE_WORD;

}



// append two digits
int total_mangles5 (char * plaintext)
{
	return 100;
}



char * mangle5 (char * MANGLE_WORD, char * plaintext, int seed)
{

	int plaintext_length;
	
	plaintext_length = strlen(plaintext);
	
	if (! seed)
		strcpy(MANGLE_WORD, plaintext);
	
	MANGLE_WORD[plaintext_length++] = '0' + (char) (seed % 10);
	seed /= 10;
	MANGLE_WORD[plaintext_length++] = '0' + (char) (seed % 10);
	seed /= 10;
	MANGLE_WORD[plaintext_length] = 0;
	
	return MANGLE_WORD;

}



// append year between 1921 and 2020
int total_mangles6 (char * plaintext)
{
	return 100;
}



char * mangle6 (char * MANGLE_WORD, char * plaintext, int seed)
{

	int plaintext_length;
	
	plaintext_length = strlen(plaintext);
	
	if (! seed)
		strcpy(MANGLE_WORD, plaintext);
	
	if (seed % 100 <= 20)
	{
		MANGLE_WORD[plaintext_length++] = '2';
		MANGLE_WORD[plaintext_length++] = '0';
		MANGLE_WORD[plaintext_length+1] = '0' + (char) (seed % 10);
		seed /= 10;
		MANGLE_WORD[plaintext_length] = '0' + (char) (seed % 10);
		plaintext_length += 2;
	}
	else
	{
		MANGLE_WORD[plaintext_length++] = '1';
		MANGLE_WORD[plaintext_length++] = '9';
		MANGLE_WORD[plaintext_length+1] = '0' + (char) (seed % 10);
		seed /= 10;
		MANGLE_WORD[plaintext_length] = '0' + (char) (seed % 10);
		plaintext_length += 2;
	}
	
	MANGLE_WORD[plaintext_length] = 0;
	
	return MANGLE_WORD;

}



// UPPERCASE whole word
int total_mangles7 (char * plaintext)
{
	return 1;
}



char * mangle7 (char * MANGLE_WORD, char * plaintext, int seed)
{

	int i;
	int plaintext_length;
	
	plaintext_length = strlen(plaintext);
	
	strcpy(MANGLE_WORD, plaintext);
	
	for (i = 0; i < plaintext_length; i++)
	{
		if ((MANGLE_WORD[i] >= 'a') && (MANGLE_WORD[i] <= 'z'))
			MANGLE_WORD[i] -= LOWER_TO_UPPER_DIFFERENCE;
	}
	
	MANGLE_WORD[i] = 0;
	
	return MANGLE_WORD;

}



// prepend a digit
int total_mangles8 (char * plaintext)
{
	return 10;
}



char * mangle8 (char * MANGLE_WORD, char * plaintext, int seed)
{

	int plaintext_length;
	
	plaintext_length = strlen(plaintext);

	MANGLE_WORD[0] = '0' + (char) (seed % 10);
	MANGLE_WORD[1] = 0;
	strncat(MANGLE_WORD, plaintext, MANGLE_WORD_SIZE);
	
	return MANGLE_WORD;

}



// prepend UPPERCASE character
int total_mangles9 (char * plaintext)
{
	return 26;
}



char * mangle9 (char * MANGLE_WORD, char * plaintext, int seed)
{

	int plaintext_length;
	
	plaintext_length = strlen(plaintext);

	MANGLE_WORD[0] = 'A' + (char) (seed % 26);
	MANGLE_WORD[1] = 0;
	strncat(MANGLE_WORD, plaintext, MANGLE_WORD_SIZE);
	
	return MANGLE_WORD;

}

