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
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

#include <functional>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

#include "Board.h"

using namespace boost::interprocess;

#define DEFAULT_INITIAL_SIZE 3

template<class _Key, class _Value>
class TranspositionTable {
public:

	typedef _Key HashKeyType;
	typedef _Value HashValueType;

	typedef std::pair<const HashKeyType, HashValueType> ValueType;

	typedef allocator<ValueType, managed_shared_memory::segment_manager> ShmemAllocator;
	// Transposition Table type
	typedef boost::unordered_map<HashKeyType, HashValueType, boost::hash<HashKeyType> , std::equal_to<HashKeyType>, ShmemAllocator > CustomHashTable;

	TranspositionTable(std::string id_, managed_shared_memory* segment_) :
		transTable(NULL),
		segment(segment_),
		id(id_) {

		transTable = segment_->construct<CustomHashTable>(id_.c_str())
									( DEFAULT_INITIAL_SIZE, boost::hash<HashKeyType>() , std::equal_to<HashKeyType>()
											, segment_->get_allocator<ValueType>());
	}
	TranspositionTable(std::string id_, size_t initialSize, managed_shared_memory* segment_) :
		hashSize(initialSize),
		transTable(NULL),
		segment(segment_),
		id(id_) {
		transTable = segment_->construct<CustomHashTable>(id_.c_str())
							( DEFAULT_INITIAL_SIZE, boost::hash<HashKeyType>() , std::equal_to<HashKeyType>()
									, segment_->get_allocator<ValueType>());
	}

	virtual ~TranspositionTable() {
		if (segment) {
			try {
				segment->destroy<CustomHashTable>(getId().c_str());
			} catch (...) {
				std::cerr << "Error while trying to release HashTable" << std::endl;
			}

		}
	}

	const size_t getHashSize() const;
	void setHashSize(const size_t _hashSize);
	void clearHash();
	bool hashPut(const HashKeyType key, const HashValueType value);
	bool hashGet(const HashKeyType key, HashValueType& value);
	void resizeHash();
	bool isHashFull();
	managed_shared_memory* getSegment();
	const std::string getId() const;

private:
	size_t hashSize;
	CustomHashTable* transTable;
	managed_shared_memory* segment;
	std::string id;

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
	transTable->clear();
}

template<class HashKeyType, class HashValueType>
inline bool TranspositionTable<HashKeyType, HashValueType>::hashPut(const HashKeyType key, const HashValueType value) {

	if (transTable->size() >= hashSize) {
		return false; // hash full
	}
	transTable->insert(ValueType(key,value));
	return true;
}

template<class HashKeyType, class HashValueType>
inline bool TranspositionTable<HashKeyType, HashValueType>::hashGet(const HashKeyType key, HashValueType& value) {
	if (transTable->count(key)>0) {
		value = transTable->at(key);
		return true;
	}
	return false;
}

template<class HashKeyType, class HashValueType>
inline void TranspositionTable<HashKeyType, HashValueType>::resizeHash() {
	transTable->rehash(hashSize);
}

template<class HashKeyType, class HashValueType>
inline bool TranspositionTable<HashKeyType, HashValueType>::isHashFull() {
	return transTable->size() >= hashSize;
}

template<class HashKeyType, class HashValueType>
inline managed_shared_memory* TranspositionTable<HashKeyType, HashValueType>::getSegment() {
	return segment;
}

template<class HashKeyType, class HashValueType>
inline const std::string TranspositionTable<HashKeyType, HashValueType>::getId() const {
	return id;
}

#endif /* TRANSPOSITIONTABLE_H_ */
