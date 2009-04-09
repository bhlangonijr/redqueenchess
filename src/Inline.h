/*
	Redqueen Chess Engine
    Copyright (C) 2008-2009 Ben-Hur Carlos Vieira Langoni Junior

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

extern unsigned char _BitScanForward64(unsigned int* const Index, const uint64_t Mask);

extern unsigned char _BitScanReverse64(unsigned int* const Index, const uint64_t Mask);

#endif /* INLINE_H_ */
