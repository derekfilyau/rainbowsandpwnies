#include "fuzzer.h"



const char FUZZER_ASCII_CHARS[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";



int fuzzer_case_insensitive_strcmp(char * string_one, char * string_two)
{

	int i;
	char c_one, c_two;
	
	for (i = 0; i < strlen(string_one); i++)
	{
		if (i >= strlen(string_two))
			return -1;
			
		c_one = string_one[i];
		c_two = string_two[i];
		
		if ((c_one >= 'a') && (c_one <= 'z'))
			c_one -= 'a' - 'A';
		if ((c_two >= 'a') && (c_two <= 'z'))
			c_two -= 'a' - 'A';
		
		if (c_one > c_two)
			return 1;
		else if (c_two > c_one)
			return -1;
	}
	
	if (i < strlen(string_two))
		return 1;

	return 0;
	
}



// won't catch for greater than 2^32/2-1, but give me a break
int fuzzer_is_valid_atoi (char * atoi_string)
{

	int i;
	
	if (strlen(atoi_string) > 10)
		return FUZZER_ENGINE_ERROR_NOT_VALID_ATOI;
		
	for (i = 0; i < strlen(atoi_string); i++)
	{
		if ((atoi_string[i] < '0') || (atoi_string[i] > '9'))
			return FUZZER_ENGINE_ERROR_NOT_VALID_ATOI;
	}
	
	return 0;
	
}



fuzzer_engine_t * fuzzer_engine_create ()
{

	fuzzer_engine_t * engine;
	
	engine = malloc(sizeof(fuzzer_engine_t));
	if (engine == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_MALLOC_FAIL;
		return NULL;
	}
	
	engine->input_len = 0;
	engine->output_len = 0;
	engine->input = NULL;
	engine->output = NULL;
	engine->tokens = NULL;
	
	// seed random number generator
	srand((unsigned int) time(NULL));
	
	return engine;
	
}



int fuzzer_engine_load (fuzzer_engine_t * engine, const char * input_string)
{

	int token_start, token_end; // used to locate a token;
	int strlen_input_string;
	fuzzer_token_parser_t * parser;
	fuzzer_token_t * token;

	if (engine->input != NULL)
		free(engine->input);
	
	strlen_input_string = strlen(input_string);
	
	engine->input = (char *) malloc(strlen_input_string + 1);
	if (engine->input == NULL)
		return FUZZER_ENGINE_ERROR_MALLOC_FAIL;
	
	strncpy(engine->input, input_string, strlen_input_string);
	engine->input_len = strlen_input_string;
	engine->output_len = 0;
	engine->output = NULL;
	engine->tokens = NULL;
	
	
	token_start = 0;
	while (token_start < engine->input_len)
	{
		// we found the beginning of a token
		if (engine->input[token_start] == FUZZER_TOKEN_MARKER)
		{
			#if FUZZER_DEBUG == 1
				printf("token_start found at %d\n", token_start);
			#endif
			// look for the end of the token
			for (token_end = token_start + 1; token_end < engine->input_len; token_end++)
			{
				// we found the end of the token
				if (engine->input[token_end] == FUZZER_TOKEN_MARKER)
				{
					#if FUZZER_DEBUG == 1
						printf("token_end found at %d\n", token_end);
					#endif
					// send text to token_parser
					parser = fuzzer_token_parser_create(engine->input, token_start + 1, token_end - 1);
					if (parser == NULL)
						return fuzzer_errno;
					// pass parser to fuzzer_token_create to create a token
					token = fuzzer_token_create(parser, token_start);
					if (token == NULL)
						return fuzzer_errno;
					// append token to engine->tokens
					if (engine->tokens == NULL)
						engine->tokens = token;
					else
					{
						if (fuzzer_token_append(engine->tokens, token))
							return fuzzer_errno;
					}
					fuzzer_token_parser_destroy(parser);
					break;
				} // found end of the token
			} // look for end of token
			// we now copy the rest of the string right of this token over this token
			// and decrease the length of input
			memcpy(&(engine->input[token_start]),
			       &(engine->input[token_end + 1]),
			       engine->input_len - token_end + 1);
			       
			#if FUZZER_DEBUG == 1
				printf("adjusting engine->input_len token_end %d, token_start %d from %d\n", token_end, token_start, engine->input_len);
			#endif
			engine->input_len -= (token_end + 1) - token_start;
			// set the rest of engine->input to 0
			memset(&(engine->input[engine->input_len]), 0, strlen_input_string - engine->input_len);
		} // found beginning of token
		else
			token_start++;
	}
	
	return 0;
	
}



unsigned char * fuzzer_engine_execute (fuzzer_engine_t * engine, int * length)
{

	int error;
	int input_location; // our location in engine->input
	fuzzer_token_t * token;
	
	if (engine->output != NULL)
		free(engine->output);
	engine->output = NULL;
	engine->output_len = 0;
	
	token = engine->tokens;
	
	input_location = 0;
	while (token != NULL)
	{
		#if FUZZER_DEBUG == 1
			printf("fuzzer_engine_execute appending input: token->location %d, input_location %d\n", token->location, input_location);
		#endif
		// append engine->input before this token
		if (fuzzer_engine_append_to_output(engine,
		                               (unsigned char *) &(engine->input[input_location]),
		                               token->location - input_location))
			return NULL;
		input_location = token->location;
		
		error = fuzzer_token_execute(token, engine);
		if (error)
		{
			fuzzer_errno = error;
			return NULL;
		}
		
		#if FUZZER_DEBUG == 1
			printf("fuzzer_engine_execute token done\n");
		#endif
		
		token = token->next;
		
	}
	
	#if FUZZER_DEBUG == 1
		printf("fuzzer_engine_execute appending input: engine->input_len %d, input_location %d\n", engine->input_len, input_location);
	#endif
	
	// append any part of the string left
	fuzzer_engine_append_to_output(engine,
	                               (unsigned char *) &(engine->input[input_location]),
	                               engine->input_len - input_location);
	
	*length = engine->output_len;
	
	// returning a null engine->output is our indicator of error, so make sure that
	// doesn't happen
	if (engine->output == NULL)
		engine->output = (unsigned char *) malloc(0);
	
	return engine->output;

}



int fuzzer_engine_append_to_output (fuzzer_engine_t * engine, unsigned char * appendee, int appendee_length)
{
	
	unsigned char * new_output;
	
	if (appendee_length == 0)
		return 0;
	
	#if FUZZER_DEBUG == 1
		printf("fuzzer_engine_append_to_output reallocing %p from %d to %d\n", engine->output, engine->output_len, engine->output_len + appendee_length);
	#endif
	
	new_output = realloc(engine->output, engine->output_len + appendee_length);
	if (new_output == NULL)
		return FUZZER_ENGINE_ERROR_APPEND_REALLOC_FAIL;
	
	engine->output = new_output;
	
	memcpy(&(engine->output[engine->output_len]), appendee, appendee_length);
	engine->output_len += appendee_length;
	
	return 0;
	
}



void fuzzer_engine_destroy (fuzzer_engine_t * engine)
{

	if (engine->input != NULL)
		free(engine->input);
	if (engine->output != NULL)
		free(engine->output);
	if (engine->tokens != NULL)
		fuzzer_token_destroy(engine->tokens);
		
	free(engine);
	
}



fuzzer_token_parser_t * fuzzer_token_parser_create (char * input, int token_start, int token_end)
{

	int i;
	int element_start, element_terminator, element_assigner;
	char key [FUZZER_ELEMENT_MAX_SIZE];
	char value [FUZZER_ELEMENT_MAX_SIZE];
	fuzzer_token_parser_t * parser;
	fuzzer_token_parser_element_t * element;
	
	parser = (fuzzer_token_parser_t *) malloc(sizeof(fuzzer_token_parser_t));
	if (parser == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_MALLOC_ERROR;
		return NULL;
	}
	parser->elements_n = 0;
	parser->elements = NULL;
	
	element_start = token_start;
	while (element_start < token_end)
	{
		element_assigner = -1;
		element_terminator = -1;
		// locate assigner and terminator
		for (i = element_start; i <= token_end; i++)
		{
			if (input[i] == FUZZER_ELEMENT_ASSIGNER)
				element_assigner = i;
			else if (input[i] == FUZZER_ELEMENT_TERMINATOR)
			{
				element_terminator = i;
				break;
			}
		}
		#if FUZZER_DEBUG == 1
			printf("parser_element start: %02d, assigner: %02d, terminator: %02d\n", element_start, element_assigner, element_terminator);
		#endif
		// make sure terminator and assigner exist
		if (element_terminator == -1)
		{
			fuzzer_errno = FUZZER_ENGINE_ERROR_ELEMENT_MISSING_TERMINATOR;
			return NULL;
		}
		else if (element_assigner == -1)
		{
			fuzzer_errno = FUZZER_ENGINE_ERROR_ELEMENT_MISSING_ASSIGNER;
			return NULL;
		}
		// make sure element is within proper size
		if (element_terminator - element_start > FUZZER_ELEMENT_MAX_SIZE)
		{
			fuzzer_errno = FUZZER_ENGINE_ERROR_ELEMENT_TOO_LARGE;
			return NULL;
		}
		// copy over key and value
		memset(key, 0, FUZZER_ELEMENT_MAX_SIZE);
		memset(value, 0, FUZZER_ELEMENT_MAX_SIZE);
		strncpy(key, &(input[element_start]), element_assigner - element_start);
		strncpy(value, &(input[element_assigner + 1]), element_terminator - element_assigner - 1);
		
		// create a new element and add it to element_first
		if (parser->elements == NULL)
		{
			parser->elements = fuzzer_token_parser_element_create(key, value);
			if (parser->elements == NULL)
				return NULL;
		}
		else
		{
			element = fuzzer_token_parser_element_create(key, value);
			if (element == NULL)
				return NULL;
			if (fuzzer_token_parser_element_append(parser->elements, element))
				return NULL;
		}
		
		parser->elements_n++;
		
		// set element_start to one past element_terminator to begin searching
		// for next element
		element_start = element_terminator + 1;
		
	}
	
	return parser;
	
}



void fuzzer_token_parser_destroy (fuzzer_token_parser_t * parser)
{

	if (parser->elements != NULL)
		fuzzer_token_parser_element_destroy(parser->elements);
	
	free(parser);

}



fuzzer_token_parser_element_t * fuzzer_token_parser_get_element_by_key (fuzzer_token_parser_t * parser, char * key)
{

	fuzzer_token_parser_element_t * element;
	
	element = parser->elements;
	
	while (element != NULL)
	{
		#if FUZZER_DEBUG == 1
			printf("fuzzer_parser_element_by_key haystack: \"%s\" needle: \"%s\"\n", element->key, key);
		#endif
		if (fuzzer_case_insensitive_strcmp(element->key, key) == 0)
			return element;
		element = element->next;
	}
	
	return NULL;
	
}



fuzzer_token_parser_element_t * fuzzer_token_parser_element_create (char * key, char * value)
{

	fuzzer_token_parser_element_t * element;
	
	element = (fuzzer_token_parser_element_t *) malloc(sizeof(fuzzer_token_parser_element_t));
	if (element == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_ELEMENT_MALLOC_ERROR;
		return NULL;
	}
	
	element->key = (char *) malloc(strlen(key) + 1);
	element->value = (char *) malloc(strlen(value) + 1);
	if ((element->key == NULL) || (element->value == NULL))
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_ELEMENT_MALLOC_ERROR;
		return NULL;
	}
	
	memset(element->key, 0, strlen(key) + 1);
	memset(element->value, 0, strlen(value) + 1);
	strncpy(element->key, key, strlen(key));
	strncpy(element->value, value, strlen(value));
	//element->key[strlen(key) - 1] = 0;
	//element->value[strlen(value) - 1] = 0;
	
	element->next = NULL;
	
	return element;
	
}



