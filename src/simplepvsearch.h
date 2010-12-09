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
 * SimplePVSearch.h
 *
 *  Created on: 18/05/2009
 *      Author: bhlangonijr
 */

#ifndef SIMPLEPVSEARCH_H_
#define SIMPLEPVSEARCH_H_

#include <time.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "searchagent.h"
#include "evaluator.h"

// game constants
const int maxScoreRepetition = 25;
const int mateRangeScore = 300;
const int maxSearchDepth = 80;
const int maxSearchPly = 100;
const int checksAtQuiescenceMargin = 30;

// internal iterative deepening
const int allowIIDAtPV = 3;
const int allowIIDAtNormal = 7;

// margin constants
const inline int futilityMargin(const int depth) {return depth * 150;}
const inline int razorMargin(const int depth) {return 150 + depth * 175;}
const inline int moveCountMargin(const int depth) {return 6 + depth * 4;}
const int iidMargin=260;
const int easyMargin=500;
const int deltaMargin=950;
const int nullMoveMargin=512;

//depth prunning threshold
const int aspirationDepth=6;
const int futilityDepth=4;
const int nullMoveDepth=4;
const int razorDepth=4;
const int lmrDepthThresholdRoot=3;
const int lmrDepthThreshold1=2;
const int lateMoveThreshold1=2;
const int lmrDepthThreshold2=7;
const int lateMoveThreshold2=3;

const int scoreTable[11]={0,80000,60000,95000,90000,45000,40000,1000,-12000,50050,50000};

const long defaultNodesToGo=0xFFF;
const long fastNodesToGo=0xFF;

class SearchAgent;

class MoveIterator;

class SimplePVSearch {

public:

	typedef struct PvLine {
		int index;
		MoveIterator::Move moves[maxSearchPly+1];
		void copy(const PvLine line) {
			index=line.index;
			memcpy(moves, line.moves, (maxSearchPly+1) * sizeof(MoveIterator::Move));
		}
	} PvLine;

	typedef struct SearchInfo {
		SearchInfo(): inCheck(false) {}
		SearchInfo(const bool _inCheck,	const MoveIterator::Move _move):
			inCheck(_inCheck), move(_move)  {}
		bool inCheck;
		MoveIterator::Move move;
	} SearchInfo;

	SimplePVSearch() : _depth(maxSearchDepth), _updateUci(true), _startTime(0), _searchFixedDepth(false),
			_infinite(false), _nodes(0), nodesToGo(defaultNodesToGo) {}

	~SimplePVSearch() {}

	void search(Board _board);

	long perft(Board& board, int depth, int ply);

	int getScore();

	inline const long getTickCount() {
		return ((clock() * 1000) / CLOCKS_PER_SEC);
	}

	inline const long toClock(const long time) {
		return long((((double)(time)/(double)1000)*(double)CLOCKS_PER_SEC));
	}

	inline const bool isUpdateUci() const {
		return _updateUci;
	}

	inline const void setUpdateUci(const bool value) {
		_updateUci = value;
	}

	inline const bool isSearchFixedDepth() const {
		return _searchFixedDepth;
	}

	inline const void setSearchFixedDepth(const bool value) {
		_searchFixedDepth = value;
	}

	inline const bool isInfinite() const {
		return _infinite;
	}

	inline const void setInfinite(const bool value) {
		_infinite = value;
	}

	inline const void setDepth(const int depth) {
		_depth = depth;
	}

	inline const int getDepth() const {
		return _depth;
	}

	inline const void setTimeToSearch(const long timeToSearch) {
		_timeToSearch = timeToSearch;
	}

	inline const long getTimeToSearch() const {
		return _timeToSearch;
	}

	inline const void setStartTime(const long startTime) {
		_startTime = startTime;
	}

	inline const long getStartTime() const {
		return _startTime;
	}

	inline const void setTimeToStop() {
		timeToStop = clock() + toClock(_timeToSearch-10);
	}

	inline const long getTimeToStop() const {
		return timeToStop;
	}

	inline void setSearchAgent(SearchAgent* _agent) {
		this->agent=_agent;
	}

	inline void cleanUp() {
		evaluator.cleanPawnInfo();
		clearHistory();
	}

private:

