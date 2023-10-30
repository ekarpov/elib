/*
    HMAC-SHA1 hash functions
*/

#include "../elib_config.h"

#include "hash_sha1.h"
#include "hash_hmacsha1.h"

/*----------------------------------------------------------------------*/
/*
    NOTE: functions expect digest_out to be of size HASH_SHA1_DIGEST_SIZE
*/

/* hash in binary data */
void hash_hmac_sha1_begin(hmac_sha1_context_t* hmac_sha1_context, const euint8_t* key, euint32_t key_length)
{
    int idx;

    EASSERT(hmac_sha1_context);
    EASSERT(key);
    EASSERT(key_length);
    if(hmac_sha1_context == 0 || key == 0 || key_length == 0) return;

    /* init ipad and opad */
    ememset(hmac_sha1_context->ipad, 0x36, sizeof(hmac_sha1_context->ipad));
    ememset(hmac_sha1_context->opad, 0x5c, sizeof(hmac_sha1_context->opad));

    /* init key with zeros */
    ememset(hmac_sha1_context->key, 0x00, sizeof(hmac_sha1_context->key));

    /* check key length */
    if(key_length > HASH_SHA1_BLOCK_SIZE)
    {
        /* has key */
        hash_sha1_begin(&hmac_sha1_context->sha1_context);
        hash_sha1_update(&hmac_sha1_context->sha1_context, key, key_length);
        hash_sha1_end(&hmac_sha1_context->sha1_context, hmac_sha1_context->key);

        /* update key size */
        key_length = HASH_SHA1_DIGEST_SIZE;

    } else
    {
        /* copy key */
        ememcpy(hmac_sha1_context->key, key, key_length);
    }

    /* XOR key */
    for(idx = 0; idx < HASH_SHA1_BLOCK_SIZE; ++idx)
    {
        hmac_sha1_context->ipad[idx] ^= hmac_sha1_context->key[idx];
        hmac_sha1_context->opad[idx] ^= hmac_sha1_context->key[idx];
    }

    /* init sha1 context */
    hash_sha1_begin(&hmac_sha1_context->sha1_context);

    /* hash in ipad */
    hash_sha1_update(&hmac_sha1_context->sha1_context, hmac_sha1_context->ipad, sizeof(hmac_sha1_context->ipad));
}

void hash_hmac_sha1_update(hmac_sha1_context_t* hmac_sha1_context, const euint8_t* data, euint32_t data_size)
{
    EASSERT(hmac_sha1_context);
    EASSERT(data);
    EASSERT(data_size);
    if(hmac_sha1_context == 0 || data == 0 || data_size == 0) return;

    /* hash in data */
    hash_sha1_update(&hmac_sha1_context->sha1_context, data, data_size);
}

void hash_hmac_sha1_end(hmac_sha1_context_t* hmac_sha1_context, euint8_t* digest_out)
{
    euint8_t tmp_digest[HASH_SHA1_DIGEST_SIZE];

    EASSERT(hmac_sha1_context);
    EASSERT(digest_out);
    if(hmac_sha1_context == 0 || digest_out == 0) return;

    /* compute (ipad + data) hash */
    hash_sha1_end(&hmac_sha1_context->sha1_context, tmp_digest);

    /* now hash (opad + temp digest) */
    hash_sha1_begin(&hmac_sha1_context->sha1_context);
    hash_sha1_update(&hmac_sha1_context->sha1_context, hmac_sha1_context->opad, sizeof(hmac_sha1_context->opad));
    hash_sha1_update(&hmac_sha1_context->sha1_context, tmp_digest, sizeof(tmp_digest));
    hash_sha1_end(&hmac_sha1_context->sha1_context, digest_out);
}

/* hash data in single run */
int hash_hmac_sha1(const euint8_t* data, euint32_t data_size,
                   const euint8_t* key, euint32_t key_length, euint8_t* digest_out)
{
    hmac_sha1_context_t hmac_sha1_context;

    EASSERT(data);
    EASSERT(data_size);
    EASSERT(key);
    EASSERT(key_length);
    EASSERT(digest_out);
    if(data_size == 0 || data == 0 || key == 0 || key_length == 0 || digest_out == 0) return ELIBC_ERROR_ARGUMENT;

    /* hash data with temp context */
    hash_hmac_sha1_begin(&hmac_sha1_context, key, key_length);
    hash_hmac_sha1_update(&hmac_sha1_context, data, data_size);
    hash_hmac_sha1_end(&hmac_sha1_context, digest_out);

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/

