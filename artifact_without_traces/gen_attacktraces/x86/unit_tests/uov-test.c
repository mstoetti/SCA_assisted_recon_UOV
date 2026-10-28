
#include <stdio.h>
#include "uov_config.h"
#include "uov_keypair.h"
#include "uov.h"
#include "utils_prng.h"
#include "utils.h"
#include "string.h"

#define TEST_RUN 100

int main(int argc, char *argv[])
{

    int seed = 0;
    if (argc > 1)
    {
        char *a = argv[1];
        seed = atoi(a);
    }

    // printf("\n\n============= setup PRNG ==============\n");

    prng_t _prng;
    prng_t *prng0 = &_prng;
    uint8_t prng_seed[32] = {seed};
    prng_set(prng0, prng_seed, 32);

    // printf("\n\n============= key pair generation ==============\n");

    pk_t _pk;
    sk_t _sk;
    pk_t *pk = &_pk;
    sk_t *sk = &_sk;

    uint8_t sk_seed[LEN_SKSEED] = {0};

    char filename[16] = "";
    sprintf(filename, "../pk_sig_%d.txt", seed);

    FILE *f;

    f = fopen(filename, "a");
    if (!f)
    {
        printf("Unable to open the target file\n");
        exit(1);
    }

    generate_keypair(pk, sk, sk_seed);

    fprintf(f, "unsigned char pk[%d] = {", (_PUB_M_BYTE)*N_TRIANGLE_TERMS(_PUB_N));
    for (unsigned int i = 0; i < (_PUB_M_BYTE)*N_TRIANGLE_TERMS(_PUB_N); i++)
    {
        fprintf(f, "0x%02X, ", (unsigned int)(_pk.pk[i] & 0xFF));
    }
    fprintf(f, "};\n\n");

    // printf("\n\n============= salt sign/verify test ==============\n");

    uint8_t digest1[_HASH_LEN];
    prng_gen(prng0, digest1, _HASH_LEN);

    uint8_t signature1[_SIGNATURE_BYTE];
    uov_sign(signature1, sk, digest1);

    fprintf(f, "\n\nunsigned char signature1[%d] = {", _SIGNATURE_BYTE);
    for (unsigned int i = 0; i < _SIGNATURE_BYTE; i++)
    {
        fprintf(f, "0x%02X, ", (unsigned int)(signature1[i] & 0xFF));
    }

    fprintf(f, "};\n\n");

    fclose(f);


    return 0;
}
