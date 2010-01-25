/*
	Redqueen Chess Engine
    Copyright (C) 2008-2009 Ben-Hur Carlos Vieira Langoni Junior

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

#include "Uci.h"
#include "SearchAgent.h"
#include "StringUtil.h"
#include "Evaluator.h"

namespace SimplePVSearchTypes {

static const int maxScore = 20000;
static const int notLegal = -30000;
static const int maxQuiescenceSearchDepth = 10;
static const int maxSearchDepth = 40;
static const int maxSearchPly = 30;

}

using namespace SimplePVSearchTypes;

class SimplePVSearch {

public:

	typedef struct PvLine {
		int index;
		MoveIterator::Move moves[maxSearchDepth];
	} PvLine;

	typedef struct SearchStats {

		SearchStats():
			ttHits(0), ttLower(0), ttUpper(0), ttExact(0), nullMoveHits(0), pvChanges(0), searchTime(0), searchNodes(0), searchDepth(0) {}

		SearchStats(const SearchStats& stats):
			ttHits(stats.ttHits), ttLower(stats.ttLower),
			ttUpper(stats.ttUpper), ttExact(stats.ttExact),
			nullMoveHits(stats.nullMoveHits), pvChanges(stats.pvChanges),
			searchTime(stats.searchTime), searchNodes(stats.searchNodes), searchDepth(stats.searchDepth) {}
		long ttHits;
		long ttLower;
		long ttUpper;
		long ttExact;
		long nullMoveHits;
		long pvChanges;
		int searchTime;
		uint64_t searchNodes;
		int searchDepth;

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
		}

	} SearchStats;

	void operator()() {
		this->search();
	}

	SimplePVSearch(Board& board) :   _board(board), _depth(maxSearchDepth), _updateUci(true), errorCount(0), _startTime(0), _searchFixedDepth(true), _infinite(false) {}
	SimplePVSearch(Board& board, int depth ) : _board(board), _depth(depth), _updateUci(true), errorCount(0), _timeToSearch(0), _startTime(0), _searchFixedDepth(true), _infinite(false) {}
	SimplePVSearch(Board& board, int timeToSearch, int depth) : _board(board), _depth(depth), _updateUci(true), errorCount(0), _timeToSearch(timeToSearch), _startTime(0), _searchFixedDepth(false), _infinite(false) {}
	SimplePVSearch(Board& board, bool infinite ) : _board(board), _depth(maxSearchDepth), _updateUci(true), errorCount(0), _timeToSearch(0), _startTime(0), _searchFixedDepth(true), _infinite(true) {}

	virtual ~SimplePVSearch() {}
	virtual void search();
	virtual int getScore();

	inline const int getTickCount() {
		return ((clock() * 1000) / CLOCKS_PER_SEC);
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

	inline const void setTimeToSearch(const int timeToSearch) {
		_timeToSearch = timeToSearch;
	}

	inline const int getTimeToSearch() const {
		return _timeToSearch;
	}

	inline const void setStartTime(const int startTime) {
		_startTime = startTime;
	}

	inline const int getStartTime() const {
		return _startTime;
	}

private:
	Board& _board;
	int _depth;
	int _score;
	bool _updateUci;
	int errorCount;
	long _timeToSearch;
	long _startTime;
	long _nodes;
	long _time;
	bool _searchFixedDepth;
	bool _infinite;
	Evaluator evaluator;
	SearchStats stats;
	int timeToStop;
	bool moveFound;
	MoveIterator rootMoves;
	MoveIterator::Move killer[maxSearchDepth][2];
	int history[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE];

	int idSearch(Board& board);
	int sortMoves(Board& board, MoveIterator& moves, MoveIterator::Move ttMove, int alpha, int beta, int ply);
	int sortQMoves(Board& board, MoveIterator& moves);
	int pvSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv, const bool allowNullMove, const bool allowIid);
	int qSearch(Board& board, int alpha, int beta, int depth, PvLine* pv);
	const std::string pvLineToString(const PvLine* pv);
	void updatePv(PvLine* pv, PvLine& line, MoveIterator::Move& move);
	const bool stop(const bool searchInProgress);
	const bool timeIsUp();
	void clearHistory();
	void updateHistory(Board& board, MoveIterator::Move& move, int depth, int ply);

};

inline const bool SimplePVSearch::stop(const bool searchInProgress) {

	return !searchInProgress || timeIsUp();

}

inline const bool SimplePVSearch::timeIsUp() {

	static const uint64_t checkNodes=0x7d0;

	if ( _searchFixedDepth || _infinite || (_nodes & checkNodes)) {
		return false;
	}

	return (int)clock() >= timeToStop;

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

// sort search moves
inline int SimplePVSearch::sortMoves(Board& board, MoveIterator& moves, MoveIterator::Move ttMove, int alpha, int beta, int ply) {

	const int TT_MOVE_SCORE=1000;
	const int KILLER1_SCORE=200;
	const int KILLER2_SCORE=100;
	const int PROMO_CAPTURE_SCORE=700;
	const int PROMO_NON_CAPTURE_SCORE=650;
	const int GOOD_CAPTURE_SCORE=600;
	const int EQUAL_CAPTURE_SCORE=500;

	PvLine line;
	moves.first();
	int moveCounter=0;

	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();
		MoveBackup backup;
		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			move.score=notLegal;
			continue; // not legal
		}

		move.score = -evaluator.evalMaterial(board, board.getSideToMove());

		if (ttMove.from!=NONE && ttMove==move) {
			move.type = MoveIterator::TT_MOVE;
			move.score+=TT_MOVE_SCORE;
		} else if (move==killer[ply][0]) {
			move.type = MoveIterator::KILLER1;
			move.score+=KILLER1_SCORE;
		} else if (move==killer[ply][1]) {
			move.type = MoveIterator::KILLER2;
			move.score+=KILLER2_SCORE;
		}

		if (move.type==MoveIterator::PROMO_CAPTURE) {
			move.score+=PROMO_CAPTURE_SCORE;
		} else if (move.type==MoveIterator::PROMO_NONCAPTURE) {
			move.score+=PROMO_NON_CAPTURE_SCORE;
		} else if (move.type==MoveIterator::GOOD_CAPTURE) {
			move.score+=GOOD_CAPTURE_SCORE;
		} else if (move.type==MoveIterator::EQUAL_CAPTURE) {
			move.score+=EQUAL_CAPTURE_SCORE;
		} else if (move.type==MoveIterator::NON_CAPTURE) {
			move.score+=history[board.getPieceTypeBySquare(move.from)][move.to];
		}

		board.undoMove(backup);

		moveCounter++;
		moves.sortOne();

	}

	return moveCounter;

}

// sort moves of quiscence search
inline int SimplePVSearch::sortQMoves(Board& board, MoveIterator& moves) {

	moves.first();
	int moveCounter=0;

	while (moves.hasNext()) {

		MoveIterator::Move& move = moves.next();
		MoveBackup backup;
		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			move.score=notLegal;
			continue; // not legal
		}

		move.score = -evaluator.evalMaterial(board, board.getSideToMove());

		board.undoMove(backup);

		moveCounter++;
		moves.sortOne();
	}

	return moveCounter;

}

// clear history
inline void SimplePVSearch::clearHistory() {
	memset(history, 0, sizeof(int)*ALL_PIECE_TYPE_BY_COLOR*ALL_SQUARE);
	memset(killer, 0, sizeof(MoveIterator::Move)*maxSearchDepth*2);
}


// update history
inline void SimplePVSearch::updateHistory(Board& board, MoveIterator::Move& move, int depth, int ply) {

	if (move.type!=MoveIterator::NON_CAPTURE) {
		return;
	}

	if (move != killer[ply][0]) {
		killer[ply][1] = killer[ply][0];
		killer[ply][0] = move;
	}

	history[board.getPieceTypeBySquare(move.from)][move.to]+=depth;

}
#endif /* SIMPLEPVSEARCH_H_ */
