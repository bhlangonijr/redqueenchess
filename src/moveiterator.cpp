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
 * MoveIterator.cpp
 *
 *  Created on: Dec 26, 2009
 *      Author: bhlangonijr
 */

#include "moveiterator.h"

MoveIterator::MoveIterator(Data& data) : _data(data) {

}

MoveIterator::MoveIterator() : _data(internal) {

}

MoveIterator::~MoveIterator() {
}

// sort
void MoveIterator::sort() {

	bool flag=true;
	for(int i = 0; i <(int)_data.size&&flag; i++){
		flag=false;
		for(int j = 0; j <(int) _data.size-1; j++)
		{
			if (_data.list[j+1].score > _data.list[j].score) {
				Move tmp=_data.list[j];
				_data.list[j]=_data.list[j+1];
				_data.list[j+1]=tmp;
				flag=true;
			}

		}
	}
}

// sort after an index
void MoveIterator::sort(const int after) {

	bool flag=true;
	for(int i = after; i <(int)_data.size&&flag; i++){
		flag=false;
		for(int j = after; j <(int) _data.size-1; j++)
		{
			if (_data.list[j+1].score > _data.list[j].score) {
				Move tmp=_data.list[j];
				_data.list[j]=_data.list[j+1];
				_data.list[j+1]=tmp;
				flag=true;
			}

		}
	}

}

// sort root moves
void MoveIterator::sortRootMoves(long moveScore[MOVE_LIST_MAX_SIZE]) {

	bool flag=true;
	for(int i = 0; i <(int)_data.size&&flag; i++){
		flag=false;
		for(int j = 0; j <(int) _data.size-1; j++)
		{
			if (moveScore[j+1] > moveScore[j]) {
				Move tmp=_data.list[j];
				_data.list[j]=_data.list[j+1];
				_data.list[j+1]=tmp;

				long tmpLong = moveScore[j];
				moveScore[j]=moveScore[j+1];
				moveScore[j+1]=tmpLong;

				flag=true;
			}
		}
	}

	/*for(int i = 0; i <(int)_data.size; i++){
		std::cout << "move " << i << " " << _data.list[i].toString() << " score: " << moveScore[i] << std::endl;
	}*/
}
