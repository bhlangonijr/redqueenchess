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
 * Evaluator.h
 *
 *  Created on: Jan 1, 2010
 *      Author: bhlangonijr
 */

#ifndef EVALUATOR_H_
#define EVALUATOR_H_

#include <string.h>
#include "inline.h"
#include "board.h"

#define NFILE(X) ((squareFile[X]!=FILE_H ? fileBB[squareFile[X]+1] : EMPTY_BB) | \
		(squareFile[X]!=FILE_A ? fileBB[squareFile[X]-1] : EMPTY_BB))

#define BYSIDEBB(X) ((squareFile[X]!=FILE_H ? squareToBitboard[X+1] : EMPTY_BB) | \
		(squareFile[X]!=FILE_A ? squareToBitboard[X-1] : EMPTY_BB))

#define FSQUARE(COLOR, BB, X) (COLOR==WHITE ?  bitsBetween(BB,X,encodeSquare[RANK_8][squareFile[X]]) : \
		bitsBetween(BB,encodeSquare[RANK_1][squareFile[X]],X))^BYSIDEBB(X)

#define PASSEDMASK(COLOR, X) (FSQUARE(COLOR, (fileAttacks[squareFile[X]] | NFILE(X)), X))

const int defaultMaterialValues[ALL_PIECE_TYPE_BY_COLOR] = {100, 318, 325, 520, 975, 10000, 100, 318, 325, 520, 975, 10000, 0};
const int endGameMaterialValues[ALL_PIECE_TYPE_BY_COLOR] = {110, 310, 325, 520, 975, 10000, 110, 310, 325, 520, 975, 10000, 0};

