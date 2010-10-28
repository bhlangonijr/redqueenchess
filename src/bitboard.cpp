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
 * Bitboard.cpp
 *
 *  Created on: Dec 10, 2009
 *      Author: bhlangonijr
 */

#include "bitboard.h"

const int DELTA_MAX = 7;
static int squareDelta[ALL_SQUARE][ALL_SQUARE];
static int inverseSquareDelta[ALL_SQUARE][ALL_SQUARE];

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

// initialize bitboards
void initializeBitboards() {

	for (int x=0;x<ALL_SQUARE;x++) {
		for (int y=0;y<ALL_SQUARE;y++) {
			const int delta1 = abs(squareRank[x]-squareRank[y]);
			const int delta2 = abs(squareFile[x]-squareFile[y]);
			squareDelta[x][y]=(delta1+delta2)/2;
			inverseSquareDelta[x][y]=DELTA_MAX-squareDelta[x][y];
		}
	}

}

int squareDistance(const Square from, const Square to) {
	return squareDelta[from][to];
}

int inverseSquareDistance(const Square from, const Square to) {
	return inverseSquareDelta[from][to];
}

