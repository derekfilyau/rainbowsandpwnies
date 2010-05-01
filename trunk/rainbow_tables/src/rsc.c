#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "r.h"



struct table_in
{
	
	char * filename;
	struct node_list_in * list_in;
	struct chain_node * node;
	
};



struct table_in * table_in_create (char * filename)
{

	struct table_in * t;
	
	t = (struct table_in *) malloc(sizeof(struct table_in));
	
	t->filename = (char *) malloc(strlen(filename) + 1);
	if (t->filename == NULL)
	{
		printf ("asdfasdfasdf");
		return NULL;
	}
	strcpy(t->filename, filename);
	
	t->list_in = open_node_list_in(filename);
	if (t->list_in == NULL)
	{
		printf ("oh shit %s\n", filename);
		return NULL;
	}
	
	t->node = next_node_list_in(t->list_in);
	
	return t;
	
}



void table_in_close (struct table_in * t)
{

	free(t->filename);
	close_node_list_in(t->list_in);
	
}



struct table_in ** load_tables_in (int tables_n, char * tables_filenames[])
{

	int i;
	struct table_in ** tables;
	
	tables = (struct table_in **) malloc(sizeof(struct tables_in *) * tables_n);
	
	for (i = 0; i < tables_n; i++)
	{
		tables[i] = table_in_create(tables_filenames[i]);
		if (tables[i] == NULL)
			printf("Error loading table %s\n", tables_filenames[i]);
	}
	
	return tables;
	
}



int main (int argc, char * argv[])
{

	int i;
	int lowest_table;
	int tables_n;
	struct node_list_out * list_out;
	struct table_in ** tables_in;
	
	
	if (argc < 4)
	{
		printf("Usage: %s <table_out> [tables_in 0..n]\n", argv[0]);
		printf("The first argument is the table file to write to.\n");
		printf("The subsequent arguments are sorted tables to merge into the table_out.\n");
		exit(0);
	}
	
	list_out = open_node_list_out(argv[1]);
	if (list_out == NULL)
	{
		printf("error opening %s for output\n", argv[1]);
		exit(0);
	}
	
	tables_in = load_tables_in (argc - 2, &(argv[2]));
	
	tables_n = argc - 2;
	lowest_table = 0;
	while (lowest_table >= 0)
	{
		lowest_table = -1;
		
		for (i = 0; i < tables_n; i++)
		{
			if (tables_in[i]->node != NULL)
			{
				if (lowest_table == -1)
					lowest_table = i;
				else if (compare_nodes(tables_in[i]->node, tables_in[lowest_table]->node) < 0)
					lowest_table = i;
				else
				{
					while (compare_nodes(tables_in[i]->node, tables_in[lowest_table]->node) == 0)
					{
						tables_in[i]->node = next_node_list_in(tables_in[i]->list_in);
						if (tables_in[i]->node == NULL)
							break;
					}
				}
			}
		}
		if (lowest_table >= 0)
		{
			append_node_list_out(list_out, tables_in[lowest_table]->node);
			tables_in[lowest_table]->node = next_node_list_in(tables_in[lowest_table]->list_in);
		}
	}
	
	flush_node_list_out(list_out);
	
	printf ("done\n");
	fflush(stdout);
	
	close_node_list_out(list_out);
	for (i = 0; i < tables_n; i++)
	{
		table_in_close(tables_in[i]);
	}
	
	return 1;
	
}
