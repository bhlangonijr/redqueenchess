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
	evaluator.setGameStage(evaluator.predictGameStage(board));
	_startTime = getTickCount();
	timeToStop = clock() + toClock(_timeToSearch);
	_score = idSearch(board);
	_time = getTickCount() - _startTime;
}

// get current score
int SimplePVSearch::getScore() {
	return this->_score;
}

// iterative deepening
int SimplePVSearch::idSearch(Board& board) {

	_nodes = 0;
	int bestScore = -maxScore;
	int iterationScore[maxSearchPly];
	long iterationTime[maxSearchPly];
	int alpha = -maxScore;
	int beta = maxScore;
	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);
	MoveIterator::Move easyMove;
	initRootMovesOrder();
	rootMoves.clear();

	if (!isKingAttacked) {
		board.generateAllMoves(rootMoves, board.getSideToMove());
	} else {
		board.generateEvasions(rootMoves, board.getSideToMove());
	}

	filterLegalMoves(board,rootMoves);

	for (int depth = 1; depth <= _depth; depth++) {

		PvLine pv = PvLine();
		pv.index=0;
		int score = 0;

		score = rootSearch(board, alpha, beta, depth, 0, &pv);

		if(score <= alpha || score >= beta) {
			alpha=-maxScore;
			beta=maxScore;
			score = rootSearch(board, alpha, beta, depth, 0, &pv);
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

		if (depth >= aspirationDepth)	{ // like in stockfish
			int delta1 = iterationScore[depth-0]-iterationScore[depth-1];
			int delta2 = iterationScore[depth-1]-iterationScore[depth-2];
			int aspirationDelta = MAX(abs(delta1)+abs(delta2)/2, 16)+7;
			alpha = MAX(iterationScore[depth]-aspirationDelta,-maxScore);
			beta  = MIN(iterationScore[depth]+aspirationDelta,+maxScore);
		}

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

			if (depth>3) {
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
int SimplePVSearch::rootSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv) {

	PvLine line = PvLine();
	int score = -maxScore;
	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);


	if (depth < 3) {
		if (depth==1) {
			scoreMoves(board,rootMoves,true);
		}
		rootMoves.sort(nodesPerMove);
	} else {
		rootMoves.sortOrderingBy(nodesPerMove);
	}

	rootMoves.first();
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

		const bool givingCheck = isGivenCheck(board,move.to);
		bool reduced = adjustDepth(extension, reduction, board, move, depth,
				remainingMoves,isKingAttacked,givingCheck,ply,false,score > alpha);
		int newDepth=depth-1+extension;

		if (score > alpha) {
			score = -pvSearch(board, -beta, -alpha, newDepth-reduction, ply+1, &line);
		} else {
			reduced=true;
			score = -zwSearch(board, -alpha, newDepth-reduction, ply+1, &line, true);
		}
		if (score > alpha && reduced) {
			score = -pvSearch(board, -beta, -alpha, newDepth, ply+1, &line);
		}

		board.undoMove(backup);

		move.score=score;
		nodes = _nodes - nodes;
		updateRootMovesScore(nodes);

		if( score >= beta) {
			if (!stop(agent->shouldStop())) {
				uciOutput(pv, move, getTickCount()-_startTime, agent->hashFull(), depth, alpha, beta);
			}
			return beta;
		}

		if( score > alpha ) {
			alpha = score;
			bestMove=move;
			updatePv(pv, line, bestMove);
			if (!stop(agent->shouldStop())) {
				uciOutput(pv, move, getTickCount()-_startTime, agent->hashFull(), depth, alpha, beta);
			}
		}

		uciOutput(move, moveCounter);

	}

	rootMoves.sort(nodesPerMove);

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	return alpha;

}

