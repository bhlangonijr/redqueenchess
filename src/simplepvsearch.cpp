/*
	Redqueen Chess Engine
    Copyright (C) 2008-2011 Ben-Hur Carlos Vieira Langoni Junior

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
	startTime = getTickCount();
	setTimeToStop();
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
	int64_t iterationTime[maxSearchPly];
	const bool isKingAttacked = board.setInCheck(board.getSideToMove());
	MoveIterator::Move easyMove;
	rootSearchInfo.alpha = -maxScore;
	rootSearchInfo.beta = maxScore;
	rootSearchInfo.allowNullMove = false;
	int searchTime;
	int lastDepth=0;
	int64_t searchNodes;
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
		iterationPVChange[depth]=0;
		// like in stockfish
		if (depth >= aspirationDepth && abs(iterationScore[depth-1]) < winningScore)	{
			int delta1 = iterationScore[depth-1]-iterationScore[depth-2];
			int delta2 = iterationScore[depth-2]-iterationScore[depth-3];
			aspirationDelta = std::max(abs(delta1)+abs(delta2)/2, 16)+7;
			rootSearchInfo.alpha = std::max(iterationScore[depth-1]-aspirationDelta,-maxScore);
			rootSearchInfo.beta  = std::min(iterationScore[depth-1]+aspirationDelta,+maxScore);
		}
		rootSearchInfo.depth=depth;
		rootSearchInfo.ply=0;
		score = rootSearch(board, rootSearchInfo, pv);
		iterationScore[depth]=score;
		iterationTime[depth]=getTickCount()-startTime;
		if (stop(rootSearchInfo) && depth > 1) {
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
			if (depth>5 && (abs(iterationScore[depth - 0]) >= maxScore-maxSearchPly &&
					abs(iterationScore[depth - 1]) >= maxScore-maxSearchPly &&
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
				if (!easyMove.none() && easyMove==bestMove && nodesPerMove[0]>=nodes*85/100 &&
						iterationTime[depth] > timeToSearch/2) {
					break;
				}
			}
			if (depth > 6 && depth < 40 &&
					iterationPVChange[depth]>0 && iterationPVChange[depth-1]>0) {
				agent->addExtraTime(depth,iterationPVChange);
			}
			if (iterationTime[depth] > timeToSearch*70/100) {
				break;
			}
		}
	}
	if (bestMove.none()) {
		bestMove=rootMoves.get(0);
		ponderMove=emptyMove;
	}
	uciOutput(pvLine, bestMove.score, getTickCount()-startTime, agent->hashFull(),
			lastDepth, maxPlySearched, rootSearchInfo.alpha, rootSearchInfo.beta);
	uciOutput(bestMove,ponderMove);
	return bestMove.score;
}

// root search
int SimplePVSearch::rootSearch(Board& board, SearchInfo& si, PvLine& pv) {
	const bool isKingAttacked = board.isInCheck();
	int alpha=si.alpha;
	int beta=si.beta;
	int countFH=0;
	int countFL=0;
	int moveCounter=0;
	MoveIterator::Move bestMove;
	while (true) {
		rootMoves.sortOrderingBy(nodesPerMove);
		rootMoves.first();
		moveCounter=0;
		int score = -maxScore;
		while (rootMoves.hasNext() && !stop(si)) {
			MoveIterator::Move& move = rootMoves.next();
			moveCounter++;
			uciOutput(move, moveCounter);
			while (true) {
				MoveBackup backup;
				int64_t nodes=this->nodes;
				nodesToGo = getTimeToSearch()<=1000?fastNodesToGo:defaultNodesToGo;
				board.doMove(move,backup);
				const bool givingCheck = board.setInCheck(board.getSideToMove());
				const bool pawnOn7thExtension = move.promotionPiece!=EMPTY;
				int extension=0;
				if (isKingAttacked) {
					extension++;
				}
				int newDepth=si.depth-1+extension;
				SearchInfo newSi(true,move,-beta,-alpha,newDepth,si.ply+1,PV_NODE,NULL);
				if (score>alpha || moveCounter==1) {
					newSi.update(newDepth,PV_NODE);
					score = -pvSearch(board, newSi);
				} else {
					int reduction=0;
					if (!extension && !givingCheck && !isPawnPush(board,move.to) &&
							moveCounter>lateMoveThreshold && !pawnOn7thExtension &&
							si.depth>lmrDepthThresholdRoot &&
							move.type == MoveIterator::NON_CAPTURE) {
						reduction=getReduction(true,si.depth,moveCounter);
					}
					newSi.update(newDepth-reduction,NONPV_NODE);
					score = -zwSearch(board, newSi);
					if (score>alpha) {
						if (reduction>0) {
							bool research=true;
							if (reduction>2) {
								newSi.update(newDepth-1,NONPV_NODE);
								score = -zwSearch(board, newSi);
								research=(score >= beta);
							}
							if (research) {
								newSi.update(newDepth,NONPV_NODE);
								score = -zwSearch(board, newSi);
							}
						}
						if (score>alpha) {
							newSi.update(newDepth,PV_NODE);
							score = -pvSearch(board, newSi);
						}
					}
				}
				board.undoMove(backup);
				nodes = this->nodes-nodes;
				updateRootMovesScore(nodes);
				move.score=-maxScore;
				if(score<beta || stop(si)) {
					break;
				}
				move.score=score;
				bestMove=move;
				pv.moves[0]=bestMove;
				retrievePvFromHash(board, pv);
				beta =  std::min(beta+aspirationDelta*(1<<countFH),maxScore);
				si.beta = beta;
				uciOutput(pv, bestMove.score, getTickCount()-startTime,
						agent->hashFull(), si.depth, maxPlySearched, alpha, beta);
				countFH++;
			}
			if (stop(si)) {
				break;
			}
			if( score > alpha ) {
				alpha = score;
				move.score=score;
				bestMove=move;
				pv.moves[0]=bestMove;
				retrievePvFromHash(board, pv);
				iterationPVChange[si.depth]++;
				uciOutput(pv, bestMove.score, getTickCount()-startTime,
						agent->hashFull(), si.depth, maxPlySearched, alpha, beta);
			} else {
				move.score=-maxScore;
			}
		}
		uciOutput(si.depth);
		if (si.alpha!=alpha || stop(si)) {
			break;
		}
		alpha = std::max(alpha-aspirationDelta*(1<<countFL),-maxScore);
		si.alpha = alpha;
		countFL++;
	}
	rootMoves.sortOrderingBy(nodesPerMove);
	rootMoves.first();
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
	TranspositionTable::HashData hashData;
	MoveIterator::Move hashMove;
	const Key key = si.partialSearch?board.getPartialSearchKey():board.getKey();
	si.alpha = std::max(-maxScore+si.ply, si.alpha);
	si.beta = std::min(maxScore-(si.ply+1), si.beta);
	if (si.alpha>=si.beta) {
		return si.alpha;
	}
	//tt retrieve
	bool hashOk = agent->hashGet(key, hashData, si.ply);
	if (hashOk) {
		hashMove = hashData.move();
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
	if (si.depth > allowIIDAtPV &&	hashMove.none() && !isKingAttacked) {
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
	bool isSingularMove = false;
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
		// se
		if (isHashMove && si.depth > sePVDepth && hashOk && !hashMove.none() && !si.partialSearch &&
				hashData.depth() >= si.depth-3 && (hashData.flag() & TranspositionTable::LOWER)) {
			if (abs(hashData.value()) < winningScore) {
				const int seValue = hashData.value() - seMargin;
				SearchInfo seSi(false,hashMove,true,0,seValue,si.depth/2,si.ply,NONPV_NODE,si.splitPoint);
				const int partialScore = zwSearch(board,seSi);
				if (partialScore < seValue) {
					isSingularMove = true;
				}
			}
		}
		MoveBackup backup;
		board.doMove(move,backup);
		moveCounter++;
		nodes++;
		const bool givingCheck = board.setInCheck(board.getSideToMove());
		const bool pawnOn7thExtension = move.promotionPiece!=EMPTY;
		int extension=0;
		if (isKingAttacked || (isSingularMove && isHashMove)) {
			extension++;
		}
		int newDepth=si.depth-1+extension;
		SearchInfo newSi(true,move,-si.beta,-si.alpha,newDepth,si.ply+1,PV_NODE,si.splitPoint);
		if (moveCounter==1) {
			newSi.update(newDepth,PV_NODE);
			score = -pvSearch(board, newSi);
		} else {
			int reduction=0;
			if (si.depth>lmrDepthThreshold && !extension && !givingCheck && !pawnOn7thExtension &&
					!isPawnPush(board,move.to) && move.type == MoveIterator::NON_CAPTURE) {
				reduction=getReduction(true, si.depth, moveCounter);
			}
			newSi.update(newDepth-reduction,NONPV_NODE);
			score = -zwSearch(board, newSi);
			if (score > si.alpha && score < si.beta) {
				if (reduction>0) {
					bool research=true;
					if (reduction>2) {
						newSi.update(newDepth-1,NONPV_NODE);
						score = -zwSearch(board, newSi);
						research=(score >= si.beta);
					}
					if (research) {
						newSi.update(newDepth,NONPV_NODE);
						score = -zwSearch(board, newSi);
					}
				}
				if (score > si.alpha && score < si.beta) {
					newSi.update(newDepth,PV_NODE);
					score = -pvSearch(board, newSi);
				}
			}
		}
		board.undoMove(backup);
		if (stop(si)) {
			return 0;
		}
		if (score>=si.beta) {
			bestScore=score;
			bestMove=move;
			TranspositionTable::NodeFlag flag = currentScore!=-maxScore && !isLazyEval?
					TranspositionTable::LOWER_EVAL:TranspositionTable::LOWER ;
			agent->updateHistory(board,bestMove,si.depth);
			updateKillers(board,bestMove,si.ply);
			agent->hashPut(key,bestScore,currentScore,si.depth,si.ply,flag,bestMove);
			return bestScore;
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
					return bestScore;
				}
			}
		}
	}
	if (!moveCounter) {
		return si.partialSearch?oldAlpha:isKingAttacked?-maxScore+si.ply:drawScore;
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
		si.update(0,NONPV_NODE,si.beta-1, si.beta, si.move);
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
	if (si.depth < razorDepth && hashMove.none() && si.allowNullMove &&
			!isKingAttacked && !isMateScore(si.beta) &&
			!board.isPawnPromoting() && !si.move.none() &&
			currentScore < si.beta-getRazorMargin(si.depth)) {
		const int newBeta = si.beta-getRazorMargin(si.depth);
		SearchInfo newSi(si.allowNullMove,si.move,newBeta-1,newBeta,0,
				si.ply,NONPV_NODE,si.splitPoint);
		score = qSearch(board, newSi);
		if (score < newBeta) {
			return score;
		}
	}
	if (!isKingAttacked && si.allowNullMove &&
			si.depth < nullMoveDepth && !board.isPawnFinal() &&
			!isMateScore(si.beta) && !si.move.none() &&
			currentScore >= si.beta+getFutilityMargin(si.depth,0)) {
		return currentScore-getFutilityMargin(si.depth,0);
	}
	// null move
	if (si.depth>1 && !isKingAttacked && si.allowNullMove &&
			!board.isPawnFinal() && !isMateScore(si.beta) &&
			currentScore >= si.beta-(si.depth>=nullMoveDepth?nullMoveMargin:0)) {
		const int reduction = 3 + (si.depth > 4 ? si.depth/6 : 0);
		MoveBackup backup;
		board.doNullMove(backup);
		SearchInfo newSi(false,emptyMove,si.beta,1-si.beta, si.depth-reduction,
				si.ply+1,NONPV_NODE,si.splitPoint);
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
			if (si.depth>6) {
				SearchInfo newSi(false,emptyMove,si.beta,si.beta,si.depth-reduction,
						si.ply+1,NONPV_NODE,si.splitPoint);
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
			!isKingAttacked && currentScore >= si.beta-iidMargin) {
		SearchInfo newSi(false,emptyMove,si.alpha,si.beta,si.depth/2,si.ply,NONPV_NODE,si.splitPoint);
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
	bool isSingularMove = false;
	while (true) {
		move = selectMove<false>(board, moves, hashMove, si.ply, si.depth);
		if (move.none()) {
			break;
		}
		if (si.partialSearch && move == si.move) {
			continue;
		}
		const bool isHashMove = move.type==MoveIterator::TT_MOVE;
		if (isHashMove && si.depth > seNonPVDepth && hashOk && !hashMove.none() && !si.partialSearch &&
				hashData.depth() >= si.depth-3 && (hashData.flag() & TranspositionTable::LOWER)) {
			if (abs(hashData.value()) < winningScore) {
				const int seValue = hashData.value() - seMargin;
				SearchInfo seSi(false,hashMove,true,0,seValue,si.depth/2,si.ply,NONPV_NODE,si.splitPoint);
				const int partialScore = zwSearch(board,seSi);
				if (partialScore < seValue) {
					isSingularMove = true;
				}
			}
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
				si.depth < futilityDepth &&
				!isKingAttacked &&
				!givingCheck &&
				!pawnOn7thExtension &&
				!passedPawn &&
				!nmMateScore) {
			if (getMoveCountMargin(si.depth) < moveCounter
					&& !isMateScore(bestScore) ) {
				board.undoMove(backup);
				continue;
			}
			const int futilityScore = currentScore + getFutilityMargin(si.depth,moveCounter);
			if (futilityScore < si.beta) {
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
		if (isKingAttacked || (isSingularMove && isHashMove)) {
			extension++;
		} else if (si.depth>lmrDepthThreshold && !givingCheck &&
				!pawnOn7thExtension && !nmMateScore && !passedPawn &&
				move.type == MoveIterator::NON_CAPTURE) {
			reduction=getReduction(false,si.depth,moveCounter);
		}
		int newDepth=si.depth-1+extension;
		SearchInfo newSi(true,move,si.beta,1-si.beta,newDepth-reduction,
				si.ply+1, NONPV_NODE,si.splitPoint);
		score = -zwSearch(board, newSi);
		if (score >= si.beta && reduction>0) {
			bool research=true;
			if (reduction>2) {
				newSi.update(newDepth-1,NONPV_NODE);
				score = -zwSearch(board, newSi);
				research=(score >= si.beta);
			}
			if (research) {
				newSi.update(newDepth,NONPV_NODE);
				score = -zwSearch(board, newSi);
			}
		}
		board.undoMove(backup);
		if (stop(si)) {
			return 0;
		}
		if (score>=si.beta) {
			bestScore=score;
			bestMove=move;
			TranspositionTable::NodeFlag flag = currentScore!=-maxScore && !isLazyEval?
					TranspositionTable::LOWER_EVAL:TranspositionTable::LOWER;
			agent->updateHistory(board,bestMove,si.depth);
			updateKillers(board,bestMove,si.ply);
			agent->hashPut(key,bestScore,currentScore,si.depth,si.ply,flag,bestMove);
			return bestScore;
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
					return bestScore;
				}
			}
		}
	}
	if (!moveCounter) {
		return si.partialSearch?si.beta-1:isKingAttacked?-maxScore+si.ply:drawScore;
	}
	TranspositionTable::NodeFlag flag = currentScore!=-maxScore && !isLazyEval?
			TranspositionTable::UPPER_EVAL:TranspositionTable::UPPER;
	bestMove=emptyMove;
	agent->hashPut(key,bestScore,currentScore,si.depth,si.ply,flag,emptyMove);
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
	const bool hashOk = agent->hashGet(okToPrune, key, hashData,
			si.ply, si.depth, si.allowNullMove, si.alpha, si.beta);
	if (hashOk) {
		hashMove = hashData.move();
		if (okToPrune && !(si.nodeType==PV_NODE)) {
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
				board.getGamePhase()<ENDGAME && !isMateScore(si.beta)) {
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
		moveCounter++;
		nodes++;
		if (move.promotionPiece==makePiece(sideToMove,ROOK) ||
				move.promotionPiece==makePiece(sideToMove,BISHOP)	) {
			continue;
		}
		if (!isKingAttacked && !(si.nodeType==PV_NODE) && !isHashMove &&
				 move.promotionPiece == EMPTY) {
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
		SearchInfo newSi(false,move,-si.beta,-si.alpha,si.depth-1,si.ply+1,si.nodeType,si.splitPoint);
		int score = -qSearch(board, newSi);
		board.undoMove(backup);
		if (stop(si)) {
			return 0;
		}
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

const bool SimplePVSearch::stop(SearchInfo& info) {
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
	return deltaMargin + depth * 30;
}
//initialize reduction arrays
void SimplePVSearch::initialize() {
	for (int x=0;x<=maxSearchDepth;x++) {
		moveCountMargin[x]=5 + x * x / 2;
		for (int y=0;y<maxMoveCount;y++) {
			reductionTablePV[x][y]=static_cast<int>(!(x&&y)?0.0:floor(log(x)*log(y))/3.0);
			reductionTableNonPV[x][y]=static_cast<int>(!(x&&y)?0.0:floor(log(x)*log(y))/2.0);
			futilityMargin[x][y]=static_cast<int>(100.03 * exp(0.35*(double(x))+-double(y*x)*0.01)) +
					(x>1 ? 90.0 * exp(0.03*(double(x))): 0);
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
		nodes += perft(board, depth-1, ply+1);
		board.undoMove(backup);
	}
	if (ply==1 && isUpdateUci()) {
		std::cout << "Node count: " << nodes << std::endl;
		std::cout << "Time: " << (getTickCount()-time) << std::endl;
	}
	return nodes;
}
