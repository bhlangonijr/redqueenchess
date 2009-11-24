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
 * TranspositionTable.cpp
 *
 *  Created on: Nov 6, 2009
 *      Author: bhlangonijr
 */


#include "TranspositionTable.h"

TranspositionTable::TranspositionTable(std::string id_, managed_shared_memory* segment_) :
	transTable(NULL),
	segment(segment_),
	id(id_) {

	transTable = segment_->construct<HashTable>(id_.c_str())
						( DEFAULT_INITIAL_SIZE, boost::hash<Key>() , std::equal_to<Key>()
						, segment_->get_allocator<ValueType>());
}

TranspositionTable::TranspositionTable(std::string id_, size_t initialSize, managed_shared_memory* segment_) :
	hashSize(initialSize),
	transTable(NULL),
	segment(segment_),
	id(id_) {
	transTable = segment_->construct<HashTable>(id_.c_str())
				( DEFAULT_INITIAL_SIZE, boost::hash<Key>() , std::equal_to<Key>()
				, segment_->get_allocator<ValueType>());
}


TranspositionTable::~TranspositionTable() {
	if (getSegment()) {
		try {
			this->getSegment()->destroy<TranspositionTable::HashTable>(getId().c_str());
		} catch (...) {
			std::cerr << "Error while trying to release HashTable" << std::endl;
		}

	}
}
