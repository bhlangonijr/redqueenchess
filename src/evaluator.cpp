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
	evalImbalances(evalInfo.side, evalInfo);
	evalImbalances(evalInfo.other, evalInfo);
	evalInfo.computeEval();
	if (evalInfo.drawFlag) {
		if (isDebugEnabled()) {
			std::cout << evalInfo.toString();
		}
		return evalInfo.getEval();
	}
	bool doNotLazyEval = evalInfo.getEval() > alpha-lazyEvalMargin && evalInfo.getEval() < beta+lazyEvalMargin;
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
	Board& board = evalInfo.board;
	const PieceColor other = board.flipSide(color);
	const Square kingSq = board.getKingSquare(color);
	const Bitboard kingSquareBB = adjacentSquares[kingSq];
	int pressure = 0;

	const Bitboard notDefended = evalInfo.attacks[other]&
			kingSquareBB&~evalInfo.attacks[color];

	const Bitboard queenContactCheck = ~board.getPieces(other)&
			notDefended&evalInfo.attackers[makePiece(other,QUEEN)];

	if (queenContactCheck) {
		const Bitboard safeCheck = queenContactCheck &
				(evalInfo.attackers[makePiece(other,PAWN)] |
						evalInfo.attackers[makePiece(other,KNIGHT)] |
						evalInfo.attackers[makePiece(other,BISHOP)] |
						evalInfo.attackers[makePiece(other,ROOK)]);
		if (safeCheck) {
			pressure+=(other==board.getSideToMove()?2:1) *
					bitCount15(safeCheck)*QUEEN_CHECK_BONUS;
		}
	}
	const Bitboard rookContactCheck = ~board.getPieces(other)&
			notDefended&evalInfo.attackers[makePiece(other,ROOK)];

	if (rookContactCheck) {
		const Bitboard safeCheck = rookContactCheck &
				(evalInfo.attackers[makePiece(other,PAWN)] |
						evalInfo.attackers[makePiece(other,KNIGHT)] |
						evalInfo.attackers[makePiece(other,BISHOP)] |
						evalInfo.attackers[makePiece(other,QUEEN)]);
		if (safeCheck) {
			pressure+= (other==board.getSideToMove()?2:1)*
					bitCount15(safeCheck)*ROOK_CHECK_BONUS;
		}
	}

	const Bitboard clearWay = ~(board.getPieces(other)|evalInfo.attacks[color]);
	const Bitboard rookAttacks = clearWay&board.getRookAttacks(kingSq);
	const Bitboard bishopAttacks = clearWay&board.getBishopAttacks(kingSq);

	const Bitboard queenIndirectAttacks = (rookAttacks|bishopAttacks)&
			evalInfo.attackers[makePiece(other,QUEEN)];
	if (queenIndirectAttacks) {
		pressure += bitCount15(queenIndirectAttacks)*INDIRECT_QUEEN_CHECK_BONUS;
	}

	const Bitboard rookIndirectAttacks = rookAttacks &
			evalInfo.attackers[makePiece(other,ROOK)];
	if (rookIndirectAttacks) {
		pressure += bitCount15(rookIndirectAttacks)*INDIRECT_ROOK_CHECK_BONUS;
	}

	const Bitboard bishopIndirectAttacks = bishopAttacks &
			evalInfo.attackers[makePiece(other,BISHOP)];
	if (bishopIndirectAttacks) {
		pressure += bitCount15(bishopIndirectAttacks)*INDIRECT_BISHOP_CHECK_BONUS;
	}

	const Bitboard kinghtIndirectAttacks = board.getKnightAttacks(kingSq) &
			evalInfo.attackers[makePiece(other,KNIGHT)]&clearWay;
	if (kinghtIndirectAttacks) {
		pressure += bitCount15(kinghtIndirectAttacks)*INDIRECT_KNIGHT_CHECK_BONUS;
	}

	evalInfo.kingThreat[color] = -pressure;
	evalInfo.evalPieces[color] +=
			board.isCastleDone(color)?DONE_CASTLE_BONUS:
	board.getCastleRights(color)==NO_CASTLE?-DONE_CASTLE_BONUS:0;
}

