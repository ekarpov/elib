/*
    SHA1 hash functions
*/

#include "../elib_config.h"

#include "hash_sha1.h"

/*----------------------------------------------------------------------*/

/* helper types */
typedef union {

    euint8_t    c[64];
    euint32_t   l[16];

} SHA1_WORKBLOCK;

/*----------------------------------------------------------------------*/

/* helper macro */
#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#ifdef ELIBC_BIG_ENDIAN
#define blk0(i) block->l[i]
#else
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00)|(rol(block->l[i],8)&0x00FF00FF))
#endif /* ELIBC_BIG_ENDIAN */

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15]^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

/*----------------------------------------------------------------------*/

/* 
    Hash a single 512-bit block. This is the core of the algorithm. 
     - state size is expected to be five 32 bits values 
     - buffer size is expected to be 64 bytes 
*/
ELIBC_FORCE_INLINE void _sha1_transform(euint32_t* state, const euint8_t* buffer)
{
    euint32_t a, b, c, d, e;
    SHA1_WORKBLOCK* block;

    block = (SHA1_WORKBLOCK*)buffer;

    /* Copy state context to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

    /* Add the working vars back into context state */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

/*----------------------------------------------------------------------*/

/* hash in binary data */
void hash_sha1_begin(sha1_context_t* sha1_context)
{
    EASSERT(sha1_context);
    if(sha1_context == 0) return;

    /* SHA1 initialization constants */
    sha1_context->state[0] = 0x67452301;
    sha1_context->state[1] = 0xEFCDAB89;
    sha1_context->state[2] = 0x98BADCFE;
    sha1_context->state[3] = 0x10325476;
    sha1_context->state[4] = 0xC3D2E1F0;

    sha1_context->count[0] = sha1_context->count[1] = 0;
}

void hash_sha1_update(sha1_context_t* sha1_context, const euint8_t* data, euint32_t data_size)
{
    size_t idx_i, idx_j;

    EASSERT(sha1_context);
    EASSERT(data);
    EASSERT(data_size);
    if(sha1_context == 0 || data == 0 || data_size == 0) return;

    /* init counts */
    idx_j = (sha1_context->count[0] >> 3) & 63;
    if ((sha1_context->count[0] += data_size << 3) < (data_size << 3)) sha1_context->count[1]++;
    sha1_context->count[1] += (data_size >> 29);

    /* update hash */
    if ((idx_j + data_size) > 63) 
    {
        ememcpy(&sha1_context->buffer[idx_j], data, (idx_i = 64-idx_j));
        _sha1_transform(sha1_context->state, sha1_context->buffer);
        for ( ; idx_i + 63 < data_size; idx_i += 64) 
        {
            _sha1_transform(sha1_context->state, data + idx_i);
        }
        idx_j = 0;
    } else  
    {
        idx_i = 0;
    }

    /* copy rest of the data */
    ememcpy(&sha1_context->buffer[idx_j], &data[idx_i], data_size - idx_i);
}

void hash_sha1_end(sha1_context_t* sha1_context, euint8_t* digest_out)
{
    size_t      idx_i;
    euint8_t    finalcount[8];

    EASSERT(sha1_context);
    EASSERT(digest_out);
    if(sha1_context == 0 || digest_out == 0) return;

    for (idx_i = 0; idx_i < 8; idx_i++) {
        finalcount[idx_i] = (euint8_t)((sha1_context->count[(idx_i >= 4 ? 0 : 1)]
         >> ((3-(idx_i & 3)) * 8) ) & 255);  /* Endian independent */
    }

    hash_sha1_update(sha1_context, (euint8_t *)"\200", 1);
    while ((sha1_context->count[0] & 504) != 448) {
        hash_sha1_update(sha1_context, (euint8_t *)"\0", 1);
    }

    hash_sha1_update(sha1_context, finalcount, 8);  /* Should cause a _sha1_transform() */
    for (idx_i = 0; idx_i < HASH_SHA1_DIGEST_SIZE; idx_i++) {
        digest_out[idx_i] = (euint8_t)
         ((sha1_context->state[idx_i>>2] >> ((3-(idx_i & 3)) * 8) ) & 255);
    }
}

/* hash data in single run */
int hash_sha1(const euint8_t* data, euint32_t data_size, euint8_t* digest_out)
{
    sha1_context_t sha1_context;

    EASSERT(data);
    EASSERT(data_size);
    EASSERT(digest_out);
    if(data_size == 0 || data == 0 || digest_out == 0) return ELIBC_ERROR_ARGUMENT;

    /* hash data with temp context */
    hash_sha1_begin(&sha1_context);
    hash_sha1_update(&sha1_context, data, data_size);
    hash_sha1_end(&sha1_context, digest_out);

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/

