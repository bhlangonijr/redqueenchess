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

const int defaultMaterialValues[ALL_PIECE_TYPE_BY_COLOR] = {100, 318, 325, 520, 975, 10000, 100, 318, 325, 520, 975, 10000, 0};
const int endGameMaterialValues[ALL_PIECE_TYPE_BY_COLOR] = {110, 310, 325, 520, 975, 10000, 110, 310, 325, 520, 975, 10000, 0};

// opening and middlegame piece square table
const int defaultPieceSquareTable[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE]={

		{ // white pawn
				0,  0,  0,  0,  0,  0,  0,  0,
				5,  10, 10,-20,-20, 10, 10, 5,
				5, -5,-10,  0,  0, -10, -5, 5,
				0,  0,  0, 20, 20,  0,  0,  0,
				5,  5, 10, 15, 	15, 10,  5,  5,
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
				5,  5, 10, 15, 15, 10,  5,  5,
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
	const GamePhase getGameStage(Board& board);

	inline const int getPieceMaterialValue(const PieceTypeByColor piece) {
		return materialValues[piece];
	}

	inline void setPieceMaterialValue(const PieceTypeByColor piece, const int value) {
		materialValues[piece]=value;
	}


private:
	GamePhase gamePhase;
	int materialValues[ALL_PIECE_TYPE_BY_COLOR];
	int pieceSquareTable[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE];

};

// main eval function
inline const int Evaluator::evaluate(Board& board) {

	int material = 0;
	int mobility = 0;
	int pieces = 0;
	int development = 0;
	int imbalances = 0;

	PieceColor side = board.getSideToMove();
	PieceColor other = board.flipSide(board.getSideToMove());

	material = evalMaterial(board, side) - evalMaterial(board, other);
	mobility = evalMobility(board, side) - evalMobility(board, other);
	development = evalDevelopment(board, side) - evalDevelopment(board, other);
	pieces = evalPieces(board, side) - evalPieces(board, other);
	imbalances = evalImbalances(board, side) - evalImbalances(board, other);

	/*
	std::cout << "material:    " << material << std::endl;
	std::cout << "mobility:    " << mobility << std::endl;
	std::cout << "development: " << development << std::endl;
	std::cout << "pieces:      " << pieces << std::endl;
	std::cout << "--------      " << pieces << std::endl;*/


	return material+mobility+pieces+development+imbalances;
}

// material eval function
inline const int Evaluator::evalMaterial(Board& board, PieceColor color) {

	int material = 0;
	const int first = board.makePiece(color,PAWN);
	const int last = board.makePiece(color,KING);

	for(int pieceType = first; pieceType <= last; pieceType++) {
		int count = board.getPieceCountByType(PieceTypeByColor(pieceType));
		if (count > 0) {
			material += count * materialValues[pieceType];
		}
	}

	return material;
}

// king eval function
inline const int Evaluator::evalPieces(Board& board, PieceColor color) {

	const PieceColor other = board.flipSide(color);
	int count=0;

	const int DONE_CASTLE_BONUS=       (board.getPiecesByType(board.makePiece(other,QUEEN))) ? 30 : 20;
	const int CAN_CASTLE_BONUS=        5;
	const int UNSTOPPABLE_PAWN_BONUS = 20;
	const int CENTERED_PAWN_BONUS =    15;
	const int DOUBLED_PAWN_PENALTY =  -15;
	const int ISOLATED_PAWN_PENALTY = -15;
	const int BACKWARD_PAWN_PENALTY = -15;

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

	//penalyze doubled & isolated pawns
	if (pawns) {
		Bitboard pieces=pawns;
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {

			if (squareToBitboard[from]&centerSquares) {
				count += CENTERED_PAWN_BONUS;
			}

			if (fileAttacks[squareFile[from]]&pawns) {
				count += DOUBLED_PAWN_PENALTY;
			}

			Bitboard neighbor =EMPTY_BB;

			if (squareFile[from]!=FILE_H) {
				neighbor |= fileBB[squareFile[from+1]];
			}

			if (squareFile[from]!=FILE_A) {
				neighbor |= fileBB[squareFile[from-1]];
			}

			if (!(neighbor&pawns)) {
				count += ISOLATED_PAWN_PENALTY;
			} else {
				if (!(adjacentSquares[from]&pawns)) {
					count += BACKWARD_PAWN_PENALTY;
				}
			}

			neighbor |= fileAttacks[squareFile[from]];

			Bitboard frontSquares = neighbor;

			if (color==WHITE) {
				frontSquares &= bitsBetween(frontSquares,from,board.makeSquare(RANK_8,squareFile[from]));
			} else {
				frontSquares &= bitsBetween(frontSquares,board.makeSquare(RANK_1,squareFile[from]),from);
			}

			if (!(frontSquares&enemyPawns)) {
				count += UNSTOPPABLE_PAWN_BONUS;
			}

			from = extractLSB(pieces);
		}

	}

	return count;
}

// mobility eval function
inline const int Evaluator::evalMobility(Board& board, PieceColor color) {

	const int KING_ATTACK_BONUS=10;
	const PieceColor other=board.flipSide(color);

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

	moves = EMPTY_BB;
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

	Bitboard king = board.getPiecesByType(board.makePiece(other,KING));

	Square kingSquare = extractLSB(king);

	Bitboard nearKingSquares =king|adjacentSquares[kingSquare];

	count += _BitCount(board.getPiecesByColor(other)&attacks);

	count += _BitCount(nearKingSquares&attacks)*KING_ATTACK_BONUS;

	return count;
}

// material eval function
inline const int Evaluator::evalDevelopment(Board& board, PieceColor color) {

	int bonus = 0;
	const int first = board.makePiece(color,PAWN);
	const int last = board.makePiece(color,KING);

	for(int pieceType = first; pieceType <= last; pieceType++) {
		Bitboard pieces = board.getPiecesByType(PieceTypeByColor(pieceType));
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {
			bonus += pieceSquareTable[pieceType][from];
			from = extractLSB(pieces);
		}
	}

	return bonus;
}

// mobility eval function
inline const int Evaluator::evalImbalances(Board& board, PieceColor color) {

	const int bishopPairBonus = 30;
	int count=0;

	Bitboard bishop = board.getPiecesByType(board.makePiece(color,BISHOP));

	if ((bishop & WHITE_SQUARES) && (bishop & BLACK_SQUARES)) {
		count += bishopPairBonus;
	}
	// TODO implement more imbalances
	return count;
}

#endif /* EVALUATOR_H_ */
