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
static int reductionTablePV[maxSearchDepth+1][maxMoveCount];
static int reductionTableNonPV[maxSearchDepth+1][maxMoveCount];

// root search
void SimplePVSearch::search(Board board) {

	clearHistory();
	initRootMovesOrder();
	rootMoves.clear();
	nodes = 0;
	startTime = getTickCount();
	setTimeToStop();
	nodesToGo = getTimeToSearch()<=1000?fastNodesToGo:defaultNodesToGo;
	searchScore = idSearch(board);
	time = getTickCount()-startTime;
}

// get current score
int SimplePVSearch::getScore() {
	return searchScore;
}

// iterative deepening
int SimplePVSearch::idSearch(Board& board) {

	int bestScore = -maxScore;
	int iterationScore[maxSearchPly];
	long iterationTime[maxSearchPly];
	int alpha = -maxScore;
	int beta = maxScore;
	const bool isKingAttacked = board.setInCheck(board.getSideToMove());
	MoveIterator::Move easyMove;
	rootSearchInfo.allowNullMove = false;
	int searchTime;
	int lastDepth=0;
	uint64_t searchNodes;
	MoveIterator::Move bestMove;
	MoveIterator::Move ponderMove;
	aspirationDelta=0;
	PvLine pvLine = PvLine();

	if (!isKingAttacked) {
		board.generateAllMoves(rootMoves, board.getSideToMove());
	} else {
		board.generateEvasions(rootMoves, board.getSideToMove());
	}

	filterLegalMoves(board,rootMoves);
	scoreRootMoves(board,rootMoves);
	rootMoves.sort();
	if (rootMoves.get(0).score > rootMoves.get(1).score + easyMargin ) {
		easyMove=rootMoves.get(0);
	}

	for (int depth = 1; depth <= depthToSearch; depth++) {

		PvLine pv = PvLine();
		pv.index=0;
		int score = 0;
		maxPlySearched = 0;
		lastDepth=depth;

		// like in stockfish
		if (depth >= aspirationDepth && abs(iterationScore[depth-1]) < winningScore)	{
			int delta1 = iterationScore[depth-1]-iterationScore[depth-2];
			int delta2 = iterationScore[depth-2]-iterationScore[depth-3];
			aspirationDelta = std::max(abs(delta1)+abs(delta2)/2, 16)+7;
			alpha = std::max(iterationScore[depth-1]-aspirationDelta,-maxScore);
			beta  = std::min(iterationScore[depth-1]+aspirationDelta,+maxScore);
		}

		score = rootSearch(board, rootSearchInfo, &alpha, &beta, depth, 0, pv);

		iterationScore[depth]=score;
		iterationTime[depth]=getTickCount()-startTime;

		if (stop() && depth > 1) {
			break;
		}

		bestMove=pv.moves[0];
		ponderMove=pv.moves[1];
		searchTime=getTickCount()-startTime;
		searchNodes=nodes;
		pvLine.copy(pv);

		if (score > bestScore) {
			bestScore = score;
		}

		int repetition=0;

		for (int x=depth-1;x>=1;x--) {
			if (score==iterationScore[x]) {
				repetition++;
			} else {
				break;
			}
		}

		if (!(searchFixedDepth || infinite)) {

			if (depth>5 && (abs(iterationScore[depth - 0]) >= maxScore-maxSearchPly ||
					abs(iterationScore[depth - 1]) >= maxScore-maxSearchPly ||
					abs(iterationScore[depth - 2]) >= maxScore-maxSearchPly)) {
				break;
			}

			if (repetition >= maxScoreRepetition &&
					iterationScore[depth - 0] == drawScore) {
				break;
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
				if (!easyMove.none() && easyMove==bestMove && nodesPerMove[0]>=(nodes*85)/100 &&
						iterationTime[depth] > timeToSearch/2) {
					break;
				}
				if (timeToSearch <	predictTimeUse(iterationTime,timeToSearch,depth+1) &&
						iterationTime[depth] > (timeToSearch*80)/100) {
					break;
				}
			}

		}

	}

	if (bestMove.none()) {
		bestMove=rootMoves.get(0);
		ponderMove=emptyMove;
	}

	uciOutput(pvLine, bestMove.score, getTickCount()-startTime, agent->hashFull(), lastDepth, maxPlySearched, alpha, beta);

	uciOutput(bestMove,ponderMove);

	return bestMove.score;
}


