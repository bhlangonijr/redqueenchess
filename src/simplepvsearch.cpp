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
MoveIterator::Move emptyMove;

// root search
void SimplePVSearch::search() {

	agent = SearchAgent::getInstance();
	Board board(_board);
	stats.clear();
	clearHistory();
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

	_nodes = 0;
	int bestScore = -maxScore;
	int iterationScore[maxSearchDepth];
	int alpha = -maxScore;
	int beta = maxScore;
	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);
	initRootMovesOrder();

	if (!isKingAttacked) {
		board.generateAllMoves(rootMoves, board.getSideToMove());
	} else {
		board.generateEvasions(rootMoves, board.getSideToMove());
	}

	scoreMoves(board,rootMoves,true);
	for (int depth = 1; depth <= _depth; depth++) {

		PvLine pv = PvLine();
		pv.index=0;
		int score = 0;

		score = rootSearch(board, alpha, beta, depth, 0, &pv);

		if(score <= alpha || score >= beta) {
			score = rootSearch(board, -maxScore, maxScore, depth, 0, &pv);
		}

		iterationScore[depth]=score;

		if (depth >= aspirationDepth)	{
			int delta1 = iterationScore[depth - 0] - iterationScore[depth - 1];
			int delta2 = iterationScore[depth - 1] - iterationScore[depth - 2];
			int aspirationDelta = MAX(abs(delta1) + abs(delta2) / 2, 16);
			aspirationDelta = (aspirationDelta + 7) / 8 * 8;
			alpha = MAX(iterationScore[depth] - aspirationDelta, -maxScore);
			beta  = MAX(iterationScore[depth] + aspirationDelta,  maxScore);
		}

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

		if (stop(agent->getSearchInProgress()) && depth > 1) {
			break;
		}

		if (score > bestScore) {
			bestScore = score;
			stats.pvChanges++;
		}

		stats.bestMove=pv.moves[0];
		stats.searchDepth=depth;
		stats.searchTime=getTickCount()-_startTime;
		stats.searchNodes=_nodes;

		uciOutput(&pv, stats.bestMove, stats.searchTime, agent->hashFull(), depth);

#if SHOW_STATS
		std::cout << "Search stats: " << std::endl;
		std::cout << stats.toString() << std::endl;
#endif
	}

	uciOutput(stats.bestMove);

	return stats.bestMove.score;
}


// root search
int SimplePVSearch::rootSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv) {

	PvLine line = PvLine();
	int score = -maxScore;
	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);

	if (depth < 3) {
		rootMoves.sort();
	} else {
		rootMoves.sortRootMoves(nodesPerMove);
	}

	rootMoves.first();
	int moveCounter=0;
	int remainingMoves=0;
	MoveIterator::Move bestMove = pv->moves[0];

	while (rootMoves.hasNext()) {

		MoveIterator::Move& move = rootMoves.next();
		MoveBackup backup;
		long nodes=_nodes;
		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

		stats.searchTime=getTickCount()-_startTime;
		moveCounter++;

		if (move.type == MoveIterator::NON_CAPTURE) {
			remainingMoves++;
		}

		uciOutput(move, moveCounter);

		int reduction=0;
		int extension=0;
		bool reduced = adjustDepth(extension, reduction, board, move, backup, depth,
				remainingMoves, isKingAttacked,ply,false,score > alpha);
		int newDepth=depth-1;

		if (score > alpha) {
			score = -pvSearch(board, -beta, -alpha, newDepth-reduction+extension, ply+1, &line);
		} else {
			reduced=true;
			score = -normalSearch(board, -beta, -alpha, newDepth-reduction+extension, ply+1, &line, true);
		}
		if (score > alpha && reduced) {
			score = -pvSearch(board, -beta, -alpha, newDepth+extension, ply+1, &line);
		}

		board.undoMove(backup);

		move.score=score;
		nodes = (_nodes - nodes);
		updateRootMovesScore(nodes);

		if( score >= beta) {
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			updatePv(pv, line, move);
			bestMove=move;
		}

	}

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	return alpha;

}

