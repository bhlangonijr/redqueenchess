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
#include <stdlib.h>
#include <math.h>
#include "inline.h"
#include "board.h"
#include "data.h"

const int DONE_CASTLE_BONUS=       +10;
const int DOUBLED_PAWN_PENALTY =   -10;
const int ISOLATED_PAWN_PENALTY =  -15;
const int BACKWARD_PAWN_PENALTY =  -5;

const int KNIGHT_MOBILITY_BONUS = 	6;
const int BISHOP_MOBILITY_BONUS = 	4;
const int ROOK_MOBILITY_BONUS = 	2;

const int KNIGHT_TROPISM_BONUS = 	1;
const int BISHOP_TROPISM_BONUS = 	1;
const int ROOK_TROPISM_BONUS = 		2;
const int QUEEN_TROPISM_BONUS = 	3;

const int KNIGHT_ATTACK_BONUS = 	4;
const int BISHOP_ATTACK_BONUS = 	3;
const int ROOK_ATTACK_BONUS = 		5;
const int QUEEN_ATTACK_BONUS = 		7;

const int SMALL_KING_SQUARE_ATTACK_BONUS = 	5;
const int BIG_KING_SQUARE_ATTACK_BONUS = 	7;

const int BISHOP_PAIR_BONUS = 15;

class Evaluator {
public:

	Evaluator();
	virtual ~Evaluator();
	const int evaluate(Board& board);
	const int quickEvaluate(Board& board);
	const int evalMaterial(Board& board, PieceColor color);
	const int evalPieces(Board& board, PieceColor color);
	const int evalMobility(Board& board, PieceColor color);
	const int evalDevelopment(Board& board, PieceColor color);
	const int evalImbalances(Board& board, PieceColor color);
	const bool isPawnPassed(Board& board, const PieceColor color, const Square from);
	const void setGameStage(const GamePhase phase);
	const int seeSign(Board& board, MoveIterator::Move& move);
	const int see(Board& board, MoveIterator::Move& move);

	inline const int interpolate(const int first, const int second, const int position) {
		return (first*position)/maxGamePhase+(second*(maxGamePhase-position))/maxGamePhase;
	}

	inline const int getPieceSquareValue(const PieceTypeByColor piece, const Square square, GamePhase phase) {
		const int egValue = endGamePieceSquareTable[piece][square];
		const int mgValue = defaultPieceSquareTable[piece][square];
		return interpolate(mgValue,egValue,phase);
	}

private:
	Bitboard getLeastValuablePiece(Board& board, Bitboard attackers, PieceColor& color, PieceTypeByColor& piece);
};



// verify if pawn is passer
inline const bool Evaluator::isPawnPassed(Board& board, const PieceColor color, const Square from) {
	const Bitboard pawns = board.getPiecesByType(board.makePiece(board.flipSide(color),PAWN));

	if (!pawns) {
		return true;
	}

	return !(passedMask[color][from]&pawns);
}

// static exchange evaluation sign
inline const int Evaluator::seeSign(Board& board, MoveIterator::Move& move) {

	if (defaultMaterialValues[board.getPieceBySquare(move.from)] <=
			defaultMaterialValues[board.getPieceBySquare(move.to)] &&
			board.getPieceType(board.getPieceBySquare(move.from)) != KING) {
		return 1;
	}

	return this->see(board,move);
}

