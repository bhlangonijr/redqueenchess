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

}

using namespace SimplePVSearchTypes;

class SimplePVSearch {

public:

	SimplePVSearch(Board& board);
	SimplePVSearch(Board& board, int depth ) : _depth(depth), _board(board), _updateUci(true), errorCount(0) {}
	virtual ~SimplePVSearch();
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

private:
	Board& _board;
	int _depth;
	int _score;
	uint64_t _nodes;
	uint32_t _time;
	bool _updateUci;
	int errorCount;

	int idSearch(Board& board);
	int pvSearch(Board& board, int alpha, int beta, int depth);
	int qSearch(Board& board, int alpha, int beta, int depth);
	int evaluate(Board& board);

};

#endif /* SIMPLEPVSEARCH_H_ */
