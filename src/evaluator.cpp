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

	evalBishops(evalInfo.side, evalInfo);
	evalBishops(evalInfo.other, evalInfo);
	evalInfo.value[WHITE] +=
			evalInfo.board.isCastleDone(WHITE)?DONE_CASTLE_BONUS:-DONE_CASTLE_BONUS;
	evalInfo.value[BLACK] +=
			evalInfo.board.isCastleDone(BLACK)?DONE_CASTLE_BONUS:-DONE_CASTLE_BONUS;


	evalInfo.computeEval();

	if ((evalInfo.eval > alpha-lazyEvalMargin && evalInfo.eval < beta+lazyEvalMargin) ||
			board.isInCheck()) {
		evalInfo.attackers[WHITE_PAWN] = ((evalInfo.pawns[WHITE] & midBoardNoFileA) << 7) |
				((evalInfo.pawns[WHITE] & midBoardNoFileH) << 9);
		evalInfo.attackers[BLACK_PAWN] = ((evalInfo.pawns[BLACK] & midBoardNoFileA) >> 9) |
				((evalInfo.pawns[BLACK] & midBoardNoFileH) >> 7);
		evalBoardControl(evalInfo.side, evalInfo);
		evalBoardControl(evalInfo.other, evalInfo);
		PawnInfo info;
		if (getPawnInfo(board.getPawnKey(),info)) {
			evalPawnsFromCache(evalInfo.side, info, evalInfo);
			evalPawnsFromCache(evalInfo.other, info, evalInfo);
		} else {
			evalPawns(evalInfo.side, evalInfo);
			evalPawns(evalInfo.other, evalInfo);
		}
		evalThreats(evalInfo.side, evalInfo);
		evalThreats(evalInfo.other, evalInfo);
		evalKing(evalInfo.side, evalInfo);
		evalKing(evalInfo.other, evalInfo);
		evalInfo.computeEval();
	}

	return evalInfo.eval;
}

// king eval function
void Evaluator::evalKing(PieceColor color, EvalInfo& evalInfo) {

	const PieceColor other = evalInfo.board.flipSide(color);
	const Square kingSq = evalInfo.board.getKingSquare(color);
	const Bitboard kingSquareBB = adjacentSquares[kingSq];
	const PieceTypeByColor knight = evalInfo.board.makePiece(other,KNIGHT);
	const PieceTypeByColor bishop = evalInfo.board.makePiece(other,BISHOP);
	const PieceTypeByColor rook = evalInfo.board.makePiece(other,ROOK);
	const PieceTypeByColor queen = evalInfo.board.makePiece(other,QUEEN);

	// king area safety
	const Bitboard knightAttacks = evalInfo.attackers[knight]&kingSquareBB;
	if (knightAttacks) {
		evalInfo.kingThreat[color] -= minorKingZoneAttackWeight[1];
	}

	const Bitboard bishopAttacks = evalInfo.attackers[bishop]&kingSquareBB;
	if (bishopAttacks) {
		const int attackCount = _BitCount(bishopAttacks);
		evalInfo.kingThreat[color] -= minorKingZoneAttackWeight[attackCount];
	}

	const Bitboard rookAttacks = evalInfo.attackers[rook]&kingSquareBB;
	if (rookAttacks) {
		const int attackCount = _BitCount(rookAttacks);
		evalInfo.kingThreat[color] -= rookKingZoneAttackWeight[attackCount];
	}

	const Bitboard queenAttacks = evalInfo.attackers[queen]&kingSquareBB;
	if (queenAttacks) {
		const int attackCount = _BitCount(queenAttacks);
		evalInfo.kingThreat[color] -= queenKingZoneAttackWeight[attackCount];
	}

}

