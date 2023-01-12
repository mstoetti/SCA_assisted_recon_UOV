/// @file uov.h
/// @brief APIs for uov.
///

#ifndef _UOV_H_
#define _UOV_H_

#include "uov_config.h"
#include "uov_keypair.h"

#include <stdint.h>

#ifdef  __cplusplus
extern  "C" {
#endif



////////////////////////  Key pair generation  ///////////////////////////////////


/// Classic UOV  /////////////////////////////////

///
/// @brief Key pair generation for classic uov.
///
/// @param[out] pk        - the public key.
/// @param[out] sk        - the secret key.
/// @param[in]  sk_seed   - seed for generating the secret key.
/// @return 0 for success. -1 otherwise.
///
int generate_keypair( pk_t * pk, sk_t* sk, const unsigned char *sk_seed );



////////////////////////////////////

///
/// @brief Secret key Generation for classic uov.
///
/// @param[out] sk        - the secret key.
/// @param[in]  sk_seed   - seed for generating the secret key.
///
void generate_secretkey( sk_t* sk, const unsigned char *sk_seed );

///
/// @brief Convert secret key to public key for classic uov.
///
/// @param[out] pk        - the public key.
/// @param[in] sk         - the secret key.
/// @return 0 for success. -1 otherwise.
///
int sk_to_pk( pk_t * pk , const sk_t* sk );



/// CZ UOV  /////////////////////////////


///
/// @brief Generate key pairs for cz uov.
///
/// @param[out] pk        - the compressed public key.
/// @param[out] sk        - the secret key.
/// @param[in]  pk_seed   - seed for generating parts of public key.
/// @param[in]  sk_seed   - seed for generating secret key.
/// @return 0 for success. -1 otherwise.
///
int generate_keypair_cz( cpk_t * pk, sk_t* sk, const unsigned char *pk_seed , const unsigned char *sk_seed );

///
/// @brief Generate compressed key pairs for cz uov.
///
/// @param[out] pk        - the compressed public key.
/// @param[out] sk        - the compressed secret key.
/// @param[in]  pk_seed   - seed for generating parts of the public key.
/// @param[in]  sk_seed   - seed for generating the secret key.
/// @return 0 for success. -1 otherwise.
///
int generate_compressed_keypair_cz( cpk_t * pk, csk_t* sk, const unsigned char *pk_seed , const unsigned char *sk_seed );





/// Key conversion : compressed key pair -> classic key pair //////////////////////////////


///
/// @brief converting formats of public keys : from compressed to classic public key
///
/// @param[out] pk       - the classic public key.
/// @param[in]  cpk      - the cycli  public key.
///
int expand_pk( pk_t * pk , const cpk_t * cpk );



///
/// @brief Generate secret key for cz uov.
///
/// @param[out] sk        - the secret key.
/// @param[in]  pk_seed   - seed for generating parts of the pbulic key.
/// @param[in]  sk_seed   - seed for generating the secret key.
/// @return 0 for success. -1 otherwise.
///
int expand_sk( sk_t* sk, const unsigned char *pk_seed , const unsigned char *sk_seed );






////////////////////////  Public map  ///////////////////////////////////


///
/// @brief Public-key evaluattion
///
/// @param[out] z         - results of the evluation of public polynomials at the w.
/// @param[in]  pk        - the classic public key.
/// @param[in]  w         - the input vector w.
///
void uov_publicmap( unsigned char * z , const unsigned char *pk , const unsigned char * w );



///
/// @brief Public-key evaluattion
///
/// @param[out] z         - results of the evluation of public polynomials at the w.
/// @param[in]  pk        - the compressed public key.
/// @param[in]  w         - the input vector w.
///
void uov_publicmap_cz( unsigned char * z , const cpk_t *pk , const unsigned char * w );








///////////////////////////// Sign and Verify ////////////////////////////////



///
/// @brief Signing function for classic secret key.
///
/// @param[out] signature - the signature.
/// @param[in]  sk        - the secret key.
/// @param[in]  digest    - the digest.
/// @return 0 for success. -1 otherwise.
///
int uov_sign( uint8_t * signature , const sk_t * sk , const uint8_t * digest );

///
/// @brief Verifying function.
///
/// @param[in]  digest    - the digest.
/// @param[in]  signature - the signature.
/// @param[in]  pk        - the public key.
/// @return 0 for successful verified. -1 for failed verification.
///
int uov_verify( const uint8_t * digest , const uint8_t * signature , const pk_t * pk );





///
/// @brief Signing function for compressed secret key.
///
/// @param[out] signature - the signature.
/// @param[in]  sk        - the compressed secret key.
/// @param[in]  digest    - the digest.
/// @return 0 for success. -1 otherwise.
///
int uov_expand_and_sign( uint8_t * signature , const csk_t * sk , const uint8_t * digest );

///
/// @brief Verifying function for compressed public keys.
///
/// @param[in]  digest    - the digest.
/// @param[in]  signature - the signature.
/// @param[in]  pk        - the public key of cyclic rainbow.
/// @return 0 for successful verified. -1 for failed verification.
///
int uov_expand_and_verify( const uint8_t * digest , const uint8_t * signature , const cpk_t * pk );






#ifdef  __cplusplus
}
#endif


#endif // _UOV_H_