// opening and middlegame piece square table
const int defaultPieceSquareTable[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE]={

		{ // white pawn
				0,  0,  0,  0,  0,  0,  0,  0,
				5,  10, 10,-20,-20, 10, 10, 5,
				5, -5,-10,  0,  0, -10, -5, 5,
				0,  -5,-10, 20, 20, -10,-5,  0,
				0,  0, 0, 5, 	5, 0,  0,  0,
				0,  0,  0,  0,  0,  0,  0,  0,
				50, 50, 50, 50, 50, 50, 50, 50,
				0,  0,  0,  0,  0,  0,  0,  0,
		},
		{//white knight
				-50,-40,-30,-30,-30,-30,-40,-50,
				-40,-20,  0,  5,  5,  0,-20,-40,
				-30,  5, 10, 15, 15, 10,  5,-30,
				-30,  0, 15, 20, 20, 15,  0,-30,
				-30,  5, 15, 20, 20, 15,  5,-30,
				-30,  0, 10, 15, 15, 10,  0,-30,
				-40,-20,  0,  0,  0,  0,-20,-40,
				-50,-40,-30,-30,-30,-30,-40,-50,
		},
		{//white bishop
				-20,-10,-10,-10,-10,-10,-10,-20,
				-10,  5,  0,  0,  0,  0,  5,-10,
				-10, 10, 10, 10, 10, 10, 10,-10,
				-10,  0, 10, 10, 10, 10,  0,-10,
				-10,  5,  5, 10, 10,  5,  5,-10,
				-10,  0,  5, 10, 10,  5,  0,-10,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-20,-10,-10,-10,-10,-10,-10,-20,
		},
		{//white rook
				0,  0,  0,  0,  0,  0,  0,  0,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				5, 10, 10, 10, 10, 10, 10,  5,
				0,  0,  0,  5,  5,  0,  0,  0,
		},
		{//white queen
				-20,-10,-10, -5, -5,-10,-10,-20,
				-10,  0,  5,  0,  0,  0,  0,-10,
				-10,  5,  5,  5,  5,  5,  0,-10,
				0,  0,  5,  5,  5,  5,  0, -5,
				-5,  0,  5,  5,  5,  5,  0, -5,
				-10,  0,  5,  5,  5,  5,  0,-10,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-20,-10,-10, -5, -5,-10,-10,-20,
		},
		{//white king
				20, 30, 10,  0,  0, 10, 30, 20,
				20, 20,  0,  0,  0,  0, 20, 20,
				-10,-20,-20,-20,-20,-20,-20,-10,
				-20,-30,-30,-40,-40,-30,-30,-20,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
		},
		{//black pawn
				0,  0,  0,  0,  0,  0,  0,  0,
				50, 50, 50, 50, 50, 50, 50, 50,
				0,  0,  0,  0,  0,  0,  0,  0,
				0,  0, 0,  5,  5, 0,  0,  0,
				0, -5,-10, 20, 20, -10, -5,  0,
				5, -5,-10,  0,  0,-10, -5,  5,
				5, 10, 10,-20,-20, 10, 10,  5,
				0,  0,  0,  0,  0,  0,  0,  0,
		},
		{//black knight
				-50,-40,-30,-30,-30,-30,-40,-50,
				-40,-20,  0,  0,  0,  0,-20,-40,
				-30,  0, 10, 15, 15, 10,  0,-30,
				-30,  5, 15, 20, 20, 15,  5,-30,
				-30,  0, 15, 20, 20, 15,  0,-30,
				-30,  5, 10, 15, 15, 10,  5,-30,
				-40,-20,  0,  5,  5,  0,-20,-40,
				-50,-40,-30,-30,-30,-30,-40,-50,
		},
		{//black bishop
				-20,-10,-10,-10,-10,-10,-10,-20,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-10,  0,  5, 10, 10,  5,  0,-10,
				-10,  5,  5, 10, 10,  5,  5,-10,
				-10,  0, 10, 10, 10, 10,  0,-10,
				-10, 10, 10, 10, 10, 10, 10,-10,
				-10,  5,  0,  0,  0,  0,  5,-10,
				-20,-10,-10,-10,-10,-10,-10,-20,
		},
		{//black rook
				0,  0,  0,  0,  0,  0,  0,  0,
				5, 10, 10, 10, 10, 10, 10,  5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				0,  0,  0,  5,  5,  0,  0,  0,
		},
		{//black queen:
				-20,-10,-10, -5, -5,-10,-10,-20,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-10,  0,  5,  5,  5,  5,  0,-10,
				-5,  0,  5,  5,  5,  5,  0, -5,
				0,  0,  5,  5,  5,  5,  0, -5,
				-10,  5,  5,  5,  5,  5,  0,-10,
				-10,  0,  5,  0,  0,  0,  0,-10,
				-20,-10,-10, -5, -5,-10,-10,-20,
		},
		{//black king
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-20,-30,-30,-40,-40,-30,-30,-20,
				-10,-20,-20,-20,-20,-20,-20,-10,
				20, 20,  0,  0,  0,  0, 20, 20,
				20, 30, 10,  0,  0, 10, 30, 20,
		},
		{ // empty
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0
		}

};

