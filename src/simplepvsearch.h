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
 * SimplePVSearch.h
 *
 *  Created on: 18/05/2009
 *      Author: bhlangonijr
 */

#ifndef SIMPLEPVSEARCH_H_
#define SIMPLEPVSEARCH_H_

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "searchagent.h"
#include "evaluator.h"
#include "threadpool.h"
#include "stringutil.h"
// game constants
const int extUnit = 2;
const int maxScoreRepetition = 25;
const int mateRangeScore = 300;
const int maxSearchDepth = 80;
const int maxSearchPly = 100;
const int maxMoveCount = 64;
const int minSplitDepth	= 3;
// internal iterative deepening
const int allowIIDAtPV = 5;
const int allowIIDAtNormal = 8;
// margin constants
const int iidMargin=160;
const int easyMargin=400;
const int deltaMargin=950;
const int futilityQSMargin=130;
const int nullMoveMargin=512;
const int seMargin=35;
//depth prunning threshold
const int aspirationDepth=6;
const int futilityDepth=6;
const int moveCountDepth=14;
const int nullMoveDepth=2;
const int razorDepth=4;
const int qsOnlyRecaptureDepth=-6;
const int lmrDepthThresholdRoot=3;
const int lmrDepthThreshold=2;
const int sePVDepth=7;
const int seNonPVDepth=9;
const int lateMoveThreshold=1;
const int scoreTable[11]={0,80000,60000,95000,90000,45000,40000,1000,-12000,50050,50000};
const int64_t defaultNodesToGo=0xFFF;
const int64_t fastNodesToGo=0xFF;
namespace SearchTypes {
enum NodeType {
	PV_NODE, ALL_NODE, CUT_NODE, NODE_NONE
};

typedef struct SearchInfo {
	SearchInfo(): allowNullMove(false), move(MoveIterator::Move()), partialSearch(false),
			alpha(0), beta(0), depth(0), ply(0), nodeType(NODE_NONE), splitPoint(NULL), extension(0) {}
	SearchInfo(const bool _allowNullMove, const MoveIterator::Move _move, const int _alpha, const int _beta,
			const int _depth, const int _ply, const NodeType _nodeType, SplitPoint* sp):
				allowNullMove(_allowNullMove), move(_move), partialSearch(false), alpha(_alpha), beta(_beta),
				depth(_depth), ply(_ply), nodeType(_nodeType), splitPoint(sp), extension(0) {}
	SearchInfo(const bool _allowNullMove, const MoveIterator::Move _move, const bool _partialSearch, const int _alpha,
			const int _beta, const int _depth, const int _ply, const NodeType _nodeType, SplitPoint* sp):
				allowNullMove(_allowNullMove), move(_move), partialSearch(_partialSearch), alpha(_alpha),
				beta(_beta), depth(_depth), ply(_ply), nodeType(_nodeType), splitPoint(sp), extension(0) {}

	inline void update(const int _depth, const NodeType _nodeType) {
		depth=_depth;
		nodeType=_nodeType;
	}
	inline void update(const int _depth, const NodeType _nodeType, const int _alpha, const int _beta,
			MoveIterator::Move& _move) {
		depth=_depth;
		nodeType=_nodeType;
		alpha=_alpha;
		beta=_beta;
		move=_move;
	}
	bool allowNullMove;
	MoveIterator::Move move;
	bool partialSearch;
	int alpha;
	int beta;
	int depth;
	int ply;
	NodeType nodeType;
	SplitPoint* splitPoint;
	int extension;
} SearchInfo;
}

using namespace SearchTypes;

class SearchAgent;

class MoveIterator;

class SimplePVSearch {

public:

	typedef struct PvLine {
		int index;
		MoveIterator::Move moves[maxSearchPly+1];
		void copy(const PvLine line) {
			index=line.index;
			memcpy(moves, line.moves, (maxSearchPly+1) * sizeof(MoveIterator::Move));
		}
	} PvLine;

	SimplePVSearch(int* _history) : depthToSearch(maxSearchDepth), updateUci(true), timeToSearch(0), startTime(0), searchFixedDepth(false),
			infinite(false), nodes(0), maxPlySearched(0), nodesToGo(defaultNodesToGo), history(_history) {
		initMutex();
	}

	~SimplePVSearch() {
		destoryMutex();
	}

