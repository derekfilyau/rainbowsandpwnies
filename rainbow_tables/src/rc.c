#include "r.h"
#include "nt.h"

#define RC_DEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>


void (*hash_function) (struct chain_context *);
void (*reduce_function) (struct chain_context *);


uint32_t hex_string_to_uint32 (char * s)
{

	uint32_t result = 0;
	uint32_t tmp;
	int i;

	for (i = 0; i < 8; i++)
	{
		if ((s[i] >= '0') && (s[i] <= '9'))
			tmp = (uint32_t) (s[i]) - (uint32_t) '0';
		else if ((s[i] >= 'a') && (s[i] <= 'f'))
			tmp = (uint32_t) s[i] - (uint32_t) 'a' + (uint32_t) 10;
		else if ((s[i] >= 'A') && (s[i] <= 'F'))
			tmp = (uint32_t) s[i] - (uint32_t) 'A' + (uint32_t) 10;
		else
			continue;
		result |= tmp << (28 - (i * 4));
	}

	return result;

}


/*
	steps back <int reduction> times
*/
void create_context_chain (struct chain_context * chain, int reduction)
{

	chain->step = chain->chain_size - (int32_t) reduction;
	
	while (chain->step < chain->chain_size)
	{
		reduce_function(chain);
		hash_function(chain);
		chain->step++;	
	}
	
}



int find_hash_in_chain (struct chain_context * chain, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
	
	double temp;
	unsigned char data[16];
	int tmp;
	
	chain->step = 0;
	
	memset(chain->plaintext, 0, 128);
	
	temp = sin((double) chain->chain);
	memcpy(data, &temp, 8);
	
	temp = sin((double) chain->table);
	memcpy(&(data[8]), &temp, 8);
	
	memcpy(&(chain->plaintext), data, 16);
	tmp = chain->plaintext_length;
	chain->plaintext_length = 16;
	
	hash_function(chain);
	chain->plaintext_length = tmp;
	memset(chain->plaintext, 0, 128);
	
	while (chain->step < chain->chain_size)
	{
	
		reduce_function(chain);
		hash_function(chain);
		
		if ((chain->A == a) && (chain->B == b) && (chain->C == c) && (chain->D == d))
		{
			printf("found in chain %d :%s\n", chain->chain, chain->plaintext);
			return 1;
		}
		chain->step++;
	}
	
	return 0;
	
}



int main (int argc, char * argv[])
{

	FILE * fh;
	struct chain_context chain;
	struct chain_node node;
	struct chain_node * nodes;
	uint32_t A, B, C, D;
	int i;

	/************************
	* DEAL WITH INPUT       *
	************************/

	if (argc != 8)
	{
		//             0  1               2               3         4                  5            6         7
		printf("Usage: %s <sorted rtable> <hash_function> <charset> <plaintext_length> <chain_size> <table_i> <hash>\n", argv[0]);
		exit(0);
	}
	
	if (strcmp(argv[2], "nt") == 0)
	{
		hash_function = nt_hash;
		reduce_function = nt_reduce;
	}
	else
	{
		printf("Invalid hash function\n");
		exit(0);
	}
	
	if (strcmp(argv[3], "az") == 0)
		chain.charset = az;
	else if (strcmp(argv[3], "az09") == 0)
		chain.charset = az09;
	else if (strcmp(argv[3], "azAZ09") == 0)
		chain.charset = azAZ09;
	else if (strcmp(argv[3], "azAZ09special") == 0)
		chain.charset = azAZ09special;
	else
	{
		printf("Invalid hash function\n");
		exit(0);
	}
	
	/*********************************
	* INIT CRAP                      *
	*********************************/
	
	A = hex_string_to_uint32(&(argv[7][0]));
	B = hex_string_to_uint32(&(argv[7][8]));
	C = hex_string_to_uint32(&(argv[7][16]));
	D = hex_string_to_uint32(&(argv[7][24]));

	chain.chain = 0;
	chain.chain_size = atoi(argv[5]);
	chain.table = atoi(argv[6]);
	chain.plaintext_length = atoi(argv[4]);
	chain.charset = az;
	memset(chain.plaintext, 0, 128);
	
	printf("chain.chain %d\tchain.plaintext_length %d\n", chain.chain, chain.plaintext_length);
	printf("chain.table %d\tchain.chain_size %d\n", chain.table, chain.chain_size);
	printf("starting hash %08x%08x%08x%08x\n", A, B, C, D);
	
	nodes = malloc(sizeof(struct chain_node) * chain.chain_size);
	nodes[0].a = A;
	nodes[0].b = B;
	nodes[0].c = C;
	nodes[0].d = D;
	
	/**********************************************
	* CONSTRUCT CHAINS TO FIND HASHES TO LOOK FOR *
	**********************************************/
	
	printf("generating chains... ");
	for (i = 1; i < chain.chain_size; i++)
	{
		chain.A = A;
		chain.B = B;
		chain.C = C;
		chain.D = D;
		create_context_chain(&chain, i);
		nodes[i].a = chain.A;
		nodes[i].b = chain.B;
		nodes[i].c = chain.C;
		nodes[i].d = chain.D;
	}
	printf("done\n");
	
	printf("merge_sort... ");
	merge_sort(nodes, chain.chain_size);
	printf("done\n");
	
	#if RC_DEBUG == 1
	printf("Testing sorted list\n");
	for (i = 0; i < chain.chain_size; i++)
	{
		printf("%08x%08x%08x%08x\n", nodes[i].a, nodes[i].b, nodes[i].c, nodes[i].d);
	}
	#endif
	
	/**************************
	* SEARCH FOR THOSE HASHES *
	**************************/
	
	i = 0;
	fh = fopen(argv[1], "rb");
	
	printf("search for hash\n");
	while (feof(fh) == 0)
	{
		if ((fread(&(node.chain_id), 4, 1, fh) == 0)
		    || (fread(&(node.a), 4, 1, fh) == 0)
		    || (fread(&(node.b), 4, 1, fh) == 0)
		    #if R_FULL_CHAINS == 1
		    || (fread(&(node.c), 4, 1, fh) == 0)
		    || (fread(&(node.d), 4, 1, fh) == 0)
		    #endif
		    )
			break;
		
		while (1)
		{
			if (node.a > nodes[i].a)
				i++;
			else if ((node.a == nodes[i].a) && (node.b > nodes[i].b))
				i++;
			else
				break;
			if (i == chain.chain_size)
				return 1;
		}
		
		#if RC_DEBUG == 1  
			printf("%08x-%08x-%08x-%08x %08x-%08x-%08x-%08x\n",
			node.a, node.b, node.c, node.d,
			nodes[i].a, nodes[i].b, nodes[i].c, nodes[i].d);
		#endif
		
		if ((node.a == nodes[i].a)
		    && (node.b == nodes[i].b))
		{
			chain.chain = node.chain_id;
			if (find_hash_in_chain (&chain, A, B, C, D) == 1)
				return 1;
		}
	}
	
	fclose(fh);
	
	return 1;

}
