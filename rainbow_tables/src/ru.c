#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "r.h"

#define RS_DEBUG 1

/*
*  THIS FILE DESPERATELY NEEDS TO BE UPDATE TO BE BROUGHT IN LINE
*  WITH NODE_LIST_IN
*/

/*
	if a > b returns 1
	if a < b returns -1
	if a = b returns 0
	memcmp anyone?
*/
/*
int compare_nodes (struct chain_node * a, struct chain_node * b)
{
	if (a->a < b->a)
		return -1;
	else if (a->a > b->a)
		return 1;
	else
	{
		if (a->b < b->b)
			return -1;
		else if (a->b > b->b)
			return 1;
		#if R_FULL_CHAINS == 1
		else
		{
			if (a->c < b->c)
				return -1;
			else if (a->c > b->c)
				return 1;
			else
			{
				if (a->d < b->d)
					return -1;
				else if (a->d > b->d)
					return 1;
			}
		}
		#endif
	}
	return 0;
}
*/



/*
	loads chains into memory from file and returns pointer to those chains in memory
	number of chains loaded is placed in chains_n
*/
int load_chains (char * filename)
{

	int unique_chains = 0;
	FILE * fh;
	struct chain_node node;
	struct chain_node last_node;
	
	last_node.a = 0xffffffff;
	last_node.b = 0xffffffff;
	last_node.c = 0xffffffff;
	last_node.d = 0xffffffff;
	
	fh = fopen(filename, "rb");
	
	// read in a chain from our file
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
			
		#if R_FULL_CHAINS == 0
		node.c = 0;
		node.d = 0;
		#endif
			
		if (compare_nodes(&node, &last_node) != 0)
		{
			unique_chains++;
			last_node.a = node.a;
			last_node.b = node.b;
			#if R_FULL_CHAINS == 1
			last_node.c = node.c;
			last_node.d = node.d;
			#endif
		}
		
	}
	
	fclose(fh);
	
	return unique_chains;
	
}
	



int main (int argc, char * argv[])
{

	int unique_chains;
	
	if (argc != 2)
	{
		printf("Usage: %s <infile>\n", argv[0]);
		exit(0);
	}
	
	unique_chains = load_chains(argv[1]);
	
	printf("%d\n", unique_chains);
		
	return 1;
	
}
