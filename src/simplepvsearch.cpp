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

// show stats info
#define SHOW_STATS false

// root search
void SimplePVSearch::search(Board& _board) {

	agent = SearchAgent::getInstance();
	Board board(_board);
	stats.clear();
	clearHistory();
	initRootMovesOrder();
	rootMoves.clear();
	_nodes = 0;
	_startTime = getTickCount();
	timeToStop = clock() + toClock(_timeToSearch-10);
	_score = idSearch(board);
	_time = getTickCount()-_startTime;
}

// get current score
int SimplePVSearch::getScore() {
	return this->_score;
}

// iterative deepening
int SimplePVSearch::idSearch(Board& board) {

	int bestScore = -maxScore;
	int iterationScore[maxSearchPly];
	long iterationTime[maxSearchPly];
	int alpha = -maxScore;
	int beta = maxScore;
	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);
	MoveIterator::Move easyMove;
	rootSearchInfo.eval = evaluator.evaluate(board);
	rootSearchInfo.inCheck = isKingAttacked;

	if (!isKingAttacked) {
		board.generateAllMoves(rootMoves, board.getSideToMove());
	} else {
		board.generateEvasions(rootMoves, board.getSideToMove());
	}

	filterLegalMoves(board,rootMoves);
	rootMoves.clearScore();

	for (int depth = 1; depth <= _depth; depth++) {

		PvLine pv = PvLine();
		pv.index=0;
		int score = 0;
		// like in stockfish
		if (depth >= aspirationDepth && abs(iterationScore[depth-1]) < winningScore)	{
			int delta1 = iterationScore[depth-1]-iterationScore[depth-2];
			int delta2 = iterationScore[depth-2]-iterationScore[depth-3];
			int aspirationDelta = MAX(abs(delta1)+abs(delta2)/2, 16)+7;
			alpha = MAX(iterationScore[depth-1]-aspirationDelta,-maxScore);
			beta  = MIN(iterationScore[depth-1]+aspirationDelta,+maxScore);
		} else {
			alpha=-maxScore;
			beta=maxScore;
		}

		score = rootSearch(board, rootSearchInfo, alpha, beta, depth, 0, &pv);

		if((score <= alpha || score >= beta) &&
				!stop(agent->shouldStop())) {
			alpha=-maxScore;
			beta=maxScore;
			score = rootSearch(board, rootSearchInfo, alpha, beta, depth, 0, &pv);
		}

		iterationScore[depth]=score;
		iterationTime[depth]=getTickCount()-_startTime;

		if (stop(agent->shouldStop()) && depth > 1) {
			break;
		}

		if (score > bestScore) {
			bestScore = score;
			stats.pvChanges++;
		}

		stats.bestMove=pv.moves[0];
		stats.ponderMove=pv.moves[1];
		stats.searchDepth=depth;
		stats.searchTime=getTickCount()-_startTime;
		stats.searchNodes=_nodes;

		uciOutput(&pv, stats.bestMove, stats.searchTime, agent->hashFull(), depth, alpha, beta);

		int repetition=0;

		for (int x=depth-1;x>=1;x--) {
			if (score==iterationScore[x]) {
				repetition++;
			} else {
				break;
			}
		}

		if (!(_searchFixedDepth || _infinite)) {

			if (repetition >= maxScoreRepetition ){
				if ((abs(iterationScore[depth - 0]) >= -maxScore-maxSearchPly) ||
						(abs(iterationScore[depth - 1]) >= -maxScore-maxSearchPly) ||
						iterationScore[depth - 0] == 0) {
					break;
				}
			}

			if (depth==2) {
				if (rootMoves.get(0).score > rootMoves.get(1).score + easyMargin ) {
					easyMove=rootMoves.get(0);
				}
			}

			if (depth>2) {
				if (stats.bestMove!=easyMove) {
					easyMove = MoveIterator::Move();
				}
			}

			if (depth>5 && rootMoves.size()==1) {
				break;
			}

			if (depth>7) {
				if (easyMove==pv.moves[0] && nodesPerMove[0]>=(_nodes*85)/100 &&
						iterationTime[depth] > _timeToSearch/10) {
					break;
				}
			}

			//std::cout << "timeToSearch: " << _timeToSearch << " prediction: " << predictTimeUse(iterationTime,_timeToSearch,depth+1) << std::endl;
			if (depth>5) {
				if (_timeToSearch <	predictTimeUse(iterationTime,_timeToSearch,depth+1)) {
					break;
				}
			}

		}

#if SHOW_STATS
		std::cout << "Search stats: " << std::endl;
		std::cout << stats.toString() << std::endl;
#endif
	}

	uciOutput(stats.bestMove,stats.ponderMove);

	return stats.bestMove.score;
}


