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
#define PV_SEARCH true
// debug iterative d search
#define DEBUG_ID false
// debug alpha-beta search
#define DEBUG_AB false
// debug qs search
#define DEBUG_QS false
// show stats info
#define SHOW_STATS false

using namespace SimplePVSearchTypes;

// root search
void SimplePVSearch::search() {

	Board board(_board);
	stats.clear();
	errorCount=0;
	_startTime = getTickCount();
	timeToStop = clock() + ((((_timeToSearch - 5)/1000)*CLOCKS_PER_SEC));
	evaluator.setGameStage(evaluator.getGameStage(board));
	_score = idSearch(board);
	SearchAgent::getInstance()->setSearchInProgress(false);
	_time = getTickCount() - _startTime;

#if CHECK_MOVE_GEN_ERRORS
	// checks for data corruption
	Key oldKey = _board.generateKey();
	Key newKey = board.generateKey();

	if (oldKey!=newKey) {
		std::cout << "old key " << oldKey <<std::endl;
		std::cout << "new key " << newKey <<std::endl;
		_board.printBoard();
		board.printBoard();
	}

	assert(oldKey==newKey);
#endif

}

// get current score
int SimplePVSearch::getScore() {
	return this->_score;
}

// iterative deepening
int SimplePVSearch::idSearch(Board& board) {

	SearchAgent* agent = SearchAgent::getInstance();
	MoveIterator::Move bestMove = MoveIterator::Move(NONE,NONE,EMPTY);
	bestMove.score = -maxScore;
	_nodes = 0;
	int totalTime = 0;
	int bestScore = -maxScore;

#if DEBUG_ID
	board.printBoard();
	std::cout << "Eval: " << evaluator.evaluate(board) << std::endl;
#endif
	for (int depth = 1; depth <= _depth; depth++) {
		if (stop(agent->getSearchInProgress())) {
			break;
		}
		PvLine pv = PvLine();
		pv.index=0;
		moveFound=false;
		int time = getTickCount();

		int score = -pvSearch(board, -maxScore, maxScore, depth, 0, &pv, true, true);

		if (!moveFound) {
			break;
		}
		bestMove = pv.moves[0];

		if (score > bestScore) {
			bestScore = score;
			stats.pvChanges++;
		}

		stats.searchDepth=depth;
		time = getTickCount()-time;
		totalTime += time;
		stats.searchTime=totalTime;
		stats.searchNodes=_nodes;

		if (isUpdateUci()) {

			uint64_t nps = totalTime>1000 ?  ((_nodes)/(totalTime/1000)) : _nodes;
			std::cout << "info depth "<< depth << std::endl;
			std::cout << "info depth "<< depth << " score cp " << bestMove.score << " time " << totalTime << " nodes " << (_nodes) << " nps " << nps << " pv" << pvLineToString(&pv) << std::endl;
			std::cout << "info nodes " << (_nodes) << " time " << totalTime << " nps " << nps << " hashfull " << agent->hashFull() << std::endl;

		}

#if SHOW_STATS
		std::cout << "Search stats: " << std::endl;
		std::cout << stats.toString() << std::endl;
#endif
	}

	if (bestMove.from!=NONE) {
		if (isUpdateUci()) {
			std::cout << "bestmove " << bestMove.toString() << std::endl;
		}
	} else {
		std::cout << "bestmove (none)" << std::endl;
	}

	return bestMove.score;
}

// sort search moves
int SimplePVSearch::sortMoves(Board& board, MoveIterator& moves, int alpha, int beta, int ply) {

	const int iidDepth=2;
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

		int score = -pvSearch(board,  -beta, -alpha, iidDepth, ply+1, &line, true, false);

		move.score=score;
		board.undoMove(backup);

		if (score > alpha) {
			alpha = score;
		}

		moveCounter++;
		moves.sortOne();
	}

	return moveCounter;

}

