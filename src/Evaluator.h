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
 * Evaluator.h
 *
 *  Created on: Jan 1, 2010
 *      Author: bhlangonijr
 */

#ifndef EVALUATOR_H_
#define EVALUATOR_H_

#include <string.h>
#include "Inline.h"
#include "Board.h"

static const int defaultMaterialValues[ALL_PIECE_TYPE_BY_COLOR] = {100, 325, 330, 500, 900, 20000, 100, 325, 330, 500, 900, 20000, 0};
static const int endGameMaterialValues[ALL_PIECE_TYPE_BY_COLOR] = {120, 310, 310, 540, 930, 20000, 120, 310, 310, 540, 930, 20000, 0};

// opening and middlegame piece square table
static const int defaultPieceSquareTable[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE]={

		{ // white pawn
		0,  0,  0,  0,  0,  0,  0,  0,
		5,  10, 10,-20,-20, 10, 10, 5,
		5, -5,-10,  0,  0, -10, -5, 5,
		0,  0,  0, 20, 20,  0,  0,  0,
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
		10, 10, 20, 30, 30, 20, 10, 10,
		5,  5, 10, 25, 25, 10,  5,  5,
		0,  0,  0, 20, 20,  0,  0,  0,
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
static const int endGamePieceSquareTable[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE]={

		{ // white pawn
		0,  0,  0,  0,  0,  0,  0,  0,
		5,  10, 10,-20,-20, 10, 10, 5,
		5, -5,-10,  0,  0, -10, -5, 5,
		0,  0,  0, 20, 20,  0,  0,  0,
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
		-20,-15,-10,-10,-10,-10,-15,-20,
		-15, -5,  0,  0,  0,  0, -5,-11,
		-15,  0,  5,  5,  5,  5,  0,-15,
		-8,  0,  5, 12, 12,  5,  0, -8,
		-8,  0,  5, 12, 12,  5,  0, -8,
		-15,  0,  5,  5,  5,  5,  0,-15,
		-15, -5,  0,  0,  0,  0, -5,-11,
		-20,-15,-10,-10,-10,-10,-15,-20,
		},
  	    {//black pawn
		0,  0,  0,  0,  0,  0,  0,  0,
		50, 50, 50, 50, 50, 50, 50, 50,
		10, 10, 20, 30, 30, 20, 10, 10,
		5,  5, 10, 25, 25, 10,  5,  5,
		0,  0,  0, 20, 20,  0,  0,  0,
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
		-20,-15,-10,-10,-10,-10,-15,-20,
		-15, -5,  0,  0,  0,  0, -5,-11,
		-15,  0,  5,  5,  5,  5,  0,-15,
		-8,  0,  5, 12, 12,  5,  0, -8,
		-8,  0,  5, 12, 12,  5,  0, -8,
		-15,  0,  5,  5,  5,  5,  0,-15,
		-15, -5,  0,  0,  0,  0, -5,-11,
		-20,-15,-10,-10,-10,-10,-15,-20,		},
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


class Evaluator {
public:


	enum GamePhase {
		OPENING, MIDDLEGAME, ENDGAME
	};

	Evaluator();
	virtual ~Evaluator();
	const int evaluate(Board& board);
	const int evalMaterial(Board& board);
	const int evalKing(Board& board, PieceColor color);
	const int evalMobility(Board& board, PieceColor color);
	const int evalDevelopment(Board& board);
	const void setGameStage(const GamePhase phase);
	const GamePhase getGameStage(Board& board);

	inline const int getPieceMaterialValue(const PieceTypeByColor piece) {
		return materialValues[piece];
	}

	inline void setPieceMaterialValue(const PieceTypeByColor piece, const int value) {
		materialValues[piece]=value;
	}


private:
	int materialValues[ALL_PIECE_TYPE_BY_COLOR];
	int pieceSquareTable[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE];

};

// main eval function
inline const int Evaluator::evaluate(Board& board) {

	if 	(board.isDraw()) {
		return 0;
	}

	int material = 0;
	int mobility = 0;
	int kingSafety = 0;
	int development = 0;

	PieceColor side = board.getSideToMove();
	PieceColor other = board.flipSide(board.getSideToMove());

	material = evalMaterial(board);
	mobility = evalMobility(board, side) - evalMobility(board, other);
	development = evalDevelopment(board);
	kingSafety = evalKing(board, side) - evalKing(board, other);
	// ...
	//std::cout << "material: " << material << std::endl;
	//std::cout << "mobility: " << mobility << std::endl;
	return material+mobility+kingSafety+development;
}

// material eval function
inline const int Evaluator::evalMaterial(Board& board) {

	PieceColor side = board.getSideToMove();

	int whiteMaterial = 0;
	int blackMaterial = 0;

	for(int pieceType = WHITE_PAWN; pieceType <= WHITE_KING; pieceType++) {
		whiteMaterial += board.getPieceCountByType(PieceTypeByColor(pieceType)) * materialValues[pieceType];
	}

	for(int pieceType = BLACK_PAWN; pieceType <= BLACK_KING; pieceType++) {
		blackMaterial += board.getPieceCountByType(PieceTypeByColor(pieceType)) * materialValues[pieceType];
	}

	return side==WHITE?whiteMaterial-blackMaterial : blackMaterial-whiteMaterial;
}

// king eval function
inline const int Evaluator::evalKing(Board& board, PieceColor color) {

	static const int DONE_CASTLE_BONUS=10;
	static const int CAN_CASTLE_BONUS=5;

	PieceColor other = board.flipSide(color);

	if (board.getPiecesByType(board.makePiece(other,QUEEN))) {
		if (board.isCastleDone(color)) {
			return DONE_CASTLE_BONUS;
		} else if (board.getCastleRights(color)!=NO_CASTLE) {
			return CAN_CASTLE_BONUS;
		} else {
			return -DONE_CASTLE_BONUS;
		}
	}

	return 0;
}

// mobility eval function
inline const int Evaluator::evalMobility(Board& board, PieceColor color) {

	Bitboard pieces = EMPTY_BB;
	PieceColor other = board.flipSide(color);
	Square from = NONE;
	Bitboard attacks = EMPTY_BB;
	int kingAttacks = 0;

	int count=0;

	Bitboard king = board.getPiecesByType(board.makePiece(other,KING));

	pieces = board.getPiecesByType(board.makePiece(color,KNIGHT));
	from = extractLSB(pieces);

	attacks = EMPTY_BB;
	while ( from!=NONE ) {

		attacks |= board.getKnightAttacks(from);
		from = extractLSB(pieces);
	}

	count+=_BitCount(attacks);

	if (king & attacks) {
		kingAttacks++;
	}

	pieces = board.getPiecesByType(board.makePiece(color,BISHOP));
	from = extractLSB(pieces);

	attacks = EMPTY_BB;
	while ( from!=NONE ) {

		attacks |= board.getBishopAttacks(from);
		from = extractLSB(pieces);
	}

	count+=_BitCount(attacks);

	if (king & attacks) {
		kingAttacks++;
	}

	pieces = board.getPiecesByType(board.makePiece(color,ROOK));
	from = extractLSB(pieces);

	attacks = EMPTY_BB;
	while ( from!=NONE ) {
		attacks |= board.getRookAttacks(from);
		from = extractLSB(pieces);
	}

	count+=_BitCount(attacks);

	if (king & attacks) {
		kingAttacks++;
	}

	pieces = board.getPiecesByType(board.makePiece(color,QUEEN));
	from = extractLSB(pieces);

	attacks = EMPTY_BB;
	while ( from!=NONE ) {
		attacks |= board.getQueenAttacks(from);
		from = extractLSB(pieces);
	}

	count+=_BitCount(attacks);

	if (king & attacks) {
		kingAttacks++;
	}

	return count + kingAttacks;
}

// material eval function
inline const int Evaluator::evalDevelopment(Board& board) {

	int whiteBonus = 0;
	int blackBonus = 0;

	PieceColor side = board.getSideToMove();

	for(int pieceType = WHITE_PAWN; pieceType <= WHITE_KING; pieceType++) {
		Bitboard pieces = board.getPiecesByType(PieceTypeByColor(pieceType));
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {
			whiteBonus += pieceSquareTable[pieceType][from];
			from = extractLSB(pieces);
		}
	}

	for(int pieceType = BLACK_PAWN; pieceType <= BLACK_KING; pieceType++) {
		Bitboard pieces = board.getPiecesByType(PieceTypeByColor(pieceType));
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {
			blackBonus += pieceSquareTable[pieceType][from];
			from = extractLSB(pieces);
		}
	}

/* why is not this code faster than the above one?
	for(int square = A1; square <= H8; square++) {
		PieceTypeByColor pieceType = board.getPieceBySquare(Square(square));
		if (pieceType!=EMPTY) {
			if (board.getPieceColor(pieceType)==WHITE) {
				whiteBonus += pieceSquareTable[pieceType][square];
			} else {
				blackBonus += pieceSquareTable[pieceType][square];
			}
		}
	}
*/
	return side==WHITE?whiteBonus-blackBonus : blackBonus-whiteBonus;
}


#endif /* EVALUATOR_H_ */
