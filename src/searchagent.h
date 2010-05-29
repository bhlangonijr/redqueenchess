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

#include "board.h"
#include "simplepvsearch.h"
#include "transpositiontable.h"
#include "uci.h"

const std::string mainHashName 		= "DefaultHashTable";
const size_t defaultDepth			= 5;
const size_t defaultHashSize		= 64;
const int defaultGameSize			= 30;
const int timeTableSize=7;

const int timeTable [7][3] = {
		{27, 900000, 180000},
		{25, 180000, 60000},
		{23, 30000, 10000},
		{21, 10000, 5000},
		{19, 5000, 1000},
		{17, 1000, 0}
};

class SearchAgent {
public:
	enum SearchMode {
		SEARCH_TIME, SEARCH_DEPTH, SEARCH_MOVESTOGO, SEARCH_MOVETIME, SEARCH_MOVES, SEARCH_INFINITE
	};

	enum NodeFlag {
		LOWER, UPPER, EXACT
	};

	struct HashData {
		HashData() : value(0), depth(0), flag(LOWER)  {};
		HashData(const int& _value, const int& _depth, const NodeFlag& _flag, const MoveIterator::Move& _move) :
			value(_value), depth(_depth), flag(_flag), move(_move)  {};
		HashData(const HashData& hashData) : value(hashData.value), depth(hashData.depth), flag(hashData.flag), move(hashData.move)  {};
		int value;
		int depth;
		NodeFlag flag;
		MoveIterator::Move move;
	};

	static SearchAgent* getInstance();
	void newGame();

	const Board getBoard() const;
	void setBoard(Board _board);

	const bool getSearchInProgress() const {
		return searchInProgress;
	}
	void setSearchInProgress(int _searchInProgress) {
		searchInProgress = _searchInProgress;
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

	inline const int getWhiteTime() const {
		return whiteTime;
	}

	inline void setWhiteTime(int _whiteTime) {
		whiteTime = _whiteTime;
	}

	inline const int getWhiteIncrement() const {
		return whiteIncrement;
	}

	inline void setWhiteIncrement(int _whiteIncrement) {
		whiteIncrement = _whiteIncrement;
	}

	inline const int getBlackTime() const {
		return blackTime;
	}

	inline void setBlackTime(int _blackTime) {
		blackTime = _blackTime;
	}

	inline const int getBlackIncrement() const {
		return blackIncrement;
	}

	inline void setBlackIncrement(int _blackIncrement) {
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

	inline const int getMoveTime() const {
		return moveTime;
	}

	inline void setMoveTime(int _moveTime) {
		moveTime = _moveTime;
	}

	inline const bool getInfinite() const {
		return infinite;
	}

	inline void setInfinite(int _infinite) {
		infinite = _infinite;
	}

	inline void clearHash() {
		if (transTable.size()>getActiveHash()) {
			transTable[getActiveHash()]->clearHash();
		}
	}

	inline bool hashPut(const Board& board, int value, const int& depth, const int ply, const int maxScore, const NodeFlag& flag, const MoveIterator::Move& move) {
		if (transTable.size()>getActiveHash()) {

			if (value >= maxScore) {
				value -= ply;
			} else if (value <= -maxScore) {
				value += ply;
			}

			return transTable[getActiveHash()]->hashPut(board.getKey(), HashData(value,depth,flag,move));
		}
		return false;
	}

	inline bool hashGet(const Key _key, HashData& hashData, const int ply, const int maxScore) {
		if (transTable.size()>getActiveHash()) {

			bool result = transTable[getActiveHash()]->hashGet(_key, hashData);

			if (hashData.value >= maxScore) {
				hashData.value -= ply;
			} else if (hashData.value <= -maxScore) {
				hashData.value += ply;
			}

			return result;
		}
		return false;

	}

	inline bool isHashFull() {

		if (transTable.size()>getActiveHash()) {
			return transTable[getActiveHash()]->isHashFull();
		}
		return true;
	}

	inline int hashFull() {

		if (transTable.size()>getActiveHash()) {
			return transTable[getActiveHash()]->hashFull();
		}
		return true;
	}

	inline void addTranspositionTable(TranspositionTable<Key,HashData>* table) {
		transTable.push_back(table);
	}

	inline size_t getActiveHash() {
		return activeHash;
	}

	inline void setActiveHash(const size_t active) {
		activeHash = active;
	}

	void createHash() {
		TranspositionTable<Key,HashData>* table = new TranspositionTable<Key,HashData>(mainHashName, getHashSize());
		addTranspositionTable(table);
	}

	void destroyHash() {
		if (transTable.size()>getActiveHash()) {
			delete transTable[getActiveHash()];
		}
		transTable.clear();
	}

	void newSearchHash() {
		if (transTable.size()>getActiveHash()) {
			return transTable[getActiveHash()]->newSearch();
		}
	}

	void *startThreadSearch();

protected:

	SearchAgent();
	SearchAgent(const SearchAgent&);
	SearchAgent& operator= (const SearchAgent&);

private:

	static SearchAgent* searchAgent;
	Board board;
	SearchMode searchMode;

	volatile bool searchInProgress;

	size_t hashSize;
	int threadNumber;
	long whiteTime;
	long whiteIncrement;
	long blackTime;
	long blackIncrement;
	int depth;
	int movesToGo;
	int moveTime;
	bool infinite;

	size_t activeHash;
	std::vector<TranspositionTable<Key,HashData>*> transTable;

	const long getTimeToSearch();

};

#endif /* SEARCHAGENT_H_ */
