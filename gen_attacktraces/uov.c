/*
    This file is part of the ChipWhisperer Example Targets
    Copyright (C) 2012-2017 NewAE Technology Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hal.h"
#include "simpleserial.h"
#include "uov.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define gf256v_add _gf256v_add_u32
#define gf256v_madd _gf256v_madd_u32

extern unsigned char trimat[25284];
extern unsigned char x[42];

extern uint32_t trigger;


void putString(const unsigned char *string, const unsigned int length)
{
    for(int i=0; i<length; i++)
         putch(string[i]);
}


static inline uint32_t gf256v_mul_u32(uint32_t a, uint8_t b)
{
    uint32_t a_msb;
    uint32_t a32 = a;
    uint32_t b32 = b;

    if(trigger == 1) 
    { 
        char c = 'A';
        // stall until ready to trace 
        while ((c != 'g'))
        {
            c = getch();
        }
        trigger_high();
    }
    uint32_t r32 = a32 * (b32 & 1);

    a_msb = a32 & 0x80808080; // MSB, 7th bits
    a32 ^= a_msb;             // clear MSB
    a32 = (a32 << 1) ^ ((a_msb >> 7) * 0x1b);
    r32 ^= (a32) * ((b32 >> 1) & 1);

    a_msb = a32 & 0x80808080; // MSB, 7th bits
    a32 ^= a_msb;             // clear MSB
    a32 = (a32 << 1) ^ ((a_msb >> 7) * 0x1b);
    r32 ^= (a32) * ((b32 >> 2) & 1);

    a_msb = a32 & 0x80808080; // MSB, 7th bits
    a32 ^= a_msb;             // clear MSB
    a32 = (a32 << 1) ^ ((a_msb >> 7) * 0x1b);
    r32 ^= (a32) * ((b32 >> 3) & 1);

    a_msb = a32 & 0x80808080; // MSB, 7th bits
    a32 ^= a_msb;             // clear MSB
    a32 = (a32 << 1) ^ ((a_msb >> 7) * 0x1b);
    r32 ^= (a32) * ((b32 >> 4) & 1);

    a_msb = a32 & 0x80808080; // MSB, 7th bits
    a32 ^= a_msb;             // clear MSB
    a32 = (a32 << 1) ^ ((a_msb >> 7) * 0x1b);
    r32 ^= (a32) * ((b32 >> 5) & 1);

    a_msb = a32 & 0x80808080; // MSB, 7th bits
    a32 ^= a_msb;             // clear MSB
    a32 = (a32 << 1) ^ ((a_msb >> 7) * 0x1b);
    r32 ^= (a32) * ((b32 >> 6) & 1);

    a_msb = a32 & 0x80808080; // MSB, 7th bits
    a32 ^= a_msb;             // clear MSB
    a32 = (a32 << 1) ^ ((a_msb >> 7) * 0x1b);
    r32 ^= (a32) * ((b32 >> 7) & 1);

    if(trigger == 1) 
    {
        trigger_low();
        putch('r');
    }    


    return r32;
}

static inline void _gf256v_add_u32_aligned(uint8_t *accu_b, const uint8_t *a, unsigned _num_byte)
{
    while (_num_byte >= 4)
    {
        uint32_t *bx = (uint32_t *)accu_b;
        uint32_t *ax = (uint32_t *)a;
        bx[0] ^= ax[0];
        a += 4;
        accu_b += 4;
        _num_byte -= 4;
    }
    while (_num_byte)
    {
        _num_byte--;
        accu_b[_num_byte] ^= a[_num_byte];
    }

}

static inline void _gf256v_madd_u32_aligned(uint8_t *accu_c, const uint8_t *a, uint8_t gf256_b, unsigned _num_byte)
{
    while (_num_byte >= 4)
    {
        const uint32_t *ax = (const uint32_t *)a;
        uint32_t *cx = (uint32_t *)accu_c;
        // gf256_b = 0x0;
        cx[0] ^= gf256v_mul_u32(ax[0], gf256_b);
        a += 4;
        accu_c += 4;
        _num_byte -= 4;
    }
    if (0 == _num_byte)
        return;

    union tmp_32
    {
        uint8_t u8[4];
        uint32_t u32;
    } t;
    for (unsigned i = 0; i < _num_byte; i++)
        t.u8[i] = a[i];
    t.u32 = gf256v_mul_u32(t.u32, gf256_b);
    for (unsigned i = 0; i < _num_byte; i++)
        accu_c[i] ^= t.u8[i];
}

static inline void _gf256v_madd_u32(uint8_t *accu_c, const uint8_t *a, uint8_t gf256_b, unsigned _num_byte)
{

    uintptr_t ap = (uintptr_t)(const void *)a;
    uintptr_t cp = (uintptr_t)(const void *)accu_c;
    if (!((cp & 3) || (ap & 3) || (_num_byte < 8)))
    {
        _gf256v_madd_u32_aligned(accu_c, a, gf256_b, _num_byte);
        return;
    }

    union tmp_32
    {
        uint8_t u8[4];
        uint32_t u32;
    } t;

    while (_num_byte >= 4)
    {
        t.u8[0] = a[0];
        t.u8[1] = a[1];
        t.u8[2] = a[2];
        t.u8[3] = a[3];
        t.u32 = gf256v_mul_u32(t.u32, gf256_b);



        accu_c[0] ^= t.u8[0];
        accu_c[1] ^= t.u8[1];
        accu_c[2] ^= t.u8[2];
        accu_c[3] ^= t.u8[3];



        a += 4;
        accu_c += 4;
        _num_byte -= 4;
    }
    if (0 == _num_byte)
        return;

    for (unsigned i = 0; i < _num_byte; i++)
        t.u8[i] = a[i];
    t.u32 = gf256v_mul_u32(t.u32, gf256_b);
    for (unsigned i = 0; i < _num_byte; i++)
        accu_c[i] ^= t.u8[i];
}

static inline void _gf256v_add_u32(uint8_t *accu_b, const uint8_t *a, unsigned _num_byte)
{
    uintptr_t bp = (uintptr_t)(const void *)accu_b;
    uintptr_t ap = (uintptr_t)(const void *)a;
    if (!((bp & 3) || (ap & 3) || (_num_byte < 8)))
    {
        _gf256v_add_u32_aligned(accu_b, a, _num_byte);
        return;
    }

    while (_num_byte)
    {
        _num_byte--;
        accu_b[_num_byte] ^= a[_num_byte];
    }
}

static inline uint8_t gf256v_get_ele(const uint8_t *a, unsigned i) { return a[i]; }

static void gf256v_set_zero(uint8_t *b, unsigned _num_byte)
{
    gf256v_add(b, b, _num_byte);
}

static void batch_quad_trimat_eval_gf256(unsigned char *y, const unsigned char *trimat, const unsigned char *x, unsigned dim, unsigned size_batch)
{
#if defined(_BLAS_AVX2_)
    batch_quad_trimat_eval_gf256_avx2(y, trimat, x, dim, size_batch);
#elif defined(_BLAS_SSE_)
    batch_quad_trimat_eval_gf256_sse(y, trimat, x, dim, size_batch);
#else
    ///
    ///    assert( dim <= 256 );
    ///    assert( size_batch <= 256 );

    unsigned char tmp[256];

    unsigned char _x[256];

    for (unsigned i = 0; i < dim; i++)
        _x[i] = gf256v_get_ele(x, i);

    trigger = 0;
    char c;
    uint16_t counter = 0;

    gf256v_set_zero(y, size_batch);
    for (unsigned i = 0; i < dim; i++)
    {
        gf256v_set_zero(tmp, size_batch);
        for (unsigned j = i; j < dim; j++)
        {
            if(i==j) trigger = 1;

            gf256v_madd(tmp, trimat, _x[j], size_batch);
            
            trigger = 0;

            trimat += size_batch;
        }
      
        gf256v_madd(y, tmp, _x[i], size_batch);


    }


#endif
}

// uint8_t uov_trace(uint8_t *in)
// {

// uint8_t uov_trace(uint8_t* pw, uint8_t len)
uint8_t uov_trace()
{

    uint8_t y[_O1_BYTE] = {0};


    const unsigned char *ptr_trimat;
    ptr_trimat = trimat;

    batch_quad_trimat_eval_gf256(y, ptr_trimat, x, _V1, _O1_BYTE);
    
    // send 'e' for end of trace
    putch('e');
    
    putString(y, _O1_BYTE);

    return 1;
}

int main(void)
{
    platform_init();
    init_uart();
    trigger_setup();

    // putch('H');
    // putch('e');
    // putch('l');
    // putch('l');
    // putch('o');
    // putch(' ');
    // putch('f');
    // putch('r');
    // putch('o');
    // putch('m');
    // putch(' ');
    // putch('t');
    // putch('h');
    // putch('e');
    // putch(' ');
    // putch('o');
    // putch('t');
    // putch('h');
    // putch('e');
    // putch('r');
    // putch(' ');
    // putch('s');        
    // putch('i'); 
    // putch('d'); 
    // putch('e');             
    // putch('!');   
    unsigned char Hello[] = "Hello from the other side!";
    putString(Hello, 26);

    // simpleserial_init();

    uov_trace();


    // while (1)
    //     simpleserial_get();
}