// principal variation search
int SimplePVSearch::pvSearch(Board& board, int alpha, int beta,	int depth, int ply, PvLine* pv) {

	if (depth<=0) {
		return qSearch(board, alpha, beta, 0, ply, pv);
	}

	_nodes++;

	if (alpha > maxScore - ply - 1) {
		pv->index=0;
		return alpha;
	}

	if	(board.isDraw()) {
		pv->index=0;
		return 0;
	}

	PvLine line = PvLine();
	const int oldAlpha = alpha;
	int score = 0;
	MoveIterator::Move ttMove;
	SearchAgent::HashData hashData;

	if (agent->hashGet(board.getKey(), hashData, ply, maxScore)) {
		ttMove=hashData.move;
	}

	if (alpha>=beta) {
		pv->index=0;
		return alpha;
	}

	if (ply >= maxSearchPly) {
		pv->index=0;
		return evaluator.evaluate(board);
	}

	bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);

	if (depth > allowIIDAtPV &&	ttMove.from == NONE && !isKingAttacked ) {
		const int iidSearchDepth = depth-2;
		score = pvSearch(board,alpha,beta,iidSearchDepth,ply+1,&line);
		if (agent->hashGet(board.getKey(), hashData, ply, maxScore)) {
			ttMove=hashData.move;
		}
	}

	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;
	int moveCounter=0;
	int remainingMoves=0;

	while (true) {

		MoveIterator::Move& move = selectMove(board, moves, ttMove, isKingAttacked, ply);
		if (moves.end()) {
			break;
		}
		MoveBackup backup;
		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

		moveCounter++;

		if (move.type == MoveIterator::NON_CAPTURE) {
			remainingMoves++;
		}

		int reduction=0;
		int extension=0;
		bool reduced = adjustDepth(extension, reduction, board, move, backup, depth,
				remainingMoves, isKingAttacked,ply,false,moveCounter==1);
		int newDepth=depth-1;

		if (moveCounter==1) {
			score = -pvSearch(board, -beta, -alpha, newDepth-reduction+extension, ply+1, &line);
		} else {
			reduced=true;
			score = -normalSearch(board, -beta, -alpha, newDepth-reduction+extension, ply+1, &line, true);
		}

		if (score > alpha && score < beta && reduced && !stop(agent->getSearchInProgress())) {
			score = -pvSearch(board, -beta, -alpha, newDepth+extension, ply+1, &line);
		}

		board.undoMove(backup);

		if (stop(agent->getSearchInProgress())) {
			return 0;
		}

		if( score >= beta) {
			updateKillers(board,move,ply);
			agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::LOWER,move);
			stats.ttLower++;
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			updateHistory(board,move,depth);
			updatePv(pv, line, move);
			bestMove=move;
		}

	}

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	if (alpha>oldAlpha) {
		agent->hashPut(board,alpha,depth,ply,maxScore,SearchAgent::EXACT,bestMove);
		stats.ttExact++;
	} else {
		agent->hashPut(board,alpha,depth,ply,maxScore,SearchAgent::UPPER,emptyMove);
		stats.ttUpper++;
	}

	return alpha;

}

