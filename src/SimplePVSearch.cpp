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

SimplePVSearch::SimplePVSearch(Board& board) :  _depth(1),  _board(board), _updateUci(true){

}

SimplePVSearch::~SimplePVSearch(){


}


// root search
void SimplePVSearch::search() {

	uint32_t tmp = getTickCount();
	_score = idSearch( _board );
	SearchAgent::getInstance()->setSearchInProgress(false);
	_time = getTickCount() - tmp;

}

// get current score
int SimplePVSearch::getScore() {
	return this->_score;
}

// iterative deepening
int SimplePVSearch::idSearch(Board& board) {

	int score = 0;
	MovePool movePool;
	Move* firstMove = board.generateAllMoves(movePool, board.getSideToMove());
	Move* move = firstMove;

	_nodes = 0;

	for (int depth = 1; depth <= _depth; depth++) {

		uint32_t time = getTickCount();
		uint32_t totalTime = 0;
		score = 0;
		move = firstMove;
		while (move) {
			_nodes++;
			MoveBackup backup;
			board.doMove(*move,backup);

			score = -pvSearch(board, -maxScore, maxScore, depth-1);
			move->score=score;

			//std::cout << "depth: " << depth << "score: " << score << " - move: " << move->toString() << " - nodes: " << _nodes << " - time: " << (getTickCount() - time) << std::endl;

			board.undoMove(backup);
			move = move->next;
		}
		time = getTickCount()-time;
		totalTime += time;

		if (isUpdateUci()) {
			std::cout << "info depth "<< depth << std::endl;
			std::cout << "info score cp " << score << " depth " << depth << " nodes " << _nodes << " time " << time << " pv " << std::endl/*TODO pv*/;
			if (totalTime>1000) {
				std::cout << "nps " << (_nodes/(totalTime/1000)) << std::endl;
			} else {
				std::cout << "nps " << _nodes << std::endl;
			}

		}

		// TODO sort moves

	}

	movePool.~object_pool();
	if (firstMove) {
		score = firstMove->score;
		if (isUpdateUci()) {
			std::cout << "bestmove " << firstMove->toString() << std::endl;
		}
	}

	return score;
}

// principal variation search
int SimplePVSearch::pvSearch(Board& board, int alpha, int beta, int depth) {

	bool bSearch = true;
	int score = 0;

	if( depth == 0 ) {
		score = qSearch(board, alpha, beta, maxQuiescenceSearchDepth);
		SearchAgent::getInstance()->hashPut(board,score,depth,board.getMoveCounter());
		return score;
	}


	HashData hashData;
	if (SearchAgent::getInstance()->hashGet(board.getKey(), hashData)) {
		if (hashData.depth>=depth) {
			return hashData.value;
		}
	}

	{
		_nodes++;
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
			move = move->next;

			if( score >= beta ) {
				SearchAgent::getInstance()->hashPut(board,score,depth,board.getMoveCounter());
				movePool.~object_pool();
				return beta;
			}

			if( score > alpha ) {
				alpha = score;
				bSearch = false;
			}
		}

		movePool.~object_pool();
	}

	SearchAgent::getInstance()->hashPut(board,score,depth,board.getMoveCounter());

	return alpha;

}

//quiescence search
int SimplePVSearch::qSearch(Board& board, int alpha, int beta, int depth) {

	if (depth == 0) return evaluate(board);

	int standPat = evaluate(board);

	if( standPat >= beta ) {
		return beta;
	}

	if( alpha < standPat ) {
		alpha = standPat;
	}
	{
		_nodes++;
		MovePool movePool;
		Move* move = board.generateCaptures(movePool, board.getSideToMove());

		while ( move )  {

			MoveBackup backup;
			board.doMove(*move,backup);

			int score = -qSearch(board, -beta, -alpha, depth - 1 );

			board.undoMove(backup);
			move = move->next;

			if( score >= beta ) {
				movePool.~object_pool();
				return beta;
			}

			if( score > alpha ) {
				alpha = score;
			}

		}
		movePool.~object_pool();
	}

	return alpha;
}

// simplest eval function
int SimplePVSearch::evaluate(Board& board) {

	int result = 0;
	PieceColor side = board.getSideToMove();

	int whiteMaterial = 0;
	int blackMaterial = 0;

	for(int pieceType = WHITE_PAWN; pieceType <= WHITE_KING; pieceType++) {
		whiteMaterial += board.getPieceCountByType(PieceTypeByColor(pieceType)) * materialValues[pieceType];
	}

	for(int pieceType = BLACK_PAWN; pieceType <= BLACK_KING; pieceType++) {
		blackMaterial += board.getPieceCountByType(PieceTypeByColor(pieceType)) * materialValues[pieceType];
	}

	result = side==WHITE?whiteMaterial-blackMaterial : blackMaterial-whiteMaterial;

	return result;
}













