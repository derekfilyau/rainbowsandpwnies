#ifndef r_H
#define r_H

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


// when set to 1, all programs expect full 20 byte chains, containing
// the complete hash
// when set to 0, all programs expect 12 byte chains, containing half the
// complete hash
#define R_FULL_CHAINS 1

#define RSM_NODES_PER_LIST 10240


char az[] = "abcdefghijklmnopqrstuvwxyz";
char az09[] = "abcdefghijklmnopqrstuvwxyz0123456789";
char azAZ09[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
char azAZ09special[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-=_+[]{};':\",./<>?~\\| ";


char * get_charset (int option)
{
	switch (option)
	{
		case 0 :
			return az;
		case 1 :
			return az09;
		case 2 :
			return azAZ09;
		case 3 :
			return azAZ09special;
	}
	
	return NULL;
}


struct chain_context
{
	int32_t chain;
	int32_t table;
	int32_t step;
	int32_t chain_size; // number of steps in chain
	int32_t plaintext_length;
	char plaintext[128];
	char * charset;
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
};



struct chain_node
{
	uint32_t chain_id;
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
};



/*
* Generic reduction function. Hash specific functions are based
* off of this one
*/
/*
void reduce (struct md4_context * context, struct chain_context * chain)
{
	
	int i;
	int r_i = 0;
	uint32_t charset_length = strlen(chain->charset);
	uint32_t r[4];
	uint32_t t;
	
	r[0] = context->A;
	r[1] = context->B;
	r[2] = context->C;
	r[3] = context->D;
	
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
*/




/*
	if a > b returns 1
	if a < b returns -1
	if a = b returns 0
	memcmp anyone?
*/
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
	}
	return 0;
}



void swap_nodes (struct chain_node * a, struct chain_node * b)
{
	struct chain_node tmp;
	memcpy(&tmp, b, sizeof(struct chain_node));
	memcpy(b, a, sizeof(struct chain_node));
	memcpy(a, &tmp, sizeof(struct chain_node));
}



int merge_sort (struct chain_node * nodes, int nodes_size)
{

	int a_i;
	int b_i;

	int a_size;
	int b_size;
	
	struct chain_node * tmp;
	
	if (nodes_size <= 1)
		return 0;
	
	a_i = 0;
	b_i = 0;
	
	a_size = (nodes_size / 2) + (nodes_size % 2);
	b_size = nodes_size / 2;
	
	if (merge_sort(nodes, a_size) == -1)
		return -1;
	if (merge_sort(&(nodes[a_size]), b_size) == -1)
		return -1;
	
	tmp = malloc(sizeof(struct chain_node) * nodes_size);
	if (tmp == NULL)
		return -1;
		
	//memset(tmp, 0xAAAAAAAA, sizeof(struct chain_node) * nodes_size);
	
	while ((a_i < a_size) || (b_i < b_size))
	{
		if (a_i == a_size)
		{
			memcpy(&(tmp[a_i + b_i]), &(nodes[a_size + b_i]), sizeof(struct chain_node));
			b_i++;
		}
		else if (b_i == b_size)
		{
			memcpy(&(tmp[a_i + b_i]), &(nodes[a_i]), sizeof(struct chain_node));
			a_i++;
		}
		else if (compare_nodes(&(nodes[a_i]), &(nodes[a_size + b_i])) <= 0)
		{
			memcpy(&(tmp[a_i + b_i]), &(nodes[a_i]), sizeof(struct chain_node));
			a_i++;
		}
		else
		{
			memcpy(&(tmp[a_i + b_i]), &(nodes[a_size + b_i]), sizeof(struct chain_node));
			b_i++;
		}
	}
	
	//memset(nodes, 0xDEADBEEF, sizeof(struct chain_node) * nodes_size);
	
	memcpy(nodes, tmp, sizeof(struct chain_node) * nodes_size);
	
	free(tmp);
	
	return 0;
	
}



int merge_sort_no_duplicates (struct chain_node * nodes, int nodes_size)
{

	int a_i;
	int b_i;
	int copy_i = 0;

	int a_size;
	int b_size;
	int b_offset;
	
	struct chain_node * tmp;
	
	if (nodes_size <= 1)
		return nodes_size;
	
	a_i = 0;
	b_i = 0;
	
	a_size = (nodes_size / 2) + (nodes_size % 2);
	b_size = nodes_size / 2;
	b_offset = a_size;
	
	a_size = merge_sort_no_duplicates(nodes, a_size);
	b_size = merge_sort_no_duplicates(&(nodes[b_offset]), b_size);
	nodes_size = a_size + b_size;
	
	tmp = malloc(sizeof(struct chain_node) * nodes_size);
	if (tmp == NULL)
		return -1;
		
	//memset(tmp, 0xAAAAAAAA, sizeof(struct chain_node) * nodes_size);
	
	while ((a_i < a_size) || (b_i < b_size))
	{
		if (a_i == a_size)
		{
			memcpy(&(tmp[copy_i++]), &(nodes[b_offset + b_i]), sizeof(struct chain_node));
			b_i++;
		}
		else if (b_i == b_size)
		{
			memcpy(&(tmp[copy_i++]), &(nodes[a_i]), sizeof(struct chain_node));
			a_i++;
		}
		else if (compare_nodes(&(nodes[a_i]), &(nodes[b_offset + b_i])) < 0)
		{
			memcpy(&(tmp[copy_i++]), &(nodes[a_i]), sizeof(struct chain_node));
			a_i++;
		}
		else if (compare_nodes(&(nodes[a_i]), &(nodes[b_offset + b_i])) > 0)
		{
			memcpy(&(tmp[copy_i++]), &(nodes[b_offset + b_i]), sizeof(struct chain_node));
			b_i++;
		}
		else
		{
			if (a_i < a_size)
				a_i++;
			else
				b_i++;
			nodes_size--;
		}
	}
	
	//memset(nodes, 0xDEADBEEF, sizeof(struct chain_node) * tmpsize);
	
	memcpy(nodes, tmp, sizeof(struct chain_node) * nodes_size);
	
	free(tmp);
	
	return nodes_size;
	
}