	void search(Board _board);
	int pvSearch(Board& board,  SearchInfo& si);
	int zwSearch(Board& board,  SearchInfo& si);
	int qSearch(Board& board,  SearchInfo& si);
	void smpPVSearch(Board board, SimplePVSearch* master,
			SimplePVSearch* ss, SplitPoint* sp);
	int64_t perft(Board& board, int depth, int ply);
	int getScore();
	void clearKillers();
	template <bool quiescenceMoves>
	MoveIterator::Move& selectMove(Board& board, MoveIterator& moves,
			MoveIterator::Move& ttMove, int ply, int depth);
	bool isMateScore(const int score);
	bool isPawnPush(Board& board, Square& square);
	const bool stop(SearchInfo& info);
	const bool timeIsUp();

	void updateKillers(Board& board, MoveIterator::Move& move, int ply);
	static void initialize();

	inline const bool isUpdateUci() const {
		return updateUci;
	}

	inline const void setUpdateUci(const bool value) {
		updateUci = value;
	}

	inline const bool isSearchFixedDepth() const {
		return searchFixedDepth;
	}

	inline const void setSearchFixedDepth(const bool value) {
		searchFixedDepth = value;
	}

	inline const bool isInfinite() const {
		return infinite;
	}

	inline const void setInfinite(const bool value) {
		infinite = value;
	}

	inline const void setDepth(const int depth) {
		depthToSearch = depth;
	}

	inline const int getDepth() const {
		return depthToSearch;
	}

	const int getReduction(const bool isPV, const int depth, const int moveCounter) const;

	const int getFutilityMargin(const int depth, const int moveCounter) const;

	const int getFutilityMargin() const;

	const int getMoveCountMargin(const int depth) const;

	const int getRazorMargin(const int depth);

	const int getDeltaMargin(const int depth);
	bool isCompletedIteration() const
	{
		return completedIteration;
	}

	void setCompletedIteration(bool completedIteration)
	{
		this->completedIteration = completedIteration;
	}

	inline const void setTimeToSearch(const int64_t value) {
		timeToSearch = value;
	}

	inline const int64_t getTimeToSearch() const {
		return timeToSearch;
	}

	inline const void setStartTime(const int64_t value) {
		startTime = value;
	}

	inline const int64_t getStartTime() const {
		return startTime;
	}

	inline const void setTimeToStop() {
		timeToStop = getTickCount() + timeToSearch;
	}

	inline const void setTimeToStop(const uint64_t _timeToStop) {
		timeToStop = _timeToStop;
	}

	inline const int64_t getTimeToStop() const {
		return timeToStop;
	}

	inline const int64_t getSearchedTime() const {
		return time;
	}

	inline const int64_t getSearchedNodes() const {
		return nodes;
	}

	inline void updateSearchedNodes(const int64_t _nodes) {
		nodes += _nodes;
	}

	inline const int getThreadId() const {
		return threadId;
	}

	inline void setThreadId(const int _threadId) {
		threadId = _threadId;
	}

	inline const int getThreadGroup() const {
		return threadGroup;
	}

	inline void setThreadGroup(const int _threadGroup) {
		threadGroup = _threadGroup;
	}

	inline Evaluator& getEvaluator() {
		return evaluator;
	}

	inline void setSearchAgent(SearchAgent* _agent) {
		this->agent=_agent;
	}
	// copy killers arrays
	inline void copyKillers(MoveIterator::Move* _killer) {
		if (_killer!=NULL) {
			memcpy(&killer[0][0],_killer,sizeof(MoveIterator::Move)*maxSearchPly*2);
		}
	}

	inline MoveIterator::Move* getKillerArray() {
		return &killer[0][0];
	}

	inline MoveIterator::Move getKiller(int depth, int type) {
		return killer[depth][type];
	}

	inline void cleanUp() {
		evaluator.cleanPawnInfo();
		clearKillers();
	}

	inline void prepareToSearch() {
		resetStats();
		clearKillers();
		initRootMovesOrder();
		rootMoves.clear();
	}

	inline void resetStats() {
		searchScore=0;
		startTime=0;
		time=0;
		timeToStop=0;
		nodes = 0;
	}

	inline void initMutex() {
		pthread_mutex_init(&mutex,NULL);
		pthread_cond_init(&waitCond,NULL);
	}

	inline void destoryMutex() {
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&waitCond);
	}

	inline void wakeUp() {
		pthread_cond_signal(&waitCond);
	}

	inline void lock() {
		pthread_mutex_lock(&mutex);
	}

	inline void unlock() {
		pthread_mutex_unlock(&mutex);
	}

	inline void waitThread() {
		pthread_cond_wait(&waitCond, &mutex);
	}

