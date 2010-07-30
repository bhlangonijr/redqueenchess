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

class SimplePVSearch;

class SearchAgent {

public:

	typedef uint32_t HashKey;

	enum SearchMode {
		SEARCH_TIME, SEARCH_DEPTH, SEARCH_MOVESTOGO, SEARCH_MOVETIME, SEARCH_MOVES, SEARCH_INFINITE
	};

	enum NodeFlag {
		LOWER=0, UPPER, EXACT, NM_LOWER
	};

	struct HashData {

		HashData() : _value(0), _depth(0), _flag(LOWER),
				_from(NONE), _to(NONE), _promotion(EMPTY), _generation(0) {};

		HashData(const int& value, const int& depth, const NodeFlag& flag, const MoveIterator::Move& move) :
			_value(int16_t(value)), _depth(int16_t(depth)), _flag(uint8_t(flag)),
			_from(uint8_t(move.from)), _to(uint8_t(move.to)),_promotion(uint8_t(move.promotionPiece)), _generation (0)  {};

		inline NodeFlag flag() {
			return NodeFlag(_flag);
		}
		inline MoveIterator::Move move() {
			return MoveIterator::Move(Square(_from),Square(_to),
					PieceTypeByColor(_promotion), MoveIterator::TT_MOVE);
		}
		inline int depth() {
			return int(_depth);
		}
		inline int value() {
			return int(_value);
		}
		inline void setValue(int value) {
			_value=int16_t(value);
		}
		inline void clear() {
			_value=0;
			_depth=0;
			_flag=uint8_t(LOWER);
			_from=uint8_t(NONE);
			_to=uint8_t(NONE);
			_promotion=uint8_t(EMPTY);
			_generation=0;
		}

		int16_t _value;
		int16_t _depth;
		uint8_t _flag;
		uint8_t _from;
		uint8_t _to;
		uint8_t _promotion;
		uint8_t _generation;
	};


	/*struct HashData {
		HashData() : _value(0), _depth(0), _flag(LOWER)  {};
		HashData(const int& value, const int& depth, const NodeFlag& flag, const MoveIterator::Move& move) :
			_value(value), _depth(depth), _flag(flag), _move(move)  {};

		HashData(const HashData& hashData) : _value(hashData._value), _depth(hashData._depth),
				_flag(hashData._flag), _move(hashData._move)  {};

		int _value;
		int _depth;
		NodeFlag _flag;
		MoveIterator::Move _move;

		NodeFlag& flag() {
			return _flag;
		}
		MoveIterator::Move& move() {
			return _move;
		}
		int& depth() {
			return _depth;
		}
		int& value() {
			return _value;
		}
		void setValue(int value) {
			_value=value;
		}
		inline void clear() {
			_value=0;
			_depth=0;
			_flag=LOWER;
		}

	};*/

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

	inline bool hashPut(const Key _key, int value, const int depth, const int ply, const NodeFlag flag, const MoveIterator::Move& move) {

		if (value >= maxScore-100) {
			value -= ply;
		} else if (value <= -maxScore+100) {
			value += ply;
		}
		MoveIterator::Move ttMove(move.from,move.to,move.promotionPiece,MoveIterator::TT_MOVE);
		return transTable->hashPut(HashKey(_key), HashData(value,depth,flag,ttMove));

	}

	inline bool hashGet(const Key _key, HashData& hashData, const int ply) {

		bool result = transTable->hashGet(HashKey(_key), hashData);
		if (result) {
			int value = hashData.value();
			if (value >= maxScore-100) {
				hashData.setValue(value-ply);
			} else if (value <= -maxScore+100) {
				hashData.setValue(value+ply);
			}
		}else {
			hashData.clear();
		}

		return result;
	}

	inline bool hashPruneGet(const Key _key, HashData& hashData, const int ply,
			const int depth, const bool allowNullMove, const int beta) {

		bool result = hashGet(HashKey(_key), hashData, ply);

		if (result) {
			result =((allowNullMove && hashData.depth()>=depth) ||
					(hashData.value() >= MAX(maxScore-100,beta)) ||
					(hashData.value() < MIN(-maxScore+100,beta))) &&
					((hashData.flag() == LOWER && hashData.value() >= beta) ||
							(hashData.flag() == UPPER && hashData.value() < beta));
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
		transTable = new TranspositionTable<HashKey,HashData>(mainHashName, getHashSize());
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

	TranspositionTable<HashKey,HashData>* transTable;

	const long getTimeToSearch();

};

#endif /* SEARCHAGENT_H_ */
