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

	evalPieces(evalInfo.side, evalInfo);
	evalPieces(evalInfo.other, evalInfo);

	if (alpha != -maxScore && beta != maxScore) {
		evalInfo.computeEval();
		if ((evalInfo.eval <= alpha-lazyEvalMargin || evalInfo.eval >=beta+lazyEvalMargin) &&
				board.isInCheck()) {
			return evalInfo.eval;
		}
	}

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

	return evalInfo.eval;
}

// king eval function
void Evaluator::evalKing(PieceColor color, EvalInfo& evalInfo) {

	const PieceColor other = evalInfo.board.flipSide(color);
	const Square kingSq = evalInfo.board.getKingSquare(color);
	const Bitboard kingSquareBB = adjacentSquares[kingSq];
	const PieceTypeByColor knight = makePiece(other,KNIGHT);
	const PieceTypeByColor bishop = makePiece(other,BISHOP);
	const PieceTypeByColor rook = makePiece(other,ROOK);
	const PieceTypeByColor queen = makePiece(other,QUEEN);

	// king area safety
	const Bitboard knightAttacks = evalInfo.attackers[knight]&kingSquareBB;
	if (knightAttacks) {
		evalInfo.kingThreat[color] -= minorKingZoneAttackWeight[1];
	}

	const Bitboard bishopAttacks = evalInfo.attackers[bishop]&kingSquareBB;
	if (bishopAttacks) {
		const int attackCount = bitCount(bishopAttacks);
		evalInfo.kingThreat[color] -= minorKingZoneAttackWeight[attackCount];
	}

	const Bitboard rookAttacks = evalInfo.attackers[rook]&kingSquareBB;
	if (rookAttacks) {
		const int attackCount = bitCount(rookAttacks);
		evalInfo.kingThreat[color] -= rookKingZoneAttackWeight[attackCount];
	}

	const Bitboard queenAttacks = evalInfo.attackers[queen]&kingSquareBB;
	if (queenAttacks) {
		const int attackCount = bitCount(queenAttacks);
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
				evalInfo.board.getPieces(WHITE_KING) |
				evalInfo.board.getPieces(BLACK_KING);
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

	//penalyze doubled, isolated and backward pawns
	//bonus to passer and candidates
	if (pawns) {
		const Bitboard pawnsAndKings = pawns | otherPawns |
				evalInfo.board.getPieces(WHITE_KING) |
				evalInfo.board.getPieces(BLACK_KING);
		const bool isPawnFinal = !(pawnsAndKings^all);
		Bitboard pieces=pawns;
		Square from = extractLSB(pieces);
		while ( from!=NONE ) {
			const Bitboard pawn = squareToBitboard[from];
			const Bitboard allButThePawn =(pawns^pawn);
			const Bitboard chainSquares = backwardPawnMask[color][from]&adjacentSquares[from];
			const bool isDoubled = fileAttacks[squareFile[from]]&allButThePawn;
			const bool isPasser = !(passedMask[color][from]&otherPawns);
			const bool isIsolated = !(neighborFiles[from]&allButThePawn);
			const bool isChained = chainSquares&allButThePawn;
			const bool halfOpenFile = !(fileBB[squareFile[from]]&otherPawns);

			if (isDoubled) {
				eval += DOUBLED_PAWN_PENALTY;
			}
			if (isIsolated) {
				eval += halfOpenFile?ISOLATED_OPEN_PAWN_PENALTY:ISOLATED_PAWN_PENALTY;
			} else if (adjacentSquares[from]&allButThePawn) {
				eval += CONNECTED_PAWN_BONUS;
			}
			if (isPasser && !(isDoubled && (frontSquares[color][from]&allButThePawn))) {
				passedBonus += evalPassedPawn(evalInfo.board,color,from,isPawnFinal,isChained);
				passers|=squareToBitboard[from];
			}
			if (!(isPasser | isIsolated | isChained) &&
					!(backwardPawnMask[color][from]&allButThePawn) &&
					!(evalInfo.board.getPawnAttacks(from,color)&otherPawns)) {
				Bitboard attacks = evalInfo.board.getPawnAttacks(from,color);
				while (!(attacks&(allButThePawn|otherPawns))) {
					attacks=(color==WHITE)?attacks<<8:attacks>>8;
				}
				const Bitboard attacks1=(color==WHITE)?attacks<<8:attacks>>8;
				if ((attacks|attacks1)&otherPawns) {
					eval += halfOpenFile?BACKWARD_OPEN_PAWN_PENALTY:BACKWARD_PAWN_PENALTY;
				}
			}
			if (!isPasser && !(frontSquares[color][from]&otherPawns)) {
				const Bitboard c = (backwardPawnMask[color][from]|pawn) & pawns;
				if (c) {
					const int countSidePawns = bitCount15(c);
					const int countOtherPawns = bitCount15(passedMask[color][from] & otherPawns);
					if (countSidePawns>=countOtherPawns) {
						eval+=candidatePasserBonus[color][squareRank[from]];
					}
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

	if (isChained) {
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

		if (std::min(5,delta1)<delta2-otherMove) {
			eval += UNSTOPPABLE_PAWN_BONUS;
		}
	}

	return eval;
}

// Bishops eval function
void Evaluator::evalPieces(PieceColor color, EvalInfo& evalInfo) {

	evalInfo.value[color] +=
			evalInfo.board.isCastleDone(color)?DONE_CASTLE_BONUS:
	evalInfo.board.getCastleRights(color)==NO_CASTLE?-DONE_CASTLE_BONUS:0;

	const Bitboard bishop = evalInfo.board.getPieces(color,BISHOP);

	if ((bishop & WHITE_SQUARES) && (bishop & BLACK_SQUARES)) {
		evalInfo.value[color] += BISHOP_PAIR_BONUS;
	}

}

// mobility eval function
void Evaluator::evalBoardControl(PieceColor color, EvalInfo& evalInfo) {

	const PieceColor other = evalInfo.board.flipSide(color);
	const Square otherKingSq = evalInfo.board.getKingSquare(other);
	const Bitboard otherKingBB = evalInfo.board.getPieces(other,KING);
	const Bitboard freeArea = ~(evalInfo.board.getPieces(color) |
			evalInfo.attackers[makePiece(other,PAWN)]);

	Bitboard pieces = EMPTY_BB;
	evalInfo.attackers[makePiece(color,KNIGHT)] = EMPTY_BB;
	evalInfo.attackers[makePiece(color,BISHOP)] = EMPTY_BB;
	evalInfo.attackers[makePiece(color,ROOK)] = EMPTY_BB;
	evalInfo.attackers[makePiece(color,QUEEN)] = EMPTY_BB;
	Square from = NONE;
	evalInfo.kingThreat[color]=0;

	pieces = evalInfo.board.getPieces(color,KNIGHT);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = evalInfo.board.getKnightAttacks(from);
		evalInfo.value[color] += knightMobility[bitCount(attacks&freeArea)];
		evalInfo.kingThreat[color] += knightKingBonus[squareDistance(from,otherKingSq)];
		evalInfo.attackers[makePiece(color,KNIGHT)] |= attacks;
		if (knightOutpostBonus[color][from] && evalInfo.attackers[makePiece(color,PAWN)]&squareToBitboard[from] &&
				!(evalInfo.attackers[makePiece(other,PAWN)]&squareToBitboard[from])) {
			evalInfo.value[color] += knightOutpostBonus[color][from];
		}
		from = extractLSB(pieces);
	}

	pieces = evalInfo.board.getPieces(color,BISHOP);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = evalInfo.board.getBishopAttacks(from);
		evalInfo.attackers[makePiece(color,BISHOP)] |= attacks;
		evalInfo.value[color] += bishopMobility[bitCount(attacks&freeArea)];
		evalInfo.kingThreat[color] += bishopKingBonus[squareDistance(from,otherKingSq)];
		from = extractLSB(pieces);
	}

	pieces = evalInfo.board.getPieces(color,ROOK);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = evalInfo.board.getRookAttacks(from);
		evalInfo.attackers[makePiece(color,ROOK)] |= attacks;
		evalInfo.value[color] += rookMobility[bitCount(attacks&freeArea)];
		evalInfo.kingThreat[color] += rookKingBonus[squareDistance(from,otherKingSq)];
		if ((squareToBitboard[from] & promoRank[color]) &&
				(otherKingBB & eighthRank[color])) {
			evalInfo.value[color] += ROOK_ON_7TH_RANK_BONUS;
		}
		from = extractLSB(pieces);
	}

	pieces = evalInfo.board.getPieces(color,QUEEN);
	from = extractLSB(pieces);

	while ( from!=NONE ) {
		const Bitboard attacks = evalInfo.board.getQueenAttacks(from);
		const int queenMobility = bitCount(attacks&freeArea)-10;
		evalInfo.attackers[makePiece(color,QUEEN)] |= attacks;
		evalInfo.value[color] += MS(queenMobility,queenMobility);
		evalInfo.kingThreat[color] += queenKingBonus[squareDistance(from,otherKingSq)];
		if ((squareToBitboard[from] & promoRank[color]) &&
				(otherKingBB & eighthRank[color])) {
			evalInfo.value[color] += QUEEN_ON_7TH_RANK_BONUS;
		}
		from = extractLSB(pieces);
	}

	// evaluate space
	const Bitboard sidePieces = evalInfo.board.getPieces(color);
	const Bitboard kingAndPawns = evalInfo.board.getPieces(color,KING) | evalInfo.pawns[color];
	const Bitboard otherSideControl = (sidePieces ^ kingAndPawns) & colorSpaceBB[other] & freeArea;
	evalInfo.value[color] += spaceBonus[bitCount(otherSideControl)];

	evalInfo.attacks[color] = evalInfo.attackers[makePiece(color,KNIGHT)] |
			evalInfo.attackers[makePiece(color,BISHOP)] | evalInfo.attackers[makePiece(color,ROOK)] |
			evalInfo.attackers[makePiece(color,QUEEN)] | evalInfo.attackers[makePiece(color,PAWN)];

}

// eval threats - idea from Stockfish
void Evaluator::evalThreats(PieceColor color, EvalInfo& evalInfo) {

	const PieceColor other = evalInfo.board.flipSide(color);
	const PieceTypeByColor pawnOther = makePiece(other,PAWN);

	Bitboard pieces=evalInfo.board.getPieces(other) &
			~evalInfo.attackers[pawnOther] & evalInfo.attacks[color];

	if (pieces) {
		for (int sideType=KNIGHT;sideType<KING;sideType++) {
			const PieceTypeByColor sidePiece = makePiece(color,PieceType(sideType));
			Bitboard attacked = evalInfo.attackers[sidePiece]&pieces;
			if (attacked) {
				for (int otherType=PAWN;otherType<=KING;otherType++) {
					const PieceTypeByColor otherPiece = makePiece(other,PieceType(otherType));
					if (attacked & evalInfo.board.getPieces(otherPiece)) {
						evalInfo.value[color] += threatBonus[sideType][otherType];
					}
				}
			}
		}
	}

}
