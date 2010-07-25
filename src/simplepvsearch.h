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
 * SimplePVSearch.h
 *
 *  Created on: 18/05/2009
 *      Author: bhlangonijr
 */

#ifndef SIMPLEPVSEARCH_H_
#define SIMPLEPVSEARCH_H_

#include <time.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "searchagent.h"
#include "evaluator.h"

// game constants
const int maxScoreRepetition = 4;
const int mateRangeScore = 300;
const int maxSearchDepth = 80;
const int maxSearchPly = 100;

// internal iterative deepening
const int allowIIDAtPV = 5;
const int allowIIDAtNormal = 11;

// margin constants
#define futilityMargin(depth) (150 + depth * 200)
const int iidMargin=250;
const int easyMargin=500;

//depth prunning threshold
const int aspirationDepth=6;
const int futilityDepth=3;
const int lmrDepthThreshold=2;
const int lateMoveThreshold=2;

const int scoreTable[11]={0,8000,5000,9500,9000,4500,4000,100,-900,5000};

class SearchAgent;

class MoveIterator;

class SimplePVSearch {

public:

	typedef struct PvLine {
		int index;
		MoveIterator::Move moves[maxSearchPly+1];
	} PvLine;

	typedef struct SearchInfo {

		SearchInfo(): eval(-maxScore), inCheck(false){}
		SearchInfo(const int _eval, const bool _inCheck): eval(_eval), inCheck(_inCheck){}
		int eval;
		bool inCheck;

	} SearchInfo;

	typedef struct SearchStats {

		SearchStats():
			ttHits(0), ttLower(0), ttUpper(0), ttExact(0), nullMoveHits(0),
			pvChanges(0), searchTime(0), searchNodes(0), searchDepth(0),
			bestMove(MoveIterator::Move()),ponderMove(MoveIterator::Move()) {}

		SearchStats(const SearchStats& stats):
			ttHits(stats.ttHits), ttLower(stats.ttLower),
			ttUpper(stats.ttUpper), ttExact(stats.ttExact),
			nullMoveHits(stats.nullMoveHits), pvChanges(stats.pvChanges),
			searchTime(stats.searchTime),searchNodes(stats.searchNodes),
			searchDepth(stats.searchDepth),bestMove(stats.bestMove),
			ponderMove(stats.ponderMove) {}
		long ttHits;
		long ttLower;
		long ttUpper;
		long ttExact;
		long nullMoveHits;
		long pvChanges;
		int searchTime;
		uint64_t searchNodes;
		int searchDepth;
		MoveIterator::Move bestMove;
		MoveIterator::Move ponderMove;

		std::string toString() {
			std::string result =
					"ttHits:      " + StringUtil::toStr(ttHits) + "\n" ;
			result += "ttLower:     " + StringUtil::toStr(ttLower) + "\n";
			result += "ttUpper:     " + StringUtil::toStr(ttUpper) + "\n";
			result += "ttExact:     " + StringUtil::toStr(ttExact) + "\n";
			result += "nullMoveHits:" + StringUtil::toStr(nullMoveHits) + "\n";
			result += "pvChanges:   " + StringUtil::toStr(pvChanges) + "\n";
			result += "searchTime:  " + StringUtil::toStr(searchTime) + "\n";
			result += "searchNodes: " + StringUtil::toStr(searchNodes) + "\n";
			result += "searchDepth: " + StringUtil::toStr(searchDepth) + "\n";
			result += "bestMove:    " + bestMove.toString() + "\n";
			result += "ponderMove:  " + ponderMove.toString() + "\n";
			return result;
		}
		void clear() {
			ttHits=0;
			ttLower=0;
			ttUpper=0;
			ttExact=0;
			nullMoveHits=0;
			pvChanges=0;
			searchTime=0;
			searchNodes=0;
			searchDepth=0;
			bestMove=MoveIterator::Move();
			ponderMove=MoveIterator::Move();
		}

	} SearchStats;

	SimplePVSearch() : _depth(maxSearchDepth), _updateUci(true), _startTime(0), _searchFixedDepth(false), _infinite(false), _nodes(0) {}

	virtual ~SimplePVSearch() {}

	virtual void search(Board& _board);

