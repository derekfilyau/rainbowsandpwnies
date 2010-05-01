#include "r.h"
#include "nt.h"

#define RMM_OPENMPI 1

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#if RMM_OPENMPI == 1
	#include <omp.h>
#endif

#define RM_DEBUG 0


#define CHAIN_SIZE_STEP 1024
#define RMM_MAX_THREADS 32


void (*hash_function) (struct chain_context *);
void (*reduce_function) (struct chain_context *);



/*
*  If value of s is greater than 2^32 - 1, or not between '0' and '9', bad things
*  There has to be a better way to do this
*/
uint32_t atouint32_t (char * s)
{

	int i, j;
	int strlen_s;
	uint32_t tmp;
	uint32_t result = 0;
	
	strlen_s = strlen(s);
	
	for (i = strlen_s - 1; i >= 0; i--)
	{
		tmp = s[i] - '0';
		for (j = i + 1; j < strlen_s; j++)
			tmp *= 10;
		result += tmp;
	}

	return result;
	
}


void init_chain (struct chain_context * chain)
{

	int tmp;
	unsigned char data[16];
	
	double temp;
	
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
	
}


void do_chain (struct chain_context * chain)
{
	
	while (chain->step < chain->chain_size)
	{
		reduce_function(chain);
		hash_function(chain);
		chain->step++;
	}
	
}



void extend_nodes (struct chain_context * chain, struct chain_node * nodes, int32_t nodes_size, int32_t step_start)
{
	
	int32_t i;
	
	#if RMM_OPENMPI == 1	
		int n_procs, tid;
		struct chain_context * chains;
		
		n_procs = RMM_MAX_THREADS;
		
		chains = malloc(sizeof(struct chain_context) * n_procs);
		for (i = 0 ; i < n_procs; i++)
			memcpy(&(chains[i]), chain, sizeof(struct chain_context));
					
		#pragma omp parallel for schedule(guided, 2048) shared(chains, nodes) private(tid)
		for (i = 0; i < nodes_size; i++)
		{
			tid = omp_get_thread_num();
			chains[tid].step = step_start;
			chains[tid].A = nodes[i].a;
			chains[tid].B = nodes[i].b;
			chains[tid].C = nodes[i].c;
			chains[tid].D = nodes[i].d;
			do_chain(&(chains[tid]));
			nodes[i].a = chains[tid].A;
			nodes[i].b = chains[tid].B;
			nodes[i].c = chains[tid].C;
			nodes[i].d = chains[tid].D;
		}
		free (chains);
	#else
	for (i = 0; i < nodes_size; i++)
	{
		chain->step = step_start;
		chain->A = nodes[i].a;
		chain->B = nodes[i].b;
		chain->C = nodes[i].c;
		chain->D = nodes[i].d;
		do_chain(chain);
		nodes[i].a = chain->A;
		nodes[i].b = chain->B;
		nodes[i].c = chain->C;
		nodes[i].d = chain->D;
	}
	#endif
}



// fills empty spaces in nodes
void fill_nodes (struct chain_context * chain, struct chain_node * nodes, int32_t * nodes_size, uint32_t * chain_i, int32_t nodes_memory_size)
{

	struct chain_node node;
	
	while (*nodes_size < nodes_memory_size)
	{
		// init chain
		chain->chain = (*chain_i)++;
		init_chain (chain);
		
		// and copy it over
		node.a = chain->A;
		node.b = chain->B;
		node.c = chain->C;
		node.d = chain->D;
		node.chain_id = chain->chain;
		memcpy(&(nodes[*nodes_size]), &node, sizeof(struct chain_node));
		
		(*nodes_size)++;
	}
	
}



int write_nodes (char * filename, struct chain_node * nodes, int nodes_size)
{

	int i;
	FILE * fh;
	
	fh = fopen(filename, "wb");
	
	for (i = 0; i < nodes_size; i++)
	{
		#if R_FULL_CHAINS == 1
		fwrite(&(nodes[i]), 20, 1, fh);
		#else
		fwrite(&(nodes[i]), 12, 1, fh);
		#endif
	}
	
	fclose(fh);
	
	return 1;
	
}
	

	

int main (int argc, char * argv[])
{
	
	int32_t chain_size_i;
	int32_t num_chains;
	uint32_t chain_i;
	int32_t chain_size;
	// number of nodes we have in nodes. nodes[nodes_size] is the next freely available node
	int32_t nodes_size = 0; 
	time_t timer;
	struct chain_context chain;
	struct chain_node * nodes;
	
	
	
	/*****************************
	* DEAL WITH INPUT            *
	*****************************/
	if (argc != 9)
	{
		//             0  1          2               3         4                  5            6            7         8
		printf("Usage: %s <filename> <hash_function> <charset> <plaintext_length> <chain_size> <num_chains> <table_i> <chain_offset>\n", argv[0]);
		printf("Valid hash functions are: nt\n");
		printf("Valid plaintexts are: az az09 azAZ09 azAZ09special\n");
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
		
	chain.plaintext_length = atoi(argv[4]);
	chain_size = atoi(argv[5]);
	num_chains = atoi(argv[6]);
	chain.table = atoi(argv[7]);
	chain_i = atouint32_t(argv[8]) * atouint32_t(argv[6]);
	memset(&(chain.plaintext), 0, 128);
	
	printf("charset %s \ttable %d\n", chain.charset, chain.table);
	printf("plaintext_length %d \tchain_size %d \tnum_chains %d\n", chain.plaintext_length, chain_size, num_chains);
	printf("starting_chain %u\n", chain_i);
	printf("\n");
	
	/********************************
	* SET UP EVERYTHING, FILL NODES *
	********************************/
	
	nodes = malloc(sizeof(struct chain_node) * num_chains);
	
	printf("filling %d nodes ... ", num_chains);
	fflush(stdout);
	timer = time(NULL);
	// fill empty nodes
	chain.chain_size = CHAIN_SIZE_STEP;
	fill_nodes(&chain, nodes, &nodes_size, &chain_i, num_chains);
	printf("done in %d seconds\n", (int) (time(NULL) - timer));
	fflush(stdout);
	
	for (chain_size_i = 0; chain_size_i * CHAIN_SIZE_STEP < chain_size; chain_size_i++)
	{
		printf("extending nodes to chain_size %d ... ", (chain_size_i + 1) * CHAIN_SIZE_STEP);
		fflush(stdout);
		timer = time(NULL);
		chain.chain_size = (chain_size_i + 1) * CHAIN_SIZE_STEP;
		extend_nodes (&chain, nodes, nodes_size, chain_size_i * CHAIN_SIZE_STEP);
		printf("done in %d seconds\n", (int) (time(NULL) - timer));
		fflush(stdout);
		
		// sort nodes, remove duplicates
		printf("sorting and eliminating identical nodes ... ");
		fflush(stdout);
		timer = time(NULL);
		nodes_size = merge_sort_no_duplicates(nodes, nodes_size);
		printf("done in %d seconds. %d nodes left\n", (int) (time(NULL) - timer), nodes_size);
		fflush(stdout);
	}
	
	write_nodes(argv[1], nodes, nodes_size);
	
	return 1;
	
}
