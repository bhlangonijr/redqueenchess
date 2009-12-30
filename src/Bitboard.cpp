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
 * Bitboard.cpp
 *
 *  Created on: Dec 10, 2009
 *      Author: bhlangonijr
 */

#include "Bitboard.h"

// print a bitboard in a readble form
void printBitboard(Bitboard bb) {

	for(long x=0;x<64;x++) {
		if ((0x1ULL << x)&bb) {
			std::cout << "1";
		} else {
			std::cout << "0";
		}
		if ((x+1) % 8 == 0) std::cout << std::endl;
	}

	std::cout << std::endl;

}

// get the bit index from a bitboard
Square bitboardToSquare(const Bitboard bitboard) {

	unsigned int square = 0;
	unsigned char ret = _BitScanForward(&square, bitboard);

	if (!ret) {
		return Square(NONE);
	}

	return Square( square );

}

// get squares between squarea and squareb
Bitboard getIntersectSquares(Square squarea, Square squareb) {

	Bitboard squares=EMPTY_BB;

	if (squareRank[squarea]==squareRank[squareb]) {
		squares = rankAttacks[squarea];
	} else if (squareFile[squarea]==squareFile[squareb]){
		squares = fileAttacks[squarea];
	} else if (squareToDiagonalA1H8[squarea]==squareToDiagonalA1H8[squareb]) {
		squares = diagA1H8Attacks[squarea];
	} else if (squareToDiagonalH1A8[squarea]==squareToDiagonalH1A8[squareb]) {
		squares = diagH1A8Attacks[squarea];
	}

	return bitsBetween(squares, squarea, squareb);
}

// lookup and set the nearest bits given a starting square index in the bitboard - downside and upside
void setNearBlocker(const Bitboard mask, const Square start, Square& minor, Square& major)
{

	unsigned int minorInt=A1;
	unsigned int majorInt=H8;
	unsigned char ret;

	if (!mask) {
		minor=A1;
		major=H8;
		return;
	}

	Bitboard lowerMask= mask & lowerMaskBitboard[start];
	Bitboard upperMask= mask & upperMaskBitboard[start];

	ret = _BitScanReverse(&minorInt, lowerMask);
	if (!ret) {
		minorInt=A1;
	}
	ret = _BitScanForward(&majorInt, upperMask);
	if (!ret) {
		majorInt=H8;
	}

	if (minorInt<0) {
		minorInt=A1;
	}
	minor = Square(minorInt);
	major = Square(majorInt);
}

// extract least significant bit of a bitboard
Square extractLSB(Bitboard& bitboard) {

	if (!bitboard) {
		return Square(NONE);
	}

	unsigned int square = 0;
	unsigned char ret = _BitScanForward(&square, bitboard);

	bitboard &= bitboard - 1;

	if (!ret /*|| !Square( square )*/) {
		return Square(NONE);
	}

	return Square( square );

}