private:
	int depthToSearch;
	int searchScore;
	bool updateUci;
	int64_t timeToSearch;
	int64_t startTime;
	int64_t time;
	bool searchFixedDepth;
	bool infinite;
	int64_t nodes;
	int64_t timeToStop;
	MoveIterator rootMoves;
	MoveIterator::Move killer[maxSearchPly+1][2];
	int iterationPVChange[maxSearchPly+1];
	int64_t nodesPerMove[MOVE_LIST_MAX_SIZE];
	Evaluator evaluator;
	SearchAgent* agent;
	MoveIterator::Move emptyMove;
	SearchInfo rootSearchInfo;
	int maxPlySearched;
	int64_t nodesToGo;
	int threadId;
	int threadGroup;
	pthread_mutex_t mutex;
	pthread_cond_t waitCond;
	int* history;
	bool completedIteration;
	int idSearch(Board& board);
	int rootSearch(Board& board, SearchInfo& si, PvLine& pv);
	void uciOutput(PvLine& pv, const int score, const int64_t totalTime,
			const int hashFull, const int depth, const int selDepth, const int alpha, const int beta);
	void uciOutput(MoveIterator::Move& bestMove, MoveIterator::Move& ponderMove);
	void uciOutput(MoveIterator::Move& move, const int moveCounter);
	void uciOutput(const int depth);
	const std::string pvLineToString(const PvLine& pv);
	template <bool isEvasion>
	void scoreMoves(Board& board, MoveIterator& moves);
	void scoreRootMoves(Board& board, MoveIterator& moves);
	void retrievePvFromHash(Board& board, PvLine& pv);
	void updateHashWithPv(Board& board, PvLine& pv);
	void filterLegalMoves(Board& board, MoveIterator& moves,
			std::string searchMoves);
	void initRootMovesOrder();
	void updateRootMovesScore(const int64_t value);
};

// select a move
template <bool quiescenceMoves>
inline MoveIterator::Move& SimplePVSearch::selectMove(Board& board, MoveIterator& moves,
		MoveIterator::Move& ttMove, int ply, int depth) {
	while (true) {
		switch (moves.getStage()) {
		case MoveIterator::END_STAGE:
			return emptyMove;
		case MoveIterator::BEGIN_STAGE:
			if (!board.isInCheck()) {
				moves.setStage(MoveIterator::INIT_CAPTURE_STAGE);
			} else {
				moves.setStage(MoveIterator::INIT_EVASION_STAGE);
			}
			if (!ttMove.none() && board.isMoveLegal<true>(ttMove)) {
				return ttMove;
			}
			break;
		case MoveIterator::INIT_EVASION_STAGE:
			board.generateEvasions(moves, board.getSideToMove());
			scoreMoves<true>(board, moves);
			moves.setStage(MoveIterator::ON_EVASION_STAGE);
			break;
		case MoveIterator::ON_EVASION_STAGE:
			while (moves.hasNext()) {
				MoveIterator::Move& move=moves.selectBest();
				if (move==ttMove || !board.isMoveLegal<false>(move)) {
					continue;
				}
				return move;
			}
			moves.setStage(MoveIterator::END_STAGE);
			return emptyMove;
		case MoveIterator::INIT_CAPTURE_STAGE:
			board.generateCaptures(moves, board.getSideToMove());
			if (quiescenceMoves) {
				board.generatePromotion(moves, board.getSideToMove(),board.getEmptySquares());
				if (depth==0) {
					board.generateNonCaptureChecks(moves, board.getSideToMove());
				}
			}
			scoreMoves<false>(board, moves);
			moves.setStage(MoveIterator::ON_CAPTURE_STAGE);
			break;
		case MoveIterator::ON_CAPTURE_STAGE:
			while (moves.hasNext()) {
				MoveIterator::Move& move=moves.selectBest();
				if (move==ttMove || !board.isMoveLegal<false>(move)) {
					continue;
				}
				if (move.type==MoveIterator::UNKNOW_CAPTURE) {
					move.score=evaluator.see<true>(board,move);
					if (move.score >= 0) {
						move.type=MoveIterator::GOOD_CAPTURE;
						move.score+=scoreTable[move.type];
						return move;
					} else {
						move.type=MoveIterator::BAD_CAPTURE;
						move.score+=scoreTable[move.type];
						moves.prior();
						continue;
					}
				}
				if (!quiescenceMoves) {
					if (move.type==MoveIterator::BAD_CAPTURE) {
						moves.prior();
						break;
					}
				}
				return move;
			}
			if (quiescenceMoves) {
				moves.setStage(MoveIterator::END_STAGE);
				return emptyMove;
			} else {
				moves.setStage(MoveIterator::KILLER1_STAGE);
			}
			break;
		case MoveIterator::KILLER1_STAGE:
			moves.setStage(MoveIterator::KILLER2_STAGE);
			if (killer[ply][0] != ttMove &&
					!board.isCaptureMove(killer[ply][0]) &&
					board.isMoveLegal<true>(killer[ply][0])) {
				return killer[ply][0];
			}
			break;
		case MoveIterator::KILLER2_STAGE:
			moves.setStage(MoveIterator::INIT_QUIET_STAGE);
			if (killer[ply][1] != ttMove &&
					!board.isCaptureMove(killer[ply][1]) &&
					board.isMoveLegal<true>(killer[ply][1])) {
				return killer[ply][1];
			}
			break;
		case MoveIterator::INIT_QUIET_STAGE:
			board.generateNonCaptures(moves, board.getSideToMove());
			scoreMoves<false>(board, moves);
			moves.setStage(MoveIterator::ON_QUIET_STAGE);
			break;
		case MoveIterator::ON_QUIET_STAGE:
			while (moves.hasNext()) {
				MoveIterator::Move& move=moves.selectBest();
				if (move==ttMove || !board.isMoveLegal<false>(move) ||
						((move==killer[ply][0] || move==killer[ply][1]) &&
								!board.isCaptureMove(move))) {
					continue;
				}
				return move;
			}
			moves.setStage(MoveIterator::END_STAGE);
			break;
		default:
			return emptyMove;
		}

	}
	return emptyMove;
}

