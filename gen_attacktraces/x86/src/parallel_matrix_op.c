///  @file parallel_matrix_op.c
///  @brief the standard implementations for functions in parallel_matrix_op.h
///
///  the standard implementations for functions in parallel_matrix_op.h
///

#include "blas_comm.h"
#include "blas.h"
#include <stdio.h>
#include <stdlib.h>

#include "parallel_matrix_op.h"

////////////////    Section: triangle matrix <-> rectangle matrix   ///////////////////////////////////


void UpperTrianglize( unsigned char * btriC , const unsigned char * bA , unsigned Awidth, unsigned size_batch )
{
    unsigned char * runningC = btriC;
    unsigned Aheight = Awidth;
    for(unsigned i=0;i<Aheight;i++) {
        for(unsigned j=0;j<i;j++) {
            unsigned idx = idx_of_trimat(j,i,Aheight);
            gf256v_add( btriC + idx*size_batch , bA + size_batch*(i*Awidth+j) , size_batch );
        }
        gf256v_add( runningC , bA + size_batch*(i*Awidth+i) , size_batch*(Aheight-i) );
        runningC += size_batch*(Aheight-i);
    }
}




/////////////////  Section: matrix multiplications  ///////////////////////////////



void batch_trimat_madd_gf16( unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned Bheight, unsigned size_Bcolvec , unsigned Bwidth, unsigned size_batch )
{
    unsigned Awidth = Bheight;
    unsigned Aheight = Awidth;
    for(unsigned i=0;i<Aheight;i++) {
        for(unsigned j=0;j<Bwidth;j++) {
            for(unsigned k=0;k<Bheight;k++) {
                if(k<i) continue;
                gf16v_madd( bC , & btriA[ (k-i)*size_batch ] , gf16v_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
        btriA += (Aheight-i)*size_batch;
    }
}

void batch_trimat_madd_gf256( unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned Bheight, unsigned size_Bcolvec , unsigned Bwidth, unsigned size_batch )
{
    unsigned Awidth = Bheight;
    unsigned Aheight = Awidth;
    for(unsigned i=0;i<Aheight;i++) {
        for(unsigned j=0;j<Bwidth;j++) {
            for(unsigned k=0;k<Bheight;k++) {
                if(k<i) continue;
                gf256v_madd( bC , & btriA[ (k-i)*size_batch ] , gf256v_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
        btriA += (Aheight-i)*size_batch;
    }
}





void batch_trimatTr_madd_gf16( unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned Bheight, unsigned size_Bcolvec , unsigned Bwidth, unsigned size_batch )
{
    unsigned Aheight = Bheight;
    for(unsigned i=0;i<Aheight;i++) {
        for(unsigned j=0;j<Bwidth;j++) {
            for(unsigned k=0;k<Bheight;k++) {
                if(i<k) continue;
                gf16v_madd( bC , & btriA[ size_batch*(idx_of_trimat(k,i,Aheight)) ] , gf16v_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}

void batch_trimatTr_madd_gf256( unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned Bheight, unsigned size_Bcolvec , unsigned Bwidth, unsigned size_batch )
{
    unsigned Aheight = Bheight;
    for(unsigned i=0;i<Aheight;i++) {
        for(unsigned j=0;j<Bwidth;j++) {
            for(unsigned k=0;k<Bheight;k++) {
                if(i<k) continue;
                gf256v_madd( bC , & btriA[ size_batch*(idx_of_trimat(k,i,Aheight)) ] , gf256v_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}




void batch_2trimat_madd_gf16( unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned Bheight, unsigned size_Bcolvec , unsigned Bwidth, unsigned size_batch )
{
    unsigned Aheight = Bheight;
    for(unsigned i=0;i<Aheight;i++) {
        for(unsigned j=0;j<Bwidth;j++) {
            for(unsigned k=0;k<Bheight;k++) {
                if(i==k) continue;
                gf16v_madd( bC , & btriA[ size_batch*(idx_of_2trimat(i,k,Aheight)) ] , gf16v_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}

void batch_2trimat_madd_gf256( unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned Bheight, unsigned size_Bcolvec , unsigned Bwidth, unsigned size_batch )
{
    unsigned Aheight = Bheight;
    for(unsigned i=0;i<Aheight;i++) {
        for(unsigned j=0;j<Bwidth;j++) {
            for(unsigned k=0;k<Bheight;k++) {
                if(i==k) continue;
                gf256v_madd( bC , & btriA[ size_batch*(idx_of_2trimat(i,k,Aheight)) ] , gf256v_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}




void batch_matTr_madd_gf16( unsigned char * bC , const unsigned char* A_to_tr , unsigned Aheight, unsigned size_Acolvec, unsigned Awidth,
        const unsigned char* bB, unsigned Bwidth, unsigned size_batch )
{
    unsigned Atr_height = Awidth;
    unsigned Atr_width  = Aheight;
    for(unsigned i=0;i<Atr_height;i++) {
        for(unsigned j=0;j<Atr_width;j++) {
            gf16v_madd( bC , & bB[ j*Bwidth*size_batch ] , gf16v_get_ele( &A_to_tr[size_Acolvec*i] , j ) , size_batch*Bwidth );
        }
        bC += size_batch*Bwidth;
    }
}

void batch_matTr_madd_gf256( unsigned char * bC , const unsigned char* A_to_tr , unsigned Aheight, unsigned size_Acolvec, unsigned Awidth,
        const unsigned char* bB, unsigned Bwidth, unsigned size_batch )
{
    unsigned Atr_height = Awidth;
    unsigned Atr_width  = Aheight;
    for(unsigned i=0;i<Atr_height;i++) {
        for(unsigned j=0;j<Atr_width;j++) {
            gf256v_madd( bC , & bB[ j*Bwidth*size_batch ] , gf256v_get_ele( &A_to_tr[size_Acolvec*i] , j ) , size_batch*Bwidth );
        }
        bC += size_batch*Bwidth;
    }
}




void batch_bmatTr_madd_gf16( unsigned char *bC , const unsigned char *bA_to_tr, unsigned Awidth_before_tr,
        const unsigned char *B, unsigned Bheight, unsigned size_Bcolvec, unsigned Bwidth, unsigned size_batch )
{
    const unsigned char *bA = bA_to_tr;
    unsigned Aheight = Awidth_before_tr;
    for(unsigned i=0;i<Aheight;i++) {
        for(unsigned j=0;j<Bwidth;j++) {
            for(unsigned k=0;k<Bheight;k++) {
                gf16v_madd( bC , & bA[ size_batch*(i+k*Aheight) ] , gf16v_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}

void batch_bmatTr_madd_gf256( unsigned char *bC , const unsigned char *bA_to_tr, unsigned Awidth_before_tr,
        const unsigned char *B, unsigned Bheight, unsigned size_Bcolvec, unsigned Bwidth, unsigned size_batch )
{
    const unsigned char *bA = bA_to_tr;
    unsigned Aheight = Awidth_before_tr;
    for(unsigned i=0;i<Aheight;i++) {
        for(unsigned j=0;j<Bwidth;j++) {
            for(unsigned k=0;k<Bheight;k++) {
                gf256v_madd( bC , & bA[ size_batch*(i+k*Aheight) ] , gf256v_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}





void batch_mat_madd_gf16( unsigned char * bC , const unsigned char* bA , unsigned Aheight,
        const unsigned char* B , unsigned Bheight, unsigned size_Bcolvec , unsigned Bwidth, unsigned size_batch )
{
    unsigned Awidth = Bheight;
    for(unsigned i=0;i<Aheight;i++) {
        for(unsigned j=0;j<Bwidth;j++) {
            for(unsigned k=0;k<Bheight;k++) {
                gf16v_madd( bC , & bA[ k*size_batch ] , gf16v_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
        bA += (Awidth)*size_batch;
    }
}

void batch_mat_madd_gf256( unsigned char * bC , const unsigned char* bA , unsigned Aheight,
        const unsigned char* B , unsigned Bheight, unsigned size_Bcolvec , unsigned Bwidth, unsigned size_batch )
{
    unsigned Awidth = Bheight;
    for(unsigned i=0;i<Aheight;i++) {
        for(unsigned j=0;j<Bwidth;j++) {
            for(unsigned k=0;k<Bheight;k++) {
                gf256v_madd( bC , & bA[ k*size_batch ] , gf256v_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
        bA += (Awidth)*size_batch;
    }
}



//IF_CRYPTO_CORE:EOF

////////////////////  Section: "quadratric" matrix evaluation  ///////////////////////////////



void batch_quad_trimat_eval_gf16( unsigned char * y, const unsigned char * trimat, const unsigned char * x, unsigned dim , unsigned size_batch )
{
#if defined(_BLAS_AVX2_)
    batch_quad_trimat_eval_gf16_avx2(y,trimat,x,dim,size_batch);
#elif defined(_BLAS_SSE_)
    batch_quad_trimat_eval_gf16_sse(y,trimat,x,dim,size_batch);
#else
///
///    assert( dim <= 128 );
///    assert( size_batch <= 128 );
    unsigned char tmp[256];

    unsigned char _x[256];
    for(unsigned i=0;i<dim;i++) _x[i] = gf16v_get_ele( x , i );

    gf256v_set_zero( y , size_batch );
    for(unsigned i=0;i<dim;i++) {
        gf256v_set_zero( tmp , size_batch );
        for(unsigned j=i;j<dim;j++) {
           gf16v_madd( tmp , trimat , _x[j] , size_batch );
           trimat += size_batch;
        }
        gf16v_madd( y , tmp , _x[i] , size_batch );
    }
#endif
}

static void print_values( unsigned char * y, const unsigned char * trimat, const unsigned char * x, unsigned dim , unsigned size_batch )
{
    (void) y;
    (void) trimat;
    (void) x;
    (void) dim;
    (void) size_batch;
    // #elif defined _UOV256_112_44
    // #define _GFSIZE 256
    // #define _V1 68
    // #define _O1 44
    // #define _O2 0
    // #define _HASH_LEN 32
    // #define N_TRIANGLE_TERMS(n_var) ((n_var)*((n_var)+1)/2)
    //
    // y := uint8_t r_l1_F1[_O1_BYTE] = {0};
    // trimat := unsigned char l1_F1[_O1_BYTE * N_TRIANGLE_TERMS(_V1)];
    // x := uint8_t vinegar[_V1_BYTE];
    // dim := #define _V1 68
    // size_batch := #define _O1_BYTE (_O1)

    // printf("unsigned char y[%d] = {", size_batch);
    // for(unsigned int i=0; i<size_batch; i++)
    // {
    //     printf("0x%02X, ", (unsigned int)(y[i] & 0xFF));
    // }
    // printf("}");

    printf("\n\nunsigned char trimat[%d] = {", ((dim)*((dim)+1)/2)*size_batch);
    for(unsigned int i=0; i<((dim)*((dim)+1)/2)*size_batch; i++)
    {
        printf("0x%02X, ", (unsigned int)(trimat[i] & 0xFF));
    }
    printf("};");

    printf("\n\nunsigned char x[%d] = {", dim);
    for(unsigned int i=0; i<dim; i++)
    {
        printf("0x%02X, ", (unsigned int)(x[i] & 0xFF));
    }
    printf("};");    

    fflush(stdout);
    // exit(0);

}

#if defined PRINTRESULT
static void print_result( unsigned char * y, unsigned size_batch )
{
    (void) y;
    (void) size_batch;

    printf("\n\nunsigned char y[%d] = {", size_batch);
    for(unsigned int i=0; i<size_batch; i++)
    {
        printf("0x%02X, ", (unsigned int)(y[i] & 0xFF));
    }
    printf("}; ");
  

    fflush(stdout);

}
#endif

void batch_quad_trimat_eval_gf256( unsigned char * y, const unsigned char * trimat, const unsigned char * x, unsigned dim , unsigned size_batch )
{
#if defined(_BLAS_AVX2_)
    batch_quad_trimat_eval_gf256_avx2(y,trimat,x,dim,size_batch);
#elif defined(_BLAS_SSE_)
    batch_quad_trimat_eval_gf256_sse(y,trimat,x,dim,size_batch);
#else
///
///    assert( dim <= 256 );
///    assert( size_batch <= 256 );

    print_values(y, trimat, x, dim, size_batch);

    unsigned char tmp[256];
    unsigned char _x[256];
    for(unsigned i=0;i<dim;i++) _x[i] = gf256v_get_ele( x , i );

    gf256v_set_zero( y , size_batch );
    for(unsigned i=0;i<dim;i++) {
        gf256v_set_zero( tmp , size_batch );
        for(unsigned j=i;j<dim;j++) {
           gf256v_madd( tmp , trimat , _x[j] , size_batch);
           trimat += size_batch;              
        }
        gf256v_madd( y , tmp , _x[i] , size_batch );
    }

#if defined PRINTRESULT
    print_result(y, size_batch);
#endif

#endif
}