int fuzzer_token_parser_element_append (fuzzer_token_parser_element_t * first_element,
                                        fuzzer_token_parser_element_t * new_element)
{

	fuzzer_token_parser_element_t * next;
	
	if (first_element == NULL)
		return FUZZER_ENGINE_ERROR_ELEMENT_APPEND;
	
	next = first_element;
	
	while (next->next != NULL)
		next = next->next;
	
	next->next = new_element;
	
	return 0;
	
}



void fuzzer_token_parser_element_destroy (fuzzer_token_parser_element_t * element)
{

	if (element->next != NULL)
		fuzzer_token_parser_element_destroy(element->next);
	
	free(element->key);
	free(element->value);
	free(element);
	
}



fuzzer_token_t * fuzzer_token_create (fuzzer_token_parser_t * parser, int location)
{
	
	fuzzer_token_t * token;
	fuzzer_token_parser_element_t * element;
	
	token = (fuzzer_token_t *) malloc(sizeof(fuzzer_token_t));
	if (token == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_MALLOC_FAIL;
		return NULL;
	}
	token->next = NULL;
	token->location = location;
	
	element = fuzzer_token_parser_get_element_by_key(parser, "type");
	if (element == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_TYPE_NOT_FOUND;
		return NULL;
	}
	
	if (fuzzer_case_insensitive_strcmp(element->value, FUZZER_TOKEN_DESCRIPTION_ASCII) == 0)
	{
		token->type = FUZZER_TOKEN_TYPE_ASCII;
		token->token = (void *) fuzzer_token_ascii_create(parser);
	}
	else if (fuzzer_case_insensitive_strcmp(element->value, FUZZER_TOKEN_DESCRIPTION_BYTE) == 0)
	{
		token->type = FUZZER_TOKEN_TYPE_BYTE;
		token->token = (void *) fuzzer_token_byte_create(parser);
	}
	else if (fuzzer_case_insensitive_strcmp(element->value, FUZZER_TOKEN_DESCRIPTION_FILE) == 0)
	{
		token->type = FUZZER_TOKEN_TYPE_FILE;
		token->token = (void *) fuzzer_token_file_create(parser);
	}
	
	return token;

}


