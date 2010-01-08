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
 * Evaluator.cpp
 *
 *  Created on: Jan 1, 2010
 *      Author: bhlangonijr
 */

#include "Evaluator.h"

Evaluator::Evaluator() {
	memcpy(materialValues, defaultMaterialValues, ALL_PIECE_TYPE_BY_COLOR*sizeof(int));
}

Evaluator::~Evaluator() {
}

// main eval function
const int Evaluator::evaluate(Board& board) {

	int material = evalMaterial(board);
	int mobility = evalMobility(board, board.getSideToMove()) - evalMobility(board, board.flipSide(board.getSideToMove()));
	// ...
	//std::cout << "material: " << material << std::endl;
	//std::cout << "mobility: " << mobility << std::endl;
	return material+mobility;
}

// material eval function
const int Evaluator::evalMaterial(Board& board) {

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
const int Evaluator::evalMobility(Board& board, PieceColor color) {

	Bitboard pieces = EMPTY_BB;
	Square from = NONE;
	int count=0;

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

