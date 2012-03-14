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
#include <sstream>
#include "bitboard.h"
#include "board.h"

const int lazyEvalMargin=200;
const size_t pawnHashSize=1<<18;

class Evaluator {
public:
	struct EvalInfo {
		EvalInfo(Board& _board) : board(_board),
				side(board.getSideToMove()),
				other(board.flipSide(side)),
				all(board.getAllPieces()),
				eval(0),
				positionalWeight(100),
				tacticalWeight(100),
				drawFlag(false),
				weightedEval(false){
			int i=0;
			pawns[WHITE] = board.getPieces(makePiece(WHITE,PAWN));
			pawns[BLACK] = board.getPieces(makePiece(BLACK,PAWN));
			pawns[COLOR_NONE] = EMPTY_BB;
			for(i=0;i<ALL_PIECE_COLOR;i++) {
				evalPieces[i] = 0;
				evalPawns[i] = 0;
				mobility[i] = 0;
				pieceThreat[i] = 0;
				kingThreat[i]=0;
				imbalance[i]=0;
				attacks[i]=0;
				openfiles[i]=0;
				bestUnstoppable[i]=100;
			}
			for(i=0;i<ALL_PIECE_TYPE_BY_COLOR;i++) {
				attackers[i]=EMPTY_BB;
			}
		}
		Board& board;
		PieceColor side;
		PieceColor other;
		Bitboard all;
		Bitboard attackers[ALL_PIECE_TYPE_BY_COLOR];
		Bitboard attacks[ALL_PIECE_COLOR];
		Bitboard pawns[ALL_PIECE_COLOR];
		Bitboard openfiles[ALL_PIECE_COLOR];
		int kingThreat[ALL_PIECE_COLOR];
		int evalPieces[ALL_PIECE_COLOR];
		int evalPawns[ALL_PIECE_COLOR];
		int mobility[ALL_PIECE_COLOR];
		int pieceThreat[ALL_PIECE_COLOR];
		int imbalance[ALL_PIECE_COLOR];
		int bestUnstoppable[ALL_PIECE_COLOR];
		int eval;
		int positionalWeight;
		int tacticalWeight;
		bool drawFlag;
		bool weightedEval;

		inline const int getScore() {
			const int score=(!drawFlag?(evalPieces[side]-evalPieces[other]) +
					(evalPawns[side]-evalPawns[other]) +
					(mobility[side]-mobility[other]) +
					(pieceThreat[side]-pieceThreat[other]) +
					(kingThreat[side]-kingThreat[other]) +
					(board.getPieceSquareValue(side)-
					board.getPieceSquareValue(other)):0)+
					(imbalance[side]-imbalance[other]);
			return interpolate(score,board.getGamePhase())+
								(board.getMaterial(side)-board.getMaterial(other));
		}

		inline const int getTacticalScore() {
			return (board.getMaterial(side)-board.getMaterial(other))+
					interpolate((imbalance[side]-imbalance[other]),
							board.getGamePhase());
		}

		inline const int getPositionalScore() {
			if (drawFlag) {
				return 0;
			}
			return interpolate((evalPieces[side]-evalPieces[other]) +
					(evalPawns[side]-evalPawns[other]) +
					(mobility[side]-mobility[other]) +
					(pieceThreat[side]-pieceThreat[other]) +
					(kingThreat[side]-kingThreat[other]) +
					(board.getPieceSquareValue(side)-
					board.getPieceSquareValue(other)),
							board.getGamePhase());
		}

		inline const int getEval() {
			return eval;
		}

		inline void computeEval() {
			if (weightedEval) {
				eval += getTacticalScore()*tacticalWeight/100;
				eval += getPositionalScore()*positionalWeight/100;
			} else {
				eval = getScore();
			}
			normalize();
		}

		inline void normalize() {
			if (eval>maxScore) {
				eval=maxScore;
			} else if (eval<-maxScore) {
				eval=-maxScore;
			}
		}

		inline void setWeigthedEval(const int tactical, const int positional) {
			positionalWeight=positional;
			tacticalWeight=tactical;
			weightedEval=!(positional==100&&tactical==100);
		}

