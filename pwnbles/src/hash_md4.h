#ifndef hash_md4_HEADER
#define hash_md4_HEADER

#include "r.h"
#include "md4.h"

int hash_md4_plaintext_to_index (step_t * step);
int hash_md4_hash_to_index(unsigned char * hash);

#endif
