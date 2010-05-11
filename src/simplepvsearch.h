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

#include "searchagent.h"

const int maxScore = 20000;
const int maxSearchDepth = 80;
const int maxSearchPly = 30;
const int allowIIDAtPV = 2;
const int allowIIDAtNormal = 4;
const int scoreTable[10]={0,8000,6000,7000,6500,5000,4500,4000,500,-900};

class SimplePVSearch {

public:

	typedef struct PvLine {
		int index;
		MoveIterator::Move moves[maxSearchDepth];
	} PvLine;

	typedef struct SearchStats {

		SearchStats():
			ttHits(0), ttLower(0), ttUpper(0), ttExact(0), nullMoveHits(0), pvChanges(0), searchTime(0), searchNodes(0), searchDepth(0) {}

		SearchStats(const SearchStats& stats):
			ttHits(stats.ttHits), ttLower(stats.ttLower),
			ttUpper(stats.ttUpper), ttExact(stats.ttExact),
			nullMoveHits(stats.nullMoveHits), pvChanges(stats.pvChanges),
			searchTime(stats.searchTime), searchNodes(stats.searchNodes), searchDepth(stats.searchDepth) {}
		long ttHits;
		long ttLower;
		long ttUpper;
		long ttExact;
		long nullMoveHits;
		long pvChanges;
		int searchTime;
		uint64_t searchNodes;
		int searchDepth;

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

	int idSearch(Board& board);
	int rootSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv);
	int pvSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv);
	int normalSearch(Board& board, int alpha, int beta,	int depth, int ply, PvLine* pv,	const bool allowNullMove);
	int qSearch(Board& board, int alpha, int beta, int depth, int ply, PvLine* pv);
	const std::string pvLineToString(const PvLine* pv);
	MoveIterator::Move& selectMove(Board& board, MoveIterator& moves, MoveIterator::Move& ttMove, int alpha, int beta, int ply);
	MoveIterator::Move& selectMove(Board& board, MoveIterator& moves, int alpha, int beta, int ply);
	void scoreMoves(Board& board, MoveIterator& moves, int alpha, int beta, int ply);
	bool okToReduce(Board& board, MoveIterator::Move& move, MoveBackup& backup,
			int depth, int remainingMoves, bool isKingAttacked);
	bool okToNullMove(Board& board);
	void updatePv(PvLine* pv, PvLine& line, MoveIterator::Move& move);
	const bool stop(const bool searchInProgress);
	const bool timeIsUp();
	void clearHistory();
	void updateHistory(Board& board, MoveIterator::Move& move, int depth, int ply);

};
// select a move
inline MoveIterator::Move& SimplePVSearch::selectMove(Board& board, MoveIterator& moves, MoveIterator::Move& ttMove, int alpha, int beta, int ply) {

	if (moves.getStage()==MoveIterator::BEGIN_STAGE) {

		moves.setStage(MoveIterator::INIT_CAPTURE_STAGE);
		if (board.isMoveLegal(ttMove)) {
			ttMove.type = MoveIterator::TT_MOVE;
			return ttMove;
		} else {
			ttMove.type = MoveIterator::UNKNOW;
		}

	}

	if (moves.getStage()==MoveIterator::INIT_CAPTURE_STAGE) {

		board.generateCaptures(moves, board.getSideToMove());
		scoreMoves(board, moves, alpha, beta, ply);
		moves.goNextStage();

	}

	if (moves.getStage()==MoveIterator::ON_CAPTURE_STAGE) {

		while (moves.hasNext()) {
			MoveIterator::Move& move=moves.selectBest();
			if (move==ttMove && ttMove.type==MoveIterator::TT_MOVE) {
				continue;
			}
			return move;
		}
		moves.goNextStage();
	}

	if (moves.getStage()==MoveIterator::KILLER1_STAGE) {

		moves.goNextStage();
		if (killer[ply][0] != ttMove &&	board.isMoveLegal(killer[ply][0])) {
			killer[ply][0].type = MoveIterator::KILLER1;
			return killer[ply][0];
		} else {
			killer[ply][0].type = MoveIterator::UNKNOW;
		}
	}

	if (moves.getStage()==MoveIterator::KILLER2_STAGE) {

		moves.goNextStage();
		if (killer[ply][1] != ttMove && board.isMoveLegal(killer[ply][1])) {
			killer[ply][1].type = MoveIterator::KILLER2;
			return killer[ply][1];
		} else {
			killer[ply][1].type = MoveIterator::UNKNOW;
		}
	}

	if (moves.getStage()==MoveIterator::INIT_QUIET_STAGE) {

		board.generateNonCaptures(moves, board.getSideToMove());
		scoreMoves(board, moves, alpha, beta, ply);
		moves.goNextStage();

	}

	if (moves.getStage()==MoveIterator::ON_QUIET_STAGE) {

		while (moves.hasNext()) {
			MoveIterator::Move& move=moves.selectBest();
			if ((move==ttMove && ttMove.type==MoveIterator::TT_MOVE) ||
					(move==killer[ply][0] && killer[ply][0].type==MoveIterator::KILLER1) ||
					(move==killer[ply][1] && killer[ply][1].type==MoveIterator::KILLER2)) {
				continue;
			}
			return move;
		}
		moves.goNextStage();

	}

	return moves.next();

}

