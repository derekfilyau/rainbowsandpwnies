#include "nt.h"


int main ()
{

	struct chain_context context;
	
	memset(&context, 0, 128);
	context.plaintext_length = 0;
	context.charset = az;
	context.step = 0;
	
	strcpy(context.plaintext, "hello");
	context.plaintext_length = 5;
	
	nt_hash(&context);
	
	printf("%08x%08x%08x%08x\n", context.A, context.B, context.C, context.D);
	printf("066ddfd4ef0e9cd7c256fe77191ef43c\n");
	
	nt_reduce(&context);
	printf("%s\n", context.plaintext);
	
	return 1;

}