	virtual long perft(Board& board, int depth, int ply);

	virtual int getScore();

	inline const long getTickCount() {
		return ((clock() * 1000) / CLOCKS_PER_SEC);
	}

	inline const long toClock(const long time) {
		return long((((double)(time)/(double)1000)*(double)CLOCKS_PER_SEC));
	}

	inline const bool isUpdateUci() const {
		return _updateUci;
	}

	inline const void setUpdateUci(const bool value) {
		_updateUci = value;
	}

	inline const bool isSearchFixedDepth() const {
		return _searchFixedDepth;
	}

	inline const void setSearchFixedDepth(const bool value) {
		_searchFixedDepth = value;
	}

	inline const bool isInfinite() const {
		return _infinite;
	}

	inline const void setInfinite(const bool value) {
		_infinite = value;
	}

	inline const void setDepth(const int depth) {
		_depth = depth;
	}

	inline const int getDepth() const {
		return _depth;
	}

	inline const void setTimeToSearch(const long timeToSearch) {
		_timeToSearch = timeToSearch;
	}

	inline const long getTimeToSearch() const {
		return _timeToSearch;
	}

	inline const void setStartTime(const long startTime) {
		_startTime = startTime;
	}

	inline const long getStartTime() const {
		return _startTime;
	}

private:

	int _depth;
	int _score;
	bool _updateUci;
	long _timeToSearch;
	long _startTime;
	long _time;
	bool _searchFixedDepth;
	bool _infinite;
	long _nodes;
	SearchStats stats;
	long timeToStop;
	MoveIterator rootMoves;
	MoveIterator::Move killer[maxSearchPly+1][2];
	long nodesPerMove[MOVE_LIST_MAX_SIZE];
	int history[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE];
	Evaluator evaluator;
	SearchAgent* agent;
	MoveIterator::Move emptyMove;
	SearchInfo rootSearchInfo;

