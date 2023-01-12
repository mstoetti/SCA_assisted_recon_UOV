/// @file api_config.h
/// @brief Defining which key format used for api.h/sign.c
///
/// Defining which key format used for api.h/sign.c
///

#ifndef _API_CONFIG_H_
#define _API_CONFIG_H_

#if defined(PQM4)
#define _PQM4_
#endif

// #defined _SUPERCOP_


#if (!defined(_UOV_CLASSIC))&&(!defined(_UOV_CIRCUMZENITHAL))&&(!defined(_UOV_COMPRESSED))
#define _UOV_CLASSIC
//#define _UOV_CIRCUMZENITHAL
//#define _UOV_COMPRESSED
#endif


#if defined _UOV_CLASSIC
#define _SUFFIX " - classic"
#elif defined _UOV_CIRCUMZENITHAL
#define _SUFFIX " - circumzenithal"
#elif defined _UOV_COMPRESSED
#define _SUFFIX " - compressed"
#else
error here
#endif



#endif  // _API_CONFIG_H_