		inline std::string toString() {
			std::stringstream out;
			const int whiteScore=evalPieces[WHITE]+evalPawns[WHITE]+mobility[WHITE]+
					pieceThreat[WHITE]+kingThreat[WHITE];
			const int blackScore=evalPieces[BLACK]+evalPawns[BLACK]+mobility[BLACK]+
					pieceThreat[BLACK]+kingThreat[BLACK];
			out << "Material[WHITE]:          " << ((board.getMaterial(WHITE)+
					interpolate(imbalance[WHITE],board.getGamePhase()))-kingValue) << std::endl;
			out << "Material[BLACK]:          " << ((board.getMaterial(BLACK)+
					interpolate(imbalance[BLACK],board.getGamePhase()))-kingValue) << std::endl;
			out << "Pieces(PST&Other)[WHITE]: " << interpolate(evalPieces[WHITE],board.getGamePhase()) << std::endl;
			out << "Pieces(PST&Other)[BLACK]: " << interpolate(evalPieces[BLACK],board.getGamePhase()) << std::endl;
			out << "Pawns[WHITE]:             " << interpolate(evalPawns[WHITE],board.getGamePhase()) << std::endl;
			out << "Pawns[BLACK]:             " << interpolate(evalPawns[BLACK],board.getGamePhase()) << std::endl;
			out << "Mobility&Space[WHITE]:    " << interpolate(mobility[WHITE],board.getGamePhase()) << std::endl;
			out << "Mobility&Space[BLACK]:    " << interpolate(mobility[BLACK],board.getGamePhase()) << std::endl;
			out << "Pieces threats[WHITE]:    " << interpolate(pieceThreat[WHITE],board.getGamePhase()) << std::endl;
			out << "Pieces threats[BLACK]:    " << interpolate(pieceThreat[BLACK],board.getGamePhase()) << std::endl;
			out << "King threats[WHITE]:      " << interpolate(kingThreat[WHITE],board.getGamePhase()) << std::endl;
			out << "King threats[BLACK]:      " << interpolate(kingThreat[BLACK],board.getGamePhase()) << std::endl;
			out << "Endgame score(WHITE):    " << upperScore(whiteScore) << std::endl;
			out << "Endgame score(BLACK):    " << upperScore(blackScore) << std::endl;
			out << "Middlegame score(WHITE): " << lowerScore(whiteScore) << std::endl;
			out << "Middlegame score(BLACK): " << lowerScore(blackScore) << std::endl;
			std::string sign = eval==0?"":(eval>0 && side==WHITE)||(eval<0 && side==BLACK)?"+":"-";
			out << "Main eval:                " << sign << abs(eval) << std::endl;
			return out.str();
		}
	};

	struct PawnInfo {
		Key key;
		int value[ALL_PIECE_COLOR-1];
		Bitboard passers[ALL_PIECE_COLOR-1];
		Bitboard openfiles[ALL_PIECE_COLOR-1];
	}__attribute__ ((aligned(64)));

	Evaluator();
	virtual ~Evaluator();
	const int evaluate(Board& board, const int alpha, const int beta);
	void evalKingPressure(PieceColor color, EvalInfo& evalInfo);
	void evalPawnsFromCache(PieceColor color, PawnInfo& info, EvalInfo& evalInfo);
	void evalPawns(PieceColor color, EvalInfo& evalInfo);
	const int evalPassedPawn(EvalInfo& evalInfo, PieceColor color,
			const Square from, const bool isChained, const bool otherHasOnlyPawns);
	void evalBoardControl(PieceColor color, EvalInfo& evalInfo);
	void evalThreats(PieceColor color, EvalInfo& evalInfo);
	void evalImbalances(EvalInfo& evalInfo);
	const bool isPawnPassed(Board& board, const Square from);
	const bool isPawnUnstoppable(Board& board, const PieceColor color,
			const Square from, const bool otherHasNoMajorPieces);
	const int verifyUnstoppablePawn(Board& board, const PieceColor color,
			const Square from, const bool otherHasNoMajorPieces);
	const void setGameStage(const GamePhase phase);
	template <bool lazySee>
	const int see(Board& board, MoveIterator::Move& move);

	inline const bool isDebugEnabled() {
		return debug;
	}

	inline const void setDebugEnabled(const bool enabled) {
		this->debug = enabled;
	}

