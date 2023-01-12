///
///  @file api.h
///
///  Created by Bassham, Lawrence E (Fed) on 9/6/17.
///  Copyright © 2017 Bassham, Lawrence E (Fed). All rights reserved.
///
///
///
///   @brief This is a sample 'api.h' for use 'sign.c'
///

#ifndef api_h
#define api_h


#include "uov_config.h"
#include "uov_keypair.h"

#include "api_config.h"


//  Set these three values apropriately for your algorithm


#if defined _UOV_CLASSIC

#define CRYPTO_SECRETKEYBYTES sizeof(sk_t)
#define CRYPTO_PUBLICKEYBYTES sizeof(pk_t)

#elif defined _UOV_CIRCUMZENITHAL

#define CRYPTO_SECRETKEYBYTES sizeof(sk_t)
#define CRYPTO_PUBLICKEYBYTES sizeof(cpk_t)

#elif defined _UOV_COMPRESSED

#define CRYPTO_SECRETKEYBYTES sizeof(csk_t)
#define CRYPTO_PUBLICKEYBYTES sizeof(cpk_t)

#else
error here
#endif


#define CRYPTO_BYTES _SIGNATURE_BYTE

// Change the algorithm name
#define CRYPTO_ALGNAME _S_NAME _SUFFIX


#if defined(_PQM4_)

#include "stddef.h"  // for size_t

#ifdef  __cplusplus
extern  "C" {
#endif


int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

int
crypto_sign(unsigned char *sm, size_t *smlen,
            const unsigned char *m, size_t mlen,
            const unsigned char *sk);

int
crypto_sign_open(unsigned char *m, size_t *mlen,
                 const unsigned char *sm, size_t smlen,
                 const unsigned char *pk);

#ifdef  __cplusplus
}
#endif


#elif defined(_SUPERCOP_)

#include "crypto_sign.h"

#else

#ifdef  __cplusplus
extern  "C" {
#endif


int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

int
crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
            const unsigned char *sk);

int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk);

#ifdef  __cplusplus
}
#endif

#endif  // defined(_PQM4_)

#endif /* api_h */