	int _depth;
	int _score;
	bool _updateUci;
	long _timeToSearch;
	long _startTime;
	long _time;
	bool _searchFixedDepth;
	bool _infinite;
	long _nodes;
	long timeToStop;
	MoveIterator rootMoves;
	MoveIterator::Move killer[maxSearchPly+1][2];
	long nodesPerMove[MOVE_LIST_MAX_SIZE];
	int history[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE];
	Evaluator evaluator;
	SearchAgent* agent;
	MoveIterator::Move emptyMove;
	SearchInfo rootSearchInfo;
	int maxPlySearched;
	int aspirationDelta;
	long nodesToGo;

	int idSearch(Board& board);
	int rootSearch(Board& board, SearchInfo& si, int* alphaRoot, int* betaRoot, int depth, int ply, PvLine* pv);
	int pvSearch(Board& board,  SearchInfo& si, int alpha, int beta, int depth, int ply, PvLine* pv);
	int zwSearch(Board& board,  SearchInfo& si, int beta, int depth, int ply, PvLine* pv, const bool allowNullMove);
	int qSearch(Board& board,  SearchInfo& si,
			int alpha, int beta, int depth, int ply, PvLine* pv, const bool isPV);
	void uciOutput(PvLine* pv, const int score, const int totalTime,
			const int hashFull, const int depth, const int selDepth, const int alpha, const int beta);
	void uciOutput(MoveIterator::Move& bestMove, MoveIterator::Move& ponderMove);
	void uciOutput(MoveIterator::Move& move, const int moveCounter);
	void uciOutput(const int depth);
	const std::string pvLineToString(const PvLine* pv);
	template <bool quiescenceMoves>
	MoveIterator::Move& selectMove(Board& board, MoveIterator& moves,
			MoveIterator::Move& ttMove, bool isKingAttacked, int ply, int depth, const bool checksAtQuiescence);
	void scoreMoves(Board& board, MoveIterator& moves);
	void scoreRootMoves(Board& board, MoveIterator& moves);
	void filterLegalMoves(Board& board, MoveIterator& moves);
	bool isMateScore(const int score);
	bool isPawnFinal(Board& board);
	bool isPawnPush(Board& board, Square& square);
	bool isCaptureOrPromotion(Board& board, MoveIterator::Move& move);
	bool isPawnPromoting(const Board& board);
	bool isPawnOn7thRank(const Board& board, Square& square);
	void updatePv(PvLine* pv, PvLine& line, MoveIterator::Move& move);
	const bool stop();
	const bool timeIsUp();
	void clearHistory();
	void updateHistory(Board& board, MoveIterator::Move& move, int depth);
	void updateKillers(Board& board, MoveIterator::Move& move, int ply);
	void initRootMovesOrder();
	void updateRootMovesScore(const long value);
	long predictTimeUse(const long iterationTime[maxSearchPly], const long totalTime, const int depth);

};
// select a move
template <bool quiescenceMoves>
inline MoveIterator::Move& SimplePVSearch::selectMove(Board& board, MoveIterator& moves,
		MoveIterator::Move& ttMove, bool isKingAttacked, int ply, int depth, const bool checksAtQuiescence) {

	while (true) {

		switch (moves.getStage()) {

		case MoveIterator::END_STAGE:
			return emptyMove;

		case MoveIterator::BEGIN_STAGE:
			if (!isKingAttacked || (quiescenceMoves && checksAtQuiescence?depth<-2:depth<-1)) {
				moves.setStage(MoveIterator::INIT_CAPTURE_STAGE);
			} else {
				moves.setStage(MoveIterator::INIT_EVASION_STAGE);
			}
			if (!ttMove.none() && board.isMoveLegal<true>(ttMove)) {
				return ttMove;
			}
			break;

		case MoveIterator::INIT_EVASION_STAGE:
			board.generateEvasions(moves, board.getSideToMove());
			scoreMoves(board, moves);
			moves.setStage(MoveIterator::ON_EVASION_STAGE);
			break;

		case MoveIterator::ON_EVASION_STAGE:
			while (moves.hasNext()) {
				MoveIterator::Move& move=moves.selectBest();
				if (move==ttMove || !board.isMoveLegal<false>(move)) {
					continue;
				}
				return move;
			}
			moves.setStage(MoveIterator::END_STAGE);
			return emptyMove;

		case MoveIterator::INIT_CAPTURE_STAGE:
			if (quiescenceMoves && checksAtQuiescence) {
				board.generateQuiesMoves(moves, board.getSideToMove());
			} else {
				board.generateCaptures(moves, board.getSideToMove());
			}
			scoreMoves(board, moves);
			moves.setStage(MoveIterator::ON_CAPTURE_STAGE);
			break;

		case MoveIterator::ON_CAPTURE_STAGE:
			while (moves.hasNext()) {
				MoveIterator::Move& move=moves.selectBest();
				if (move==ttMove || !board.isMoveLegal<false>(move)) {
					continue;
				}
				if (move.type==MoveIterator::UNKNOW_CAPTURE) {
					move.score=evaluator.see<false>(board,move);
					if (move.score >= 0) {
						move.type=MoveIterator::GOOD_CAPTURE;
						move.score+=scoreTable[move.type];
						return move;
					} else {
						move.type=MoveIterator::BAD_CAPTURE;
						move.score+=scoreTable[move.type];
						moves.prior();
						continue;
					}
				}
				if (!quiescenceMoves) {
					if (move.type==MoveIterator::BAD_CAPTURE) {
						moves.prior();
						break;
					}
				}
				return move;
			}
			if (quiescenceMoves) {
				moves.setStage(MoveIterator::END_STAGE);
				return emptyMove;
			} else {
				moves.setStage(MoveIterator::KILLER1_STAGE);
			}
			break;

		case MoveIterator::KILLER1_STAGE:
			moves.setStage(MoveIterator::KILLER2_STAGE);
			if (killer[ply][0] != ttMove &&
					!board.isCaptureMove(killer[ply][0]) &&
					board.isMoveLegal<true>(killer[ply][0])) {
				return killer[ply][0];
			}
			break;

		case MoveIterator::KILLER2_STAGE:
			moves.setStage(MoveIterator::INIT_QUIET_STAGE);
			if (killer[ply][1] != ttMove &&
					!board.isCaptureMove(killer[ply][1]) &&
					board.isMoveLegal<true>(killer[ply][1])) {
				return killer[ply][1];
			}
			break;

		case MoveIterator::INIT_QUIET_STAGE:
			board.generateNonCaptures(moves, board.getSideToMove());
			scoreMoves(board, moves);
			moves.setStage(MoveIterator::ON_QUIET_STAGE);
			break;

		case MoveIterator::ON_QUIET_STAGE:
			while (moves.hasNext()) {
				MoveIterator::Move& move=moves.selectBest();
				if (move==ttMove || move==killer[ply][0] ||
						move==killer[ply][1] || !board.isMoveLegal<false>(move)) {
					continue;
				}
				return move;
			}
			moves.setStage(MoveIterator::END_STAGE);
			break;
		default:
			return emptyMove;
		}

	}
	return emptyMove;

}

