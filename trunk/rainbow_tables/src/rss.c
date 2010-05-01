#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "r.h"


#define RS_DEBUG 1





int main (int argc, char * argv[])
{

	struct node_list_in * list_in;
	struct node_list_out * list_out;
	struct chain_node * node_a;
	struct chain_node * node_b;
	
	int i = 0;
	
	if (argc != 3)
	{
		printf("Usage: %s <infile(sorted)> <outfile>\n", argv[0]);
		exit(0);
	}
	
	list_in = open_node_list_in(argv[1]);
	if (list_in == NULL)
	{
		printf("error opening %s\n", argv[1]);
		return 0;
	}
	
	list_out = open_node_list_out(argv[2]);
	if (list_out == NULL)
	{
		printf("error opening %s\n", argv[2]);
		return 0;
	}
	
	
	node_a = next_node_list_in(list_in);
	node_b = next_node_list_in(list_in);
	while (node_b != NULL)
	{
	
		if (compare_nodes(node_a, node_b) != 0)
			append_node_list_out(list_out, node_a);
			
		node_a = node_b;
		node_b = next_node_list_in(list_in);
		if (++i % 0xFFFFF == 0)
			printf("%d nodes done\n", i);
	}
	append_node_list_out(list_out, node_a);
			
	close_node_list_in(list_in);
	
	flush_node_list_out(list_out);
	close_node_list_out(list_out);
	
	printf("%d nodes processed\n", i);
	
	return 1;
	
}
