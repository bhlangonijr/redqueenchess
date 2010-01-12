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

static const int defaultMaterialValues[ALL_PIECE_TYPE_BY_COLOR] = {100, 325, 325, 500, 975, 10000, 100, 325, 325, 500, 975, 10000, 0};

class Evaluator {
public:

	Evaluator();
	virtual ~Evaluator();
	const int evaluate(Board& board);
	const int evalMaterial(Board& board);
	const int evalMobility(Board& board, PieceColor color);

	inline const int getPieceMaterialValue(const PieceTypeByColor piece) {
		return materialValues[piece];
	}

	inline void setPieceMaterialValue(const PieceTypeByColor piece, const int value) {
		materialValues[piece]=value;
	}


private:
	int materialValues[ALL_PIECE_TYPE_BY_COLOR];

};

// main eval function
inline const int Evaluator::evaluate(Board& board) {

/*	if (board.isDraw()) {
		return 0;
	}*/

	int material = 0;
	int mobility = 0;
	material = evalMaterial(board);

	if (!(board.getPiecesByType(WHITE_PAWN)|board.getPiecesByType(BLACK_PAWN))) {
		if (material <= materialValues[WHITE_KNIGHT]) {
			return 0;
		}
	}

	mobility = evalMobility(board, board.getSideToMove()) - evalMobility(board, board.flipSide(board.getSideToMove()));
	// ...
	//std::cout << "material: " << material << std::endl;
	//std::cout << "mobility: " << mobility << std::endl;
	return material+mobility;
}

// material eval function
inline const int Evaluator::evalMaterial(Board& board) {

	int result = 0;
	PieceColor side = board.getSideToMove();

	int whiteMaterial = 0;
	int blackMaterial = 0;

	for(int pieceType = WHITE_PAWN; pieceType <= WHITE_KING; pieceType++) {
		whiteMaterial += board.getPieceCountByType(PieceTypeByColor(pieceType)) * materialValues[pieceType];
	}

	for(int pieceType = BLACK_PAWN; pieceType <= BLACK_KING; pieceType++) {
		blackMaterial += board.getPieceCountByType(PieceTypeByColor(pieceType)) * materialValues[pieceType];
	}

	result = side==WHITE?whiteMaterial-blackMaterial : blackMaterial-whiteMaterial;
	return result;
}

// mobility eval function
inline const int Evaluator::evalMobility(Board& board, PieceColor color) {

	Bitboard pieces = EMPTY_BB;
	Square from = NONE;
	int count=0;


	pieces = board.getPiecesByType(board.makePiece(color,KNIGHT));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		count+=_BitCount(board.getKnightAttacks(from));
		from = extractLSB(pieces);
	}

	pieces = board.getPiecesByType(board.makePiece(color,BISHOP));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		count+=_BitCount(board.getBishopAttacks(from));
		from = extractLSB(pieces);
	}

	pieces = board.getPiecesByType(board.makePiece(color,ROOK));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		count+=_BitCount(board.getRookAttacks(from));
		from = extractLSB(pieces);
	}

	pieces = board.getPiecesByType(board.makePiece(color,QUEEN));
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		count+=_BitCount(board.getQueenAttacks(from));
		from = extractLSB(pieces);
	}

	return count;
}

#endif /* EVALUATOR_H_ */
