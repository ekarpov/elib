/*
    SHA1 hash functions
*/

#ifndef _HASH_SHA1_H_
#define _HASH_SHA1_H_

/*
    Adapted implementation from: http://svn.ghostscript.com/jbig2dec/trunk/sha1.c
*/

/*----------------------------------------------------------------------*/

/* constants */
#define HASH_SHA1_BLOCK_SIZE                    64
#define HASH_SHA1_DIGEST_SIZE                   20

/*----------------------------------------------------------------------*/

/* context */
typedef struct
{
    euint32_t   state[5];
    euint32_t   count[2];
    euint8_t    buffer[64];

} sha1_context_t;

/*----------------------------------------------------------------------*/

/*
    NOTE: functions expect digest_out to be of size HASH_SHA1_DIGEST_SIZE
*/

/* hash in binary data */
void hash_sha1_begin(sha1_context_t* sha1_context);
void hash_sha1_update(sha1_context_t* sha1_context, const euint8_t* data, euint32_t data_size);
void hash_sha1_end(sha1_context_t* sha1_context, euint8_t* digest_out);

/* hash data in single run */
int hash_sha1(const euint8_t* data, euint32_t data_size, euint8_t* digest_out);

/*----------------------------------------------------------------------*/

#endif /* _HASH_SHA1_H_ */