// normal search - non pv nodes
int SimplePVSearch::normalSearch(Board& board, int alpha, int beta,
		int depth, int ply, PvLine* pv,	const bool allowNullMove) {

	if (depth<=0) {
		return qSearch(board, beta-1, beta, 0, ply+1, pv);
	}

	_nodes++;

	if (alpha > maxScore - ply - 1) {
		return alpha;
	}

	if	(board.isDraw()) {
		return 0;
	}

	PvLine line = PvLine();
	const int oldAlpha = alpha;
	int score = 0;
	MoveIterator::Move ttMove;
	SearchAgent::HashData hashData;

	if (agent->hashGet(board.getKey(), hashData, ply, maxScore)) {
		if (hashData.depth>=depth) {
			const bool okToUseTT = ((
					(hashData.flag == SearchAgent::UPPER && hashData.value <= alpha) ||
					(hashData.flag == SearchAgent::LOWER && hashData.value >= beta) ||
					(hashData.flag == SearchAgent::EXACT)));

			if (okToUseTT) {
				stats.ttHits++;
				return hashData.value;
			}
		}
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
	bool nullMoveMateScore=false;

	if (!isKingAttacked && allowNullMove && okToNullMove(board)) {

		const int reduction = 3 + (depth > 4 ? depth/6 : 0);
		MoveBackup backup;
		board.doNullMove(backup);
		score = -normalSearch(board, -beta, -(beta-1), depth-reduction, ply+1, &line, false);
		board.undoNullMove(backup);

		if (stop(agent->getSearchInProgress())) {
			return 0;
		}

		if (score >= beta) {
			agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::LOWER,emptyMove);
			stats.nullMoveHits++;
			return score;
		}

		if (score < -maxScore+maxSearchPly) {
			nullMoveMateScore=true;
		}
	}

	if (depth > allowIIDAtNormal && ttMove.from == NONE &&
			!isKingAttacked && !nullMoveMateScore) {
		const int iidSearchDepth = depth/2;
		score = normalSearch(board,alpha,beta,iidSearchDepth,ply+1,&line,false);
		if (agent->hashGet(board.getKey(), hashData, ply, maxScore)) {
			ttMove=hashData.move;
		}
	}

	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;
	int moveCounter=0;
	int remainingMoves=0;

	while (true) {

		MoveIterator::Move& move = selectMove(board, moves, ttMove, isKingAttacked, ply);
		if (moves.end()) {
			break;
		}
		MoveBackup backup;
		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

		moveCounter++;

		if (move.type == MoveIterator::NON_CAPTURE) {
			remainingMoves++;
		}

		int reduction=0;
		int extension=0;
		bool reduced = adjustDepth(extension, reduction, board, move, backup, depth,
				remainingMoves, isKingAttacked,ply,nullMoveMateScore,false);
		int newDepth=depth-1;

		score = -normalSearch(board, -beta, -(beta-1), newDepth-reduction+extension, ply+1, &line, allowNullMove);

		if (score >= beta && reduced && !stop(agent->getSearchInProgress())) {
			score = -normalSearch(board, -beta, -(beta-1), newDepth+extension, ply+1, &line, allowNullMove);
		}

		board.undoMove(backup);

		if (stop(agent->getSearchInProgress())) {
			return 0;
		}

		if( score >= beta) {
			updateKillers(board,move,ply);
			agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::LOWER,move);
			stats.ttLower++;
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			updateHistory(board,move,depth);
			updatePv(pv, line, move);
			bestMove=move;
		}

	}

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	if (alpha>oldAlpha) {
		agent->hashPut(board,alpha,depth,ply,maxScore,SearchAgent::EXACT,bestMove);
		stats.ttExact++;
	} else {
		agent->hashPut(board,alpha,depth,ply,maxScore,SearchAgent::UPPER,emptyMove);
		stats.ttUpper++;
	}

	return alpha;
}

//quiescence search
int SimplePVSearch::qSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv) {

	_nodes++;

	if (stop(agent->getSearchInProgress())) {
		pv->index=0;
		return 0;
	}

	int standPat = evaluator.evaluate(board);

	if(standPat>=beta) {
		pv->index=0;
		return beta;
	}

	const int queenValue = pieceMaterialValues[WHITE_QUEEN];
	const int bigDelta =  queenValue + (isPawnPromoting(board) ? queenValue : 0);

	if (standPat < alpha - bigDelta) {
		pv->index=0;
		return alpha;
	}

	if( alpha < standPat ) {
		alpha = standPat;
	}

	if (ply >= maxSearchPly) {
		pv->index=0;
		return standPat;
	}

	if	(board.isDraw()) {
		return 0;
	}

	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);
	int moveCounter=0;
	bool pvNode = (beta - alpha > 1);
	PvLine line = PvLine();
	MoveIterator moves = MoveIterator();

	while (true)  {

		MoveIterator::Move& move = selectMove(board,moves,isKingAttacked);
		if (moves.end()) {
			break;
		}

		if (!isKingAttacked && !pvNode && depth < 0 &&
				move.type!=MoveIterator::PROMO_CAPTURE &&
				move.type!=MoveIterator::PROMO_NONCAPTURE &&
				!isPawnPromoting(board) &&
				move.score < 0) {
			continue;
		}

		MoveBackup backup;

		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}
		moveCounter++;

		int score = -qSearch(board, -beta, -alpha, depth-1, ply+1, &line);

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

	if (!moveCounter && isKingAttacked) {
		return -maxScore+ply;
	}

	return alpha;
}


//perft
long SimplePVSearch::perft(Board& board, int depth, int ply) {

	if (depth == 0) {
		return 1;
	}

	long time=0;

	if (ply==1) {
		time=getTickCount();
	}

	long nodes=0;
	MoveIterator moves = MoveIterator();
	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);

	while (true)  {

		MoveIterator::Move& move = selectMove(board, moves, emptyMove, isKingAttacked, ply);
		if (moves.end()) {
			break;
		}
		MoveBackup backup;

		board.doMove(move,backup);

		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue; // not legal
		}

		nodes += perft(board, depth-1, ply+1);

		board.undoMove(backup);

	}

	if (ply==1 && isUpdateUci()) {
		std::cout << "Node count: " << nodes << std::endl;
		std::cout << "Time: " << (getTickCount()-time) << std::endl;
	}

	return nodes;
}
