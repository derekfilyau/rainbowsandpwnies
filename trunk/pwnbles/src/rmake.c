#include <stdio.h>
#include <string.h>

#include "r.h"
#include "hash_md4.h"

int (* plaintext_to_index) (step_t * step);


int main (int argc, char * argv[])
{

	step_t step;
	
	step.step = 0;
	step.index = 0;
	step.plaintext.length = 9;
	step.plaintext.charset = az;
	step.plaintext.strlen_charset = strlen(step.plaintext.charset);
	memset(step.plaintext.plaintext, 0, 64);
	
	plaintext_to_index = hash_md4_plaintext_to_index;
	
	extend_chain(&step, 5);
	
	return 0;

}
