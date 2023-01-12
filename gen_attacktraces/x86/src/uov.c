/// @file rainbow.c
/// @brief The standard implementations for functions in rainbow.h
///
#include "uov_config.h"

#include "uov_keypair.h"

#include "uov.h"

#include "blas.h"

#include "uov_blas.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "utils_prng.h"
#include "utils_hash.h"
#include "utils_malloc.h"

#include "api_config.h"


#define MAX_ATTEMPT_FRMAT  128


/////////////////////////////



#define _MAX_O  _O1
#define _MAX_O_BYTE  _O1_BYTE




int uov_sign( uint8_t * signature , const sk_t * sk , const uint8_t * _digest )
{
    // allocate temporary storage.
    uint8_t mat_l1[_O1*_O1_BYTE];


#if defined(_LDU_DECOMPOSE_)
    uint8_t submat_A[(_O1/2)*(_O1_BYTE/2)];
    uint8_t submat_B[(_O1/2)*(_O1_BYTE/2)];
    uint8_t submat_C[(_O1/2)*(_O1_BYTE/2)];
    uint8_t submat_D[(_O1/2)*(_O1_BYTE/2)];
#endif

    // initialize seed of PRNG
    uint8_t prng_preseed[LEN_SKSEED+_HASH_LEN];
    uint8_t prng_seed[_HASH_LEN];
    memcpy( prng_preseed , sk->sk_seed , LEN_SKSEED );
    memcpy( prng_preseed + LEN_SKSEED , _digest , _HASH_LEN );                        // prng_preseed = sk_seed || digest
    hash_msg( prng_seed , _HASH_LEN , prng_preseed , _HASH_LEN+LEN_SKSEED );
    memset( prng_preseed , 0 , LEN_SKSEED+_HASH_LEN );                                // clean

    // setup PRNG
    prng_t prng_sign;
    prng_set( &prng_sign , prng_seed , _HASH_LEN );                                   // seed = H( sk_seed || digest )
    memset( prng_seed , 0 , _HASH_LEN );                                              // clean

#if defined(_MUL_WITH_MULTAB_)
    uint8_t multabs[(_V1)*32] __attribute__((aligned(32)));
#endif
    // roll vinegars.
    uint8_t vinegar[_V1_BYTE];
    unsigned n_attempt = 0;
    unsigned l1_succ = 0;
    while( !l1_succ ) {
        if( MAX_ATTEMPT_FRMAT <= n_attempt ) break;
        prng_gen( &prng_sign , vinegar , _V1_BYTE );                       // generating vinegars

#if defined(_MUL_WITH_MULTAB_)
        gfv_generate_multabs( multabs , vinegar , _V1 );
        gfmat_prod_multab( mat_l1 , sk->l1_F2 , _O1*_O1_BYTE , _V1 , multabs );
#else
        gfmat_prod( mat_l1 , sk->l1_F2 , _O1*_O1_BYTE , _V1 , vinegar );   // generating the linear equations for layer 1
#endif

#if defined(_LDU_DECOMPOSE_)
#if _GFSIZE == 256
        l1_succ = gf256mat_LDUinv( submat_B , submat_A , submat_D , submat_C , mat_l1 , _O1 );  // check if the linear equation solvable
#elif _GFSIZE == 16
        l1_succ = gf16mat_LDUinv( submat_B , submat_A , submat_D , submat_C , mat_l1 , _O1 );  // check if the linear equation solvable
#else
error -- _GFSIZE
#endif
#else
        l1_succ = gfmat_inv( mat_l1 , mat_l1 , _O1 );         // check if the linear equation solvable
#endif
        n_attempt ++;
    }
    // Given the vinegars, pre-compute variables needed for layer 2
    uint8_t r_l1_F1[_O1_BYTE] = {0};
#if defined(_MUL_WITH_MULTAB_)
    batch_quad_trimat_eval( r_l1_F1, sk->l1_F1, multabs, _V1, _O1_BYTE );
#else
    batch_quad_trimat_eval( r_l1_F1, sk->l1_F1, vinegar, _V1, _O1_BYTE );
#endif
    // Some local variables.
    uint8_t y[_PUB_N_BYTE];
    uint8_t x_o1[_O1_BYTE];

    uint8_t digest_salt[_HASH_LEN + _SALT_BYTE];
    memcpy( digest_salt , _digest , _HASH_LEN );
    uint8_t * salt = digest_salt + _HASH_LEN;

    do {
        // The computation:  H(digest||salt)  -->   y  --C-map-->   x   --T-->   w
        prng_gen( &prng_sign , salt , _SALT_BYTE );                        // generate the salt
        hash_msg( y , _PUB_M_BYTE , digest_salt , _HASH_LEN+_SALT_BYTE ); // H(digest||salt)

        // Central Map:
        // layer 1: calculate x_o1
        gf256v_add( y , r_l1_F1 , _O1_BYTE );
#if defined(_LDU_DECOMPOSE_)
#if _GFSIZE == 256
        gf256mat_LDUinv_prod( x_o1 , submat_B , submat_A , submat_D , submat_C , y , _O1_BYTE );
#elif _GFSIZE == 16
        gf16mat_LDUinv_prod( x_o1 , submat_B , submat_A , submat_D , submat_C , y , _O1_BYTE );
#else
error -- _GFSIZE
#endif
#else
        gfmat_prod( x_o1 , mat_l1, _O1_BYTE , _O1 , y );
#endif
    } while(0);

    //  w = T^-1 * x
    uint8_t w[_PUB_N_BYTE];
    // identity part of T.
    memcpy( w , vinegar , _V1_BYTE );
    memcpy( w + _V1_BYTE , x_o1 , _O1_BYTE );

    // Computing the t1 part.
    gfmat_prod(y, sk->t1, _V1_BYTE , _O1 , x_o1 );
    gf256v_add(w, y, _V1_BYTE );

    // return: copy w and salt to the signature.
    memset( signature , 0 , _SIGNATURE_BYTE );  // set the output 0
    gf256v_add( signature , w , _PUB_N_BYTE );
    gf256v_add( signature + _PUB_N_BYTE , salt , _SALT_BYTE );

    // clean
    memset( mat_l1 , 0 , _O1*_O1_BYTE );
    memset( &prng_sign , 0 , sizeof(prng_t) );
    memset( vinegar , 0 , _V1_BYTE );
    memset( r_l1_F1 , 0 , _O1_BYTE );
    memset( y , 0 , _PUB_N_BYTE );
    memset( x_o1 , 0 , _O1_BYTE );

    return 0;
}


