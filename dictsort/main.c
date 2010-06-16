#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct word_list_s
{
	char * word;
	struct word_list_s * next;
} word_list_t;



int strip(char * word)
{
	
	int i;
	
	for (i = 0; i < strlen(word); i++)
	{
		if (word[i] == '\n')
		{
			word[i] = 0;
			return 0;
		}
	}
	
	return 1;

}



word_list_t * load_word_list (char * filename)
{

	int entries;
	size_t word_size;
	FILE * fh;
	word_list_t * word_list, * next;

	word_list = NULL;
	next = NULL;
	entries = 0;
	
	fh = fopen(filename, "r");
	if (fh == NULL)
	{
		printf("error opening file %s\n", filename);
		return NULL;
	}
	
	while (feof(fh) == 0)
	{
		if (word_list == NULL)
		{
			word_list = malloc(sizeof(word_list_t));
			next = word_list;
		}
		else
		{
			next->next = malloc(sizeof(word_list_t));
			next = next->next;
			next->word = NULL;
			next->next = NULL;
		}
		
		word_size = 0;
		next->word = (char *) malloc(6);
		word_size = 6;
		int crap; crap = getline(&(next->word), &word_size, fh);
		strip(next->word);
		entries++;
		
		if (entries % 100000 == 0)
			printf("%d words loaded - %s\n", entries, next->word);
	}
	fclose(fh);

	return word_list;

}



void zero_followers (int64_t followers[256][256])
{
	
	int i, j;
	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 256; j++)
			followers[i][j] = (int64_t) 0;
	}
	
}



int calculate_word_followers (word_list_t * word_list, int64_t followers[256][256])
{

	int i;
	word_list_t * next;
	
	next = word_list;
	while (next != NULL)
	{
	
		//printf("[%d] %s\n", strlen(next->word), next->word);
		//fflush(stdout);
		if (strlen(next->word) < 2)
		{
			next = next->next;
			continue;
		}
		for (i = 0; i < strlen(next->word) - 1; i++)
		{
			if (((int) next->word[i] < 0)
			    || ((int) next->word[i] > 255)
			    || ((int) next->word[i + 1] < 0)
			    || ((int) next->word[i + 1] > 255))
			    continue;
			followers[(int) next->word[i]][(int) next->word[i + 1]]++;
		}
		
		next = next->next;
	
	}
	
	return 1;
	
}



double score_word (char * word, int64_t followers[256][256])
{

	double score;
	int i;
	
	score = 0;
	
	if (strlen(word) < 2)
		return (int64_t) 0;

	for (i = 0; i < strlen(word) - 1; i++)
	{
		if (((int) word[i] < 0)
			|| ((int) word[i] > 255)
			|| ((int) word[i + 1] < 0)
			|| ((int) word[i + 1] > 255))
			continue;
		score += (double) followers[(int) word[i]][(int) word[i + 1]];
	}
	
	score /= strlen(word);
	
	return score;
	
}



void debug_word (char * word, int64_t followers[256][256])
{
	if (word == NULL)
		printf("(null word)");
	else
		printf("(%f) %s", score_word(word, followers), word);
}



void debug_word_list (word_list_t * word_list, int64_t followers[256][256])
{
	
	word_list_t * next;
	
	next = word_list;
	
	while (next != NULL)
	{
		debug_word(next->word, followers);
		printf(" -> ");
		next = next->next;
	}
	
	printf("\n");

}




word_list_t * sort_words (word_list_t * word_list, int64_t followers[256][256])
{
	
	word_list_t * one, * two;
	word_list_t * final, * final_next;
	
	// split the word_list in to two groups
	
	// one is where the middle element will be
	// and two will move along the list at twice the speed to find the end
	// when two->next == NULL, one is in the middle
	if (word_list == NULL)
		return NULL;
	
	one = word_list;
	two = one->next;
	
	// there is only one element in this list
	if (two == NULL)
		return one;
			
		
	while (two != NULL)
	{
		two = two->next;
		if (two == NULL)
			break;
		one = one->next;
		two = two->next;
	}
	
	// two becomes the second half
	two = one->next;
	
	//printf("given: ");
	//debug_word_list(word_list, followers);
	// and we set one->next to NULL to terminate the first half
	one->next = NULL;
	
	// sort the two groups
	one = sort_words(word_list, followers);
	two = sort_words(two, followers);
	//printf("one: ");
	//debug_word_list(one, followers);
	//printf("two: ");
	//debug_word_list(two, followers);
	
	if (score_word(one->word, followers) > score_word(two->word, followers))
	{
		final = one;
		one = one->next;
	}
	else
	{
		final = two;
		two = two->next;
	}
	final_next = final;
	
	// combine sorted groups
	while (1)
	{
	
		if (one == NULL)
		{
			if (two == NULL)
				break;
			final_next->next = two;
			final_next = final_next->next;
			two = two->next;
		}
		else if (two == NULL)
		{
			final_next->next = one;
			final_next = final_next->next;
			one = one->next;
		}
		else if (score_word(one->word, followers) > score_word(two->word, followers))
		{
			final_next->next = one;
			final_next = final_next->next;
			one = one->next;
		}
		else
		{
			final_next->next = two;
			final_next = final_next->next;
			two = two->next;
		}
	}
	
	final_next->next = NULL;
	
	//printf("final: ");
	//debug_word_list(final, followers);
	
	return final;
			
}



void write_words (word_list_t * word_list, char * filename)
{
	
	FILE * fh;
	word_list_t * next;
	
	fh = fopen(filename, "w");
	
	next = word_list;
	while (next != NULL)
	{
		fwrite(next->word, 1, strlen(next->word), fh);
		if (next->next != NULL)
			fwrite("\n", 1, 1, fh);
		next = next->next;
	}
	
	fclose(fh);

}


int main (int argc, char * argv[])
{

	int64_t followers[256][256];
	word_list_t * word_list;
	
	if (argc != 3)
	{
		printf("Usage: %s <filename_in> <filename_out>\n", argv[0]);
		exit(0);
	}
	
	printf("loading dictionary file\n");
	word_list = load_word_list(argv[1]);

	printf("zeroing word order\n");
	zero_followers(followers);
	
	printf("calculating optimal word order\n");
	calculate_word_followers(word_list, followers);
	
	printf("sorting words\n");
	word_list = sort_words(word_list, followers);

	printf("outputing words\n");
	write_words(word_list, argv[2]);

	/*
	word_list_t * next;
	next = word_list;
	
	while (next != NULL)
	{
		debug_word(next->word, followers);
		printf("\n");
		next = next->next;
	}
	*/
		
	return 1;

}