// end game piece square table
const int endGamePieceSquareTable[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE]={

		{ // white pawn
				0,  0,  0,  0,  0,  0,  0,  0,
				1,  1,  1, -2, -2,  1,  1,  1,
				1, -1, -1,  0,  0, -1, -1,  1,
				0,  5,  5, 10, 10,  5,  5,  0,
				5,  5, 10, 25, 25, 10,  5,  5,
				10, 10, 20, 30, 30, 20, 10, 10,
				50, 50, 50, 50, 50, 50, 50, 50,
				0,  0,  0,  0,  0,  0,  0,  0,
		},
		{//white knight
				-50,-40,-30,-30,-30,-30,-40,-50,
				-40,-20,  0,  5,  5,  0,-20,-40,
				-30,  5, 10, 15, 15, 10,  5,-30,
				-30,  0, 15, 20, 20, 15,  0,-30,
				-30,  5, 15, 20, 20, 15,  5,-30,
				-30,  0, 10, 15, 15, 10,  0,-30,
				-40,-20,  0,  0,  0,  0,-20,-40,
				-50,-40,-30,-30,-30,-30,-40,-50,
		},
		{//white bishop
				-20,-10,-10,-10,-10,-10,-10,-20,
				-10,  5,  0,  0,  0,  0,  5,-10,
				-10, 10, 10, 10, 10, 10, 10,-10,
				-10,  0, 10, 10, 10, 10,  0,-10,
				-10,  5,  5, 10, 10,  5,  5,-10,
				-10,  0,  5, 10, 10,  5,  0,-10,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-20,-10,-10,-10,-10,-10,-10,-20,
		},
		{//white rook
				0,  0,  0,  0,  0,  0,  0,  0,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				5, 10, 10, 10, 10, 10, 10,  5,
				0,  0,  0,  5,  5,  0,  0,  0,
		},
		{//white queen
				-20,-10,-10, -5, -5,-10,-10,-20,
				-10,  0,  5,  0,  0,  0,  0,-10,
				-10,  5,  5,  5,  5,  5,  0,-10,
				0,  0,  5,  5,  5,  5,  0, -5,
				-5,  0,  5,  5,  5,  5,  0, -5,
				-10,  0,  5,  5,  5,  5,  0,-10,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-20,-10,-10, -5, -5,-10,-10,-20,
		},
		{//white king
				0,  10, 20, 30, 30, 20, 10,  0,
				10, 20, 30, 40, 40, 30, 20, 10,
				20, 30, 40, 50, 50, 40, 30, 20,
				30, 40, 50, 60, 60, 50, 40, 30,
				30, 40, 50, 60, 60, 50, 40, 30,
				20, 30, 40, 50, 50, 40, 30, 20,
				10, 20, 30, 40, 40, 30, 20, 10,
				0, 10, 20, 30, 30, 20, 10,  0,
		},
		{//black pawn
				0,  0,  0,  0,  0,  0,  0,  0,
				50, 50, 50, 50, 50, 50, 50, 50,
				10, 10, 20, 30, 30, 20, 10, 10,
				5,  5, 10, 25, 25, 10,  5,  5,
				0,  5,  5, 10, 10,  5,  5,  0,
				1, -1, -1,  0,  0,-1, -1,  1,
				1,  1,  1, -2, -2, 1,  1,  1,
				0,  0,  0,  0,  0, 0,  0,  0,
		},
		{//black knight
				-50,-40,-30,-30,-30,-30,-40,-50,
				-40,-20,  0,  0,  0,  0,-20,-40,
				-30,  0, 10, 15, 15, 10,  0,-30,
				-30,  5, 15, 20, 20, 15,  5,-30,
				-30,  0, 15, 20, 20, 15,  0,-30,
				-30,  5, 10, 15, 15, 10,  5,-30,
				-40,-20,  0,  5,  5,  0,-20,-40,
				-50,-40,-30,-30,-30,-30,-40,-50,
		},
		{//black bishop
				-20,-10,-10,-10,-10,-10,-10,-20,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-10,  0,  5, 10, 10,  5,  0,-10,
				-10,  5,  5, 10, 10,  5,  5,-10,
				-10,  0, 10, 10, 10, 10,  0,-10,
				-10, 10, 10, 10, 10, 10, 10,-10,
				-10,  5,  0,  0,  0,  0,  5,-10,
				-20,-10,-10,-10,-10,-10,-10,-20,
		},
		{//black rook
				0,  0,  0,  0,  0,  0,  0,  0,
				5, 10, 10, 10, 10, 10, 10,  5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				-5,  0,  0,  0,  0,  0,  0, -5,
				0,  0,  0,  5,  5,  0,  0,  0,
		},
		{//black queen:
				-20,-10,-10, -5, -5,-10,-10,-20,
				-10,  0,  0,  0,  0,  0,  0,-10,
				-10,  0,  5,  5,  5,  5,  0,-10,
				-5,  0,  5,  5,  5,  5,  0, -5,
				0,  0,  5,  5,  5,  5,  0, -5,
				-10,  5,  5,  5,  5,  5,  0,-10,
				-10,  0,  5,  0,  0,  0,  0,-10,
				-20,-10,-10, -5, -5,-10,-10,-20,
		},
		{//black king
				0,  10, 20, 30, 30, 20, 10,  0,
				10, 20, 30, 40, 40, 30, 20, 10,
				20, 30, 40, 50, 50, 40, 30, 20,
				30, 40, 50, 60, 60, 50, 40, 30,
				30, 40, 50, 60, 60, 50, 40, 30,
				20, 30, 40, 50, 50, 40, 30, 20,
				10, 20, 30, 40, 40, 30, 20, 10,
				0, 10, 20, 30, 30, 20, 10,  0},
				{ // empty
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0
				}

};

