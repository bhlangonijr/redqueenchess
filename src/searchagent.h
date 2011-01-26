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
 * SearchAgent.h
 *
 *  Created on: 01/05/2009
 *      Author: bhlangonijr
 */

#ifndef SEARCHAGENT_H_
#define SEARCHAGENT_H_
#include <iostream>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include "board.h"
#include "simplepvsearch.h"
#include "transpositiontable.h"
#include "uci.h"
const std::string mainHashName 		= "DefaultHashTable";
const size_t defaultDepth			= 5;
const size_t defaultHashSize		= 128;
const int defaultGameSize			= 30;
const int timeTableSize				= 7;
const int benchSize					= 12;
const int benchDepth				= 14;
const int timeTable [7][3] = {
		{25, 900000, 180000},
		{23, 180000, 60000},
		{21, 30000, 10000},
		{19, 10000, 5000},
		{17, 5000, 1000},
		{15, 1000, 0}
};

const std::string benchPositions[benchSize] = {
		"8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - - 0 1",
		"1q2r1k1/5pb1/2bppnp1/p1p5/Nr2PPP1/1BBP4/1PP4Q/2KR3R w - - 0 1",
		"1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1",
		"rn3rk1/pbppq1pp/1p2pb2/4N2Q/3PN3/3B4/PPP2PPP/R3K2R w KQ - 6 11",
		"1Q6/5kpp/2p4r/b4p2/P2R4/1N4P1/5PKP/4q3 b - - 0 1",
		"r1r2bk1/pp1n1p1p/2pqb1p1/3p4/1P1P4/1QN1PN2/P3BPPP/2RR2K1 w - - 0 1",
		"6k1/8/8/8/8/1p6/6K1/8 w - - 0 1",
		"8/1B3k2/4Rbp1/3Pp1p1/5p2/5P1P/3r2PK/8 b - - 0 1",
		"1r6/r7/2bkp3/1p1p1p1p/p1pP1PpP/P1P1P1B1/1PK4P/1N1B4 b - - 0 150",
		"8/8/8/8/3k4/1r6/6K1/8 w - - 0 1",
		"2n5/2P5/1Pk5/2B5/8/p7/8/1K6 b - - 2 86",
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
};

class SimplePVSearch;

class SearchAgent {

public:

	enum SearchMode {
		SEARCH_TIME, SEARCH_DEPTH, SEARCH_MOVESTOGO, SEARCH_MOVETIME, SEARCH_MOVES, SEARCH_INFINITE
	};

	static SearchAgent* getInstance();

	void newGame();

	const Board getBoard() const;
	void setBoard(Board _board);

	const bool shouldStop() const {
		return requestStop;
	}

	const bool getSearchInProgress() const {
		return searchInProgress;
	}

	void setSearchInProgress(bool _searchInProgress) {
		searchInProgress = _searchInProgress;
	}

	void setRequestStop(bool shouldStop) {
		requestStop = shouldStop;
	}

	bool getRequestStop() {
		return requestStop;
	}

	void setQuit(bool shouldQuit) {
		quit = shouldQuit;
	}

	const SearchMode getSearchMode() const {
		return searchMode;
	}

	void setSearchMode(SearchMode _searchMode) {
		searchMode = _searchMode;
	}

	void setPositionFromSAN(std::string startPosMoves);
	void setPositionFromFEN(std::string fenMoves);
	void startSearch();
	void doPerft();
	void doBench();
	void doEval();
	void stopSearch();

	inline const size_t getHashSize() const {
		return hashSize;
	}

	inline void setHashSize(size_t _hashSize) {
		hashSize = _hashSize;
	}

	inline const int getThreadNumber() const {
		return threadNumber;
	}

	inline void setThreadNumber(int _threadNumber) {
		threadNumber = _threadNumber;
	}

	inline const long getWhiteTime() const {
		return whiteTime;
	}

	inline void setWhiteTime(long _whiteTime) {
		whiteTime = _whiteTime;
	}

