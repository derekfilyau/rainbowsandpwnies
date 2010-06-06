#ifndef r_HEADER
#define r_HEADER

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define az "abcdefghijklmnopqrstuvwxyz"
#define azAZ "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define azAZ09 "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#define az09 "abcdefghijklmnopqrstuvwxyz0123456789"
#define azAZ09special "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!#@$%^&*()-=_+[]{};':\",./<>?"

typedef struct plaintext_s
{
	int length;
	int strlen_charset;
	char * charset;
	char plaintext[64];
} plaintext_t;

typedef struct step_s
{
	int32_t step;
	uint64_t index;
	plaintext_t plaintext;
} step_t;

step_t * step_create ();
void     step_destroy (step_t * step);

int index_to_plaintext (step_t * step);

int extend_chain (step_t * step, int num_steps);

#endif
