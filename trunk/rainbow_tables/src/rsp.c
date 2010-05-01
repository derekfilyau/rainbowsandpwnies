#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "r.h"



#define FILENAME_SIZE 256



struct node_list_out * open_list_out (char * filename_prefix, int file_i)
{
	
	struct node_list_out * list_out;
	char filename[FILENAME_SIZE];
	
	snprintf(filename, FILENAME_SIZE, "%s_%d", filename_prefix, file_i);
	list_out = open_node_list_out(filename);
	if (list_out == NULL)
	{
		printf("Error opening %s for output\n", filename);
		exit(0);
	}
	else
		printf("Opened %s for writing\n", filename);
		
	return list_out;
	
}



int main (int argc, char * argv[])
{

	int file_i;
	int node_i;
	int chains_per_table;
	
	struct chain_node * node;
	struct node_list_in * list_in;
	struct node_list_out * list_out;
	
	
	if (argc != 4)
	{
		printf("%s <input_file> <out_prefix> <chains_per_table>\n", argv[0]);
		exit(0);
	}
	
	if (strlen(argv[2]) > 200)
	{
		printf("Pick a out_prefix <= 200 strlen\n");
		exit(0);
	}
	
	chains_per_table = atoi(argv[3]);
	
	list_in = open_node_list_in(argv[1]);
	if (list_in == NULL)
	{
		printf("Error opening input_file for reading\n");
		exit(0);
	}
	
	
	file_i = 0;
	node_i = 0;
	
	
	list_out = open_list_out(argv[2], file_i++);
	while ((node = next_node_list_in(list_in)) != NULL)
	{
		append_node_list_out(list_out, node);
		node_i++;
		if (node_i == chains_per_table)
		{
			node_i = 0;
			flush_node_list_out(list_out);
			close_node_list_out(list_out);
			list_out = open_list_out(argv[2], file_i++);
		}
	}
	
	flush_node_list_out(list_out);
	close_node_list_out(list_out);
	
	close_node_list_in(list_in);

	return 1;	

}
