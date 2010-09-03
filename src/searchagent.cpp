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
 * SearchAgent.cpp
 *
 *  Created on: 01/05/2009
 *      Author: bhlangonijr
 */

#include "searchagent.h"

extern "C" void *threadStartup(void *);

SearchAgent* SearchAgent::searchAgent = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

SearchAgent* SearchAgent::getInstance ()
{
	if (searchAgent == 0) {
		searchAgent = new SearchAgent();
	}
	return searchAgent;
}

SimplePVSearch simpleSearcher;

SearchAgent::SearchAgent() :
							searchMode(SEARCH_TIME), searchInProgress(false), requestStop(false),
							hashSize(defaultHashSize),	threadNumber(1), whiteTime(0), whiteIncrement(0), blackTime(0),
							blackIncrement(0), depth(defaultDepth), movesToGo(0), moveTime(0), infinite(false),
							generation(1) {
	// creates initial hashtables
	this->createHash();
}

// start a new game
void SearchAgent::newGame() {
	pthread_mutex_lock(&mutex);

	board.setInitialPosition();

	this->clearHash();

	this->setSearchMode(SEARCH_TIME);
	this->setWhiteTime(0);
	this->setWhiteIncrement(0);
	this->setBlackTime(0);
	this->setBlackIncrement(0);

	this->setDepth(defaultDepth);
	this->setMovesToGo(0);
	this->setMoveTime(0);
	this->setInfinite(false);

	pthread_mutex_unlock(&mutex);

}

// get board
const Board SearchAgent::getBoard() const {
	return board;
}

// set board
void SearchAgent::setBoard(Board _board) {
	board = _board;
}

// set position from SAN moves
void SearchAgent::setPositionFromSAN(std::string startPosMoves) {
	board.loadFromString(startPosMoves);
}

// set position from FEN moves
void SearchAgent::setPositionFromFEN(std::string fenMoves) {
	board.loadFromFEN(fenMoves);
}

// start search
void* SearchAgent::startThreadSearch() {
	pthread_mutex_lock(&mutex);

	setSearchInProgress(true);
	newSearchHash();
	if (this->getSearchMode()==SearchAgent::SEARCH_DEPTH) {
		simpleSearcher.setInfinite(false);
		simpleSearcher.setSearchFixedDepth(true);
		simpleSearcher.setDepth(this->getDepth());
	} else if (this->getSearchMode()==SearchAgent::SEARCH_TIME ||
			this->getSearchMode()==SearchAgent::SEARCH_MOVETIME) {
		simpleSearcher.setInfinite(false);
		simpleSearcher.setSearchFixedDepth(false);
		simpleSearcher.setTimeToSearch(this->getTimeToSearch());
	} else if (this->getSearchMode()==SearchAgent::SEARCH_INFINITE) {
		simpleSearcher.setInfinite(true);
		simpleSearcher.setDepth(maxSearchDepth);
	} else {
		simpleSearcher.setInfinite(false);
		simpleSearcher.setSearchFixedDepth(true);
		simpleSearcher.setDepth(defaultDepth);
	}
	simpleSearcher.search(board);
	setSearchInProgress(false);
	setRequestStop(false);
	pthread_mutex_unlock(&mutex);
	return 0;
}

// start search
void SearchAgent::startSearch() {
	pthread_t executor;
	int ret = 0;
	ret = pthread_create( &executor, NULL, threadStartup, this);

}

// start perft
void SearchAgent::doPerft() {

	std::cout << "info Executing perft..." << std::endl;
	Board newBoard(board);
	SimplePVSearch simplePV;
	long nodes = simplePV.perft(board,this->getDepth(),1);
	std::cout << "info Finished perft: " << nodes << std::endl;

}

// stop search
void SearchAgent::stopSearch() {
	if (getSearchInProgress()) {
		setRequestStop(true);
	}
}

const long SearchAgent::getTimeToSearch() {

	if (this->getSearchMode()==SearchAgent::SEARCH_MOVETIME) {
		return this->getMoveTime();
	}

	long time=board.getSideToMove()==WHITE ?  this->getWhiteTime() : this->getBlackTime();
	long incTime=board.getSideToMove()==WHITE ? this->getWhiteIncrement() : this->getBlackIncrement();

	int movesLeft = defaultGameSize;
	if (movesToGo>0) {
		movesLeft = movesToGo;
	} else {

		for (int x=0;x<timeTableSize;x++) {
			if (time<timeTable[x][1] && time >= timeTable[x][2]) {
				movesLeft=timeTable[x][0];
				break;
			}
		}

	}

	return time/(long(movesLeft))+incTime;

}

void *threadStartup(void *_object) {
	SearchAgent *object = (SearchAgent *)_object;
	void *threadResult = object->startThreadSearch();
	return threadResult;
}

void SearchAgent::shutdown() {
	this->destroyHash();
}
