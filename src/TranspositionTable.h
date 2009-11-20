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
 * TranspositionTable.h
 *
 *  Created on: Nov 6, 2009
 *      Author: bhlangonijr
 */

#ifndef TRANSPOSITIONTABLE_H_
#define TRANSPOSITIONTABLE_H_

#include <boost/unordered_map.hpp>
#include <iostream>
#include <inttypes.h>

#include "Board.h"

struct HashFunction {
	size_t operator()( const Key& key ) const
	{
#if  defined(__MINGW32__) || defined(__MINGW64__)
	return boost::hash<uint32_t>()(key);
#else
	return boost::hash<uint64_t>()(key);
#endif
	}
	bool operator()( const Key& key1, const Key& key2 ) const
	{
		return key1==key2;
	}
};

struct HashData {
	HashData() : value(0), depth(0), generation(0)  {};
	HashData(int _value, uint32_t _depth, uint32_t _generation) : value(_value), depth(_depth), generation(_generation)  {};
	int value;
	uint32_t depth;
	uint32_t generation;
};

// Transposition Table type
typedef boost::unordered_map<Key, HashData, HashFunction > HashTable;

class TranspositionTable {
public:

	TranspositionTable();
	TranspositionTable(size_t initialSize);
	virtual ~TranspositionTable();

	const size_t getHashSize() const {
		return hashSize;
	}
	void setHashSize(const size_t _hashSize) {
		hashSize = _hashSize;
	}
	void clearHash() {
		transTable.clear();
	}

	bool hashPut(const Board& board, const int value, const uint32_t depth, const uint32_t generation) {

		if (transTable.size() >= hashSize) {
			return false; // hash full
		}

		transTable[board.getKey()] = HashData(value, depth, generation);
		return true;
	}

	bool hashGet(const Key _key, HashData& hashData) {

		if (transTable.count(_key)>0) {
			hashData = transTable[_key];
			return true;
		}
		return false;
	}

	void resizeHash() {

		transTable.rehash(hashSize);

	}

	bool isHashFull() {

		return transTable.size() >= hashSize;
	}

private:
	size_t hashSize;
	HashTable transTable;
};

#endif /* TRANSPOSITIONTABLE_H_ */
