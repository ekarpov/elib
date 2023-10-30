/*
    Fowler–Noll–Vo hash function
*/

#include "../elib_config.h"
#include "hash_fnv.h"

/*----------------------------------------------------------------------*/

/* 32 bit hash constants */
#define HASH_FNV_32_INIT ((unsigned long)0x811c9dc5)    
#define HASH_FNV_32_MULT ((unsigned long)0x01000193)    

unsigned long hash_fnv32(const unsigned char* buffer, unsigned long buffer_size)
{
    /* initial hash value */
    unsigned long hash_val = HASH_FNV_32_INIT;
    unsigned long idx;

    /* loop over buffer */
    for(idx = 0; idx < buffer_size; ++idx)
    {
        /* xor bottom 8 bits with current byte */
        hash_val ^= buffer[idx];

        /* multiply by magic prime number */
        hash_val *= HASH_FNV_32_MULT;
    }

    return hash_val;
}

/*----------------------------------------------------------------------*/

#ifdef _ELIBC_64_BITS_SUPPORT

/* 64 bit hash constants */
#define HASH_FNV_64_INIT ((euint64_t)0xcbf29ce484222325ULL)    
#define HASH_FNV_64_MULT ((euint64_t)0x100000001b3ULL)    

euint64_t hash_fnv64(const unsigned char* buffer, int buffer_size)
{
    /* initial hash value */
    euint64_t hash_val = HASH_FNV_64_INIT;
    euint64_t idx;

    /* loop over buffer */
    for(idx = 0; idx < buffer_size; ++idx)
    {
        /* xor bottom 8 bits with current byte */
        hash_val ^= buffer[idx];

        /* multiply by magic prime number */
        hash_val *= HASH_FNV_64_MULT;
    }

    return hash_val;
}

#endif /* _ELIBC_64_BITS_SUPPORT */

/*----------------------------------------------------------------------*/