// root search
int SimplePVSearch::rootSearch(Board& board, SearchInfo& si, int* alphaRoot, int* betaRoot, int depth, int ply, PvLine& pv) {

	const bool isKingAttacked = board.isInCheck();
	int alpha=*alphaRoot;
	int beta=*betaRoot;
	int countFH=0;
	int countFL=0;
	int moveCounter=0;
	MoveIterator::Move bestMove;

	while (true) {

		rootMoves.sortOrderingBy(nodesPerMove);
		rootMoves.first();
		moveCounter=0;
		int score = -maxScore;

		while (rootMoves.hasNext()) {

			MoveIterator::Move& move = rootMoves.next();
			moveCounter++;
			uciOutput(move, moveCounter);

			while (true) {

				MoveBackup backup;
				long nodes=this->nodes;
				board.doMove(move,backup);

				const bool givingCheck = board.setInCheck(board.getSideToMove());
				const bool pawnOn7thExtension = move.promotionPiece!=EMPTY;

				int extension=0;
				if (isKingAttacked || pawnOn7thExtension) {
					extension++;
				}

				int newDepth=depth-1+extension;
				SearchInfo newSi(true,move);

				if (score>alpha || moveCounter==1) {
					score = -pvSearch(board, newSi, -beta, -alpha, newDepth, ply+1);
				} else {
					int reduction=0;
					if (!extension && !givingCheck && !isPawnPush(board,move.to) &&
							moveCounter>lateMoveThreshold && depth>lmrDepthThresholdRoot &&
							move.type == MoveIterator::NON_CAPTURE) {
						reduction++;
					}

					score = -zwSearch(board, newSi, -alpha, newDepth-reduction, ply+1);

					if (score>alpha) {
						score = -pvSearch(board, newSi, -beta, -alpha, newDepth, ply+1);
					}
				}

				board.undoMove(backup);
				nodes = this->nodes-nodes;
				updateRootMovesScore(nodes);

				if(score<beta || (stop() && ply)) {
					break;
				}

				move.score=score;
				bestMove=move;
				pv.moves[0]=bestMove;
				retrievePvFromHash(board, pv);
				uciOutput(pv, bestMove.score, getTickCount()-startTime, agent->hashFull(), depth, maxPlySearched, alpha, beta);
				beta =  std::min(beta+aspirationDelta*(1<<countFH),maxScore);
				*betaRoot = beta;
				countFH++;
			}

			if( score > alpha ) {
				alpha = score;
				move.score=score;
				bestMove=move;
				pv.moves[0]=bestMove;
				retrievePvFromHash(board, pv);
				uciOutput(pv, bestMove.score, getTickCount()-startTime, agent->hashFull(), depth, maxPlySearched, alpha, beta);
			} else {
				move.score=-maxScore;
			}

		}
		uciOutput(depth);
		if (*alphaRoot!=alpha || (stop() && ply)) {
			break;
		}

		alpha = std::max(alpha-aspirationDelta*(1<<countFL),-maxScore);
		*alphaRoot = alpha;
		countFL++;
	}

	rootMoves.sortOrderingBy(nodesPerMove);
	rootMoves.first();

	if (!moveCounter) {
		return isKingAttacked ? -maxScore+ply : drawScore;
	}

	return alpha;

}

