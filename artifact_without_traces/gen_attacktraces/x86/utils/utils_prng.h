/// @file utils_prng.h
/// @brief the interface for adapting PRNG functions.
///
///

#ifndef _UTILS_PRNG_H_
#define _UTILS_PRNG_H_


#ifdef  __cplusplus
extern  "C" {
#endif



#include "utils_config.h"



#if defined(_UTILS_SUPERCOP_)||defined(_UTILS_PQM4_)
#include "randombytes.h"
#else  // defined(_UTILS_OPENSSL_)
void randombytes(unsigned char *x, unsigned long long xlen);
#endif



#if defined(_UTILS_OPENSSL_)||defined(_UTILS_SUPERCOP_)||defined(_UTILS_PQM4_)

#define AES256CTR_KEYLEN   32
#define AES256CTR_NONCELEN 16
#define RNG_STREAM_UNIT    768
#define RNG_OUTPUTLEN      (RNG_STREAM_UNIT-AES256CTR_KEYLEN)

typedef struct {
    unsigned char   key[AES256CTR_KEYLEN];
    unsigned char   buf[RNG_OUTPUTLEN];
    unsigned used;
} prng_t;

#elif defined( _DEBUG_ )

#include "hash_utils.h"

typedef
struct prng_context {
    unsigned char buf[_HASH_LEN];
    unsigned used;
} prng_t;

#endif




int prng_set(prng_t *ctx, const void *prng_seed, unsigned long prng_seedlen);

int prng_gen(prng_t *ctx, unsigned char *out, unsigned long outlen);




#ifdef  __cplusplus
}
#endif



#endif // _UTILS_PRNG_H_