static
int _uov_verify( const uint8_t * digest , const uint8_t * salt , const unsigned char * digest_ck )
{
    unsigned char correct[_PUB_M_BYTE];
    unsigned char digest_salt[_HASH_LEN + _SALT_BYTE];
    memcpy( digest_salt , digest , _HASH_LEN );
    memcpy( digest_salt+_HASH_LEN , salt , _SALT_BYTE );
    hash_msg( correct , _PUB_M_BYTE , digest_salt , _HASH_LEN+_SALT_BYTE );  // H( digest || salt )

    // check consistency.
    unsigned char cc = 0;
    for(unsigned i=0;i<_PUB_M_BYTE;i++) {
        cc |= (digest_ck[i]^correct[i]);
    }
    return (0==cc)? 0: -1;
}


int uov_verify( const uint8_t * digest , const uint8_t * signature , const pk_t * pk )
{
    unsigned char digest_ck[_PUB_M_BYTE];
    uov_publicmap( digest_ck , pk->pk , signature );

    return _uov_verify( digest , signature+_PUB_N_BYTE , digest_ck );
}



///////////////  cz uov  ///////////////////////////


int uov_expand_and_sign( uint8_t * signature , const csk_t * csk , const uint8_t * digest )
{
    sk_t _sk;
    sk_t * sk = &_sk;
    expand_sk( sk, csk->pk_seed , csk->sk_seed );   // generating classic secret key.

    int r = uov_sign( signature , sk , digest );
    memset( sk , 0 , sizeof(sk_t) );  // clean
    return r;
}

int uov_expand_and_verify( const uint8_t * digest , const uint8_t * signature , const cpk_t * cpk )
{
#if !defined(_SAVE_MEMORY_)
    pk_t _pk;
    pk_t * pk = &_pk;
    expand_pk( pk , cpk );

    return uov_verify( digest, signature , pk );
#else
    unsigned char digest_ck[_PUB_M_BYTE];
    uov_publicmap_cz( digest_ck , cpk , signature );

    return _uov_verify( digest , signature+_PUB_N_BYTE , digest_ck );
#endif
}