// select a move
inline MoveIterator::Move& SimplePVSearch::selectMove(Board& board, MoveIterator& moves, int alpha, int beta, int ply) {

	if (moves.getStage()==MoveIterator::BEGIN_STAGE) {
		moves.setStage(MoveIterator::INIT_CAPTURE_STAGE);
	}

	if (moves.getStage()==MoveIterator::INIT_CAPTURE_STAGE) {
		board.generateCaptures(moves, board.getSideToMove());
		scoreMoves(board, moves, alpha, beta, ply);
		moves.goNextStage();
	}

	if (moves.getStage()==MoveIterator::ON_CAPTURE_STAGE) {

		if (moves.hasNext()) {
			return moves.selectBest();
		}
		moves.setStage(MoveIterator::END_STAGE);
	}

	return moves.next();

}

// score all moves
inline void SimplePVSearch::scoreMoves(Board& board, MoveIterator& moves, int alpha, int beta, int ply) {

	moves.bookmark();

	while (moves.hasNext()) {
		MoveIterator::Move& move = moves.next();
		move.score=0;
		if (board.getPieceBySquare(move.to) != EMPTY) {
			move.score = pieceMaterialValues[board.getPieceBySquare(move.from)] - pieceMaterialValues[board.getPieceBySquare(move.to)];
		}

		if (move.type==MoveIterator::NON_CAPTURE) {
			move.score+=history[board.getPieceTypeBySquare(move.from)][move.to];
		}

		move.score+=scoreTable[move.type];

	}

	moves.goToBookmark();

}

// Checks if search can be reduced for a given move
inline bool SimplePVSearch::okToReduce(Board& board, MoveIterator::Move& move, MoveBackup& backup,
		int depth, int remainingMoves, bool isKingAttacked) {

	const int prunningDepth=3;
	const int prunningMoves=3;

	bool verify = (
			(remainingMoves > prunningMoves) &&
			(move.type == MoveIterator::NON_CAPTURE) &&
			(depth > prunningDepth) &&
			(!isKingAttacked) &&
			(!history[board.getPieceTypeBySquare(move.from)][move.to]));

	if (!verify) {
		return false;
	}

	bool isPawnPush = (backup.movingPiece==WHITE_PAWN && squareRank[move.to] >= RANK_6) ||
			(backup.movingPiece==BLACK_PAWN && squareRank[move.to] <= RANK_3);

	if (isPawnPush) {
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

	const Bitboard pawns = board.getPiecesByType(WHITE_PAWN) |
			board.getPiecesByType(BLACK_PAWN);
	const Bitboard kings = board.getPiecesByType(WHITE_KING) |
			board.getPiecesByType(BLACK_KING);

	return ((pawns|kings)^board.getAllPieces());

}

inline const bool SimplePVSearch::stop(const bool searchInProgress) {

	return !searchInProgress || timeIsUp();

}

inline const bool SimplePVSearch::timeIsUp() {

	const uint64_t checkNodes=0x3E8;

	if ( _searchFixedDepth || _infinite || ((_nodes & checkNodes)==checkNodes)) {
		return false;
	}

	return (int)clock() >= timeToStop;

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
inline void SimplePVSearch::updateHistory(Board& board, MoveIterator::Move& move, int depth, int ply) {

	if (!(move.type==MoveIterator::NON_CAPTURE) ) {
		return;
	}

	if (move != killer[ply][0]) {
		killer[ply][1] = killer[ply][0];
		killer[ply][0] = move;
	}

	history[board.getPieceTypeBySquare(move.from)][move.to]+=depth;

}
#endif /* SIMPLEPVSEARCH_H_ */