// principal variation search
int SimplePVSearch::pvSearch(Board& board, int alpha, int beta,	int depth, int ply, PvLine* pv) {

	if (stop(agent->shouldStop())) {
		return 0;
	}

	if (depth<=0) {
		return qSearch(board, alpha, beta, 0, ply, pv);
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

	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);

	//iid
	if (depth > allowIIDAtPV &&	hashData.move.from == NONE && !isKingAttacked ) {
		const int iidSearchDepth = depth-2;
		score = pvSearch(board,alpha,beta,iidSearchDepth,ply,&line);
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

		const bool givingCheck = isGivenCheck(board,move.to);

		const bool reduced = adjustDepth(extension, reduction, board, move, depth, remainingMoves,
				isKingAttacked,givingCheck,ply,false,moveCounter==1);
		int newDepth=depth-1;
		bool fullSearch=false;

		if (moveCounter==1) {
			score = -pvSearch(board, -beta, -alpha, newDepth-reduction+extension, ply+1, &line);
			fullSearch=(score > alpha && reduced);
		} else {
			score = -zwSearch(board, -alpha, newDepth-reduction+extension, ply+1, &line, true);
			fullSearch=(score > alpha && score < beta);
		}

		if (fullSearch) {
			score = -pvSearch(board, -beta, -alpha, newDepth+extension, ply+1, &line);
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
int SimplePVSearch::zwSearch(Board& board, int beta, int depth, int ply, PvLine* pv, const bool allowNullMove) {

	if (stop(agent->shouldStop())) {
		return 0;
	}

	if (depth<=0) {
		return qSearch(board, beta-1, beta, 0, ply, pv);
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
		if ((hashData.depth>=depth ||
				(hashData.value >= MAX(maxScore-ply,beta)) ||
				(hashData.value < MIN(-maxScore+ply,beta))) &&
				(allowNullMove || hashData.flag != SearchAgent::NM_LOWER) &&
				((hashData.flag == SearchAgent::LOWER && hashData.value >= beta) ||
						(hashData.flag == SearchAgent::UPPER && hashData.value < beta))) {
			stats.ttHits++;
			return hashData.value;
		}
	}

	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);
	int eval = beta;

	if (!isKingAttacked) {
		eval=evaluator.evaluate(board);
	}

	//razoring
	if (depth < razorDepth && hashData.move.from == NONE &&
			!isKingAttacked && !isMateScore(beta) &&
			!isPawnPromoting(board) &&
			eval < beta - (razorMargin+16*depth)) {
		const int newBeta = beta - (razorMargin+16*depth);
		score = qSearch(board, newBeta-1, newBeta, 0, ply, pv);
		if (score < newBeta) {
			return score;
		}
	}

	// null move #1
	if (!isKingAttacked && allowNullMove &&
			depth < razorDepth && okToNullMove(board) &&
			!isMateScore(beta) && eval >= beta+futilityMargin(depth)) {
		return eval-futilityMargin(depth);
	}

	// null move #2
	if (!isKingAttacked && allowNullMove &&
			okToNullMove(board) && !isMateScore(beta) &&
			eval >= beta-(depth>=razorDepth?nullMoveMargin:0)) {

		const int reduction = 3 + (depth > 4 ? depth/4 : 0);
		MoveBackup backup;
		board.doNullMove(backup);
		score = -zwSearch(board, 1-beta, depth-reduction, ply+1, &line, false);
		board.undoNullMove(backup);

		if (stop(agent->shouldStop())) {
			return 0;
		}

		if (score >= beta) {
			if (score < -maxScore+maxSearchPly) {
				nullMoveMateScore=true;
			}
			if (score >= maxScore-maxSearchPly) {
				score = beta;
			}
			agent->hashPut(board,score,depth,ply,maxScore,SearchAgent::NM_LOWER,emptyMove);
			stats.nullMoveHits++;
			return score;
		}

	}

	//iid
	if (depth > allowIIDAtNormal &&	hashData.move.from == NONE &&
			!isKingAttacked && eval >= beta-iidMargin) {
		const int iidSearchDepth = depth/2;
		score = zwSearch(board,beta,iidSearchDepth,ply,&line,false);
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

		const bool givingCheck = isGivenCheck(board,move.to);
		//futility
		if (depth < futilityDepth && !isKingAttacked &&
				!isMateScore(beta) && move != hashData.move &&
				!givingCheck && !isPawnPush(board,move) &&
				(move.type == MoveIterator::NON_CAPTURE ||
						(move.type == MoveIterator::BAD_CAPTURE &&
								evaluator.see(board,move)<0) )) {

			const int gain = evaluator.getPieceSquareValue(board.getPieceBySquare(move.from),move.to)-
					evaluator.getPieceSquareValue(board.getPieceBySquare(move.from),move.from);

			const int futilityScore = eval + gain + futilityMargin(depth) + 50;

			if (futilityScore < beta) {
				board.undoMove(backup);
				if (futilityScore>bestScore) {
					bestScore=futilityScore;
				}
				continue;
			}
		}

		if (move.type == MoveIterator::NON_CAPTURE) {
			remainingMoves++;
		}

		bool reduced = adjustDepth(extension, reduction, board, move, depth,remainingMoves,
				isKingAttacked,givingCheck,ply,nullMoveMateScore,false);
		int newDepth=depth-1+extension;

		score = -zwSearch(board, 1-beta, newDepth-reduction, ply+1, &line, true);

		if (score >= beta && reduced) {
			score = -zwSearch(board, 1-beta, newDepth, ply+1, &line, true);
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

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : 0;
	}

	if (bestScore==-maxScore) {
		bestScore=beta-1;
	}

	agent->hashPut(board,bestScore,depth,ply,maxScore,SearchAgent::UPPER,emptyMove);
	stats.ttUpper++;

	return bestScore;
}

//quiescence search
int SimplePVSearch::qSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv) {

	_nodes++;

	if (stop(agent->shouldStop())) {
		return 0;
	}

	if	(board.isDraw()) {
		return 0;
	}

	int standPat = evaluator.evaluate(board);

	if(standPat>=beta) {
		return beta;
	}

	if	(ply >= maxSearchPly) {
		return standPat;
	}

	if( alpha < standPat) {
		alpha = standPat;
	}

	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);

	int moveCounter=0;
	bool pvNode = bool(beta - alpha != 1);
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
		if (!isKingAttacked && !pvNode && depth < 0 &&
				move.type!=MoveIterator::PROMO_CAPTURE &&
				move.type!=MoveIterator::PROMO_NONCAPTURE &&
				!isPawnPromoting(board) &&
#if (!USE_SEE_ORDERING)
				evaluator.see(board,move) < 0
#else
				move.score < 0
#endif
		) {
			board.undoMove(backup);
			continue;
		}

		int score = -qSearch(board, -beta, -alpha, depth-1, ply+1, &line);

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
		return 1;
	}

	long time=0;

	if (ply==1) {
		time=getTickCount();
	}

	long nodes=0;
	MoveIterator moves = MoveIterator();
	const bool isKingAttacked = board.isAttacked(board.getSideToMove(),KING);

	if (!isKingAttacked) {
		board.generateAllMoves(moves, board.getSideToMove());
	} else {
		board.generateEvasions(moves, board.getSideToMove());
	}
	//filterLegalMoves(board,rootMoves);
	while (moves.hasNext())  {

		/*MoveIterator::Move& move = selectMove(board, moves, emptyMove, isKingAttacked, ply);
		if (moves.end()) {
			break;
		}*/
		MoveIterator::Move& move = moves.next();
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
