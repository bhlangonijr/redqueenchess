/*
	Redqueen Chess Engine
    Copyright (C) 2008-2012 Ben-Hur Carlos Vieira Langoni Junior

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

Evaluator::Evaluator() : debug(false), lazyEval(false),
positionalWeight(100), tacticalWeight(100) {
	cleanPawnInfo();
}

Evaluator::~Evaluator() {

}
// main eval function
const int Evaluator::evaluate(Board& board, const int alpha, const int beta) {
	EvalInfo evalInfo(board);
	setLazyEval(true);
	evalInfo.setWeigthedEval(getTacticalWeight(),getPositionalWeight());
	evalImbalances(evalInfo);
	evalInfo.computeEval();
	if (evalInfo.drawFlag) {
		if (isDebugEnabled()) {
			std::cout << evalInfo.toString();
		}
		return evalInfo.getEval();
	}
	int lazyEval=lazyEvalMargin;
	if (board.isPawnPromoting()) {
		lazyEval+=200;
	}
	if (board.isPawnFinal()) {
		lazyEval+=100;
	}
	bool doNotLazyEval = evalInfo.getEval() > alpha-lazyEval && evalInfo.getEval() < beta+lazyEval;
	if (doNotLazyEval) {
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
		setUnstoppableBonus(evalInfo.side, evalInfo);
		setUnstoppableBonus(evalInfo.other, evalInfo);
		evalInfo.computeEval();
	}
	doNotLazyEval = evalInfo.getEval() > alpha-lazyEval && evalInfo.getEval() < beta+lazyEval;
	if (doNotLazyEval) {
		setLazyEval(false);
		evalBoardControl(evalInfo.side, evalInfo);
		evalBoardControl(evalInfo.other, evalInfo);
		evalThreats(evalInfo.side, evalInfo);
		evalThreats(evalInfo.other, evalInfo);
		evalKingPressure(evalInfo.side, evalInfo);
		evalKingPressure(evalInfo.other, evalInfo);
		evalInfo.computeEval();
	}
	if (isDebugEnabled()) {
		std::cout << evalInfo.toString();
	}
	return evalInfo.getEval();
}
// king eval function
void Evaluator::evalKingPressure(PieceColor color, EvalInfo& evalInfo) {
	Board& board = evalInfo.board;
	const PieceColor other = board.flipSide(color);
	const Square kingSq = board.getKingSquare(other);
	const Bitboard kingAreaBB = adjacentSquares[kingSq];
	int pressure = 0;
	//score attack weight
	for (int piece=makePiece(color,KNIGHT);piece<=makePiece(color,QUEEN);piece++) {
		const Bitboard attacks = evalInfo.attackers[piece]&kingAreaBB;
		if (attacks) {
			const int attackCount = pieceType[piece]==KNIGHT?1:bitCount15(attacks);
			pressure += getKingAttackWeight(piece,attackCount);
		}
	}
	// score contact checks
	const Bitboard notDefended = evalInfo.attacks[color]&
			kingAreaBB&~evalInfo.attacks[other];

	const Bitboard queenContactCheck = ~board.getPieces(color)&
			notDefended&evalInfo.attackers[makePiece(color,QUEEN)];

	if (queenContactCheck) {
		const Bitboard safeCheck = queenContactCheck &
				(evalInfo.attackers[makePiece(color,PAWN)] |
						evalInfo.attackers[makePiece(color,KNIGHT)] |
						evalInfo.attackers[makePiece(color,BISHOP)] |
						evalInfo.attackers[makePiece(color,ROOK)]);
		if (safeCheck) {
			pressure+=(color==board.getSideToMove()?2:1) *
					bitCount15(safeCheck)*QUEEN_CHECK_BONUS;
		}
	}
	const Bitboard rookContactCheck = ~board.getPieces(color)&
			notDefended&evalInfo.attackers[makePiece(color,ROOK)];

	if (rookContactCheck) {
		const Bitboard safeCheck = rookContactCheck &
				(evalInfo.attackers[makePiece(other,PAWN)] |
						evalInfo.attackers[makePiece(color,KNIGHT)] |
						evalInfo.attackers[makePiece(color,BISHOP)] |
						evalInfo.attackers[makePiece(color,QUEEN)]);
		if (safeCheck) {
			pressure+= (color==board.getSideToMove()?2:1)*
					bitCount15(safeCheck)*ROOK_CHECK_BONUS;
		}
	}

	const Bitboard clearWay = ~(board.getPieces(color)|evalInfo.attacks[other]);
	const Bitboard rookAttacks = clearWay&board.getRookAttacks(kingSq);
	const Bitboard bishopAttacks = clearWay&board.getBishopAttacks(kingSq);

	const Bitboard queenIndirectAttacks = (rookAttacks|bishopAttacks)&
			evalInfo.attackers[makePiece(color,QUEEN)];
	if (queenIndirectAttacks) {
		pressure += bitCount15(queenIndirectAttacks)*
				INDIRECT_QUEEN_CHECK_BONUS;
	}

	const Bitboard rookIndirectAttacks = rookAttacks &
			evalInfo.attackers[makePiece(color,ROOK)];
	if (rookIndirectAttacks) {
		pressure += bitCount15(rookIndirectAttacks)*
				INDIRECT_ROOK_CHECK_BONUS;
	}

	const Bitboard bishopIndirectAttacks = bishopAttacks &
			evalInfo.attackers[makePiece(color,BISHOP)];
	if (bishopIndirectAttacks) {
		pressure += bitCount15(bishopIndirectAttacks)*
				INDIRECT_BISHOP_CHECK_BONUS;
	}

	const Bitboard kinghtIndirectAttacks = board.getKnightAttacks(kingSq)&
			evalInfo.attackers[makePiece(color,KNIGHT)]&clearWay;
	if (kinghtIndirectAttacks) {
		pressure += bitCount15(kinghtIndirectAttacks)*
				INDIRECT_KNIGHT_CHECK_BONUS;
	}
	//quality of opponent's shelter
	if ((board.getPieces(color,QUEEN) || board.getPieces(color,ROOK)) &&
			(board.getPieces(other,KING)&shelterArea[other])) {
		const Rank kingRank = squareRank[kingSq];
		const int sign = other==WHITE?+1:-1;
		int shelterScore=0;
		const File fileFixed = squareFile[kingSq]==FILE_H?FILE_G:
				squareFile[kingSq]==FILE_A?FILE_B:squareFile[kingSq];
		const Square sqFixed = board.makeSquare(kingRank,fileFixed);
		const Bitboard shieldPawns = passedMask[other][sqFixed]&
				evalInfo.pawns[other];
		const Bitboard layer1 = shieldPawns&
				rankBB[Rank(kingRank+sign*1)];
		const Bitboard layer2 = shieldPawns&
				rankBB[Rank(kingRank+sign*2)];
		const Bitboard shield = layer1 |
				(color==WHITE?layer2>>8:layer2<<8);
		const Bitboard openfiles = passedMask[other][sqFixed]&
				rankBB[RANK_5]&evalInfo.openfiles[color];
		const int countOpenFiles = bitCount15(openfiles);
		const int countLayer1 = bitCount15(layer1);
		const int countShield = bitCount15(shield);

		shelterScore = (countShield * std::max(countLayer1,1)) *
				SHELTER_BONUS;
		shelterScore += countOpenFiles * SHELTER_OPEN_FILE_PENALTY;
		pressure -= shelterScore;
	}

	evalInfo.kingThreat[color] += pressure;
	evalInfo.evalPieces[color] +=
			evalInfo.board.isCastleDone(color)?DONE_CASTLE_BONUS:
					evalInfo.board.getCastleRights(color)==NO_CASTLE?-DONE_CASTLE_BONUS:0;
}

void Evaluator::evalPawnsFromCache(PieceColor color, PawnInfo& info, EvalInfo& evalInfo) {
	const Bitboard pawns = evalInfo.pawns[color];
	const PieceColor other =  evalInfo.board.flipSide(color);
	evalInfo.evalPawns[color]+=info.value[color];
	evalInfo.openfiles[color]=info.openfiles[color];
	Bitboard passed=info.passers[color] & pawns;
	if (passed) {
		const Bitboard pawnsAndKing = evalInfo.board.getPieces(other,PAWN) |
				evalInfo.board.getPieces(other,KING);
		const bool otherHasOnlyPawns = !(pawnsAndKing^evalInfo.board.getPieces(other));
		Square from = extractLSB(passed);
		while ( from!=NONE ) {
			const Bitboard pawn = squareToBitboard[from];
			const Bitboard allButThePawn =(pawns^pawn);
			const Bitboard chainSquares = backwardPawnMask[color][from]&adjacentSquares[from];
			const bool isChained = (chainSquares&allButThePawn);
			evalInfo.evalPawns[color] += evalPassedPawn(evalInfo,color,from,
					isChained,otherHasOnlyPawns);
			from = extractLSB(passed);
		}
	}
}
// pawns eval function
void Evaluator::evalPawns(PieceColor color, EvalInfo& evalInfo) {
	Board& board = evalInfo.board;
	const PieceColor other = board.flipSide(color);
	const Bitboard pawns = evalInfo.pawns[color];
	const Bitboard otherPawns = evalInfo.pawns[other];
	Bitboard passers=EMPTY_BB;
	Bitboard closedfiles=EMPTY_BB;
	int passedBonus=0;
	int eval=0;
	//penalyze doubled, isolated and backward pawns
	//bonus to passer and candidates
	if (pawns) {
		const Bitboard pawnsAndKing = board.getPieces(other,PAWN) |
				board.getPieces(other,KING);
		const bool otherHasOnlyPawns = !(pawnsAndKing^board.getPieces(other));
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
			bool isBackward = false;
			closedfiles|=fileBB[squareFile[from]];
			if (isDoubled) {
				eval += DOUBLED_PAWN_PENALTY;
			}
			if (isIsolated) {
				eval += halfOpenFile?ISOLATED_OPEN_PAWN_PENALTY:ISOLATED_PAWN_PENALTY;
			} else if (adjacentSquares[from]&allButThePawn) {
				eval += CONNECTED_PAWN_BONUS;
			}
			if (isPasser && !(isDoubled && (frontSquares[color][from]&allButThePawn))) {
				passedBonus += evalPassedPawn(evalInfo,color,from,isChained,otherHasOnlyPawns);
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
					isBackward = true;
				}
			}
			if (!isPasser && !isBackward && !(frontSquares[color][from]&otherPawns)) {
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
	setPawnInfo(board.getPawnKey(),eval,color,passers,~closedfiles);
	evalInfo.openfiles[color]=~closedfiles;
	evalInfo.evalPawns[color] += eval+passedBonus;
}
// Eval passed pawns
const int Evaluator::evalPassedPawn(EvalInfo& evalInfo, PieceColor color,
		const Square from, const bool isChained, const bool otherHasOnlyPawns) {
	int eval=0;
	Board& board = evalInfo.board;
	const PieceColor other = board.flipSide(color);
	const Square otherKingSq = board.getKingSquare(other);
	if (isChained) {
		eval += connectedPasserBonus[color][squareRank[from]];
	} else {
		eval += passedPawnBonus[color][squareRank[from]];
	}
	eval += pawnWeight[color][squareFile[from]];
	const int dist = verifyUnstoppablePawn(board, color, from, otherHasOnlyPawns);
	if (dist<evalInfo.bestUnstoppable[color]) {
		evalInfo.bestUnstoppable[color]=dist;
		eval += UNSTOPPABLE_CANDIDATE_BONUS;
	}
	const Rank r = color==WHITE?squareRank[from+8]:squareRank[from-8];
	const Square next = board.makeSquare(r,squareFile[from]);
	const Square sideKingSq = board.getKingSquare(color);
	eval += squareDistance(next,otherKingSq)*PASSER_AND_KING_BONUS;
	eval -= squareDistance(next,sideKingSq)*PASSER_AND_KING_BONUS;
	if (board.getPiece(next)==EMPTY) {
		const bool advanced = color==WHITE?squareRank[from]>=RANK_6:
				squareRank[from]<=RANK_3;
		if (advanced && !(passedMask[color][from]&board.getPieces(other))) {
			if (!board.isAttacked(color,next)) {
				eval += freePasserBonus[color][squareRank[from]];
			}
		}
		const Bitboard fromFileMask = fileBB[squareFile[from]];
		if (board.getPieces(color,QUEEN) & fromFileMask) {
			const Bitboard queenAttacks = board.getQueenAttacks(from) &
					board.getPieces(color,QUEEN);
			if (queenAttacks & squareToBitboard[from]) {
				eval += MS(+0,+3);
			}
		}
		if (board.getPieces(color,ROOK) & fromFileMask) {
			const Bitboard rookAttacks = board.getRookAttacks(from) &
					board.getPieces(color,ROOK);
			if (rookAttacks & squareToBitboard[from]) {
				eval += MS(+0,+4);
			}
		}
	}
	return eval;
}

// mobility & control eval function
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
		evalInfo.mobility[color] += bitCount15(pieces)*PAWN_MOBILITY_PENALTY;
	}
	Square from = NONE;
	pieces = board.getPieces(color,KNIGHT);
	from = extractLSB(pieces);
	while ( from!=NONE ) {
		const Bitboard attacks = board.getKnightAttacks(from);
		evalInfo.mobility[color] += knightMobility[bitCount(attacks&freeArea)];
		evalInfo.attackers[makePiece(color,KNIGHT)] |= attacks;
		if (knightOutpostBonus[color][from] &&
				(evalInfo.attackers[makePiece(color,PAWN)]&squareToBitboard[from]) &&
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
void Evaluator::evalImbalances(EvalInfo& evalInfo) {
	Board& board = evalInfo.board;
	const int balance = board.getMaterial(evalInfo.side)-
			board.getMaterial(evalInfo.other);
	const PieceColor color = balance>=0?evalInfo.side:evalInfo.other;
	const PieceColor other = board.flipSide(color);
	const int sidePawnCount = board.getPieceCount(color,PAWN);
	const int otherPawnCount = board.getPieceCount(other,PAWN);
	const int pawnCount = sidePawnCount + otherPawnCount;
	const Bitboard otherKing = board.getPieces(other,KING);
	const int pawnDiff = sidePawnCount-otherPawnCount;
	const int totalPieceCount = board.getPieceCount(color)+
			board.getPieceCount(other);
	const int sideMinors = board.getPieceCount(color,BISHOP)+
			board.getPieceCount(color,KNIGHT);
	const int otherMinors = board.getPieceCount(other,BISHOP)+
			board.getPieceCount(other,KNIGHT);
	const int sideMajors = board.getPieceCount(color,ROOK)+
			board.getPieceCount(color,QUEEN);
	const int otherMajors = board.getPieceCount(other,ROOK)+
			board.getPieceCount(other,QUEEN);
	const Bitboard sideBishop = board.getPieces(color,BISHOP);
	const Bitboard otherBishop = board.getPieces(other,BISHOP);
	const bool hasLightBishop = sideBishop&WHITE_SQUARES;
	const bool hasDarkBishop = sideBishop&BLACK_SQUARES;
	const bool otherHasLightBishop = otherBishop&WHITE_SQUARES;
	const bool otherHasDarkBishop = otherBishop&BLACK_SQUARES;
	const int minors = sideMinors + otherMinors;
	const int majors = sideMajors + otherMajors;

	if (totalPieceCount < 6 && balance > 0) {
		int drawishness=0;
		if (majors==0 && pawnCount==0) {
			if (minors <= 1 || (minors == 2 && sideMinors==otherMinors)) {
				drawishness=100;
			} else if (minors <= 3 && otherMinors>0) {
				drawishness=95;
			} else if (minors <= 2 && sideMinors==2 &&
					otherMinors==0 && !board.getPieces(color,BISHOP)) {
				drawishness=100; //draw - KNN vs K
			}
		} else if (majors<=2 && minors <= 3 && pawnCount==0) {
			if (sideMajors==1 && otherMajors==1 &&
					sideMinors==0 && otherMinors <= 1) {
				drawishness=90;
			} else if (sideMajors==1 && otherMajors==1 &&
					sideMinors==1 && otherMinors == 0) {
				if (balance<=drawishMaxValue) {
					drawishness=95;
				}
			} else if (sideMajors==1 && otherMajors==0 &&
					sideMinors==0 && otherMinors >= 2) {
				if (board.getPieces(color,QUEEN)) {
					drawishness=50;
				} else {
					drawishness=90;
				}
			} else if (sideMajors==0 && otherMajors==1 &&
					sideMinors>=2 && otherMinors==0 ) {
				if (sideMinors==3) {
					drawishness=40;
				} else {
					drawishness=90;
				}
			}
		} else if (majors==0 && minors<=2 && pawnCount<=5) {
			const Bitboard sidePawns = board.getPieces(color,PAWN);
			const Bitboard otherPawns = board.getPieces(other,PAWN);
			const bool pawnOnAFile = sidePawns & fileBB[FILE_A];
			const bool pawnOnHFile = sidePawns & fileBB[FILE_H];
			if (minors==1 && sidePawnCount==1 && sideMinors==1 &&
					otherPawnCount<=1 && !board.getPieces(color,KNIGHT)) {
				if (pawnOnAFile || pawnOnHFile) {
					const bool wrongBishop = color==WHITE?
							(pawnOnHFile && hasLightBishop) ||
							(pawnOnAFile && hasDarkBishop):
							(pawnOnHFile && hasDarkBishop) ||
							(pawnOnAFile && hasLightBishop);
					if ((otherPawnCount==0 && pawnOnAFile && wrongBishop &&
							(otherKing & kingFileACorner[other])) ||
							(pawnOnHFile && wrongBishop &&
									(otherKing & kingFileHCorner[other]))) {
						drawishness=100;
					} else if ((sidePawns & blockedRank[color]) &
							(color==WHITE?otherPawns>>8:otherPawns<<8)) {
						if ((pawnOnAFile && (otherKing & blockRankOnAFile[other])) ||
								(pawnOnHFile && (otherKing & blockRankOnHFile[other]))) {
							drawishness=100;
						}
					}
				}
			} else if (minors == 2 && sideMinors==1 && otherMinors==1 &&
					sidePawnCount==1 && otherPawnCount==0) {
				if ((hasLightBishop && otherHasLightBishop) ||
						(hasDarkBishop && otherHasDarkBishop) ) {
					drawishness=40;
				} else {
					drawishness=80;
				}
			} else if (minors == 1 && sideMinors==1 &&
					sidePawnCount==0 && otherPawnCount==1) {
				drawishness=90;
			} else if (minors == 1 && sideMinors==1 &&
					sidePawnCount==0 && otherPawnCount>=2) {
				drawishness=100+30*otherPawnCount;
			} else if (minors==0 && sidePawnCount==1 &&	otherPawnCount==0) {
				const Square pawnSquare = bitboardToSquare(board.getPieces(color,PAWN));
				if (pawnSquare!=NONE && (passedMask[color][board.getKingSquare(color)]&
						passedMask[color][pawnSquare]&otherKing)) {
					drawishness=100;
				}
			}
		}
		evalInfo.imbalance[color] += MSE((-balance*drawishness)/100); //draw
		evalInfo.drawFlag = (drawishness==100);
	}
	if (!evalInfo.drawFlag) {
		if (pawnDiff>0) {
			evalInfo.imbalance[color] += pawnDiff*PAWN_END_GAME_BONUS;
		} else if (pawnDiff<0) {
			evalInfo.imbalance[other] += (-pawnDiff)*PAWN_END_GAME_BONUS;
		}
		if (hasLightBishop && hasDarkBishop) {
			evalInfo.imbalance[color] += BISHOP_PAIR_BONUS;
		}
		if (otherHasLightBishop && otherHasDarkBishop) {
			evalInfo.imbalance[other] += BISHOP_PAIR_BONUS;
		}
		if (balance!=0) {
			evalInfo.imbalance[color] += (8-sidePawnCount)*TRADE_PAWN_PENALTY;
			evalInfo.imbalance[other] += (7-otherMinors-otherMajors)*TRADE_PIECE_PENALTY;

			evalInfo.imbalance[other] += (pawnCount)*TRADE_PAWN_BONUS;
			evalInfo.imbalance[color] += (minors+majors)*TRADE_PIECE_BONUS;

		}
		if (color==board.getSideToMove()) {
			evalInfo.imbalance[color] += TEMPO_BONUS;
		} else {
			evalInfo.imbalance[other] += TEMPO_BONUS;
		}
		const int tempoPenalty=-MSE(board.getMoveCounter()-(board.getSideToMove()?1:0));
		evalInfo.imbalance[color] += tempoPenalty;
		evalInfo.imbalance[other] += tempoPenalty;

	}
}
