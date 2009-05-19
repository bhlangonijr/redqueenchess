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
 * SimplePVSearch.cpp
 *
 *  Created on: 18/05/2009
 *      Author: bhlangonijr
 */

#include "SimplePVSearch.h"

using namespace SimplePVSearchTypes;

SimplePVSearch::SimplePVSearch() :  _depth(1),  _board(board) {

}

SimplePVSearch::~SimplePVSearch() {

}

// root search
void SimplePVSearch::search() {

	_score = idSearch( _board );

}

// get current score
int SimplePVSearch::getScore() {
	return this->_score;
}

// iterative deepening
int SimplePVSearch::idSearch(Board& board) {

	int score = 0;

	for (int depth = 1; depth <= _depth; depth++) {
		score = pvSearch(_board, -maxScore, maxScore, depth);
	}


}

// principal variation search
int SimplePVSearch::pvSearch(Board& board, int alpha, int beta, int depth) {

	if( depth == 0 ) return qSearch(board, alpha, beta, depth);

	bool bSearch = true;

	MovePool movePool;
	Move* move = board.generateAllMoves(movePool, board.getSideToMove());

	while ( move )  {

		MoveBackup backup;
		board.doMove(*move,backup);

		if ( bSearch ) {
			score = -pvSearch(board, -beta, -alpha, depth - 1);
		} else {
			score = -pvSearch(board, -alpha-1, -alpha, depth - 1);
			if ( score > alpha ) {
				score = -pvSearch(board, -beta, -alpha, depth - 1);
			}
		}

		board.undoMove(backup);

		if( score >= beta ) {
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			bSearch = false;
		}
	}
	return alpha;

}

//quiescence search
int SimplePVSearch::qSearch(Board& board, int alpha, int beta, int depth) {

	if (depth == 0) Evaluate();

	int standPat = Evaluate();

	if( standPat >= beta ) {
		return beta;
	}

	if( alpha < standPat ) {
		alpha = standPat;
	}

	MovePool movePool;
	Move* move = board.generateCaptures(movePool, board.getSideToMove());

	while ( move )  {

		MoveBackup backup;
		board.doMove(*move,backup);

		score = -qSearch(board, -beta, -alpha, depth - 1 );

		board.undoMove(backup);

		if( score >= beta ) {
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
		}

	}

	return alpha;

}














