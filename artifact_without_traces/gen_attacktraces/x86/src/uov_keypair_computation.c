/// @file uov_keypair_computation.c
/// @brief Implementations for functions in uov_keypair_computation.h
///

#include "uov_keypair.h"
#include "uov_keypair_computation.h"

#include "blas_comm.h"
#include "blas.h"
#include "uov_blas.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "utils_malloc.h"


////////////////////////////////////////////////////////////////////////////



// Choosing implementations depends on the macros: _BLAS_SSE_ and _BLAS_AVX2_
//#if defined(_BLAS_SSE_) || defined(_BLAS_AVX2_)
#if 0

#include "uov_keypair_computation_simd.h"
#define calculate_Q_from_F_impl        calculate_Q_from_F_simd
#define calculate_F_from_Q_impl        calculate_F_from_Q_simd
#define calculate_Q_from_F_cyclic_impl calculate_Q_from_F_cyclic_simd

#elif defined(_BLAS_M4F_)
#define calculate_Q_from_F_impl         calculate_Q_from_F_ref
#define czuov_calculate_F_from_Q_impl czuov_calculate_F_from_Q_m4f
#define czuov_calculate_Q_from_F_impl   czuov_calculate_Q_from_F_ref

#else

#define calculate_Q_from_F_ref         calculate_Q_from_F_impl
#define czuov_calculate_F_from_Q_ref   czuov_calculate_F_from_Q_impl
#define czuov_calculate_Q_from_F_ref   czuov_calculate_Q_from_F_impl

#endif


/////////////////////////////////////////////////////////


static
void UpperTrianglize_inplace(unsigned char * bA , unsigned dim, unsigned size_batch )
{
    for(unsigned i=0;i<dim;i++) {
        for(unsigned j=0;j<i;j++) {
            unsigned idx = idx_of_trimat(j,i,dim);
            gf256v_add( bA + idx*size_batch , bA + size_batch*(i*dim+j) , size_batch );
        }
        unsigned idx = idx_of_trimat(i, i, dim);
        memmove( bA + idx*size_batch,  bA + size_batch*(i*dim+i), size_batch*(dim-i));
    }
}



static
void calculate_Q_from_F_ref( pk_t * pk,  const sk_t * Fs , const sk_t * Ts)
{
    unsigned char *tmp_Q2 = pk->pk;                          // needs _O1_BYTE*_O1*_V1
    unsigned char *tmp_Q5 = pk->pk + _O1_BYTE * _V1 * _O1;   // needs _O1_BYTE*_O1*_O1

    #if ((_PUB_M_BYTE) * N_TRIANGLE_TERMS(_PUB_N)) < (_O1_BYTE*_O1*_V1+_O1_BYTE*_O1*_O1)
        error: cannot compute public key in-place
    #endif
/*
    Layer 1
    Computing :
    Q_pk.l1_F1s[i] = F_sk.l1_F1s[i]

    Q_pk.l1_F2s[i] = (F1* T1 + F2) + F1tr * t1
    Q_pk.l1_F5s[i] = UT( T1tr* (F1 * T1 + F2) )
*/

    memcpy( tmp_Q2, Fs->l1_F2 , _O1_BYTE * _V1 * _O1 );
    batch_trimat_madd( tmp_Q2 , Fs->l1_F1 , Ts->t1 , _V1, _V1_BYTE , _O1, _O1_BYTE );    // F1*T1 + F2

    memset( tmp_Q5 , 0 , _O1_BYTE * _O1 * _O1 );   // l1_Q5
    batch_matTr_madd( tmp_Q5 , Ts->t1 , _V1, _V1_BYTE, _O1, tmp_Q2, _O1, _O1_BYTE );  // t1_tr*(F1*T1 + F2)

    UpperTrianglize_inplace(tmp_Q5 , _O1, _O1_BYTE );    // UT( ... )   // Q5
    batch_trimatTr_madd( tmp_Q2, Fs->l1_F1 , Ts->t1 , _V1, _V1_BYTE , _O1, _O1_BYTE );    // Q2

    // Q5
    uint8_t *ptr_pk = pk->pk + _O1_BYTE*(N_TRIANGLE_TERMS(_V1) + _V1*_O1);
    for(unsigned i=_V1;i<_V1+_O1;i++) {

        memcpy(ptr_pk, tmp_Q5, _O1_BYTE*(_V1+_O1-i));
        ptr_pk += _O1_BYTE*(_V1+_O1-i);
        tmp_Q5 += _O1_BYTE*(_V1+_O1-i);

    }

    // Q2
    uint8_t *tmp_pk;
    const unsigned char * idx_Q2;
    for(int i=_V1-1;i >= 0;i--) {
        idx_Q2 = tmp_Q2 + i*_O1*_O1_BYTE;
        tmp_pk = pk->pk + i*_O1*_O1_BYTE + _O1_BYTE*idx_of_trimat(i+1, i+1, _V1);
        memcpy(tmp_pk, idx_Q2, _O1*_O1_BYTE);
    }

    // Q1
    const uint8_t *q1 = Fs->l1_F1;
    for(unsigned i=0;i<_V1;i++) {
        unsigned char *pk_row = pk->pk + i*_O1*_O1_BYTE + _O1_BYTE*idx_of_trimat(i, i, _V1);
        memcpy(pk_row, q1, _O1_BYTE*(_V1-i));
        q1 += _O1_BYTE*(_V1-i);
    }
}


