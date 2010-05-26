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

#include "searchagent.h"
#include "evaluator.h"

const int MATE_RANGE_CHECK = 10;
const int MATE_RANGE_SCORE = 300;
const int maxScore = 20000;
const int maxSearchDepth = 80;
const int maxSearchPly = 64;
const int allowIIDAtPV = 3;
const int allowIIDAtNormal = 13;
const int prunningDepth=4;
const int prunningMoves=4;
const int scoreTable[10]={1,900,100,9500,9000,50,45,40,50,-90};

class SimplePVSearch {

public:

	typedef struct PvLine {
		int index;
		MoveIterator::Move moves[maxSearchDepth];
	} PvLine;

	typedef struct SearchStats {

		SearchStats():
			ttHits(0), ttLower(0), ttUpper(0), ttExact(0), nullMoveHits(0),
			pvChanges(0), searchTime(0), searchNodes(0), searchDepth(0), bestMove(MoveIterator::Move()) {}

		SearchStats(const SearchStats& stats):
			ttHits(stats.ttHits), ttLower(stats.ttLower),
			ttUpper(stats.ttUpper), ttExact(stats.ttExact),
			nullMoveHits(stats.nullMoveHits), pvChanges(stats.pvChanges),
			searchTime(stats.searchTime), searchNodes(stats.searchNodes),
			searchDepth(stats.searchDepth), bestMove(stats.bestMove) {}
		long ttHits;
		long ttLower;
		long ttUpper;
		long ttExact;
		long nullMoveHits;
		long pvChanges;
		int searchTime;
		uint64_t searchNodes;
		int searchDepth;
		MoveIterator::Move bestMove;

		std::string toString() {
			std::string result =
					"ttHits:      " + StringUtil::toStr(ttHits) + "\n" ;
			result += "ttLower:     " + StringUtil::toStr(ttLower) + "\n";
			result += "ttUpper:     " + StringUtil::toStr(ttUpper) + "\n";
			result += "ttExact:     " + StringUtil::toStr(ttExact) + "\n";
			result += "nullMoveHits:" + StringUtil::toStr(nullMoveHits) + "\n";
			result += "pvChanges:   " + StringUtil::toStr(pvChanges) + "\n";
			result += "searchTime:  " + StringUtil::toStr(searchTime) + "\n";
			result += "searchNodes: " + StringUtil::toStr(searchNodes) + "\n";
			result += "searchDepth: " + StringUtil::toStr(searchDepth) + "\n";
			result += "bestMove:    " + bestMove.toString() + "\n";
			return result;
		}
		void clear() {
			ttHits=0;
			ttLower=0;
			ttUpper=0;
			ttExact=0;
			nullMoveHits=0;
			pvChanges=0;
			searchTime=0;
			searchNodes=0;
			searchDepth=0;
			bestMove=MoveIterator::Move();
		}

	} SearchStats;

	void operator()() {
		this->search();
	}

	SimplePVSearch(Board& board) :   _board(board), _depth(maxSearchDepth), _updateUci(true), errorCount(0), _startTime(0), _searchFixedDepth(true), _infinite(false) {}
	SimplePVSearch(Board& board, int depth ) : _board(board), _depth(depth), _updateUci(true), errorCount(0), _timeToSearch(0), _startTime(0), _searchFixedDepth(true), _infinite(false) {}
	SimplePVSearch(Board& board, int timeToSearch, int depth) : _board(board), _depth(depth), _updateUci(true), errorCount(0), _timeToSearch(timeToSearch), _startTime(0), _searchFixedDepth(false), _infinite(false) {}
	SimplePVSearch(Board& board, bool infinite ) : _board(board), _depth(maxSearchDepth), _updateUci(true), errorCount(0), _timeToSearch(0), _startTime(0), _searchFixedDepth(true), _infinite(true) {}

	virtual ~SimplePVSearch() {}
	virtual void search();
	virtual long perft(Board& board, int depth, int ply);
	virtual int getScore();

	inline const int getTickCount() {
		return ((clock() * 1000) / CLOCKS_PER_SEC);
	}

