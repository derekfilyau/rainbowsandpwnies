#include "r.h"

#define RM_DEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

	


void do_chain (struct chain_context * chain)
{
	
	unsigned char data[16];
	struct md4_context context;
	
	double temp;
	
	chain->step = 0;
	chain->A = 0;
	chain->B = 0;
	chain->C = 0;
	chain->D = 0;
	
	memset(chain->plaintext, 0, 128);
	
	temp = sin((double) chain->chain);
	memcpy(data, &temp, 8);
	
	temp = sin((double) chain->table);
	memcpy(&(data[8]), &temp, 8);
	
	md4_begin(&context, data, 16);
	md4_transform(&context);
	
	while (chain->step < chain->chain_size)
	{
	
		#if RM_DEBUG == 1
			printf("%08x%08x%08x%08x ", context.A,
										 context.B,
										 context.C,
										 context.D);
		#endif
	
		reduce(&context, chain);
		
		md4_begin(&context, chain->plaintext, chain->plaintext_length);
		md4_transform(&context);
		
		#if RM_DEBUG ==  1
		printf("%d %s %08x%08x%08x%08x\n", chain->step, chain->plaintext,
		                                context.A,
		                                context.B,
		                                context.C,
		                                context.D);
		#endif
		
		chain->step++;
	}
	
	
	chain->A = context.A;
	chain->B = context.B;
	chain->C = context.C;
	chain->D = context.D;
	
}
	

int main (int argc, char * argv[])
{
	
	int32_t i, num_chains;
	FILE * fh;
	struct chain_context chain;
	
	
	
	#if RM_DEBUG == 1
		chain.chain = 0;
		chain.plaintext_length = 2;
		chain.charset = az;
		chain.table = 0;
		chain.chain_size = 5;
		printf("chain.chain %d\tchain.plaintext_length %d\n", chain.chain, chain.plaintext_length);
		printf("chain.table %d\tchain.chain_size %d\n", chain.table, chain.chain_size);
		do_chain(&chain);
		return 1;
	#endif
	
	
	if (argc != 6)
	{
		printf("Usage: %s <filename> <plaintext_length> <chain_size> <num_chains> <table_i>\n", argv[0]);
		exit(0);
	}

	fh = fopen(argv[1], "wb");
	chain.chain_size = atoi(argv[3]);
	num_chains = atoi(argv[4]);
	chain.table = atoi(argv[5]);
	
	chain.chain = 0;
	chain.plaintext_length = atoi(argv[2]);
	chain.charset = az;
	
	printf("Making %d chains of %d size for table %d\n", num_chains, chain.chain_size, chain.table);
	
	for (i = 0; i < num_chains; i++)
	{
		chain.chain = i;
		do_chain(&chain);
		fwrite(&i, 4, 1, fh);
		fwrite(&(chain.A), 4, 1, fh);
		fwrite(&(chain.B), 4, 1, fh);
		fwrite(&(chain.C), 4, 1, fh);
		fwrite(&(chain.D), 4, 1, fh);
		if (i % 10000 == 0)
			printf("Chain #%d\n", i);
	}
	
	fclose(fh);
	
	return 1;
	
}