// score moves
inline void SimplePVSearch::scoreMoves(Board& board, MoveIterator& moves) {

	moves.bookmark();

	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();
		const PieceTypeByColor pieceFrom = board.getPieceBySquare(move.from);
		const PieceTypeByColor pieceTo = board.getPieceBySquare(move.to);
		if (move.type==MoveIterator::UNKNOW) {
			move.score=-maxScore;
			if (pieceTo!=EMPTY) {
				move.score=(materialValues[pieceTo]-materialValues[pieceFrom]);
			} else if (board.getPieceType(pieceFrom)==PAWN){
				if (board.getEnPassant()!=NONE &&
						board.getSquareFile(move.from)!=board.getSquareFile(move.to)) {
					move.score=0;
				}
			}
			if (move.score!=-maxScore) {
				move.type = (move.score >= 0 ?
						MoveIterator::GOOD_CAPTURE : MoveIterator::UNKNOW_CAPTURE);
			} else {
				move.type=MoveIterator::NON_CAPTURE;
				const int hist = history[pieceFrom][move.to];
				const GamePhase phase = board.getGamePhase();
				const int gain =
						board.getPieceSquareValue(pieceFrom,move.to,phase)-
						board.getPieceSquareValue(pieceFrom,move.from,phase);
				move.score=hist+gain;

			}
		}
		move.score+=scoreTable[move.type];
	}
	moves.goToBookmark();

}