int fuzzer_token_append (fuzzer_token_t * first, fuzzer_token_t * new_token)
{

	fuzzer_token_t * next;
	
	if (first == NULL)
		return FUZZER_ENGINE_ERROR_FUZZER_TOKEN_APPEND;
	
	next = first;
	
	while (next->next != NULL)
	{
		next = next->next;
	}
	
	next->next = new_token;
	
	return 0;
	
}



void fuzzer_token_destroy (fuzzer_token_t * token)
{
	
	if (token == NULL)
		return;

	if (token->next != NULL)
		fuzzer_token_destroy(token->next);
	
	if (token->type == FUZZER_TOKEN_TYPE_ASCII)
		fuzzer_token_ascii_destroy((fuzzer_token_ascii_t *) token->token);
	else if (token->type == FUZZER_TOKEN_TYPE_BYTE)
		fuzzer_token_byte_destroy((fuzzer_token_byte_t *) token->token);
	else if (token->type == FUZZER_TOKEN_TYPE_FILE)
		fuzzer_token_file_destroy((fuzzer_token_file_t *) token->token);
	
	free(token);
	
}



int fuzzer_token_execute (fuzzer_token_t * token, fuzzer_engine_t * engine)
{
	
	int error;
	int length;
	unsigned char * result;
	
	if (token->type == FUZZER_TOKEN_TYPE_ASCII)
		error = fuzzer_token_ascii_execute((fuzzer_token_ascii_t *) token->token, &result, &length);
	else if (token->type == FUZZER_TOKEN_TYPE_BYTE)
		error = fuzzer_token_byte_execute ((fuzzer_token_byte_t *)  token->token, &result, &length);
	else if (token->type == FUZZER_TOKEN_TYPE_FILE)
		error = fuzzer_token_file_execute ((fuzzer_token_file_t *)  token->token, &result, &length);
	else
		return FUZZER_ENGINE_ERROR_TOKEN_TYPE_NOT_FOUND;
		
	if (error)
		return error;
	
	error = fuzzer_engine_append_to_output (engine, result, length);
	if (error)
		return error;
	
	free(result);
		
	return 0;

}



