/// @file uov_config.h
/// @brief Defining the parameters of the Rainbow and the corresponding constants.
///
/// Defining one of the parameters _UOV16_96_64 ,or ??
/// for (GF16,96,64) in this file.
///
///

#ifndef _UOV_CONFIG_H_
#define _UOV_CONFIG_H_

#define _LDU_DECOMPOSE_

/// the defined parameter
#if (!defined(_UOV16_96_64))&&(!defined(_UOV256_112_44))&&(!defined(_UOV256_184_72))&&(!defined(_UOV256_244_96))

// default:
//#define _UOV16_96_64
#define _UOV256_112_44

#endif

#if defined _UOV16_96_64
#define _USE_GF16
#define _GFSIZE 16
#define _V1 96
#define _O1 64
#define _O2 0
#define _HASH_LEN 32

#elif defined _UOV256_112_44
#define _GFSIZE 256
#define _V1 42
#define _O1 28
#define _O2 0
#define _HASH_LEN 32

// #elif defined _UOV256_112_44
// #define _GFSIZE 256
// #define _V1 68
// #define _O1 44
// #define _O2 0
// #define _HASH_LEN 32

#elif defined _UOV256_184_72
#define _GFSIZE 256
#define _V1 112
#define _O1 72
#define _O2 0
#define _HASH_LEN 48

#elif defined _UOV256_244_96
#define _GFSIZE 256
#define _V1 148
#define _O1 96
#define _O2 0
#define _HASH_LEN 64

#else
error: has to define a parameter.
#endif


#define _V2 ((_V1)+(_O1))


#define STR1(x) #x
#define THE_NAME(gf,n,m) "UOV(" STR1(gf) "," STR1(n) "," STR1(m) ")"
#define _S_NAME THE_NAME(_GFSIZE,_V1+_O1,_O1)


/// size of N, in # of gf elements.
#define _PUB_N  (_V1+_O1)

/// size of M, in # gf elements.
#define _PUB_M  (_O1)


/// size of variables, in # bytes.


#ifdef _USE_GF16
// GF16
#define _V1_BYTE (_V1/2)
#define _O1_BYTE (_O1/2)
#define _PUB_N_BYTE  (_PUB_N/2)
#define _PUB_M_BYTE  (_PUB_M/2)

#else
// GF256
#define _V1_BYTE (_V1)
#define _O1_BYTE (_O1)
#define _PUB_N_BYTE  (_PUB_N)
#define _PUB_M_BYTE  (_PUB_M)

#endif


/// length of seed for public key, in # bytes
#define LEN_PKSEED 32

/// length of seed for secret key, in # bytes
#define LEN_SKSEED 32

/// length of salt for a signature, in # bytes
#define _SALT_BYTE 16

/// length of a signature
#define _SIGNATURE_BYTE (_PUB_N_BYTE + _SALT_BYTE )






#endif //  _UOV_CONFIG_H_