void Evaluator::evalPawnsFromCache(PieceColor color, PawnInfo& info, EvalInfo& evalInfo) {
	const Bitboard pawns = evalInfo.pawns[color];
	evalInfo.evalPawns[color]+=info.value[color];
	Bitboard passed=info.passers[color] & pawns;
	if (passed) {
		Square from = extractLSB(passed);
		while ( from!=NONE ) {
			const Bitboard pawn = squareToBitboard[from];
			const Bitboard allButThePawn =(pawns^pawn);
			const Bitboard chainSquares = backwardPawnMask[color][from]&adjacentSquares[from];
			const bool isChained = (chainSquares&allButThePawn);
			evalInfo.evalPawns[color] += evalPassedPawn(evalInfo,color,from,isChained);
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
	int passedBonus=0;
	int eval=0;
	//penalyze doubled, isolated and backward pawns
	//bonus to passer and candidates
	if (pawns) {
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
			if (isDoubled) {
				eval += DOUBLED_PAWN_PENALTY;
			}
			if (isIsolated) {
				eval += halfOpenFile?ISOLATED_OPEN_PAWN_PENALTY:ISOLATED_PAWN_PENALTY;
			} else if (adjacentSquares[from]&allButThePawn) {
				eval += CONNECTED_PAWN_BONUS;
			}
			if (isPasser && !(isDoubled && (frontSquares[color][from]&allButThePawn))) {
				passedBonus += evalPassedPawn(evalInfo,color,from,isChained);
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
	setPawnInfo(board.getPawnKey(),eval,color,passers);
	evalInfo.evalPawns[color] += eval+passedBonus;
}
// Eval passed pawns
const int Evaluator::evalPassedPawn(EvalInfo& evalInfo, PieceColor color, const Square from, const bool isChained) {
	int eval=0;
	Board& board = evalInfo.board;
	const PieceColor other = board.flipSide(color);
	const Square otherKingSq = board.getKingSquare(other);
	const Bitboard pawnsAndKing = board.getPieces(other,PAWN) |
			board.getPieces(other,KING);
	const bool otherHasOnlyPawns = !(pawnsAndKing^board.getPieces(other));
	if (isChained) {
		eval += connectedPasserBonus[color][squareRank[from]];
	} else {
		eval += passedPawnBonus[color][squareRank[from]];
	}
	eval += pawnWeight[color][squareFile[from]];
	if (otherHasOnlyPawns) {
		const Rank rank = color==WHITE?RANK_8:RANK_1;
		const Square target = board.makeSquare(rank,squareFile[from]);
		const int delta1 = squareDistance(from,target);
		const int delta2 = squareDistance(otherKingSq,target);
		const int otherMove=(board.getSideToMove()==other?1:0);
		if (std::min(5,delta1)<delta2-otherMove) {
			eval += UNSTOPPABLE_PAWN_BONUS;
		}
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
	const Bitboard freeArea = ~(board.getPieces(color) |
			evalInfo.attackers[makePiece(other,PAWN)]);
	Bitboard pieces = EMPTY_BB;
	pieces = board.getAllPieces();
	pieces = (color==WHITE?pieces>>8:pieces<<8) & board.getPieces(color,PAWN);
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
				evalInfo.attackers[makePiece(color,PAWN)]&squareToBitboard[from] &&
				!(evalInfo.attackers[makePiece(other,PAWN)]&squareToBitboard[from])) {
			evalInfo.evalPieces[color] += knightOutpostBonus[color][from];
		}
		from = extractLSB(pieces);
	}
	pieces = board.getPieces(color,BISHOP);
	from = extractLSB(pieces);
	while ( from!=NONE ) {
		const Bitboard attacks = board.getBishopAttacks(from);
		evalInfo.attackers[makePiece(color,BISHOP)] |= attacks;
		evalInfo.mobility[color] += bishopMobility[bitCount(attacks&freeArea)];
		from = extractLSB(pieces);
	}
	pieces = board.getPieces(color,ROOK);
	from = extractLSB(pieces);
	while ( from!=NONE ) {
		const Bitboard attacks = board.getRookAttacks(from);
		evalInfo.attackers[makePiece(color,ROOK)] |= attacks;
		evalInfo.mobility[color] += rookMobility[bitCount(attacks&freeArea)];
		if ((squareToBitboard[from] & promoRank[color]) &&
				(board.getPieces(other,KING) & eighthRank[color])) {
			evalInfo.evalPieces[color] += ROOK_ON_7TH_RANK_BONUS;
		}
		if (!(evalInfo.pawns[color]&fileBB[squareFile[from]])) {
			if (!(evalInfo.pawns[other]&fileBB[squareFile[from]])) {
				evalInfo.evalPieces[color] += ROOK_ON_OPEN_FILE_BONUS;
			} else {
				evalInfo.evalPieces[color] += ROOK_ON_HALF_OPEN_FILE_BONUS;
			}
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
				(board.getPieces(other,KING) & eighthRank[color])) {
			evalInfo.evalPieces[color] += QUEEN_ON_7TH_RANK_BONUS;
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
	const int sidePawnCount = board.getPieceCount(makePiece(color,PAWN));
	const int otherPawnCount = board.getPieceCount(makePiece(other,PAWN));
	const int pawnCount = sidePawnCount + otherPawnCount;
	const Bitboard bishop = board.getPieces(color,BISHOP);
	const Bitboard otherKing = board.getPieces(other,KING);
	const int pawnDiff = sidePawnCount-otherPawnCount;
	const bool hasLightBishop = bishop & WHITE_SQUARES;
	const bool hasBlackBishop = bishop & BLACK_SQUARES;
	const int totalPieceCount = board.getPieceCount(color)+
			board.getPieceCount(other);
	const int balance = board.getMaterial(color)-
			board.getMaterial(other);

	const int sideMinors = board.getPieceCount(makePiece(color,BISHOP))+
			board.getPieceCount(makePiece(color,KNIGHT));
	const int otherMinors = board.getPieceCount(makePiece(other,BISHOP))+
			board.getPieceCount(makePiece(other,KNIGHT));
	const int sideMajors = board.getPieceCount(makePiece(color,ROOK))+
			board.getPieceCount(makePiece(color,QUEEN));
	const int otherMajors = board.getPieceCount(makePiece(other,ROOK))+
			board.getPieceCount(makePiece(other,QUEEN));

	const int minors = sideMinors + otherMinors;
	const int majors = sideMajors + otherMajors;

	if (totalPieceCount < 6) {
		if (majors==0 && pawnCount==0 && balance >= 0) {
			if (minors <= 1 || (minors == 2 && sideMinors==otherMinors)) {
				evalInfo.imbalance[color] += MSE(-balance); //draw
				evalInfo.drawFlag = true;
			} else if (minors <= 3 && otherMinors>0) {
				evalInfo.imbalance[color] += MSE(-balance*90/100); //drawish -
			} else if (minors <= 2 && sideMinors==2 &&
					otherMinors==0 && !board.getPieces(color,BISHOP)) {
				evalInfo.imbalance[color] += MSE(-balance); //draw - KNN vs K
				evalInfo.drawFlag = true;
			}
		} else if (majors==0 && pawnCount<=2 &&	balance >= 0) {
			const Bitboard sidePawns = board.getPieces(color,PAWN);
			const Bitboard otherPawns = board.getPieces(other,PAWN);
			const bool pawnOnAFile = sidePawns & fileBB[FILE_A];
			const bool pawnOnHFile = sidePawns & fileBB[FILE_H];
			if (minors==1 && sidePawnCount==1 && sideMinors==1 &&
					otherPawnCount==0 && !board.getPieces(color,KNIGHT)) {
				if (pawnOnAFile || pawnOnHFile) {
					const bool wrongBishop = color==WHITE?
							(pawnOnHFile && hasLightBishop) ||
							(pawnOnAFile && hasBlackBishop):
							(pawnOnHFile && hasBlackBishop) ||
							(pawnOnAFile && hasLightBishop);
					if ((pawnOnAFile && wrongBishop && (otherKing & kingFileACorner[other])) ||
							(pawnOnHFile && wrongBishop && (otherKing & kingFileHCorner[other]))) {
						evalInfo.imbalance[color] += MSE(-balance); //draw -- wrong bishop
						evalInfo.drawFlag = true;
					}
				}
			} else if (minors <= 1 && sideMinors==1 && sidePawnCount==1 &&
					otherPawnCount==1 && !board.getPieces(color,KNIGHT)) {
				if ((sidePawns & blockedRank[color]) & (color==WHITE?otherPawns>>8:otherPawns<<8)) {
					if (pawnOnAFile || pawnOnHFile) {
						if ((pawnOnAFile && (otherKing & blockRankOnAFile[other])) ||
								(pawnOnHFile && (otherKing & blockRankOnHFile[other]))) {
							evalInfo.imbalance[color] += MSE(-balance); //draw -- rook pawn
							evalInfo.drawFlag = true;
						}
					}
				}
			} else if (minors == 1 && sideMinors==1 &&
					sidePawnCount==0 && otherPawnCount==1) {
				evalInfo.imbalance[color] += MSE(-balance*90/100); //drawish
			} else if (minors==0 && sidePawnCount==1 &&	otherPawnCount==0) {
				const Square pawnSquare = bitboardToSquare(board.getPieces(color,PAWN));
				if (pawnSquare!=NONE && (passedMask[color][board.getKingSquare(color)]&
						passedMask[color][pawnSquare]&otherKing)) {
					evalInfo.imbalance[color] += MSE(-balance); //draw -- king opposition
					evalInfo.drawFlag = true;
				}
			}
		}
	}
	if (!evalInfo.drawFlag) {
		if (pawnDiff>0) {
			evalInfo.imbalance[color] += pawnDiff*PAWN_END_GAME_BONUS;
		}
		if (hasLightBishop && hasBlackBishop) {
			evalInfo.imbalance[color] += BISHOP_PAIR_BONUS;
		}
		if (balance!=0) {
			const int sign = balance>0?+1:-1;
			evalInfo.imbalance[color] += (8-sidePawnCount)*sign*TRADE_PAWN_PENALTY;
			evalInfo.imbalance[color] += (7-sideMinors-sideMajors)*sign*TRADE_PIECE_BONUS;
		}
	}
}
