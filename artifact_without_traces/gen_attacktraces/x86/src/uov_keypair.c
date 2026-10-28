/// @file uov_keypair.c
/// @brief implementations of functions in uov_keypair.h
///
#include "uov_keypair.h"
#include "uov.h"
#include "uov_keypair_computation.h"

#include "blas_comm.h"
#include "blas.h"
#include "uov_blas.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>


/////////////////////////////////////////////////////////////////


#include "utils_prng.h"
#include "utils_malloc.h"

#if 96 < _V1
#define _MALLOC_
#endif


static void generate_T( unsigned char * t , prng_t * prng0 )
{
    sk_t * _sk;
    prng_gen( prng0 , t , sizeof(_sk->t1) );
}

static void generate_F1F2( unsigned char * f1f2, prng_t * prng0 )
{
    prng_gen( prng0 , f1f2 , _O1_BYTE * N_TRIANGLE_TERMS(_V1) + _O1_BYTE * _V1*_O1);
}


static void generate_F1_F2( unsigned char * f1_ptr, unsigned char *f2_ptr, prng_t * prng0 )
{
    for(unsigned i=0;i<_V1;i++) {
        prng_gen( prng0 , f1_ptr , _O1_BYTE*(_V1-i));
        f1_ptr += _O1_BYTE*(_V1-i);
        prng_gen( prng0 , f2_ptr , _O1_BYTE*_O1);
        f2_ptr += _O1_BYTE*_O1;
    }
}





/////////////////////////////////////////////////////////


#if 0
// For debug
static void dump_sk( const sk_t* sk )
{
  byte_fdump(stdout, "sk->sk_seed:" , sk->sk_seed , LEN_SKSEED ); printf("\n");
  byte_fdump(stdout, "sk->t1 head:" , sk->t1 , _O2_BYTE ); printf("\n");
  byte_fdump(stdout, "sk->l1_F1 head:" , sk->l1_F1 , _O2_BYTE ); printf("\n");
  byte_fdump(stdout, "sk->l1_F2 head:" , sk->l1_F2 , _O2_BYTE ); printf("\n");
}
#endif



///////////////////  Classic uov  //////////////////////////////////



void generate_secretkey( sk_t* sk, const unsigned char *sk_seed )
{
    memcpy( sk->sk_seed , sk_seed , LEN_SKSEED );

    // set up prng
    prng_t prng0;
    prng_set( &prng0 , sk_seed , LEN_SKSEED );

    // generating secret key with prng.
    generate_T( sk->t1 , &prng0 );
    generate_F1_F2( sk->l1_F1 , sk->l1_F2 , &prng0 );

    // clean prng
    memset( &prng0 , 0 , sizeof(prng_t) );
}


int sk_to_pk( pk_t * rpk , const sk_t* sk )
{
    calculate_Q_from_F(rpk, sk, sk);
    return 0;
}


int generate_keypair( pk_t * rpk, sk_t* sk, const unsigned char *sk_seed )
{
    generate_secretkey( sk , sk_seed );

    return sk_to_pk( rpk , sk );
}




/////////////////////   cz uov   //////////////////////////////////


int expand_pk( pk_t * rpk, const cpk_t * cpk )
{
    prng_t prng0;
    prng_set(&prng0 , cpk->pk_seed , LEN_PKSEED);

    generate_F1F2(rpk->pk, &prng0);
    memcpy( rpk->pk + _O1_BYTE * N_TRIANGLE_TERMS(_V1) + _O1_BYTE * _V1*_O1, cpk->l1_Q5 , sizeof(cpk->l1_Q5) );

    return 0;
}




int expand_sk( sk_t* sk, const unsigned char *pk_seed , const unsigned char *sk_seed )
{
    memcpy( sk->sk_seed , sk_seed , LEN_SKSEED );

    // prng for sk
    prng_t _prng;
    prng_t * prng0 = &_prng;
    prng_set( prng0 , sk_seed , LEN_SKSEED );

    generate_T( sk->t1 , prng0 );

    // prng for pk
    prng_set( prng0 , pk_seed , LEN_PKSEED );
    generate_F1_F2( sk->l1_F1 , sk->l1_F2 , prng0 );

    // calcuate the parts of sk according to pk.
    czuov_calculate_F_from_Q( sk );
    return 0;
}



////////////////////////////////////////////////////////////////////////////////////




int generate_keypair_cz( cpk_t * pk, sk_t* sk, const unsigned char *pk_seed , const unsigned char *sk_seed )
{
    memcpy( pk->pk_seed , pk_seed , LEN_PKSEED );
    memcpy( sk->sk_seed , sk_seed , LEN_SKSEED );

    // prng for sk
    prng_t prng;
    prng_t * prng0 = &prng;
    prng_set( prng0 , sk_seed , LEN_SKSEED );
    generate_T( sk->t1 , prng0 );   // S,T:  only a part of sk

    // prng for pk
    prng_set( prng0 , pk_seed , LEN_PKSEED );
    generate_F1_F2( sk->l1_F1 , sk->l1_F2 , prng0 );

    czuov_calculate_F_from_Q( sk );     // calcuate the rest parts of secret key from Qs and S,T
    czuov_calculate_Q_from_F( pk, sk , sk );      // calculate the rest parts of public key: l1_Q5

    return 0;
}



int generate_compressed_keypair_cz( cpk_t * pk, csk_t* rsk, const unsigned char *pk_seed , const unsigned char *sk_seed )
{
    memcpy( rsk->pk_seed , pk_seed , LEN_PKSEED );
    memcpy( rsk->sk_seed , sk_seed , LEN_SKSEED );

#if defined(_MALLOC_)
    sk_t * sk = malloc(sizeof(sk_t));
    if(NULL==sk) return -1;
#else
    sk_t _sk;
    sk_t * sk = &_sk;
#endif
    int r = generate_keypair_cz( pk , sk , pk_seed , sk_seed );

    memset( sk , 0 , sizeof(sk_t) ); // clean
#if defined(_MALLOC_)
    free(sk);
#endif
    return r;
}



