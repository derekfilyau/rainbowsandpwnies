#ifndef md4_HEADER
#define md4_HEADER

/*
	Only use for md4 hashes < ~50 bytes
*/

#define DEBUG 0


#include <inttypes.h>
#include <string.h>

#if DEBUG == 1
	#include <stdio.h>
#endif


#define md4_A 0x67452301
#define md4_B 0xefcdab89
#define md4_C 0x98badcfe
#define md4_D 0x10325476

struct md4_context
{
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
	uint32_t M[16];
};


inline uint32_t md4_rotl (uint32_t x, uint32_t s)
{
	return (x << s) | (x >> (32 - s));
}


inline uint32_t md4_F (uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) | ((~x) & z);
}


inline uint32_t md4_G (uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) | (x & z) | (y & z);
}


inline uint32_t md4_H (uint32_t x, uint32_t y, uint32_t z)
{
	return x ^ y ^ z;
}


inline uint32_t l2bendian (uint32_t little)
{
	return (little << 24)
	       | (little >> 24)
	       | ((little & 0xFF00) << 8)
	       | ((little & 0xFF0000) >> 8);
}


void md4_begin (struct md4_context * context, unsigned char * data, int data_len)
{

	memset (context->M, 0, 64);
	memcpy (context->M, data, data_len);
	
	((unsigned char *) context->M)[data_len] = 0x80;
	
	context->M[14] = data_len * 8;
	
	#if DEBUG == 1
		printf("M %x %x %x %x\n", context->M[0], context->M[1], context->M[2], context->M[3]);
		printf("M %x %x %x %x\n", context->M[4], context->M[5], context->M[6], context->M[7]);
		printf("M %x %x %x %x\n", context->M[8], context->M[9], context->M[10], context->M[11]);
		printf("M %x %x %x %x\n", context->M[12], context->M[13], context->M[14], context->M[15]);
	#endif
}



void md4_transform (struct md4_context * context)
{
	
	uint32_t a = md4_A;
	uint32_t b = md4_B;
	uint32_t c = md4_C;
	uint32_t d = md4_D;
	
	
	// round 1
	a = md4_rotl(a + md4_F(b, c, d) + context->M[0], 3);
	d = md4_rotl(d + md4_F(a, b, c) + context->M[1], 7);
	c = md4_rotl(c + md4_F(d, a, b) + context->M[2], 11);
	b = md4_rotl(b + md4_F(c, d, a) + context->M[3], 19);

	a = md4_rotl(a + md4_F(b, c, d) + context->M[4], 3);
	d = md4_rotl(d + md4_F(a, b, c) + context->M[5], 7);
	c = md4_rotl(c + md4_F(d, a, b) + context->M[6], 11);
	b = md4_rotl(b + md4_F(c, d, a) + context->M[7], 19);
	
	a = md4_rotl(a + md4_F(b, c, d) + context->M[8],  3);
	d = md4_rotl(d + md4_F(a, b, c) + context->M[9],  7);
	c = md4_rotl(c + md4_F(d, a, b) + context->M[10], 11);
	b = md4_rotl(b + md4_F(c, d, a) + context->M[11], 19);
	
	a = md4_rotl(a + md4_F(b, c, d) + context->M[12], 3);
	d = md4_rotl(d + md4_F(a, b, c) + context->M[13], 7);
	c = md4_rotl(c + md4_F(d, a, b) + context->M[14], 11);
	b = md4_rotl(b + md4_F(c, d, a) + context->M[15], 19);
	
	// round 2
	a = md4_rotl(a + md4_G(b, c, d) + context->M[0]  + 0x5a827999, 3);
	d = md4_rotl(d + md4_G(a, b, c) + context->M[4]  + 0x5a827999, 5);
	c = md4_rotl(c + md4_G(d, a, b) + context->M[8]  + 0x5a827999, 9);
	b = md4_rotl(b + md4_G(c, d, a) + context->M[12] + 0x5a827999, 13);
	
	a = md4_rotl(a + md4_G(b, c, d) + context->M[1]  + 0x5a827999, 3);
	d = md4_rotl(d + md4_G(a, b, c) + context->M[5]  + 0x5a827999, 5);
	c = md4_rotl(c + md4_G(d, a, b) + context->M[9]  + 0x5a827999, 9);
	b = md4_rotl(b + md4_G(c, d, a) + context->M[13] + 0x5a827999, 13);
	
	a = md4_rotl(a + md4_G(b, c, d) + context->M[2]  + 0x5a827999, 3);
	d = md4_rotl(d + md4_G(a, b, c) + context->M[6]  + 0x5a827999, 5);
	c = md4_rotl(c + md4_G(d, a, b) + context->M[10] + 0x5a827999, 9);
	b = md4_rotl(b + md4_G(c, d, a) + context->M[14] + 0x5a827999, 13);
	
	a = md4_rotl(a + md4_G(b, c, d) + context->M[3]  + 0x5a827999, 3);
	d = md4_rotl(d + md4_G(a, b, c) + context->M[7]  + 0x5a827999, 5);
	c = md4_rotl(c + md4_G(d, a, b) + context->M[11] + 0x5a827999, 9);
	b = md4_rotl(b + md4_G(c, d, a) + context->M[15] + 0x5a827999, 13);
	
	// round 3
	a = md4_rotl(a + md4_H(b, c, d) + context->M[0]  + 0x6ed9eba1, 3);
	d = md4_rotl(d + md4_H(a, b, c) + context->M[8]  + 0x6ed9eba1, 9);
	c = md4_rotl(c + md4_H(d, a, b) + context->M[4]  + 0x6ed9eba1, 11);
	b = md4_rotl(b + md4_H(c, d, a) + context->M[12] + 0x6ed9eba1, 15);
	
	a = md4_rotl(a + md4_H(b, c, d) + context->M[2]  + 0x6ed9eba1, 3);
	d = md4_rotl(d + md4_H(a, b, c) + context->M[10] + 0x6ed9eba1, 9);
	c = md4_rotl(c + md4_H(d, a, b) + context->M[6]  + 0x6ed9eba1, 11);
	b = md4_rotl(b + md4_H(c, d, a) + context->M[14] + 0x6ed9eba1, 15);
	
	a = md4_rotl(a + md4_H(b, c, d) + context->M[1]  + 0x6ed9eba1, 3);
	d = md4_rotl(d + md4_H(a, b, c) + context->M[9]  + 0x6ed9eba1, 9);
	c = md4_rotl(c + md4_H(d, a, b) + context->M[5]  + 0x6ed9eba1, 11);
	b = md4_rotl(b + md4_H(c, d, a) + context->M[13] + 0x6ed9eba1, 15);
	
	a = md4_rotl(a + md4_H(b, c, d) + context->M[3]  + 0x6ed9eba1, 3);
	d = md4_rotl(d + md4_H(a, b, c) + context->M[11] + 0x6ed9eba1, 9);
	c = md4_rotl(c + md4_H(d, a, b) + context->M[7]  + 0x6ed9eba1, 11);
	b = md4_rotl(b + md4_H(c, d, a) + context->M[15] + 0x6ed9eba1, 15);
	
	context->A = l2bendian(md4_A + a);
	context->B = l2bendian(md4_B + b);
	context->C = l2bendian(md4_C + c);
	context->D = l2bendian(md4_D + d);
	
}

#endif
