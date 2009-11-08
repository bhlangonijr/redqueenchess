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
// If set to true, will check move integrity - used for trace purposes
#define CHECK_MOVE_GEN_ERRORS false
// Exit the program if the count of errors in CHECK_MOVE_GEN_ERRORS exced the threshold - used for trace purposes
#define EXIT_PROGRAM_THRESHOLD 100

using namespace SimplePVSearchTypes;

SimplePVSearch::SimplePVSearch(Board& board) :  _depth(1),  _board(board), _updateUci(true){

}

SimplePVSearch::~SimplePVSearch(){


}


// root search
void SimplePVSearch::search() {

	errorCount=0;
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
	int bestScore = 0;
	MovePool movePool;
	Move* firstMove = board.generateAllMoves(movePool, board.getSideToMove());
	Move* move = firstMove;
	Move* bestMove = firstMove;

	_nodes = 0;

	for (int depth = 1; depth <= _depth; depth++) {

		uint32_t time = getTickCount();
		uint32_t totalTime = 0;
		score = 0;
		bestScore = -maxScore;
		move = firstMove;
		while (move) {
			_nodes++;
			MoveBackup backup;
			board.doMove(*move,backup);

			//std::cout << "depth: " << depth << "score: " << score << " - move: " << move->toString() << " - nodes: " << _nodes << " - time: " << (getTickCount() - time) << std::endl;
			//board.printBoard(); // test

			score = -pvSearch(board, -maxScore, maxScore, depth-1);
			move->score=score;

			if (score > bestScore) {
				bestScore = score;
				bestMove = move;
			}

			board.undoMove(backup);
			move = move->next;
		}
		time = getTickCount()-time;
		totalTime += time;

		if (isUpdateUci()) {
			std::cout << "info depth "<< depth << std::endl;
			std::cout << "info score cp " << score << " depth " << depth << " nodes " << _nodes << " time " << time << " pv " << std::endl/*TODO pv*/;
			if (totalTime>1000) {
				std::cout << "info nps " << (_nodes/(totalTime/1000)) << std::endl;
			} else {
				std::cout << "info nps " << _nodes << std::endl;
			}

		}

		// TODO sort moves

	}

	if (bestMove) {
		score = bestMove->score;
		if (isUpdateUci()) {
			std::cout << "bestmove " << bestMove->toString() << std::endl;
		}
	}

	return score;
}

// principal variation search
int SimplePVSearch::pvSearch(Board& board, int alpha, int beta, int depth) {

	bool bSearch = true;
	int score = 0;

#if CHECK_MOVE_GEN_ERRORS
	Board old(board);
#endif

	if( depth == 0 ) {
		score = qSearch(board, alpha, beta, maxQuiescenceSearchDepth);
		SearchAgent::getInstance()->hashPut(board,score,depth,board.getMoveCounter());
		return score;
	}

	_nodes++;

	HashData hashData;
	if (SearchAgent::getInstance()->hashGet(board.getKey(), hashData)) {
		if (hashData.depth>=depth) {
			return hashData.value;
		}
	}

	{

		MovePool movePool;
		Move* move = board.generateAllMoves(movePool, board.getSideToMove());
#if CHECK_MOVE_GEN_ERRORS
		Key key1 = old.generateKey();
#endif
		while ( move )  {


			MoveBackup backup;
			board.doMove(*move,backup);
#if CHECK_MOVE_GEN_ERRORS
			Board newBoard(board);
#endif
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

#if CHECK_MOVE_GEN_ERRORS
			Key key2 = board.generateKey();
			if (key1!=key2) {
				errorCount++;
				std::cout << "CRITICAL - Error in undoMove: restored board differs from original " << std::endl;
				std::cout << "Original board: " << std::endl;
				old.printBoard();
				std::cout << "Board with move: " << std::endl;
				newBoard.printBoard();
				std::cout << "Board with undone move: " << std::endl;
				board.printBoard();
				std::cout << move->toString() << std::endl;
				std::cout << "End - Error in undoMove " << std::endl;
				if (errorCount>EXIT_PROGRAM_THRESHOLD) {
					std::cout << "Error count exced threshold: " << errorCount << std::endl;
					exit(1);
				}
			}
#endif
			if( score >= beta ) {
				SearchAgent::getInstance()->hashPut(board,score,depth,board.getMoveCounter());
				return beta;
			}

			if( score > alpha ) {
				alpha = score;
				bSearch = false;
			}
		}

	}

	SearchAgent::getInstance()->hashPut(board,score,depth,board.getMoveCounter());

	return alpha;

}

//quiescence search
int SimplePVSearch::qSearch(Board& board, int alpha, int beta, int depth) {

	_nodes++;

	int standPat = evaluate(board);

	if( standPat >= beta ) {
		return beta;
	}

	if( alpha < standPat ) {
		alpha = standPat;
	}
	{

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