// root search
int SimplePVSearch::rootSearch(Board& board, SearchInfo& si, int alpha, int beta, int depth, int ply, PvLine* pv) {

	PvLine line = PvLine();
	int score = -maxScore;
	const bool isKingAttacked = si.inCheck;
	rootMoves.sortOrderingBy(nodesPerMove);
	rootMoves.first();
	rootMoves.clearScore();

	int moveCounter=0;
	int remainingMoves=0;
	int reduction=0;
	int extension=0;
	MoveIterator::Move bestMove;

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

		const bool givingCheck = board.isAttacked(board.getSideToMove(),KING);
		bool reduced = adjustDepth(extension, reduction, board, move, depth,
				remainingMoves,isKingAttacked,givingCheck,ply,false);
		int newDepth=depth-1+extension;
		SearchInfo newSi(si.eval,givingCheck);

		if (score > alpha) {
			score = -pvSearch(board, newSi, -beta, -alpha, newDepth-reduction, ply+1, &line);
		} else {
			reduced=true;
			score = -zwSearch(board, newSi, -alpha, newDepth-reduction, ply+1, &line, true);
		}
		if (score > alpha && reduced) {
			score = -pvSearch(board, newSi, -beta, -alpha, newDepth, ply+1, &line);
		}

		board.undoMove(backup);

		nodes = _nodes-nodes;
		updateRootMovesScore(nodes);

		if (stop(agent->shouldStop()) && depth>1) {
			return 0;
		}

		if( score >= beta) {
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			rootMoves.clearScore();
			move.score=score;
			bestMove=move;
			updatePv(pv, line, bestMove);
		}

		uciOutput(move, moveCounter);

	}

	rootMoves.sortOrderingBy(nodesPerMove);
	rootMoves.first();

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	return alpha;

}

