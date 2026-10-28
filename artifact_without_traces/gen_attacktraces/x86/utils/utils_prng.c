/// @file utils_prng.c
/// @brief The implementation of PRNG related functions.
///

#include "utils_prng.h"

#include <stdlib.h>
#include <string.h>


#include "utils_config.h"



#if defined(_UTILS_OPENSSL_)
#include <openssl/rand.h>

void randombytes(unsigned char *x, unsigned long long xlen)
{
  RAND_bytes(x,xlen);
}

#endif



////////////////////////////////// crypto_stream_aes256ctr //////////////////////////////



#if defined(_UTILS_SUPERCOP_)

#include "crypto_stream_aes256ctr.h"

// crypto_stream_aes256ctr(unsigned char *x,unsigned xlen, const unsigned char *nonce, const unsigned char *key)
#define aes256ctr  crypto_stream_aes256ctr

#elif defined(_UTILS_PQM4_)

#include "aes.h"

static
int aes256ctr( unsigned char *out, unsigned outlen, const unsigned char *n, const unsigned char *k )
{
  aes256ctx ctx;
  aes256_ctr_keyexp( &ctx, k);
  aes256_ctr(out,outlen,n,&ctx);
  memset(&ctx,0,sizeof(aes256ctx));
  return 0;
}

#elif defined(_UTILS_OPENSSL_)

#include <openssl/evp.h>

static
int aes256ctr( unsigned char *out, unsigned outlen, const unsigned char *n, const unsigned char *k )
{
  unsigned char in[RNG_STREAM_UNIT];
  unsigned inlen = sizeof(in);
  memset(in, 0, inlen );

  EVP_CIPHER_CTX *ctx;
  int ok;
  int outl = 0;

  ctx = EVP_CIPHER_CTX_new();
  if (!ctx) return -111;

  ok = EVP_EncryptInit_ex(ctx,EVP_aes_256_ctr(),0,k,n);
  if (ok == 1) ok = EVP_CIPHER_CTX_set_padding(ctx, 0);
  if (ok == 1) ok = EVP_EncryptUpdate(ctx, out, &outl, in, inlen);
  if (ok == 1) ok = EVP_EncryptFinal_ex(ctx, out, &outl);

  EVP_CIPHER_CTX_free(ctx);
  if( (unsigned int) outl != outlen ) ok = 0;
  return ok == 1 ? 0 : -111;

}

#else

error : no _UTILS_OPENSSL_ or _UTILS_SUPERCOP_ or _UTILS_PQM4_

#endif   ///


///////////////////////////////////////  crypto_rng  ////////////////////////////////////////


static const unsigned char nonce[AES256CTR_NONCELEN] = {0};

static
int crypto_rng( unsigned char *r, /* random output */
        unsigned char *n, /* new key */
  const unsigned char *g  /* old key */
)
{
  unsigned char x[RNG_STREAM_UNIT];
  aes256ctr(x,sizeof(x),nonce,g);
  memcpy(n,x, AES256CTR_KEYLEN );
  memcpy(r,x + AES256CTR_KEYLEN,sizeof(x)-AES256CTR_KEYLEN);
  return 0;
}



///////////////////////////////////////  fastrandombytes //////////////////////////




// int hash_msg( unsigned char * digest , unsigned len_digest , const unsigned char * m , unsigned long long mlen );
#include "utils_hash.h"



int prng_set(prng_t *ctx, const void *prng_seed, unsigned long prng_seedlen)
{
    hash_msg( ctx->key , AES256CTR_KEYLEN, (const unsigned char *)prng_seed, prng_seedlen);
    ctx->used = RNG_OUTPUTLEN;

    return 0;
}



//void randombytes(unsigned char *x,unsigned long long xlen)
int prng_gen(prng_t *ctx, unsigned char *out, unsigned long outlen)
{
  unsigned char *x = out;
  unsigned long long xlen = outlen;

  while (xlen > 0) {
    unsigned long long ready;

    if (ctx->used == RNG_OUTPUTLEN) {
      while (xlen > RNG_OUTPUTLEN) {
        crypto_rng(x,ctx->key,ctx->key);
        x += RNG_OUTPUTLEN;
        xlen -= RNG_OUTPUTLEN;
      }
      if (xlen == 0) return outlen;

      crypto_rng(ctx->buf,ctx->key,ctx->key);
      ctx->used = 0;
    }

    ready = RNG_OUTPUTLEN - ctx->used;
    if (xlen <= ready) ready = xlen;
    memcpy(x,ctx->buf + ctx->used,ready);
    memset(ctx->buf + ctx->used,0,ready);
    x += ready;
    xlen -= ready;
    ctx->used += ready;
  }

  return outlen;

}
















#if defined( _DEBUG_ )

void randombytes(unsigned char *x, unsigned long long xlen);
{
  while(xlen--) { *x++=rand()&0xff; }
}

int prng_set(prng_t *ctx, const void *prng_seed, unsigned long prng_seedlen)
{
    memset(ctx, 0, sizeof (prng_t));

    hash_msg(ctx->buf, _HASH_LEN , (const unsigned char *)prng_seed, prng_seedlen);

    return 0;
}

int prng_gen(prng_t *ctx, unsigned char *out, unsigned long outlen) {

   while( outlen ) {
      if( _HASH_LEN == ctx->used ) { hash_msg(ctx->buf, _HASH_LEN , ctx->buf, _HASH_LEN); ctx->used = 0; }
      out[0] = ctx->buf[ctx->used];
      out++;
      ctx->used++;
      outlen--;
   }
   return 0;
}

#endif
