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

	EvalInfo evalInfo(board);

	evalKing(board, evalInfo.side, evalInfo);
	evalKing(board, evalInfo.other, evalInfo);
	evalBishops(board, evalInfo.side, evalInfo);
	evalBishops(board, evalInfo.other, evalInfo);

	PawnInfo info;
	if (getPawnInfo(board.getPawnKey(),info)) {
		evalPawnsFromCache(board, evalInfo.side, info, evalInfo);
		evalPawnsFromCache(board, evalInfo.other, info, evalInfo);
	} else {
		evalPawns(board, evalInfo.side, evalInfo);
		evalPawns(board, evalInfo.other, evalInfo);
	}

	evalInfo.computeEval();

	if (evalInfo.eval > alpha-lazyEvalMargin && evalInfo.eval < beta+lazyEvalMargin) {
		evalBoardControl(board, evalInfo.side, evalInfo);
		evalBoardControl(board, evalInfo.other, evalInfo);
		evalInfo.computeEval();
	}

	return evalInfo.eval;
}

// king eval function
void Evaluator::evalKing(Board& board, PieceColor color, EvalInfo& evalInfo) {

	// king
	if (board.isCastleDone(color) &&
			board.getGamePhase() < ENDGAME) {
		evalInfo.value[color] += DONE_CASTLE_BONUS;
	}

}

void Evaluator::evalPawnsFromCache(Board& board, PieceColor color, PawnInfo& info, EvalInfo& evalInfo) {
	const PieceColor other = board.flipSide(color);
	const Bitboard all = board.getAllPieces();
	const Bitboard pawns = board.getPiecesByType(board.makePiece(color,PAWN));
	const Bitboard otherPawns = board.getPiecesByType(board.makePiece(other,PAWN));

	evalInfo.value[color]+=info.value[color];
	Bitboard passed=info.passers[color] & pawns;
	if (passed) {
		const Bitboard pawnsAndKings = pawns | otherPawns |
				board.getPiecesByType(WHITE_KING) |	board.getPiecesByType(BLACK_KING);
		const bool isPawnFinal = !(pawnsAndKings^all);
		Square from = extractLSB(passed);
		while ( from!=NONE ) {
			const Bitboard pawn = squareToBitboard[from];
			const Bitboard allButThePawn =(pawns^pawn);
			const Bitboard chainSquares = backwardPawnMask[color][from]&adjacentSquares[from];
			const bool isChained = (chainSquares&allButThePawn);
			evalInfo.value[color] += evalPassedPawn(board,color,from,isPawnFinal,isChained);
			from = extractLSB(passed);
		}
	}
}

// pawns eval function
void Evaluator::evalPawns(Board& board, PieceColor color, EvalInfo& evalInfo) {

	const PieceColor other = board.flipSide(color);
	const Bitboard all = board.getAllPieces();
	const Bitboard pawns = board.getPiecesByType(board.makePiece(color,PAWN));
	const Bitboard otherPawns = board.getPiecesByType(board.makePiece(other,PAWN));
	Bitboard passers=EMPTY_BB;
	int passedBonus=0;
	int eval=0;

	//pawns - penalyze doubled, isolated and backward pawns
	if (pawns) {
		const Bitboard pawnsAndKings = pawns | otherPawns |
				board.getPiecesByType(WHITE_KING) |	board.getPiecesByType(BLACK_KING);
		const bool isPawnFinal = !(pawnsAndKings^all);
		Bitboard pieces=pawns;
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {
			const Bitboard pawn = squareToBitboard[from];
			const Bitboard allButThePawn =(pawns^pawn);
			const Bitboard chainSquares = backwardPawnMask[color][from]&adjacentSquares[from];
			const bool isDoubled = (fileAttacks[squareFile[from]]&allButThePawn);
			const bool isPasser = !(passedMask[color][from]&otherPawns) ||
					 	 	 	   (squareToBitboard[from] & promoRank[color]);
			const bool isIsolated = !(neighborFiles[from]&allButThePawn);
			const bool isChained = (chainSquares&allButThePawn);

			if (isDoubled) {
				eval += DOUBLED_PAWN_PENALTY;
			}
			if (isIsolated) {
				eval += ISOLATED_PAWN_PENALTY;
			} else if (isChained) {
				eval += CONNECTED_PAWN_BONUS;
			}
			if (isPasser && !(isDoubled && (frontSquares[color][from]&allButThePawn))) {

				passedBonus += evalPassedPawn(board,color,from,isPawnFinal,isChained);
				passers|=squareToBitboard[from];

			}
			if (!(isPasser | isIsolated | isChained) &&
					!(backwardPawnMask[color][from]&allButThePawn) &&
					!(board.getPawnAttacks(from,color) & otherPawns)) {
				const Square stop = color==WHITE?Square(from+8):Square(from-8);
				if (board.getPawnAttacks(stop,color) & otherPawns) {
					eval += BACKWARD_PAWN_PENALTY;
				}
			}

			from = extractLSB(pieces);
		}
	}

	setPawnInfo(board.getPawnKey(),eval,color,passers);

	evalInfo.value[color] += eval+passedBonus;
}