	inline const bool isLazyEval() {
		return lazyEval;
	}

	inline const void setLazyEval(const bool _lazyEval) {
		this->lazyEval = _lazyEval;
	}

	inline const int getPositionalWeight() const {
		return positionalWeight;
	}

	inline void setPositionalWeight(const int positional) {
		positionalWeight=positional;
	}

	inline const int getTacticalWeight() const {
		return tacticalWeight;
	}

	inline void setTacticalWeight(const int tactical) {
		tacticalWeight=tactical;
	}

	inline const static int interpolate(const int value, const int gamePhase) {
		const int mgValue = upperScore(value);
		const int egValue = lowerScore(value);
		return (egValue*gamePhase)/maxGamePhase+
				(mgValue*(maxGamePhase-gamePhase))/maxGamePhase;
	}

	inline const int getKingAttackWeight(const int piece, const int count) {
		return kingZoneAttackWeight[pieceType[piece]][count];
	}

	inline bool getPawnInfo(const Key key, PawnInfo& pawnHash) {
		PawnInfo& entry = pawnInfo[(size_t)(key) & (pawnHashSize-1)];
		if (key!=0 && entry.key==key) {
			pawnHash.key=entry.key;
			pawnHash.passers[WHITE]=entry.passers[WHITE];
			pawnHash.passers[BLACK]=entry.passers[BLACK];
			pawnHash.openfiles[WHITE]=entry.openfiles[WHITE];
			pawnHash.openfiles[BLACK]=entry.openfiles[BLACK];
			pawnHash.value[WHITE]=entry.value[WHITE];
			pawnHash.value[BLACK]=entry.value[BLACK];
			return true;
		}
		return false;
	}

	inline void setPawnInfo(const Key key, const int value, const PieceColor color,
			const Bitboard passers, const Bitboard openfiles) {
		PawnInfo& entry = pawnInfo[(size_t)(key) & (pawnHashSize-1)];
		entry.key=key;
		entry.value[color]=value;
		entry.passers[color]=passers;
		entry.openfiles[color]=openfiles;
	}

	inline void cleanPawnInfo() {
		memset(pawnInfo, 0, pawnHashSize * sizeof(PawnInfo));
	}

	inline void setUnstoppableBonus(const PieceColor color, EvalInfo& evalInfo) {
		if (evalInfo.bestUnstoppable[evalInfo.board.flipSide(color)]>
				evalInfo.bestUnstoppable[color]) {
			evalInfo.evalPawns[color]+=UNSTOPPABLE_PAWN_BONUS;
		}
	}

private:
	Bitboard getLeastValuablePiece(Board& board, Bitboard attackers,
			PieceColor& color, PieceTypeByColor& piece);
	PawnInfo pawnInfo[pawnHashSize];
	bool debug;
	bool lazyEval;
	int positionalWeight;
	int tacticalWeight;

};

// verify if pawn is passer
inline const bool Evaluator::isPawnPassed(Board& board, const Square from) {
	const PieceColor color = board.getPieceColor(from);
	const PieceColor other = board.flipSide(color);
	const Bitboard otherPawns = board.getPieces(other,PAWN);
	return !(passedMask[color][from]&otherPawns);
}

