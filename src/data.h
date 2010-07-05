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
 * data.h
 *
 *  Created on: 26/06/2010
 *      Author: bhlangonijr
 */

#ifndef DATA_H_
#define DATA_H_

#include "bitboard.h"

#define FSQUARE(COLOR,BB,X) (BB&(COLOR==WHITE?upperMaskBitboard[X]:lowerMaskBitboard[X]))&~(rankBB[squareRank[X]])

#define PASSEDMASK(COLOR, X) (FSQUARE(COLOR, (fileBB[squareFile[X]] | neighborFiles[X]), X))

const int DONE_CASTLE_BONUS=       +10;

const int DOUBLED_PAWN_PENALTY =   -10;
const int ISOLATED_PAWN_PENALTY =  -15;
const int BACKWARD_PAWN_PENALTY =  -5;
const int BISHOP_PAIR_BONUS = 	   +15;


const int knightMobilityBonus[maxGamePhase+1] = {6,6,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,4,4,4,4,3,3,3,3,3,3,2,2,2,1,1,1};
const int bishopMobilityBonus[maxGamePhase+1] = {4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,1,1,1};
const int rookMobilityBonus[maxGamePhase+1] =   {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1};

const int bishopKingBonus[maxGamePhase+1] = {1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3};
const int rookKingBonus[maxGamePhase+1] =   {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4};
const int queenKingBonus[maxGamePhase+1] =  {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5};

const int minorKingZoneAttackBonus[maxGamePhase+1] = {2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,5,5,5,5,6,6,6,6,6,7,7,7,7,7,7};
const int majorKingZoneAttackBonus[maxGamePhase+1] = {4,4,4,4,5,5,5,5,5,6,6,6,6,6,7,7,7,8,8,8,9,9,10,10,10,10,10,10,10,10,10,10,10};

const int passedPawnBonus[ALL_PIECE_COLOR][ALL_RANK] = {
		{0,25,30,35,40,50,70,0},
		{0,70,50,40,35,30,25,0},
		{0,0,0,0,0,0,0,0}
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
		{
				EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,
				EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,
				EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,
				EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB,EMPTY_BB
		}

};

// opening and middlegame piece square table
const int defaultPieceSquareTable[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE]={

		{ // white pawn
				0,  0,  0,  0,  0,  0,  0,  0,
				5,  10, 10,-20,-20, 10, 10, 5,
				5, -5,-10,  0,  0, -10, -5, 5,
				0,  -5,-10, 20, 20, -10,-5,  0,
				0,  0, 0, 5, 	5, 0,  0,  0,
				10, 10, 10, 10, 10, 10, 10, 10,
				50, 50, 50, 50, 50,  50, 50, 50,
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
				50, 50, 50, 50, 50,  50, 50, 50,
				10, 10, 10, 10, 10, 10, 10, 10,
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
				5,  5, 10, 15, 15, 10,  5,  5,
				25, 25, 25, 25, 25, 25, 25, 25,
				50, 50, 50, 50, 50,  50, 50,50,
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
				50, 50, 50, 50, 50,  50, 50,50,
				25, 25, 25, 25, 25, 25, 25, 25,
				5,  5, 10, 15, 15, 10,  5,  5,
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
				0, 10, 20, 30, 30, 20, 10,  0
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


#endif /* DATA_H_ */
