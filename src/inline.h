/*
	Redqueen Chess Engine
    Copyright (C) 2008-2010 Ben-Hur Carlos Vieira Langoni Junior

    This file is part of Redqueen Chess Engine.

    Redqueen is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Redqueen is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Redqueen.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * Inline.h
 *
 *  Created on: Apr 8, 2009
 *      Author: bhlangonijr
 */

#ifndef INLINE_H_
#define INLINE_H_

#include <inttypes.h>

const uint64_t debruijn64 = 0x07EDD5E59A4E28C2ULL;

const uint32_t index64[64] = {
		63,  0, 58,  1, 59, 47, 53,  2,
		60, 39, 48, 27, 54, 33, 42,  3,
		61, 51, 37, 40, 49, 18, 28, 20,
		55, 30, 34, 11, 43, 14, 22,  4,
		62, 57, 46, 52, 38, 26, 32, 41,
		50, 36, 17, 19, 29, 10, 13, 21,
		56, 45, 25, 31, 35, 16,  9, 12,
		44, 24, 15,  8, 23,  7,  6,  5
};
// return the index of LSB
inline unsigned int _BitScanForward(unsigned int* const index, const uint64_t mask)
{
#if defined(__LP64__)
	uint64_t ret;
	asm	("bsfq %[mask], %[ret]" : [ret] "=r" (ret) :[mask] "mr" (mask));
	*index = (unsigned int)ret;
#else
	*index = (unsigned int) index64[((mask & -mask) * debruijn64) >> 58];
#endif

	return mask?1:0;
}
// return the index of MSB
inline unsigned int _BitScanReverse(unsigned int* const index, const uint64_t mask)
{
#if defined(__LP64__)
	uint64_t ret;
	asm ("bsrq %[mask], %[ret]" :[ret] "=r" (ret) :[mask] "mr" (mask));
	*index = (unsigned int)ret;
#else
	union {
		double d;
		struct {
			unsigned int mantissal : 32;
			unsigned int mantissah : 20;
			unsigned int exponent : 11;
			unsigned int sign : 1;
		};
	} ud;
	ud.d = (double)(mask & ~(mask >> 32));

	*index = ud.exponent - 1023;
#endif
	return mask?1:0;
}

inline uint32_t _BitCount(const uint64_t data)
{
   #if defined(__LP64__)
      return __builtin_popcountll( data );
   #else
      uint64_t x = data;
      const uint64_t k1 = 0x5555555555555555ULL;
      const uint64_t k2 = 0x3333333333333333ULL;
      const uint64_t k4 = 0x0f0f0f0f0f0f0f0fULL;
      const uint64_t kf = 0x0101010101010101ULL;
      x =  x       - ((x >> 1)  & k1);
      x = (x & k2) + ((x >> 2)  & k2);
      x = (x       +  (x >> 4)) & k4 ;
      x = (x * kf) >> 56;
      return (uint32_t) x;
   #endif
}

extern int getNumProcs();

#endif /* INLINE_H_ */