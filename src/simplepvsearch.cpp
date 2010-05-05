/*
	Redqueen Chess Engine
    Copyright (C) 2008-2010 Ben-Hur Carlos Vieira Langoni Junior

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


#include "simplepvsearch.h"
// If set to true, will check move integrity - used for trace purposes
#define CHECK_MOVE_GEN_ERRORS false
// show stats info
#define SHOW_STATS false

SearchAgent* agent = 0;

// root search
void SimplePVSearch::search() {

	agent = SearchAgent::getInstance();
	Board board(_board);
	stats.clear();
	clearHistory();
	errorCount=0;
	_startTime = getTickCount();
	timeToStop = clock() + toClock(_timeToSearch);
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
	pthread_exit(NULL);

}

// get current score
int SimplePVSearch::getScore() {
	return this->_score;
}

// iterative deepening
int SimplePVSearch::idSearch(Board& board) {

	MoveIterator::Move bestMove = MoveIterator::Move(NONE,NONE,EMPTY);
	bestMove.score = -maxScore;
	_nodes = 0;
	int totalTime = 0;
	int bestScore = -maxScore;
	int iterationScore[maxSearchDepth];

	board.generateAllMoves(rootMoves, board.getSideToMove());

	for (int depth = 1; depth <= _depth; depth++) {

		const int MATE_RANGE_CHECK = 10;

		PvLine pv = PvLine();
		pv.index=0;
		int time = getTickCount();
		int score = -rootSearch(board, -maxScore, maxScore, depth, 0, &pv);
		iterationScore[depth]=score;
		int repetition=0;

		for (int x=depth-1;x>=1;x--) {
			if (score==iterationScore[x]) {
				repetition++;
			} else {
				break;
			}
		}

		if (repetition >= MATE_RANGE_CHECK &&
				(abs(score) >= -maxScore-depth) ) {
			break;
		}

		if (stop(agent->getSearchInProgress()) &&
				depth > 1) {
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

			const int MATE_RANGE_SCORE = 300;
			std::string scoreString = "cp " + StringUtil::toStr(bestMove.score);

			if (abs(bestMove.score) > (maxScore-MATE_RANGE_SCORE)) {
				if (bestMove.score>0) {
					scoreString = "mate " +StringUtil::toStr((maxScore - bestMove.score+1)/2);
				} else {
					scoreString = "mate " +StringUtil::toStr(-(maxScore + bestMove.score)/2);
				}
			}

			long nps = totalTime>1000 ?  ((_nodes)/(totalTime/1000)) : _nodes;
			std::cout << "info depth "<< depth << std::endl;
			std::cout << "info depth "<< depth << " score " << scoreString << " time " << totalTime << " nodes " << (_nodes) << " nps " << nps << " pv" << pvLineToString(&pv) << std::endl;
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


// root search
int SimplePVSearch::rootSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv) {

	PvLine line = PvLine();
	const int uciOutputSecs=1500;
	int oldAlpha = alpha;
	int score = 0;
	MoveIterator::Move ttMove(NONE,NONE,EMPTY);

	_nodes++;
	bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);

	MoveIterator moves;
	moves(rootMoves);
	scoreMoves(board, moves, ttMove, alpha, beta, ply, true);
	moves.first();
	int moveCounter=0;

	while (moves.hasNext()) {

		MoveIterator::Move& move = moves.next();
		MoveBackup backup;
		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

		moveCounter++;

		if (isUpdateUci() && (_startTime+uciOutputSecs < getTickCount())) {
			std::cout << "info currmove " << move.toString() << " currmovenumber " << moveCounter << std::endl;
		}

		score = -normalSearch(board, -beta, -alpha, depth-1, ply+1, &line, true, true);

		if (score > alpha && !stop(agent->getSearchInProgress())) {
			score = -pvSearch(board, -beta, -alpha, depth-1, ply+1, &line, true);
		}

		move.score=score;
		board.undoMove(backup);

		if( score >= beta) {
			stats.ttLower++;
			updateHistory(board,move,depth,ply);
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			updatePv(pv, line, move);
		}

	}

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	if (alpha>oldAlpha) {
		updateHistory(board,pv->moves[0],depth,ply);
		stats.ttExact++;
	} else {
		stats.ttUpper++;
	}

	return alpha;

}

// principal variation search
int SimplePVSearch::pvSearch(Board& board, int alpha, int beta,
		int depth, int ply, PvLine* pv,	const bool allowPvSearch) {


	if (alpha > maxScore - ply - 1) {
		return alpha;
	}

	if	(board.isDraw()) {
		return 0;
	}

	PvLine line = PvLine();

	if (depth<=0) {
		return qSearch(board, alpha, beta, maxQuiescenceSearchDepth, ply, &line);
	}

	_nodes++;
	int oldAlpha = alpha;
	int score = 0;
	int extension=0;
	MoveIterator::Move ttMove(NONE,NONE,EMPTY);
	SearchAgent::HashData hashData;

	if (agent->hashGet(board.getKey(), hashData, ply, maxScore)) {
		ttMove=hashData.move;
	}

	if (alpha>=beta) {
		return alpha;
	}

	if (ply >= maxSearchPly) {
		pv->index=0;
		return evaluator.evaluate(board);
	}

	bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);

	if (isKingAttacked) {
		extension++;
	}

	if (allowIIDAtPV && allowPvSearch
			&& depth > 2 && ttMove.from == NONE && !isKingAttacked) {
		const int iidSearchDepth = depth-2;
		PvLine pvCandidate;
		score = pvSearch(board,alpha,beta,iidSearchDepth,ply+1,&pvCandidate,false);
		ttMove=pvCandidate.moves[0];
	}

	MoveIterator moves;
	board.generateAllMoves(moves, board.getSideToMove());
	scoreMoves(board, moves, ttMove, alpha, beta, ply, false);
	moves.first();

	int moveCounter=0;
	int remainingMoves=0;

	while (moves.hasNext()) {

		MoveIterator::Move& move = moves.next();
		MoveBackup backup;
		int reduction=1;
		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

		moveCounter++;

		if (move.type == MoveIterator::NON_CAPTURE) {
			remainingMoves++;
		}

		if (okToReduce(board, move, backup, depth, remainingMoves, isKingAttacked)) {
			reduction++;
		}

		if ( moveCounter==1 ) {

			score = -pvSearch(board, -beta, -alpha, depth-reduction+extension, ply+1, &line, allowPvSearch);

		} else {

			score = -normalSearch(board, -beta, -alpha, depth-reduction+extension, ply+1, &line, true, allowPvSearch);

			if (score > alpha && !stop(agent->getSearchInProgress())) {

				score = -pvSearch(board, -beta, -alpha, depth-reduction+extension, ply+1, &line, allowPvSearch);

			}

		}

		board.undoMove(backup);

		if (stop(agent->getSearchInProgress())) {
			return 0;
		}

		if( score >= beta) {
			stats.ttLower++;
			agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::LOWER,move);
			updateHistory(board,move,depth,ply);
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			updatePv(pv, line, move);
		}

	}

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	if (alpha>oldAlpha) {
		updateHistory(board,pv->moves[0],depth,ply);
		stats.ttExact++;
	} else {
		stats.ttUpper++;
	}

	agent->hashPut(board,alpha,depth,ply,maxScore,(alpha>oldAlpha ? SearchAgent::EXACT : SearchAgent::UPPER),pv->moves[0]);

	return alpha;

}

// normal search - non pv nodes
int SimplePVSearch::normalSearch(Board& board, int alpha, int beta,
		int depth, int ply, PvLine* pv,	const bool allowNullMove, const bool allowPvSearch) {


	if (alpha > maxScore - ply - 1) {
		return alpha;
	}

	if	(board.isDraw()) {
		return 0;
	}

	PvLine line = PvLine();

	if (depth<=0) {
		return qSearch(board, beta-1, beta, maxQuiescenceSearchDepth, ply+1, &line);
	}

	_nodes++;
	int oldAlpha = alpha;
	int score = 0;
	int extension = 0;
	MoveIterator::Move ttMove(NONE,NONE,EMPTY);
	SearchAgent::HashData hashData;
	if (agent->hashGet(board.getKey(), hashData, ply, maxScore)) {
		if (hashData.depth>=depth) {
			if ((hashData.flag == SearchAgent::UPPER && hashData.value <= alpha) ||
					(hashData.flag == SearchAgent::LOWER && hashData.value >= beta) ||
					(hashData.flag == SearchAgent::EXACT)) {
				stats.ttHits++;
				return hashData.value;
			}
			ttMove=hashData.move;
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

	if ((!isKingAttacked) && beta < maxScore && allowNullMove && okToNullMove(board)) {

		const int reduction = 3;
		MoveBackup backup;
		board.doNullMove(backup);
		score = -normalSearch(board, -beta, -(beta-1), depth-reduction, ply+1, &line, false, true);
		board.undoNullMove(backup);

		if (stop(agent->getSearchInProgress())) {
			return 0;
		}

		if (score >= beta) {
			stats.nullMoveHits++;
			agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::LOWER,MoveIterator::Move());
			return beta;
		}
	}

	if (isKingAttacked) {
		extension++;
	}

	if (allowIIDAtNormal && allowPvSearch &&
			depth > 2 && ttMove.from == NONE && !isKingAttacked) {
		const int iidSearchDepth = depth/2;
		PvLine pvCandidate;
		score = normalSearch(board,alpha,beta,iidSearchDepth,ply+1,&pvCandidate,false, true);
		ttMove=pvCandidate.moves[0];
	}

	MoveIterator moves;
	board.generateAllMoves(moves, board.getSideToMove());
	scoreMoves(board, moves, ttMove, alpha, beta, ply, false);

	moves.first();
	int moveCounter=0;
	int remainingMoves=0;

	while (moves.hasNext()) {

		MoveIterator::Move& move = moves.next();
		MoveBackup backup;
		int reduction=1;
		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

		moveCounter++;

		if (move.type == MoveIterator::NON_CAPTURE) {
			remainingMoves++;
		}

		if (okToReduce(board, move, backup, depth, remainingMoves, isKingAttacked)) {
			reduction++;
		}

		score = -normalSearch(board, -beta, -(beta-1), depth-reduction+extension, ply+1, &line, allowNullMove, allowPvSearch);

		board.undoMove(backup);

		if (stop(agent->getSearchInProgress())) {
			return 0;
		}

		if( score >= beta) {
			stats.ttLower++;
			agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::LOWER,move);
			updateHistory(board,move,depth,ply);
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			updatePv(pv, line, move);
		}

	}

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	if (alpha>oldAlpha) {
		updateHistory(board,pv->moves[0],depth,ply);
		stats.ttExact++;
	} else {
		stats.ttUpper++;
	}

	agent->hashPut(board,alpha,depth,ply,maxScore,(alpha>oldAlpha ? SearchAgent::EXACT : SearchAgent::UPPER),pv->moves[0]);

	return alpha;

}

//quiescence search
int SimplePVSearch::qSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv) {

	_nodes++;

	if (stop(agent->getSearchInProgress())) {
		return 0;
	}

	int oldAlpha = alpha;
	MoveIterator::Move ttMove(NONE,NONE,EMPTY);
	SearchAgent::HashData hashData;
	if (agent->hashGet(board.getKey(), hashData, ply, maxScore)) {
		if (hashData.depth>=depth) {
			if (((hashData.flag == SearchAgent::UPPER && hashData.value <= alpha) ||
					(hashData.flag == SearchAgent::LOWER && hashData.value >= beta) ||
					(hashData.flag == SearchAgent::EXACT)) && (beta - alpha != 1)) {
				stats.ttHits++;
				return hashData.value;
			}
			ttMove=hashData.move;
		}
	}

	int standPat = evaluator.evaluate(board);

	if(standPat>=beta||depth==0) {
		pv->index=0;
		return beta;
	}

	if( alpha < standPat ) {
		alpha = standPat;
	}

	PvLine line = PvLine();

	MoveIterator moves;
	board.generateCaptures(moves, board.getSideToMove());
	scoreMoves(board, moves, alpha, beta, 0);

	moves.first();

	while (moves.hasNext())  {

		MoveIterator::Move& move = moves.next();
		MoveBackup backup;

		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

		int score = -qSearch(board, -beta, -alpha, depth-1, ply+1, &line);

		board.undoMove(backup);

		if (stop(agent->getSearchInProgress())) {
			return 0;
		}

		if( score >= beta ) {
			stats.ttLower++;
			agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::LOWER,move);
			updateHistory(board,move,depth,ply);
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			updatePv(pv, line, move);
		}

	}

	if (alpha>oldAlpha) {
		updateHistory(board,pv->moves[0],depth,ply);
		stats.ttExact++;
	} else {
		stats.ttUpper++;
	}

	agent->hashPut(board,alpha,depth,ply,maxScore,(alpha>oldAlpha ? SearchAgent::EXACT : SearchAgent::UPPER),pv->moves[0]);

	return alpha;
}

