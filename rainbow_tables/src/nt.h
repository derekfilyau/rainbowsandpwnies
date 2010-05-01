#ifndef nt_HEADER
#define nt_HEADER

#include "md4.h"
#include "r.h"

inline void nt_reduce (struct chain_context * chain)
{
	
	int i;
	int r_i = 0;
	uint32_t charset_length = strlen(chain->charset);
	uint32_t r[4];
	
	r[0] = chain->A;
	r[1] = chain->B;
	r[2] = chain->C;
	r[3] = chain->D;
	
	r[0] += (uint32_t) (chain->step + chain->table * chain->chain_size);
	
	for (i = 0; i < chain->plaintext_length; i++)
	{
		chain->plaintext[i] = chain->charset[r[r_i] % charset_length];
		r[r_i] /= charset_length;
		if (r[r_i] == 0)
		{
			if (r_i == 3)
				r_i = 0;
			else
				r_i++;
		}
	}
	
}



inline void nt_hash (struct chain_context * chain)
{

	int i;
	char tmp[128];
	struct md4_context md4;
	
	memset(tmp, 0, 128);
	for (i = 0; i < chain->plaintext_length; i++)
	{
		tmp[i*2] = chain->plaintext[i];
	}
	
	md4_begin(&md4, (unsigned char *) &tmp, chain->plaintext_length * 2);
	md4_transform(&md4);
	
	chain->A = md4.A;
	chain->B = md4.B;
	chain->C = md4.C;
	chain->D = md4.D;
	
}


#endif