	int idSearch(Board& board);
	int rootSearch(Board& board, SearchInfo& si, int alpha, int beta, int depth, int ply, PvLine* pv);
	int pvSearch(Board& board,  SearchInfo& si, int alpha, int beta, int depth, int ply, PvLine* pv);
	int zwSearch(Board& board,  SearchInfo& si, int beta, int depth, int ply, PvLine* pv, const bool allowNullMove);
	int qSearch(Board& board,  SearchInfo& si, int alpha, int beta, int depth, int ply, PvLine* pv);
	void uciOutput(PvLine* pv, MoveIterator::Move& bestMove, const int totalTime,
			const int hashFull, const int depth, const int alpha, const int beta);
	void uciOutput(MoveIterator::Move& bestMove, MoveIterator::Move& ponderMove);
	void uciOutput(MoveIterator::Move& move, const int moveCounter);
	const std::string pvLineToString(const PvLine* pv);
	MoveIterator::Move& selectMove(Board& board, MoveIterator& moves,
			MoveIterator::Move& ttMove, bool isKingAttacked, int ply);
	MoveIterator::Move& selectMove(Board& board, MoveIterator& moves, bool isKingAttacked);
	void scoreMoves(Board& board, MoveIterator& moves);
	void scoreRootMoves(Board& board, MoveIterator& moves);
	void filterLegalMoves(Board& board, MoveIterator& moves);
	bool okToReduce(Board& board, MoveIterator::Move& move,	bool isKingAttacked, const bool isGivingCheck,
			const bool nullMoveMateScore, int ply);
	bool okToPrune(Board& board, MoveIterator::Move& move, MoveIterator::Move& hashMove,
			bool isKingAttacked, const bool isGivingCheck, const bool nullMoveMateScore, const int depth);
	bool okToNullMove(Board& board);
	bool isMateScore(const int score);
	bool isPawnFinal(Board& board);
	bool isPawnPush(Board& board, MoveIterator::Move& move);
	bool isPawnPromoting(const Board& board);
	bool adjustDepth(int& extension, int& reduction, Board& board, MoveIterator::Move& move, int depth,
			int remainingMoves, bool isKingAttacked, bool isGivingCheck, int ply, const bool nullMoveMateScore, const bool isPV);
	void updatePv(PvLine* pv, PvLine& line, MoveIterator::Move& move);
	const bool stop(const bool shouldStop);
	const bool timeIsUp();
	void clearHistory();
	void updateHistory(Board& board, MoveIterator::Move& move, int depth);
	void updateKillers(Board& board, MoveIterator::Move& move, int ply);
	void initRootMovesOrder();
	void updateRootMovesScore(const long value);
	long predictTimeUse(const long iterationTime[maxSearchPly], const long totalTime, const int depth);

};
// select a move
inline MoveIterator::Move& SimplePVSearch::selectMove(Board& board, MoveIterator& moves,
		MoveIterator::Move& ttMove, bool isKingAttacked, int ply) {

	MoveIterator::Move& killer1 = killer[ply][0];
	MoveIterator::Move& killer2 = killer[ply][1];

	if (moves.getStage()==MoveIterator::BEGIN_STAGE) {
		if (!isKingAttacked) {
			moves.setStage(MoveIterator::INIT_CAPTURE_STAGE);
		} else {
			moves.setStage(MoveIterator::INIT_EVASION_STAGE);
		}
		if (board.isMoveLegal(ttMove)) {
			return ttMove;
		}
	}

	if (moves.getStage()==MoveIterator::INIT_EVASION_STAGE) {
		board.generateEvasions(moves, board.getSideToMove());
		scoreMoves(board, moves);
		moves.setStage(MoveIterator::ON_EVASION_STAGE);
	}

	if (moves.getStage()==MoveIterator::ON_EVASION_STAGE) {
		if (moves.hasNext()) {
			return moves.selectBest();
		}
		moves.setStage(MoveIterator::END_STAGE);
		return emptyMove;
	}

	if (moves.getStage()==MoveIterator::INIT_CAPTURE_STAGE) {
		board.generateCaptures(moves, board.getSideToMove());
		scoreMoves(board, moves);
		moves.setStage(MoveIterator::ON_CAPTURE_STAGE);
	}

	if (moves.getStage()==MoveIterator::ON_CAPTURE_STAGE) {
		while (moves.hasNext()) {
			MoveIterator::Move& move=moves.selectBest();
			if (move==ttMove) {
				continue;
			}
			if (move.type==MoveIterator::BAD_CAPTURE) {
				moves.prior();
				break;
			}
			return move;
		}
		moves.setStage(MoveIterator::KILLER1_STAGE);
	}

	if (moves.getStage()==MoveIterator::KILLER1_STAGE) {
		moves.setStage(MoveIterator::KILLER2_STAGE);
		if (killer1 != ttMove && board.isMoveLegal(killer1)) {
			return killer1;
		}
	}

	if (moves.getStage()==MoveIterator::KILLER2_STAGE) {
		moves.setStage(MoveIterator::INIT_QUIET_STAGE);
		if (killer2 != ttMove && board.isMoveLegal(killer2)) {
			return killer2;
		}
	}

	if (moves.getStage()==MoveIterator::INIT_QUIET_STAGE) {
		board.generateNonCaptures(moves, board.getSideToMove());
		scoreMoves(board, moves);
		moves.setStage(MoveIterator::ON_QUIET_STAGE);
	}

	if (moves.getStage()==MoveIterator::ON_QUIET_STAGE) {
		while (moves.hasNext()) {
			MoveIterator::Move& move=moves.selectBest();
			if (move==ttMove || move==killer1 || move==killer2) {
				continue;
			}
			return move;
		}
		moves.setStage(MoveIterator::END_STAGE);
	}

	return emptyMove;

}

