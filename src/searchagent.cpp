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

SearchAgent* SearchAgent::getInstance ()
{
	if (searchAgent == 0) {
		searchAgent = new SearchAgent();
	}
	return searchAgent;
}

SearchAgent::SearchAgent() :
	searchMode(SEARCH_TIME), searchInProgress(false), hashSize(defaultHashSize), threadNumber(1), whiteTime(0), whiteIncrement(0), blackTime(0),
	blackIncrement(0), depth(defaultDepth), movesToGo(0), moveTime(0), infinite(false), activeHash(0)
	{
	// creates initial hashtables
	createHash();
	}

// start a new game
void SearchAgent::newGame() {
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
	this->setSearchInProgress(false);

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

	SimplePVSearch simplePV(board);

	if (this->getSearchMode()==SearchAgent::SEARCH_DEPTH) {
		simplePV.setSearchFixedDepth(true);
		simplePV.setDepth(this->getDepth());
	} else if (this->getSearchMode()==SearchAgent::SEARCH_TIME ||
				this->getSearchMode()==SearchAgent::SEARCH_MOVETIME) {
		simplePV.setSearchFixedDepth(false);
		simplePV.setTimeToSearch(this->getTimeToSearch());
	} else if (this->getSearchMode()==SearchAgent::SEARCH_INFINITE) {
		simplePV.setInfinite(true);
		simplePV.setDepth(maxSearchDepth);
	} else {
		simplePV.setSearchFixedDepth(true);
		simplePV.setDepth(defaultDepth);
	}

	simplePV.search();
	return 0;
}

// start search
void SearchAgent::startSearch() {

	if (getSearchInProgress()) {
		Uci::getInstance()->text("Search in progress...");
		return;
	}

	setSearchInProgress(true);
	newSearchHash();

	pthread_t executor;
	int ret = 0;
	ret = pthread_create( &executor, NULL, threadStartup, this);

	/*pthread_t executor2;
	int ret2 = 0;
	ret2 = pthread_create( &executor2, NULL, threadStartup, this);*/

}

// start perft
void SearchAgent::doPerft() {

	if (getSearchInProgress()) {
		Uci::getInstance()->text("Search in progress...");
		return;
	}

	std::cout << "info Executing perft..." << std::endl;
	SimplePVSearch simplePV(board);
	long nodes = simplePV.perft(board,this->getDepth(),1);
	std::cout << "info Finished perft: " << nodes << std::endl;

}

// stop search
void SearchAgent::stopSearch() {
	setSearchInProgress(false);
}

const int SearchAgent::getTimeToSearch() {

	if (this->getSearchMode()==SearchAgent::SEARCH_MOVETIME) {
		return this->getMoveTime();
	}

	int time=board.getSideToMove()==WHITE ?  this->getWhiteTime() : this->getBlackTime();
	int incTime=board.getSideToMove()==WHITE ? this->getWhiteIncrement() : this->getBlackIncrement();

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

	time /= movesLeft + incTime;

	return time;

}

void *threadStartup(void *_object) {
  SearchAgent *object = (SearchAgent *)_object;
  void *threadResult = object->startThreadSearch();
  return threadResult;
}