// static exchange evaluation
inline const int Evaluator::see(Board& board, MoveIterator::Move& move) {

	const int gainTableSize=32;
	PieceColor side = board.getPieceColorBySquare(move.from);
	PieceColor other = board.flipSide(side);
	PieceTypeByColor firstPiece = board.getPieceBySquare(move.from);
	PieceTypeByColor secondPiece = board.getPieceBySquare(move.to);

	if (secondPiece==EMPTY) {
		return 0;
	}

	const Bitboard sidePawn =  board.getPiecesByType(board.makePiece(side,PAWN));
	const Bitboard sideKnight =  board.getPiecesByType(board.makePiece(side,KNIGHT));
	const Bitboard sideBishop =  board.getPiecesByType(board.makePiece(side,BISHOP));
	const Bitboard sideRook =  board.getPiecesByType(board.makePiece(side,ROOK));
	const Bitboard sideQueen =  board.getPiecesByType(board.makePiece(side,QUEEN));
	const Bitboard sideKing =  board.getPiecesByType(board.makePiece(side,KING));

	const Bitboard otherPawn =  board.getPiecesByType(board.makePiece(other,PAWN));
	const Bitboard otherKnight =  board.getPiecesByType(board.makePiece(other,KNIGHT));
	const Bitboard otherBishop =  board.getPiecesByType(board.makePiece(other,BISHOP));
	const Bitboard otherRook =  board.getPiecesByType(board.makePiece(other,ROOK));
	const Bitboard otherQueen =  board.getPiecesByType(board.makePiece(other,QUEEN));
	const Bitboard otherKing =  board.getPiecesByType(board.makePiece(other,KING));

	const Bitboard bishopAttacks =  board.getBishopAttacks(move.to);
	const Bitboard rookAttacks =  board.getRookAttacks(move.to);
	const Bitboard knightAttacks =  board.getKnightAttacks(move.to);
	const Bitboard pawnAttacks =  board.getPawnAttacks(move.to);
	const Bitboard kingAttacks =  board.getKingAttacks(move.to);

	const Bitboard rooks = sideRook | otherRook;
	const Bitboard bishops = sideBishop | otherBishop;
	const Bitboard queens = sideQueen | otherQueen;

	const Bitboard bishopAndQueen =  rooks | queens;
	const Bitboard rookAndQueen =  bishops | queens;

	Bitboard occupied = board.getAllPieces();

	Bitboard attackers =
			(bishopAttacks & bishopAndQueen) |
			(rookAttacks & rookAndQueen) |
			(knightAttacks & (sideKnight | otherKnight)) |
			(pawnAttacks & (sidePawn | otherPawn)) |
			(kingAttacks & (sideKing | otherKing));


	int idx = 0;
	int gain[gainTableSize];
	Bitboard fromPiece = squareToBitboard[move.from];
	PieceColor sideToMove = side;
	Bitboard allAttackers = EMPTY_BB;

	gain[idx] = defaultMaterialValues[secondPiece];

	if (board.getPieceType(secondPiece)==KING) {
		return queenValue*10;
	}

	while (true) {

		allAttackers |= attackers;
		idx++;
		gain[idx]  = defaultMaterialValues[firstPiece] - gain[idx-1];
		attackers ^= fromPiece;
		occupied  ^= fromPiece;
		Bitboard moreAttackers = (bishopAndQueen | rookAndQueen) & (~allAttackers);

		if (moreAttackers) {
			Bitboard findMoreAttackers =
					(board.getBishopAttacks(move.to,occupied) |
							board.getRookAttacks(move.to,occupied));
			findMoreAttackers &= moreAttackers;

			if (findMoreAttackers) {
				attackers |= findMoreAttackers;
			}
		}

		sideToMove=board.flipSide(sideToMove);
		fromPiece  = getLeastValuablePiece (board, attackers, sideToMove, firstPiece);

		if (!fromPiece) {
			break;
		}
	}

	while (--idx) {
		gain[idx-1]= -MAX(-gain[idx-1], gain[idx]);
	}

	return gain[0];
}

inline Bitboard Evaluator::getLeastValuablePiece(Board& board, Bitboard attackers, PieceColor& color, PieceTypeByColor& piece) {

	const int first = board.makePiece(color,PAWN);
	const int last = board.makePiece(color,KING);

	for(register int pieceType = first; pieceType <= last; pieceType++) {
		Bitboard pieces = board.getPiecesByType(PieceTypeByColor(pieceType)) & attackers;
		if (pieces) {
			piece = PieceTypeByColor(pieceType);
			return pieces & -pieces;
		}
	}

	return EMPTY_BB;
}

#endif /* EVALUATOR_H_ */

