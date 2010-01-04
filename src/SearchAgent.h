/*
	Redqueen Chess Engine
    Copyright (C) 2008-2009 Ben-Hur Carlos Vieira Langoni Junior

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
#include "Uci.h"
#include "Board.h"
#include "SimplePVSearch.h"
#include "TranspositionTable.h"

static const std::string mainHashName 		= "DefaultHashTable";
static const size_t defaultDepth			= 5;
static const size_t defaultHashSize			= 64;
static const size_t defaultGameSize			= 40;
static const size_t defaultGameSizeInc		= 5;

class SearchAgent {
public:
	enum SearchMode {
		SEARCH_TIME, SEARCH_DEPTH, SEARCH_MOVESTOGO, SEARCH_MOVETIME, SEARCH_MOVES, SEARCH_INFINITE
	};

	enum NodeFlag {
		LOWER, UPPER, EXACT
	};

	struct HashData {
		HashData() : value(0), depth(0), generation(0), flag(LOWER)  {};
		HashData(const int& _value, const uint32_t& _depth, const uint32_t& _generation, const NodeFlag& _flag, const MoveIterator::Move& _move) :
			value(_value), depth(_depth), generation(_generation), flag(_flag), move(_move)  {};
		HashData(const HashData& hashData) : value(hashData.value), depth(hashData.depth), generation(hashData.generation), flag(hashData.flag), move(hashData.move)  {};
		int value;
		uint32_t depth;
		uint32_t generation;
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
	void stopSearch();

	const size_t getHashSize() const {
		return hashSize;
	}
	void setHashSize(size_t _hashSize) {
		hashSize = _hashSize;
	}

	const int getThreadNumber() const {
		return threadNumber;
	}
	void setThreadNumber(int _threadNumber) {
		threadNumber = _threadNumber;
	}

	const uint32_t getWhiteTime() const {
		return whiteTime;
	}
	void setWhiteTime(uint32_t _whiteTime) {
		whiteTime = _whiteTime;
	}

	const uint32_t getWhiteIncrement() const {
		return whiteIncrement;
	}
	void setWhiteIncrement(uint32_t _whiteIncrement) {
		whiteIncrement = _whiteIncrement;
	}

	const uint32_t getBlackTime() const {
		return blackTime;
	}
	void setBlackTime(uint32_t _blackTime) {
		blackTime = _blackTime;
	}

	const uint32_t getBlackIncrement() const {
		return blackIncrement;
	}
	void setBlackIncrement(uint32_t _blackIncrement) {
		blackIncrement = _blackIncrement;
	}

	const int getDepth() const {
		return depth;
	}
	void setDepth(int _depth) {
		depth = _depth;
	}

	const uint32_t getMovesToGo() const {
		return movesToGo;
	}
	void setMovesToGo(uint32_t _movesToGo) {
		movesToGo = _movesToGo;
	}

	const uint32_t getMoveTime() const {
		return moveTime;
	}
	void setMoveTime(uint32_t _moveTime) {
		moveTime = _moveTime;
	}

	const bool getInfinite() const {
		return infinite;
	}
	void setInfinite(int _infinite) {
		infinite = _infinite;
	}
	void clearHash() {
		if (transTable.size()>getActiveHash()) {
			transTable[getActiveHash()]->clearHash();
		}
	}
	bool hashPut(const Board& board, const int& value, const uint32_t& depth, const uint32_t& generation, const NodeFlag& flag, const MoveIterator::Move& move) {
		if (transTable.size()>getActiveHash()) {
			return transTable[getActiveHash()]->hashPut(board.getKey(), HashData(value,depth,generation,flag,move));
		}
		return false;
	}

	bool hashGet(const Key _key, HashData& hashData) {
		if (transTable.size()>getActiveHash()) {
			return transTable[getActiveHash()]->hashGet(_key, hashData);
		}
		return false;

	}

	bool isHashFull() {

		if (transTable.size()>getActiveHash()) {
			return transTable[getActiveHash()]->isHashFull();
		}
		return true;
	}

	int hashFull() {

		if (transTable.size()>getActiveHash()) {
			return transTable[getActiveHash()]->hashFull();
		}
		return true;
	}

	void addTranspositionTable(TranspositionTable<Key,HashData>* table) {
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
	uint32_t threadNumber;
	uint32_t whiteTime;
	uint32_t whiteIncrement;
	uint32_t blackTime;
	uint32_t blackIncrement;
	int depth;
	uint32_t movesToGo;
	uint32_t moveTime;
	bool infinite;

	size_t activeHash;
	std::vector<TranspositionTable<Key,HashData>*> transTable;

	const uint32_t getTimeToSearch();

};

#endif /* SEARCHAGENT_H_ */
