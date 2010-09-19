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

// root search
void SimplePVSearch::search(Board& _board) {

	agent = SearchAgent::getInstance();
	Board board(_board);
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
	rootSearchInfo.inCheck = isKingAttacked;
	int searchTime;
	uint64_t searchNodes;
	MoveIterator::Move bestMove;
	MoveIterator::Move ponderMove;

	if (!isKingAttacked) {
		board.generateAllMoves(rootMoves, board.getSideToMove());
	} else {
		board.generateEvasions(rootMoves, board.getSideToMove());
	}

	filterLegalMoves(board,rootMoves);
	rootMoves.clearScore();
	scoreRootMoves(board,rootMoves);
	rootMoves.sort();
	if (rootMoves.get(0).score > rootMoves.get(1).score + easyMargin ) {
		easyMove=rootMoves.get(0);
	}

	for (int depth = 1; depth <= _depth; depth++) {

		PvLine pv = PvLine();
		pv.index=0;
		int score = 0;
		maxPlySearched = 0;
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

		while (true) {
			score = rootSearch(board, rootSearchInfo, alpha, beta, depth, 0, &pv);
			if((score <= alpha || score >= beta) &&	!stop(agent->shouldStop())) {
				alpha=-maxScore;
				beta=maxScore;
				continue;
			}
			break;
		}

		iterationScore[depth]=score;
		iterationTime[depth]=getTickCount()-_startTime;

		if (stop(agent->shouldStop()) && depth > 1) {
			break;
		}

		if (score > bestScore) {
			bestScore = score;
		}

		bestMove=pv.moves[0];
		ponderMove=pv.moves[1];
		searchTime=getTickCount()-_startTime;
		searchNodes=_nodes;

		uciOutput(&pv, bestMove, searchTime, agent->hashFull(), depth, maxPlySearched, alpha, beta);

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

			if (depth>1) {
				if (bestMove!=easyMove) {
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

			if (depth>5) {
				if (_timeToSearch <	predictTimeUse(iterationTime,_timeToSearch,depth+1)) {
					break;
				}
			}

		}

	}

	uciOutput(bestMove,ponderMove);

	return bestMove.score;
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

		moveCounter++;

		if (move.type == MoveIterator::NON_CAPTURE) {
			remainingMoves++;
		}

		const bool givingCheck = board.isAttacked(board.getSideToMove(),KING);
		const bool pawnOn7thExtension = isPawnOn7thRank(board,move.to);
		int reduction=0;
		int extension=0;
		if (isKingAttacked || pawnOn7thExtension) {
			extension++;
		} else if (move.type == MoveIterator::NON_CAPTURE && !givingCheck &&
				!isPawnPush(board,move.to) && remainingMoves>lateMoveThreshold1 &&
				depth>lmrDepthThresholdRoot) {
			reduction++;
		}
		int newDepth=depth-1+extension;
		SearchInfo newSi(givingCheck,move);

		if (score>alpha) {
			score = -pvSearch(board, newSi, -beta, -alpha, newDepth, ply+1, &line);
		} else {
			score = -zwSearch(board, newSi, -alpha, newDepth-reduction, ply+1, &line, true);
			if (score>alpha && reduction>0) {
				score = -zwSearch(board, newSi, -alpha, newDepth, ply+1, &line, true);
			}
			if (score>alpha) {
				score = -pvSearch(board, newSi, -beta, -alpha, newDepth, ply+1, &line);
			}
		}

		board.undoMove(backup);

		nodes = _nodes-nodes;
		updateRootMovesScore(nodes);

		if (stop(agent->shouldStop()) && depth>1) {
			return 0;
		}

		if( score >= beta) {
			return score;
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
		return qSearch(board, si, alpha, beta, 0, ply, pv, true);
	}

	if (ply>maxPlySearched) {
		maxPlySearched=ply;
	}

	_nodes++;

	if	(board.isDraw()) {
		return 0;
	}

	if	(ply >= maxSearchPly-1) {
		return evaluator.evaluate(board,alpha,beta);
	}

	PvLine line = PvLine();
	const int oldAlpha = alpha;
	int score = -maxScore;
	SearchAgent::HashData hashData;
	MoveIterator::Move hashMove;
	const Key key = board.getKey();
	alpha = MAX(-maxScore+ply, alpha);
	beta = MIN(maxScore-(ply+1), beta);

	if (alpha>=beta) {
		return alpha;
	}

	//tt retrieve
	if (agent->hashGet(key, hashData, ply)) {
		hashMove = hashData.move();
	}

	const bool isKingAttacked = si.inCheck;

	//iid
	if (depth > allowIIDAtPV &&	hashMove.none()) {
		const int iidSearchDepth = depth-2;
		score = pvSearch(board,si,alpha,beta,iidSearchDepth,ply,&line);
		if (agent->hashGet(key, hashData, ply)) {
			hashMove = hashData.move();
		}
	}

	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;
	int moveCounter=0;
	int remainingMoves=0;
	int bestScore=-maxScore;

	while (true) {

		MoveIterator::Move& move = selectMove(board, moves, hashMove, isKingAttacked, ply);
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

		if (move.promotionPiece!=EMPTY) {
			if (board.getPiecesByType(move.promotionPiece)==ROOK ||
					board.getPiecesByType(move.promotionPiece)==BISHOP) {
				board.undoMove(backup);
				continue; // useless
			}
		}

		if (move.type == MoveIterator::NON_CAPTURE) {
			remainingMoves++;
		}
		const bool givingCheck = board.isAttacked(board.getSideToMove(),KING);
		const bool pawnOn7thExtension = isPawnOn7thRank(board,move.to);
		int reduction=0;
		int extension=0;
		if (isKingAttacked || pawnOn7thExtension) {
			extension++;
		} else if (move.type == MoveIterator::NON_CAPTURE && !givingCheck &&
				!isPawnPush(board,move.to) && remainingMoves>lateMoveThreshold1 &&
				depth>lmrDepthThreshold1) {
			reduction++;
		}
		SearchInfo newSi(givingCheck,move);
		int newDepth=depth-1+extension;

		if (moveCounter==1) {
			score = -pvSearch(board, newSi, -beta, -alpha, newDepth, ply+1, &line);
		} else {
			score = -zwSearch(board, newSi, -alpha, newDepth-reduction, ply+1, &line, true);
			if (score>alpha && reduction) {
				score = -zwSearch(board, newSi, -alpha, newDepth, ply+1, &line, true);
			}
			if (score > alpha && score < beta) {
				score = -pvSearch(board, newSi, -beta, -alpha, newDepth, ply+1, &line);
			}
		}

		board.undoMove(backup);

		if (stop(agent->shouldStop())) {
			return 0;
		}

		if( score >= beta) {
			updateKillers(board,move,ply);
			updateHistory(board,move,depth);
			agent->hashPut(key,score,depth,ply,SearchAgent::LOWER,move);
			return score;
		}

		if (score>bestScore) {
			bestScore=score;
			if( score > alpha ) {
				alpha = score;
				updatePv(pv, line, move);
				bestMove=move;
			}
		}

	}

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	if (bestScore>oldAlpha) {
		agent->hashPut(key,bestScore,depth,ply,SearchAgent::EXACT,bestMove);
	} else {
		agent->hashPut(key,bestScore,depth,ply,SearchAgent::UPPER,emptyMove);
	}

	return bestScore;

}

// zero window search - non pv nodes
int SimplePVSearch::zwSearch(Board& board, SearchInfo& si, int beta, int depth, int ply, PvLine* pv, const bool allowNullMove) {

	if (stop(agent->shouldStop())) {
		return 0;
	}

	if (depth<=0) {
		return qSearch(board, si, beta-1, beta, 0, ply, pv, false);
	}

	if (ply>maxPlySearched) {
		maxPlySearched=ply;
	}

	_nodes++;

	if	(board.isDraw()) {
		return 0;
	}

	if	(ply >= maxSearchPly-1) {
		return evaluator.evaluate(board,beta-1,beta);
	}

	if (-maxScore+ply >= beta) {
		return beta;
	}

	if (maxScore-(ply+1) < beta) {
		return beta-1;
	}

	PvLine line = PvLine();
	const bool isKingAttacked = si.inCheck;
	bool nullMoveMateScore=false;
	bool okToPruneWithHash=false;
	int score = 0;
	int currentScore = -maxScore;
	SearchAgent::HashData hashData;
	MoveIterator::Move hashMove;
	const Key key = board.getKey();

	// tt retrieve & prunning
	if (agent->hashPruneGet(okToPruneWithHash, key, hashData, ply, depth, allowNullMove, beta-1, beta)) {
		hashMove = hashData.move();
		if (okToPruneWithHash) {
			return hashData.value();
		}
	}

	if (!isKingAttacked) {
		currentScore = evaluator.evaluate(board,beta-1,beta);
	}

	if (depth < razorDepth && hashMove.none() &&
			!isKingAttacked && !isMateScore(beta) &&
			!isPawnPromoting(board) && !si.move.none() &&
			currentScore < beta - razorMargin(depth)) {
		const int newBeta = beta - razorMargin(depth);
		score = qSearch(board, si, newBeta-1, newBeta, 0, ply, pv, false);
		if (score < newBeta) {
			return score;
		}
	}

	if (!isKingAttacked && allowNullMove && depth < nullMoveDepth &&
			!isPawnFinal(board) && !isMateScore(beta) &&
			currentScore >= beta+futilityMargin(depth)) {
		return currentScore-futilityMargin(depth);
	}

	// null move
	if (depth>1 && !isKingAttacked && allowNullMove &&
			!isPawnFinal(board) && !isMateScore(beta) &&
			currentScore >= beta-(depth>=nullMoveDepth?nullMoveMargin:0)) {

		const int reduction = 3 + (depth > 4 ? depth/6 : 0);
		MoveBackup backup;

		board.doNullMove(backup);
		SearchInfo newSi(false,emptyMove);
		score = -zwSearch(board, newSi, 1-beta, depth-reduction, ply+1, &line, false);
		board.undoNullMove(backup);

		if (stop(agent->shouldStop())) {
			return 0;
		}
		if (score >= beta) {
			if (score >= maxScore-maxSearchPly) {
				score = beta;
			}
			agent->hashPut(key,score,depth,ply,SearchAgent::NM_LOWER,emptyMove);
			return score;
		} else {
			if (score < -maxScore-maxSearchPly) {
				nullMoveMateScore=true;
			}
		}
	}

	//iid
	if (depth > allowIIDAtNormal &&	hashMove.none() &&
			!isKingAttacked && currentScore >= beta-iidMargin) {
		const int iidSearchDepth = depth/2;
		score = zwSearch(board,si,beta,iidSearchDepth,ply,&line,false);
		if (agent->hashGet(key, hashData, ply)) {
			hashMove = hashData.move();
		}
	}

	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;
	int moveCounter=0;
	int remainingMoves=0;
	int bestScore=-maxScore;

	while (true) {
		MoveIterator::Move& move = selectMove(board, moves, hashMove, isKingAttacked, ply);
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

		if (move.promotionPiece!=EMPTY) {
			if (board.getPiecesByType(move.promotionPiece)==ROOK ||
					board.getPiecesByType(move.promotionPiece)==BISHOP) {
				board.undoMove(backup);
				continue; // useless
			}
		}

		const bool givingCheck = board.isAttacked(board.getSideToMove(),KING);
		const bool passedPawn = isPawnPush(board,move.to);
		const bool pawnOn7thExtension = isPawnOn7thRank(board,move.to);

		//futility
		if  (	move.type == MoveIterator::NON_CAPTURE &&
				depth < futilityDepth &&
				!isKingAttacked &&
				!givingCheck &&
				!passedPawn &&
				!isPawnPromoting(board) &&
				!nullMoveMateScore) {
			if (moveCountMargin(depth) < moveCounter  && !isMateScore(bestScore) ) {
				board.undoMove(backup);
				continue;
			}
			const int futilityScore = currentScore + futilityMargin(depth);
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

		//reductions
		int reduction=0;
		int extension=0;
		if (isKingAttacked || pawnOn7thExtension){
			extension=1;
		} else if (move.type == MoveIterator::NON_CAPTURE && !givingCheck &&
				!passedPawn && !nullMoveMateScore &&
				remainingMoves>lateMoveThreshold1 && depth>lmrDepthThreshold1) {
			reduction++;
			if (remainingMoves>lateMoveThreshold2 && depth>lmrDepthThreshold2 &&
					!history[board.getPieceBySquare(move.to)][move.to]) {
				reduction+=depth/8;
			}
		}
		SearchInfo newSi(givingCheck,move);
		int newDepth=depth-1+extension;

		score = -zwSearch(board, newSi, 1-beta, newDepth-reduction, ply+1, &line, true);

		if (score >= beta && reduction>0) {
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
				agent->hashPut(key,bestScore,depth,ply,SearchAgent::LOWER,move);
				return bestScore;
			}
		}

	}

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	agent->hashPut(key,bestScore,depth,ply,SearchAgent::UPPER,emptyMove);

	return bestScore;
}

//quiescence search
int SimplePVSearch::qSearch(Board& board, SearchInfo& si,
		int alpha, int beta, int depth, int ply, PvLine* pv, const bool isPV) {

	_nodes++;

	if (ply>maxPlySearched) {
		maxPlySearched=ply;
	}

	if (stop(agent->shouldStop())) {
		return 0;
	}

	if	(board.isDraw()) {
		return 0;
	}

	if	(ply >= maxSearchPly-1) {
		return evaluator.evaluate(board,alpha,beta);
	}

	SearchAgent::HashData hashData;
	MoveIterator::Move hashMove;
	const Key key = board.getKey();
	bool okToPruneWithHash=false;
	const bool oldAlpha=alpha;

	// tt retrieve & prunning
	if (agent->hashPruneGet(okToPruneWithHash, key, hashData, ply, depth, true, alpha, beta)) {
		hashMove = hashData.move();
		if (okToPruneWithHash && !isPV) {
			return hashData.value();
		}
	}
	const bool isKingAttacked = si.inCheck;
	int bestScore = isKingAttacked?-maxScore:evaluator.evaluate(board,alpha,beta);
	if (!isKingAttacked) {
		if(bestScore>=beta) {
			return beta;
		}
		if( alpha < bestScore) {
			alpha = bestScore;
		}

		const int delta =  deltaMargin + (isPawnPromoting(board) ? deltaMargin : 0);
		if (bestScore < alpha - delta && !isPV) {
			return alpha;
		}
	}

	int moveCounter=0;
	PvLine line = PvLine();
	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;

	while (true)  {

		MoveIterator::Move& move = selectMove(board,moves,isKingAttacked,hashMove);
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

		if (!isKingAttacked && !isPV && depth < 0 &&
				move.type==MoveIterator::BAD_CAPTURE &&
				move != hashMove &&
				!isPawnPromoting(board)) {
			board.undoMove(backup);
			continue;
		}

		const bool givingCheck = board.isAttacked(board.getSideToMove(),KING);
		SearchInfo newSi(givingCheck,move);

		int score = -qSearch(board, newSi, -beta, -alpha, depth-1, ply+1, &line, isPV);

		board.undoMove(backup);

		if (stop(agent->shouldStop())) {
			return 0;
		}

		if( score >= beta ) {
			agent->hashPut(key,score,depth!=0?-1:0,ply,SearchAgent::LOWER,move);
			return score;
		}

		if (score>bestScore) {
			bestScore=score;
			if( score>alpha ) {
				alpha = score;
				updatePv(pv, line, move);
				bestMove=move;
			}
		}

	}

	if (!moveCounter && isKingAttacked) {
		return -maxScore+ply;
	}

	if (bestScore>oldAlpha) {
		agent->hashPut(key,bestScore,depth!=0?-1:0,ply,SearchAgent::EXACT,bestMove);
	} else {
		agent->hashPut(key,bestScore,depth!=0?-1:0,ply,SearchAgent::UPPER,emptyMove);
	}

	return bestScore;
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
