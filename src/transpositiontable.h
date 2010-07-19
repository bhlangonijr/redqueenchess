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

#define DEFAULT_INITIAL_SIZE 64

template<class _Key, class _Value>
class TranspositionTable {
public:

	typedef _Key HashKeyType;
	typedef _Value HashValueType;

	typedef struct _Entry {
		_Key key;
		_Value value;
	} HashEntry;

	TranspositionTable(std::string id_) :
		hashSize(DEFAULT_INITIAL_SIZE),
		id(id_),
		transTable(0),
		writes(0) {
		init();
	}

	TranspositionTable(std::string id_, size_t initialSize) :
		hashSize(initialSize),
		id(id_),
		transTable(0),
		writes(0) {
		init();
	}

	virtual ~TranspositionTable() {
		delete [] transTable;
	}

	const size_t getHashSize() const;
	void setHashSize(const size_t _hashSize);
	void clearHash();
	bool hashPut(const HashKeyType key, const HashValueType value);
	bool hashGet(const HashKeyType key, HashValueType& value);
	void resizeHash();
	bool isHashFull();
	const int hashFull();
	const std::string getId() const;
	void newSearch();

private:

	void init() {
		for (_size = 2; _size<=hashSize; _size *= 2);
		_size = ((_size/2)<<20)/sizeof(HashEntry);
		_mask = _size - 1;

		try {
			transTable = new HashEntry[_size];
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
	HashEntry* transTable;
	size_t writes;

};

template<class HashKeyType, class HashValueType>
inline const size_t TranspositionTable<HashKeyType, HashValueType>::getHashSize() const {
	return hashSize;
}

template<class HashKeyType, class HashValueType>
inline void TranspositionTable<HashKeyType, HashValueType>::setHashSize(const size_t _hashSize) {
	hashSize = _hashSize;
}

template<class HashKeyType, class HashValueType>
inline void TranspositionTable<HashKeyType, HashValueType>::clearHash() {
	memset(transTable, 0, _size * sizeof(HashEntry));
}

template<class HashKeyType, class HashValueType>
inline bool TranspositionTable<HashKeyType, HashValueType>::hashPut(const HashKeyType key, const HashValueType value) {
	HashEntry *entry;
	entry = transTable + (size_t(key) & _mask);
	if (!entry) {
		return false;
	}
	entry->key = key;
	entry->value = value;
	writes++;
	return true;
}

template<class HashKeyType, class HashValueType>
inline bool TranspositionTable<HashKeyType, HashValueType>::hashGet(const HashKeyType key, HashValueType& value) {

	HashEntry *entry;
	entry = transTable + (size_t(key) & _mask);
	if (entry->key==key) {
		value = entry->value;
		return true;
	}

	return false;
}

template<class HashKeyType, class HashValueType>
inline void TranspositionTable<HashKeyType, HashValueType>::resizeHash() {
	writes=0;
	_size=0;
	_mask=0;
	if (transTable) {
		delete [] transTable;
	}
	init();

}

template<class HashKeyType, class HashValueType>
inline bool TranspositionTable<HashKeyType, HashValueType>::isHashFull() {
	return false;
}

template<class HashKeyType, class HashValueType>
inline const int TranspositionTable<HashKeyType, HashValueType>::hashFull() {
	double n = double(_size);
	return int(1000 * (1 - exp(writes * log(1.0 - 1.0/n))));
}

template<class HashKeyType, class HashValueType>
inline const std::string TranspositionTable<HashKeyType, HashValueType>::getId() const {
	return id;
}

template<class HashKeyType, class HashValueType>
inline void TranspositionTable<HashKeyType, HashValueType>::newSearch() {
	writes=0;
}

#endif /* TRANSPOSITIONTABLE_H_ */