	inline const int toClock(const int time) {
		return int((((double)(time)/(double)1000)*(double)CLOCKS_PER_SEC));
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

	inline const void setTimeToSearch(const int timeToSearch) {
		_timeToSearch = timeToSearch;
	}

	inline const int getTimeToSearch() const {
		return _timeToSearch;
	}

	inline const void setStartTime(const int startTime) {
		_startTime = startTime;
	}

	inline const int getStartTime() const {
		return _startTime;
	}

private:
	Board& _board;
	int _depth;
	int _score;
	bool _updateUci;
	int errorCount;
	long _timeToSearch;
	long _startTime;
	long _nodes;
	long _time;
	bool _searchFixedDepth;
	bool _infinite;
	SearchStats stats;
	int timeToStop;
	MoveIterator rootMoves;
	MoveIterator::Move killer[maxSearchDepth][2];
	int history[ALL_PIECE_TYPE_BY_COLOR][ALL_SQUARE];
	Evaluator evaluator;

	int idSearch(Board& board);
	int rootSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv);
	int pvSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv);
	int normalSearch(Board& board, int alpha, int beta,	int depth, int ply, PvLine* pv,	const bool allowNullMove);
	int qSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv);
	void uciOutput(PvLine* pv, MoveIterator::Move& bestMove, const int totalTime, const int hashFull, const int depth);
	void uciOutput(MoveIterator::Move& bestMove);
	void uciOutput(MoveIterator::Move& move, const int moveCounter);
	const std::string pvLineToString(const PvLine* pv);
	MoveIterator::Move& selectMove(Board& board, MoveIterator& moves, MoveIterator::Move& ttMove, bool isKingAttacked, int ply);
	MoveIterator::Move& selectMove(Board& board, MoveIterator& moves, bool isKingAttacked);
	void scoreEvasions(Board& board, MoveIterator& moves);
	void scoreCaptures(Board& board, MoveIterator& moves, const bool seeOrdered);
	void scoreQuiet(Board& board, MoveIterator& moves);
	bool okToReduce(Board& board, MoveIterator::Move& move, MoveBackup& backup,
			int depth, int remainingMoves, bool isKingAttacked, int ply);
	bool okToNullMove(Board& board);
	bool isPawnFinal(Board& board);
	bool isPawnPush(MoveIterator::Move& move, MoveBackup& backup);
	int extendDepth(const bool isKingAttacked, const bool nullMoveMateScore, const bool pawnPush);
	int reduceDepth(Board& board, MoveIterator::Move& move, MoveBackup& backup,
			int depth, int remainingMoves, bool isKingAttacked, int ply, bool isPV);
	void updatePv(PvLine* pv, PvLine& line, MoveIterator::Move& move);
	const bool stop(const bool searchInProgress);
	const bool timeIsUp();
	void clearHistory();
	void updateHistory(Board& board, MoveIterator::Move& move, int depth);
	void updateKillers(Board& board, MoveIterator::Move& move, int ply);

};
// select a move
inline MoveIterator::Move& SimplePVSearch::selectMove(Board& board, MoveIterator& moves, MoveIterator::Move& ttMove, bool isKingAttacked, int ply) {

	MoveIterator::Move& killer1 = killer[ply][0];
	MoveIterator::Move& killer2 = killer[ply][1];

	if (moves.getStage()==MoveIterator::BEGIN_STAGE) {
		if (!isKingAttacked) {
			moves.setStage(MoveIterator::INIT_CAPTURE_STAGE);
		} else {
			moves.setStage(MoveIterator::INIT_EVASION_STAGE);
		}
		if (board.isMoveLegal(ttMove)) {
			return ttMove;
		}
	}

	if (moves.getStage()==MoveIterator::INIT_EVASION_STAGE) {
		board.generateEvasions(moves, board.getSideToMove());
		scoreEvasions(board, moves);
		moves.goNextStage();
	}

	if (moves.getStage()==MoveIterator::ON_EVASION_STAGE) {
		while (moves.hasNext()) {
			return moves.selectBest();
		}
		moves.setStage(MoveIterator::END_STAGE);
		return moves.next();
	}

	if (moves.getStage()==MoveIterator::INIT_CAPTURE_STAGE) {
		board.generateCaptures(moves, board.getSideToMove());
		scoreCaptures(board, moves,true);
		moves.goNextStage();
	}

	if (moves.getStage()==MoveIterator::ON_CAPTURE_STAGE) {
		while (moves.hasNext()) {
			MoveIterator::Move& move=moves.selectBest();
			if (move==ttMove) {
				continue;
			}
			//move.score = evaluator.see(board,move);
			if (move.score < 0) {
				moves.prior();
				break; // it will keep the bad captures after non captures
			}
			return move;
		}
		moves.goNextStage();
	}

	if (moves.getStage()==MoveIterator::KILLER1_STAGE) {
		moves.goNextStage();
		if (killer1 != ttMove && board.isMoveLegal(killer1)) {
			return killer1;
		}
	}

	if (moves.getStage()==MoveIterator::KILLER2_STAGE) {
		moves.goNextStage();
		if (killer2 != ttMove && board.isMoveLegal(killer2)) {
			return killer2;
		}
	}

	if (moves.getStage()==MoveIterator::INIT_QUIET_STAGE) {
		board.generateNonCaptures(moves, board.getSideToMove());
		scoreQuiet(board, moves);
		moves.goNextStage();
	}

	if (moves.getStage()==MoveIterator::ON_QUIET_STAGE) {
		while (moves.hasNext()) {
			MoveIterator::Move& move=moves.selectBest();
			if (move==ttMove || move==killer1 || move==killer2) {
				continue;
			}
			return move;
		}
		moves.goNextStage();
	}

	return moves.next();

}

