#include <stdio.h>
#include <string.h>
#include "md4.h"

int main ()
{

	int all_tests_pass = 0;

	/*
	* md4.h
	*/
	
	struct md4_context context;
	md4_begin(&context, (unsigned char *) "testing", strlen("testing"));
	md4_transform(&context);
	
	printf("%08x%08x%08x%08x\n0c2be0003f0debdcf644525bdaf6e45d\n", context.A, context.B, context.C, context.D);
	if ((context.A != 0x0c2be000) || (context.B != 0x3f0debdc)
	    || (context.C != 0xf644525b) || (context.D != 0xdaf6e45d))
		all_tests_pass = -1;
	
	if (all_tests_pass)
		printf("TEST FAIL\n");
	else
		printf("ALL TESTS PASS\n");
	
	return 1;

}
