/*
	Redqueen Chess Engine
    Copyright (C) 2008-2012 Ben-Hur Carlos Vieira Langoni Junior

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
static int futilityMargin[maxSearchDepth+1][maxMoveCount];
static int moveCountMargin[maxSearchDepth+1];
// root search
void SimplePVSearch::search(Board board) {
	prepareToSearch();
	setStartTime(getTickCount());
	setTimeToStop();
	searchScore = idSearch(board);
	time = getTickCount()-getStartTime();
}

// get current score
int SimplePVSearch::getScore() {
	return searchScore;
}

// iterative deepening
int SimplePVSearch::idSearch(Board& board) {
	int bestScore = -maxScore;
	int iterationScore[maxSearchPly];
	int64_t iterationTime[maxSearchPly];
	board.setInCheck(board.getSideToMove());
	MoveIterator::Move easyMove;
	rootSearchInfo.alpha = -maxScore;
	rootSearchInfo.beta = maxScore;
	rootSearchInfo.allowNullMove = false;
	int lastDepth=0;
	MoveIterator::Move bestMove;
	MoveIterator::Move ponderMove;
	board.generateAllMoves(rootMoves, board.getSideToMove());
	filterLegalMoves(board,rootMoves,agent->getSearchMoves());
	scoreRootMoves(board,rootMoves);
	rootMoves.sort();
	setCompletedIteration(false);
	if (rootMoves.get(0).score > rootMoves.get(1).score + easyMargin ) {
		easyMove=rootMoves.get(0);
	}
	PvLine pv = PvLine();
	pv.index=0;

	for (int depth = 1; depth <= depthToSearch; depth++) {
		int score = 0;
		int aspirationDelta=0;
		maxPlySearched = 0;
		lastDepth=depth;
		iterationPVChange[depth]=0;
		rootSearchInfo.depth=depth;
		rootSearchInfo.ply=0;
		score=-maxScore;

		if (depth >= aspirationDepth) {
			const int delta = iterationScore[depth-1]-iterationScore[depth-2];
			aspirationDelta = std::max(delta, 10) + 5;
			rootSearchInfo.alpha = std::max(iterationScore[depth-1]-aspirationDelta,-maxScore);
			rootSearchInfo.beta  = std::min(iterationScore[depth-1]+aspirationDelta,+maxScore);
		}
		bool finished=false;
		setCompletedIteration(false);
		rootMoves.clearScore();
		while (!finished) {
			rootMoves.sort();
			rootMoves.first();
			score = rootSearch(board, rootSearchInfo, pv);
			rootMoves.sort();
			rootMoves.first();

			if (!stop(rootSearchInfo)) {
				setCompletedIteration(true);
			}
			iterationScore[depth]=score;
			updateHashWithPv(board,pv);
			if (!stop(rootSearchInfo)) {
				uciOutput(pv, score, getTickCount()-startTime,
						agent->hashFull(), rootSearchInfo.depth, maxPlySearched,
						rootSearchInfo.alpha, rootSearchInfo.beta);
			}

			const bool fail = score <= rootSearchInfo.alpha ||
					score >= rootSearchInfo.beta;
			const bool fullWidth = rootSearchInfo.alpha == -maxScore &&
					rootSearchInfo.beta == maxScore;
			finished = !fail || fullWidth ||
					depth < aspirationDepth ||
					(stop(rootSearchInfo) && depth>1);
			if (!finished) {
				rootSearchInfo.alpha = std::max(rootSearchInfo.alpha-aspirationDelta,-maxScore);
				rootSearchInfo.beta  = std::min(rootSearchInfo.beta+aspirationDelta,+maxScore);
				aspirationDelta = std::max(aspirationDelta*130/100,15);
			}

		}
		if (isCompletedIteration()) {
			bestMove=pv.moves[0];
			ponderMove=pv.moves[1];
			if (score > bestScore) {
				bestScore = score;
			}
		}
		iterationTime[depth]=getTickCount()-startTime;
		if (stop(rootSearchInfo) && depth > 1) {
			break;
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
			if (depth>5 && (abs(iterationScore[depth]) >= maxScore-maxSearchPly &&
					abs(iterationScore[depth-1]) >= maxScore-maxSearchPly &&
					abs(iterationScore[depth-2]) >= maxScore-maxSearchPly)) {
				break;
			}
			if (repetition >= maxScoreRepetition &&
					iterationScore[depth-0] == drawScore) {
				break;
			}
			if (depth>12) {
				if (bestMove!=easyMove) {
					easyMove = MoveIterator::Move();
				}
			}
			if (depth>10 && rootMoves.size()==1) {
				break;
			}
			if (depth>7) {
				if (!easyMove.none() && easyMove==bestMove &&
						nodesPerMove[0]>=nodes*85/100 &&
						iterationTime[depth] > timeToSearch/2) {
					break;
				}
			}
			if (depth > 8 && depth < 40 &&
					iterationPVChange[depth]>0) {
				agent->addExtraTime(depth,iterationPVChange);
			}
			if (iterationTime[depth] > getTimeToSearch()*70/100) {
				break;
			}
		}
	}
	if (bestMove.none()) {
		bestMove=rootMoves.get(0);
		ponderMove=emptyMove;
	}
	uciOutput(pv, bestMove.score, getTickCount()-startTime, agent->hashFull(),
			lastDepth, maxPlySearched, rootSearchInfo.alpha, rootSearchInfo.beta);
	uciOutput(bestMove,ponderMove);
	return bestMove.score;
}

// root search
int SimplePVSearch::rootSearch(Board& board, SearchInfo& si, PvLine& pv) {
	const bool isKingAttacked = board.isInCheck();
	int alpha=si.alpha;
	int beta=si.beta;
	int moveCounter=0;
	MoveIterator::Move bestMove;
	int score = -maxScore;
	rootMoves.first();
	while (rootMoves.hasNext() && !stop(si)) {
		MoveIterator::Move& move = rootMoves.next();
		moveCounter++;
		move.score=-maxScore;
		MoveBackup backup;
		int64_t nodes=this->nodes;
		nodesToGo = getTimeToSearch()<=1000?fastNodesToGo:defaultNodesToGo;
		int extension=0;
		if (isKingAttacked) {
			extension++;
		}
		board.doMove(move,backup);
		const bool givingCheck = board.setInCheck(board.getSideToMove());
		const bool passedPawnPush = isPawnPush(board,move.to);
		const bool pawnOn7thRank = move.promotionPiece!=EMPTY;
		int newDepth=si.depth-1+extension;
		SearchInfo newSi(true,move,-beta,-alpha,newDepth,si.ply+1,PV_NODE,NULL);
		if (score>alpha || moveCounter==1) {
			newSi.update(newDepth,PV_NODE,-beta,-alpha,move);
			score = -pvSearch(board, newSi);
		} else {
			int reduction=0;
			if (extension==0 && !isKingAttacked && !givingCheck && !passedPawnPush && !pawnOn7thRank &&
					moveCounter>lateMoveThreshold && si.depth>lmrDepthThresholdRoot &&
					move.type == MoveIterator::NON_CAPTURE &&
					killer[si.ply][0] != move && killer[si.ply][1] != move) {
				reduction=getReduction(true,si.depth,moveCounter);
				if (agent->getHistory(board.getPiece(move.from), move.to) <= 0) {
					reduction++;
				}
			}
			newSi.update(newDepth-reduction,CUT_NODE,-beta,-alpha,move);
			score = -zwSearch(board, newSi);
			if (score>alpha) {
				if (reduction>0) {
					bool research=true;
					if (reduction>2) {
						newSi.update(newDepth-1,CUT_NODE,-beta,-alpha,move);
						score = -zwSearch(board, newSi);
						research=(score >= beta);
					}
					if (research) {
						newSi.update(newDepth,CUT_NODE,-beta,-alpha,move);
						score = -zwSearch(board, newSi);
					}
				}
				if (score>alpha) {
					newSi.update(newDepth,PV_NODE,-beta,-alpha,move);
					score = -pvSearch(board, newSi);
				}
			}
		}
		board.undoMove(backup);
		if (stop(si)) {
			return 0;
		}
		this->nodes++;
		nodes = this->nodes-nodes;
		updateRootMovesScore(nodes);
		if(score>=beta) {
			move.score=score;
			bestMove=move;
			pv.moves[0]=move;
			retrievePvFromHash(board, pv);
			return score;
		}
		if(score>alpha || moveCounter==1) {
			move.score=score;
			alpha = score;
			bestMove=move;
			pv.moves[0]=move;
			retrievePvFromHash(board, pv);
			iterationPVChange[si.depth]++;
		}
		uciOutput(move, moveCounter);
	}
	if (!moveCounter) {
		return isKingAttacked?-maxScore+si.ply:drawScore;
	}
	return alpha;
}

// principal variation search
int SimplePVSearch::pvSearch(Board& board, SearchInfo& si) {
	if (stop(si)) {
		return 0;
	}
	if (si.ply>maxPlySearched) {
		maxPlySearched=si.ply;
	}
	if (si.depth<=0) {
		si.update(0,PV_NODE);
		return qSearch(board, si);
	}
	if	(board.isDraw() || si.ply >= maxSearchPly-1) {
		return drawScore;
	}
	const int oldAlpha = si.alpha;
	int score = -maxScore;
	int currentScore = -maxScore;
	bool okToPrune=false;
	si.alpha = std::max(-maxScore+si.ply, si.alpha);
	si.beta = std::min(maxScore-(si.ply+1), si.beta);
	if (si.alpha>=si.beta) {
		return si.alpha;
	}
	TranspositionTable::HashData hashData;
	MoveIterator::Move hashMove;
	const Key key = si.partialSearch?board.getPartialSearchKey():board.getKey();
	// tt retrieve & prunning
	bool hashOk = agent->hashGet(okToPrune, key, hashData, si.ply, si.depth);
	if (hashOk) {
		hashMove = hashData.move();
		if (okToPrune) {
			return hashData.value();
		}
	}
	const bool isKingAttacked = board.isInCheck();
	bool isLazyEval=false;
	if (!isKingAttacked) {
		if (hashOk && (hashData.flag() & TranspositionTable::NODE_EVAL)) {
			currentScore = hashData.evalValue();
		} else {
			currentScore = evaluator.evaluate(board,si.alpha,si.beta);
			isLazyEval = evaluator.isLazyEval();
		}
	}
	//iid
	if (si.depth > allowIIDAtPV &&	hashMove.none()) {
		SearchInfo newSi(false,emptyMove,si.alpha,si.beta,si.depth-2,si.ply,PV_NODE,si.splitPoint);
		score = pvSearch(board,newSi);
		hashOk = agent->hashGet(key, hashData, si.ply);
		if (hashOk) {
			hashMove = hashData.move();
		}
	}
	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove=emptyMove;
	MoveIterator::Move move;
	int moveCounter=0;
	int bestScore=-maxScore;
	bool nmMateScore=false;
	while (true) {
		move = selectMove<false>(board, moves, hashMove, si.ply, si.depth);
		if (move.none()) {
			break;
		}
		if (si.partialSearch && move == si.move) {
			continue;
		}
		const bool isHashMove = move.type==MoveIterator::TT_MOVE;
		int extension=0;
		if (isKingAttacked) {
			extension++;
		} else if (isHashMove && si.depth > sePVDepth && hashOk && !hashMove.none()
				&& !si.partialSearch && hashData.depth() >= si.depth-3 &&
				(hashData.flag() & TranspositionTable::LOWER)) {
			if (abs(hashData.value()) < winningScore) {
				const int seValue = hashData.value() - seMargin;
				SearchInfo seSi(false, hashMove, true, seValue-1, seValue, si.depth/2,si.ply,
						si.nodeType, si.splitPoint);
				const int partialScore = zwSearch(board,seSi);
				if (partialScore < seValue) {
					extension++;
				}
			}
		}
		MoveBackup backup;
		board.doMove(move,backup);
		moveCounter++;
		const bool givingCheck = board.setInCheck(board.getSideToMove());
		const bool passedPawnPush = isPawnPush(board,move.to);
		const bool pawnOn7thRank = move.promotionPiece!=EMPTY;
		int newDepth=si.depth-1+extension;
		SearchInfo newSi(true,move,-si.beta,-si.alpha,newDepth,si.ply+1,PV_NODE,si.splitPoint);
		if (moveCounter==1 || isHashMove) {
			newSi.update(newDepth,PV_NODE,-si.beta,-si.alpha,move);
			score = -pvSearch(board, newSi);
		} else {
			int reduction=0;
			if (extension==0 && !isKingAttacked && !givingCheck && !passedPawnPush && !pawnOn7thRank &&
					si.depth>lmrDepthThreshold && move.type == MoveIterator::NON_CAPTURE &&
					killer[si.ply][0] != move && killer[si.ply][1] != move) {
				reduction=getReduction(true, si.depth, moveCounter);
				if (agent->getHistory(board.getPiece(move.from), move.to) <= 0) {
					reduction++;
				}
			}
			newSi.update(newDepth-reduction,CUT_NODE,-si.beta,-si.alpha,move);
			score = -zwSearch(board, newSi);
			if (score > si.alpha && score < si.beta) {
				if (reduction>0) {
					bool research=true;
					if (reduction>2) {
						newSi.update(newDepth-1,CUT_NODE,-si.beta,-si.alpha,move);
						score = -zwSearch(board, newSi);
						research=(score >= si.beta);
					}
					if (research) {
						newSi.update(newDepth,CUT_NODE,-si.beta,-si.alpha,move);
						score = -zwSearch(board, newSi);
					}
				}
				if (score > si.alpha && score < si.beta) {
					newSi.update(newDepth,PV_NODE,-si.beta,-si.alpha,move);
					score = -pvSearch(board, newSi);
				}
			}
		}
		board.undoMove(backup);
		if (stop(si)) {
			return 0;
		}
		nodes++;
		if (score>=si.beta) {
			bestScore=score;
			bestMove=move;
			break;
		}
		if (score>bestScore) {
			bestScore=score;
			if(score>si.alpha ) {
				si.alpha = score;
				bestMove=move;
			}
		}
		if (!stop(si) && agent->getThreadNumber()>1 &&
				agent->getFreeThreads()>0 && si.depth>minSplitDepth) {
			if (agent->spawnThreads(board, &si, getThreadId(), &moves, &move, &hashMove,
					&bestScore, &si.alpha, &currentScore, &moveCounter, &nmMateScore)) {
				if (bestScore>=si.beta) {
					break;
				}
			}
		}
	}
	if (!moveCounter) {
		return si.partialSearch?oldAlpha:isKingAttacked?-maxScore+si.ply:drawScore;
	}
	TranspositionTable::NodeFlag flag;
	if (bestScore>=si.beta) {
		flag = currentScore!=-maxScore && !isLazyEval?
				TranspositionTable::LOWER_EVAL:TranspositionTable::LOWER ;
		agent->updateHistory(board,bestMove,si.depth);
		updateKillers(board,bestMove,si.ply);
	} else if (bestScore>oldAlpha) {
		flag = currentScore!=-maxScore && !isLazyEval?
				TranspositionTable::EXACT_EVAL:TranspositionTable::EXACT;
	} else {
		flag = currentScore!=-maxScore && !isLazyEval?
				TranspositionTable::UPPER_EVAL:TranspositionTable::UPPER;
		bestMove=emptyMove;
	}
	agent->hashPut(key,bestScore,currentScore,si.depth,si.ply,flag,bestMove);
	return bestScore;
}

// zero window search - non pv nodes
int SimplePVSearch::zwSearch(Board& board, SearchInfo& si) {
	if (stop(si)) {
		return 0;
	}
	if (si.ply>maxPlySearched) {
		maxPlySearched=si.ply;
	}
	if (si.depth<=0) {
		si.update(0,si.nodeType,si.beta-1, si.beta, si.move);
		return qSearch(board, si);
	}
	if	(board.isDraw() || si.ply >= maxSearchPly-1) {
		return drawScore;
	}
	if (-maxScore+si.ply >= si.beta) {
		return si.beta;
	}
	if (maxScore-(si.ply+1) < si.beta) {
		return si.beta-1;
	}
	const bool isKingAttacked = board.isInCheck();
	bool isLazyEval = false;
	bool nmMateScore=false;
	bool okToPrune=false;
	int score = 0;
	int currentScore = -maxScore;
	TranspositionTable::HashData hashData;
	MoveIterator::Move hashMove;
	const Key key = si.partialSearch?board.getPartialSearchKey():board.getKey();
	// tt retrieve & prunning
	bool hashOk = agent->hashGet(okToPrune, key, hashData, si.ply, si.depth,
			si.allowNullMove, si.beta-1, si.beta);
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
			currentScore = evaluator.evaluate(board,si.beta-1,si.beta);
			isLazyEval = evaluator.isLazyEval();
		}
	}
	//razoring
	if (si.depth < razorDepth && hashMove.none() && si.allowNullMove &&
			!isKingAttacked && !isMateScore(si.beta) &&
			!board.isPawnPromoting() && !si.move.none() &&
			currentScore < si.beta-getRazorMargin(si.depth)) {
		const int newBeta = si.beta-getRazorMargin(si.depth);
		SearchInfo newSi(si.allowNullMove,si.move,newBeta-1,newBeta,0,
				si.ply,CUT_NODE,si.splitPoint);
		score = qSearch(board, newSi);
		if (score < newBeta) {
			return score;
		}
	}
	//futility
	if (!isKingAttacked && si.allowNullMove &&
			si.depth < futilityDepth && !board.isPawnFinal() &&
			!isMateScore(si.beta) && !si.move.none() &&
			currentScore >= si.beta+getFutilityMargin(si.depth,0)) {
		return currentScore-getFutilityMargin(si.depth,0);
	}
	// null move
	if (si.depth>1 && !isKingAttacked && si.allowNullMove &&
			!board.isPawnFinal() && !isMateScore(si.beta) &&
			currentScore >= si.beta-(si.depth>=nullMoveDepth?nullMoveMargin:0)) {
		const int reduction = 3 + (si.depth > 4 ? si.depth/4 : 0);
		MoveBackup backup;
		board.doNullMove(backup);
		SearchInfo newSi(false,emptyMove,si.beta,1-si.beta, si.depth-reduction,
				si.ply+1,CUT_NODE,si.splitPoint);
		score = -zwSearch(board, newSi);
		board.undoNullMove(backup);
		if (stop(si)) {
			return 0;
		}
		if (score >= si.beta) {
			if (score >= maxScore-maxSearchPly) {
				score = si.beta;
			}
			bool okToPrune = true;
			if (si.depth>11) {
				SearchInfo newSi(false,emptyMove,si.alpha,si.beta,si.depth-reduction,
						si.ply+1,CUT_NODE,si.splitPoint);
				const int newScore = zwSearch(board, newSi);
				if (newScore<si.beta) {
					okToPrune = false;
				}
			}
			if (okToPrune) {
				const TranspositionTable::NodeFlag flag = currentScore!=-maxScore && !isLazyEval?
						TranspositionTable::NM_LOWER_EVAL:TranspositionTable::NM_LOWER;
				agent->hashPut(key,score,currentScore,si.depth,si.ply,flag,emptyMove);
				return score;
			}
		} else {
			if (score == -maxScore+si.ply+2) {
				nmMateScore=true;
			}
		}
	}
	//iid
	if (si.depth > allowIIDAtNormal &&	hashMove.none() &&
			currentScore >= si.beta-iidMargin) {
		SearchInfo newSi(false,emptyMove,si.alpha,si.beta,si.depth/2,si.ply,ALL_NODE,si.splitPoint);
		score = zwSearch(board,newSi);
		hashOk=agent->hashGet(key, hashData, si.ply);
		if (hashOk) {
			hashMove = hashData.move();
		}
	}
	MoveIterator moves = MoveIterator();
	MoveIterator::Move move;
	MoveIterator::Move bestMove=emptyMove;
	int moveCounter=0;
	int bestScore=-maxScore;
	while (true) {
		move = selectMove<false>(board, moves, hashMove, si.ply, si.depth);
		if (move.none()) {
			break;
		}
		if (si.partialSearch && move == si.move) {
			continue;
		}
		const bool isHashMove = move.type==MoveIterator::TT_MOVE;
		int extension=0;
		if (isKingAttacked) {
			extension++;
		} else if (isHashMove && si.depth > seNonPVDepth && hashOk && !hashMove.none() &&
				!si.partialSearch && hashData.depth() >= si.depth-3 &&
				(hashData.flag() & TranspositionTable::LOWER)) {
			if (abs(hashData.value()) < winningScore) {
				const int seValue = hashData.value() - seMargin;
				SearchInfo seSi(false, hashMove, true, seValue-1, seValue, si.depth/2, si.ply,
						si.nodeType, si.splitPoint);
				const int partialScore = zwSearch(board,seSi);
				if (partialScore < seValue) {
					extension++;
				}
			}
		}
		MoveBackup backup;
		board.doMove(move,backup);
		moveCounter++;
		const bool givingCheck = board.setInCheck(board.getSideToMove());
		const bool passedPawnPush = isPawnPush(board,move.to);
		const bool pawnOn7thRank = move.promotionPiece!=EMPTY;
		//futility
		if  (	move.type == MoveIterator::NON_CAPTURE &&
				!isKingAttacked &&
				!givingCheck &&
				!passedPawnPush &&
				!pawnOn7thRank &&
				!nmMateScore &&
				extension == 0) {

			if (getMoveCountMargin(si.depth) < moveCounter &&
					si.depth < moveCountDepth && !isMateScore(bestScore) ) {
				board.undoMove(backup);
				continue;
			}
			if (si.depth < futilityDepth) {
				const int futilityScore = currentScore + getFutilityMargin(si.depth,moveCounter);
				if (futilityScore < si.beta) {
					if (futilityScore>bestScore) {
						bestScore=futilityScore;
					}
					board.undoMove(backup);
					continue;
				}
			}
		}
		//reductions
		int reduction=0;
		if (extension==0 && !isKingAttacked && !givingCheck && si.depth>lmrDepthThreshold &&
				!nmMateScore && move.type == MoveIterator::NON_CAPTURE &&
				!isHashMove && moveCounter != 1 && killer[si.ply][0] != move && killer[si.ply][1] != move) {
			reduction=getReduction(false, si.depth, moveCounter);
			if (si.nodeType == CUT_NODE ||
					agent->getHistory(board.getPiece(move.from), move.to) <= 0) {
				reduction++;
			}
		}
		int newDepth=si.depth-1+extension;
		SearchInfo newSi(true,move,si.beta,1-si.beta,newDepth-reduction,
				si.ply+1,si.nodeType==CUT_NODE?ALL_NODE:CUT_NODE,si.splitPoint);
		score = -zwSearch(board, newSi);
		if (score >= si.beta && reduction>0) {
			bool research=true;
			if (reduction>2) {
				newSi.update(newDepth-1,si.nodeType==CUT_NODE?ALL_NODE:CUT_NODE,
						si.beta,1-si.beta,move);
				score = -zwSearch(board, newSi);
				research=(score >= si.beta);
			}
			if (research) {
				newSi.update(newDepth,si.nodeType==CUT_NODE?ALL_NODE:CUT_NODE,
						si.beta,1-si.beta,move);
				score = -zwSearch(board, newSi);
			}
		}
		board.undoMove(backup);
		if (stop(si)) {
			return 0;
		}
		nodes++;
		if (score>=si.beta) {
			bestScore=score;
			bestMove=move;
			break;
		}
		if (score>bestScore) {
			bestScore=score;
			bestMove=move;
		}
		if (!stop(si) && agent->getThreadNumber()>1 &&
				agent->getFreeThreads()>0 && si.depth>minSplitDepth) {
			if (agent->spawnThreads(board, &si, getThreadId(), &moves, &move, &hashMove,
					&bestScore, &si.alpha, &currentScore, &moveCounter, &nmMateScore)) {
				if (bestScore>=si.beta) {
					break;
				}
			}
		}
	}
	if (!moveCounter) {
		return si.partialSearch?si.beta-1:isKingAttacked?-maxScore+si.ply:drawScore;
	}
	TranspositionTable::NodeFlag flag;
	if (bestScore>=si.beta) {
		flag = currentScore!=-maxScore && !isLazyEval?
				TranspositionTable::LOWER_EVAL:TranspositionTable::LOWER;
		agent->updateHistory(board,bestMove,si.depth);
		updateKillers(board,bestMove,si.ply);
	} else {
		flag = currentScore!=-maxScore && !isLazyEval?
				TranspositionTable::UPPER_EVAL:TranspositionTable::UPPER;
		bestMove=emptyMove;
	}
	agent->hashPut(key,bestScore,currentScore,si.depth,si.ply,flag,bestMove);
	return bestScore;
}

//quiescence search
int SimplePVSearch::qSearch(Board& board, SearchInfo& si) {
	if (stop(si)) {
		return 0;
	}
	if	(board.isDraw() || si.ply >= maxSearchPly-1) {
		return drawScore;
	}
	TranspositionTable::HashData hashData;
	MoveIterator::Move hashMove;
	const Key key = board.getKey();
	bool okToPrune=false;
	const int oldAlpha=si.alpha;
	// tt retrieve & prunning
	const bool hashOk = si.nodeType == PV_NODE ?
			agent->hashGet(okToPrune, key, hashData,
					si.ply, si.depth) :
					agent->hashGet(okToPrune, key, hashData,
							si.ply, si.depth, si.allowNullMove, si.alpha, si.beta);
	if (hashOk) {
		hashMove = hashData.move();
		if (okToPrune) {
			return hashData.value();
		}
	}
	const bool isKingAttacked = board.isInCheck();
	bool isLazyEval = false;
	int bestScore = -maxScore;
	int currentScore = -maxScore;
	if (!isKingAttacked) {
		if (hashOk && (hashData.flag() & TranspositionTable::NODE_EVAL)) {
			currentScore = hashData.evalValue();
		} else {
			currentScore=evaluator.evaluate(board,si.alpha,si.beta);
			isLazyEval = evaluator.isLazyEval();
		}
		bestScore=currentScore;
		if(bestScore>=si.beta) {
			if (!hashOk && !isLazyEval) {
				const TranspositionTable::NodeFlag flag = TranspositionTable::NODE_EVAL;
				agent->hashPut(key,bestScore,currentScore,-maxSearchPly,si.ply,flag,emptyMove);
			}
			return bestScore;
		}
		const int delta = board.isPawnPromoting()?getDeltaMargin(si.depth)*2:getDeltaMargin(si.depth);
		if (bestScore < si.alpha - delta && !(si.nodeType==PV_NODE) && hashMove.none() &&
				!isMateScore(si.beta)) {
			if (!hashOk && !isLazyEval) {
				const TranspositionTable::NodeFlag flag = TranspositionTable::NODE_EVAL;
				agent->hashPut(key,si.alpha,currentScore,-maxSearchPly,si.ply,flag,emptyMove);
			}
			return si.alpha;
		}
		if((si.nodeType==PV_NODE) && si.alpha<bestScore) {
			si.alpha = bestScore;
		}
	}
	int moveCounter=0;
	MoveIterator moves = MoveIterator();
	MoveIterator::Move bestMove=emptyMove;
	MoveIterator::Move move;
	const PieceColor sideToMove = board.getSideToMove();
	while (true)  {
		move = selectMove<true>(board, moves, hashMove, si.ply, si.depth);
		if (move.none()) {
			break;
		}
		const bool isHashMove = move.type==MoveIterator::TT_MOVE;
		const bool passedPawn = isPawnPush(board,move.to);
		const bool pawnOn7thRank = move.promotionPiece!=EMPTY;
		const bool allowFutility = !isKingAttacked && !(si.nodeType==PV_NODE) &&
				!isHashMove && !pawnOn7thRank;
		moveCounter++;
		if (move.promotionPiece==makePiece(sideToMove,ROOK) ||
				move.promotionPiece==makePiece(sideToMove,BISHOP)	) {
			continue;
		}
		if (allowFutility) {
			if (si.depth < qsOnlyRecaptureDepth && move.to != si.move.to) {
				continue;
			}
			if (move.type==MoveIterator::BAD_CAPTURE) {
				continue;
			}
		}
		MoveBackup backup;
		board.doMove(move,backup);
		const bool givingCheck = board.setInCheck(board.getSideToMove());
		if (move.promotionPiece==makePiece(sideToMove,KNIGHT) &&
				!givingCheck) {
			board.undoMove(backup);
			continue;
		}
		if (allowFutility && !board.isPawnFinal() && !passedPawn && !givingCheck ) {
			const int gain = materialValues[backup.capturedPiece];
			const int futilityScore = currentScore+gain+getFutilityMargin();
			if (futilityScore<si.alpha) {
				if (futilityScore>bestScore) {
					bestScore=futilityScore;
				}
				board.undoMove(backup);
				continue;
			}
		}
		SearchInfo newSi(false,move,-si.beta,-si.alpha,si.depth-1,
				si.ply+1,si.nodeType,si.splitPoint);
		int score = -qSearch(board, newSi);
		board.undoMove(backup);
		if (stop(si)) {
			return 0;
		}
		nodes++;
		if( score >= si.beta ) {
			const TranspositionTable::NodeFlag flag = currentScore!=-maxScore && !isLazyEval?
					TranspositionTable::LOWER_EVAL:TranspositionTable::LOWER;
			agent->hashPut(key,score,currentScore,si.depth!=0?-1:0,si.ply,flag,move);
			return score;
		}
		if (score>bestScore) {
			bestScore=score;
			if( score>si.alpha ) {
				si.alpha = score;
				bestMove=move;
			}
		}
	}
	if (!moveCounter && isKingAttacked) {
		return -maxScore+si.ply;
	}
	TranspositionTable::NodeFlag flag;
	if (bestScore>oldAlpha) {
		flag = currentScore!=-maxScore && !isLazyEval?
				TranspositionTable::EXACT_EVAL:TranspositionTable::EXACT;
	} else {
		flag = currentScore!=-maxScore && !isLazyEval?
				TranspositionTable::UPPER_EVAL:TranspositionTable::UPPER;
		bestMove=emptyMove;
	}
	agent->hashPut(key,bestScore,currentScore,si.depth!=0?-1:0,si.ply,flag,bestMove);
	return bestScore;
}

// parallel search
void SimplePVSearch::smpPVSearch(Board board, SimplePVSearch* master,
		SimplePVSearch* ss, SplitPoint* sp) {
	if (sp==NULL) {
		return;
	}
	const bool isKingAttacked = board.isInCheck();
	const bool isPV = sp->isPV;
	int score = -maxScore;
	MoveIterator::Move move;
	while (sp->moves) {
		master->lock();
		move = master->selectMove<false>(board, *sp->moves, *sp->hashMove, sp->ply, sp->depth);
		(*sp->moveCounter)++;
		master->unlock();
		if (move.none()) {
			break;
		}
		MoveBackup backup;
		board.doMove(move,backup);
		const bool givingCheck = board.setInCheck(board.getSideToMove());
		const bool passedPawn = ss->isPawnPush(board,move.to);
		const bool pawnOn7thExtension = move.promotionPiece!=EMPTY;
		//futility
		if  (	!isPV &&
				move.type == MoveIterator::NON_CAPTURE &&
				sp->depth < futilityDepth &&
				!isKingAttacked &&
				!givingCheck &&
				!pawnOn7thExtension &&
				!passedPawn &&
				!*sp->nmMateScore) {
			if (ss->getMoveCountMargin(sp->depth) < *sp->moveCounter
					&& !master->isMateScore(*sp->bestScore) ) {
				board.undoMove(backup);
				continue;
			}
			const int futilityScore = *sp->currentScore +
					ss->getFutilityMargin(sp->depth, *sp->moveCounter);
			master->lock();
			if (futilityScore < sp->beta) {
				if (futilityScore>*sp->bestScore) {
					*sp->bestScore=futilityScore;
				}
				master->unlock();
				board.undoMove(backup);
				continue;
			}
			master->unlock();
		}
		//reductions
		int reduction=0;
		int extension=0;
		if (isKingAttacked) {
			extension++;
		} else if (sp->depth>lmrDepthThreshold && !givingCheck &&
				!passedPawn && !pawnOn7thExtension && !(*sp->nmMateScore) &&
				move.type == MoveIterator::NON_CAPTURE && (*sp->moveCounter) != 1 &&
				master->getKiller(sp->ply, 0) != move && master->getKiller(sp->ply, 1) != move) {
			reduction=ss->getReduction(isPV, sp->depth, *sp->moveCounter);
		}
		int newDepth=sp->depth-1+extension;
		SearchInfo newSi(true,move,sp->beta,1-sp->beta, newDepth-reduction, sp->ply+1, CUT_NODE, sp);
		if (isPV) {
			newSi.update(newDepth-reduction,PV_NODE,-sp->beta,-(*sp->currentAlpha),move);
		}
		score = -ss->zwSearch(board, newSi);
		bool research=isPV?score > *sp->currentAlpha &&
				score < sp->beta:score >= sp->beta && reduction>0;
		if (research) {
			if (reduction>2) {
				newSi.update(newDepth-1,CUT_NODE);
				score = -ss->zwSearch(board, newSi);
				research=(score >= sp->beta);
			}
			if (research) {
				newSi.update(newDepth,CUT_NODE);
				score = -ss->zwSearch(board, newSi);
			}
			if (isPV && score > *sp->currentAlpha && score < sp->beta) {
				newSi.update(newDepth,PV_NODE);
				score = -ss->pvSearch(board, newSi);
			}
		}
		board.undoMove(backup);
		master->lock();
		if (!master->stop(newSi) && sp->moves) {
			if (score>=sp->beta) {
				*sp->bestScore=score;
				*sp->bestMove=move;
				sp->shouldStop=true;
				master->unlock();
				return;
			}
			if (score>*sp->bestScore) {
				*sp->bestScore=score;
				if(score>*sp->currentAlpha ) {
					*sp->currentAlpha=score;
					*sp->bestMove=move;
				}
			}
		}
		master->unlock();
	}
}

const bool SimplePVSearch::stop(SearchInfo& info) {
	if ((agent->getSearchNodes()>0 &&
			agent->getSearcher(MAIN_THREAD)->getSearchedNodes() >= agent->getSearchNodes())) {
		return true;
	}
	if (timeIsUp() || agent->shouldStop()) {
		return true;
	}
	return info.splitPoint!=NULL && info.splitPoint->shouldStop;
}

//retrieve PV from transposition table
inline void SimplePVSearch::retrievePvFromHash(Board& board, PvLine& pv) {
	if (pv.moves[0].none()) {
		return;
	}
	MoveBackup backup[maxSearchPly];
	int i=0;
	board.doMove(pv.moves[0],backup[0]);
	board.setInCheck(board.getSideToMove());
	pv.index=1;
	for (i=1;i<maxSearchPly;i++) {
		TranspositionTable::HashData hashData;
		if (!agent->hashGet(board.getKey(), hashData, i)) {
			pv.moves[i].clear();
			break;
		}
		MoveIterator::Move move = hashData.move();
		if ((board.isDraw() && i>1) ||
				!board.isMoveLegal<true>(move)) {
			pv.moves[i].clear();
			break;
		}
		pv.moves[i]=move;
		pv.index=i;
		board.doMove(move,backup[i]);
		board.setInCheck(board.getSideToMove());
	}
	while (--i>=0) {
		board.undoMove(backup[i]);
	}
}
//retrieve PV from transposition table
inline void SimplePVSearch::updateHashWithPv(Board& board, PvLine& pv) {
	if (pv.moves[0].none()) {
		return;
	}
	if (!board.isMoveLegal<true>(pv.moves[0])) {
		return;
	}
	MoveBackup backup[maxSearchPly];
	int i=0;
	while (!pv.moves[i].none()) {
		TranspositionTable::HashData hashData;
		const bool found = agent->hashGet(board.getKey(), hashData, i);
		if (!found || hashData.move() != pv.moves[i]) {
			const int score = board.isInCheck()?-maxScore:
					evaluator.evaluate(board,-maxScore,maxScore);
			agent->hashPut(board.getKey(),score,score,0,i,
					TranspositionTable::NODE_EVAL,pv.moves[i]);
		}
		board.doMove(pv.moves[i],backup[i]);
		board.setInCheck(board.getSideToMove());
		i++;
	}
	while (--i>=0) {
		board.undoMove(backup[i]);
	}
}
// get reduction factor
const int SimplePVSearch::getReduction(const bool isPV, const int depth, const int moveCounter) const {
	int reduction = isPV?reductionTablePV[std::min(maxSearchDepth,depth)][std::min(maxMoveCount,moveCounter)]:
			reductionTableNonPV[std::min(maxSearchDepth,depth)][std::min(maxMoveCount,moveCounter)];
	return reduction;
}
//get futility margins
const int SimplePVSearch::getFutilityMargin(const int depth, const int moveCounter) const {
	return futilityMargin[std::min(maxSearchDepth,depth)][std::min(maxMoveCount,moveCounter)];
}
//get futility margins / QS
const int SimplePVSearch::getFutilityMargin() const {
	return futilityQSMargin;
}
//get move count margins
const int SimplePVSearch::getMoveCountMargin(const int depth) const {
	return moveCountMargin[std::min(maxSearchDepth,depth)];
}
//get razor margins
const int SimplePVSearch::getRazorMargin(const int depth) {
	return 125 + depth * 175;
}
//get razor margins
const int SimplePVSearch::getDeltaMargin(const int depth) {
	return deltaMargin + depth * 5;
}
//initialize reduction arrays
void SimplePVSearch::initialize() {
	for (int x=0;x<=maxSearchDepth;x++) {
		moveCountMargin[x]=5 + x * x / 2;
		for (int y=0;y<maxMoveCount;y++) {
			reductionTablePV[x][y]=(int)(!(x&&y)?0.0:floor(log(x)*log(y))/3.0);
			reductionTableNonPV[x][y]=(int)(!(x&&y)?0.0:floor(log(x)*log(y))/2.0);
			futilityMargin[x][y]=(int)(100.03 * exp(0.23*(double(x))+-double(y*x)*0.01)) +
					(x>1 ? 90.0 * exp(0.03*(double(x))): 0);
			//std::cout << x << "," << y << " = " << futilityMargin[x][y] << std::endl;
		}
	}
}
//perft
int64_t SimplePVSearch::perft(Board& board, int depth, int ply) {
	if (depth == 0) {
		return 1;
	}
	int64_t time=0;
	if (ply==1) {
		prepareToSearch();
		time=getTickCount();
	}
	int64_t nodes=0;
	int64_t partialNodes=0;
	MoveIterator moves = MoveIterator();
	MoveIterator::Move move;
	board.setInCheck(board.getSideToMove());
	while (true)  {
		move = selectMove<false>(board, moves, emptyMove, ply, depth);
		if (moves.end()) {
			break;
		}
		MoveBackup backup;
		board.doMove(move,backup);
		partialNodes = perft(board, depth-1, ply+1);
		nodes += partialNodes;
		if (ply == 1) std::cout << move.toString() << ": " << partialNodes << std::endl;
		board.undoMove(backup);
	}
	if (ply==1 && isUpdateUci()) {
		std::cout << "Node count: " << nodes << std::endl;
		std::cout << "Time: " << (getTickCount()-time) << std::endl;
	}
	return nodes;
}
