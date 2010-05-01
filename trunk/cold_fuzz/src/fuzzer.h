#ifndef fuzzer_HEADER
#define fuzzer_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "error.h"

#define FUZZER_DEBUG 1


#define FUZZER_TOKEN_MARKER       '%'
#define FUZZER_ELEMENT_ASSIGNER   '='
#define FUZZER_ELEMENT_TERMINATOR ';'
#define FUZZER_ELEMENT_MAX_SIZE 64

#define FUZZER_TOKEN_FILE_BUFFER_SIZE 4096 // this is also the max size of a line
                                           // in bytes
#define FUZZER_TOKEN_FILE_SEPERATOR '\n' // what seperates entries in a file

#define FUZZER_TOKEN_TYPE_ASCII 0
#define FUZZER_TOKEN_TYPE_BYTE  1
#define FUZZER_TOKEN_TYPE_FILE  2

#define FUZZER_TOKEN_DESCRIPTION_ASCII "ASCII"
#define FUZZER_TOKEN_DESCRIPTION_BYTE  "BYTE"
#define FUZZER_TOKEN_DESCRIPTION_FILE  "FILE"



typedef struct fuzzer_token_parser_element_s
{
	char * key;
	char * value;
	struct fuzzer_token_parser_element_s * next;
} fuzzer_token_parser_element_t;



typedef struct
{
	int elements_n;
	fuzzer_token_parser_element_t * elements;
} fuzzer_token_parser_t;



typedef struct
{
	int min_length;
	int max_length;
} fuzzer_token_ascii_t;



typedef struct
{
	int min_length;
	int max_length;
} fuzzer_token_byte_t;



typedef struct
{
	int bytes_in_buffer;
	FILE * fh;
	char buffer[FUZZER_TOKEN_FILE_BUFFER_SIZE];
} fuzzer_token_file_t;



typedef struct fuzzer_token_s
{
	int type;
	int location;
	void * token;
	struct fuzzer_token_s * next;
} fuzzer_token_t;



typedef struct 
{
	int input_len;
	int output_len;
	char * input;
	unsigned char * output;
	fuzzer_token_t * tokens;
} fuzzer_engine_t;



int fuzzer_case_insensitive_strcmp (char * string_one, char * string_two);
int fuzzer_is_valid_atoi (char * atoi_string);


fuzzer_engine_t * fuzzer_engine_create           ();
int               fuzzer_engine_load             (fuzzer_engine_t * engine, const char * input_string);
unsigned char *   fuzzer_engine_execute          (fuzzer_engine_t * engine, int * length);
int               fuzzer_engine_append_to_output (fuzzer_engine_t * engine, unsigned char * apendee, int apendee_length);
void              fuzzer_engine_destroy          (fuzzer_engine_t * engine);


fuzzer_token_parser_t *         fuzzer_token_parser_create             (char * input, int token_start, int token_end);
void                            fuzzer_token_parser_destroy            (fuzzer_token_parser_t * parser);
fuzzer_token_parser_element_t * fuzzer_token_parser_get_element_by_key (fuzzer_token_parser_t * parser, char * key);


fuzzer_token_parser_element_t * fuzzer_token_parser_element_create  (char * key, char * value);
void                            fuzzer_token_parser_element_destroy (fuzzer_token_parser_element_t * element);
int                             fuzzer_token_parser_element_append  (fuzzer_token_parser_element_t * first_element,
/****************************************************************/   fuzzer_token_parser_element_t * new_element);


fuzzer_token_t * fuzzer_token_create  (fuzzer_token_parser_t * parser, int location);
int              fuzzer_token_append  (fuzzer_token_t * first, fuzzer_token_t * new_token);
void             fuzzer_token_destroy (fuzzer_token_t * token);
int              fuzzer_token_execute (fuzzer_token_t * token, fuzzer_engine_t * engine);


fuzzer_token_ascii_t * fuzzer_token_ascii_create  (fuzzer_token_parser_t * parser);
void                   fuzzer_token_ascii_destroy (fuzzer_token_ascii_t * ascii);
int                    fuzzer_token_ascii_execute (fuzzer_token_ascii_t * ascii, unsigned char ** result, int * length);


fuzzer_token_byte_t * fuzzer_token_byte_create  (fuzzer_token_parser_t * parser);
void                  fuzzer_token_byte_destroy (fuzzer_token_byte_t * byte);
int                   fuzzer_token_byte_execute (fuzzer_token_byte_t * byte, unsigned char ** result, int * length);


fuzzer_token_file_t * fuzzer_token_file_create  (fuzzer_token_parser_t * parser);
void                  fuzzer_token_file_destroy (fuzzer_token_file_t * file);
int                   fuzzer_token_file_execute (fuzzer_token_file_t * file, unsigned char ** result, int * length);

#endif
