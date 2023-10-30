/*
    Fowler–Noll–Vo hash function
    
    NOTE: Simple hash functions for strings, non cryptographic

    More info:
    http://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
    http://www.isthe.com/chongo/tech/comp/fnv/index.html

*/

#ifndef _HASH_FNV_H_
#define _HASH_FNV_H_

/*----------------------------------------------------------------------*/

/* 32 bit hash function */
unsigned long hash_fnv32(const unsigned char* buffer, unsigned long buffer_size);

#ifdef _ELIBC_64_BITS_SUPPORT

/* 64 bit hash function */
euint64_t hash_fnv64(const unsigned char* buffer, int buffer_size);

#endif /* _ELIBC_64_BITS_SUPPORT */

/*----------------------------------------------------------------------*/

#endif /* _HASH_FNV_H_ */