// principal variation search
int SimplePVSearch::pvSearch(Board& board, SearchInfo& si, int alpha, int beta,	int depth, int ply, PvLine* pv) {

	if (stop(agent->shouldStop())) {
		return 0;
	}

	if (depth<=0) {
		return qSearch(board, si, alpha, beta, 0, ply, pv);
	}

	PvLine line = PvLine();
	const int oldAlpha = alpha;
	int score = -maxScore;
	SearchAgent::HashData hashData;

	_nodes++;
	if	(board.isDraw() || ply >= maxSearchPly) {
		return 0;
	}

	alpha = MAX(-maxScore+ply, alpha);
	beta = MIN(maxScore-(ply+1), beta);

	if (alpha>=beta) {
		return alpha;
	}

	//tt retrieve
	agent->hashGet(board.getKey(), hashData, ply, maxScore);

	const bool isKingAttacked = si.inCheck;

	//iid
	if (depth > allowIIDAtPV &&	hashData.move.from == NONE && !isKingAttacked ) {
		const int iidSearchDepth = depth-2;
		score = pvSearch(board,si,alpha,beta,iidSearchDepth,ply,&line);
		agent->hashGet(board.getKey(), hashData, ply, maxScore);
	}

	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;
	int moveCounter=0;
	int remainingMoves=0;
	int reduction=0;
	int extension=0;

	while (true) {

		MoveIterator::Move& move = selectMove(board, moves, hashData.move, isKingAttacked, ply);

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

		const bool givingCheck = board.isAttacked(board.getSideToMove(),KING);

		const bool reduced = adjustDepth(extension, reduction, board, move, depth, remainingMoves,
				isKingAttacked,givingCheck,ply,false);
		SearchInfo newSi(si.eval,givingCheck);
		int newDepth=depth-1;
		bool fullSearch=false;

		if (moveCounter==1) {
			score = -pvSearch(board, newSi, -beta, -alpha, newDepth-reduction+extension, ply+1, &line);
			fullSearch=(score > alpha && reduced);
		} else {
			score = -zwSearch(board, newSi, -alpha, newDepth-reduction+extension, ply+1, &line, true);
			fullSearch=(score > alpha && score < beta);
		}

		if (fullSearch) {
			score = -pvSearch(board, newSi, -beta, -alpha, newDepth+extension, ply+1, &line);
		}

		board.undoMove(backup);

		if (stop(agent->shouldStop())) {
			return 0;
		}

		if( score >= beta) {
			updateKillers(board,move,ply);
			updateHistory(board,move,depth);
			agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::LOWER,move);
			stats.ttLower++;
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

	if (alpha>oldAlpha) {
		agent->hashPut(board,alpha,depth,ply,maxScore,SearchAgent::EXACT,bestMove);
		stats.ttExact++;
	} else {
		agent->hashPut(board,alpha,depth,ply,maxScore,SearchAgent::UPPER,emptyMove);
		stats.ttUpper++;
	}

	return alpha;

}

// zero window search - non pv nodes
int SimplePVSearch::zwSearch(Board& board, SearchInfo& si, int beta, int depth, int ply, PvLine* pv, const bool allowNullMove) {

	if (stop(agent->shouldStop())) {
		return 0;
	}

	if (depth<=0) {
		return qSearch(board, si, beta-1, beta, 0, ply, pv);
	}

	_nodes++;

	if	(board.isDraw() || ply >= maxSearchPly) {
		return 0;
	}

	if (-maxScore+ply >= beta) {
		return beta;
	}

	if (maxScore-(ply+1) < beta) {
		return beta-1;
	}

	PvLine line = PvLine();
	bool nullMoveMateScore=false;
	int score = 0;
	SearchAgent::HashData hashData;

	// tt retrieve & prunning
	if (agent->hashGet(board.getKey(), hashData, ply, maxScore)) {
		if (((allowNullMove && hashData.depth>=depth) ||
				(hashData.value >= MAX(maxScore-ply,beta)) ||
				(hashData.value < MIN(-maxScore+ply,beta))) &&
				((hashData.flag == SearchAgent::LOWER && hashData.value >= beta) ||
						(hashData.flag == SearchAgent::UPPER && hashData.value < beta))) {
			stats.ttHits++;
			return hashData.value;
		}
	}

	const bool isKingAttacked = si.inCheck;
	if (!isKingAttacked) {
		si.eval = evaluator.quickEvaluate(board);
	}

	//razoring
	if (depth < razorDepth && hashData.move.from == NONE &&
			!isKingAttacked && !isMateScore(beta) &&
			!isPawnPromoting(board) && allowNullMove &&
			beta > si.eval + (razorMargin(depth)) ) {
		score = qSearch(board, si, beta-1, beta, 0, ply, pv);
		if (score < beta) {
			return score;
		}
	}

	// null move #1
	if (!isKingAttacked && allowNullMove &&
			depth < razorDepth && okToNullMove(board) &&
			!isMateScore(beta) && si.eval >= beta+futilityMargin(depth)) {
		return si.eval-futilityMargin(depth);
	}

	// null move #2
	if (!isKingAttacked && allowNullMove &&
			okToNullMove(board) && !isMateScore(beta) &&
			si.eval >= beta-(depth>=razorDepth?nullMoveMargin:0)) {

		const int reduction = 3 + (depth > 4 ? depth/4 : 0);
		MoveBackup backup;

		board.doNullMove(backup);
		score = -zwSearch(board, si, 1-beta, depth-reduction, ply+1, &line, false);
		board.undoNullMove(backup);

		if (stop(agent->shouldStop())) {
			return 0;
		}
		if (score >= beta) {
			if (score >= maxScore-maxSearchPly) {
				score = beta;
			}
			agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::NM_LOWER,emptyMove);
			stats.nullMoveHits++;
			return score;
		} else {
			if (score < -maxScore-maxSearchPly) {
				nullMoveMateScore=true;
			}
		}
	}

	//iid
	if (depth > allowIIDAtNormal &&	hashData.move.from == NONE &&
			!isKingAttacked && si.eval >= beta-iidMargin && !nullMoveMateScore) {
		const int iidSearchDepth = depth/2;
		score = zwSearch(board,si,beta,iidSearchDepth,ply,&line,false);
		agent->hashGet(board.getKey(), hashData, ply, maxScore);
	}

	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;
	int moveCounter=0;
	int remainingMoves=0;
	int reduction=0;
	int extension=0;
	int bestScore=-maxScore;

	while (true) {

		MoveIterator::Move& move = selectMove(board, moves, hashData.move, isKingAttacked, ply);

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

		const bool givingCheck = board.isAttacked(board.getSideToMove(),KING);
		SearchInfo newSi(si.eval,givingCheck);

		//futility
		if (okToPrune(board, move, hashData.move,
				isKingAttacked, givingCheck, nullMoveMateScore, depth)) {

			const int futilityScore = si.eval + futilityMargin(depth) + 50;
			if (futilityScore < beta) {
				if (futilityScore>bestScore) {
					bestScore=futilityScore;
				}
				board.undoMove(backup);
				continue;
			}
		}

		if (move.type == MoveIterator::NON_CAPTURE) {
			remainingMoves++;
		}

		bool reduced = adjustDepth(extension, reduction, board, move, depth,remainingMoves,
				isKingAttacked,givingCheck,ply,nullMoveMateScore);
		int newDepth=depth-1+extension;

		score = -zwSearch(board, newSi, 1-beta, newDepth-reduction, ply+1, &line, true);

		if (score >= beta && reduced) {
			score = -zwSearch(board, newSi, 1-beta, newDepth, ply+1, &line, true);
		}

		board.undoMove(backup);

		if (stop(agent->shouldStop())) {
			return 0;
		}

		if (score > bestScore) {
			bestScore=score;
			if( score >= beta) {
				updatePv(pv, line, move);
				updateKillers(board,move,ply);
				updateHistory(board,move,depth);
				agent->hashPut(board,bestScore,depth,ply,maxScore,SearchAgent::LOWER,move);
				stats.ttLower++;
				return bestScore;
			}
		}
	}

	if (bestScore==-maxScore) {
		bestScore=beta-1;
	}

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	agent->hashPut(board,bestScore,depth,ply,maxScore,SearchAgent::UPPER,emptyMove);
	stats.ttUpper++;

	return bestScore;
}