inline void SimplePVSearch::scoreRootMoves(Board& board, MoveIterator& moves) {
	PvLine pv = PvLine();
	moves.bookmark();
	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();
		MoveBackup backup;
		const bool isCapture = board.isCaptureMove(move);
		const int value = isCapture ? evaluator.see<false>(board,move) : 0;
		board.doMove(move,backup);
		const bool givingCheck = board.isInCheck(board.getSideToMove());
		SearchInfo newSi(givingCheck,move);
		move.score = -qSearch(board,newSi,-maxScore,maxScore,0,0,&pv,true);
		if (move.type==MoveIterator::UNKNOW) {
			if (isCapture) {
				move.type = (value >= 0 ?
						MoveIterator::GOOD_CAPTURE : MoveIterator::BAD_CAPTURE);
			} else {
				move.type=MoveIterator::NON_CAPTURE;

			}
		}
		board.undoMove(backup);
	}
	moves.goToBookmark();

}

// filter the move list to only legal moves
inline void SimplePVSearch::filterLegalMoves(Board& board, MoveIterator& moves) {

	MoveIterator newMoves;

	moves.first();
	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();
		MoveBackup backup;
		board.doMove(move,backup);
		if (board.isNotLegal()) {
			board.undoMove(backup);
			continue;
		}
		newMoves.add(move);
		board.undoMove(backup);
	}
	moves.clear();
	moves.addAll(newMoves);
	moves.first();
}

// is mate score?
inline bool SimplePVSearch::isMateScore(const int score) {
	return score < -maxScore+maxSearchPly ||
			score > maxScore-maxSearchPly;
}

// remains pawns & kings only?
inline bool SimplePVSearch::isPawnFinal(Board& board) {

	Bitboard pieces = board.getPiecesByType(WHITE_PAWN) |
			board.getPiecesByType(BLACK_PAWN) |
			board.getPiecesByType(WHITE_KING) |
			board.getPiecesByType(BLACK_KING);

	return !(pieces^board.getAllPieces());
}

// pawn push
inline bool SimplePVSearch::isPawnPush(Board& board, Square& square) {

	if (board.getPieceTypeBySquare(square)!=PAWN) {
		return false;
	}
	return evaluator.isPawnPassed(board,square);
}

inline bool SimplePVSearch::isCaptureOrPromotion(Board& board, MoveIterator::Move& move) {
	return board.isCaptureMove(move) ||
			move.promotionPiece != EMPTY;
}

// pawn promoting
inline bool SimplePVSearch::isPawnPromoting(const Board& board) {
	return (board.getPiecesByType(WHITE_PAWN) & rankBB[RANK_7]) ||
			(board.getPiecesByType(BLACK_PAWN) & rankBB[RANK_2]);
}

// pawn on 7th rank
inline bool SimplePVSearch::isPawnOn7thRank(const Board& board, Square& square) {
	if (board.getPieceTypeBySquare(square)!=PAWN) {
		return false;
	}
	const PieceColor color=board.getPieceColorBySquare(square);
	return  (squareToBitboard[square] & promoRank[color]);
}

inline const bool SimplePVSearch::timeIsUp() {
	if (_searchFixedDepth || _infinite || _nodes & nodesToGo) {
		return false;
	}
	return clock()>=timeToStop;

}

inline void SimplePVSearch::uciOutput(PvLine* pv, const int score, const int totalTime,
		const int hashFull, const int depth, const int selDepth, const int alpha, const int beta) {

	MoveIterator::Move& move = pv->moves[0];

	if (isUpdateUci() && !move.none()) {

		std::string scoreString = "cp " + StringUtil::toStr(score);

		if (abs(score) > (maxScore-mateRangeScore)) {
			if (score>0) {
				scoreString = "mate " +StringUtil::toStr((maxScore - (score+1))/2);
			} else {
				scoreString = "mate " +StringUtil::toStr(-(maxScore + score)/2);
			}
		}

		if (score >= beta) {
			scoreString += " lowerbound";
		} else if (score <= alpha) {
			scoreString += " upperbound";
		}

		long nps = totalTime>1000 ?  ((_nodes)/(totalTime/1000)) : _nodes;
		std::cout << "info depth "<< depth << " seldepth " << selDepth << std::endl;
		std::cout << "info depth "<< depth << " score " << scoreString << " time " << totalTime
				<< " nodes " << (_nodes) << " nps " << nps << " pv" << pvLineToString(pv) << std::endl;
		std::cout << "info nodes " << (_nodes) << " time " << totalTime << " nps " << nps
				<< " hashfull " << hashFull << std::endl;

	}
}