// select a move
inline MoveIterator::Move& SimplePVSearch::selectMove(Board& board, MoveIterator& moves, bool isKingAttacked) {

	if (moves.getStage()==MoveIterator::BEGIN_STAGE) {
		if (!isKingAttacked) {
			moves.setStage(MoveIterator::INIT_CAPTURE_STAGE);

		} else {
			moves.setStage(MoveIterator::INIT_EVASION_STAGE);
		}
	}

	if (moves.getStage()==MoveIterator::INIT_EVASION_STAGE) {
		board.generateEvasions(moves, board.getSideToMove());
		scoreEvasions(board, moves);
		moves.goNextStage();
	}

	if (moves.getStage()==MoveIterator::ON_EVASION_STAGE) {
		while (moves.hasNext()) {
			return moves.selectBest();
		}
		moves.setStage(MoveIterator::END_STAGE);
		return moves.next();
	}

	if (moves.getStage()==MoveIterator::INIT_CAPTURE_STAGE) {
		board.generateCaptures(moves, board.getSideToMove());
		scoreCaptures(board, moves, true);
		moves.goNextStage();
	}

	if (moves.getStage()==MoveIterator::ON_CAPTURE_STAGE) {
		while (moves.hasNext()) {
			return moves.selectBest();
		}
		moves.setStage(MoveIterator::END_STAGE);
	}

	return moves.next();

}

// score evasion moves
inline void SimplePVSearch::scoreEvasions(Board& board, MoveIterator& moves) {
	const int historyBonus=20;
	moves.bookmark();

	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();

		if (board.getPieceBySquare(move.to) != EMPTY) {
			move.score = evaluator.see(board,move);
			if (move.type==MoveIterator::PROMO_CAPTURE) {
				move.score+=scoreTable[MoveIterator::PROMO_CAPTURE];
			}

		} else {
			move.score=history[board.getPieceTypeBySquare(move.from)][move.to]*historyBonus;
			if (move.type==MoveIterator::UNKNOW) {
				move.type=MoveIterator::NON_CAPTURE;
			} else if (move.type==MoveIterator::PROMO_NONCAPTURE) {
				move.score+=scoreTable[MoveIterator::PROMO_NONCAPTURE];
			}
		}
	}
	moves.goToBookmark();

}
// score capture moves
inline void SimplePVSearch::scoreCaptures(Board& board, MoveIterator& moves, const bool seeOrdered) {

	moves.bookmark();

	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();
		if (!seeOrdered) {
			move.score = pieceMaterialValues[board.getPieceBySquare(move.to)] -
					pieceMaterialValues[board.getPieceBySquare(move.from)];
		} else {
			move.score = evaluator.see(board,move);
		}
		if (move.type==MoveIterator::PROMO_CAPTURE) {
			move.score+=scoreTable[MoveIterator::PROMO_CAPTURE];
		}
	}
	moves.goToBookmark();

}

// score all moves
inline void SimplePVSearch::scoreQuiet(Board& board, MoveIterator& moves) {

	const int historyBonus=10000;
	moves.bookmark();

	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();
		move.score=history[board.getPieceTypeBySquare(move.from)][move.to]*historyBonus;
		if (move.type==MoveIterator::UNKNOW) {
			move.type=MoveIterator::NON_CAPTURE;
		}
		if (move.type==MoveIterator::PROMO_NONCAPTURE) {
			move.score+=scoreTable[MoveIterator::PROMO_NONCAPTURE];
		}

	}
	moves.goToBookmark();

}

// Checks if search can be reduced for a given move
inline bool SimplePVSearch::okToReduce(Board& board, MoveIterator::Move& move, MoveBackup& backup,
		int depth, int remainingMoves, bool isKingAttacked, int ply) {

	MoveIterator::Move& killer1 = killer[ply][0];
	MoveIterator::Move& killer2 = killer[ply][1];

	bool verify = (
			(remainingMoves > prunningMoves) &&
			(move.type == MoveIterator::NON_CAPTURE) &&
			(move.type!=MoveIterator::PROMO_CAPTURE) &&
			(move.type!=MoveIterator::PROMO_NONCAPTURE) &&
			(move!=killer1) &&
			(move!=killer2) &&
			(depth > prunningDepth) &&
			(!isKingAttacked) &&
			(!history[board.getPieceTypeBySquare(move.from)][move.to]
			));

	if (!verify) {
		return false;
	}

	if (isPawnPush(move,backup) || isPawnFinal(board)) {
		return false;
	}

	bool isCastling = backup.hasWhiteKingCastle ||
			backup.hasBlackKingCastle ||
			backup.hasWhiteQueenCastle ||
			backup.hasBlackQueenCastle;

	return !(isCastling);

}