	inline const long getWhiteIncrement() const {
		return whiteIncrement;
	}

	inline void setWhiteIncrement(long _whiteIncrement) {
		whiteIncrement = _whiteIncrement;
	}

	inline const long getBlackTime() const {
		return blackTime;
	}

	inline void setBlackTime(long _blackTime) {
		blackTime = _blackTime;
	}

	inline const long getBlackIncrement() const {
		return blackIncrement;
	}

	inline void setBlackIncrement(long _blackIncrement) {
		blackIncrement = _blackIncrement;
	}

	inline const int getDepth() const {
		return depth;
	}

	inline void setDepth(int _depth) {
		depth = _depth;
	}

	inline const int getMovesToGo() const {
		return movesToGo;
	}

	inline void setMovesToGo(int _movesToGo) {
		movesToGo = _movesToGo;
	}

	inline const long getMoveTime() const {
		return moveTime;
	}

	inline void setMoveTime(long _moveTime) {
		moveTime = _moveTime;
	}

	inline const bool getPonder() const {
		return ponder;
	}

	inline void setPonder(bool _ponder) {
		ponder = _ponder;
	}

	void ponderHit();

	inline void clearHash() {
		transTable->clearHash();

	}

	inline bool hashPut(const Key _key, int value, int evalValue, const int depth, const int ply,
			const TranspositionTable::NodeFlag flag, MoveIterator::Move move) {
		TranspositionTable::HashKey key=static_cast<TranspositionTable::HashKey>(_key>>32);
		if (value >= maxScore-100) {
			value -= ply;
		} else if (value <= -maxScore+100) {
			value += ply;
		}
		return transTable->hashPut(key,value,evalValue,flag,move,depth);
	}

	inline bool hashGet(const Key _key, TranspositionTable::HashData& hashData, const int ply) {
		TranspositionTable::HashKey key=static_cast<TranspositionTable::HashKey>(_key>>32);
		bool result = transTable->hashGet(key, hashData);
		if (result) {
			int value = hashData.value();
			if (value >= maxScore-100) {
				hashData.setValue(value-ply);
			} else if (value <= -maxScore+100) {
				hashData.setValue(value+ply);
			}
		} else {
			hashData.clear();
		}
		return result;
	}

	inline bool hashPruneGet(bool& okToPrune, const Key _key, TranspositionTable::HashData& hashData, const int ply,
			const int depth, const bool allowNullMove, const int alpha, const int beta) {
		const bool result = hashGet(_key, hashData, ply);
		if (result) {
			okToPrune =
					(allowNullMove || !(hashData.flag() & TranspositionTable::NODE_NULL)) &&
					(hashData.depth()>=depth) &&
					(((hashData.flag() & TranspositionTable::LOWER) && hashData.value() >= beta) ||
					((hashData.flag() & TranspositionTable::UPPER) && hashData.value() <= alpha));
		} else {
			okToPrune = false;
		}
		return result;
	}

	inline bool isHashFull() {
		return transTable->isHashFull();
	}

	inline int hashFull() {
		return transTable->hashFull();
	}

	void createHash() {
		transTable = new TranspositionTable(mainHashName, getHashSize());
	}

	void destroyHash() {
		if (transTable) {
			delete transTable;
		}
	}

	void newSearchHash() {
		return transTable->newSearch();
	}

	void shutdown();
	void *startThreadSearch();
	void initThreads();

protected:

	SearchAgent();
	SearchAgent(const SearchAgent&);
	SearchAgent& operator= (const SearchAgent&);

private:

	static SearchAgent* searchAgent;
	Board board;
	SearchMode searchMode;
	volatile bool searchInProgress;
	volatile bool requestStop;
	volatile bool quit;
	size_t hashSize;
	int threadNumber;
	long whiteTime;
	long whiteIncrement;
	long blackTime;
	long blackIncrement;
	int depth;
	int movesToGo;
	long moveTime;
	bool ponder;
	TranspositionTable* transTable;
	const long getTimeToSearch();
};

#endif /* SEARCHAGENT_H_ */
