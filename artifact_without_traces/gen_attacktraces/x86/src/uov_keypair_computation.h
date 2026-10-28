/// @file uov_keypair_computation.h
/// @brief Functions for calculating pk/sk while generating keys.
///
/// Defining an internal structure of public key.
/// Functions for calculating pk/sk for key generation.
///

#ifndef _uov_KEYPAIR_COMP_H_
#define _uov_KEYPAIR_COMP_H_

#include "uov_keypair.h"

//IF_CRYPTO_CORE:define CRYPTO_NAMESPACE

#ifdef  __cplusplus
extern  "C" {
#endif




///
/// @brief Computing public key from secret key, classic uov
///
/// @param[out] Qs       - the public key: l1_Q1, l1_Q2, l1_Q5
/// @param[in]  Fs       - parts of the secret key: l1_F1, l1_F2
/// @param[in]  Ts       - parts of the secret key: T1
///
void calculate_Q_from_F( pk_t * Qs, const sk_t * Fs , const sk_t * Ts );

///
/// @brief Computing parts of the sk from parts of pk and sk
///
/// @param[out] Fs       - secret key
///
void czuov_calculate_F_from_Q( sk_t * Fs);

///
/// @brief Computing parts of the pk from the secret key
///
/// @param[out] Qs       - parts of the pk: l1_Q5
/// @param[in]  Fs       - parts of the sk: l1_F1, l1_F2
/// @param[in]  Ts       - parts of the sk: T1
///
void czuov_calculate_Q_from_F( cpk_t * Qs, const sk_t * Fs , const sk_t * Ts );





#ifdef  __cplusplus
}
#endif

#endif  // _uov_KEYPAIR_COMP_H_

