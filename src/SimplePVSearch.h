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
static const int maxQuiescenceSearchDepth = 10;
static const int materialValues[ALL_PIECE_TYPE_BY_COLOR] = {100, 325, 325, 500, 975, 10000, 100, 325, 325, 500, 975, 10000, 0};
static const int maxSearchDepth = 40;

}

using namespace SimplePVSearchTypes;

class SimplePVSearch {

public:

	void operator()() {
		this->search();
	}

	SimplePVSearch(Board& board) :  _depth(maxSearchDepth),  _board(board), _updateUci(true), errorCount(0), _startTime(0), _searchFixedDepth(true), _infinite(false) {}
	SimplePVSearch(Board& board, int depth ) : _depth(depth), _board(board), _updateUci(true), errorCount(0), _timeToSearch(0), _startTime(0), _searchFixedDepth(true), _infinite(false) {}
	SimplePVSearch(Board& board, uint32_t timeToSearch ) : _depth(maxSearchDepth), _board(board), _updateUci(true), _timeToSearch(timeToSearch), errorCount(0), _startTime(0), _searchFixedDepth(false), _infinite(false) {}
	SimplePVSearch(Board& board, bool infinite ) : _depth(maxSearchDepth), _board(board), _updateUci(true), errorCount(0), _timeToSearch(0), _startTime(0), _searchFixedDepth(true), _infinite(true) {}

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

	const void setDepth(const int depth) {
		_depth = depth;
	}

	const int getDepth() const {
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

	void sort(std::vector<Move*>& moves);
	Move* sortMoves(MovePool& movePool, Move* firstMove);

	void addPv(const Move move) {
		pv.push_back(move);
	}

	Move getPvAt(const int index) {

		Move move;

		if (index >= pv.size()) {
			return move;
		}
		return pv[index];
	}

	std::vector<Move>& getPv() {
		return pv;
	}

	void clearPv() {
		pv.clear();
	}

	void updatePv(const int index, const Move value) {
		Move move;
		if (index >= pv.size()) {
			return ;
		}
		pv[index]=value;

	}

	std::string getPvString(const int depth) {
		if (depth < 1) {
			return "";
		}

		std::string result="";
		for(int x=0;x<pv.size();x++) {
			result+=pv[x].toString()+" ";
		}
		return result;
	}

private:
	Board& _board;
	int _depth;
	int _score;
	uint32_t _timeToSearch;
	uint32_t _startTime;
	uint64_t _nodes;
	uint32_t _time;
	bool _infinite;
	bool _updateUci;
	bool _searchFixedDepth;
	int errorCount;
	std::vector<Move> pv;

	int idSearch(Board& board);
	int pvSearch(Board& board, int alpha, int beta, int depth, int maxDepth, Move* pvNode, MovePool& pvPool);
	int qSearch(Board& board, int alpha, int beta, int depth, int maxDepth);
	int evaluate(Board& board);
	void updatePv(Move* move, int depth, int maxDepth);

	const bool stop();
	const bool timeIsUp();

};




#endif /* SIMPLEPVSEARCH_H_ */