// select a move
inline MoveIterator::Move& SimplePVSearch::selectMove(Board& board, MoveIterator& moves, bool isKingAttacked) {

	if (moves.getStage()==MoveIterator::BEGIN_STAGE) {
		if (!isKingAttacked) {
			moves.setStage(MoveIterator::INIT_CAPTURE_STAGE);
		} else {
			moves.setStage(MoveIterator::INIT_EVASION_STAGE);
		}
	}

	if (moves.getStage()==MoveIterator::INIT_EVASION_STAGE) {
		board.generateEvasions(moves, board.getSideToMove());
		scoreMoves(board, moves);
		moves.first();
		moves.setStage(MoveIterator::ON_EVASION_STAGE);
	}

	if (moves.getStage()==MoveIterator::ON_EVASION_STAGE) {
		if (moves.hasNext()) {
			return moves.selectBest();
		}
		moves.setStage(MoveIterator::END_STAGE);
		return emptyMove;
	}

	if (moves.getStage()==MoveIterator::INIT_CAPTURE_STAGE) {
		board.generateCaptures(moves, board.getSideToMove());
		scoreMoves(board, moves);
		moves.first();
		moves.setStage(MoveIterator::ON_CAPTURE_STAGE);
	}

	if (moves.getStage()==MoveIterator::ON_CAPTURE_STAGE) {
		while (moves.hasNext()) {
			return moves.selectBest();
		}
		moves.setStage(MoveIterator::END_STAGE);
	}

	return emptyMove;

}

// score moves
inline void SimplePVSearch::scoreMoves(Board& board, MoveIterator& moves) {

	moves.bookmark();

	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();
		const PieceTypeByColor pieceFrom = board.getPieceBySquare(move.from);
		const PieceTypeByColor pieceTo = board.getPieceBySquare(move.to);

		if (pieceTo != EMPTY) {
			move.score = evaluator.see(board,move);

			if (move.type==MoveIterator::UNKNOW) {
				if (move.score >= 0) {
					move.type=MoveIterator::GOOD_CAPTURE;
				} else {
					move.type=MoveIterator::BAD_CAPTURE;
				}
			}
		} else {
			if (move.type==MoveIterator::UNKNOW) {
				move.type=MoveIterator::NON_CAPTURE;
				const int hist = history[pieceFrom][move.to];
				const GamePhase phase = board.getGamePhase();
				const int gain =
						evaluator.getPieceSquareValue(pieceFrom,move.to,phase)-
						evaluator.getPieceSquareValue(pieceFrom,move.from,phase);
				move.score=hist+gain;
			}
		}

		move.score+=scoreTable[move.type];
	}
	moves.goToBookmark();

}

inline void SimplePVSearch::scoreRootMoves(Board& board, MoveIterator& moves) {

	moves.bookmark();

	while (moves.hasNext()) {

		MoveIterator::Move& move = moves.next();
		MoveBackup backup;
		const PieceTypeByColor pieceFrom = board.getPieceBySquare(move.from);
		const PieceTypeByColor pieceTo = board.getPieceBySquare(move.to);
		board.doMove(move,backup);

		const bool givingCheck = board.isAttacked(board.getSideToMove(),KING);

		SearchInfo newSi(rootSearchInfo.eval,givingCheck);
		PvLine pv;
		move.score = -pvSearch(board,newSi,-maxScore,maxScore,1,0,&pv);

		if (pieceTo!=EMPTY) {

			const int value = defaultMaterialValues[pieceTo] -
					defaultMaterialValues[pieceFrom];

			if (move.type==MoveIterator::UNKNOW) {
				if (value >= 0) {
					move.type=MoveIterator::GOOD_CAPTURE;
				} else {
					move.type=MoveIterator::BAD_CAPTURE;
				}
			}
		} else {
			if (move.type==MoveIterator::UNKNOW) {
				move.type=MoveIterator::NON_CAPTURE;
			}
		}
		board.undoMove(backup);
	}
	moves.goToBookmark();

}

// filter the move list to only legal moves
inline void SimplePVSearch::filterLegalMoves(Board& board, MoveIterator& moves) {

	MoveIterator newMoves;

	moves.first();
	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();
		MoveBackup backup;
		board.doMove(move,backup);
		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue;
		}
		newMoves.add(move);
		board.undoMove(backup);
	}
	moves.clear();
	moves.addAll(newMoves);
	moves.first();
}

// Checks if search can be reduced for a given move
inline bool SimplePVSearch::okToReduce(Board& board, MoveIterator::Move& move,
		bool isKingAttacked, const bool isGivingCheck, const bool nullMoveMateScore, int ply) {

	bool verify = (
			move.type == MoveIterator::NON_CAPTURE &&
			move!=killer[ply][0] &&
			move!=killer[ply][1] &&
			!isKingAttacked &&
			!isGivingCheck &&
			!isPawnPush(board,move) &&
			!nullMoveMateScore //&&
			//!history[board.getPieceBySquare(move.from)][move.to]
	);

	return verify;

}

