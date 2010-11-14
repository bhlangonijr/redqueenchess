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
const int Evaluator::evaluate(Board& board, const int alpha, const int beta) {

	const PieceColor side = board.getSideToMove();
	const PieceColor other = board.flipSide(board.getSideToMove());

	int value = board.getMaterial(side) - board.getMaterial(other);
	value += board.getPieceSquareValue(side)-board.getPieceSquareValue(other);
	value += evalKing(board, side) - evalKing(board, other);
	value += evalBishops(board, side) - evalBishops(board, other);

	if (value > alpha-lazyEvalMargin && value < beta+lazyEvalMargin) {
		int kingThreatSide=0;
		int kingThreatOther=0;
		value += evalBoardControl(board, side, kingThreatSide) -
				evalBoardControl(board, other, kingThreatOther);
		value += kingThreatSide-kingThreatOther;
		value += evalPawns(board, side) - evalPawns(board, other);
	}

	if (value>maxScore) {
		value=maxScore;
	} else if (value<-maxScore) {
		value=-maxScore;
	}

	return value;
}

// king eval function
const int Evaluator::evalKing(Board& board, PieceColor color) {

	const PieceColor other = board.flipSide(color);
	int count=0;

	// king
	if (board.isCastleDone(color) &&
			board.getGamePhase() < ENDGAME) {
		count+= DONE_CASTLE_BONUS +
				board.getPiecesByType(board.makePiece(other,QUEEN)) ? 10 : 0;
	}

	return count;
}

// pawns eval function
const int Evaluator::evalPawns(Board& board, PieceColor color) {

	const PieceColor other = board.flipSide(color);
	const Square otherKingSq = board.getKingSquare(other);
	const Bitboard all = board.getAllPieces();
	const Bitboard pawns = board.getPiecesByType(board.makePiece(color,PAWN));
	const Bitboard otherPawns = board.getPiecesByType(board.makePiece(other,PAWN));
	const Bitboard pieces = pawns | otherPawns |
			board.getPiecesByType(WHITE_KING) |	board.getPiecesByType(BLACK_KING);
	const bool isPawnFinal = !(pieces^all);
	int count=0;

	//pawns - penalyze doubled, isolated and backward pawns
	if (pawns) {
		Bitboard pieces=pawns;
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {
			const Bitboard pawn = squareToBitboard[from];
			const Bitboard allButThePawn =(pawns^pawn);
			const Bitboard chainSquares = backwardPawnMask[color][from]&adjacentSquares[from];
			const bool isDoubled = (fileAttacks[squareFile[from]]&allButThePawn);
			const bool isPasser = !(passedMask[color][from]&otherPawns);
			const bool isIsolated = !(neighborFiles[from]&allButThePawn);
			const bool isChained = (chainSquares&allButThePawn);

			if (isDoubled) {
				count += DOUBLED_PAWN_PENALTY;
			}
			if (isIsolated) {
				count += ISOLATED_PAWN_PENALTY;
			} else if (isChained) {
				count += CONNECTED_PAWN_BONUS;
			}
			if (isPasser && !(isDoubled && (frontSquares[color][from]&allButThePawn))) {

				count += ((isChained && !(frontSquares[color][from]&all))?
						passedPawnBonus1[color][squareRank[from]]:
						passedPawnBonus2[color][squareRank[from]]);

				if (isPawnFinal) {
					const Rank rank = color==WHITE?RANK_8:RANK_1;
					const Square target = board.makeSquare(rank,squareFile[from]);

					const int delta1 = squareDistance(from,target);
					const int delta2 = squareDistance(otherKingSq,target);
					const int otherMove=board.getSideToMove();

					if (MIN(5,delta1)<delta2-otherMove) {
						count += rookValue;
					}
				}
			}
			if (!(isPasser | isIsolated | isChained) &&
					!(backwardPawnMask[color][from]&allButThePawn) &&
					!(board.getPawnAttacks(from,color) & otherPawns)) {
				const Square stop = color==WHITE?Square(from+8):Square(from-8);
				if (board.getPawnAttacks(stop,color) & otherPawns) {
					count += BACKWARD_PAWN_PENALTY;
				}
			}

			from = extractLSB(pieces);
		}
	}

	return count;
}

// Bishops eval function
const int Evaluator::evalBishops(Board& board, PieceColor color) {

	int count=0;

	const Bitboard bishop = board.getPiecesByType(board.makePiece(color,BISHOP));

	if ((bishop & WHITE_SQUARES) && (bishop & BLACK_SQUARES)) {
		count += BISHOP_PAIR_BONUS;
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
	const Bitboard notFriends = ~board.getPiecesByColor(color);
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
		count+=(_BitCount(attacks&notFriends)-4)*
				knightMobilityBonus[phase];
		knightAttacks |= attacks;
		from = extractLSB(pieces);
	}

	pieces = bishops;
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const int delta = inverseSquareDistance(from,otherKingSq);
		const Bitboard attacks = board.getBishopAttacks(from);
		count+=(_BitCount(attacks&notFriends)-6)*
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
		count+=(_BitCount(attacks&notFriends)-7)*rookMobilityBonus[phase];
		kingThreat += delta*rookKingBonus[phase];
		from = extractLSB(pieces);
	}

	pieces = queens;
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const int delta = inverseSquareDistance(from,otherKingSq);
		const Bitboard attacks = board.getQueenAttacks(from);
		queenAttacks |= attacks;
		count+=(_BitCount(attacks&notFriends)-10);
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