void Evaluator::evalPawnsFromCache(PieceColor color, PawnInfo& info, EvalInfo& evalInfo) {
	const PieceColor other = evalInfo.board.flipSide(color);
	const Bitboard all = evalInfo.all;
	const Bitboard pawns = evalInfo.pawns[color];
	const Bitboard otherPawns = evalInfo.pawns[other];

	evalInfo.value[color]+=info.value[color];
	Bitboard passed=info.passers[color] & pawns;
	if (passed) {
		const Bitboard pawnsAndKings = pawns | otherPawns |
				evalInfo.board.getPiecesByType(WHITE_KING) |
				evalInfo.board.getPiecesByType(BLACK_KING);
		const bool isPawnFinal = !(pawnsAndKings^all);
		Square from = extractLSB(passed);
		while ( from!=NONE ) {
			const Bitboard pawn = squareToBitboard[from];
			const Bitboard allButThePawn =(pawns^pawn);
			const Bitboard chainSquares = backwardPawnMask[color][from]&adjacentSquares[from];
			const bool isChained = (chainSquares&allButThePawn);
			evalInfo.value[color] += evalPassedPawn(evalInfo.board,color,from,isPawnFinal,isChained);
			from = extractLSB(passed);
		}
	}
}

// pawns eval function
void Evaluator::evalPawns(PieceColor color, EvalInfo& evalInfo) {

	const PieceColor other = evalInfo.board.flipSide(color);
	const Bitboard all = evalInfo.all;
	const Bitboard pawns = evalInfo.pawns[color];
	const Bitboard otherPawns = evalInfo.pawns[other];
	Bitboard passers=EMPTY_BB;
	int passedBonus=0;
	int eval=0;

	//pawns - penalyze doubled, isolated and backward pawns
	if (pawns) {
		const Bitboard pawnsAndKings = pawns | otherPawns |
				evalInfo.board.getPiecesByType(WHITE_KING) |
				evalInfo.board.getPiecesByType(BLACK_KING);
		const bool isPawnFinal = !(pawnsAndKings^all);
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
				eval += DOUBLED_PAWN_PENALTY;
			}
			if (isIsolated) {
				eval += ISOLATED_PAWN_PENALTY;
			} else if (isChained) {
				eval += CONNECTED_PAWN_BONUS;
			}
			if (isPasser && !(isDoubled && (frontSquares[color][from]&allButThePawn))) {
				passedBonus += evalPassedPawn(evalInfo.board,color,from,isPawnFinal,isChained);
				passers|=squareToBitboard[from];
			}
			if (!(isPasser | isIsolated | isChained) &&
					!(backwardPawnMask[color][from]&allButThePawn) &&
					!(evalInfo.board.getPawnAttacks(from,color) & otherPawns)) {
				const Square stop = color==WHITE?Square(from+8):Square(from-8);
				if (evalInfo.board.getPawnAttacks(stop,color) & otherPawns) {
					eval += BACKWARD_PAWN_PENALTY;
				}
			}

			from = extractLSB(pieces);
		}
	}

	setPawnInfo(evalInfo.board.getPawnKey(),eval,color,passers);

	evalInfo.value[color] += eval+passedBonus;
}