// principal variation search
int SimplePVSearch::pvSearch(Board& board, SearchInfo& si, int alpha, int beta,	int depth, int ply) {

	if (stop()) {
		return 0;
	}

	if (ply>maxPlySearched) {
		maxPlySearched=ply;
	}

	if (depth<=0) {
		return qSearch(board, si, alpha, beta, 0, ply, true);
	}

	if	(board.isDraw()) {
		return drawScore;
	}

	if	(ply >= maxSearchPly-1) {
		return evaluator.evaluate(board,alpha,beta);
	}

	const int oldAlpha = alpha;
	int score = -maxScore;
	TranspositionTable::HashData hashData;
	MoveIterator::Move hashMove;
	const Key key = si.partialSearch?board.getPartialSearchKey():board.getKey();
	alpha = std::max(-maxScore+ply, alpha);
	beta = std::min(maxScore-(ply+1), beta);

	if (alpha>=beta) {
		return alpha;
	}

	//tt retrieve
	bool hashOk = agent->hashGet(key, hashData, ply);
	if (hashOk) {
		hashMove = hashData.move();
	}

	const bool isKingAttacked = board.isInCheck();

	//iid
	if (depth > allowIIDAtPV &&	hashMove.none() && !isKingAttacked) {
		SearchInfo newSi(false,emptyMove);
		score = pvSearch(board,newSi,alpha,beta,depth-2,ply);
		hashOk = agent->hashGet(key, hashData, ply);
		if (hashOk) {
			hashMove = hashData.move();
		}
	}

	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;
	int moveCounter=0;
	int bestScore=-maxScore;
	bool isSingularMove = false;

	// se
	if (depth > sePVDepth && hashOk && !hashMove.none() && !si.partialSearch &&
			hashData.depth() >= depth-3 && (hashData.flag() & TranspositionTable::LOWER)) {
		if (abs(hashData.value()) < winningScore && board.isMoveLegal<true>(hashMove)) {
			SearchInfo seSi(false,hashMove,true);
			const int seValue = hashData.value() - seMargin;
			const int partialScore = zwSearch(board,seSi,seValue,depth/2,ply);
			if (partialScore < seValue) {
				isSingularMove = true;
			}
		}
	}

	while (true) {

		MoveIterator::Move& move = selectMove<false>(board, moves, hashMove, ply, depth);
		if (move.none()) {
			break;
		}
		if (si.partialSearch && move == si.move) {
			continue;
		}
		MoveBackup backup;
		board.doMove(move,backup);

		moveCounter++;
		nodes++;

		const bool givingCheck = board.setInCheck(board.getSideToMove());
		const bool pawnOn7thExtension = move.promotionPiece!=EMPTY;

		int extension=0;
		if (isKingAttacked || pawnOn7thExtension ||
				(isSingularMove && move==hashMove)) {
			extension++;
		}

		SearchInfo newSi(true,move);
		int newDepth=depth-1+extension;

		if (moveCounter==1) {
			score = -pvSearch(board, newSi, -beta, -alpha, newDepth, ply+1);
		} else {
			int reduction=0;
			if (depth>lmrDepthThreshold && !extension && !givingCheck &&
					!isPawnPush(board,move.to) && move.type == MoveIterator::NON_CAPTURE) {
				reduction=reductionTablePV[depth][moveCounter];
			}

			score = -zwSearch(board, newSi, -alpha, newDepth-reduction, ply+1);

			if (score > alpha && score < beta) {
				if (reduction>0) {
					bool research=true;
					if (reduction>2) {
						score = -zwSearch(board, newSi, 1-beta, newDepth-1, ply+1);
						research=(score >= beta);
					}
					if (research) {
						score = -zwSearch(board, newSi, 1-beta, newDepth, ply+1);
					}
				}
				if (score > alpha && score < beta) {
					score = -pvSearch(board, newSi, -beta, -alpha, newDepth, ply+1);
				}
			}
		}

		board.undoMove(backup);

		if (stop()) {
			return 0;
		}

		if( score >= beta) {
			updateKillers(board,move,ply);
			updateHistory(board,move,depth);
			const TranspositionTable::NodeFlag flag = TranspositionTable::LOWER;
			agent->hashPut(key,score,0,depth,ply,flag,move);
			return score;
		}

		if (score>bestScore) {
			bestScore=score;
			if( score > alpha ) {
				alpha = score;
				bestMove=move;
			}
		}

	}

	if (!moveCounter) {
		return si.partialSearch?oldAlpha:isKingAttacked?-maxScore+ply:drawScore;
	}

	TranspositionTable::NodeFlag flag;
	if (bestScore>oldAlpha) {
		flag = TranspositionTable::EXACT;
	} else {
		flag = TranspositionTable::UPPER;
		bestMove=emptyMove;
	}
	agent->hashPut(key,bestScore,0,depth,ply,flag,bestMove);

	return bestScore;

}

