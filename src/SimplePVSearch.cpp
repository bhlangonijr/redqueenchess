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
// Exit the program if the count of errors in CHECK_MOVE_GEN_ERRORS exceed the threshold - used for trace purposes
#define EXIT_PROGRAM_THRESHOLD 100
// If true uses Principal Variation Search
#define PV_SEARCH false

using namespace SimplePVSearchTypes;

SimplePVSearch::SimplePVSearch(Board& board) :  _depth(1),  _board(board), _updateUci(true){

}

SimplePVSearch::~SimplePVSearch(){

}

// root search
void SimplePVSearch::search() {
	this->clearPv();
	this->getPv().assign(_depth,Move());
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
	int bestScore = -maxScore;
	MovePool movePool;
	MovePool pvPool;
	Move* firstMove = board.generateAllMoves(movePool, board.getSideToMove());
	Move* move = firstMove;
	Move* bestMove;
	if (firstMove) {
		bestMove=pvPool.construct(Move(firstMove));
	}

	_nodes = 0;

	for (int depth = 1; depth <= _depth; depth++) {

		uint32_t time = getTickCount();
		uint32_t totalTime = 0;
		score = 0;
		bestScore = -maxScore;
		move = firstMove;
		if (bestMove) {
			bestMove->copy(move);
			bestMove->next=NULL;
		}

		FOREACHMOVE(move) {
			_nodes++;
			MoveBackup backup;
			board.doMove(move,backup);
			score = -pvSearch(board, bestScore, -bestScore, depth-1, depth-1, bestMove, pvPool);
			move->score=score;

			if (score > bestScore) {
				bestScore = score;
				if (bestMove) {
					bestMove->copy(move);
				}
				updatePv(bestMove,0,depth);
			}

			board.undoMove(backup);
		}
		time = getTickCount()-time;
		totalTime += time;

		if (isUpdateUci()) {
			std::cout << "info depth "<< depth << std::endl;
			std::cout << "info score cp " << bestScore << " depth " << depth << " nodes " << _nodes << " time " << time << " pv " << getPvString(depth) << std::endl;
			if (totalTime>1000) {
				std::cout << "info nps " << (_nodes/(totalTime/1000)) << std::endl;
			} else {
				std::cout << "info nps " << _nodes << std::endl;
			}

		}
		firstMove = sortMoves(movePool, firstMove);
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
int SimplePVSearch::pvSearch(Board& board, int alpha, int beta, int depth, int maxDepth, Move* pvNode, MovePool& pvPool) {

#if PV_SEARCH
	bool bSearch = true;
#endif
	int score = 0;
#if CHECK_MOVE_GEN_ERRORS
	Board old(board);
#endif

	if( depth == 0 ) {
		score = qSearch(board, alpha, beta, maxQuiescenceSearchDepth, maxQuiescenceSearchDepth);
		SearchAgent::getInstance()->hashPut(board,score,depth,0);
		return score;
	}

	_nodes++;

	TranspositionTable::HashData hashData;
	if (SearchAgent::getInstance()->hashGet(board.getKey(), hashData)) {
		if (hashData.depth>=depth) {
			return hashData.value;
		}
	}

	{

		MovePool movePool;
		Move* move = board.generateAllMoves(movePool, board.getSideToMove());
		Move* bestMove;
		if (move) {
			bestMove=pvPool.construct(Move(move));
			bestMove->next=NULL;
		}
		if (pvNode) {
			pvNode->next=bestMove;
		}
		//bool ktbt = false;
		if (board.isAttacked(board.getSideToMove(), KING)) {
			//std::cout << "My King to be taken " << std::endl;
			if (!move) {
				//std::cout << "... and there's nothing to do " << std::endl;
				//board.printBoard();
				return -maxScore;
			}
			//	ktbt = true;
		}
		/*if (board.isAttacked(board.flipSide(board.getSideToMove()), KING)) {
			std::cout << "Their King to be taken " << std::endl;
		}*/


#if CHECK_MOVE_GEN_ERRORS
		Key key1 = old.generateKey();
#endif
		FOREACHMOVE(move) {

			MoveBackup backup;
			board.doMove(move,backup);

			/*
			if (backup.capturedPiece==WHITE_KING || backup.capturedPiece==BLACK_KING ) {
				std::cout << "Captured king " << std::endl;
				board.printBoard();
			} else if (ktbt) {
				std::cout << "King is safe now! " << std::endl;
				board.printBoard();
			}

			 */


#if CHECK_MOVE_GEN_ERRORS
			Board newBoard(board);
#endif

#if PV_SEARCH
			if ( bSearch ) {
#endif
				score = -pvSearch(board, -beta, -alpha, depth - 1, maxDepth, bestMove, pvPool);
#if PV_SEARCH
			} else {
				score = -pvSearch(board, -alpha-1, -alpha, depth - 1, maxDepth, bestMove, pvPool);
				if ( score > alpha ) {
					score = -pvSearch(board, -beta, -alpha, depth - 1, maxDepth, bestMove, pvPool);
				}
			}
#endif
			board.undoMove(backup);
			/*
			if (backup.capturedPiece==WHITE_KING || backup.capturedPiece==BLACK_KING ) {
				std::cout << "before capture king " << " Depth: " << depth << std::endl;
				board.printBoard();

			}

			assert(!(backup.capturedPiece==WHITE_KING || backup.capturedPiece==BLACK_KING));*/

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
				SearchAgent::getInstance()->hashPut(board,score,depth,0);
				return beta;
			}

			if( score > alpha ) {
				if (bestMove) {
					bestMove->copy(move);
				}
				//std::cout << "Move: " << bestMove->toString() << " at " << depth << std::endl;

				alpha = score;
#if PV_SEARCH
				bSearch = false;
#endif
			}
		}
	}

	SearchAgent::getInstance()->hashPut(board,alpha,depth,0);

	return alpha;

}

//quiescence search
int SimplePVSearch::qSearch(Board& board, int alpha, int beta, int depth, int maxDepth) {

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

		FOREACHMOVE(move)  {
			MoveBackup backup;
			board.doMove(move,backup);

			int score = -qSearch(board, -beta, -alpha, depth - 1, maxDepth);

			board.undoMove(backup);

			if( score >= beta ) {
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

//sort
void SimplePVSearch::sort(std::vector<Move*>& moves) {
	Move* tmp;
	bool flag=true;
	for(int i = 0; i < moves.size()&&flag; i++){
		flag=false;
		for(int j = 0; j < moves.size()-1; j++)
		{
			if (moves[j+1]->score>moves[j]->score) {
				tmp=moves[j];
				moves[j]=moves[j+1];
				moves[j+1]=tmp;
				flag=true;
			}

		}
	}

}

// sort moves by score
Move* SimplePVSearch::sortMoves(MovePool& movePool, Move* firstMove) {

	std::vector<Move*> moves;
	Move* move=firstMove;

	FOREACHMOVE (move) {
		moves.push_back(move);
	}

	if (moves.size()<1) {
		return NULL;
	}

	sort(moves);
	for(int x=1;x<moves.size();x++) {
		moves[x-1]->next = moves[x];
	}
	moves[moves.size()-1]->next=NULL;

	return moves[0];

}

void SimplePVSearch::updatePv(Move* move, int depth, int maxDepth) {

	Move* tmp=move;
	int n=0;
	FOREACHMOVE(tmp) {
		updatePv(n,*tmp);
		n++;
	}

}









