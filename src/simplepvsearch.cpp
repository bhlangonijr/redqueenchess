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
	rootSearchInfo.eval = evaluator.evaluate(board,beta);
	rootSearchInfo.inCheck = isKingAttacked;

	if (!isKingAttacked) {
		board.generateAllMoves(rootMoves, board.getSideToMove());
	} else {
		board.generateEvasions(rootMoves, board.getSideToMove());
	}

	filterLegalMoves(board,rootMoves);
	rootMoves.clearScore();
	scoreRootMoves(board,rootMoves);
	if (rootMoves.get(0).score > rootMoves.get(1).score + easyMargin ) {
		easyMove=rootMoves.get(0);
	}

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

			if (depth>1) {
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
				remainingMoves,isKingAttacked,givingCheck,ply,false,score > alpha);
		int newDepth=depth-1+extension;
		SearchInfo newSi(si.eval,givingCheck,score>alpha,move);

		if (score>alpha) {
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
		return qSearch(board, si, alpha, beta, 0, ply, pv);
	}

	PvLine line = PvLine();
	const int oldAlpha = alpha;
	int score = -maxScore;
	SearchAgent::HashData hashData;
	MoveIterator::Move hashMove;

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
	if (agent->hashGet(board.getKey(), hashData, ply)) {
		stats.ttHits++;
		hashMove = hashData.move();
	}

	const bool isKingAttacked = si.inCheck;

	//iid
	if (depth > allowIIDAtPV &&	hashMove.none() && !isKingAttacked ) {
		const int iidSearchDepth = depth-2;
		score = pvSearch(board,si,alpha,beta,iidSearchDepth,ply,&line);
		if (agent->hashGet(board.getKey(), hashData, ply)) {
			stats.ttHits++;
			hashMove = hashData.move();
		}
	}

	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;
	int moveCounter=0;
	int remainingMoves=0;
	int reduction=0;
	int extension=0;

	while (true) {

		MoveIterator::Move& move = selectMove(board, moves, hashMove, isKingAttacked, ply);
		if (moves.end()) {
			break;
		}
		MoveBackup backup;
		board.doMove(move,backup);
		assert(move.from!=NONE);

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
		const bool reduced = adjustDepth(extension, reduction, board, move, depth, remainingMoves,
				isKingAttacked,givingCheck,ply,false,moveCounter==1);
		SearchInfo newSi(si.eval,givingCheck,moveCounter==1,move);
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
			agent->hashPut(board.getKey(),score,depth,ply,SearchAgent::LOWER,move);
			stats.ttLower++;
			return score;
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
		agent->hashPut(board.getKey(),alpha,depth,ply,SearchAgent::EXACT,bestMove);
		stats.ttExact++;
	} else {
		agent->hashPut(board.getKey(),alpha,depth,ply,SearchAgent::UPPER,emptyMove);
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
	const bool isKingAttacked = si.inCheck;
	const bool pawnPromoting = isPawnPromoting(board);
	bool nullMoveMateScore=false;
	bool okToPruneWithHash=false;
	int score = 0;
	SearchAgent::HashData hashData;
	MoveIterator::Move hashMove;
	Key key = board.getKey();

	// tt retrieve & prunning
	if (agent->hashPruneGet(okToPruneWithHash, key, hashData, ply, depth, allowNullMove, beta-1, beta)) {
		hashMove = hashData.move();
		stats.ttHits++;
		if (okToPruneWithHash) {
			return hashData.value();
		}
	}

	if (!isKingAttacked) {
		si.eval = evaluator.evaluate(board,beta);
	}

	if (depth < razorDepth && hashMove.none() &&
			!isKingAttacked && !isMateScore(beta) &&
			!pawnPromoting && !si.move.none() &&
			si.eval+razorMargin(depth) < beta) {
		score = qSearch(board, si, beta-1, beta, 0, ply, pv);
		if (score<beta) {
			return score;
		}
	}

	if (!isKingAttacked && allowNullMove && depth < nullMoveDepth &&
			!isPawnFinal(board) && !pawnPromoting && !isMateScore(beta) &&
			si.eval >= beta+nullMoveMargin+futilityMargin(depth)) {
		return si.eval-futilityMargin(depth);
	}

	// null move
	if (depth>1 && !isKingAttacked && allowNullMove &&
			!isPawnFinal(board) && !isMateScore(beta) &&
			si.eval >= beta-(depth>=nullMoveDepth?nullMoveMargin:0)) {

		const int reduction = 3 + (depth > 4 ? depth/6 : 0);
		MoveBackup backup;

		board.doNullMove(backup);
		SearchInfo newSi(si.eval,false,false,emptyMove);
		score = -zwSearch(board, si, 1-beta, depth-reduction, ply+1, &line, false);
		board.undoNullMove(backup);

		if (stop(agent->shouldStop())) {
			return 0;
		}
		if (score >= beta) {
			if (score >= maxScore-maxSearchPly) {
				score = beta;
			}
			agent->hashPut(key,score,depth,ply,SearchAgent::NM_LOWER,emptyMove);
			stats.nullMoveHits++;
			return score;
		} else {
			if (score < -maxScore-maxSearchPly) {
				nullMoveMateScore=true;
			}
		}
	}

	//iid
	if (depth > allowIIDAtNormal &&	hashMove.none() &&
			!isKingAttacked && si.eval >= beta-iidMargin) {
		const int iidSearchDepth = depth/2;
		score = zwSearch(board,si,beta,iidSearchDepth,ply,&line,false);
		if (agent->hashGet(key, hashData, ply)) {
			stats.ttHits++;
			hashMove = hashData.move();
		}
	}

	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;
	int moveCounter=0;
	int remainingMoves=0;
	int reduction=0;
	int extension=0;

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

		//futility
		if  (	move.type == MoveIterator::NON_CAPTURE &&
				depth < futilityDepth &&
				!isKingAttacked &&
				!givingCheck &&
				!isPawnPush(board,move.to) &&
				!pawnPromoting &&
				!nullMoveMateScore) {
			if (moveCountMargin(depth) < moveCounter) {
				board.undoMove(backup);
				continue;
			}
			const int futilityScore = si.eval + futilityMargin(depth);
			if (futilityScore < beta) {
				board.undoMove(backup);
				continue;
			}
		}
		if (move.type == MoveIterator::NON_CAPTURE) {
			remainingMoves++;
		}
		bool reduced = adjustDepth(extension, reduction, board, move, depth,remainingMoves,
				isKingAttacked,givingCheck,ply,nullMoveMateScore,false);
		SearchInfo newSi(si.eval,givingCheck,false,move);
		int newDepth=depth-1+extension;

		score = -zwSearch(board, newSi, 1-beta, newDepth-reduction, ply+1, &line, true);

		if (score >= beta && reduced) {
			score = -zwSearch(board, newSi, 1-beta, newDepth, ply+1, &line, true);
		}

		board.undoMove(backup);

		if (stop(agent->shouldStop())) {
			return 0;
		}

		if( score >= beta) {
			updatePv(pv, line, move);
			updateKillers(board,move,ply);
			updateHistory(board,move,depth);
			agent->hashPut(key,score,depth,ply,SearchAgent::LOWER,move);
			stats.ttLower++;
			return score;
		}

	}

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	agent->hashPut(key,beta-1,depth,ply,SearchAgent::UPPER,emptyMove);
	stats.ttUpper++;

	return beta-1;
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

	SearchAgent::HashData hashData;
	MoveIterator::Move hashMove;
	bool okToPruneWithHash=false;
	bool pvNode = si.isPV;

	// tt retrieve & prunning
	if (agent->hashPruneGet(okToPruneWithHash, board.getKey(), hashData, ply, depth, true, alpha, beta)) {
		hashMove = hashData.move();
		stats.ttHits++;
		if (okToPruneWithHash && !pvNode) {
			return hashData.value();
		}
	}
	const bool isKingAttacked = si.inCheck;
	const bool pawnPromoting = isPawnPromoting(board);
	const int oldAlpha = alpha;
	if (!isKingAttacked) {
		int standPat = evaluator.evaluate(board,beta);
		if(standPat>=beta) {
			return beta;
		}
		if( alpha < standPat && pvNode) {
			alpha = standPat;
		}
		const int delta =  deltaMargin + (pawnPromoting ? deltaMargin : 0);
		if (standPat < alpha - delta && !pvNode) {
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

		if (!isKingAttacked && !pvNode && depth < 0 &&
				move.type==MoveIterator::BAD_CAPTURE &&
				move != hashMove &&
				!pawnPromoting) {
			board.undoMove(backup);
			continue;
		}

		const bool givingCheck = board.isAttacked(board.getSideToMove(),KING);
		SearchInfo newSi(alpha,givingCheck,si.isPV,move);
		int score = -qSearch(board, newSi, -beta, -alpha, depth-1, ply+1, &line);

		board.undoMove(backup);

		if (stop(agent->shouldStop())) {
			return 0;
		}

		if( score >= beta ) {
			agent->hashPut(board.getKey(),score,depth,ply,SearchAgent::LOWER,move);
			return score;
		}

		if( score>alpha ) {
			alpha = score;
			updatePv(pv, line, move);
			bestMove=move;
		}

	}

	if (!moveCounter && isKingAttacked) {
		return -maxScore+ply;
	}

	if (alpha>oldAlpha) {
		agent->hashPut(board.getKey(),alpha,depth,ply,SearchAgent::EXACT,bestMove);
		stats.ttExact++;
	} else {
		agent->hashPut(board.getKey(),alpha,depth,ply,SearchAgent::UPPER,emptyMove);
		stats.ttUpper++;
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