/*
*  Yes, they aren't linked lists. I know.
*/


struct node_list_out
{
	FILE * fh;
	int node_i; // always points to the next available node
	int error;
	int eof;
	struct chain_node nodes[RSM_NODES_PER_LIST];
};



struct node_list_in
{
	FILE * fh;
	int node_i;
	int nodes_in_list;
	int error;
	int eof;
	struct chain_node nodes[RSM_NODES_PER_LIST];
};



struct node_list_out * open_node_list_out (char * filename)
{

	struct node_list_out * node_list;
	
	node_list = malloc(sizeof(struct node_list_out));
	
	node_list->fh = fopen(filename, "wb");
	if (node_list->fh == NULL)
	{
		free(node_list);
		return NULL;
	}
	
	node_list->node_i = 0;
	node_list->eof = 0;
	node_list->error = 0;
	
	return node_list;
	
}



void append_node_list_out (struct node_list_out * node_list, struct chain_node * node)
{

	int i;

	// if our buffer is full, time to write out
	if (node_list->node_i == RSM_NODES_PER_LIST)
	{
		for (i = 0; i < RSM_NODES_PER_LIST; i++)
		{
			#if R_FULL_CHAINS == 1
			fwrite(&(node_list->nodes[i]), 20, 1, node_list->fh);
			#else
			fwrite(&(node_list->nodes[i]), 12, 1, node_list->fh);
			#endif
		}
		node_list->node_i = 0;
	}
	
	memcpy(&(node_list->nodes[node_list->node_i++]), node, sizeof(struct chain_node));
	
}



void flush_node_list_out (struct node_list_out * node_list)
{

	int i;
	
	for (i = 0; i < node_list->node_i; i++)
	{
		#if R_FULL_CHAINS == 1
		fwrite(&(node_list->nodes[i]), 20, 1, node_list->fh);
		#else
		fwrite(&(node_list->nodes[i]), 12, 1, node_list->fh);
		#endif
	}
	
}



void close_node_list_out (struct node_list_out * node_list)
{

	fclose(node_list->fh);
	free(node_list);
	
}
		



struct node_list_in * open_node_list_in (char * filename)
{

	struct node_list_in * node_list;
	
	node_list = malloc(sizeof(struct node_list_in));
	
	node_list->fh = fopen(filename, "rb");
	if (node_list->fh == NULL)
	{
		free(node_list);
		return NULL;
	}
	
	node_list->node_i = 0;
	node_list->nodes_in_list = 0;
	node_list->eof = 0;
	node_list->error = 0;
	
	return node_list;
	
}



// returns NULL on error, check error flag
struct chain_node * next_node_list_in (struct node_list_in * node_list)
{

	int i;
	
	// we have reached the end of the file
	if ((node_list->node_i == node_list->nodes_in_list) && (node_list->eof == 1))
		return NULL;
	
	// we need to load more nodes
	if ((node_list->node_i == RSM_NODES_PER_LIST) || (node_list->nodes_in_list == 0))
	{
		node_list->nodes_in_list = 0;
		node_list->node_i = 0;
		for (i = 0; i < RSM_NODES_PER_LIST; i++)
		{
			if ((fread(&(node_list->nodes[i].chain_id), 4, 1, node_list->fh) == 0)
			 || (fread(&(node_list->nodes[i].a), 4, 1, node_list->fh) == 0)
			 || (fread(&(node_list->nodes[i].b), 4, 1, node_list->fh) == 0)
			 #if R_FULL_CHAINS == 1
			 || (fread(&(node_list->nodes[i].c), 4, 1, node_list->fh) == 0)
			 || (fread(&(node_list->nodes[i].d), 4, 1, node_list->fh) == 0)
			 #endif
			 )
			{
			 	if (feof(node_list->fh))
			 		node_list->eof = 1;
			 	else
			 		node_list->error = errno;
			 	break;
			}
			#if R_FULL_CHAINS == 0
			node_list->nodes[i].c = 0;
			node_list->nodes[i].d = 0;
			#endif
			node_list->nodes_in_list++;
		}
		// we encountered an error before we loaded anymore nodes
		if (node_list->node_i == node_list->nodes_in_list)
			return NULL;
	}
	
	return &(node_list->nodes[node_list->node_i++]);
	
}



void close_node_list_in (struct node_list_in * node_list)
{
	
	fclose(node_list->fh);
	free(node_list);
	
}



#endif
