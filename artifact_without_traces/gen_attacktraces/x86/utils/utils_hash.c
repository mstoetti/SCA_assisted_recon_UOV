/// @file utils_hash.c
/// @brief the adapter for SHA2 families in openssl.
///
///

#include "utils_hash.h"


#include "hash_len_config.h"

#ifndef _HASH_LEN
#define _HASH_LEN (32)
#endif



#include <utils_config.h>


#if defined(_UTILS_SUPERCOP_)

#if 32 == _HASH_LEN
#include "crypto_hash_sha256.h"
#elif 48 == _HASH_LEN
#include "crypto_hash_sha384.h"
#elif 64 == _HASH_LEN
#include "crypto_hash_sha512.h"
#endif

#elif defined(_UTILS_OPENSSL_)

#include "openssl/sha.h"

#elif defined(_UTILS_PQM4_)

#include "sha2.h"

#else

no _UTILS_OEPNSSL_ or _UTILS_SUPERCOP_ or _UTILS_PQM4_

#endif



static inline
int _hash( unsigned char * digest , const unsigned char * m , unsigned long long mlen )
{
#if 32 == _HASH_LEN
#if defined(_UTILS_SUPERCOP_)
	crypto_hash_sha256(digest,m,mlen);
#elif defined(_UTILS_PQM4_)
	sha256(digest,m,mlen);
#else  // defined(_UTILS_OPENSSL_)
	SHA256(m,mlen,digest);
#endif
#elif 48 == _HASH_LEN
#if defined(_UTILS_SUPERCOP_)
	crypto_hash_sha384(digest,m,mlen);
#elif defined(_UTILS_PQM4_)
	sha384(digest,m,mlen);
#else   // defined(_UTILS_OPENSSL_)
	SHA384(m,mlen,digest);
#endif
#elif 64 == _HASH_LEN
#if defined(_UTILS_SUPERCOP_)
	crypto_hash_sha512(digest,m,mlen);
#elif defined(_UTILS_PQM4_)
	sha512(digest,m,mlen);
#else   // defined(_UTILS_OPENSSL_)
	SHA512(m,mlen,digest);
#endif
#else
error: un-supported _HASH_LEN
#endif
	return 0;
}





static inline
int expand_hash( unsigned char * digest , unsigned n_digest , const unsigned char * hash )
{
	if( _HASH_LEN >= n_digest ) {
		for(unsigned i=0;i<n_digest;i++) digest[i] = hash[i];
		return 0;
	} else {
		for(unsigned i=0;i<_HASH_LEN;i++) digest[i] = hash[i];
		n_digest -= _HASH_LEN;
	}

	while( _HASH_LEN <= n_digest ) {
		_hash( digest+_HASH_LEN , digest , _HASH_LEN );

		n_digest -= _HASH_LEN;
		digest += _HASH_LEN;
	}
	unsigned char temp[_HASH_LEN];
	if( n_digest ){
		_hash( temp , digest , _HASH_LEN );
		for(unsigned i=0;i<n_digest;i++) digest[_HASH_LEN+i] = temp[i];
	}
	return 0;
}




int hash_msg( unsigned char * digest , unsigned len_digest , const unsigned char * m , unsigned long long mlen )
{
	unsigned char buf[_HASH_LEN];
	_hash( buf , m , mlen );

	return expand_hash( digest , len_digest , buf );
}