const Bitboard neighborFiles[ALL_SQUARE]={
		NFILE(A1), NFILE(B1), NFILE(C1), NFILE(D1), NFILE(E1), NFILE(F1), NFILE(G1), NFILE(H1),
		NFILE(A2), NFILE(B2), NFILE(C2), NFILE(D2), NFILE(E2), NFILE(F2), NFILE(G2), NFILE(H2),
		NFILE(A3), NFILE(B3), NFILE(C3), NFILE(D3), NFILE(E3), NFILE(F3), NFILE(G3), NFILE(H3),
		NFILE(A4), NFILE(B4), NFILE(C4), NFILE(D4), NFILE(E4), NFILE(F4), NFILE(G4), NFILE(H4),
		NFILE(A5), NFILE(B5), NFILE(C5), NFILE(D5), NFILE(E5), NFILE(F5), NFILE(G5), NFILE(H5),
		NFILE(A6), NFILE(B6), NFILE(C6), NFILE(D6), NFILE(E6), NFILE(F6), NFILE(G6), NFILE(H6),
		NFILE(A7), NFILE(B7), NFILE(C7), NFILE(D7), NFILE(E7), NFILE(F7), NFILE(G7), NFILE(H7),
		NFILE(A8), NFILE(B8), NFILE(C8), NFILE(D8), NFILE(E8), NFILE(F8), NFILE(G8), NFILE(H8)
};

