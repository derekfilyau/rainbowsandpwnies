#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "r.h"



#define RS_DEBUG 1





/*
	loads chains into memory from file and returns pointer to those chains in memory
	number of chains loaded is placed in chains_n
*/
struct chain_node * load_chains (char * filename, int * chains_n)
{

	int nodes_size;
	FILE * fh;
	struct chain_node * nodes;
	struct chain_node * nodes_realloc;
	
	nodes_size = 512000;
	*chains_n = 0;
	
	nodes = malloc(sizeof(struct chain_node) * nodes_size);
	if (nodes == NULL)
		return NULL;
	
	fh = fopen(filename, "rb");
	
	// read in a chain from our file
	while (feof(fh) == 0)
	{
	
		if ((fread(&(nodes[*chains_n].chain_id), 4, 1, fh) == 0)
		    || (fread(&(nodes[*chains_n].a), 4, 1, fh) == 0)
		    || (fread(&(nodes[*chains_n].b), 4, 1, fh) == 0)
		    #if R_FULL_CHAINS == 1
		    || (fread(&(nodes[*chains_n].c), 4, 1, fh) == 0)
		    || (fread(&(nodes[*chains_n].d), 4, 1, fh) == 0)
		    #endif
		    )
			break;
			
		#if R_FULL_CHAINS == 0
		nodes[*chains_n].c = 0;
		nodes[*chains_n].d = 0;
		#endif
			
		(*chains_n)++;
		
		if (*chains_n == nodes_size)
		{
			nodes_size += 512000;
			nodes_realloc = realloc(nodes, sizeof(struct chain_node) * nodes_size);
			if (nodes_realloc == NULL)
				return NULL;
			nodes = nodes_realloc;
		}
		
	}
	
	fclose(fh);
	
	return nodes;
	
}



int write_chains (char * filename, struct chain_node * nodes, int nodes_size)
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

	struct chain_node * nodes;
	int nodes_size;
	
	if (argc != 3)
	{
		printf("Usage: %s <infile> <outfile(sorted)>\n", argv[0]);
		exit(0);
	}
	
	printf("loading chains\n");
	nodes = load_chains(argv[1], &nodes_size);
	if (nodes == NULL)
	{
		printf("error loading chains\n");
		exit(0);
	}
	
	//printf("merge_sort %d\n", merge_sort(nodes, nodes_size));

	printf("sorting chains\n");
	merge_sort(nodes, nodes_size);

	printf("writing sorted chains\n");
	write_chains(argv[2], nodes, nodes_size);
	
	return 1;
	
}
