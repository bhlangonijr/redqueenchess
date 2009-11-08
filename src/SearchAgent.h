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

#include <boost/unordered_map.hpp>
#include <iostream>
#include <assert.h>
#include "Uci.h"
#include "Board.h"
#include "SimplePVSearch.h"
#include "TranspositionTable.h"

namespace SearchAgentTypes {

enum SearchMode {
	SEARCH_TIME, SEARCH_DEPTH, SEARCH_MOVESTOGO, SEARCH_MOVETIME, SEARCH_MOVES, SEARCH_INFINITE
};

}

using namespace SearchAgentTypes;

class SearchAgent {
public:

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

	const int getWhiteTime() const {
		return whiteTime;
	}
	void setWhiteTime(int _whiteTime) {
		whiteTime = _whiteTime;
	}

	const int getWhiteIncrement() const {
		return whiteIncrement;
	}
	void setWhiteIncrement(int _whiteIncrement) {
		whiteIncrement = _whiteIncrement;
	}

	const int getBlackTime() const {
		return blackTime;
	}
	void setBlackTime(int _blackTime) {
		blackTime = _blackTime;
	}

	const int getBlackIncrement() const {
		return blackIncrement;
	}
	void setBlackIncrement(int _blackIncrement) {
		blackIncrement = _blackIncrement;
	}

	const int getDepth() const {
		return depth;
	}
	void setDepth(int _depth) {
		depth = _depth;
	}

	const int getMovesToGo() const {
		return movesToGo;
	}
	void setMovesToGo(int _movesToGo) {
		movesToGo = _movesToGo;
	}

	const int getMoveTime() const {
		return moveTime;
	}
	void setMoveTime(int _moveTime) {
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
			transTable[getActiveHash()].clearHash();
		}
	}
	bool hashPut(const Board board, const int value, const uint32_t depth, const uint32_t generation) {
		if (transTable.size()>getActiveHash()) {
			return transTable[getActiveHash()].hashPut(board, value, depth, generation);
		}
		return false;
	}

	bool hashGet(const Key _key, HashData& hashData) {
		if (transTable.size()>getActiveHash()) {
			return transTable[getActiveHash()].hashGet(_key, hashData);
		}
		return false;

	}

	void resizeHash() {
		if (transTable.size()>getActiveHash()) {
			transTable[getActiveHash()].resizeHash();
		}
	}

	bool isHashFull() {
		if (transTable.size()>getActiveHash()) {
			return transTable[getActiveHash()].isHashFull();
		}
		return false;
	}

	void addTranspositionTable(TranspositionTable& table) {
		transTable.push_back(table);
	}

	inline int getActiveHash() {
		return activeHash;
	}

	inline void setActiveHash(const int active) {
		activeHash = active;
	}

	void relaseHash() {

		for(int x=0;x<transTable.size();x++) {
			transTable[x].clearHash();
			//delete transTable[x];
		}
		transTable.clear();
	}

protected:

	SearchAgent();
	SearchAgent(const SearchAgent&);
	SearchAgent& operator= (const SearchAgent&);

private:

	static SearchAgent* searchAgent;
	Board board;
	SearchMode searchMode;

	bool searchInProgress;

	size_t hashSize;
	int threadNumber;
	int whiteTime;
	int whiteIncrement;
	int blackTime;
	int blackIncrement;
	int depth;
	int movesToGo;
	int moveTime;
	bool infinite;

	int activeHash;
	std::vector< TranspositionTable> transTable;

};

#endif /* SEARCHAGENT_H_ */
