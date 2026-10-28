
#include <stdio.h>



#include "blas.h"
#include "blas_comm.h"
#include "blas_matrix.h"

#include "utils_prng.h"

#include "utils.h"

#include "string.h"





#include "benchmark.h"


#define LEN  64
#define LEN_2  (LEN/2)

#define TEST_RUN 100




int main()
{
	struct benchmark bm_inv;
	struct benchmark bm_ldu;

	bm_init( &bm_inv );
	bm_init( &bm_ldu );

#if defined(_BLAS_AVX2_)
	struct benchmark bm_sol;
	bm_init( &bm_sol );
#endif

        printf("====== unit test ======\n");
        printf("teting equality of reesults of mattrix-vector multiplication from 2 inverse matrices [%dx%d].\n\n", LEN , LEN );

#if defined(_BLAS_AVX2_)
	char * arch = "avx2";
#elif defined(_BLAS_SSE_)
	char * arch = "ssse3";
#elif defined(_BLAS_UINT64_)
	char * arch = "uint64";
#else
	char * arch = "ref";
#endif
	printf("arch = %s\n", arch );


	printf("\n\n============= setup PRNG ==============\n");

	prng_t _prng;
	prng_t * prng0 = &_prng;
	uint8_t prng_seed[32] = {0};
	prng_set( prng0 , prng_seed , 32 );

	printf("\n\n============= random matrix generation ==============\n");

	uint8_t matA[ LEN*LEN ];
	uint8_t matB[ LEN*LEN ];
	uint8_t matC[ LEN*LEN ];

	uint8_t submat_A[ LEN_2*LEN_2];
	uint8_t submat_B[ LEN_2*LEN_2];
	uint8_t submat_C[ LEN_2*LEN_2];
	uint8_t submat_D[ LEN_2*LEN_2];

	uint8_t vecA[LEN];
	uint8_t vecB[LEN];
	uint8_t vecC[LEN];
	uint8_t vecD[LEN];

	uint8_t vec_sol[LEN];
	uint8_t vec_test[LEN];

	int test_pass = 1;

	int j=0;
	for(;j<TEST_RUN;j++) {
		prng_gen( prng0 , matA , LEN*LEN );
		prng_gen( prng0 , vec_sol , LEN_2 );
		gf16mat_prod( vecA , matA , LEN_2 , LEN , vec_sol );


	int inv1;
BENCHMARK( bm_inv , {
		inv1 = gf16mat_inv( matB , matA , LEN );
		gf16mat_prod( vecB , matB , LEN_2 , LEN , vecA );
} );

	int inv2;
BENCHMARK( bm_ldu , {
		inv2 = gf16mat_LDUinv( submat_B , submat_A , submat_D , submat_C , matA , LEN );
		gf16mat_LDUinv_prod( vecC , submat_B , submat_A , submat_D , submat_C , vecA , LEN_2 );
} );

#if defined(_BLAS_AVX2_) && (64==LEN)
	int inv3;
BENCHMARK( bm_sol , {
		inv3 = gf16mat_solve_linear_eq_64x64_avx2( vec_test , matA , vecA );
} );

		if( inv1 != inv3 ) {
			printf("inv1 != inv3: %d!=%d\n", inv1 , inv3 );
			test_pass = 0;
			break;
		}
#endif

		if( inv1 != inv2 ) {
			printf("inv1 != inv2:  %d!=%d\n", inv1 , inv2 );
			continue;
		}
		if( 0 == inv1 ) {
			printf("singular matrix.\n");
			continue;
		}

		gf256v_add( vec_sol , vecB , LEN_2 );
		if( !gf256v_is_zero( vec_sol , LEN_2 ) ) {
			printf("wrong solution: diff:\n");
			byte_fdump( stdout , "vec_sol: " , vec_sol , LEN_2 ); printf("\n");
			test_pass = 0;
			break;
		}

#if defined(_BLAS_AVX2_) && (64==LEN)
		gf256v_add( vec_test , vecB , LEN_2 );
#if 1
		if( !gf256v_is_zero( vec_test , LEN_2 ) ) {
			printf("wrong solution (gaussian): diff:\n");
			byte_fdump( stdout , "vec_test: " , vec_test , LEN_2 ); printf("\n");
			test_pass = 0;
			break;
		}
#endif
#endif

		memcpy( vecD , vecB , LEN_2 );
		gf256v_add( vecD , vecC , LEN_2 );

		if( !gf256v_is_zero( vecD , LEN_2 ) ){
			test_pass = 0;
			printf("[%d,%d] 2 vector differ:\n", j );
			byte_fdump( stdout , "vecB: " , vecB , LEN_2 ); printf("\n");
			byte_fdump( stdout , "vecC: " , vecC , LEN_2 ); printf("\n");
			byte_fdump( stdout , "diff: " , vecD , LEN_2 ); printf("\n");
			break;
		}
	}

	printf("[%d/%d] test %s.\n\n", j , TEST_RUN , (test_pass)?"PASS":"FAIL" );

	char msg[256];
	bm_dump( msg , 256 , & bm_inv );
	printf("bm_inv: %s\n\n", msg );
	bm_dump( msg , 256 , & bm_ldu );
	printf("bm_ldu: %s\n\n", msg );

#if defined(_BLAS_AVX2_)
	bm_dump( msg , 256 , & bm_sol );
	printf("bm_sol: %s\n\n", msg );
#endif

	return 0;
}

