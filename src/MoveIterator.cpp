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
 * MoveIterator.cpp
 *
 *  Created on: Dec 26, 2009
 *      Author: bhlangonijr
 */

#include "MoveIterator.h"

MoveIterator::MoveIterator() : _size(0), idx(0) {

}

MoveIterator::~MoveIterator() {
}

//sort
void MoveIterator::sort() {
	bool flag=true;
	for(uint32_t i = 0; i < _size&&flag; i++){
		flag=false;
		for(uint32_t j = 0; j < _size-1; j++)
		{
			if (list[j+1].score > list[j].score) {
				Move tmp=list[j];
				list[j]=list[j+1];
				list[j+1]=tmp;
				flag=true;
			}

		}
	}
//	for (uint32_t x=0;x<_size;x++) {
//		std::cout << list[x].toString() << " - score: " << list[x].score << std::endl;
//	}

}