// verify if pawn is unstoppable
inline const bool Evaluator::isPawnUnstoppable(Board& board, const PieceColor color,
		const Square from, const bool otherHasNoMajorPieces) {
	return verifyUnstoppablePawn(board,color,from,otherHasNoMajorPieces)<100;
}
// verify if pawn is unstoppable
inline const int Evaluator::verifyUnstoppablePawn(Board& board, const PieceColor color,
		const Square from, const bool otherHasNoMajorPieces) {
	int dist=100;
	if (otherHasNoMajorPieces) {
		const PieceColor other = board.flipSide(color);
		const Rank rank = color==WHITE?RANK_8:RANK_1;
		const Square target = board.makeSquare(rank,squareFile[from]);
		const int delta1 = squareDistance(from,target);
		const int delta2 = squareDistance(board.getKingSquare(other),target);
		const int otherMove=(board.getSideToMove()==other?1:0);
		if (std::min(5,delta1)<delta2-otherMove) {
			dist = delta1+otherMove;
		}
	}
	return dist;
}
// static exchange evaluation
template <bool lazySee>
inline const int Evaluator::see(Board& board, MoveIterator::Move& move) {
	const int gainTableSize=32;
	PieceColor side = board.getPieceColor(move.from);
	PieceColor other = board.flipSide(side);
	PieceTypeByColor firstPiece = board.getPiece(move.from);
	PieceTypeByColor secondPiece = board.getPiece(move.to);
	Bitboard fromPiece = squareToBitboard[move.from];
	Bitboard occupied = board.getAllPieces();
	if (secondPiece==EMPTY && board.getPieceType(firstPiece)==PAWN && board.getEnPassant()!=NONE &&
			board.getSquareFile(move.from)!=board.getSquareFile(move.to)) {
		secondPiece=makePiece(other,PAWN);
		occupied^=squareToBitboard[board.getEnPassant()];
	}
	if (lazySee && secondPiece!=EMPTY &&
			materialValues[secondPiece]>=materialValues[firstPiece]) {
		return materialValues[secondPiece]-materialValues[firstPiece];
	}
	const Bitboard bishopAttacks =  board.getBishopAttacks(move.to,occupied);
	const Bitboard rookAttacks =  board.getRookAttacks(move.to,occupied);
	const Bitboard knightAttacks =  board.getKnightAttacks(move.to);
	const Bitboard pawnAttacks =  board.getPawnAttacks(move.to);
	const Bitboard kingAttacks =  board.getKingAttacks(move.to);
	const Bitboard rooks = board.getPieces(side,ROOK) | board.getPieces(other,ROOK);
	const Bitboard bishops = board.getPieces(side,BISHOP) | board.getPieces(other,BISHOP);
	const Bitboard queens = board.getPieces(side,QUEEN) | board.getPieces(other,QUEEN);
	const Bitboard bishopAndQueen = bishops | queens;
	const Bitboard rookAndQueen = rooks | queens;
	Bitboard attackers =
			(bishopAttacks & bishopAndQueen) |
			(rookAttacks & rookAndQueen) |
			(knightAttacks & (board.getPieces(side,KNIGHT) | board.getPieces(other,KNIGHT))) |
			(pawnAttacks & (board.getPieces(side,PAWN) | board.getPieces(other,PAWN))) |
			(kingAttacks & (board.getPieces(side,KING) | board.getPieces(other,KING)));
	int idx = 0;
	int gain[gainTableSize];
	PieceColor sideToMove = side;
	Bitboard allAttackers = EMPTY_BB;
	Bitboard moreAttackers = EMPTY_BB;
	gain[0] = materialValues[secondPiece];
	if (board.getPieceType(secondPiece)==KING) {
		return queenValue*10;
	}
	while (fromPiece) {
		allAttackers |= attackers;
		idx++;
		gain[idx]  = materialValues[firstPiece] - gain[idx-1];
		attackers ^= fromPiece;
		occupied  ^= fromPiece;
		moreAttackers = bishopAndQueen & ~allAttackers;
		if (moreAttackers) {
			attackers |= moreAttackers & board.getBishopAttacks(move.to,occupied);
		}
		moreAttackers = rookAndQueen & ~allAttackers;
		if (moreAttackers) {
			attackers |= moreAttackers & board.getRookAttacks(move.to,occupied);
		}
		sideToMove=board.flipSide(sideToMove);
		fromPiece  = getLeastValuablePiece (board, attackers, sideToMove, firstPiece);
	}
	while (--idx) {
		gain[idx-1]= -std::max(-gain[idx-1], gain[idx]);
	}
	return gain[0];
}

inline Bitboard Evaluator::getLeastValuablePiece(Board& board, Bitboard attackers,
		PieceColor& color, PieceTypeByColor& piece) {
	const int first = makePiece(color,PAWN);
	const int last = makePiece(color,KING);
	for(register int pieceType = first; pieceType <= last; pieceType++) {
		Bitboard pieces = board.getPieces((PieceTypeByColor)(pieceType)) & attackers;
		if (pieces) {
			piece = (PieceTypeByColor)(pieceType);
			return pieces & -pieces;
		}
	}
	return EMPTY_BB;
}

#endif /* EVALUATOR_H_ */