const Bitboard passedMask[ALL_PIECE_COLOR][ALL_SQUARE]= {
		{
				PASSEDMASK(WHITE,A1), PASSEDMASK(WHITE,B1), PASSEDMASK(WHITE,C1), PASSEDMASK(WHITE,D1), PASSEDMASK(WHITE,E1), PASSEDMASK(WHITE,F1), PASSEDMASK(WHITE,G1), PASSEDMASK(WHITE,H1),
				PASSEDMASK(WHITE,A2), PASSEDMASK(WHITE,B2), PASSEDMASK(WHITE,C2), PASSEDMASK(WHITE,D2), PASSEDMASK(WHITE,E2), PASSEDMASK(WHITE,F2), PASSEDMASK(WHITE,G2), PASSEDMASK(WHITE,H2),
				PASSEDMASK(WHITE,A3), PASSEDMASK(WHITE,B3), PASSEDMASK(WHITE,C3), PASSEDMASK(WHITE,D3), PASSEDMASK(WHITE,E3), PASSEDMASK(WHITE,F3), PASSEDMASK(WHITE,G3), PASSEDMASK(WHITE,H3),
				PASSEDMASK(WHITE,A4), PASSEDMASK(WHITE,B4), PASSEDMASK(WHITE,C4), PASSEDMASK(WHITE,D4), PASSEDMASK(WHITE,E4), PASSEDMASK(WHITE,F4), PASSEDMASK(WHITE,G4), PASSEDMASK(WHITE,H4),
				PASSEDMASK(WHITE,A5), PASSEDMASK(WHITE,B5), PASSEDMASK(WHITE,C5), PASSEDMASK(WHITE,D5), PASSEDMASK(WHITE,E5), PASSEDMASK(WHITE,F5), PASSEDMASK(WHITE,G5), PASSEDMASK(WHITE,H5),
				PASSEDMASK(WHITE,A6), PASSEDMASK(WHITE,B6), PASSEDMASK(WHITE,C6), PASSEDMASK(WHITE,D6), PASSEDMASK(WHITE,E6), PASSEDMASK(WHITE,F6), PASSEDMASK(WHITE,G6), PASSEDMASK(WHITE,H6),
				PASSEDMASK(WHITE,A7), PASSEDMASK(WHITE,B7), PASSEDMASK(WHITE,C7), PASSEDMASK(WHITE,D7), PASSEDMASK(WHITE,E7), PASSEDMASK(WHITE,F7), PASSEDMASK(WHITE,G7), PASSEDMASK(WHITE,H7),
				PASSEDMASK(WHITE,A8), PASSEDMASK(WHITE,B8), PASSEDMASK(WHITE,C8), PASSEDMASK(WHITE,D8), PASSEDMASK(WHITE,E8), PASSEDMASK(WHITE,F8), PASSEDMASK(WHITE,G8), PASSEDMASK(WHITE,H8)
		},
		{
				PASSEDMASK(BLACK,A1), PASSEDMASK(BLACK,B1), PASSEDMASK(BLACK,C1), PASSEDMASK(BLACK,D1), PASSEDMASK(BLACK,E1), PASSEDMASK(BLACK,F1), PASSEDMASK(BLACK,G1), PASSEDMASK(BLACK,H1),
				PASSEDMASK(BLACK,A2), PASSEDMASK(BLACK,B2), PASSEDMASK(BLACK,C2), PASSEDMASK(BLACK,D2), PASSEDMASK(BLACK,E2), PASSEDMASK(BLACK,F2), PASSEDMASK(BLACK,G2), PASSEDMASK(BLACK,H2),
				PASSEDMASK(BLACK,A3), PASSEDMASK(BLACK,B3), PASSEDMASK(BLACK,C3), PASSEDMASK(BLACK,D3), PASSEDMASK(BLACK,E3), PASSEDMASK(BLACK,F3), PASSEDMASK(BLACK,G3), PASSEDMASK(BLACK,H3),
				PASSEDMASK(BLACK,A4), PASSEDMASK(BLACK,B4), PASSEDMASK(BLACK,C4), PASSEDMASK(BLACK,D4), PASSEDMASK(BLACK,E4), PASSEDMASK(BLACK,F4), PASSEDMASK(BLACK,G4), PASSEDMASK(BLACK,H4),
				PASSEDMASK(BLACK,A5), PASSEDMASK(BLACK,B5), PASSEDMASK(BLACK,C5), PASSEDMASK(BLACK,D5), PASSEDMASK(BLACK,E5), PASSEDMASK(BLACK,F5), PASSEDMASK(BLACK,G5), PASSEDMASK(BLACK,H5),
				PASSEDMASK(BLACK,A6), PASSEDMASK(BLACK,B6), PASSEDMASK(BLACK,C6), PASSEDMASK(BLACK,D6), PASSEDMASK(BLACK,E6), PASSEDMASK(BLACK,F6), PASSEDMASK(BLACK,G6), PASSEDMASK(BLACK,H6),
				PASSEDMASK(BLACK,A7), PASSEDMASK(BLACK,B7), PASSEDMASK(BLACK,C7), PASSEDMASK(BLACK,D7), PASSEDMASK(BLACK,E7), PASSEDMASK(BLACK,F7), PASSEDMASK(BLACK,G7), PASSEDMASK(BLACK,H7),
				PASSEDMASK(BLACK,A8), PASSEDMASK(BLACK,B8), PASSEDMASK(BLACK,C8), PASSEDMASK(BLACK,D8), PASSEDMASK(BLACK,E8), PASSEDMASK(BLACK,F8), PASSEDMASK(BLACK,G8), PASSEDMASK(BLACK,H8)
		},
		{EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,
				EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,
				EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,
				EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB}

};

const int gameSize=60;

class Evaluator {
public:

	enum GamePhase {
		OPENING, MIDDLEGAME, ENDGAME
	};

	Evaluator();
	virtual ~Evaluator();
	const int evaluate(Board& board);
	const int evalMaterial(Board& board, PieceColor color);
	const int evalPieces(Board& board, PieceColor color);
	const int evalMobility(Board& board, PieceColor color);
	const int evalDevelopment(Board& board, PieceColor color);
	const int evalImbalances(Board& board, PieceColor color);
	const void setGameStage(const GamePhase phase);
	const GamePhase getGameStage();
	const GamePhase predictGameStage(Board& board);

