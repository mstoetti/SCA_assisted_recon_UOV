/// @file uov_keypair.h
/// @brief Formats of key pairs and functions for generating key pairs.
/// Formats of key pairs and functions for generating key pairs.
///

#ifndef _uov_KEYPAIR_H_
#define _uov_KEYPAIR_H_


#include "uov_config.h"


#define N_TRIANGLE_TERMS(n_var) ((n_var)*((n_var)+1)/2)


#ifdef  __cplusplus
extern  "C" {
#endif

/// alignment 1 for sturct
#pragma pack(push,1)


/// @brief public key for classic uov
///
///  public key for classic uov
///
typedef
struct uov_publickey {
    unsigned char pk[(_PUB_M_BYTE) * N_TRIANGLE_TERMS(_PUB_N)];
} pk_t;


/// @brief secret key for classic uov
///
/// secret key for classic uov
///
typedef
struct uov_secretkey {
    unsigned char sk_seed[LEN_SKSEED];   ///< seed for generating secret key

    unsigned char t1[_V1_BYTE*_O1];   ///< T map

    unsigned char l1_F1[_O1_BYTE * N_TRIANGLE_TERMS(_V1)];  ///< part of C-map, F1, Layer1
    unsigned char l1_F2[_O1_BYTE * _V1*_O1];                ///< part of C-map, F2, Layer1

} sk_t;



/// @brief compressed public key for cz uov
///
///  compressed public key for cz uov
///
typedef
struct uov_compressed_publickey {
    unsigned char pk_seed[LEN_PKSEED];                      ///< seed for generating l1_Q1,l1_Q2
    unsigned char l1_Q5[_O1_BYTE * N_TRIANGLE_TERMS(_O1)];  ///< Q5, layer1
} cpk_t;



/// @brief compressed secret key for cz uov
///
/// compressed secret key for cz uov
///
typedef
struct uov_compressed_secretkey {
    unsigned char pk_seed[LEN_PKSEED];   ///< seed for generating a part of public key.
    unsigned char sk_seed[LEN_SKSEED];   ///< seed for generating a part of secret key.
} csk_t;





/// restores alignment
#pragma pack(pop)


/////////////////////////////////////






#ifdef  __cplusplus
}
#endif

#endif //  _uov_KEYPAIR_H_