fuzzer_token_ascii_t * fuzzer_token_ascii_create (fuzzer_token_parser_t * parser)
{
	
	fuzzer_token_ascii_t * ascii;
	fuzzer_token_parser_element_t * min_length;
	fuzzer_token_parser_element_t * max_length;
	
	ascii = (fuzzer_token_ascii_t *) malloc(sizeof(fuzzer_token_ascii_t));
	if (ascii == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_ASCII_MALLOC_FAIL;
		return NULL;
	}
	
	min_length = fuzzer_token_parser_get_element_by_key(parser, "min_length");
	max_length = fuzzer_token_parser_get_element_by_key(parser, "max_length");
	
	if (min_length == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_ASCII_MIN_LENGTH_NOT_FOUND;
		return NULL;
	}
	if (max_length == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_ASCII_MAX_LENGTH_NOT_FOUND;
		return NULL;
	}
	
	if (fuzzer_is_valid_atoi(min_length->value))
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_ASCII_MIN_LENGTH_ATOI;
		return NULL;
	}
	if (fuzzer_is_valid_atoi(max_length->value))
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_ASCII_MAX_LENGTH_ATOI;
		return NULL;
	}
	
	ascii->min_length = atoi(min_length->value);
	ascii->max_length = atoi(max_length->value);
	
	return ascii;

}



