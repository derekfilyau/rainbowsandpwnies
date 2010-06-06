#include "r.h"

int extend_chain (step_t * step, int num_steps)
{

	int i;
	
	for (i = 0; i < num_steps; i++)
	{
		index_to_plaintext(step);
		plaintext_to_index(step);
		printf("%d %s %llu\n", step->step, step->plaintext.plaintext, step->index);
		step->step++;
	}
	
	return 0;

}



step_t * step_create ()
{

	step_t * step;
	
	step = (step_t *) malloc(sizeof(step_t));
	
	if (step == NULL)
		return NULL;
	
	step->step = 0;
	step->index = 0;
	step->plaintext.length = 0;
	step->plaintext.strlen_charset = 0;
	step->plaintext.charset = NULL;
	
	memset(&(step->plaintext.plaintext), 0, 64);
	
	return step;

}



void step_destroy (step_t * step)
{

	if (step != NULL)
		free(step);

}



int index_to_plaintext (step_t * step)
{

	uint64_t index;
	int i;
	
	index = step->index + (uint64_t) step->step;
	
	for (i = 0; i < step->plaintext.length; i++)
	{
		step->plaintext.plaintext[i] = step->plaintext.charset[index % step->plaintext.strlen_charset];
		index /= step->plaintext.strlen_charset;
	}
	
	return 0;

}