	inline const int getPieceMaterialValue(Board& board, const PieceTypeByColor piece) {

		const int egValue = endGameMaterialValues[piece];
		const int mgValue = defaultMaterialValues[piece];
		const int mc = board.getMoveCounter();

		if (gamePhase==ENDGAME || mc >= gameSize) {
			return egValue;
		}

		return int((double)mgValue + (double)1/(double)(gameSize-mc) * (double)(egValue-mgValue));
	}

	inline const int getPieceSquareValue(Board& board, const PieceTypeByColor piece, const Square square) {

		const int egValue = endGamePieceSquareTable[piece][square];
		const int mgValue = defaultPieceSquareTable[piece][square];
		const int mc = board.getMoveCounter();

		if (gamePhase==ENDGAME || mc >= gameSize) {
			return egValue;
		}

		return int((double)mgValue + (double)1/(double)(gameSize-mc) * (double)(egValue-mgValue));
	}

private:
	GamePhase gamePhase;

};

// main eval function
inline const int Evaluator::evaluate(Board& board) {

	const int CHECK_MATERIAL = 300;
	const PieceColor side = board.getSideToMove();
	const PieceColor other = board.flipSide(board.getSideToMove());

	int material = evalMaterial(board, side) - evalMaterial(board, other);
	int pieces = evalPieces(board, side) - evalPieces(board, other);
	int development = evalDevelopment(board, side) - evalDevelopment(board, other);
	int imbalances = evalImbalances(board, side) - evalImbalances(board, other);
	int mobility = 0;

	setGameStage(predictGameStage(board));

	if ((gamePhase!=ENDGAME) && (material > -CHECK_MATERIAL && material < CHECK_MATERIAL )) {
		mobility = evalMobility(board, side) - evalMobility(board, other);
	}

	return material+mobility+pieces+development+imbalances;
}

// material eval function
inline const int Evaluator::evalMaterial(Board& board, PieceColor color) {

	const int first = board.makePiece(color,PAWN);
	const int last = board.makePiece(color,KING);
	int material = 0;

	for(int pieceType = first; pieceType <= last; pieceType++) {
		int count = board.getPieceCountByType(PieceTypeByColor(pieceType));
		if (count > 0) {
			material += count * getPieceMaterialValue(board, PieceTypeByColor(pieceType));
		}
	}

	return material;
}

// king eval function
inline const int Evaluator::evalPieces(Board& board, PieceColor color) {

	const PieceColor other = board.flipSide(color);
	const int DONE_CASTLE_BONUS=       +(board.getPiecesByType(board.makePiece(other,QUEEN))) ? 15 : 5;
	const int CAN_CASTLE_BONUS=        +2;
	const int UNSTOPPABLE_PAWN_BONUS = +2;
	const int CENTERED_PAWN_BONUS =    +5;
	const int DOUBLED_ROOKS =          +10;
	const int DOUBLED_PAWN_PENALTY =   -15;
	const int ISOLATED_PAWN_PENALTY =  -15;
	const int BACKWARD_PAWN_PENALTY =  -10;
	int count=0;

	// king
	if (gamePhase!=ENDGAME) {
		// king castle bonus
		if (board.isCastleDone(color)) {
			count= DONE_CASTLE_BONUS;
		} else if (board.getCastleRights(color)==BOTH_SIDE_CASTLE) {
			count= CAN_CASTLE_BONUS;
		} else if (board.getCastleRights(color)==NO_CASTLE) {
			count= -DONE_CASTLE_BONUS;
		}
	}

	Bitboard pawns = board.getPiecesByType(board.makePiece(color,PAWN));
	Bitboard enemyPawns = board.getPiecesByType(board.makePiece(other,PAWN));

	//pawns - penalyze doubled & isolated pawns
	if (pawns) {
		Bitboard pieces=pawns;
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {
			if (squareToBitboard[from]&centerSquares) {
				count += CENTERED_PAWN_BONUS;
			}
			if (fileAttacks[squareFile[from]]&pieces) {
				count += DOUBLED_PAWN_PENALTY;
			}
			if (!(neighborFiles[from]&pawns)) {
				count += ISOLATED_PAWN_PENALTY;
			} else {
				if (!(adjacentSquares[from]&pawns)) {
					count += BACKWARD_PAWN_PENALTY;
				}
			}
			if (!(passedMask[color][from]&enemyPawns)) {
				count += UNSTOPPABLE_PAWN_BONUS * endGamePieceSquareTable[board.makePiece(color,PAWN)][from];
			}
			from = extractLSB(pieces);
		}
	}

	Bitboard rooks = board.getPiecesByType(board.makePiece(color,ROOK));
	Square from = extractLSB(rooks);

	while ( from!=NONE ) {

		if ((fileAttacks[squareFile[from]]|rankAttacks[squareFile[from]])&rooks) {
			count += DOUBLED_ROOKS;
		}
		from = extractLSB(rooks);
	}


	return count;
}

