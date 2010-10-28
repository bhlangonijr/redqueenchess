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
 * Evaluator.cpp
 *
 *  Created on: Jan 1, 2010
 *      Author: bhlangonijr
 */

#include "evaluator.h"

Evaluator::Evaluator() {

}

Evaluator::~Evaluator() {

}

// main eval function
const int Evaluator::evaluate(Board& board, const int& alpha, const int& beta) {

	const PieceColor side = board.getSideToMove();
	const PieceColor other = board.flipSide(board.getSideToMove());

	int value = board.getMaterial(side) - board.getMaterial(other);
	value += board.getPieceSquareValue(side)-board.getPieceSquareValue(other);
	value += evalImbalances(board, side) - evalImbalances(board, other);

	if (value > alpha-lazyEvalMargin && value < beta+lazyEvalMargin) {
		value += evalPieces(board, side) - evalPieces(board, other);
		int kingThreatSide=0;
		int kingThreatOther=0;
		value += evalBoardControl(board, side, kingThreatSide) -
				evalBoardControl(board, other, kingThreatOther);
		value += kingThreatSide-kingThreatOther;
	}

	if (value>maxScore) {
		value=maxScore;
	} else if (value<-maxScore) {
		value=-maxScore;
	}

	return value;
}

// king eval function
const int Evaluator::evalPieces(Board& board, PieceColor color) {

	const PieceColor other = board.flipSide(color);
	int count=0;

	// king
	if (board.isCastleDone(color) &&
			board.getGamePhase() < ENDGAME) {
		count+= DONE_CASTLE_BONUS +
				board.getPiecesByType(board.makePiece(other,QUEEN)) ? 10 : 0;
	}

	const Bitboard pawns = board.getPiecesByType(board.makePiece(color,PAWN));

	//pawns - penalyze doubled & isolated pawns
	if (pawns) {
		Bitboard pieces=pawns;
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {
			const Bitboard pawn = squareToBitboard[from];
			const Bitboard allButThePawn =(pawns^pawn);
			bool pawnProblem = true;
			if (fileAttacks[squareFile[from]]&allButThePawn) {
				count += DOUBLED_PAWN_PENALTY;
			}
			if (!(neighborFiles[from]&allButThePawn)) {
				count += ISOLATED_PAWN_PENALTY;
			} else if (!(adjacentSquares[from]&allButThePawn)) {
				count += BACKWARD_PAWN_PENALTY;
			} else {
				pawnProblem = false;
			}
			if (isPawnPassed(board,from)) {
				count += (pawnProblem && board.getGamePhase()<ENDGAME?
						passedPawnBonus2[color][squareRank[from]]:
						passedPawnBonus1[color][squareRank[from]]);
			}

			from = extractLSB(pieces);
		}
	}

	return count;
}

// mobility eval function
const int Evaluator::evalBoardControl(Board& board, PieceColor color, int& kingThreat) {

	const PieceColor other = board.flipSide(color);
	const Square otherKingSq = board.getKingSquare(other);
	const Bitboard otherKingSquareBB = adjacentSquares[otherKingSq];
	const Bitboard knights = board.getPiecesByType(board.makePiece(color,KNIGHT));
	const Bitboard bishops = board.getPiecesByType(board.makePiece(color,BISHOP));
	const Bitboard rooks = board.getPiecesByType(board.makePiece(color,ROOK));
	const Bitboard queens = board.getPiecesByType(board.makePiece(color,QUEEN));
	const Bitboard allPieces = board.getAllPieces();
	const int phase = int(board.getGamePhase());

	Bitboard pieces = EMPTY_BB;
	Bitboard knightAttacks = EMPTY_BB;
	Bitboard bishopAttacks = EMPTY_BB;
	Bitboard rookAttacks = EMPTY_BB;
	Bitboard queenAttacks = EMPTY_BB;
	Square from = NONE;
	int count=0;
	kingThreat=0;

	pieces = knights;
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = board.getKnightAttacks(from);
		count+=(_BitCount(attacks&~allPieces)-4)*
				knightMobilityBonus[phase];
		knightAttacks |= attacks;
		from = extractLSB(pieces);
	}

	pieces = bishops;
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const int delta = inverseSquareDistance(from,otherKingSq);
		const Bitboard attacks = board.getBishopAttacks(from);
		count+=(_BitCount(attacks)-6)*
				bishopMobilityBonus[phase];
		bishopAttacks |= attacks;
		kingThreat += delta*bishopKingBonus[phase];
		from = extractLSB(pieces);
	}

	pieces = rooks;
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const int delta = inverseSquareDistance(from,otherKingSq);
		const Bitboard attacks = board.getRookAttacks(from);
		rookAttacks |= attacks;
		count+=(_BitCount(attacks)-7)*rookMobilityBonus[phase];
		kingThreat += delta*rookKingBonus[phase];
		from = extractLSB(pieces);
	}

	pieces = queens;
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const int delta = inverseSquareDistance(from,otherKingSq);
		const Bitboard attacks = board.getQueenAttacks(from);
		queenAttacks |= attacks;
		count+=(_BitCount(attacks)-10);
		kingThreat += delta*queenKingBonus[phase];
		from = extractLSB(pieces);
	}

	if (knightAttacks&otherKingSquareBB) {
		kingThreat += minorKingZoneAttackBonus[phase];
	}

	if (bishopAttacks&otherKingSquareBB) {
		int attackCount = _BitCount(bishopAttacks&otherKingSquareBB);
		kingThreat += attackCount*minorKingZoneAttackBonus[phase];
	}

	if (rookAttacks&otherKingSquareBB) {
		int attackCount = _BitCount(rookAttacks&otherKingSquareBB);
		kingThreat += attackCount*minorKingZoneAttackBonus[phase];
	}

	if (queenAttacks&otherKingSquareBB) {
		int attackCount = _BitCount(queenAttacks&otherKingSquareBB);
		kingThreat += attackCount*majorKingZoneAttackBonus[phase];
	}

	return count;
}

// imbalances eval function
const int Evaluator::evalImbalances(Board& board, PieceColor color) {

	int count=0;

	Bitboard bishop = board.getPiecesByType(board.makePiece(color,BISHOP));

	if ((bishop & WHITE_SQUARES) && (bishop & BLACK_SQUARES)) {
		count += BISHOP_PAIR_BONUS;
	}

	// TODO implement more imbalances
	return count;
}
