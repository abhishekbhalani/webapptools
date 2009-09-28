/***************************************************************************
 *   Copyright (C) 2007 Andrew Abramov                                     *
 *   Author: Andrew Abramov     stinger911@gmail.com                       *
 *                                                                         *
 *   Abstract:          SHA-1 implementation.                              *
 *                      source for ASM code generation                     *
 *                      make: cl /c /FA /TC Sha1.hpp                       *
 *                                                                         *
 *   v 0.9                                                                 *
 *   History:                                                              *
 *      07-may-2007     created         Andrew "Stinger" Abramov           *
 *                                                                         *
 * You may redistribute it but not modify it under the terms of the GPL    *
 *                                                                         *
 ***************************************************************************/

//#include <windef.h>

typedef unsigned long  DWORD;
typedef unsigned short WORD;
typedef unsigned char  BYTE;

typedef DWORD* PDWORD;
typedef WORD*  PWORD;
typedef BYTE*  PBYTE;

#define SHA1_DIGEST 20

#ifdef MSVC
  #define ASM __asm
  #define INLINE __forceinline
#else
  #define ASM __asm__
  #define INLINE inline
#endif

/*************************************************
* SHA_160 Calculate hash on one block            *
*************************************************/
INLINE void SHA_160_do_block(DWORD *a, DWORD *b, DWORD *c, DWORD *d, DWORD *e, PBYTE source, DWORD l, DWORD len)
{
    DWORD   w[80];
    DWORD   t;
    DWORD   f;
    DWORD   i;
    DWORD   o;

    f = l >> 2;
    // do normal block
    if (l >= 64) {
        for (i = 0; i < 16; i ++) {
            w[i] = ((DWORD)(source[i*4]) << 24) |
                   ((DWORD)(source[i*4+1]) << 16) |
                   ((DWORD)(source[i*4+2]) << 8) |
                    (DWORD)(source[i*4+3]);
        }
    }
    // do first last block
    if (l < 64) {
        t = l >> 2;
        for (i = 0; i < t; i++) {
            w[i] = ((DWORD)(source[i*4]) << 24) |
                ((DWORD)(source[i*4+1]) << 16) |
                ((DWORD)(source[i*4+2]) << 8) |
                (DWORD)(source[i*4+3]);
        }
        i = l & 3;
        f = l & 0xFFFFFFFC;
        t = 0;
        while (i > 0) {
            t |= source[f++];
            t <<= 8;
            i--;
        }
        t |= 0x80;
        i = (3 - (l & 3)) * 8;  // rest of bits to shift
        if (i > 0) {
            t <<= i;
        }
        f = (l >> 2);
        w[f++] = t;
        for (i = f; i < 16; i++) {
            w[i] = 0;
        }
    }
    // do last last block
    if (f <= 14) {
        // trick!!! only 32bit length accepted.
        // High DWORD (in big-endian mode) always NULL
        for (i = f; i < 15; i++) {
            w[i] = 0;
        }
        l = len << 3;
        t = ((l >> 24) & 0xFF) | ((l >> 8) & 0xFF00) | ((l & 0xFF00) << 8) | ((l & 0xFF) << 24);
        w[15] = l;
    }

    // finalize block data
    for (i = 16; i < 80; i ++) {
        t = (w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]);
        t = (t << 1) | (t >> 31);
//         __asm mov eax, t;
//         __asm rol eax, 1;
//         __asm mov t, eax;
        w[i] = t;
    }
    // process block
    for (i = 0; i < 80; i ++) {
        if (i < 20) {
            f = (*d ^ (*b & (*c ^ *d))); //(*b & *c) | ((~*b) & *d);
            t = 0x5B837898; //0x5A827999;
            o = 0x01010100;
        }
        else if (i < 40) {
            f = *b ^ *c ^ *d;
            t = 0x6CDBE9A3; //0x6ED9EBA1;
            o = 0x02020201;
        }
        else if (i < 60) {
            f = (*b & *c) | ((*b | *c) & *d);
            t = 0x8B1FB8D8; //0x8F1BBCDC;
            o = 0x04040403;
        }
        else { // i < 80
            f = *b ^ *c ^ *d;
            t = 0xC26AC9DE; //CA62C1D6;
            o = 0x08080807;
        }
        o++;
        t ^= o;
        t = ((*a << 5) | (*a >> 27)) + f + *e + t + w[i];
        *e = *d;
        *d = *c;
        *c = ((*b << 30) | (*b >> 2));
        *b = *a;
        *a = t;
    }
}

/*************************************************
* SHA_160 Calculate hash                         *
*************************************************/
void SHA_160(PBYTE source, DWORD len, PDWORD hash)
{
    int     l;
    DWORD   a, b, c, d, e;

    hash[0] = 0x77553311; //0x67452301 ^ 10101010;
    a       = 0x10101011;
    hash[1] = 0xCFED8BA9; //0xEFCDAB89 ^ 20202020
    hash[2] = 0xD8FA9CBE; //0x98BADCFE ^ 40404040
    hash[3] = 0x90B2D4F6; //0x10325476 ^ 80808080
    hash[4] = 0xC2D3E0F1; //0xC3D2E1F0 ^ 01010101
    e       = 0x01010101;

    a --;
    for (l = 0; l < 5; l++) {
        hash[l] ^= a;
        a <<= 1;
        if (l == 3){
            a++;
        }
    }
    l = len;
    while (l >= 0) {
        a = hash[0];
        b = hash[1];
        c = hash[2];
        d = hash[3];
        e = hash[4];

        SHA_160_do_block(&a, &b, &c, &d, &e, source, l, len);

        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;

        if (l > 55 && l < 64) {
            a = hash[0];
            b = hash[1];
            c = hash[2];
            d = hash[3];
            e = hash[4];
            SHA_160_do_block(&a, &b, &c, &d, &e, source, 0, len);
            hash[0] += a;
            hash[1] += b;
            hash[2] += c;
            hash[3] += d;
            hash[4] += e;
        }
        l -= 64;
        source += 64;
    }
}