// Ok to do null move?
inline bool SimplePVSearch::okToNullMove(Board& board) {
	return !isPawnFinal(board);
}

// remains pawns & kings only?
inline bool SimplePVSearch::isPawnFinal(Board& board) {

	const Bitboard pawns = board.getPiecesByType(WHITE_PAWN) |
			board.getPiecesByType(BLACK_PAWN);
	const Bitboard kings = board.getPiecesByType(WHITE_KING) |
			board.getPiecesByType(BLACK_KING);

	return !((pawns|kings)^board.getAllPieces());

}

// pawn push
inline bool SimplePVSearch::isPawnPush(MoveIterator::Move& move, MoveBackup& backup) {

	return (backup.movingPiece==WHITE_PAWN && squareRank[move.to] >= RANK_6) ||
			(backup.movingPiece==BLACK_PAWN && squareRank[move.to] <= RANK_3);

}

// depth extension
inline int SimplePVSearch::extendDepth(const bool isKingAttacked,
		const bool nullMoveMateScore, const bool pawnPush) {

	return isKingAttacked || nullMoveMateScore || pawnPush ? 1 : 0;

}
// depth reduction
inline int SimplePVSearch::reduceDepth(Board& board, MoveIterator::Move& move, MoveBackup& backup,
		int depth, int remainingMoves, bool isKingAttacked, int ply, bool isPV) {
	//TODO verify this

	if (!okToReduce(board, move, backup, depth, remainingMoves, isKingAttacked, ply)) {
		return 1;
	}

	return isPV ? 2 : 3;
}

inline const bool SimplePVSearch::stop(const bool searchInProgress) {
	return !searchInProgress || timeIsUp();
}

inline const bool SimplePVSearch::timeIsUp() {

	const uint64_t checkNodes=0x1FFF;
	if ( _searchFixedDepth || _infinite || (!_nodes & checkNodes)) {
		return false;
	}
	return (int)clock() >= timeToStop;

}

inline void SimplePVSearch::uciOutput(PvLine* pv, MoveIterator::Move& bestMove,
		const int totalTime, const int hashFull, const int depth) {

	if (isUpdateUci() && bestMove.from != NONE && pv->moves[0].from != NONE) {

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
		std::cout << "info depth "<< depth << " score " << scoreString << " time " << totalTime
				<< " nodes " << (_nodes) << " nps " << nps << " pv" << pvLineToString(pv) << std::endl;
		std::cout << "info nodes " << (_nodes) << " time " << totalTime << " nps " << nps
				<< " hashfull " << hashFull << std::endl;

	}
}

inline void SimplePVSearch::uciOutput(MoveIterator::Move& bestMove) {

	if (isUpdateUci()) {
		if (bestMove.from!=NONE) {
			if (isUpdateUci()) {
				std::cout << "bestmove " << bestMove.toString() << std::endl;
			}
		} else {
			std::cout << "bestmove (none)" << std::endl;
		}
	}
}

inline void SimplePVSearch::uciOutput(MoveIterator::Move& move, const int moveCounter) {

	const int uciOutputSecs=1500;

	if (isUpdateUci()) {
		if (_startTime+uciOutputSecs < getTickCount()) {
			std::cout << "info currmove " << move.toString()
																	<< " currmovenumber " << moveCounter << std::endl;
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
	memset(killer, 0, sizeof(MoveIterator::Move)*maxSearchDepth*2);
}

// update history
inline void SimplePVSearch::updateHistory(Board& board, MoveIterator::Move& move, int depth) {

	if (board.getPieceBySquare(move.to)!=EMPTY ||
			move.type == MoveIterator::PROMO_NONCAPTURE ||
			move.from ==NONE) {
		return;
	}

	history[board.getPieceTypeBySquare(move.from)][move.to]+=depth;

}

// update killers
inline void SimplePVSearch::updateKillers(Board& board, MoveIterator::Move& move, int ply) {
	if (board.getPieceBySquare(move.to)!=EMPTY ||
			move.type == MoveIterator::PROMO_NONCAPTURE ||
			move.from ==NONE) {
		return;
	}
	if (move != killer[ply][0]) {
		killer[ply][1] = killer[ply][0];
		killer[ply][0] = move;
	}
}

#endif /* SIMPLEPVSEARCH_H_ */
