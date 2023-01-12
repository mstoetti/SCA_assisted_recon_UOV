///  @file  sign.c
///  @brief the implementations for functions in api.h
///
///
#include <stdlib.h>
#include <string.h>

#include "uov_config.h"
#include "uov_keypair.h"
#include "uov.h"

#include "api.h"

#include "utils_hash.h"

#include <utils_prng.h>

#if defined(_UTILS_SUPERCOP_)
#include "crypto_sign.h"
#endif


int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
    unsigned char sk_seed[LEN_SKSEED] = {0};
    randombytes( sk_seed , LEN_SKSEED );

#if defined _UOV_CLASSIC

    int r = generate_keypair( (pk_t*) pk , (sk_t*) sk , sk_seed );

#elif defined _UOV_CIRCUMZENITHAL

    unsigned char pk_seed[LEN_PKSEED] = {0};
    randombytes( pk_seed , LEN_PKSEED );
    int r = generate_keypair_cz( (cpk_t*) pk , (sk_t*) sk , pk_seed , sk_seed );

    for(int i=0;i<LEN_PKSEED;i++) pk_seed[i]=0;
#elif defined _UOV_COMPRESSED

    unsigned char pk_seed[LEN_PKSEED] = {0};
    randombytes( pk_seed , LEN_PKSEED );
    int r = generate_compressed_keypair_cz( (cpk_t*) pk , (csk_t*) sk , pk_seed , sk_seed );

    for(int i=0;i<LEN_PKSEED;i++) pk_seed[i]=0;
#else
error here
#endif
    for(int i=0;i<LEN_SKSEED;i++) sk_seed[i]=0;
    return r;
}





int
#if defined(_PQM4_)
crypto_sign(unsigned char *sm, size_t *smlen, const unsigned char *m, size_t mlen, const unsigned char *sk)
#else
crypto_sign(unsigned char *sm, unsigned long long *smlen, const unsigned char *m, unsigned long long mlen, const unsigned char *sk)
#endif
{
	unsigned char digest[_HASH_LEN];

	hash_msg( digest , _HASH_LEN , m , mlen );

	int r = -1;
#if defined _UOV_CLASSIC

	r = uov_sign( sm + mlen , (const sk_t*)sk , digest );

#elif defined _UOV_CIRCUMZENITHAL

	r = uov_sign( sm + mlen , (const sk_t*)sk , digest );

#elif defined _UOV_COMPRESSED

	r = uov_expand_and_sign( sm + mlen , (const csk_t*)sk , digest );

#else
error here
#endif
	memcpy( sm , m , mlen );
	smlen[0] = mlen + _SIGNATURE_BYTE;

	return r;
}






int
#if defined(_PQM4_)
crypto_sign_open(unsigned char *m, size_t *mlen,const unsigned char *sm, size_t smlen,const unsigned char *pk)
#else
crypto_sign_open(unsigned char *m, unsigned long long *mlen,const unsigned char *sm, unsigned long long smlen,const unsigned char *pk)
#endif
{
	if( _SIGNATURE_BYTE > smlen ) return -1;

	unsigned char digest[_HASH_LEN];
	hash_msg( digest , _HASH_LEN , sm , smlen-_SIGNATURE_BYTE );

	int r = -1;

#if defined _UOV_CLASSIC

	r = uov_verify( digest , sm + smlen-_SIGNATURE_BYTE , (const pk_t *)pk );

#elif defined _UOV_CIRCUMZENITHAL

	r = uov_expand_and_verify( digest , sm + smlen-_SIGNATURE_BYTE , (const cpk_t *)pk );

#elif defined _UOV_COMPRESSED

	r = uov_expand_and_verify( digest , sm + smlen-_SIGNATURE_BYTE , (const cpk_t *)pk );

#else
error here
#endif

	memcpy( m , sm , smlen-_SIGNATURE_BYTE );
	mlen[0] = smlen-_SIGNATURE_BYTE;

	return r;
}

