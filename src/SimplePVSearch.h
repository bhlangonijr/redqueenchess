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

#include "SearchAgent.h"

namespace SimplePVSearchTypes {

	static const int maxScore = 400000;
	static const int maxQuiescenceSearchDepth = 10;
	static const int materialValues[ALL_PIECE_TYPE_BY_COLOR] = {100, 325, 325, 500, 975, 10000, 100, 325, 325, 500, 975, 10000, 0};

}

using namespace SimplePVSearchTypes;

class SimplePVSearch {

public:
	SimplePVSearch(Board& board);
	SimplePVSearch(Board& board, int depth ) : _depth(depth), _board(board) {}
	virtual ~SimplePVSearch();
	void search();
	int getScore();
private:
	Board& _board;
	int _depth;
	int _score;

	int idSearch(Board& board);
	int pvSearch(Board& board, int alpha, int beta, int depth);
	int qSearch(Board& board, int alpha, int beta, int depth);
	int evaluate(Board& board);

};

#endif /* SIMPLEPVSEARCH_H_ */