// score moves
template <bool isEvasion>
inline void SimplePVSearch::scoreMoves(Board& board, MoveIterator& moves) {
	moves.bookmark();
	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();
		const PieceTypeByColor pieceFrom = board.getPiece(move.from);
		const PieceTypeByColor pieceTo = board.getPiece(move.to);
		if (move.type==MoveIterator::UNKNOW) {
			move.score=-maxScore;
			if (isEvasion) {
				int seeValue = evaluator.see<true>(board,move);
				if (seeValue<0) {
					move.score = seeValue-10000;
				}
			}
			bool isCapture=false;
			if (pieceTo!=EMPTY) {
				if (move.score==-maxScore) {
					move.score=(materialValues[pieceTo]-materialValues[pieceFrom]);
				}
				isCapture=true;
			} else if (board.getPieceType(pieceFrom)==PAWN){
				if (board.getEnPassant()!=NONE &&
						board.getSquareFile(move.from)!=board.getSquareFile(move.to)) {
					if (move.score==-maxScore) {
						move.score=0;
					}
					isCapture=true;
				}
			}
			if (isCapture) {
				move.type = (move.score >= 0 ?
						MoveIterator::GOOD_CAPTURE : MoveIterator::UNKNOW_CAPTURE);
			} else {
				move.type=MoveIterator::NON_CAPTURE;
				if (move.score==-maxScore) {
					const int hist = history[pieceFrom*ALL_SQUARE+move.to];
					const GamePhase phase = board.getGamePhase();
					const int gain =
							board.getPieceSquareValue(pieceFrom,move.to,phase)-
							board.getPieceSquareValue(pieceFrom,move.from,phase);
					move.score=hist+gain;
				}
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
		const bool isCapture = board.isCaptureMove(move);
		const int value = isCapture ? evaluator.see<false>(board,move) : 0;
		board.doMove(move,backup);
		board.setInCheck(board.getSideToMove());
		SearchInfo newSi(false,move,true,-maxScore,maxScore,0,0,PV_NODE,NULL);
		move.score = -qSearch(board,newSi);
		if (move.type==MoveIterator::UNKNOW) {
			if (isCapture) {
				move.type = (value >= 0 ?
						MoveIterator::GOOD_CAPTURE : MoveIterator::BAD_CAPTURE);
			} else {
				move.type=MoveIterator::NON_CAPTURE;

			}
		}
		board.undoMove(backup);
	}
	moves.goToBookmark();
}

// filter the move list to only legal moves
inline void SimplePVSearch::filterLegalMoves(Board& board, MoveIterator& moves,
		std::string searchMoves) {
	MoveIterator newMoves;
	moves.first();
	const bool isSearchMoves=(searchMoves!="none");
	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();
		MoveBackup backup;
		board.doMove(move,backup);
		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue;
		}
		if (!(isSearchMoves &&
				!StringUtil::containsString(searchMoves, move.toString()))) {
			newMoves.add(move);
		}
		board.undoMove(backup);
	}
	moves.clear();
	moves.addAll(newMoves);
	moves.first();
}