inline void SimplePVSearch::uciOutput(MoveIterator::Move& bestMove, MoveIterator::Move& ponderMove) {

	if (isUpdateUci()) {
		if (!bestMove.none()) {
			std::cout << "bestmove " << bestMove.toString();
			if (!ponderMove.none()) {
				std::cout << " ponder "<< ponderMove.toString();
			}
			std::cout << std::endl;

		} else {
			std::cout << "bestmove (none)" << std::endl;
		}
	}
}

inline void SimplePVSearch::uciOutput(MoveIterator::Move& move, const int moveCounter) {

	const long uciOutputSecs=1500;
	if (isUpdateUci()) {
		if (_startTime+uciOutputSecs < getTickCount()) {
			std::cout << "info currmove " << move.toString() << " currmovenumber " << moveCounter << std::endl;
		}
	}
}

inline void SimplePVSearch::uciOutput(const int depth) {

	const long uciOutputSecs=1500;
	if (isUpdateUci()) {
		if (_startTime+uciOutputSecs < getTickCount()) {
			std::cout << "info depth "<< depth << " seldepth " << maxPlySearched << " nodes " << _nodes << std::endl;
		}
	}
}

inline const std::string SimplePVSearch::pvLineToString(const PvLine* pv) {
	std::string result="";
	for(int x=0;x<pv->index;x++) {
		result += " ";
		result += pv->moves[x].toString();
	}
	return result;
}

inline void SimplePVSearch::updatePv(PvLine* pv, PvLine& line, MoveIterator::Move& move) {
	pv->moves[0] = move;
	memcpy(pv->moves + 1, line.moves, line.index * sizeof(MoveIterator::Move));
	pv->index = line.index + 1;
}

// clear history
inline void SimplePVSearch::clearHistory() {
	memset(history, 0, sizeof(int)*ALL_PIECE_TYPE_BY_COLOR*ALL_SQUARE);
	memset(killer, 0, sizeof(MoveIterator::Move)*maxSearchPly*2);
}

// update history
inline void SimplePVSearch::updateHistory(Board& board, MoveIterator::Move& move, int depth) {

	if (isCaptureOrPromotion(board,move) || move.none()) {
		return;
	}

	history[board.getPieceBySquare(move.from)][move.to]+=depth*depth;

}

// update killers
inline void SimplePVSearch::updateKillers(Board& board, MoveIterator::Move& move, int ply) {

	if (isCaptureOrPromotion(board,move) || move.none()) {
		return;
	}

	if (move != killer[ply][0]) {
		killer[ply][1] = killer[ply][0];
		killer[ply][0] = move;
		killer[ply][0].type = MoveIterator::KILLER1;
		killer[ply][1].type = MoveIterator::KILLER2;
	}
}

// init root moves ordering array
inline void SimplePVSearch::initRootMovesOrder() {
	for(int x=0;x<MOVE_LIST_MAX_SIZE;x++) {
		nodesPerMove[x]=0L;
	}
}

// update root moves score
inline void SimplePVSearch::updateRootMovesScore(const long value) {
	nodesPerMove[rootMoves.getIndex()]+=value;
}

inline long SimplePVSearch::predictTimeUse(const long iterationTime[maxSearchPly], const long totalTime, const int depth) {

	double ratio1 = double(iterationTime[depth-1])/double(totalTime);
	double ratio2 = double(iterationTime[depth-2])/double(totalTime);
	double ratio3 = double(iterationTime[depth-3])/double(totalTime);

	double newRatio = ratio1*exp((ratio2/ratio1)*0.7 + (ratio3/ratio2)*0.3);

	double newTime = double(iterationTime[depth-1])*exp(newRatio);

	return long(newTime);
}

#endif /* SIMPLEPVSEARCH_H_ */