// zero window search - non pv nodes
int SimplePVSearch::zwSearch(Board& board, SearchInfo& si, int beta, int depth, int ply) {

	if (stop()) {
		return 0;
	}

	if (depth<=0) {
		return qSearch(board, si, beta-1, beta, 0, ply, false);
	}

	if	(board.isDraw()) {
		return drawScore;
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

	const bool isKingAttacked = board.isInCheck();
	bool nmMateScore=false;
	bool okToPrune=false;
	int score = 0;
	int currentScore = -maxScore;
	TranspositionTable::HashData hashData;
	MoveIterator::Move hashMove;
	const Key key = si.partialSearch?board.getPartialSearchKey():board.getKey();

	// tt retrieve & prunning
	bool hashOk = agent->hashPruneGet(okToPrune, key, hashData, ply, depth, si.allowNullMove, beta-1, beta);
	if (hashOk) {
		hashMove = hashData.move();
		if (okToPrune) {
			return hashData.value();
		}
	}

	if (!isKingAttacked) {
		if (hashOk && (hashData.flag() & TranspositionTable::NODE_EVAL)) {
			currentScore = hashData.evalValue();
		} else {
			currentScore = evaluator.evaluate(board,beta-1,beta);
		}
	}

	if (depth < razorDepth && hashMove.none() && si.allowNullMove &&
			!isKingAttacked && !isMateScore(beta) &&
			!isPawnPromoting(board) && !si.move.none() &&
			currentScore < beta - razorMargin(depth)) {
		const int newBeta = beta - razorMargin(depth);
		score = qSearch(board, si, newBeta-1, newBeta, 0, ply, false);
		if (score < newBeta) {
			return score;
		}
	}

	if (!isKingAttacked && si.allowNullMove &&
			depth < nullMoveDepth && !isPawnFinal(board) &&
			!isMateScore(beta) && !si.move.none() &&
			currentScore >= beta+futilityMargin(depth)) {
		return currentScore-futilityMargin(depth);
	}

	// null move
	if (depth>1 && !isKingAttacked && si.allowNullMove &&
			!isPawnFinal(board) && !isMateScore(beta) &&
			currentScore >= beta-(depth>=nullMoveDepth?nullMoveMargin:0)) {

		const int reduction = 3 + (depth > 4 ? depth/6 : 0);
		MoveBackup backup;

		board.doNullMove(backup);
		SearchInfo newSi(false,emptyMove);
		score = -zwSearch(board, newSi, 1-beta, depth-reduction, ply+1);
		board.undoNullMove(backup);

		if (stop()) {
			return 0;
		}
		if (score >= beta) {
			if (score >= maxScore-maxSearchPly) {
				score = beta;
			}
			bool okToPrune = true;
			if (depth>6) {
				const int newScore = zwSearch(board, newSi, beta, depth-reduction, ply);
				if (newScore<beta) {
					okToPrune = false;
				}
			}
			if (okToPrune) {
				const TranspositionTable::NodeFlag flag = currentScore!=-maxScore?
						TranspositionTable::NM_LOWER_EVAL:TranspositionTable::NM_LOWER;
				agent->hashPut(key,score,currentScore,depth,ply,flag,emptyMove);
				return score;
			}

		} else {
			if (score < -maxScore-maxSearchPly) {
				nmMateScore=true;
			}
		}
	}

	//iid
	if (depth > allowIIDAtNormal &&	hashMove.none() &&
			!isKingAttacked && currentScore >= beta-iidMargin) {
		SearchInfo newSi(false,emptyMove);
		score = zwSearch(board,newSi,beta,depth/2,ply);
		hashOk=agent->hashGet(key, hashData, ply);
		if (hashOk) {
			hashMove = hashData.move();
		}
	}

	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;
	int moveCounter=0;
	int bestScore=-maxScore;
	bool isSingularMove = false;

	// se
	if (depth > seNonPVDepth && hashOk && !hashMove.none() && !si.partialSearch &&
			hashData.depth() >= depth-3 && (hashData.flag() & TranspositionTable::LOWER)) {
		if (abs(hashData.value()) < winningScore && board.isMoveLegal<true>(hashMove)) {
			SearchInfo seSi(false,hashMove,true);
			const int seValue = hashData.value() - seMargin;
			const int partialScore = zwSearch(board,seSi,seValue,depth/2,ply);
			if (partialScore < seValue) {
				isSingularMove = true;
			}
		}
	}

	while (true) {
		MoveIterator::Move& move = selectMove<false>(board, moves, hashMove, ply, depth);
		if (move.none()) {
			break;
		}
		if (si.partialSearch && move == si.move) {
			continue;
		}
		MoveBackup backup;
		board.doMove(move,backup);

		moveCounter++;
		nodes++;

		const bool givingCheck = board.setInCheck(board.getSideToMove());
		const bool passedPawn = isPawnPush(board,move.to);
		const bool pawnOn7thExtension = move.promotionPiece!=EMPTY;

		//futility
		if  (	move.type == MoveIterator::NON_CAPTURE &&
				depth < futilityDepth &&
				!isKingAttacked &&
				!givingCheck &&
				!pawnOn7thExtension &&
				!passedPawn &&
				!nmMateScore) {
			if (moveCountMargin(depth) < moveCounter
					&& !isMateScore(bestScore) ) {
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

		//reductions
		int reduction=0;
		int extension=0;
		if (isKingAttacked || pawnOn7thExtension ||
				(isSingularMove && move==hashMove)) {
			extension++;
		} else if (depth>lmrDepthThreshold && !givingCheck && !passedPawn && !nmMateScore &&
				move.type == MoveIterator::NON_CAPTURE) {
			reduction=reductionTableNonPV[depth][moveCounter];
		}

		SearchInfo newSi(true,move);
		int newDepth=depth-1+extension;

		score = -zwSearch(board, newSi, 1-beta, newDepth-reduction, ply+1);

		if (score >= beta && reduction>0) {
			bool research=true;
			if (reduction>2) {
				score = -zwSearch(board, newSi, 1-beta, newDepth-1, ply+1);
				research=(score >= beta);
			}
			if (research) {
				score = -zwSearch(board, newSi, 1-beta, newDepth, ply+1);
			}
		}

		board.undoMove(backup);

		if (stop()) {
			return 0;
		}

		if (score > bestScore) {
			bestScore=score;
			if( score >= beta) {
				updateKillers(board,move,ply);
				updateHistory(board,move,depth);
				const TranspositionTable::NodeFlag flag = currentScore!=-maxScore?
						TranspositionTable::LOWER_EVAL:TranspositionTable::LOWER;
				agent->hashPut(key,bestScore,currentScore,depth,ply,flag,move);
				return bestScore;
			}
		}

	}

	if (!moveCounter) {
		return si.partialSearch?beta-1:isKingAttacked?-maxScore+ply:drawScore;
	}

	const TranspositionTable::NodeFlag flag = currentScore!=-maxScore?
			TranspositionTable::UPPER_EVAL:TranspositionTable::UPPER;
	agent->hashPut(key,bestScore,currentScore,depth,ply,flag,emptyMove);

	return bestScore;
}

//quiescence search
int SimplePVSearch::qSearch(Board& board, SearchInfo& si,
		int alpha, int beta, int depth, int ply, const bool isPV) {

	if (stop()) {
		return 0;
	}

	if	(board.isDraw()) {
		return drawScore;
	}

	if	(ply >= maxSearchPly-1) {
		return evaluator.evaluate(board,alpha,beta);
	}

	TranspositionTable::HashData hashData;
	MoveIterator::Move hashMove;
	const Key key = board.getKey();
	bool okToPrune=false;
	const int oldAlpha=alpha;

	// tt retrieve & prunning
	const bool hashOk = agent->hashPruneGet(okToPrune, key, hashData, ply, depth, true, alpha, beta);
	if (hashOk) {
		hashMove = hashData.move();
		if (okToPrune && !isPV) {
			return hashData.value();
		}
	}

	const bool isKingAttacked = board.isInCheck();
	int bestScore = -maxScore;
	int currentScore = -maxScore;

	if (!isKingAttacked) {
		if (hashOk && (hashData.flag() & TranspositionTable::NODE_EVAL)) {
			currentScore = hashData.evalValue();
		} else {
			currentScore=evaluator.evaluate(board,alpha,beta);
		}
		bestScore=currentScore;
		if(bestScore>=beta) {
			if (!hashOk) {
				const TranspositionTable::NodeFlag flag = TranspositionTable::NODE_EVAL;
				agent->hashPut(key,bestScore,currentScore,-maxSearchPly,ply,flag,emptyMove);
			}
			return bestScore;
		}
		const int delta = isPawnPromoting(board)?deltaMargin*2:deltaMargin;
		if (bestScore < alpha - delta && !isPV && hashMove.none() &&
				board.getGamePhase()<ENDGAME && !isMateScore(beta)) {
			if (!hashOk) {
				const TranspositionTable::NodeFlag flag = TranspositionTable::NODE_EVAL;
				agent->hashPut(key,alpha,currentScore,-maxSearchPly,ply,flag,emptyMove);
			}
			return alpha;
		}
		if( isPV && alpha < bestScore) {
			alpha = bestScore;
		}
	}

	int moveCounter=0;
	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove;
	const PieceColor sideToMove = board.getSideToMove();

	while (true)  {

		MoveIterator::Move& move = selectMove<true>(board, moves, hashMove, ply, depth);
		if (move.none()) {
			break;
		}

		moveCounter++;
		nodes++;

		if (!isKingAttacked && !isPV && move != hashMove &&
				move.type==MoveIterator::BAD_CAPTURE && depth < 0) {
			continue;
		}

		if (move.promotionPiece==makePiece(sideToMove,ROOK) ||
				move.promotionPiece==makePiece(sideToMove,BISHOP)	) {
			continue;
		}

		MoveBackup backup;
		board.doMove(move,backup);

		const bool givingCheck = board.setInCheck(board.getSideToMove());

		if (move.promotionPiece==makePiece(sideToMove,KNIGHT) &&
				!givingCheck) {
			board.undoMove(backup);
			continue;
		}

		SearchInfo newSi(false,move);

		int score = -qSearch(board, newSi, -beta, -alpha, depth-1, ply+1, isPV);

		board.undoMove(backup);

		if (stop()) {
			return 0;
		}

		if( score >= beta ) {
			const TranspositionTable::NodeFlag flag = currentScore!=-maxScore?
					TranspositionTable::LOWER_EVAL:TranspositionTable::LOWER;
			agent->hashPut(key,score,currentScore,depth!=0?-1:0,ply,flag,move);
			return score;
		}

		if (score>bestScore) {
			bestScore=score;
			if( score>alpha ) {
				alpha = score;
				bestMove=move;
			}
		}

	}

	if (!moveCounter && isKingAttacked) {
		return -maxScore+ply;
	}

	TranspositionTable::NodeFlag flag;
	if (bestScore>oldAlpha) {
		flag = currentScore!=-maxScore?
				TranspositionTable::EXACT_EVAL:TranspositionTable::EXACT;
	} else {
		flag = currentScore!=-maxScore?
				TranspositionTable::UPPER_EVAL:TranspositionTable::UPPER;
		bestMove=emptyMove;
	}
	agent->hashPut(key,bestScore,currentScore,depth!=0?-1:0,ply,flag,bestMove);

	return bestScore;
}

const bool SimplePVSearch::stop() {
	return (timeIsUp() || agent->shouldStop());

}

//retrieve PV from transposition table
inline void SimplePVSearch::retrievePvFromHash(Board& board, PvLine& pv) {
	if (pv.moves[0].none()) {
		return;
	}
	MoveBackup backup[maxSearchPly];
	int i=0;
	board.doMove(pv.moves[0],backup[0]);
	pv.index=1;
	for (i=1;i<maxSearchPly;i++) {
		TranspositionTable::HashData hashData;
		if (!agent->hashGet(board.getKey(), hashData, i)) {
			break;
		}
		MoveIterator::Move move = hashData.move();
		if (!board.isMoveLegal<true>(move)) {
			break;
		}
		if (board.isDraw() && i>1) {
			break;
		}
		pv.moves[i]=move;
		pv.index=i;
		board.doMove(move,backup[i]);
	}
	while (--i>=0) {
		board.undoMove(backup[i]);
	}
}

void SimplePVSearch::initialize() {

	for (int x=0;x<=maxSearchDepth;x++) {
		for (int y=0;y<maxMoveCount;y++) {
			reductionTablePV[x][y]=static_cast<int>(!(x&&y)?0.0:floor(log(x)*log(y))/3.0);
			reductionTableNonPV[x][y]=static_cast<int>(!(x&&y)?0.0:floor(log(x)*log(y))/2.0);
		}
	}
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
	board.setInCheck(board.getSideToMove());

	while (true)  {

		MoveIterator::Move& move = selectMove<false>(board, moves, emptyMove, ply, depth);
		if (moves.end()) {
			break;
		}
		MoveBackup backup;
		board.doMove(move,backup);
		nodes += perft(board, depth-1, ply+1);
		board.undoMove(backup);

	}

	if (ply==1 && isUpdateUci()) {
		std::cout << "Node count: " << nodes << std::endl;
		std::cout << "Time: " << (getTickCount()-time) << std::endl;
	}

	return nodes;
}
