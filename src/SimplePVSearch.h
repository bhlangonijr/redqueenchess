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

static const int maxScore = 200000;
static const uint32_t maxQuiescenceSearchDepth = 10;
static const uint32_t maxSearchDepth = 40;
static const uint32_t maxSearchPly = 30;

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
		uint32_t searchTime;
		uint64_t searchNodes;
		uint32_t searchDepth;

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
	SimplePVSearch(Board& board, uint32_t timeToSearch ) : _board(board), _depth(maxSearchDepth), _updateUci(true), errorCount(0), _timeToSearch(timeToSearch), _startTime(0), _searchFixedDepth(false), _infinite(false) {}
	SimplePVSearch(Board& board, bool infinite ) : _board(board), _depth(maxSearchDepth), _updateUci(true), errorCount(0), _timeToSearch(0), _startTime(0), _searchFixedDepth(true), _infinite(true) {}

	virtual ~SimplePVSearch() {}
	virtual void search();
	virtual int getScore();

	inline const uint32_t getTickCount() {
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

	inline const void setDepth(const uint32_t depth) {
		_depth = depth;
	}

	inline const uint32_t getDepth() const {
		return _depth;
	}

	inline const void setTimeToSearch(const uint32_t timeToSearch) {
		_timeToSearch = timeToSearch;
	}

	inline const uint32_t getTimeToSearch() const {
		return _timeToSearch;
	}

	inline const void setStartTime(const uint32_t startTime) {
		_startTime = startTime;
	}

	inline const uint32_t getStartTime() const {
		return _startTime;
	}

private:
	Board& _board;
	uint32_t _depth;
	int _score;
	bool _updateUci;
	int errorCount;
	uint32_t _timeToSearch;
	uint32_t _startTime;
	uint64_t _nodes;
	uint32_t _time;
	bool _searchFixedDepth;
	bool _infinite;
	Evaluator evaluator;
	SearchStats stats;
	MoveIterator moves;

	int idSearch(Board& board);
	int pvSearch(Board& board, int alpha, int beta, uint32_t depth, uint32_t ply, PvLine* pv, const bool allowNullMove);
	int qSearch(Board& board, int alpha, int beta, uint32_t depth, PvLine* pv);
	const std::string pvLineToString(const PvLine* pv);
	void updatePv(PvLine* pv, PvLine& line, MoveIterator::Move& move);
	const bool stop(const bool searchInProgress);
	const bool timeIsUp();

};

inline const bool SimplePVSearch::stop(const bool searchInProgress) {

	return !searchInProgress || timeIsUp();

}

inline const bool SimplePVSearch::timeIsUp() {

	static const uint64_t checkNodes=4000;

	if ( _searchFixedDepth || _infinite || _nodes % checkNodes != 0) {
		return false;
	}

	return (getTickCount()-_startTime)>=_timeToSearch;

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



#endif /* SIMPLEPVSEARCH_H_ */
