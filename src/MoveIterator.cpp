/*
 * MoveIterator.cpp
 *
 *  Created on: Dec 26, 2009
 *      Author: bhlangonijr
 */

#include "MoveIterator.h"

MoveIterator::MoveIterator() : _size(0), idx(0) {
	// TODO Auto-generated constructor stub

}

MoveIterator::~MoveIterator() {
	// TODO Auto-generated destructor stub
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
/*	for (uint32_t x=0;x<_size;x++) {
		std::cout << list[x].toString() << " - score: " << list[x].score << std::endl;
	}*/

}
