#include "hash_md4.h"



uint64_t hash_md4_hash_to_index(unsigned char * hash)
{

	uint64_t index;
	
	index = (uint64_t) ((uint32_t) hash);
	index ^= (uint64_t) ((uint32_t) &(hash[8]));
	index <<= 32;
	index |= (uint64_t) ((uint32_t) &(hash[16]));
	index ^= (uint64_t) ((uint32_t) &(hash[24]));
	
	return index;
	
}



int hash_md4_plaintext_to_index (step_t * step)
{

	struct md4_context context;
	
	md4_begin(&context, (unsigned char *) step->plaintext.plaintext, step->plaintext.length);
	md4_transform(&context);
	
	step->index = (uint64_t) context.A;
	step->index ^= (uint64_t) context.B;
	step->index <<= 32;
	step->index |= (uint64_t) context.C;
	step->index ^= (uint64_t) context.D;
	
	return 1;

}
	
