/*
	Redqueen Chess Engine
    Copyright (C) 2008-2 10 Ben-Hur Carlos Vieira Langoni Junior

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
 *  Created on: 26/06/2 10
 *      Author: bhlangonijr
 */

#ifndef DATA_H_
#define DATA_H_

#include "bitboard.h"

// opening and middlegame piece square table
const int defaultPieceSquareTable[ALL_PIECE_TYPE][ALL_SQUARE]={
		{//pawn
				 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
				 -5, 10, 10,-20,-20, 10, 10, -5,
				 -5, -5,-10, 0 , 0 ,-10, -5, -5,
				 0 , -5,-10, 20, 20,-10, -5, 0 ,
				 0 , 0 , 0 , 5 , 5 , 0 , 0 , 0 ,
				 10, 10, 10, 10, 10, 10, 10, 10,
				 50, 50, 50, 50, 50, 50, 50, 50,
				 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
		},
		{//knight
				-50,-40,-30,-30,-30,-30,-40,-50,
				-40,-20, 0 , 5 , 5 , 0 ,-20,-40,
				-30, 5 , 10, 15, 15, 10, 5 ,-30,
				-30, 0 , 15, 20, 20, 15, 0 ,-30,
				-30, 5 , 15, 20, 20, 15, 5 ,-30,
				-30, 0 , 10, 15, 15, 10, 0 ,-30,
				-40,-20, 0 , 0 , 0 , 0 ,-20,-40,
				-50,-40,-30,-30,-30,-30,-40,-50,
		},
		{//bishop
				-20,-10,-10,-10,-10,-10,-10,-20,
				-10, 5 , 0 , 0 , 0 , 0 , 5 ,-10,
				-10, 10, 10, 10, 10, 10, 10,-10,
				-10, 0 , 10, 10, 10, 10, 0 ,-10,
				-10, 5 , 5 , 10, 10, 5 , 5 ,-10,
				-10, 0 , 5 , 10, 10, 5 , 0 ,-10,
				-10, 0 , 0 , 0 , 0 , 0 , 0 ,-10,
				-20,-10,-10,-10,-10,-10,-10,-20,
		},
		{//rook
				-15, 0 , 0 , 0 , 0 , 0 , 0 ,-15,
				-15, 0 , 0 , 0 , 0 , 0 , 0 ,-15,
				-15, 0 , 0 , 0 , 0 , 0 , 0 ,-15,
				-15, 0 , 0 , 0 , 0 , 0 , 0 ,-15,
				-15, 0 , 0 , 0 , 0 , 0 , 0 ,-15,
				-15, 0 , 0 , 0 , 0 , 0 , 0 ,-15,
				 5 , 10, 10, 10, 10, 10, 10, 5 ,
				 0 , 0 , 0 , 5 , 5 , 0 , 0 , 0 ,
		},
		{//queen
				 -5,-5 ,-5 ,-5 ,-5 ,-5 ,-5 ,-5 ,
				 -5, 5 , 5 , 5 , 5 , 5 , 5 ,-5 ,
				 -5, 5 , 5 , 5 , 5 , 5 , 5 ,-5 ,
				 -5, 5 , 5 ,10 ,10 , 5 , 5 ,-5 ,
				 -5, 5 , 5 ,10 ,10 , 5 , 5 ,-5 ,
				 -5, 5 , 5 , 5 , 5 , 5 , 5 ,-5 ,
				 -5, 5 , 5 , 5 , 5 , 5 , 5 ,-5 ,
				 -5,-5 ,-5 ,-5 ,-5 ,-5 ,-5 ,-5 ,
		},
		{//king
				 20, 30, 10, 0 , 0 , 10, 30, 20,
				 20, 20, 0 , 0 , 0 , 0 , 20, 20,
				-10,-20,-20,-20,-20,-20,-20,-10,
				-20,-30,-30,-40,-40,-30,-30,-20,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
				-30,-40,-40,-50,-50,-40,-40,-30,
		},
		{}

};

// end game piece square table
const int endGamePieceSquareTable[ALL_PIECE_TYPE][ALL_SQUARE]={

		{ //pawn
				 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
				 0 ,  1, 1 , 1 , 1 , 1 ,  1, 0 ,
				 0 ,  1, 1 , 10, 10, 1 ,  1, 0 ,
				 0 ,  1, 10, 15, 15, 10,  1, 0 ,
				 0 ,  1, 10, 20, 20, 10,  1, 0 ,
				 10, 10, 20, 25, 25, 20, 10, 10,
				 50, 50, 50, 50, 50, 50, 50, 50,
				 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
		},
		{//knight
				-50,-40,-30,-30,-30,-30,-40,-50,
				-40,-20, 0 , 0 , 0 , 0 ,-20,-40,
				-30, 0 , 5 , 10, 10, 5 , 0 ,-30,
				-30, 0 , 10, 15, 15, 10, 0 ,-30,
				-30, 0 , 10, 15, 15, 10, 0 ,-30,
				-30, 0 , 5 , 10, 10, 5 , 0 ,-30,
				-40,-20, 0 , 5 , 5 , 0 ,-20,-40,
				-50,-40,-30,-30,-30,-30,-40,-50,
		},
		{//bishop
				-20,-10,-10,-10,-10,-10,-10,-20,
				-10, 5 , 0 , 0 , 0 , 0 , 5 ,-10,
				-10, 10, 10, 10, 10, 10, 10,-10,
				-10, 0 , 10, 10, 10, 10, 0 ,-10,
				-10, 5 , 5 , 10, 10, 5 , 5 ,-10,
				-10, 0 , 5 , 10, 10, 5 , 0 ,-10,
				-10, 0 , 0 , 0 , 0 , 0 , 0 ,-10,
				-20,-10,-10,-10,-10,-10,-10,-20,
		},
		{//rook
				 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 ,
				 5 , 6 , 6 , 6 , 6 , 6 , 6 , 5 ,
				 5 , 6 , 5 , 5 , 5 , 5 , 6 , 5 ,
				 5 , 6 , 5 , 5 , 5 , 5 , 6 , 5 ,
				 5 , 6 , 5 , 5 , 5 , 5 , 6 , 5 ,
				 5 , 6 , 5 , 5 , 5 , 5 , 6 , 5 ,
				 5 , 6 , 6 , 6 , 6 , 6 , 6 , 5 ,
				 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 ,
		},
		{//queen
				-20,-10,-10, -5, -5,-10,-10,-20,
				-10, 0 , 0 , 0 , 0 , 0 , 0 ,-10,
				-10, 0 , 5 , 5 , 5 , 5 , 0 ,-10,
				 -5, 0 , 5 , 15, 15, 5 , 0 , -5,
				 -5, 0 , 5 , 15, 15, 5 , 0 , -5,
				-10, 5 , 5 , 5 , 5 , 5 , 0 ,-10,
				-10, 0 , 5 , 0 , 0 , 0 , 0 ,-10,
				-20,-10,-10, -5, -5,-10,-10,-20,
		},
		{//king
				 0 , 10, 20, 30, 30, 20, 10, 0 ,
				 10, 20, 30, 40, 40, 30, 20, 10,
				 20, 30, 40, 50, 50, 40, 30, 20,
				 30, 40, 50, 60, 60, 50, 40, 30,
				 30, 40, 50, 60, 60, 50, 40, 30,
				 20, 30, 40, 50, 50, 40, 30, 20,
				 10, 20, 30, 40, 40, 30, 20, 10,
				 0 , 10, 20, 30, 30, 20, 10, 0 ,
		},
		{}
};


#endif /* DATA_H_ */