void fuzzer_token_ascii_destroy (fuzzer_token_ascii_t * ascii)
{

	if (ascii == NULL)
		return;

	free (ascii);
	
}



int fuzzer_token_ascii_execute (fuzzer_token_ascii_t * ascii, unsigned char ** result, int * length)
{
	
	int i;
	int strlen_fuzzer_ascii_chars;
	
	if (ascii->max_length - ascii->min_length <= 0)
	{
		if (ascii->min_length <= 0)
			*length = 0;
		else
			*length = ascii->min_length;
	}
	else
	{
		*length = (int) rand() % (ascii->max_length - ascii->min_length);
		*length += ascii->min_length;
	}
	
	*result = (unsigned char *) malloc(*length);
	if (result == NULL)
		return FUZZER_ENGINE_ERROR_TOKEN_ASCII_EXECUTE_MALLOC_FAIL;
	
	strlen_fuzzer_ascii_chars = strlen(FUZZER_ASCII_CHARS);
	// cache hit from two ints so far apart?
	for(i = 0; i < *length; i++)
	{
		(*result)[i] = FUZZER_ASCII_CHARS[rand() % strlen_fuzzer_ascii_chars];
	}
	
	return 0;
	
}



fuzzer_token_byte_t * fuzzer_token_byte_create (fuzzer_token_parser_t * parser)
{
	
	fuzzer_token_byte_t * byte;
	fuzzer_token_parser_element_t * min_length;
	fuzzer_token_parser_element_t * max_length;
	
	byte = (fuzzer_token_byte_t *) malloc(sizeof(fuzzer_token_byte_t));
	if (byte == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_BYTE_MALLOC_FAIL;
		return NULL;
	}
	
	min_length = fuzzer_token_parser_get_element_by_key(parser, "min_length");
	max_length = fuzzer_token_parser_get_element_by_key(parser, "max_length");
	
	if (min_length == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_BYTE_MIN_LENGTH_NOT_FOUND;
		return NULL;
	}
	if (max_length == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_BYTE_MAX_LENGTH_NOT_FOUND;
		return NULL;
	}
	
	if (fuzzer_is_valid_atoi(min_length->value))
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_BYTE_MIN_LENGTH_ATOI;
		return NULL;
	}
	if (fuzzer_is_valid_atoi(max_length->value))
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_BYTE_MAX_LENGTH_ATOI;
		return NULL;
	}
	
	byte->min_length = atoi(min_length->value);
	byte->max_length = atoi(max_length->value);
	
	return byte;

}