// Checks if the given move can be prunned
inline bool SimplePVSearch::okToPrune(Board& board, MoveIterator::Move& move, MoveIterator::Move& hashMove,
		bool isKingAttacked, const bool isGivingCheck, const bool nullMoveMateScore, const int depth) {

	bool verify = (
			depth < futilityDepth &&
			!isKingAttacked &&
			move != hashMove &&
			!isGivingCheck &&
			!isPawnPush(board,move) &&
			!isPawnPromoting(board) &&
			!nullMoveMateScore &&
			move.type == MoveIterator::NON_CAPTURE
	);

	return verify;

}

// Ok to do null move?
inline bool SimplePVSearch::okToNullMove(Board& board) {
	return !isPawnFinal(board);
}

// is mate score?
inline bool SimplePVSearch::isMateScore(const int score) {
	return score < -maxScore+maxSearchPly ||
	score > maxScore-maxSearchPly;
}

// remains pawns & kings only?
inline bool SimplePVSearch::isPawnFinal(Board& board) {

	const Bitboard pawns = board.getPiecesByType(WHITE_PAWN) |
			board.getPiecesByType(BLACK_PAWN);
	const Bitboard kings = board.getPiecesByType(WHITE_KING) |
			board.getPiecesByType(BLACK_KING);

	return !((pawns|kings)^board.getAllPieces());
}

// pawn push
inline bool SimplePVSearch::isPawnPush(Board& board, MoveIterator::Move& move) {

	if (board.getPieceTypeBySquare(move.to)!=PAWN) {
		return false;
	}

	return evaluator.isPawnPassed(board,move.to);
}

// pawn promoting
inline bool SimplePVSearch::isPawnPromoting(const Board& board) {

	return (board.getPiecesByType(WHITE_PAWN) & rankBB[RANK_7]) ||
			(board.getPiecesByType(BLACK_PAWN) & rankBB[RANK_2]);

}

// depth reduction
inline bool SimplePVSearch::adjustDepth(int& extension, int& reduction,
		Board& board, MoveIterator::Move& move, int depth, int remainingMoves,
		bool isKingAttacked, bool isGivingCheck, int ply, const bool nullMoveMateScore, const bool isPV) {

	extension=0;
	reduction=0;

	if (isKingAttacked) {
		extension=1;
		return false;
	}

	if (!okToReduce(board, move, isKingAttacked, isGivingCheck, nullMoveMateScore, ply)) {
		return false;
	}

	if (remainingMoves>lateMoveThreshold &&
			depth>lmrDepthThreshold) {
		reduction= 1;
		if (!isPV && depth > 7) {
			reduction+=depth/8;
		}
		return true;
	}

	return false;
}

inline const bool SimplePVSearch::stop(const bool shouldStop) {

	return (timeIsUp() || shouldStop);

}

inline const bool SimplePVSearch::timeIsUp() {

	if (_searchFixedDepth || _infinite) {
		return false;
	}

	if (_nodes & 0xFFF) {
		return false;
	}

	return clock()>=timeToStop;

}

inline void SimplePVSearch::uciOutput(PvLine* pv, MoveIterator::Move& bestMove,
		const int totalTime, const int hashFull, const int depth,
		const int alpha, const int beta) {

	if (isUpdateUci() && !bestMove.none() && !pv->moves[0].none()) {

		std::string scoreString = "cp " + StringUtil::toStr(bestMove.score);

		if (abs(bestMove.score) > (maxScore-mateRangeScore)) {
			if (bestMove.score>0) {
				scoreString = "mate " +StringUtil::toStr((maxScore - (bestMove.score+1))/2);
			} else {
				scoreString = "mate " +StringUtil::toStr(-(maxScore + bestMove.score)/2);
			}
		}

		if (bestMove.score >= beta) {
			scoreString += " lowerbound";
		} else if (bestMove.score <= alpha) {
			scoreString += " upperbound";
		}

		long nps = totalTime>1000 ?  ((_nodes)/(totalTime/1000)) : _nodes;
		std::cout << "info depth "<< depth << std::endl;
		std::cout << "info depth "<< depth << " score " << scoreString << " time " << totalTime
				<< " nodes " << (_nodes) << " nps " << nps << " pv" << pvLineToString(pv) << std::endl;
		std::cout << "info nodes " << (_nodes) << " time " << totalTime << " nps " << nps
				<< " hashfull " << hashFull << std::endl;

	}
}