// sort moves of quiscence search
int SimplePVSearch::sortQMoves(Board& board, MoveIterator& moves) {

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

// principal variation search
int SimplePVSearch::pvSearch(Board& board, int alpha, int beta,
							 int depth, int ply, PvLine* pv,
							 const bool allowNullMove, const bool allowIid) {

	if	(board.isDraw() && ply) {
		return 0;
	}

#if PV_SEARCH
	bool bSearch = true;
#endif
	SearchAgent* agent = SearchAgent::getInstance();
	PvLine line = PvLine();
#if CHECK_MOVE_GEN_ERRORS
	Board old(board);
#endif

	if (depth==0) {
		int score = qSearch(board, alpha, beta, maxQuiescenceSearchDepth, &line);
		agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::LOWER,MoveIterator::Move());
		stats.ttLower++;
		return score;
	}

	_nodes++;
	int oldAlpha = alpha;
	int score = 0;
	SearchAgent::HashData hashData;
	if (ply && agent->hashGet(board.getKey(), hashData, ply, maxScore)) {
		if (hashData.depth>=depth) {
			if ((hashData.flag == SearchAgent::UPPER && hashData.value <= alpha) ||
					(hashData.flag == SearchAgent::LOWER && hashData.value >= beta) ||
					(hashData.flag == SearchAgent::EXACT)) {
				stats.ttHits++;
				return hashData.value;
			}
		}
	}

	if (alpha>=beta) {
		return alpha;
	}

	if (ply >= maxSearchPly) {
		pv->index=0;
		return evaluator.evaluate(board);
	}

	bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);

	if (!isKingAttacked) {

		Bitboard pawns = board.getPiecesByType(WHITE_PAWN) |
				board.getPiecesByType(BLACK_PAWN);
		Bitboard kings = board.getPiecesByType(WHITE_KING) |
				board.getPiecesByType(BLACK_KING);

		bool okToNullMove = ((pawns|kings)^board.getAllPieces());

		if (beta < maxScore && allowNullMove && pawns && depth > 1 && okToNullMove) {

			int reduction = depth >= 3 ? 3 : 2;

			MoveBackup backup;
			board.doNullMove(backup);
			score = -pvSearch(board, -beta, 1-beta, depth-reduction, ply+1, &line, false, false);
			board.undoNullMove(backup);
			if (stop(agent->getSearchInProgress())) {
				return 0;
			}
			if (score >= beta) {
				stats.nullMoveHits++;
				agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::LOWER,MoveIterator::Move());
				return score;
			}
		}
	} else {
		depth++;
	}

	MoveIterator moves;
	board.generateAllMoves(moves, board.getSideToMove());
	int totalMoves = moves.size();

	if (allowIid) {
		totalMoves = sortMoves(board, moves, alpha, beta, ply);

		if (!totalMoves) {
			return isKingAttacked ? -maxScore+ply : 0;
		}
	}

	moves.first();
	int moveCounter=0;
	const int prunningDepth=2;
	const int prunningMoves=4;

#if CHECK_MOVE_GEN_ERRORS
	Key key1 = old.generateKey();