void fuzzer_token_byte_destroy (fuzzer_token_byte_t * byte)
{

	if (byte == NULL)
		return;

	free (byte);
	
}



int fuzzer_token_byte_execute (fuzzer_token_byte_t * byte, unsigned char ** result, int * length)
{
	
	int i;

	if (byte->max_length - byte->min_length <= 0)
	{
		if (byte->min_length <= 0)
			*length = 0;
		else
			*length = byte->min_length;
	}
	else
	{
		*length = (int) rand() % (byte->max_length - byte->min_length);
		*length += byte->min_length;
	}
	
	*result = (unsigned char *) malloc(*length);
	if (result == NULL)
		return FUZZER_ENGINE_ERROR_BYTE_EXECUTE_MALLOC_FAIL;
	
	// cache hit from two ints so far apart?
	for(i = 0; i < *length; i++)
		(*result)[i] = (unsigned char) rand();
	
	return 0;
	
}



fuzzer_token_file_t * fuzzer_token_file_create  (fuzzer_token_parser_t * parser)
{

	fuzzer_token_file_t * file;
	fuzzer_token_parser_element_t * element;
	
	file = (fuzzer_token_file_t *) malloc(sizeof(fuzzer_token_file_t));
	if (file == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_TOKEN_FILE_MALLOC_FAIL;
		return NULL;
	}
	file->bytes_in_buffer = 0;
	
	element = fuzzer_token_parser_get_element_by_key(parser, "filename");
	if (element == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_FILE_FILENAME_NOT_FOUND;
		return NULL;
	}
	
	file->fh = fopen(element->value, "r");
	if (file->fh == NULL)
	{
		fuzzer_errno = FUZZER_ENGINE_ERROR_FILE_COULD_NOT_OPEN_FILE;
		return NULL;
	}
	
	return file;

}



void fuzzer_token_file_destroy (fuzzer_token_file_t * file)
{
	
	if (file->fh != NULL)
		fclose(file->fh);
	free(file);
	
}



int fuzzer_token_file_execute (fuzzer_token_file_t * file, unsigned char ** result, int * length)
{
	
	int bytes_read;
	int i;
	char buffer_tmp[FUZZER_TOKEN_FILE_BUFFER_SIZE];
	
	// if there are no bytes in the buffer, make sure we are at the beginning
	// of the file before we start reading
	if (file->bytes_in_buffer == 0)
		fseek(file->fh, 0, SEEK_SET);
	
	// load up bytes into our file if need be
	if (file->bytes_in_buffer < FUZZER_TOKEN_FILE_BUFFER_SIZE)
	{
		bytes_read = fread(&(file->buffer[file->bytes_in_buffer]),
		                   1,
		                   FUZZER_TOKEN_FILE_BUFFER_SIZE - file->bytes_in_buffer,
		                   file->fh);
		if (ferror(file->fh) && feof(file->fh) != 0)
			return FUZZER_ENGINE_ERROR_FILE_READ_ERROR;
		file->bytes_in_buffer += bytes_read;
	}
	
	// find the next item in the file
	for (i = 0; i < file->bytes_in_buffer; i++)
	{
		if (file->buffer[i] == FUZZER_TOKEN_FILE_SEPERATOR)
			break;
	}
	
	*result = (unsigned char *) malloc(i);
	memcpy(*result, file->buffer, i);
	*length = i;
	
	#if FUZZER_DEBUG == 1
		printf("fuzzer_token_file_execute found seperator at %d\n", i);
	#endif
	
	// if memcpy copies over objects that overlap, behavior = undefined
	memcpy(buffer_tmp, &(file->buffer[i+1]), file->bytes_in_buffer - i + 1);
	memcpy(file->buffer, buffer_tmp, file->bytes_in_buffer - i + 1);
	file->bytes_in_buffer -= i + 1;
	
	return 0;

}
			

