
#include <stdio.h>



#include "blas.h"
#include "blas_comm.h"
#include "blas_matrix.h"

#include "utils_prng.h"

#include "utils.h"

#include "string.h"






//#include "benchmark.h"
//struct benchmark bmm;

#define LEN  32
#define TEST_RUN 5

int main()
{
//	bm_init( &bmm );


        printf("====== unit test ======\n");
        printf("mat[%dx%d] * mat_inv -> I\n\n", LEN , LEN );


	printf("\n\n============= setup PRNG ==============\n");

	prng_t _prng;
	prng_t * prng0 = &_prng;
	uint8_t prng_seed[32] = {0};
	prng_set( prng0 , prng_seed , 32 );

	printf("\n\n============= random matrix generation ==============\n");

	uint8_t matA[ LEN*LEN ];
	uint8_t matB[ LEN*LEN ];
	uint8_t matC[ LEN*LEN ];

	uint8_t mat_corr[ LEN*LEN ];
	gf256v_set_zero( mat_corr , sizeof(mat_corr) );
	for(int i=0;i<LEN;i++) mat_corr[i*LEN+i] = 1;
	

for(int j=0;j<TEST_RUN;j++) {
	prng_gen( prng0 , matA , LEN*LEN );
	//gf256v_set_zero( matA , sizeof(matA) );
	//for(int k=0;k<LEN;k++) matA[k*LEN+k] = 1;

	//for(int i=0;i<LEN;i++) {
	//	byte_fdump( stdout , "matA: " , &matA[i*LEN/2] , LEN/2 ); printf("\n");
	//}

	int inv = gf256mat_inv( matB , matA , LEN );
	printf("inv: %d\n", inv );
	//gf256v_set_zero( matB , sizeof(matB) );
	//for(int k=0;k<LEN;k++) matB[k*LEN+k] = 1;

	//for(int i=0;i<LEN;i++) {
	//	byte_fdump( stdout , "matB: " , &matB[i*LEN/2] , LEN/2 ); printf("\n");
	//}
	

	gf256mat_mul( matC , matA , LEN , LEN , matB , LEN );

	gf256v_add( matC , mat_corr , sizeof(matC) );
	if( !gf256v_is_zero( matC , sizeof(matC)) )
	for(int i=0;i<LEN;i++) {
		byte_fdump( stdout , "matC: " , &matC[i*LEN/2] , LEN/2 ); printf("\n");
	}
}
	printf("if no error message -> test pass.\n\n");

	return 0;
}