// Eval passed pawns
const int Evaluator::evalPassedPawn(Board& board, PieceColor color, const Square from,
		const bool isPawnFinal, const bool isChained) {

	int eval=0;

	const Bitboard all = board.getAllPieces();
	eval += passedPawnBonus[color][squareRank[from]];

	if ((isChained && !(frontSquares[color][from]&all))) {
		eval += connectedPasserBonus[color][squareRank[from]];
	}

	if (isPawnFinal) {
		const Square otherKingSq = board.getKingSquare(board.flipSide(color));
		const Rank rank = color==WHITE?RANK_8:RANK_1;
		const Square target = board.makeSquare(rank,squareFile[from]);
		const int delta1 = squareDistance(from,target);
		const int delta2 = squareDistance(otherKingSq,target);
		const int otherMove=board.getSideToMove();

		if (MIN(5,delta1)<delta2-otherMove) {
			eval += UNSTOPPABLE_PAWN_BONUS;
		}
	}

	return eval;
}

// Bishops eval function
void Evaluator::evalBishops(Board& board, PieceColor color, EvalInfo& evalInfo) {

	const Bitboard bishop = board.getPiecesByType(board.makePiece(color,BISHOP));

	if ((bishop & WHITE_SQUARES) && (bishop & BLACK_SQUARES)) {
		evalInfo.value[color] += BISHOP_PAIR_BONUS;
	}

}

// mobility eval function
void Evaluator::evalBoardControl(Board& board, PieceColor color, EvalInfo& evalInfo) {

	const PieceColor other = board.flipSide(color);
	const Square otherKingSq = board.getKingSquare(other);
	const Bitboard otherKingSquareBB = adjacentSquares[otherKingSq];

	const PieceTypeByColor knight = board.makePiece(color,KNIGHT);
	const PieceTypeByColor bishop = board.makePiece(color,BISHOP);
	const PieceTypeByColor rook = board.makePiece(color,ROOK);
	const PieceTypeByColor queen = board.makePiece(color,QUEEN);
	const Bitboard notFriends = ~board.getPiecesByColor(color);

	Bitboard pieces = EMPTY_BB;
	evalInfo.attackers[knight] = EMPTY_BB;
	evalInfo.attackers[bishop] = EMPTY_BB;
	evalInfo.attackers[rook] = EMPTY_BB;
	evalInfo.attackers[queen] = EMPTY_BB;
	Square from = NONE;
	evalInfo.kingThreat[color]=0;

	pieces = board.getPiecesByType(knight);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = board.getKnightAttacks(from);
		evalInfo.value[color] += knightMobility[_BitCount(attacks&notFriends)];
		evalInfo.attackers[knight] |= attacks;
		from = extractLSB(pieces);
	}

	pieces = board.getPiecesByType(bishop);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = board.getBishopAttacks(from);
		evalInfo.attackers[bishop] |= attacks;
		evalInfo.value[color] += bishopMobility[_BitCount(attacks&notFriends)];
		evalInfo.kingThreat[color] += bishopKingBonus[squareDistance(from,otherKingSq)];
		from = extractLSB(pieces);
	}

	pieces = board.getPiecesByType(rook);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = board.getRookAttacks(from);
		evalInfo.attackers[rook] |= attacks;
		evalInfo.value[color] += rookMobility[_BitCount(attacks&notFriends)];
		evalInfo.kingThreat[color] += rookKingBonus[squareDistance(from,otherKingSq)];
		from = extractLSB(pieces);
	}

	pieces = board.getPiecesByType(queen);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = board.getQueenAttacks(from);
		const int queenMobility = _BitCount(attacks&notFriends)-10;
		evalInfo.attackers[queen] |= attacks;
		evalInfo.value[color] += MS(queenMobility,queenMobility);
		evalInfo.kingThreat[color] += queenKingBonus[squareDistance(from,otherKingSq)];
		from = extractLSB(pieces);
	}

	const Bitboard knightAttacks = evalInfo.attackers[knight]&otherKingSquareBB;
	if (knightAttacks) {
		evalInfo.kingThreat[color] += minorKingZoneAttackBonus[1];
	}

	const Bitboard bishopAttacks = evalInfo.attackers[bishop]&otherKingSquareBB;
	if (bishopAttacks) {
		const int attackCount = _BitCount(bishopAttacks);
		evalInfo.kingThreat[color] += minorKingZoneAttackBonus[attackCount];
	}

	const Bitboard rookAttacks = evalInfo.attackers[rook]&otherKingSquareBB;
	if (rookAttacks) {
		const int attackCount = _BitCount(rookAttacks);
		evalInfo.kingThreat[color] += minorKingZoneAttackBonus[attackCount];
	}

	const Bitboard queenAttacks = evalInfo.attackers[queen]&otherKingSquareBB;
	if (queenAttacks) {
		const int attackCount = _BitCount(queenAttacks);
		evalInfo.kingThreat[color] += majorKingZoneAttackBonus[attackCount];
	}
}
