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

	/*	std::cout << "----- init " << std::endl;
	for (int x=0;x<_data.size;x++) {
		std::cout << _data.list[x].toString() << " - score: " << _data.list[x].score << std::endl;
	}
	std::cout << "----- end " << std::endl;*/

}