// is mate score?
inline bool SimplePVSearch::isMateScore(const int score) {
	return score < -maxScore+maxSearchPly || score > maxScore-maxSearchPly;
}

// pawn push
inline bool SimplePVSearch::isPawnPush(Board& board, Square& square) {
	if (board.getPieceType(square)!=PAWN) {
		return false;
	}
	return evaluator.isPawnPassed(board,square);
}

// time is up?
inline const bool SimplePVSearch::timeIsUp() {
	if (searchFixedDepth ||
			infinite || (nodes & nodesToGo)) {
		return false;
	}
	return getTickCount()>=getTimeToStop();
}

inline void SimplePVSearch::uciOutput(PvLine& pv, const int score, const int64_t totalTime,
		const int hashFull, const int depth, const int selDepth, const int alpha, const int beta) {
	MoveIterator::Move& move = pv.moves[0];
	if (isUpdateUci() && !move.none()) {
		std::string scoreString = "cp " + StringUtil::toStr(score);
		if (abs(score) > (maxScore-mateRangeScore)) {
			if (score>0) {
				scoreString = "mate " +StringUtil::toStr((maxScore - (score+1))/2);
			} else {
				scoreString = "mate " +StringUtil::toStr(-(maxScore + score)/2);
			}
		}
		if (score >= beta) {
			scoreString += " lowerbound";
		} else if (score <= alpha) {
			scoreString += " upperbound";
		}
		int64_t nps = totalTime>1000 ?  ((nodes)/(totalTime/1000)) : nodes;
		std::cout << "info depth "<< depth << " seldepth " << selDepth << std::endl;
		std::cout << "info depth "<< depth << " score " << scoreString << " time " << totalTime
				<< " nodes " << (nodes) << " nps " << nps << " pv" << pvLineToString(pv) << std::endl;
		std::cout << "info nodes " << (nodes) << " time " << totalTime << " nps " << nps
				<< " hashfull " << hashFull << std::endl;
	}
}

inline void SimplePVSearch::uciOutput(MoveIterator::Move& bestMove, MoveIterator::Move& ponderMove) {
	if (isUpdateUci()) {
		if (!bestMove.none()) {
			std::cout << "bestmove " << bestMove.toString();
			if (!ponderMove.none()) {
				std::cout << " ponder "<< ponderMove.toString();
			}
			std::cout << std::endl;

		} else {
			std::cout << "bestmove (none)" << std::endl;
		}
	}
}

inline void SimplePVSearch::uciOutput(MoveIterator::Move& move, const int moveCounter) {
	const int64_t uciOutputSecs=1500;
	if (isUpdateUci()) {
		if (startTime+uciOutputSecs < getTickCount()) {
			std::cout << "info currmove " << move.toString() << " currmovenumber " << moveCounter << std::endl;
		}
	}
}

inline void SimplePVSearch::uciOutput(const int depth) {
	const int64_t uciOutputSecs=1500;
	if (isUpdateUci()) {
		if (startTime+uciOutputSecs < getTickCount()) {
			std::cout << "info depth "<< depth << " seldepth " << maxPlySearched << " nodes " << nodes << std::endl;
		}
	}
}

inline const std::string SimplePVSearch::pvLineToString(const PvLine& pv) {
	std::string result="";
	for(int x=0;x<pv.index;x++) {
		result += " ";
		result += pv.moves[x].toString();
	}
	return result;
}

// clear killers
inline void SimplePVSearch::clearKillers() {
	memset(killer, 0, sizeof(MoveIterator::Move)*maxSearchPly*2);
}

// update killers
inline void SimplePVSearch::updateKillers(Board& board, MoveIterator::Move& move, int ply) {
	if (board.isCaptureOrPromotion(move) || move.none()) {
		return;
	}
	if (move != killer[ply][0]) {
		killer[ply][1] = killer[ply][0];
		killer[ply][0] = move;
		killer[ply][0].type = MoveIterator::KILLER1;
		killer[ply][1].type = MoveIterator::KILLER2;
	}
}

// init root moves ordering array
inline void SimplePVSearch::initRootMovesOrder() {
	memset(nodesPerMove, 0, sizeof(int64_t)*MOVE_LIST_MAX_SIZE);
}
// update root moves score
inline void SimplePVSearch::updateRootMovesScore(const int64_t value) {
	nodesPerMove[rootMoves.getIndex()]+=value;
}
#endif /* SIMPLEPVSEARCH_H_ */
