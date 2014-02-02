/*
	Redqueen Chess Engine
    Copyright (C) 2008-2012 Ben-Hur Carlos Vieira Langoni Junior

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
 * TranspositionTable.h
 *
 *  Created on: Nov 6, 2009
 *      Author: bhlangonijr
 */

#ifndef TRANSPOSITIONTABLE_H_
#define TRANSPOSITIONTABLE_H_
#include <cstring>
#include <cmath>
#include <stdlib.h>
const int DEFAULT_INITIAL_SIZE = 128;
const int BUCKET_SIZE = 4;
class TranspositionTable {
public:

	typedef uint32_t HashKey;

	enum NodeFlag {
		NODE_NONE=0,
		LOWER=1,
		UPPER=2,
		EXACT=4,
		NODE_NULL=8,
		NODE_EVAL=16,
		NM_LOWER=LOWER|NODE_NULL,
		LOWER_EVAL=LOWER|NODE_EVAL,
		UPPER_EVAL=UPPER|NODE_EVAL,
		EXACT_EVAL=EXACT|NODE_EVAL,
		NM_LOWER_EVAL=NM_LOWER|NODE_EVAL
	};

	struct HashData {
		HashData() : key(0),_value(-maxScore), _evalValue(-maxScore), _depth(0), _flag(NODE_NONE),
				_from(NONE), _to(NONE), _promotion(EMPTY), _generation(0) {};

		inline void update(const HashKey hashKey, const int& value, const int& evalValue, const int& depth,
				const NodeFlag& flag, const MoveIterator::Move& move, const int& generation) {
			key=hashKey;
			_value=int16_t(value);
			_evalValue=int16_t(evalValue);
			_depth=int16_t(depth);
			_flag=int8_t(flag);
			_from=int8_t(move.from);
			_to=int8_t(move.to);
			_promotion=int8_t(move.promotionPiece);
			_generation=int16_t(generation);
		}
		inline NodeFlag flag() {
			return NodeFlag(_flag);
		}
		inline MoveIterator::Move move() {
			return MoveIterator::Move(Square(_from),Square(_to),
					PieceTypeByColor(_promotion), MoveIterator::TT_MOVE);
		}
		inline int depth() {
			return (int)(_depth);
		}
		inline int value() {
			return (int)(_value);
		}
		inline int evalValue() {
			return (int)(_evalValue);
		}
		inline int generation() {
			return (int)(_generation);
		}
		inline void setValue(int value) {
			_value=int16_t(value);
		}
		inline void clear() {
			key=0L;
			_value=-maxScore;
			_evalValue=-maxScore;
			_depth=-80;
			_flag=uint8_t(NODE_NONE);
			_from=uint8_t(NONE);
			_to=uint8_t(NONE);
			_promotion=uint8_t(EMPTY);
			_generation=0;
		}
		HashKey key;
		int16_t _value;
		int16_t _evalValue;
		int16_t _depth;
		uint8_t _flag;
		uint8_t _from;
		uint8_t _to;
		uint8_t _promotion;
		uint16_t _generation;

	};

	struct Bucket {
		HashData entry[BUCKET_SIZE];
	} __attribute__ ((aligned(64)));

	TranspositionTable(std::string id_) :
		hashSize(DEFAULT_INITIAL_SIZE),
		id(id_),
		transTable(0),
		writes(0),
		generation(0) {
		init();
	}

	TranspositionTable(std::string id_, size_t initialSize) :
		hashSize(initialSize),
		id(id_),
		transTable(0),
		writes(0),
		generation(0) {
		init();
	}

	virtual ~TranspositionTable() {
		delete [] transTable;
	}

	const uint16_t getGeneration() const;
	const size_t getHashSize() const;
	void setHashSize(const size_t _hashSize);
	void clearHash();
	bool hashPut(const HashKey key, const int value, const int evalValue, const NodeFlag flag,
			MoveIterator::Move& move, const int depth);
	bool hashGet(const HashKey key, HashData& hashData);
	void resizeHash();
	bool isHashFull();
	const int hashFull();
	const std::string getId() const;
	void newSearch();

private:

	void init() {
		for (_size = 2; _size<=hashSize; _size *= 2);
		_size = ((_size/2)<<20)/sizeof(Bucket);
		_mask = _size - 1;
		try {
			transTable = new Bucket[_size];
		} catch (std::exception const& e) {
			std::cerr << "Failed to allocate memory for transposition table: " << e.what() << std::endl;
			exit(EXIT_FAILURE);
		}
		clearHash();
	}
	size_t hashSize;
	std::string id;
	size_t _size;
	size_t _mask;
	Bucket* transTable;
	size_t writes;
	uint16_t generation;
};

inline const uint16_t TranspositionTable::getGeneration() const {
	return generation;
}

inline const size_t TranspositionTable::getHashSize() const {
	return hashSize;
}

inline void TranspositionTable::setHashSize(const size_t _hashSize) {
	hashSize = _hashSize;
}

inline void TranspositionTable::clearHash() {
	generation=0;
	memset(transTable, 0, _size * sizeof(Bucket));
	for(size_t i=0;i<_size;i++) {
		HashData *entry = transTable[i].entry;
		for (int x=0;x<BUCKET_SIZE;x++,entry++) {
			entry->clear();
		}
	}
}

inline bool TranspositionTable::hashPut(const HashKey key, const int value, const int evalValue,
		const NodeFlag flag, MoveIterator::Move& move, const int depth) {
	HashData *entry = transTable[key & _mask].entry;
	HashData *replace = entry;
	for (int x=0; x<BUCKET_SIZE; ++x, ++entry) {
		if (!entry->key || entry->key==key) {
			if (move.none()) {
				move=entry->move();
			}
			replace=entry;
			break;
		}
		if ((entry->generation()<replace->generation()) ||
				(entry->generation()==replace->generation() &&
				replace->depth()>entry->depth() &&
				!(entry->flag() & TranspositionTable::EXACT))) {
			replace=entry;
		}
	}
	replace->update(key,value,evalValue,depth,flag,move,generation);
	writes++;
	return true;
}

inline bool TranspositionTable::hashGet(const HashKey key, HashData& hashData) {
	HashData *entry = transTable[key & _mask].entry;
	for (int x=0;x<BUCKET_SIZE;x++,entry++) {
		if (entry->key==key) {
			hashData=*entry;
			return true;
		}
	}
	return false;
}

inline void TranspositionTable::resizeHash() {
	writes=0;
	_size=0;
	_mask=0;
	generation=0;
	if (transTable) {
		delete [] transTable;
	}
	init();

}

inline bool TranspositionTable::isHashFull() {
	return false;
}

inline const int TranspositionTable::hashFull() {
	double n = double(_size);
	return (int)(1000 * (1 - exp(writes * log(1.0 - 1.0/n))));
}

inline const std::string TranspositionTable::getId() const {
	return id;
}

inline void TranspositionTable::newSearch() {
	generation++;
	writes=0;
}

#endif /* TRANSPOSITIONTABLE_H_ */
