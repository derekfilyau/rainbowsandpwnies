#ifndef md4_HEADER
#define md4_HEADER

/*
	Only use for md4 hashes < ~50 bytes
	Reasonably fast
*/

#include <inttypes.h>
#include <string.h>


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


inline uint32_t md4_rotl (uint32_t x, uint32_t s);
inline uint32_t md4_F (uint32_t x, uint32_t y, uint32_t z);
inline uint32_t md4_G (uint32_t x, uint32_t y, uint32_t z);
inline uint32_t md4_H (uint32_t x, uint32_t y, uint32_t z);
inline uint32_t l2bendian (uint32_t little);
void md4_begin (struct md4_context * context, unsigned char * data, int data_len);
void md4_transform (struct md4_context * context);

#endif