inline void SimplePVSearch::uciOutput(MoveIterator::Move& bestMove, MoveIterator::Move& ponderMove) {

	if (isUpdateUci()) {
		if (!bestMove.none()) {
			if (isUpdateUci()) {
				std::cout << "bestmove " << bestMove.toString();
				if (!ponderMove.none()) {
					std::cout << " ponder "<< ponderMove.toString();
				}
				std::cout << std::endl;
			}
		} else {
			std::cout << "bestmove (none)" << std::endl;
		}
	}
}

inline void SimplePVSearch::uciOutput(MoveIterator::Move& move, const int moveCounter) {

	const long uciOutputSecs=1500;
	if (isUpdateUci()) {
		if (_startTime+uciOutputSecs < getTickCount()) {
			std::cout << "info currmove " << move.toString() << " currmovenumber " << moveCounter << std::endl;
		}
	}
}
inline const std::string SimplePVSearch::pvLineToString(const PvLine* pv) {
	std::string result="";
	for(int x=0;x<pv->index;x++) {
		result += " ";
		result += pv->moves[x].toString();
	}
	return result;
}

inline void SimplePVSearch::updatePv(PvLine* pv, PvLine& line, MoveIterator::Move& move) {
	pv->moves[0] = move;
	memcpy(pv->moves + 1, line.moves, line.index * sizeof(MoveIterator::Move));
	pv->index = line.index + 1;
}

// clear history
inline void SimplePVSearch::clearHistory() {
	memset(history, 0, sizeof(int)*ALL_PIECE_TYPE_BY_COLOR*ALL_SQUARE);
	memset(killer, 0, sizeof(MoveIterator::Move)*maxSearchPly*2);
}

// update history
inline void SimplePVSearch::updateHistory(Board& board, MoveIterator::Move& move, int depth) {

	if (board.getPieceBySquare(move.to)!=EMPTY ||
			move.type == MoveIterator::PROMO_NONCAPTURE ||
			move.type == MoveIterator::PROMO_CAPTURE ||
			move.type == MoveIterator::CASTLE ||
			move.none()) {
		return;
	}

	history[board.getPieceBySquare(move.from)][move.to]+=depth*depth;

}

// update killers
inline void SimplePVSearch::updateKillers(Board& board, MoveIterator::Move& move, int ply) {
	if (board.getPieceBySquare(move.to)!=EMPTY ||
			move.type == MoveIterator::PROMO_NONCAPTURE ||
			move.type == MoveIterator::PROMO_CAPTURE ||
			move.type == MoveIterator::CASTLE ||
			move.none()) {
		return;
	}
	if (move != killer[ply][0]) {
		killer[ply][1] = killer[ply][0];
		killer[ply][0] = move;
	}
}

// init root moves ordering array
inline void SimplePVSearch::initRootMovesOrder() {
	for(int x=0;x<MOVE_LIST_MAX_SIZE;x++) {
		nodesPerMove[x]=0L;
	}
}

// update root moves score
inline void SimplePVSearch::updateRootMovesScore(const long value) {
	nodesPerMove[rootMoves.getIndex()]+=value;
}

inline long SimplePVSearch::predictTimeUse(const long iterationTime[maxSearchPly], const long totalTime, const int depth) {

	double ratio1 = double(iterationTime[depth-1])/double(totalTime);
	double ratio2 = double(iterationTime[depth-2])/double(totalTime);
	double ratio3 = double(iterationTime[depth-3])/double(totalTime);

	double newRatio = ratio1*exp((ratio2/ratio1)*0.7 + (ratio3/ratio2)*0.3);

	double newTime = double(iterationTime[depth-1])*exp(newRatio);

	return long(newTime);
}

#endif /* SIMPLEPVSEARCH_H_ */

