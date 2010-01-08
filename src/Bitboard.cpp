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

#define MIN(x,y) (x<y?x:y)
#define MAX(x,y) (x>y?x:y)

Bitboard bitsBetweenSquares[ALL_SQUARE][ALL_SQUARE];

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

// get squares between squarea and squareb
Bitboard getIntersectSquares(Square squarea, Square squareb) {

	Bitboard squares=EMPTY_BB;

	if (squareRank[squarea]==squareRank[squareb]) {
		squares = rankBB[squareRank[squarea]];
	} else if (squareFile[squarea]==squareFile[squareb]){
		squares = fileBB[squareFile[squarea]];
	} else if (squareToDiagonalA1H8[squarea]==squareToDiagonalA1H8[squareb]) {
		squares = diagonalA1H8BB[squareToDiagonalA1H8[squarea]];
	} else if (squareToDiagonalH1A8[squarea]==squareToDiagonalH1A8[squareb]) {
		squares = diagonalH1A8BB[squareToDiagonalH1A8[squarea]];
	}

	return bitsBetween(squares, MIN(squarea, squareb), MAX(squarea, squareb));
}

