/*
    HMAC-SHA1 hash functions
*/

#ifndef _HASH_HMACSHA1_H_
#define _HASH_HMACSHA1_H_

/*
    HMAC-SHA1 hashing (Keyed-Hashing for Message Authentication). 
    For algorithm details refer to RFC2104: http://tools.ietf.org/html/rfc2104
*/

/*----------------------------------------------------------------------*/

/* context */
typedef struct
{
    euint8_t         ipad[HASH_SHA1_BLOCK_SIZE];
    euint8_t         opad[HASH_SHA1_BLOCK_SIZE];
    euint8_t         key[HASH_SHA1_BLOCK_SIZE];
    sha1_context_t  sha1_context;

} hmac_sha1_context_t;

/*----------------------------------------------------------------------*/
/*
    NOTE: functions expect digest_out to be of size HASH_SHA1_DIGEST_SIZE
*/

/* hash in binary data */
void hash_hmac_sha1_begin(hmac_sha1_context_t* hmac_sha1_context, const euint8_t* key, euint32_t key_length);
void hash_hmac_sha1_update(hmac_sha1_context_t* hmac_sha1_context, const euint8_t* data, euint32_t data_size);
void hash_hmac_sha1_end(hmac_sha1_context_t* hmac_sha1_context, euint8_t* digest_out);

/* hash data in single run */
int hash_hmac_sha1(const euint8_t* data, euint32_t data_size,
                   const euint8_t* key, euint32_t key_length, euint8_t* digest_out);

/*----------------------------------------------------------------------*/

#endif /* _HASH_HMACSHA1_H_ */

