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
const int defaultGameSize			= 35;
const int timeTableSize=7;

const int timeTable [7][3] = {
		{25, 900000, 180000},
		{23, 180000, 60000},
		{21, 30000, 10000},
		{19, 10000, 5000},
		{17, 5000, 1000},
		{15, 1000, 0}
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

	inline const bool getInfinite() const {
		return infinite;
	}

	inline void setInfinite(bool _infinite) {
		infinite = _infinite;
	}

	inline void clearHash() {
		transTable->clearHash();

	}

	inline bool hashPut(const Key _key, int value, const int depth, const int ply,
			const TranspositionTable::NodeFlag flag, MoveIterator::Move move) {

		TranspositionTable::HashKey key=TranspositionTable::HashKey(_key>>32);

		if (value >= maxScore-100) {
			value -= ply;
		} else if (value <= -maxScore+100) {
			value += ply;
		}

		return transTable->hashPut(key,value,flag,move,depth);

	}

	inline bool hashGet(const Key _key, TranspositionTable::HashData& hashData, const int ply) {

		TranspositionTable::HashKey key=TranspositionTable::HashKey(_key>>32);

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
	bool infinite;
	TranspositionTable* transTable;

	const long getTimeToSearch();

};

#endif /* SEARCHAGENT_H_ */
