
#include <stdio.h>

#include "uov_config.h"

#include "uov_keypair.h"

#include "uov.h"

#include "utils_prng.h"

#include "utils.h"

#include "string.h"

#define TEST_RUN 100

//TODO: clean up this file


//#include "benchmark.h"
//struct benchmark bmm;

int main()
{
//	bm_init( &bmm );
	//unsigned char seed[32] = {0};

//         printf("%s\n", _S_NAME );

//         printf("sk size: %lu\n", sizeof(sk_t) );
//         printf("pk size: %lu\n", sizeof(pk_t) );
// //        printf("csk size: %lu\n", sizeof(csk_t) );
// //        printf("cpk size: %lu\n", sizeof(cpk_t) );
//         printf("digest size: %d\n", _PUB_M_BYTE );
//         printf("signature /wo salt size: %d\n\n", _PUB_N_BYTE );
//         printf("signature size: %d\n\n", _SIGNATURE_BYTE );


// 	printf("\n\n============= setup PRNG ==============\n");

	prng_t _prng;
	prng_t * prng0 = &_prng;
	uint8_t prng_seed[32] = {0};
	prng_set( prng0 , prng_seed , 32 );

	// printf("\n\n============= key pair generation ==============\n");

	pk_t _pk;
	sk_t _sk;
	pk_t * pk = &_pk;
	sk_t * sk = &_sk;

	uint8_t sk_seed[LEN_SKSEED] = {0};
	// uint8_t pk_seed[LEN_PKSEED] = {0};

	generate_keypair( pk , sk , sk_seed );

	// printf("\n\n============= salt sign/verify test ==============\n");

	uint8_t  digest1[_HASH_LEN];
	prng_gen( prng0 , digest1,_HASH_LEN);
	// byte_fdump( stdout , "dgst: " , digest1 , _HASH_LEN ); printf("\n");

	uint8_t signature1[_SIGNATURE_BYTE];
	uov_sign( signature1 , sk , digest1 );
	byte_fdump( stdout , "sig : " , signature1 , _SIGNATURE_BYTE ); printf("\n");

	int r = uov_verify( digest1 , signature1 , pk );
	printf("verify: %d (0 is success.)\n", r );

	for(unsigned i=0;i<TEST_RUN;i++) {
		prng_gen( prng0 , digest1,_HASH_LEN);
		uov_sign( signature1 , sk , digest1 );
		r = uov_verify( digest1 , signature1 , pk );

		if( 0 != r ) {
			printf("fail:[%d]\n",i);
			byte_fdump( stdout , "dgst: " , digest1 , _HASH_LEN ); printf("\n");
			byte_fdump( stdout , "sig : " , signature1 , _SIGNATURE_BYTE ); printf("\n");
			printf("verify: %d (0 is success.)\n", r );
			return -1;
		}
	}
	printf("%d pk/sk test success.\n", TEST_RUN);
	printf("\n\n");


/*

//	cpk_t _cpk;
//	csk_t _csk;
//	cpk_t * cpk = &_cpk;
//	csk_t * csk = &_csk;

	printf("\n\n============= cyclic version ==============\n");

	generate_keypair_cyclic( cpk, sk, pk_seed , sk_seed );

	printf("\n\n============= salt sign/verify test ==============\n");

#if 0
	pk_t * pk2 = (pk_t*) malloc( sizeof(pk_t) );
	cpk_to_pk( pk2 , cpk );
	sk_to_pk( pk , sk );
	if( 0 != memcmp(pk,pk2,sizeof(pk_t)) ) {
		printf("2 unqual pks.\n");
	}

#endif

	prng_gen( prng0 , digest1,_HASH_LEN);
	byte_fdump( stdout , "dgst: " , digest1 , _HASH_LEN ); printf("\n");

	uov_sign( signature1 , sk , digest1 );
	byte_fdump( stdout , "sig : " , signature1 , _SIGNATURE_BYTE ); printf("\n");

	r = uov_verify_cyclic( digest1 , signature1 , cpk );
	printf("verify: %d (0 is success.)\n", r );
#if 0
        r = uov_verify( digest1 , signature1 , pk );
	printf("verify(classic pk): %d (0 is success.)\n", r );
        r = uov_verify( digest1 , signature1 , pk2 );
	printf("verify(classic pk2): %d (0 is success.)\n", r );
	free(pk2);
#endif

	for(unsigned i=0;i<TEST_RUN;i++) {
		prng_gen( prng0 , digest1,_HASH_LEN);
		uov_sign( signature1 , sk , digest1 );
		r = uov_verify_cyclic( digest1 , signature1 , cpk );

		if( 0 != r ) {
			printf("fail:[%d]\n",i);
			byte_fdump( stdout , "dgst: " , digest1 , _HASH_LEN ); printf("\n");
			byte_fdump( stdout , "sig : " , signature1 , _SIGNATURE_BYTE ); printf("\n");
			printf("verify: %d (0 is success.)\n", r );
			return -1;
		}
	}
	printf("%d pk/sk test success.\n", TEST_RUN);


	printf("\n\n============= compact cyclic version ==============\n");

	generate_compact_keypair_cyclic( cpk , csk , pk_seed , sk_seed );

	printf("\n\n============= salt sign/verify test ==============\n");

	prng_gen( prng0 , digest1,_HASH_LEN);
	byte_fdump( stdout , "dgst: " , digest1 , _HASH_LEN ); printf("\n");

	uov_sign_cyclic( signature1 , csk , digest1 );
	byte_fdump( stdout , "sig : " , signature1 , _SIGNATURE_BYTE ); printf("\n");

	r = uov_verify_cyclic( digest1 , signature1 , cpk );
	printf("verify: %d (0 is success.)\n", r );

	for(unsigned i=0;i<TEST_RUN;i++) {
		prng_gen( prng0 , digest1,_HASH_LEN);
		uov_sign_cyclic( signature1 , csk , digest1 );
		r = uov_verify_cyclic( digest1 , signature1 , cpk );

		if( 0 != r ) {
			printf("fail:[%d]\n",i);
			byte_fdump( stdout , "dgst: " , digest1 , _HASH_LEN ); printf("\n");
			byte_fdump( stdout , "sig : " , signature1 , _SIGNATURE_BYTE ); printf("\n");
			printf("verify: %d (0 is success.)\n", r );
			return -1;
		}
	}
	printf("%d pk/sk test success.\n", TEST_RUN);
*/


	return 0;
}