// Eval passed pawns
const int Evaluator::evalPassedPawn(Board& board, PieceColor color, const Square from,
		const bool isPawnFinal, const bool isChained) {

	int eval=0;

	const Bitboard all = board.getAllPieces();

	if ((isChained && !(frontSquares[color][from]&all))) {
		eval += connectedPasserBonus[color][squareRank[from]];
	} else {
		eval += passedPawnBonus[color][squareRank[from]];
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
void Evaluator::evalBishops(PieceColor color, EvalInfo& evalInfo) {

	const Bitboard bishop = evalInfo.board.getPiecesByType(evalInfo.board.makePiece(color,BISHOP));

	if ((bishop & WHITE_SQUARES) && (bishop & BLACK_SQUARES)) {
		evalInfo.value[color] += BISHOP_PAIR_BONUS;
	}

}

// mobility eval function
void Evaluator::evalBoardControl(PieceColor color, EvalInfo& evalInfo) {

	const PieceColor other = evalInfo.board.flipSide(color);
	const Square otherKingSq = evalInfo.board.getKingSquare(other);
	const PieceTypeByColor pawnOther = evalInfo.board.makePiece(other,PAWN);
	const PieceTypeByColor pawn = evalInfo.board.makePiece(color,PAWN);
	const PieceTypeByColor knight = evalInfo.board.makePiece(color,KNIGHT);
	const PieceTypeByColor bishop = evalInfo.board.makePiece(color,BISHOP);
	const PieceTypeByColor rook = evalInfo.board.makePiece(color,ROOK);
	const PieceTypeByColor queen = evalInfo.board.makePiece(color,QUEEN);
	const PieceTypeByColor king = evalInfo.board.makePiece(color,KING);
	const Bitboard freeArea = ~(evalInfo.board.getPiecesByColor(color) |
			evalInfo.attackers[pawnOther]);

	Bitboard pieces = EMPTY_BB;
	evalInfo.attackers[knight] = EMPTY_BB;
	evalInfo.attackers[bishop] = EMPTY_BB;
	evalInfo.attackers[rook] = EMPTY_BB;
	evalInfo.attackers[queen] = EMPTY_BB;
	Square from = NONE;
	evalInfo.kingThreat[color]=0;

	pieces = evalInfo.board.getPiecesByType(knight);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = evalInfo.board.getKnightAttacks(from);
		evalInfo.value[color] += knightMobility[_BitCount(attacks&freeArea)];
		evalInfo.attackers[knight] |= attacks;
		from = extractLSB(pieces);
	}

	pieces = evalInfo.board.getPiecesByType(bishop);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = evalInfo.board.getBishopAttacks(from);
		evalInfo.attackers[bishop] |= attacks;
		evalInfo.value[color] += bishopMobility[_BitCount(attacks&freeArea)];
		evalInfo.kingThreat[color] += bishopKingBonus[squareDistance(from,otherKingSq)];
		from = extractLSB(pieces);
	}

	pieces = evalInfo.board.getPiecesByType(rook);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = evalInfo.board.getRookAttacks(from);
		evalInfo.attackers[rook] |= attacks;
		evalInfo.value[color] += rookMobility[_BitCount(attacks&freeArea)];
		evalInfo.kingThreat[color] += rookKingBonus[squareDistance(from,otherKingSq)];
		from = extractLSB(pieces);
	}

	pieces = evalInfo.board.getPiecesByType(queen);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = evalInfo.board.getQueenAttacks(from);
		const int queenMobility = _BitCount(attacks&freeArea)-10;
		evalInfo.attackers[queen] |= attacks;
		evalInfo.value[color] += MS(queenMobility,queenMobility);
		evalInfo.kingThreat[color] += queenKingBonus[squareDistance(from,otherKingSq)];
		from = extractLSB(pieces);
	}

	// evaluate space
	const Bitboard sidePieces = evalInfo.board.getPiecesByColor(color);
	const Bitboard kingAndPawns = evalInfo.board.getPiecesByType(king) | evalInfo.pawns[color];
	const Bitboard otherSideControl = (sidePieces ^ kingAndPawns) & colorSpaceBB[other] & freeArea;
	evalInfo.value[color] += spaceBonus[_BitCount(otherSideControl)];

	evalInfo.attacks[color] = evalInfo.attackers[knight] | evalInfo.attackers[bishop] |
			evalInfo.attackers[rook] | evalInfo.attackers[queen] | evalInfo.attackers[pawn];

}

// eval threats - idea from Stockfish
void Evaluator::evalThreats(PieceColor color, EvalInfo& evalInfo) {

	const PieceColor other = evalInfo.board.flipSide(color);
	const PieceTypeByColor pawnOther = evalInfo.board.makePiece(other,PAWN);

	Bitboard pieces=evalInfo.board.getPiecesByColor(other) &
			~evalInfo.attackers[pawnOther] & evalInfo.attacks[color];

	if (pieces) {
		for (int sideType=KNIGHT;sideType<KING;sideType++) {
			const PieceTypeByColor sidePiece = evalInfo.board.makePiece(color,PieceType(sideType));
			Bitboard attacked = evalInfo.attackers[sidePiece]&pieces;
			if (attacked) {
				for (int otherType=PAWN;otherType<=KING;otherType++) {
					const PieceTypeByColor otherPiece = evalInfo.board.makePiece(other,PieceType(otherType));
					if (attacked & evalInfo.board.getPiecesByType(otherPiece)) {
						evalInfo.value[color] += threatBonus[sideType][otherType];
					}
				}
			}
		}
	}

}





