
#include <stdio.h>

#include "uov_config.h"

#include "uov_keypair.h"

#include "uov.h"

#include "utils_prng.h"

#include "utils.h"

#include "string.h"

#define TEST_RUN 100





//#include "benchmark.h"
//struct benchmark bmm;

int main()
{
//	bm_init( &bmm );
	//unsigned char seed[32] = {0};

        printf("%s - CZ version\n", _S_NAME );

        printf("sk size: %lu\n", sizeof(sk_t) );
        printf("pk size: %lu\n", sizeof(pk_t) );
        printf("csk size: %lu\n", sizeof(csk_t) );
        printf("cpk size: %lu\n", sizeof(cpk_t) );
        printf("digest size: %d\n", _PUB_M_BYTE );
        printf("signature /wo salt size: %d\n\n", _PUB_N_BYTE );
        printf("signature size: %d\n\n", _SIGNATURE_BYTE );


	printf("\n\n============= setup PRNG ==============\n");

	prng_t _prng;
	prng_t * prng0 = &_prng;
	uint8_t prng_seed[32] = {0};
	prng_set( prng0 , prng_seed , 32 );

	printf("\n\n============= key pair generation ==============\n");

	sk_t _sk;
	sk_t * sk = &_sk;

	cpk_t _cpk;
	cpk_t * cpk = &_cpk;

	uint8_t sk_seed[LEN_SKSEED] = {0};
	uint8_t pk_seed[LEN_PKSEED] = {0};

	//generate_keypair( pk , sk , sk_seed );
	generate_keypair_cz( cpk, sk, pk_seed , sk_seed );
	//generate_compressed_keypair_cz( cpk, csk, pk_seed , sk_seed );

	printf("\n\n============= salt sign/verify test ==============\n");

	uint8_t  digest1[_HASH_LEN];
	prng_gen( prng0 , digest1,_HASH_LEN);
	byte_fdump( stdout , "dgst: " , digest1 , _HASH_LEN ); printf("\n");

	uint8_t signature1[_SIGNATURE_BYTE];
	uov_sign( signature1 , sk , digest1 );
	//uov_expand_and_sign( signature1 , csk , digest1 );
	byte_fdump( stdout , "sig : " , signature1 , _SIGNATURE_BYTE ); printf("\n");

	//int r = uov_verify( digest1 , signature1 , pk );
	int r = uov_expand_and_verify( digest1 , signature1 , cpk );
	printf("verify: %d (0 is success.)\n", r );

	for(unsigned i=0;i<TEST_RUN;i++) {
		prng_gen( prng0 , digest1,_HASH_LEN);
		uov_sign( signature1 , sk , digest1 );
		//uov_expand_and_sign( signature1 , csk , digest1 );
		//r = uov_verify( digest1 , signature1 , pk );
		r = uov_expand_and_verify( digest1 , signature1 , cpk );

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


	return 0;
}

