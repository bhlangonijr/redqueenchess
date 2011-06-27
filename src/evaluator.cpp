/*
	Redqueen Chess Engine
    Copyright (C) 2008-2011 Ben-Hur Carlos Vieira Langoni Junior

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

Evaluator::Evaluator() : debug(false), lazyEval(false) {
	cleanPawnInfo();
}

Evaluator::~Evaluator() {

}
// main eval function
const int Evaluator::evaluate(Board& board, const int alpha, const int beta) {
	EvalInfo evalInfo(board);
	setLazyEval(true);
	evalPieces(evalInfo.side, evalInfo);
	evalPieces(evalInfo.other, evalInfo);
	evalImbalances(evalInfo.side, evalInfo);
	evalImbalances(evalInfo.other, evalInfo);
	evalInfo.computeEval();
	bool doNotLazyEval = evalInfo.getEval() > alpha-lazyEvalMargin/2 && evalInfo.getEval() < beta+lazyEvalMargin/2;
	bool doPawnEval = doNotLazyEval || board.isPawnPromoting() || board.isPawnFinal();
	if (doPawnEval) {
		setLazyEval(false);
		evalInfo.attackers[WHITE_PAWN] = ((evalInfo.pawns[WHITE] & midBoardNoFileA) << 7) |
				((evalInfo.pawns[WHITE] & midBoardNoFileH) << 9);
		evalInfo.attackers[BLACK_PAWN] = ((evalInfo.pawns[BLACK] & midBoardNoFileA) >> 9) |
				((evalInfo.pawns[BLACK] & midBoardNoFileH) >> 7);
		PawnInfo info;
		if (getPawnInfo(board.getPawnKey(),info)) {
			evalPawnsFromCache(evalInfo.side, info, evalInfo);
			evalPawnsFromCache(evalInfo.other, info, evalInfo);
		} else {
			evalPawns(evalInfo.side, evalInfo);
			evalPawns(evalInfo.other, evalInfo);
		}
		evalInfo.computeEval();
	}
	doNotLazyEval = evalInfo.getEval() > alpha-lazyEvalMargin && evalInfo.getEval() < beta+lazyEvalMargin;
	if (doNotLazyEval) {
		setLazyEval(false);
		evalBoardControl(evalInfo.side, evalInfo);
		evalBoardControl(evalInfo.other, evalInfo);
		evalThreats(evalInfo.side, evalInfo);
		evalThreats(evalInfo.other, evalInfo);
		evalKing(evalInfo.side, evalInfo);
		evalKing(evalInfo.other, evalInfo);
		evalInfo.computeEval();
	}
	if (isDebugEnabled()) {
		std::cout << evalInfo.toString();
	}
	return evalInfo.getEval();
}
// king eval function
void Evaluator::evalKing(PieceColor color, EvalInfo& evalInfo) {
	const PieceColor other = evalInfo.board.flipSide(color);
	const Square kingSq = evalInfo.board.getKingSquare(other);
	const Bitboard kingSquareBB = adjacentSquares[kingSq];
	int pressure = 0;
	for (int piece=makePiece(color,KNIGHT);piece<=makePiece(color,QUEEN);piece++) {
		// king area safety
		const Bitboard attacks = evalInfo.attackers[piece]&kingSquareBB;
		if (attacks) {
			const int attackCount = pieceType[piece]==KNIGHT?1:bitCount15(attacks);
			pressure += getKingAttackWeight(piece,attackCount);
		}
	}
	evalInfo.kingThreat[color] += pressure;
}

void Evaluator::evalPawnsFromCache(PieceColor color, PawnInfo& info, EvalInfo& evalInfo) {
	const PieceColor other = evalInfo.board.flipSide(color);
	const Bitboard all = evalInfo.all;
	const Bitboard pawns = evalInfo.pawns[color];
	const Bitboard otherPawns = evalInfo.pawns[other];
	evalInfo.evalPawns[color]+=info.value[color];
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
			evalInfo.evalPawns[color] += evalPassedPawn(evalInfo,color,from,isPawnFinal,isChained);
			from = extractLSB(passed);
		}
	}
}
// pawns eval function
void Evaluator::evalPawns(PieceColor color, EvalInfo& evalInfo) {
	Board& board = evalInfo.board;
	const PieceColor other = board.flipSide(color);
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
				board.getPieces(WHITE_KING) |
				board.getPieces(BLACK_KING);
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
				passedBonus += evalPassedPawn(evalInfo,color,from,isPawnFinal,isChained);
				passers|=squareToBitboard[from];
			}
			if (!(isPasser | isIsolated | isChained) &&
					!(backwardPawnMask[color][from]&allButThePawn) &&
					!(board.getPawnAttacks(from,color)&otherPawns)) {
				Bitboard attacks = board.getPawnAttacks(from,color);
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
	setPawnInfo(board.getPawnKey(),eval,color,passers);
	evalInfo.evalPawns[color] += eval+passedBonus;
}
// Eval passed pawns
const int Evaluator::evalPassedPawn(EvalInfo& evalInfo, PieceColor color, const Square from,
		const bool isPawnFinal, const bool isChained) {
	int eval=0;
	Board& board = evalInfo.board;
	const PieceColor other = board.flipSide(color);
	const Square otherKingSq = board.getKingSquare(other);
	bool isUnstoppable = false;
	if (isChained) {
		eval += connectedPasserBonus[color][squareRank[from]];
	} else {
		eval += passedPawnBonus[color][squareRank[from]];
	}
	eval += pawnWeight[color][squareFile[from]];
	if (isPawnFinal) {
		const Rank rank = color==WHITE?RANK_8:RANK_1;
		const Square target = board.makeSquare(rank,squareFile[from]);
		const int delta1 = squareDistance(from,target);
		const int delta2 = squareDistance(otherKingSq,target);
		const int otherMove=(board.getSideToMove()==other?1:0);
		if (std::min(5,delta1)<delta2-otherMove) {
			eval += UNSTOPPABLE_PAWN_BONUS;
			isUnstoppable = true;
		}
	}
	if (!isUnstoppable) {
		const Rank r = color==WHITE?squareRank[from+8]:squareRank[from-8];
		const Square next = board.makeSquare(r,squareFile[from]);
		const Square sideKingSq = board.getKingSquare(color);
		eval += squareDistance(next,otherKingSq)*PASSER_AND_KING_BONUS;
		eval -= squareDistance(next,sideKingSq)*PASSER_AND_KING_BONUS;
		if (board.getPiece(next)==EMPTY) {
			const Bitboard block = evalInfo.attacks[other]|
					board.getPieces(other);
			if (!(frontSquares[color][from]&block)) {
				eval += freePasserBonus[color][squareRank[from]];
			}
			const PieceTypeByColor sideQueen = makePiece(color,QUEEN);
			const PieceTypeByColor sideRook = makePiece(color,ROOK);
			const Bitboard fromFileMask = fileBB[squareFile[from]];
			if (board.getPieces(sideQueen) & fromFileMask) {
				if (evalInfo.attackers[sideQueen] & squareToBitboard[from]) {
					eval += MS(+0,+3);
				}
			}
			if (board.getPieces(sideRook) & fromFileMask) {
				if (evalInfo.attackers[sideRook] & squareToBitboard[from]) {
					eval += MS(+0,+4);
				}
			}
		}
	}
	return eval;
}
// Pieces eval function
void Evaluator::evalPieces(PieceColor color, EvalInfo& evalInfo) {
	Board& board = evalInfo.board;
	evalInfo.evalPieces[color] +=
			board.isCastleDone(color)?DONE_CASTLE_BONUS:
	board.getCastleRights(color)==NO_CASTLE?-DONE_CASTLE_BONUS:0;
	const Bitboard bishop = board.getPieces(color,BISHOP);
	const Bitboard lightBishop = bishop & WHITE_SQUARES;
	const Bitboard darkBishop = bishop & BLACK_SQUARES;
	if (lightBishop && darkBishop) {
		evalInfo.imbalance[color] += lowerScore(BISHOP_PAIR_BONUS);
	}
}
// mobility eval function
void Evaluator::evalBoardControl(PieceColor color, EvalInfo& evalInfo) {
	Board& board = evalInfo.board;
	const PieceColor other = board.flipSide(color);
	const Square otherKingSq = board.getKingSquare(other);
	const Bitboard otherKingBB = board.getPieces(other,KING);
	const Bitboard freeArea = ~(board.getPieces(color) |
			evalInfo.attackers[makePiece(other,PAWN)]);
	Bitboard pieces = EMPTY_BB;
	for (int piece=makePiece(color,KNIGHT);piece<=makePiece(color,QUEEN);piece++) {
		evalInfo.attackers[piece] = EMPTY_BB;
	}
	pieces = evalInfo.board.getAllPieces();
	pieces = (color==WHITE?pieces>>8:pieces<<8) & evalInfo.board.getPieces(color,PAWN);
	if (pieces) {
		evalInfo.mobility[color] += PAWN_MOBILITY_PENALTY * bitCount(pieces);
	}
	Square from = NONE;
	evalInfo.kingThreat[color]=0;
	pieces = evalInfo.board.getPieces(color,KNIGHT);
	from = extractLSB(pieces);
	while ( from!=NONE ) {
		const Bitboard attacks = evalInfo.board.getKnightAttacks(from);
		evalInfo.mobility[color] += knightMobility[bitCount(attacks&freeArea)];
		evalInfo.attackers[makePiece(color,KNIGHT)] |= attacks;
		if (knightOutpostBonus[color][from] && evalInfo.attackers[makePiece(color,PAWN)]&squareToBitboard[from] &&
				!(evalInfo.attackers[makePiece(other,PAWN)]&squareToBitboard[from])) {
			evalInfo.evalPieces[color] += knightOutpostBonus[color][from];
		}
		if(!(evalInfo.attackers[makePiece(other,PAWN)]&squareToBitboard[from])) {
			evalInfo.kingThreat[color] += knightKingBonus[squareDistance(from,otherKingSq)];
		}
		from = extractLSB(pieces);
	}
	pieces = board.getPieces(color,BISHOP);
	from = extractLSB(pieces);
	while ( from!=NONE ) {
		const Bitboard attacks = board.getBishopAttacks(from);
		evalInfo.attackers[makePiece(color,BISHOP)] |= attacks;
		evalInfo.mobility[color] += bishopMobility[bitCount(attacks&freeArea)];
		if(!(evalInfo.attackers[makePiece(other,PAWN)]&squareToBitboard[from])) {
			evalInfo.kingThreat[color] += bishopKingBonus[squareDistance(from,otherKingSq)];
		}
		from = extractLSB(pieces);
	}
	pieces = board.getPieces(color,ROOK);
	from = extractLSB(pieces);
	while ( from!=NONE ) {
		const Bitboard attacks = board.getRookAttacks(from);
		evalInfo.attackers[makePiece(color,ROOK)] |= attacks;
		evalInfo.mobility[color] += rookMobility[bitCount(attacks&freeArea)];
		if ((squareToBitboard[from] & promoRank[color]) &&
				(otherKingBB & eighthRank[color])) {
			evalInfo.evalPieces[color] += ROOK_ON_7TH_RANK_BONUS;
		}
		if (!(evalInfo.pawns[color]&fileBB[squareFile[from]])) {
			if (!(evalInfo.pawns[other]&fileBB[squareFile[from]])) {
				evalInfo.evalPieces[color] += ROOK_ON_OPEN_FILE_BONUS;
			} else {
				evalInfo.evalPieces[color] += ROOK_ON_HALF_OPEN_FILE_BONUS;
			}
		}
		if(!(evalInfo.attackers[makePiece(other,PAWN)]&squareToBitboard[from])) {
			evalInfo.kingThreat[color] += rookKingBonus[squareDistance(from,otherKingSq)];
		}
		from = extractLSB(pieces);
	}
	pieces = board.getPieces(color,QUEEN);
	from = extractLSB(pieces);
	while ( from!=NONE ) {
		const Bitboard attacks = board.getQueenAttacks(from);
		const int queenMobility = bitCount(attacks&freeArea)-10;
		evalInfo.attackers[makePiece(color,QUEEN)] |= attacks;
		evalInfo.mobility[color] += MS(queenMobility,queenMobility);
		if ((squareToBitboard[from] & promoRank[color]) &&
				(otherKingBB & eighthRank[color])) {
			evalInfo.evalPieces[color] += QUEEN_ON_7TH_RANK_BONUS;
		}
		if(!(evalInfo.attackers[makePiece(other,PAWN)]&squareToBitboard[from])) {
			evalInfo.kingThreat[color] += queenKingBonus[squareDistance(from,otherKingSq)];
		}
		from = extractLSB(pieces);
	}
	// evaluate space
	const Bitboard sidePieces = board.getPieces(color);
	const Bitboard kingAndPawns = board.getPieces(color,KING) | evalInfo.pawns[color];
	const Bitboard otherSideControl = (sidePieces ^ kingAndPawns) & colorSpaceBB[other] & freeArea;
	evalInfo.mobility[color] += spaceBonus[bitCount(otherSideControl)];
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
		for (int sideType=PAWN;sideType<KING;sideType++) {
			const PieceTypeByColor sidePiece = makePiece(color,PieceType(sideType));
			Bitboard attacked = evalInfo.attackers[sidePiece]&pieces;
			if (attacked) {
				for (int otherType=PAWN;otherType<=KING;otherType++) {
					const PieceTypeByColor otherPiece = makePiece(other,PieceType(otherType));
					if (attacked & evalInfo.board.getPieces(otherPiece)) {
						evalInfo.pieceThreat[color] += threatBonus[sideType][otherType];
					}
				}
			}
		}
	}
}
// End game eval
void Evaluator::evalImbalances(PieceColor color, EvalInfo& evalInfo) {
	Board& board = evalInfo.board;
	const PieceColor other = board.flipSide(color);
	const int pawnCount = board.getPieceCount(makePiece(color,PAWN));
	const int otherPawnCount = board.getPieceCount(makePiece(other,PAWN));
	if (pawnCount==0) {
		const int balance = board.getMaterial(color)-
				board.getMaterial(other);
		if (balance>0) {
			const bool otherHasNonPawnMaterial = board.getPieces(other)^board.getPieces(other,PAWN);
			if (board.getMaterial(color) <= kingValue+drawishMaxValue) {
				if (board.getMaterial(other) <= kingValue ||
						otherHasNonPawnMaterial) {
					evalInfo.imbalance[color] += -balance; //draw
				} else {
					const int otherPawnCount=board.getPieceCount(makePiece(other,PAWN));
					if (otherPawnCount<2) {
						evalInfo.imbalance[color] += -(balance*95)/100;
					} else {
						evalInfo.imbalance[color] += -balance + -((pawnValue/2)*otherPawnCount);
					}
				}
			} else {
				if (balance <= drawishMaxValue) {
					evalInfo.imbalance[color] += -balance/3;
				} else if (balance <= rookValue) {
					evalInfo.imbalance[color] += -balance/12;
				}
			}
		}
	}
	const int pawnDiff = pawnCount-otherPawnCount;
	if (pawnDiff>0) {
		evalInfo.imbalance[color] += interpolate(PAWN_END_GAME_BONUS,board.getGamePhase())*pawnDiff;
	}

}