/////////////////////////////////////////////////////



#ifndef _BLAS_M4F_
static
void czuov_calculate_F_from_Q_ref(sk_t * Fs)
{
    // F_sk.l1_F1s[i] = Q_pk.l1_F1s[i]
    // F_sk.l1_F2s[i] = ( Q_pk.l1_F1s[i] + Q_pk.l1_F1s[i].transpose() ) * T_sk.t1 + Q_pk.l1_F2s[i]
    batch_2trimat_madd( Fs->l1_F2 , Fs->l1_F1 , Fs->t1 , _V1, _V1_BYTE , _O1, _O1_BYTE );
}
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////

#include "utils_hash.h"

static
void czuov_calculate_Q_from_F_ref( cpk_t * Qs, const sk_t * Fs , const sk_t * Ts )
{
// Layer 1: Computing Q5

//  Q_pk.l1_F5s[i] = UT( T1tr* (F1 * T1 + F2) )

#define _SIZE_BUFFER_F2 (_O1_BYTE * _V1 * _O1)
    unsigned char _ALIGN_(32) buffer_F2[_SIZE_BUFFER_F2];
    memcpy( buffer_F2 , Fs->l1_F2 , _O1_BYTE * _V1 * _O1 );
    batch_trimat_madd( buffer_F2 , Fs->l1_F1 , Ts->t1 , _V1, _V1_BYTE , _O1, _O1_BYTE );      // F1*T1 + F2

#define _SIZE_BUFFER_F5 (_O1_BYTE * _O1 * _O1)
    unsigned char _ALIGN_(32) buffer_F5[_SIZE_BUFFER_F5];
    memset( buffer_F5 , 0 , _O1_BYTE * _O1 * _O1 );
    batch_matTr_madd( buffer_F5 , Ts->t1 , _V1, _V1_BYTE, _O1, buffer_F2, _O1, _O1_BYTE );  // T1tr*(F1*T1 + F2) , release buffer_F2

    memset( Qs->l1_Q5 , 0 , _O1_BYTE * N_TRIANGLE_TERMS(_O1) );
    UpperTrianglize( Qs->l1_Q5 , buffer_F5 , _O1, _O1_BYTE );                        // UT( ... )   // Q5 , release buffer_F3

    memset( buffer_F2 , 0 , _SIZE_BUFFER_F2 );
    memset( buffer_F5 , 0 , _SIZE_BUFFER_F5 );
}












///////////////////////////////////////////////////////////////////////





void calculate_Q_from_F( pk_t * Qs, const sk_t * Fs , const sk_t * Ts )
{
    calculate_Q_from_F_impl( Qs , Fs , Ts );
}


void czuov_calculate_F_from_Q( sk_t * Fs)
{
    czuov_calculate_F_from_Q_impl( Fs );
}


void czuov_calculate_Q_from_F( cpk_t * Qs, const sk_t * Fs , const sk_t * Ts )
{
    czuov_calculate_Q_from_F_impl( Qs , Fs , Ts );
}
