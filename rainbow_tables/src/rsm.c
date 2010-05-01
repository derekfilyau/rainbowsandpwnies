#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "r.h"


#define RS_DEBUG 1



int main (int argc, char * argv[])
{

	struct node_list_in * list_a;
	struct node_list_in * list_b;
	struct node_list_out * list_out;
	struct chain_node * node_a;
	struct chain_node * node_b;
	
	uint32_t i = 0;
	
	if (argc != 4)
	{
		printf("Usage: %s <infile_a> <infile_b> <outfile(sorted)>\n", argv[0]);
		exit(0);
	}
	
	list_a = open_node_list_in(argv[1]);
	if (list_a == NULL)
	{
		printf("error opening %s\n", argv[1]);
		return 0;
	}
	
	list_b = open_node_list_in(argv[2]);
	if (list_b == NULL)
	{
		printf("error opening %s\n", argv[2]);
		return 0;
	}
	
	list_out = open_node_list_out(argv[3]);
	if (list_out == NULL)
	{
		printf("error opening %s\n", argv[3]);
		return 0;
	}
	
	node_a = next_node_list_in(list_a);
	node_b = next_node_list_in(list_b);
	while (1)
	{
		if (node_a == NULL)
		{
			if (node_b == NULL)
				break;
			append_node_list_out(list_out, node_b);
			node_b = next_node_list_in(list_b);
		}
		else if (node_b == NULL)
		{
			append_node_list_out(list_out, node_a);
			node_a = next_node_list_in(list_a);
		}
		else if (compare_nodes(node_a, node_b) < 0)
		{
			append_node_list_out(list_out, node_a);
			node_a = next_node_list_in(list_a);
		}
		else if (compare_nodes(node_a, node_b) > 0)
		{
			append_node_list_out(list_out, node_b);
			node_b = next_node_list_in(list_b);
		}
		else
			node_a = next_node_list_in(list_a);
		i++;
	}
	
	close_node_list_in(list_a);
	close_node_list_in(list_b);
	
	flush_node_list_out(list_out);
	close_node_list_out(list_out);
	
	printf("%d nodes processed\n", i);
	
	return 1;
	
}