#endif

	while (moves.hasNext()) {

		MoveIterator::Move& move = moves.next();
		if (move.score==notLegal) {
			continue;
		}
		moveCounter++;
		MoveBackup backup;
		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

		if (!ply && isUpdateUci()) {
			if (_startTime+1500 < getTickCount()) {
				std::cout << "info currmove " << move.toString() << " currmovenumber " << moveCounter << std::endl;
			}
		}

#if CHECK_MOVE_GEN_ERRORS
		Board newBoard(board);
#endif

		int reduction=1;
		if (!isKingAttacked && allowIid) {
			const int reductionValue = (depth >= 3? 3 : 2);

			if ((depth > prunningDepth) &&
				//(move.score <= alpha) &&
				(!(backup.hasCapture || backup.hasPromotion) &&
				(moveCounter >= prunningMoves))) {
				reduction=reductionValue;
			}
		}

#if PV_SEARCH
		if ( bSearch ) {
#endif
			score = -pvSearch(board, -beta, -alpha, depth-reduction, ply+1, &line, allowNullMove, allowIid);

#if PV_SEARCH
		} else {

			score = -pvSearch(board, -alpha-1, -alpha, depth-reduction, ply+1, &line, allowNullMove, allowIid);

			if ( (score > alpha) && (score < beta) && !stop(agent->getSearchInProgress())) {
				score = -pvSearch(board, -beta, -alpha, depth-reduction, ply+1, &line, allowNullMove, allowIid);
			}
		}

#endif
#if DEBUG_AB
		std::cout << "(AB) score: " << score << " / move: " << move.toString() << " / depth " << depth << std::endl;
		std::string pad=" ";
		pad.append((20-depth)*4, ' ');
		board.printBoard(pad);
#endif

		board.undoMove(backup);

		if (stop(agent->getSearchInProgress())) {
			return 0;
		}

#if CHECK_MOVE_GEN_ERRORS
		Key key2 = board.generateKey();
		if (key1!=key2) {
			errorCount++;
			std::cout << "CRITICAL - Error in undoMove: restored board differs from original " << std::endl;
			std::cout << "Original board: " << std::endl;
			old.printBoard();
			std::cout << "position fen " << old.getFEN() << std::endl;
			std::cout << "Board with move: " << std::endl;
			newBoard.printBoard();
			std::cout << "Board with undone move: " << std::endl;
			board.printBoard();
			std::cout << move.toString() << std::endl;
			std::cout << "End - Error in undoMove " << std::endl;
			if (errorCount>EXIT_PROGRAM_THRESHOLD) {
				std::cout << "Error count exceed threshold: " << errorCount << std::endl;
				exit(1);
			}
		}
#endif
		if( score >= beta ) {
			stats.ttLower++;
			agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::LOWER,move);
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
#if PV_SEARCH
			bSearch = false;
#endif
			if (!stop(agent->getSearchInProgress())) {
				updatePv(pv, line, move);
				if (!ply) {
					moveFound=true;
				}
			}

		}

	}

	if (alpha>oldAlpha) {
		stats.ttExact++;
	} else {
		stats.ttUpper++;
	}

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	agent->hashPut(board,alpha,depth,ply,maxScore,(alpha>oldAlpha ? SearchAgent::EXACT : SearchAgent::UPPER),pv->moves[0]);

	return alpha;

}

//quiescence search
int SimplePVSearch::qSearch(Board& board, int alpha, int beta, int depth, PvLine* pv) {

	SearchAgent* agent = SearchAgent::getInstance();

	_nodes++;

	if (stop(agent->getSearchInProgress())) {
		return 0;
	}

	int standPat = evaluator.evaluate(board);

	if(standPat>=beta||depth==0) {
#if DEBUG_QS
		std::cout << "(QS) beta: " << beta << " / eval: " << standPat << " / depth " << depth << std::endl;
		std::string pad=" ";
		pad.append((maxQuiescenceSearchDepth-depth+_depth)*4, ' ');
		board.printBoard(pad);
#endif
		pv->index=0;
		return beta;
	}

	if( alpha < standPat ) {
		alpha = standPat;
	}

	PvLine line = PvLine();

	MoveIterator moves;
	board.generateCaptures(moves, board.getSideToMove());
	sortQMoves(board,moves);
	moves.first();

	while (moves.hasNext())  {

		MoveIterator::Move& move = moves.next();
		MoveBackup backup;

		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

		int score = -qSearch(board, -beta, -alpha, depth-1, &line);

#if DEBUG_QS
		std::cout << "(QS) score: " << alpha << " / move: " << move.toString() << " / depth " << depth << std::endl;
		std::string pad=" ";
		pad.append((depth)*4, ' ');
		board.printBoard(pad);
#endif

		board.undoMove(backup);

		if (stop(agent->getSearchInProgress())) {
			return 0;
		}

		if( score >= beta ) {
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			updatePv(pv, line, move);
		}

	}

	return alpha;
}


