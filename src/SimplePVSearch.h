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

namespace SimplePVSearchTypes {

static const int maxScore = 200000;
static const uint32_t maxQuiescenceSearchDepth = 20;
static const int materialValues[ALL_PIECE_TYPE_BY_COLOR] = {100, 325, 325, 500, 975, 10000, 100, 325, 325, 500, 975, 10000, 0};
static const uint32_t maxSearchDepth = 40;

}

using namespace SimplePVSearchTypes;

class SimplePVSearch {

public:

	typedef struct PvLine {
	    int index;
	    MoveIterator::Move moves[maxSearchDepth];
	}   PvLine;

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

	const uint32_t getTickCount() {
		return ((clock() * 1000) / CLOCKS_PER_SEC);
	}

	const bool isUpdateUci() const {
		return _updateUci;
	}

	const void setUpdateUci(const bool value) {
		_updateUci = value;
	}

	const bool isSearchFixedDepth() const {
		return _searchFixedDepth;
	}

	const void setSearchFixedDepth(const bool value) {
		_searchFixedDepth = value;
	}

	const bool isInfinite() const {
		return _infinite;
	}

	const void setInfinite(const bool value) {
		_infinite = value;
	}

	const void setDepth(const uint32_t depth) {
		_depth = depth;
	}

	const uint32_t getDepth() const {
		return _depth;
	}

	const void setTimeToSearch(const uint32_t timeToSearch) {
		_timeToSearch = timeToSearch;
	}

	const uint32_t getTimeToSearch() const {
		return _timeToSearch;
	}

	const void setStartTime(const uint32_t startTime) {
		_startTime = startTime;
	}

	const uint32_t getStartTime() const {
		return _startTime;
	}

	void addPv(const MoveIterator::Move move) {
		pv.push_back(move);
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

	std::vector<MoveIterator::Move> pv;

	int idSearch(Board& board);
	int pvSearch(Board& board, int alpha, int beta, uint32_t depth, PvLine* pv);
	int qSearch(Board& board, int alpha, int beta, uint32_t depth, PvLine* pv);
	int evaluate(Board& board);
	const std::string pvLineToString(const PvLine* pv);
	void updatePv(PvLine* pv, PvLine& line, MoveIterator::Move& move);

	const bool stop();
	const bool timeIsUp();

};




#endif /* SIMPLEPVSEARCH_H_ */