//quiescence search
int SimplePVSearch::qSearch(Board& board, SearchInfo& si, int alpha, int beta, int depth, int ply, PvLine* pv) {

	_nodes++;

	if (stop(agent->shouldStop())) {
		return 0;
	}

	if	(board.isDraw() || ply >= maxSearchPly) {
		return 0;
	}

	int standPat = evaluator.evaluate(board);

	if(standPat>=beta) {
		return beta;
	}

	if( alpha < standPat) {
		alpha = standPat;
	}

	const bool isKingAttacked = si.inCheck;

	int moveCounter=0;
	bool pvNode = bool(beta-alpha != 1);
	PvLine line = PvLine();
	MoveIterator moves = MoveIterator();

	while (true)  {

		MoveIterator::Move& move = selectMove(board,moves,isKingAttacked);
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
		const bool givingCheck = board.isAttacked(board.getSideToMove(),KING);
		SearchInfo newSi(standPat,givingCheck);

		if (!isKingAttacked && !pvNode && depth < 0 &&
				move.type!=MoveIterator::PROMO_CAPTURE &&
				move.type!=MoveIterator::PROMO_NONCAPTURE &&
				!isPawnPromoting(board) &&
				evaluator.see(board,move) < 0) {
			board.undoMove(backup);
			continue;
		}

		int score = -qSearch(board, newSi, -beta, -alpha, depth-1, ply+1, &line);

		board.undoMove(backup);

		if (stop(agent->shouldStop())) {
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
		//int x = evaluator.evaluate(board);
		return 1;
	}

	long time=0;

	if (ply==1) {
		time=getTickCount();
	}

	long nodes=0;
	MoveIterator moves = MoveIterator();
	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);

	/*if (!isKingAttacked) {
		board.generateAllMoves(moves, board.getSideToMove());
	} else {
		board.generateEvasions(moves, board.getSideToMove());
	}*/
	//filterLegalMoves(board,rootMoves);
	while (true/*moves.hasNext()*/)  {

		MoveIterator::Move& move = selectMove(board, moves, emptyMove, isKingAttacked, ply);
		if (moves.end()) {
			break;
		}
		//MoveIterator::Move& move = moves.next();
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