// mobility eval function
inline const int Evaluator::evalMobility(Board& board, PieceColor color) {

	const int NEAR_KING_ATTACK_BONUS=5;
	const int KING_ATTACK_BONUS=15;
	const PieceColor other=board.flipSide(color);

	Bitboard king = board.getPiecesByType(board.makePiece(other,KING));
	const Square kingSquare = extractLSB(king);
	const Bitboard nearKingSquares =king|adjacentSquares[kingSquare];

	Bitboard pieces = EMPTY_BB;
	Bitboard moves = EMPTY_BB;
	Bitboard attacks = EMPTY_BB;

	Square from = NONE;
	int count=0;

	pieces = board.getPiecesByType(board.makePiece(color,BISHOP));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		moves |= board.getBishopAttacks(from);
		attacks |= moves;
		from = extractLSB(pieces);
	}
	count+=_BitCount(moves);

	pieces = board.getPiecesByType(board.makePiece(color,ROOK));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		moves = board.getRookAttacks(from);
		attacks |= moves;
		count+=_BitCount(moves);
		from = extractLSB(pieces);
	}

	pieces = board.getPiecesByType(board.makePiece(color,QUEEN));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		moves = board.getQueenAttacks(from);
		attacks |= moves;
		count+=_BitCount(moves);
		from = extractLSB(pieces);
	}

	if (attacks) {
		const Bitboard otherAttacks = board.getPiecesByColor(other)&attacks;
		if (otherAttacks) {
			count += _BitCount(otherAttacks);
		}
		const Bitboard nearKingAttacks=nearKingSquares&attacks;
		if (nearKingAttacks) {
			count += _BitCount(nearKingAttacks) * NEAR_KING_ATTACK_BONUS;
		}
		const Bitboard kingAttacks = attacks&board.getPiecesByType(board.makePiece(other,KING));
		if (kingAttacks) {
			count += _BitCount(kingAttacks)*KING_ATTACK_BONUS;
		}
	}

	return count;
}

// material eval function
inline const int Evaluator::evalDevelopment(Board& board, PieceColor color) {

	const int first = board.makePiece(color,PAWN);
	const int last = board.makePiece(color,KING);
	int bonus = 0;

	for(int pieceType = first; pieceType <= last; pieceType++) {
		Bitboard pieces = board.getPiecesByType(PieceTypeByColor(pieceType));
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {
			bonus += getPieceSquareValue(board,PieceTypeByColor(pieceType),Square(from));
			from = extractLSB(pieces);
		}
	}

	return bonus;
}

// mobility eval function
inline const int Evaluator::evalImbalances(Board& board, PieceColor color) {

	const int bishopPairBonus = 20;
	int count=0;

	Bitboard bishop = board.getPiecesByType(board.makePiece(color,BISHOP));

	if ((bishop & WHITE_SQUARES) && (bishop & BLACK_SQUARES)) {
		count += bishopPairBonus;
	}

	// TODO implement more imbalances
	return count;
}

#endif /* EVALUATOR_H_ */
